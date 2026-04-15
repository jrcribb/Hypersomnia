#include "game/detail/sentience/sentience_logic.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/stateless_systems/driver_system.h"
#include "game/detail/inventory/drop_from_all_slots.h"
#include "game/cosmos/create_entity.hpp"
#include "game/cosmos/data_living_one_step.h"
#include "game/detail/explosive/detonate.h"
#include "game/detail/sentience/gore/blood_splatter.hpp"
#include "game/detail/sentience/gore/idle_splatter.hpp"
#include "game/messages/pure_color_highlight_message.h"
#include "game/detail/view_input/sound_effect_input.h"

#include "augs/log.h"

static constexpr real32 arm_detach_damage_offset = 30.f;

static void try_detach_arms(
	const allocate_new_entity_access access,
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def,
	const vec2 point_of_impact
) {
	if (!sentience.is_dead()) {
		return;
	}

	/* Cap: max 2 arms total */
	if (sentience.arms_queued_for_detach >= 2) {
		return;
	}

	const auto& health = sentience.get<health_meter_instance>();
	const auto current_health = health.value;

	const bool was_headshot_kill = sentience.knockout_origin.circumstances.headshot;
	const auto base = was_headshot_kill ? sentience.health_value_at_death : 0.f;

	const auto threshold_1 = base - arm_detach_damage_offset;
	const auto threshold_2 = base - arm_detach_damage_offset * 2.f;

	const int arms_should_be_detached = 
		current_health <= threshold_2 ? 2 :
		current_health <= threshold_1 ? 1 : 0;

	if (arms_should_be_detached <= sentience.arms_queued_for_detach) {
		return;
	}

	const auto arm_flavour = sentience_def.detached_flavours.arm_upper;

	if (!arm_flavour.is_set()) {
		return;
	}

	const auto subject_transform = subject.get_logic_transform();

	/*
		Use the damage direction (if available) to determine the perpendicular
		flight directions for detached arms. This looks more natural than
		using the corpse facing since arms should fly sideways relative to
		the bullet impact direction.
	*/
	const auto base_direction = [&]() {
		if (sentience.last_corpse_damage_direction.is_nonzero()) {
			return sentience.last_corpse_damage_direction.normalize();
		}
		return vec2::from_degrees(subject_transform.rotation);
	}();
	const auto perp_up = base_direction.perpendicular_cw();

	auto determine_arm_is_upper = [&]() -> bool {
		if (point_of_impact.is_nonzero()) {
			const auto body_center = subject_transform.pos;
			const auto to_impact = point_of_impact - body_center;
			const auto dot = to_impact.dot(perp_up);
			return dot < 0;
		}
		return true;
	};

	/* Only detach one arm per call (fix for one-shot peeling off two arms) */
	const int currently_queued = sentience.arms_queued_for_detach;
	const bool is_first_arm = (currently_queued == 0);

	/*
		First arm direction is determined by impact position.
		Second arm always goes in the opposite direction of the first.
	*/
	const bool is_upper = is_first_arm ? determine_arm_is_upper() : !sentience.first_arm_queued_as_upper;
	const auto fly_direction = is_upper ? perp_up : -perp_up;

	const auto arm_velocity = fly_direction * sentience_def.base_detached_head_speed;
	const auto arm_transform = subject_transform;
	const auto typed_subject_id = subject.get_id();
	const auto head_effect = sentience_def.detached_head_particles;
	const bool should_flip = !is_upper;

	/*
		Precompute the splatter origin for the immediate arm splatter.
		This will be spawned in the post_construction callback using the arm entity as orbit subject.
	*/
	const auto arm_splatter_origin = point_of_impact.is_nonzero() ? point_of_impact : subject_transform.pos;

	/* Increment immediately to prevent duplicate spawns before post_construction fires */
	if (is_first_arm) {
		sentience.first_arm_queued_as_upper = is_upper;
	}
	sentience.arms_queued_for_detach++;

	cosmic::queue_create_entity(
		step,
		arm_flavour,
		[arm_transform, arm_velocity, typed_subject_id, should_flip](const auto& typed_entity, auto& agg) {
			typed_entity.set_logic_transform(arm_transform);

			const auto& rigid_body = typed_entity.template get<components::rigid_body>();
			rigid_body.set_velocity(arm_velocity);
			rigid_body.set_angular_velocity(7200.f);
			rigid_body.get_special().during_cooldown_ignore_collision_with = typed_subject_id;

			if (should_flip) {
				if (auto* geo = agg.template find<components::overridden_geo>()) {
					geo->flip.vertically = true;
				}
			}
		},

		[head_effect, typed_subject_id, is_upper, access, arm_splatter_origin, fly_direction](const auto& typed_entity, const logic_step step) {
			if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
				auto& s = typed_subject.template get<components::sentience>();

				if (is_upper) {
					s.detached.arm_upper = typed_entity;
				}
				else {
					s.detached.arm_lower = typed_entity;
				}

				s.when_arms_detached = step.get_cosmos().get_timestamp();
				s.pending_arm_splatters += 2;
			}

			const auto predictability = 
				step.get_settings().effect_prediction.predict_death_particles 
				? always_predictable_v
				: never_predictable_v
			;

			head_effect.start(
				step,
				particle_effect_start_input::orbit_local(typed_entity, { vec2::zero, 180 } ),
				predictability
			);

			/*
				Spawn an immediate splatter at the damage location,
				oriented in the flight direction.
				Use typed_entity (arm) as orbit subject to avoid orbit glitches.
			*/
			{
				auto& cosm = step.get_cosmos();
				auto rng = cosm.get_rng_for(typed_entity.get_id());
				::spawn_blood_splatter(access, rng, step, typed_entity, arm_splatter_origin + fly_direction * 20.f, arm_splatter_origin, 0.7f);
			}
		}
	);
}

void handle_corpse_damage(
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def,
	const vec2 impact_direction,
	const vec2 point_of_impact
) {
	if (impact_direction.is_nonzero()) {
		sentience.last_corpse_damage_direction = impact_direction;
	}

	auto& health = sentience.get<health_meter_instance>();
	auto& when_ignited = sentience.when_corpse_catched_fire;

	const auto& cosm = subject.get_cosmos();
	const auto now = cosm.get_timestamp();

	auto ignite_corpse = [&]() {
		when_ignited = now;

		{
			const auto& effect = sentience_def.corpse_catch_fire_particles;

			effect.start(
				step,
				particle_effect_start_input::orbit_local(subject, { vec2::zero, 180 } ),
				never_predictable_v
			);
		}

		{
			const auto& effect = sentience_def.corpse_catch_fire_sound;

			effect.start(
				step,
				sound_effect_start_input::at_listener(subject),
				never_predictable_v
			);
		}
	};

	const auto damage_past_breaking_point = -health.value - sentience_def.damage_required_for_corpse_explosion;
	const bool is_ignited = when_ignited.was_set();

	if (damage_past_breaking_point > 0) {
		if (!is_ignited) {
			ignite_corpse();
		}
	}

	try_detach_arms(allocate_new_entity_access(), step, subject, sentience, sentience_def, point_of_impact);
}

void handle_corpse_detonation(
	const allocate_new_entity_access access,
	const logic_step step,
	const entity_handle subject,
	components::sentience& sentience,
	const invariants::sentience& sentience_def
) {
	if (sentience.has_exploded) {
		return;
	}

	const auto& cosm = subject.get_cosmos();
	const auto& clk = cosm.get_clock();

	const auto& health = sentience.get<health_meter_instance>();
	/* The accumulated negative damage on the corpse (health.value is negative when dead) */
	const auto accumulated_corpse_damage = -health.value;

	/*
		gentle = false: violent explosion with detonation, splatters, and damage push.
		gentle = true: quiet bleed-out fall, only spawns the lying corpse entity.
	*/
	auto spawn_lying_corpse = [&](bool gentle) {
		const auto subject_pos = subject.get_logic_transform().pos;
		const auto subject_transform = subject.get_logic_transform();

		if (!gentle) {
			::detonate({
				step,
				subject,
				sentience_def.corpse_explosion,
				subject.get_logic_transform()
			}, false);

			/* Spawn blood splatters in all directions based on accumulated corpse damage */
			::spawn_blood_splatters_omnidirectional(access, step, subject, subject_pos, accumulated_corpse_damage);
		}

		/*
			Determine lying corpse rotation from last received damage direction.
			The corpse lies along the damage direction (legs point towards damage source).
		*/
		const auto lying_rotation = [&]() {
			if (sentience.last_corpse_damage_direction.is_nonzero()) {
				return (-sentience.last_corpse_damage_direction).degrees();
			}
			return subject_transform.rotation;
		}();

		const auto lying_transform = transformr(subject_pos, lying_rotation);
		const auto lying_facing = vec2::from_degrees(lying_rotation);
		const auto lying_perp = lying_facing.perpendicular_cw();

		if (!gentle) {
			/* Spawn blood splatters at positions of broken body parts on the lying corpse */
			auto rng = cosm.get_rng_for(subject);

			auto spawn_splatters_at_offset = [&](const vec2 offset, const int count) {
				const auto world_offset = lying_facing * offset.x + lying_perp * offset.y;
				const auto world_pos = subject_pos + world_offset;

				for (int i = 0; i < count; ++i) {
					const auto random_offset = vec2::from_degrees(rng.randval(0.f, 360.f)) * rng.randval(3.f, 15.f);
					::spawn_blood_splatter(access, rng, step, subject, world_pos + random_offset, world_pos, rng.randval(0.3f, 0.6f));
				}
			};

			const bool head_detached = sentience.detached.head.is_set() || sentience.knockout_origin.circumstances.headshot;
			const int num_arms = sentience.num_arms_detached();
			const bool flip_tattered = sentience.should_flip_tattered_sprite();

			if (head_detached) {
				spawn_splatters_at_offset(vec2(sentience_def.corpse_head_offset), 2);
			}

			if (num_arms >= 1) {
				auto upper_off = vec2(sentience_def.corpse_arm_upper_offset);
				if (flip_tattered) {
					upper_off.y = -upper_off.y;
				}
				spawn_splatters_at_offset(upper_off, 2);
			}

			if (num_arms >= 2) {
				spawn_splatters_at_offset(vec2(sentience_def.corpse_arm_lower_offset), 2);
			}
		}

		/* Choose the lying corpse flavour based on how many arms are detached */
		const auto num_arms = sentience.num_arms_detached();
		const auto corpse_flavour = [&]() {
			if (num_arms >= 2) {
				return sentience_def.lying_corpse_noarms_flavour;
			}
			if (num_arms >= 1) {
				return sentience_def.lying_corpse_noarm_flavour;
			}
			return sentience_def.lying_corpse_flavour;
		}();

		if (corpse_flavour.is_set()) {
			const auto typed_subject_id = subject.get_id();
			const bool should_flip = sentience.should_flip_tattered_sprite();

			/*
				Lying corpse inherits velocity from the tattered corpse.
				On violent explosion, also add a push in the last damage direction.
			*/
			const auto tattered_velocity = subject.get_effective_velocity();
			const auto damage_push = [&]() {
				if (sentience.last_corpse_damage_direction.is_nonzero()) {
					return sentience.last_corpse_damage_direction.normalize() * 1300.f;
				}
				return vec2::zero;
			}();
			const auto lying_velocity = tattered_velocity + damage_push;

			cosmic::queue_create_entity(
				step,
				corpse_flavour,
				[lying_transform, lying_velocity, should_flip](const auto& typed_entity, auto& agg) {
					typed_entity.set_logic_transform(lying_transform);

					const auto& rigid_body = typed_entity.template get<components::rigid_body>();
					rigid_body.set_velocity(lying_velocity);
					LOG_NVPS(lying_velocity.length());
					rigid_body.set_angular_velocity(0.f);

					if (should_flip) {
						if (auto* geo = agg.template find<components::overridden_geo>()) {
							geo->flip.vertically = true;
						}
					}
				},

				[typed_subject_id, gentle](const auto& typed_entity, const logic_step step) {
					if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
						auto& s = typed_subject.template get<components::sentience>();
						s.detached.lying_corpse = typed_entity;
					}

					/*
						Send a white highlight for the lying corpse when it first appears.
						Analogous to the damage highlight in audiovisual_state.cpp.
					*/

					{
						constexpr float highlight_size_bounce_mult = 1.5f;

						messages::pure_color_highlight msg;
						msg.subject = typed_entity;
						msg.input.starting_alpha_ratio = 1.f;
						msg.input.maximum_duration_seconds = 0.25f;
						msg.input.color = white;

						if (!gentle) {
							msg.input.size_mult_start = highlight_size_bounce_mult;
						}

						if (gentle) {
							msg.input.size_mult_start = 1.15f;
							msg.input.maximum_duration_seconds = 0.18f;
						}

						step.post_message(msg);
					}
				}
			);
		}

		sentience.has_exploded = true;

		/* Make the tattered corpse invisible by reinferring colliders (becomes sensor) */
		subject.infer_colliders_from_scratch();
	};

	const auto& when_ignited = sentience.when_corpse_catched_fire;
	const auto damage_past_breaking_point = accumulated_corpse_damage - sentience_def.damage_required_for_corpse_explosion;
	const bool is_ignited = when_ignited.was_set();

	if (is_ignited) {
		const auto secs_simulated_by_damaging = (damage_past_breaking_point * 2) / 1000.f;
		const auto passed_secs = clk.get_passed_secs(when_ignited);

		if (passed_secs + secs_simulated_by_damaging >= sentience_def.corpse_burning_seconds) {
			spawn_lying_corpse(false);
		}
	}
	else if (sentience.is_dead() && sentience.idle_blood_drip_count >= IDLE_SPLATTER_MAX_CORPSE_DRIPS) {
		/*
			The tattered corpse has bled out.
			Gently fall to the ground without any explosion effects.
		*/
		spawn_lying_corpse(true);
	}
}

void perform_knockout(
	const entity_id& subject_id, 
	const logic_step step, 
	const vec2 direction,
	const damage_origin& origin,
	const vec2 point_of_impact
) {
	auto& cosm = step.get_cosmos(); 

	const auto subject = cosm[subject_id];

	if (subject.dead()) {
		return;
	}

	subject.dispatch_on_having_all<invariants::sentience>([&](const auto& typed_subject) {
		auto& sentience = typed_subject.template get<components::sentience>();
		auto& sentience_def = typed_subject.template get<invariants::sentience>();
		
		if (typed_subject.template get<components::item_slot_transfers>().allow_drop_and_pick) {
			if (const auto* const container = typed_subject.template find<invariants::container>()) {
				drop_from_all_slots(*container, entity_handle(typed_subject), sentience_def.drop_impulse_on_knockout, step);
			}
		}
		else {
			::queue_delete_all_owned_items(step, typed_subject);
		}

		if (const auto* const driver = typed_subject.template find<components::driver>();
			driver != nullptr && cosm[driver->owned_vehicle].alive()
		) {
			driver_system().release_car_ownership(typed_subject);
		}

		impulse_input knockout_impulse;
		knockout_impulse.linear = direction;
		knockout_impulse.angular = 1.f;

		const auto knocked_out_body = typed_subject.template get<components::rigid_body>();
		knocked_out_body.apply(knockout_impulse * sentience_def.knockout_impulse);

		{
			auto& special_physics = typed_subject.get_special_physics();

			const auto disable_collision_for_ms = 300;

			special_physics.dropped_or_created_cooldown.set(
				disable_collision_for_ms,
				cosm.get_timestamp()
			);

			special_physics.during_cooldown_ignore_collision_with = origin.sender.capability_of_sender;
		}

		sentience.when_knocked_out = cosm.get_timestamp();
		sentience.knockout_origin = origin;

		if (sentience.is_dead()) {
			sentience.health_value_at_death = sentience.template get<health_meter_instance>().value;
		}

		if (sentience.is_dead() && origin.circumstances.headshot) {
			const auto head_transform = typed_subject.get_logic_transform();
			/*
				Head flies in the opposite direction of the damage,
				at reduced speed for a heavier feel.
			*/
			const auto head_velocity = -direction * sentience_def.base_detached_head_speed * 0.5f;
			const auto typed_subject_id = typed_subject.get_id();
			const auto head_effect = sentience_def.detached_head_particles;

			/*
				Precompute the splatter origin and flight direction for the immediate head splatter.
				This will be spawned in the post_construction callback using the head entity as orbit subject.
			*/
			const auto head_splatter_origin = point_of_impact.is_nonzero() ? point_of_impact : head_transform.pos;
			const auto head_flight_dir = head_velocity.is_nonzero() ? vec2(head_velocity).normalize() : -direction;
			const auto head_access = allocate_new_entity_access();

			auto spawn_detached_body_part = [&](const auto& flavour) {
				cosmic::queue_create_entity(
					step,
					flavour,
					[head_transform, head_velocity, typed_subject_id](const auto& typed_entity, auto&) {
						typed_entity.set_logic_transform(head_transform);

						const auto& rigid_body = typed_entity.template get<components::rigid_body>();

						rigid_body.set_velocity(head_velocity);
						rigid_body.set_angular_velocity(7200.f);
						rigid_body.get_special().during_cooldown_ignore_collision_with = typed_subject_id;
					},

					[head_effect, typed_subject_id, head_access, head_splatter_origin, head_flight_dir](const auto& typed_entity, const logic_step step) {
						if (const auto typed_subject = step.get_cosmos()[typed_subject_id]) {
							auto& s = typed_subject.template get<components::sentience>();
							s.detached.head = typed_entity;
							s.pending_head_splatters = 2;
						}

						const auto predictability = 
							step.get_settings().effect_prediction.predict_death_particles 
							? always_predictable_v
							: never_predictable_v
						;

						head_effect.start(
							step,
							particle_effect_start_input::orbit_local(typed_entity, { vec2::zero, 180 } ),
							predictability
						);

						head_effect.start(
							step,
							particle_effect_start_input::orbit_local(typed_subject_id, { vec2::zero, 180 } ),
							predictability
						);

						/*
							Spawn an immediate splatter at the damage location,
							oriented in the head's flight direction.
							Use typed_entity (head) as orbit subject to avoid orbit glitches.
						*/
						{
							auto& cosm = step.get_cosmos();
							auto rng = cosm.get_rng_for(typed_entity.get_id());
							::spawn_blood_splatter(head_access, rng, step, typed_entity, head_splatter_origin + head_flight_dir * 20.f, head_splatter_origin, 1.2f);
						}
					}
				);
			};

			spawn_detached_body_part(sentience_def.detached_flavours.head);
		}

		if (sentience.is_dead()) {
			try_detach_arms(allocate_new_entity_access(), step, typed_subject, sentience, sentience_def, point_of_impact);
		}
	});
}

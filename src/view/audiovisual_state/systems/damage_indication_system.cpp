#include "augs/templates/container_templates.h"
#include "augs/drawing/drawing.hpp"
#include "augs/math/camera_cone.h"
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

#include "augs/gui/text/printer.h"

#include "view/viewables/images_in_atlas_map.h"
#include "view/audiovisual_state/systems/damage_indication_system.h"
#include "view/damage_indication_settings.h"

void damage_indication_system::clear() {
	streaks.clear();
}

void damage_indication_system::add_white_highlight(
	const entity_id subject,
	const messages::health_event::target_type target,
	const float original_ratio
) {
	auto& highlight = active_white_highlights[subject];

	const bool should_reset_ratio = 
		original_ratio >= highlight.original_ratio
		|| target != highlight.target
	;

	if (should_reset_ratio) {
		highlight.original_ratio = original_ratio;
		highlight.target = target;
	}

	highlight.time_of_occurence_seconds = global_time_seconds;
}

void damage_indication_system::add(const entity_id subject, const damage_event::input new_in, bool merge) {
	auto& streak = streaks[subject];

	if (merge) {
		auto& events = streak.events;

		if (events.size() > 0) {
			events.back().in.amount += new_in.amount;
			streak.total += new_in.amount;
			return;
		}
	}

	damage_event new_event;

	new_event.in = new_in;
	new_event.time_of_occurence_seconds = global_time_seconds;
	new_event.offset_slot = streak.current_event_slot_offset++;

	streak.events.push_back(new_event);
	streak.total += new_in.amount;
	++streak.total_damage_events;

	active_white_highlights[subject].time_of_occurence_seconds = global_time_seconds;
}

std::optional<damage_indication_system::white_highlight> damage_indication_system::find_white_highlight(const entity_id id, const damage_indication_settings& settings) const {
	if (const auto found = mapped_or_nullptr(active_white_highlights, id)) {
		const auto passed = global_time_seconds - found->time_of_occurence_seconds;
		const auto passed_mult = std::clamp(passed / settings.white_damage_highlight_secs, 0.0, 1.0);

		return white_highlight { found->original_ratio, static_cast<float>(passed_mult) };
	}

	return std::nullopt;
}

void damage_indication_system::advance(
	const damage_indication_settings& settings,
	const augs::delta dt
) {
	const auto secs = dt.in_seconds();
	global_time_seconds += secs;

	const auto& accum_speed = settings.numbers_accumulation_speed;

	erase_if(
		active_white_highlights,
		[&](const auto& entry) {
			auto& highlight = entry.second;

			const auto passed = global_time_seconds - highlight.time_of_occurence_seconds;

			return passed >= settings.white_damage_highlight_secs;
		}
	);

	erase_if(
		streaks,
		[&](auto& entry) {
			auto& streak = entry.second;

			if (streak.total_damage_events > 1) {
				if (accum_speed.is_enabled) {
					streak.displayed_total += accum_speed.value * secs;
					streak.displayed_total = std::min(streak.displayed_total, streak.total);
				}
				else {
					streak.displayed_total = streak.total;
				}
			}

			erase_if(
				streak.events,
				[&](auto& e) {
					const auto passed = global_time_seconds - e.time_of_occurence_seconds;

					const auto max_total_lifetime = 
						settings.indicator_fading_duration_secs 
						+ settings.single_indicator_lifetime_secs
					;

					if (passed >= max_total_lifetime) {
						streak.when_last_event_disappeared = global_time_seconds;
						return true;
					}

					if (accum_speed.is_enabled) {
						e.displayed_amount += accum_speed.value * secs;
						e.displayed_amount = std::min(e.displayed_amount, e.in.amount);
					}
					else {
						e.displayed_amount = e.in.amount;
					}

					return false;
				}
			);

			if (streak.events.empty()) {
				streak.current_event_slot_offset = 0;

				const auto passed_idle = global_time_seconds - streak.when_last_event_disappeared;

				const auto max_total_idle_lifetime = 
					settings.indicator_fading_duration_secs
					+ settings.accumulative_indicator_idle_lifetime_secs
				;

				if (passed_idle >= max_total_idle_lifetime) {
					return true;
				}
			}

			return false;
		}
	);
}

#include "augs/log.h"

void damage_indication_system::draw_indicators(
	const std::function<bool(const_entity_handle)> is_reasonably_in_view,
	const const_entity_handle& viewed_character,
	const interpolation_system& interp,
	const damage_indication_settings& settings,
	const images_in_atlas_map& game_images,
	const all_loaded_gui_fonts& fonts,
	const augs::drawer output,
	const camera_cone cone
) const {
	const auto& cosm = viewed_character.get_cosmos();

	const auto& meter_metas = cosm.get_common_significant().meters;
	const auto shield_icon = std::get<electric_shield_perk>(cosm.get_common_significant().perks).appearance.icon;
	const auto shield_destruction_icon = cosm.get_common_assets().broken_shield_icon;
	const auto shield_color = rgba(std::get<personal_electricity_meter>(meter_metas).appearance.bar_color).mult_brightness(1.5f);

	// const auto& cumulative_damage_font = fonts.large_numbers;

	auto get_indicator_font = [&](const int damage_amount) -> auto& {
		if (damage_amount < settings.small_damage_threshold) {
			return fonts.gui;
		}

		if (damage_amount < settings.medium_damage_threshold) {
			return fonts.larger_gui;
		}

		if (damage_amount < 100) {
			return fonts.medium_numbers;
		}

		return fonts.large_numbers;
	};

	auto get_indicator_color = [&](const auto& in) {
		if (in.critical) {
			return settings.critical_color;
		}

		switch (in.type) {
			case damage_event::event_type::HEALTH:
				return white;
			case damage_event::event_type::CRITICAL:
				return settings.critical_color;
			case damage_event::event_type::SHIELD:
			case damage_event::event_type::SHIELD_DRAIN:
				return shield_color;
			case damage_event::event_type::SHIELD_DESTRUCTION:
				return rgba(shield_color).mult_brightness(1.25f);
		}
	};

	const auto indicator_offsets = settings.single_indicator_offsets;

	for (const auto& s : streaks) {
		const auto subject_id = s.first;
		const auto subject = cosm[subject_id];

		const auto& streak = s.second;

		const auto border_color = subject == viewed_character ? red : black;

		{
			/* Render the streak itself */

			if (streak.total_damage_events > 1) {
				if (const auto transform = subject.find_viewing_transform(interp)) {
					if (streak.last_visible_pos == std::nullopt || is_reasonably_in_view(subject)) {
						streak.last_visible_pos = transform->pos;
					}

					auto text_pos = cone.to_screen_space(*streak.last_visible_pos + settings.accumulative_indicator_offset);
					auto text_color = white;

					if (streak.events.empty()) {
						const auto passed = global_time_seconds - streak.when_last_event_disappeared;
						const auto fading_progress = passed - settings.accumulative_indicator_idle_lifetime_secs;

						if (fading_progress >= 0.0f) {
							const auto fading_mult = std::sqrt(fading_progress / settings.indicator_fading_duration_secs);

							text_pos.y -= fading_mult * settings.indicator_rising_speed;
							text_color.mult_alpha(1 - fading_mult);
						}
					}

					const auto round_total = static_cast<int>(streak.displayed_total);
					const auto total_text = std::to_string(round_total);

					auto border = border_color;
					border.a = text_color.a;

					augs::gui::text::print_stroked(
						output,
						text_pos,
						{ total_text, { get_indicator_font(round_total), text_color } },
						{ augs::ralign::CX, augs::ralign::CY },
						border
					);
				}
			}
		}

		for (const auto& e : streak.events) {
			const auto passed = global_time_seconds - e.time_of_occurence_seconds;

			const auto current_offset = indicator_offsets[e.offset_slot % indicator_offsets.size()];
			const auto fading_progress = passed - settings.single_indicator_lifetime_secs;

			const auto round_amount = static_cast<int>(e.displayed_amount);

			if (round_amount == 0) {
				continue;
			}

			const auto indicator_number_text = std::to_string(round_amount);

			const auto& indicator_font = get_indicator_font(round_amount);
			const auto offset_mult = static_cast<float>(indicator_font.metrics.get_height()) / fonts.gui.metrics.get_height();

			auto world_pos = e.in.pos;

			if (viewed_character == subject) {
				const auto viewed_pos = subject.get_viewing_transform(interp).pos;

				if (!e.first_pos.has_value()) {
					e.first_pos = viewed_pos;
				}

				world_pos += viewed_pos - *e.first_pos;
			}

			auto text_pos = cone.to_screen_space(world_pos + current_offset * offset_mult);
			auto text_color = get_indicator_color(e.in);

			if (fading_progress >= 0.0f) {
				const auto fading_mult = std::sqrt(fading_progress / settings.indicator_fading_duration_secs);

				text_pos.y -= fading_mult * settings.indicator_rising_speed;
				text_color.mult_alpha(1 - fading_mult);
			}

			auto border = border_color;
			border.a = text_color.a;

			const auto pixel_perfect_text_pos = vec2i(text_pos);

			augs::gui::text::print_stroked(
				output,
				pixel_perfect_text_pos,
				{ indicator_number_text, { indicator_font, text_color } },
				{ augs::ralign::R },
				border
			);

			if (e.in.type == damage_event::event_type::SHIELD) {
				if (const auto& entry = game_images.at(shield_icon).diffuse; entry.exists()) {
					const auto shield_icon_pos = pixel_perfect_text_pos - vec2i(0, indicator_font.metrics.descender);

					output.aabb_lt(entry, shield_icon_pos, text_color);
				}
			}
			else if (e.in.type == damage_event::event_type::SHIELD_DESTRUCTION) {
				if (const auto& entry = game_images.at(shield_destruction_icon).diffuse; entry.exists()) {
					const auto shield_icon_pos = pixel_perfect_text_pos - vec2i(0, indicator_font.metrics.descender);

					output.aabb_lt(entry, shield_icon_pos, text_color);
				}
			}
		}
	}
}
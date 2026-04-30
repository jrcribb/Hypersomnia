#pragma once
#include "game/components/hand_fuse_component.h"
#include "game/components/movement_component.h"
#include "game/cosmos/for_each_entity.h"
#include "game/detail/explosive/like_explosive.h"
#include "game/detail/path_navigation/navigate_pathfinding.hpp"
#include "game/detail/pathfinding/pathfinding.h"
#include "game/enums/filters.h"
#include "game/modes/ai/ai_character_context.h"
#include "game/modes/ai/intents/calc_pathfinding_request.hpp"

/*
	Emergency avoidance layer — danger (grenade) avoidance component.

	Called from the emergency avoidance layer (PHASE 8) alongside
	update_bot_avoidance. should_run_avoidance_update must be computed
	by the caller and shared between both avoidance functions.

	When active, danger avoidance has HIGHER priority than bot avoidance:
	it is applied last (after bot_avoidance) so its direction wins.

	State fields in arena_mode_ai_state:
	  danger_pathfinding_request  — target cover cell; nullopt = in cover or inactive
	  danger_pathfinding_time_left — seconds remaining; -1.0f = inactive

	Behaviour:
	  - Each scan tick (throttled by should_run_avoidance_update) the closest
	    visible thrown explosive is found and cover is recalculated.  This runs
	    even while a danger navigation is already active so the target stays
	    up-to-date as the grenade moves.
	  - When the bot reaches cover (path_completed on the danger path) the
	    request is cleared and the bot stands still until the timer expires or
	    a new visible threat is detected on the next scan tick.
*/

/*
	Extra time added to the grenade fuse time so the bot keeps avoiding
	even during the explosion itself.
*/
constexpr float EXPLOSION_BUFFER_SECS = 0.2f;

inline void update_danger_avoidance(
	const ai_character_context& ctx,
	components::movement& movement,
	const cosmos_navmesh& navmesh,
	const navigate_pathfinding_result& move_result,
	const float dt_secs,
	const bool should_run_avoidance_update,
	const bool is_freeze_time
) {
	if (is_freeze_time) {
		return;
	}

	auto& ai_state = ctx.ai_state;
	const auto& cosm = ctx.cosm;
	const auto& physics = ctx.physics;
	const auto character_pos = ctx.character_pos;
	const auto si = cosm.get_si();
	const auto& clk = cosm.get_clock();

	/* Tick the danger timer down every frame */
	if (ai_state.danger_pathfinding_time_left >= 0.0f) {
		ai_state.danger_pathfinding_time_left -= dt_secs;

		if (ai_state.danger_pathfinding_time_left < 0.0f) {
			ai_state.danger_pathfinding_time_left = -1.0f;
			ai_state.danger_pathfinding_request = std::nullopt;
		}
	}

	/*
		When cover is reached, clear the navigation request so the bot stands
		still.  The timer keeps running; the scan will issue a new request if
		the grenade is still visible from the new position.
	*/
	if (move_result.path_completed
		&& ai_state.current_pathfinding_request == ai_state.danger_pathfinding_request
		&& ai_state.danger_pathfinding_request.has_value()
	) {
		ai_state.danger_pathfinding_request = std::nullopt;
	}

	/*
		Scan for the closest visible thrown explosive and recalculate cover.
		Runs every throttled tick, even when a danger session is already active,
		so the target stays accurate as the grenade moves.
	*/
	if (should_run_avoidance_update) {
		const auto filter = predefined_queries::pathfinding();

		struct grenade_candidate {
			vec2 pos;
			float remaining_secs;
			float dist_sq;
		};

		std::optional<grenade_candidate> closest;

		cosm.for_each_having<components::hand_fuse>(
			[&](const auto& grenade_handle) {
				if (!::is_like_thrown_explosive(grenade_handle)) {
					return;
				}

				const auto grenade_pos = grenade_handle.get_logic_transform().pos;
				const auto dist_sq = (grenade_pos - character_pos).length_sq();

				if (dist_sq > COVER_SEARCH_RADIUS * COVER_SEARCH_RADIUS) {
					return;
				}

				const auto los_check = physics.ray_cast_px(si, character_pos, grenade_pos, filter);

				if (los_check.hit) {
					/* Wall between character and grenade — already safe */
					return;
				}

				if (!closest.has_value() || dist_sq < closest->dist_sq) {
					const auto* fuse = grenade_handle.template find<components::hand_fuse>();

					const auto remaining_secs =
						clk.get_remaining_ms(fuse->fuse_delay_ms, fuse->when_armed) / 1000.0f
						+ EXPLOSION_BUFFER_SECS
					;

					closest = grenade_candidate{ grenade_pos, remaining_secs, dist_sq };
				}
			}
		);

		if (closest.has_value()) {
			const auto cover_pos = ::find_closest_cover(navmesh, character_pos, closest->pos, physics, si);

			if (cover_pos.has_value()) {
				auto req = ai_pathfinding_request::to_position(*cover_pos);
				req.resolved_cell = ::resolve_cell_for_position(navmesh, *cover_pos);
				ai_state.danger_pathfinding_time_left = closest->remaining_secs;
				ai_state.danger_pathfinding_request = req;
			}
		}
	}

	const bool danger_active = ai_state.danger_pathfinding_time_left >= 0.0f;

	if (!danger_active) {
		return;
	}

	if (ai_state.danger_pathfinding_request.has_value()) {
		/* Navigating to cover — sprint, dash, and follow the pathfinding direction */
		movement.flags.sprinting = move_result.can_sprint;
		movement.flags.dashing = move_result.can_sprint;

		if (move_result.movement_direction.has_value()
			&& ai_state.current_pathfinding_request == ai_state.danger_pathfinding_request
		) {
			movement.flags.set_from_closest_direction(*move_result.movement_direction);
		}
	}
	else {
		/* In cover — stand completely still until the timer expires */
		movement.reset_movement_flags();
	}
}

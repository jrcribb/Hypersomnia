#pragma once
#include <optional>
#include "augs/math/vec2.h"

/*
	Combat behavior.
	Bot is engaged in combat with an enemy target.
	
	The combat_target tracking (who we're fighting, timeouts, last known positions)
	is handled by ai_target_tracking in arena_mode_ai_state, NOT here.
	
	This struct only tracks internal combat behavior state like:
	- Whether we've dashed at the last seen target position
	- Whether we've dashed at the last known target position
*/

struct ai_behavior_combat {
	// GEN INTROSPECTOR struct ai_behavior_combat
	std::optional<vec2> dashed_to_visual_pos;
	std::optional<vec2> dashed_to_known_pos;
	// END GEN INTROSPECTOR

	bool has_dashed_to_visual_pos(const vec2 pos) const {
		if (!dashed_to_visual_pos.has_value()) {
			return false;
		}

		constexpr float EPSILON = 10.0f;
		return (*dashed_to_visual_pos - pos).length() < EPSILON;
	}

	bool has_dashed_to_known_pos(const vec2 pos) const {
		if (!dashed_to_known_pos.has_value()) {
			return false;
		}

		constexpr float EPSILON = 10.0f;
		return (*dashed_to_known_pos - pos).length() < EPSILON;
	}

	void mark_dashed_to_visual_pos(const vec2 pos) {
		dashed_to_visual_pos = pos;
	}

	void mark_dashed_to_known_pos(const vec2 pos) {
		dashed_to_known_pos = pos;
	}

	bool operator==(const ai_behavior_combat&) const = default;

	void process(ai_behavior_process_ctx&) {}
};

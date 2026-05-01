#pragma once
#include "game/cosmos/cosmos.h"
#include "game/cosmos/entity_handle.h"
#include "game/components/sentience_component.h"
#include "game/components/gun_component.h"
#include "game/modes/ai/behaviors/ai_behavior_variant.hpp"

/*
	Returns the bot_aim_radius_to_shoot for the first wielded gun found on the
	character, falling back to 25.0f when no gun is wielded.
*/
template <class CharacterHandle>
real32 calc_aim_radius_to_shoot(const CharacterHandle& character_handle) {
	const auto& cosm = character_handle.get_cosmos();

	for (const auto& item_id : character_handle.get_wielded_items()) {
		if (const auto gun_def = cosm[item_id].template find<invariants::gun>()) {
			return gun_def->bot_aim_radius_to_shoot;
		}
	}

	return 25.0f;
}

/*
	Stateless calculation of hand_flags.
	
	This function determines whether the bot should be holding the trigger based on:
	- Current behavior type (planting, combat, etc.)
	- Combat target state (has target, is aiming correctly)
	- Frozen state
	
	Always returns an up-to-date state for the hands.
	
	target_enemy_pos: The position to aim at (last known or current enemy position).
*/

struct hand_flags_result {
	bool hand_flag_0 = false;
	bool hand_flag_1 = false;
};

template <typename CharacterHandle>
inline hand_flags_result calc_hand_flags(
	const ai_behavior_variant& behavior,
	const bool target_acquired,
	const vec2 target_enemy_pos,
	const vec2 character_pos,
	CharacterHandle character_handle
) {
	hand_flags_result result;

	/*
		If planting, set hand_flags to trigger arming_requested on the bomb.
	*/
	if (const auto* plant = ::get_behavior_if<ai_behavior_plant>(behavior)) {
		if (plant->is_planting) {
			result.hand_flag_0 = true;
			result.hand_flag_1 = true;
			return result;
		}
	}

	/*
		Combat target: trigger if aiming correctly.
		Instead of comparing angles, project the enemy position onto the bullet
		trajectory and check if the perpendicular (lateral) distance from the
		enemy to the aimed ray is within the threshold.
	*/
	if (const auto* combat = ::get_behavior_if<ai_behavior_combat>(behavior)) {
		if (target_acquired) {
			const auto to_enemy = target_enemy_pos - character_pos;

			if (auto crosshair = character_handle.find_crosshair()) {
				const auto current_aim = vec2(crosshair->base_offset).normalize();

				/*
					Project the enemy vector onto the aim ray and compute the
					perpendicular (lateral) offset from that ray.
				*/
				const auto proj_len = to_enemy.dot(current_aim);
				const auto lateral_dist = (to_enemy - current_aim * proj_len).length();

				if (proj_len > 0.0f && lateral_dist <= ::calc_aim_radius_to_shoot(character_handle)) {
					result.hand_flag_0 = true;
					result.hand_flag_1 = true;
				}
			}
		}
	}

	/*
		If frozen, don't trigger.
	*/
	if (character_handle.is_frozen()) {
		result.hand_flag_0 = false;
		result.hand_flag_1 = false;
	}

	return result;
}

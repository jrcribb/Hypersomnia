#pragma once
#include "game/enums/faction_type.h"

/*
	Stateless calculation of whether to fully acquire a target just from hearing.
	
	For Metropolis only, returns true when the bomb is planted, otherwise false.
	This causes bots to immediately engage enemies they hear near a bomb site
	when defending.

	already_acquired_by_hearing gates this: once the bot has already engaged
	via hearing once this bomb plant, it won't reaggregate purely from sound
	again — preventing endless chase from footstep spam.
*/

inline bool should_acquire_target_by_hearing(
	const faction_type bot_faction,
	const bool bomb_planted,
	const bool already_acquired_by_hearing
) {
	if (already_acquired_by_hearing) {
		return false;
	}

	if (bot_faction == faction_type::METROPOLIS && bomb_planted) {
		return true;
	}

	return false;
}

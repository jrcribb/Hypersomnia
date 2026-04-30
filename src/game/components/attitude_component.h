#pragma once
#include "augs/misc/enum/enum_boolset.h"
#include "game/cosmos/entity_id.h"
#include "game/enums/attitude_type.h"
#include "game/enums/faction_type.h"

#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "augs/pad_bytes.h"

namespace components {
	struct attitude {
		// GEN INTROSPECTOR struct components::attitude
		faction_type official_faction;
		// END GEN INTROSPECTOR
	};
}

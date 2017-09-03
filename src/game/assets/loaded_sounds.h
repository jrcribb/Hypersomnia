#pragma once
#include "augs/misc/enum_associative_array.h"

#include "game/assets/assets_declarations.h"

#include "game/assets/sound_buffer_id.h"
#include "augs/audio/sound_buffer.h"

struct loaded_sounds : public asset_map<
	assets::sound_buffer_id,
	augs::sound_buffer
> {
	loaded_sounds() = default;
	loaded_sounds(const sound_buffer_definitions&);
};
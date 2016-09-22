#pragma once
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include <vector>

#include "game/components/transform_component.h"

class interpolation_system {
public:
	std::vector<components::transform> per_entity_cache;

	void integrate_interpolated_transforms(const cosmos&, float seconds);

	void construct(const const_entity_handle);
	void destruct(const const_entity_handle);

	components::transform& get_interpolated(const const_entity_handle);
	void reserve_caches_for_entities(const size_t);
	void write_current_to_interpolated(const const_entity_handle);
};
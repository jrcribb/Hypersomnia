#include "position_scripts.h"
#include "game/components/transform_component.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/entity_handle.h"
#include "game/detail/physics_scripts.h"
#include "game/cosmos.h"

vec2 position(const_entity_handle e) {
	return e.get<components::transform>().pos;
}

vec2 mass_center(const_entity_handle e) {
	return e.get<components::physics>().get_mass_position();
}

vec2 mass_center_or_position(const_entity_handle e) {
	if (e.has<components::physics>()) {
		const auto& phys = e.get<components::physics>();
		
		if(phys.is_constructed())
			return phys.get_mass_position();
	}

	return position(e);
}

float rotation(const_entity_handle e) {
	return e.get<components::transform>().rotation;
}

vec2 orientation(const_entity_handle of) {
	return vec2().set_from_degrees(rotation(of));
}

vec2 direction(const_entity_handle a, const_entity_handle b) {
	return position(a) - position(b);
}

vec2 direction_norm(const_entity_handle a, const_entity_handle b) {
	return direction(a, b).normalize();
}

vec2 velocity(const_entity_handle e) {
	return e.get_cosmos()[e.get_owner_body()].get<components::physics>().velocity();
}

float speed(const_entity_handle e) {
	return velocity(e).length();
}

bool is_entity_physical(const_entity_handle e) {
	return e.has<components::fixtures>() || e.has<components::physics>();
}

float distance_sq(const_entity_handle a, const_entity_handle b) {
	return direction(a, b).length_sq();
}

float distance(const_entity_handle a, const_entity_handle b) {
	return sqrt(distance_sq(a, b));
}

void set_velocity(entity_handle h, vec2 v) {
	h.get<components::physics>().set_velocity(v);
}
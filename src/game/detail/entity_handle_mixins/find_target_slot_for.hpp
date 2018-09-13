#pragma once
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/detail/inventory/item_mounting.hpp"

template <class E>
template <class handle_type>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::find_slot_for(
	const handle_type item,
   	const slot_finding_opts& opts
) const {
	const auto& searched_root_container = *static_cast<const E*>(this);
	auto& cosmos = item.get_cosmos();

	ensure(item.alive()) 
	ensure(searched_root_container.alive());
	ensure(item.get_id() != searched_root_container.get_id());

	inventory_slot_id target_slot;

	auto check_slot = [&](const auto slot) {
		if (slot.dead() || target_slot.is_set()) {
			return;
		}

		if (slot.can_contain(item)) {
			target_slot = slot;
		}
	};

	for (const auto& o : opts) {
		if (o == slot_finding_opt::CHECK_WEARABLES) {
			check_slot(searched_root_container[slot_function::BELT]);
			check_slot(searched_root_container[slot_function::BACK]);
			check_slot(searched_root_container[slot_function::TORSO_ARMOR]);
			check_slot(searched_root_container[slot_function::HAT]);
			check_slot(searched_root_container[slot_function::PERSONAL_DEPOSIT]);
		}
		else if (o == slot_finding_opt::CHECK_HANDS) {
			check_slot(searched_root_container[slot_function::PRIMARY_HAND]);
			check_slot(searched_root_container[slot_function::SECONDARY_HAND]);
		}
		else if (o == slot_finding_opt::CHECK_CONTAINERS) {
			if (const auto personal_slot = searched_root_container[slot_function::PERSONAL_DEPOSIT]) {
				if (const auto personal_wearable = personal_slot.get_item_if_any()) {
					check_slot(personal_wearable[slot_function::ITEM_DEPOSIT]);
				}
			}

			if (const auto back_slot = searched_root_container[slot_function::BACK]) {
				if (const auto back_wearable = back_slot.get_item_if_any()) {
					check_slot(back_wearable[slot_function::ITEM_DEPOSIT]);
				}
			}
		}

		if (target_slot.is_set()) {
			break;
		}
	}

	return cosmos[target_slot];
}

template <class E>
template <class handle_type>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::find_holstering_slot_for(const handle_type holstered_item) const {
	return find_slot_for(holstered_item, { slot_finding_opt::CHECK_WEARABLES, slot_finding_opt::CHECK_CONTAINERS });
}

template <class E>
bool is_weapon_like(const E& typed_handle) {
	return typed_handle.template has<components::gun>() || typed_handle.template has<components::hand_fuse>();
}

template <class E>
template <class handle_type>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::find_pickup_target_slot_for(const handle_type picked_item) const {
	auto finding_order = slot_finding_opts { 
		slot_finding_opt::CHECK_WEARABLES, 
		slot_finding_opt::CHECK_CONTAINERS, 
		slot_finding_opt::CHECK_HANDS 
	};

	{
		if (is_weapon_like(picked_item)) {
			/* If it is a weapon, try to hold them in hands before trying containers. */

			std::swap(finding_order.front(), finding_order.back());
		}
	}

	return find_slot_for(picked_item, finding_order);
}

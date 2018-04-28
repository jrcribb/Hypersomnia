#include "game/transcendental/cosmos_solvable.h"

#include "augs/templates/introspect.h"

const cosmos_solvable cosmos_solvable::zero;

void cosmos_solvable::clear() {
	destroy_all_caches();

	for_each_pool([](auto& p){
		p.clear();
	});

	guid_to_id.clear();
	significant.clock = {};
}

std::size_t cosmos_solvable::get_entities_count() const {
	std::size_t total = 0u;

	for_each_pool([&total](const auto& p) { total += p.size(); } );
	ensure_eq(guid_to_id.size(), total);
	return total;
}

bool cosmos_solvable::empty() const {
	return get_entities_count() == 0;
}

cosmos_solvable::cosmos_solvable(const cosmic_pool_size_type reserved_entities) {
	reserve_storage_for_entities(reserved_entities);
}

static auto make_reserver(const std::size_t n) {
	return [n](auto, auto& sys) {
		using T = std::decay_t<decltype(sys)>;

		if constexpr(can_reserve_caches_v<T>) {
			sys.reserve_caches_for_entities(n);
		}
	};
}

void cosmos_solvable::reserve_storage_for_entities(const cosmic_pool_size_type n) {
	for_each_pool([n](auto& p){
		p.reserve(n);
	});

	augs::introspect(make_reserver(n), inferred);
}

void cosmos_solvable::destroy_all_caches() {
	inferred.~cosmos_solvable_inferred();
	new (&inferred) cosmos_solvable_inferred;

#if TODO
	const auto n = significant.entity_pool.capacity();

	augs::introspect(make_reserver(n), inferred);
#endif
}

void cosmos_solvable::remap_guids() {
	auto& guids = guid_to_id;
	guids.clear();

	for_each_entity([&](auto& subject, const auto iteration_index) {
		using E = typename std::decay_t<decltype(subject)>::used_entity_type;

		const auto id = entity_id(
			significant.template get_pool<E>().to_id(iteration_index),
			entity_type_id::of<E>()
		);

		guids[subject.guid] = id;
	});
}

void cosmos_solvable::increment_step() {
	++significant.clock.now.step;
}

double cosmos_solvable::get_total_seconds_passed(const double view_interpolation_ratio) const {
	return get_total_seconds_passed() + get_fixed_delta().per_second(view_interpolation_ratio);
}

double cosmos_solvable::get_total_seconds_passed() const {
	return significant.clock.now.step * get_fixed_delta().in_seconds<double>();
}

decltype(augs::stepped_timestamp::step) cosmos_solvable::get_total_steps_passed() const {
	return significant.clock.now.step;
}

augs::stepped_timestamp cosmos_solvable::get_timestamp() const {
	return significant.clock.now;
}

augs::delta cosmos_solvable::get_fixed_delta() const {
	return significant.clock.delta;
}

void cosmos_solvable::set_steps_per_second(const unsigned steps) {
	significant.clock.delta = augs::delta::steps_per_second(steps);
}

unsigned cosmos_solvable::get_steps_per_second() const {
	return get_fixed_delta().in_steps_per_second();
}

std::optional<cosmic_pool_undo_free_input> cosmos_solvable::free_entity(const entity_id id) {
	clear_guid(id);

	return significant.on_pool(id.type_id, [id](auto& p){ return p.free(id); });
}

void cosmos_solvable::undo_last_allocate_entity(const entity_id id) {
	const auto erased_guid = clear_guid(id);

	auto& next_entity_guid = significant.clock.next_entity_guid.value;
	--next_entity_guid;

	ensure_eq(next_entity_guid, erased_guid);

	return significant.on_pool(id.type_id, [id](auto& p){ return p.undo_last_allocate(id); });
}

entity_guid cosmos_solvable::clear_guid(const entity_id cleared) {
	const auto guid = get_guid(cleared);
	const auto erased_num = guid_to_id.erase(guid);

	ensure_eq(erased_num, 1);
	return guid;
}

template <template <class> class Guidized, class source_id_type>
Guidized<entity_guid> cosmos_solvable::guidize(const Guidized<source_id_type>& id_source) const {
	return rewrite_members_and_transform_templated_type_into<entity_guid>(
		id_source,
		[this](auto& guid_member, const auto& id_member) {
			guid_member = get_guid(id_member);
		}
	);
}

template <template <class> class Deguidized, class source_id_type>
Deguidized<entity_id> cosmos_solvable::deguidize(const Deguidized<source_id_type>& guid_source) const {
	return rewrite_members_and_transform_templated_type_into<entity_id>(
		guid_source,
		[this](auto& id_member, const auto& guid_member) {
			if (guid_member != entity_guid()) {
				id_member = guid_to_id.at(guid_member);
			}
		}
	);
}

template basic_inventory_slot_id<entity_guid> cosmos_solvable::guidize<basic_inventory_slot_id, entity_id>(const basic_inventory_slot_id<entity_id>&) const;
template basic_inventory_slot_id<entity_id> cosmos_solvable::deguidize<basic_inventory_slot_id, entity_guid>(const basic_inventory_slot_id<entity_guid>&) const;

template basic_item_slot_transfer_request<entity_guid> cosmos_solvable::guidize<basic_item_slot_transfer_request, entity_id>(const basic_item_slot_transfer_request<entity_id>&) const;
template basic_item_slot_transfer_request<entity_id> cosmos_solvable::deguidize<basic_item_slot_transfer_request, entity_guid>(const basic_item_slot_transfer_request<entity_guid>&) const;

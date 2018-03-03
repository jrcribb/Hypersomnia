#include "augs/templates/string_templates.h"

#include "augs/templates/algorithm_templates.h"
#include "augs/filesystem/file.h"

#include "game/transcendental/cosmos.h"
#include "game/organization/for_each_component_type.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/all_entity_types.h"
#include "game/transcendental/entity_solvable.h"
#include "game/organization/for_each_entity_type.h"

void dump_detailed_sizeof_information(const augs::path_type& where) {
	std::string content;

	std::vector<std::string> lines;

	auto add_sorted_lines = [&]() {
		sort_range(
			lines,
			[](const auto& l1, const auto& l2) {
				return stoul(l1) > stoul(l2);
			}
		);	

		for (const auto& l : lines) {
			content += l;
		}
	};

	content += "Components:\n";

	for_each_component_type([&](auto c){
		using T = decltype(c);

		lines.push_back(typesafe_sprintf("%x == %x\n", sizeof(T), get_type_name_strip_namespace<T>()));
	});

	add_sorted_lines();
	lines.clear();

	content += "\nInvariants:\n";

	for_each_invariant_type([&](auto c){
		using T = decltype(c);

		lines.push_back(typesafe_sprintf("%x == %x\n", sizeof(T), get_type_name_strip_namespace<T>()));
	});

	add_sorted_lines();
	lines.clear();

	content += "\nEntity solvables:\n";

	for_each_entity_solvable_type([&](auto s, auto e){
		using T = decltype(s);

		lines.push_back(typesafe_sprintf("%x == %x\n", sizeof(T), get_type_name_strip_namespace<T>()));
	});

	add_sorted_lines();
	lines.clear();

	content += "\nEntity flavours:\n";

	for_each_entity_type([&](auto e){
		using T = entity_flavour<decltype(e)>;

		lines.push_back(typesafe_sprintf("%x == %x\n", sizeof(T), get_type_name_strip_namespace<T>()));
	});

	add_sorted_lines();
	lines.clear();

	augs::save_as_text(where, content);
}

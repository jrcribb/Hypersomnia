#include "game/messages/queue_destruction.h"
#include "game/stateless_systems/remnant_system.h"

#include "game/transcendental/data_living_one_step.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

void remnant_system::shrink_and_destroy_remnants(const logic_step step) const {
	auto& cosm = step.get_cosmos();

	const auto now = cosm.get_timestamp();
	const auto delta = step.get_delta();

	cosm.for_each_having<components::remnant>(
		[&](const auto subject) {
			const auto& def = subject.template get<invariants::remnant>();
			auto& state = subject.template get<components::remnant>();

			const auto remaining_ms = augs::get_remaining_time_ms(
				def.lifetime_secs * 1000,
				subject.when_born(),
				now,
				delta
			);

			const auto size_mult = remaining_ms / def.start_shrinking_when_remaining_ms;

			if (size_mult <= 0.f) {
				step.post_message(messages::queue_destruction(subject.get_id()));
			}
			else if (size_mult < 1.f) {
				state.last_size_mult = size_mult;
			}
		}
	);
}
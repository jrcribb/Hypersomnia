#pragma once
#include "game/messages/message.h"
#include "game/detail/inventory/item_transfer_result.h"

namespace messages {
	struct performed_transfer_message {
		item_transfer_result result;

		entity_id source_root;
		entity_id target_root;
		entity_id item;
		entity_id target_slot;

		bool is_successful() const {
			return result.is_successful();
		}
	};
}

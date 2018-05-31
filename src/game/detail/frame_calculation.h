#pragma once
#include "game/components/gun_component.h"
#include "game/components/movement_component.h"
#include "game/assets/animation_math.h"

template <class T>
frame_type_t<T>* get_frame(
	const components::gun& gun,
	T& anim, 
	const augs::stepped_timestamp now, 
	const augs::delta dt
) {
	const auto animation_time_ms = (now - gun.when_last_fired).in_milliseconds(dt);
	return calc_current_frame(anim, animation_time_ms);
}

template <class T, class C>
frame_type_t<T>* get_frame(
	const components::gun& gun,
	T& anim, 
	const C& cosm
) {
	return ::get_frame(gun, anim, cosm.get_timestamp(), cosm.get_fixed_delta());
}

template <class T>
struct frame_and_flip {
	frame_type_t<T>& frame;
	const bool flip;
};

template <class T>
frame_and_flip<T> get_frame_and_flip(
	const movement_animation_state& state, 
	T& animation
) {
	const auto frames_n = static_cast<unsigned>(animation.frames.size());

	const auto index = state.index;
	auto i = index;

	if (animation.has_backward_frames && state.backward) {
		i = frames_n - index - 1;
	}

	return { 
		animation.frames[std::min(frames_n - 1, i)], 
		animation.flip_when_cycling && state.flip
	};
}

template <class T>
struct stance_frame_info {
	const frame_type_t<T>* const frame;
	const bool flip;
	const bool is_shooting;

	static auto none() {
		return stance_frame_info<T> { nullptr, false, false };
	}

	static auto carry(const frame_and_flip<T>& f) {
		return stance_frame_info<T> { std::addressof(f.frame), f.flip, false };
	}

	static auto shoot(const frame_type_t<T>& f) {
		return stance_frame_info<T> { std::addressof(f), false, true };
	}

	explicit operator bool() const {
		return frame != nullptr;
	}

	auto get_with_flip() const {
		return frame_and_flip<T>{ *frame, flip };
	}
};

template <class C, class T>
auto calc_stance_info(
	const C& cosm,
	const stance_animations& stance,
	const movement_animation_state& movement, 
	const T& wielded_items
) {
	using result_t = stance_frame_info<const torso_animation>;

	const auto& logicals = cosm.get_logical_assets();

	if (const auto shoot_animation = mapped_or_nullptr(logicals.torso_animations, stance.shoot)) {
		/* Determine whether we want a carrying or a shooting animation */
		if (wielded_items.size() > 0) {
			if (const auto gun = cosm[wielded_items[0]].template find<components::gun>()) {
				if (const auto frame = ::get_frame(*gun, *shoot_animation, cosm)) {
					return result_t::shoot(*frame);
				}
			}
		}
	}

	if (const auto carry_animation = mapped_or_nullptr(logicals.torso_animations, stance.carry)) {
		return result_t::carry(::get_frame_and_flip(movement, *carry_animation));
	}

	return result_t::none();
}
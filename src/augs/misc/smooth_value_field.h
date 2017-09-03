#pragma once
#include "augs/math/vec2.h"
#include "augs/misc/timer.h"

namespace augs {
	template <class T>
	struct smoothing_settings {
		// GEN INTROSPECTOR struct augs::smoothing_settings class T
		T averages_per_sec = static_cast<T>(1.0);
		T average_factor = static_cast<T>(0.5);
		// END GEN INTROSPECTOR
	};

	struct smooth_value_field {
		vec2i discrete_value;
		vec2 value;
		vec2 target_value;

		void tick(const double delta_seconds, const smoothing_settings<double>);
	};
}
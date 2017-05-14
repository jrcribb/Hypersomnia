#include "generated/setting_build_unit_tests.h"

#if BUILD_UNIT_TESTS
#include "augs/misc/enum_boolset.h"
#include "augs/misc/enum_bitset.h"
#include <catch.hpp>

TEST_CASE("CustomContainers EnumBoolsetTest") {
	enum class bb {
		A,
		B,

		COUNT
	};

	{
		augs::enum_boolset<bb> myar( bb::B );

		REQUIRE(myar.test(bb::B));
		REQUIRE(!myar.test(bb::A));
	}

	{
		augs::enum_boolset<bb> myar2{ bb::A, bb::B };

		REQUIRE(myar2.test(bb::A));
		REQUIRE(myar2.test(bb::B));
	}

	{
		augs::enum_boolset<bb> myar3{ bb::B, bb::A };

		REQUIRE(myar3.test(bb::B));
		REQUIRE(myar3.test(bb::A));
	}
}
#endif
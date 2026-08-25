#include <catch2/catch_test_macros.hpp>

#include "move_semantics_module.hpp"

TEST_CASE("MoveSemanticsModule moves data from r1 to r2 then mutates r2") {
    underhood::modules::MoveSemanticsModule module;
    module.reset({{"initial_value", 3, 1, 100}, {"new_value", 9, 1, 100}});

    REQUIRE(module.r1Value() == 3);
    REQUIRE_FALSE(module.r2Value().has_value());
    REQUIRE(module.currentHighlightedLine() == 1);

    REQUIRE(module.step());  // move r1 -> r2
    REQUIRE_FALSE(module.r1Value().has_value());
    REQUIRE(module.r2Value() == 3);
    REQUIRE(module.currentHighlightedLine() == 2);

    REQUIRE(module.step());  // r2.setValue(new_value)
    REQUIRE(module.r2Value() == 9);
    REQUIRE(module.currentHighlightedLine() == 3);

    REQUIRE_FALSE(module.step());  // finished
}

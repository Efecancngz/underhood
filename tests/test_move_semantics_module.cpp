#include <catch2/catch_test_macros.hpp>

#include "move_semantics_module.hpp"

TEST_CASE("MoveSemanticsModule moves data from r1 to r2 then mutates r2") {
    underhood::modules::MoveSemanticsModule module;
    // Deliberately not 3/9 (the module's own hardcoded defaults, see
    // parameters() below) so this test can only pass if reset() actually
    // reads the passed-in params rather than always falling back to them.
    module.reset({{"initial_value", 7, 1, 100}, {"new_value", 15, 1, 100}});

    REQUIRE(module.r1Value() == 7);
    REQUIRE_FALSE(module.r2Value().has_value());
    REQUIRE(module.currentHighlightedLine() == 1);

    REQUIRE(module.step());  // move r1 -> r2
    REQUIRE_FALSE(module.r1Value().has_value());
    REQUIRE(module.r2Value() == 7);
    REQUIRE(module.currentHighlightedLine() == 2);

    REQUIRE(module.step());  // r2.setValue(new_value)
    REQUIRE(module.r2Value() == 15);
    REQUIRE(module.currentHighlightedLine() == 3);

    REQUIRE_FALSE(module.step());  // finished
}

TEST_CASE("MoveSemanticsModule exposes its parameters with defaults 3 and 9") {
    underhood::modules::MoveSemanticsModule module;
    auto params = module.parameters();
    REQUIRE(params.size() == 2);
    REQUIRE(params[0].name == "initial_value");
    REQUIRE(params[0].value == 3);
    REQUIRE(params[1].name == "new_value");
    REQUIRE(params[1].value == 9);
}

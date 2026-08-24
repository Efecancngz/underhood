#include <catch2/catch_test_macros.hpp>

#include "unique_ptr_module.hpp"

TEST_CASE("UniquePtrModule walks through move then reset") {
    underhood::modules::UniquePtrModule module;
    module.reset({{"initial_value", 7, 1, 100}});

    REQUIRE(module.ownedByA());
    REQUIRE_FALSE(module.ownedByB());
    REQUIRE(module.currentValue() == 7);
    REQUIRE(module.currentHighlightedLine() == 1);

    REQUIRE(module.step());  // move a -> b
    REQUIRE_FALSE(module.ownedByA());
    REQUIRE(module.ownedByB());
    REQUIRE(module.currentValue() == 7);
    REQUIRE(module.currentHighlightedLine() == 2);

    REQUIRE(module.step());  // b.reset()
    REQUIRE_FALSE(module.ownedByA());
    REQUIRE_FALSE(module.ownedByB());
    REQUIRE(module.currentHighlightedLine() == 3);

    REQUIRE_FALSE(module.step());  // finished, no more steps
}

TEST_CASE("UniquePtrModule exposes its parameter with default 42") {
    underhood::modules::UniquePtrModule module;
    auto params = module.parameters();
    REQUIRE(params.size() == 1);
    REQUIRE(params[0].name == "initial_value");
    REQUIRE(params[0].value == 42);
}

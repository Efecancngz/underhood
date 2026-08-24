#include <catch2/catch_test_macros.hpp>

#include "shared_ptr_module.hpp"

TEST_CASE("SharedPtrModule tracks ref count through share and release") {
    underhood::modules::SharedPtrModule module;
    module.reset({{"initial_value", 5, 1, 100}});

    REQUIRE(module.refCount() == 1);
    REQUIRE(module.aAlive());
    REQUIRE_FALSE(module.bAlive());
    REQUIRE(module.currentHighlightedLine() == 1);

    REQUIRE(module.step());  // b = a
    REQUIRE(module.refCount() == 2);
    REQUIRE(module.bAlive());
    REQUIRE(module.currentHighlightedLine() == 2);

    REQUIRE(module.step());  // a.reset()
    REQUIRE(module.refCount() == 1);
    REQUIRE_FALSE(module.aAlive());
    REQUIRE(module.currentHighlightedLine() == 3);

    REQUIRE(module.step());  // b.reset()
    REQUIRE(module.refCount() == 0);
    REQUIRE_FALSE(module.bAlive());
    REQUIRE(module.currentHighlightedLine() == 4);

    REQUIRE_FALSE(module.step());  // finished
}

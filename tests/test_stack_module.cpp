#include <catch2/catch_test_macros.hpp>

#include "stack_module.hpp"

TEST_CASE("StackModule starts empty") {
    underhood::modules::StackModule module;
    REQUIRE(module.size() == 0);
    REQUIRE_FALSE(module.canPerform("Pop"));
    REQUIRE(module.canPerform("Push"));
}

TEST_CASE("StackModule Push adds to the top and Pop removes it in LIFO order") {
    underhood::modules::StackModule module;
    module.performOperation("Push", 1);
    module.performOperation("Push", 2);
    module.performOperation("Push", 3);

    REQUIRE(module.size() == 3);
    REQUIRE(module.topValue() == 3);

    module.performOperation("Pop", 0);
    REQUIRE(module.size() == 2);
    REQUIRE(module.topValue() == 2);
}

TEST_CASE("StackModule records operation history") {
    underhood::modules::StackModule module;
    module.performOperation("Push", 5);
    module.performOperation("Pop", 0);

    auto history = module.history();
    REQUIRE(history.size() == 2);
    REQUIRE(history[0] == "Push 5");
    REQUIRE(history[1] == "Pop -> 5");
}

TEST_CASE("StackModule Pop on an empty stack is a no-op") {
    underhood::modules::StackModule module;
    module.performOperation("Pop", 0);
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("StackModule refuses Push once at capacity") {
    underhood::modules::StackModule module;
    for (int i = 0; i < 8; ++i) {
        module.performOperation("Push", i);
    }
    REQUIRE(module.size() == 8);
    REQUIRE_FALSE(module.canPerform("Push"));

    module.performOperation("Push", 99);  // no-op, already full
    REQUIRE(module.size() == 8);
    REQUIRE(module.topValue() == 7);
}

TEST_CASE("StackModule clear empties the stack and history") {
    underhood::modules::StackModule module;
    module.performOperation("Push", 1);
    module.performOperation("Push", 2);
    module.clear();
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("StackModule caps history at 10 entries, dropping the oldest") {
    underhood::modules::StackModule module;
    // 6 push+pop cycles = 12 history entries, stack size never exceeds 1
    // (capacity is a separate concern from history capping).
    for (int i = 0; i < 6; ++i) {
        module.performOperation("Push", i);
        module.performOperation("Pop", 0);
    }
    auto history = module.history();
    REQUIRE(history.size() == 10);
    REQUIRE(history.front() == "Push 1");  // "Push 0" and "Pop -> 0" fell off
    REQUIRE(history.back() == "Pop -> 5");
}

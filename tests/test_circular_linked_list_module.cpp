#include <catch2/catch_test_macros.hpp>

#include "circular_linked_list_module.hpp"

TEST_CASE("CircularLinkedListModule starts empty") {
    underhood::modules::CircularLinkedListModule module;
    REQUIRE(module.size() == 0);
    REQUIRE_FALSE(module.canPerform("Delete Front"));
    REQUIRE(module.canPerform("Insert"));
}

TEST_CASE("CircularLinkedListModule Insert always appends to the back") {
    underhood::modules::CircularLinkedListModule module;
    module.performOperation("Insert", 1);
    module.performOperation("Insert", 2);
    module.performOperation("Insert", 3);

    REQUIRE(module.size() == 3);
    REQUIRE(module.frontValue() == 1);
}

TEST_CASE("CircularLinkedListModule Delete Front removes the head") {
    underhood::modules::CircularLinkedListModule module;
    module.performOperation("Insert", 1);
    module.performOperation("Insert", 2);
    module.performOperation("Delete Front", 0);

    REQUIRE(module.size() == 1);
    REQUIRE(module.frontValue() == 2);
}

TEST_CASE("CircularLinkedListModule records operation history") {
    underhood::modules::CircularLinkedListModule module;
    module.performOperation("Insert", 5);
    module.performOperation("Delete Front", 0);

    auto history = module.history();
    REQUIRE(history.size() == 2);
    REQUIRE(history[0] == "Insert 5");
    REQUIRE(history[1] == "Delete Front -> 5");
}

TEST_CASE("CircularLinkedListModule Delete Front on an empty list is a no-op") {
    underhood::modules::CircularLinkedListModule module;
    module.performOperation("Delete Front", 0);
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("CircularLinkedListModule refuses Insert once at capacity") {
    underhood::modules::CircularLinkedListModule module;
    for (int i = 0; i < 8; ++i) {
        module.performOperation("Insert", i);
    }
    REQUIRE(module.size() == 8);
    REQUIRE_FALSE(module.canPerform("Insert"));
}

TEST_CASE("CircularLinkedListModule clear empties the list and history") {
    underhood::modules::CircularLinkedListModule module;
    module.performOperation("Insert", 1);
    module.clear();
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("CircularLinkedListModule caps history at 10 entries, dropping the oldest") {
    underhood::modules::CircularLinkedListModule module;
    // 6 insert+delete cycles = 12 history entries, size never exceeds 1.
    for (int i = 0; i < 6; ++i) {
        module.performOperation("Insert", i);
        module.performOperation("Delete Front", 0);
    }
    auto history = module.history();
    REQUIRE(history.size() == 10);
    REQUIRE(history.front() == "Insert 1");  // first cycle's 2 entries fell off
    REQUIRE(history.back() == "Delete Front -> 5");
}

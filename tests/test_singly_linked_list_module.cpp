#include <catch2/catch_test_macros.hpp>

#include "singly_linked_list_module.hpp"

TEST_CASE("SinglyLinkedListModule starts empty") {
    underhood::modules::SinglyLinkedListModule module;
    REQUIRE(module.size() == 0);
    REQUIRE_FALSE(module.canPerform("Delete Front"));
    REQUIRE(module.canPerform("Insert Front"));
    REQUIRE(module.canPerform("Insert Back"));
}

TEST_CASE("SinglyLinkedListModule Insert Front adds to the head") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Insert Front", 1);
    module.performOperation("Insert Front", 2);

    REQUIRE(module.size() == 2);
    REQUIRE(module.frontValue() == 2);
}

TEST_CASE("SinglyLinkedListModule Insert Back adds to the tail") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Insert Back", 1);
    module.performOperation("Insert Back", 2);

    REQUIRE(module.size() == 2);
    REQUIRE(module.frontValue() == 1);
}

TEST_CASE("SinglyLinkedListModule Delete Front removes the head") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Insert Back", 1);
    module.performOperation("Insert Back", 2);
    module.performOperation("Delete Front", 0);

    REQUIRE(module.size() == 1);
    REQUIRE(module.frontValue() == 2);
}

TEST_CASE("SinglyLinkedListModule records operation history") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Insert Front", 5);
    module.performOperation("Delete Front", 0);

    auto history = module.history();
    REQUIRE(history.size() == 2);
    REQUIRE(history[0] == "Insert Front 5");
    REQUIRE(history[1] == "Delete Front -> 5");
}

TEST_CASE("SinglyLinkedListModule Delete Front on an empty list is a no-op") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Delete Front", 0);
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("SinglyLinkedListModule refuses inserts once at capacity") {
    underhood::modules::SinglyLinkedListModule module;
    for (int i = 0; i < 8; ++i) {
        module.performOperation("Insert Back", i);
    }
    REQUIRE(module.size() == 8);
    REQUIRE_FALSE(module.canPerform("Insert Front"));
    REQUIRE_FALSE(module.canPerform("Insert Back"));
}

TEST_CASE("SinglyLinkedListModule clear empties the list and history") {
    underhood::modules::SinglyLinkedListModule module;
    module.performOperation("Insert Back", 1);
    module.clear();
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("SinglyLinkedListModule caps history at 10 entries, dropping the oldest") {
    underhood::modules::SinglyLinkedListModule module;
    // 6 insert+delete cycles = 12 history entries, size never exceeds 1.
    for (int i = 0; i < 6; ++i) {
        module.performOperation("Insert Front", i);
        module.performOperation("Delete Front", 0);
    }
    auto history = module.history();
    REQUIRE(history.size() == 10);
    REQUIRE(history.front() == "Insert Front 1");  // first cycle's 2 entries fell off
    REQUIRE(history.back() == "Delete Front -> 5");
}

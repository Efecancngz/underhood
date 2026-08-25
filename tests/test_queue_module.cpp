#include <catch2/catch_test_macros.hpp>

#include "queue_module.hpp"

TEST_CASE("QueueModule starts empty") {
    underhood::modules::QueueModule module;
    REQUIRE(module.size() == 0);
    REQUIRE_FALSE(module.canPerform("Dequeue"));
    REQUIRE(module.canPerform("Enqueue"));
}

TEST_CASE("QueueModule Enqueue adds to the back and Dequeue removes from the front (FIFO)") {
    underhood::modules::QueueModule module;
    module.performOperation("Enqueue", 1);
    module.performOperation("Enqueue", 2);
    module.performOperation("Enqueue", 3);

    REQUIRE(module.size() == 3);
    REQUIRE(module.frontValue() == 1);

    module.performOperation("Dequeue", 0);
    REQUIRE(module.size() == 2);
    REQUIRE(module.frontValue() == 2);
}

TEST_CASE("QueueModule records operation history") {
    underhood::modules::QueueModule module;
    module.performOperation("Enqueue", 5);
    module.performOperation("Dequeue", 0);

    auto history = module.history();
    REQUIRE(history.size() == 2);
    REQUIRE(history[0] == "Enqueue 5");
    REQUIRE(history[1] == "Dequeue -> 5");
}

TEST_CASE("QueueModule Dequeue on an empty queue is a no-op") {
    underhood::modules::QueueModule module;
    module.performOperation("Dequeue", 0);
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("QueueModule refuses Enqueue once at capacity") {
    underhood::modules::QueueModule module;
    for (int i = 0; i < 8; ++i) {
        module.performOperation("Enqueue", i);
    }
    REQUIRE(module.size() == 8);
    REQUIRE_FALSE(module.canPerform("Enqueue"));

    module.performOperation("Enqueue", 99);  // no-op, already full
    REQUIRE(module.size() == 8);
    REQUIRE(module.frontValue() == 0);
}

TEST_CASE("QueueModule clear empties the queue and history") {
    underhood::modules::QueueModule module;
    module.performOperation("Enqueue", 1);
    module.performOperation("Enqueue", 2);
    module.clear();
    REQUIRE(module.size() == 0);
    REQUIRE(module.history().empty());
}

TEST_CASE("QueueModule caps history at 10 entries, dropping the oldest") {
    underhood::modules::QueueModule module;
    // 6 enqueue+dequeue cycles = 12 history entries, size never exceeds 1.
    for (int i = 0; i < 6; ++i) {
        module.performOperation("Enqueue", i);
        module.performOperation("Dequeue", 0);
    }
    auto history = module.history();
    REQUIRE(history.size() == 10);
    REQUIRE(history.front() == "Enqueue 1");  // "Enqueue 0" and "Dequeue -> 0" fell off
    REQUIRE(history.back() == "Dequeue -> 5");
}

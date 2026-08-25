#include <catch2/catch_test_macros.hpp>

#include "underhood/animated_list.hpp"

TEST_CASE("AnimatedList insertAt appends a new animating entry") {
    underhood::AnimatedList<int> list;
    REQUIRE(list.empty());

    list.insertAt(0, 42);
    REQUIRE(list.size() == 1);
    REQUIRE(list.entries()[0].value == 42);
    REQUIRE(list.entries()[0].insertAnim.isAnimating());
}

TEST_CASE("AnimatedList insertAt respects index for front/back insertion") {
    underhood::AnimatedList<int> list;
    list.insertAt(0, 1);
    list.insertAt(1, 2);  // back
    list.insertAt(0, 0);  // front

    REQUIRE(list.size() == 3);
    REQUIRE(list.entries()[0].value == 0);
    REQUIRE(list.entries()[1].value == 1);
    REQUIRE(list.entries()[2].value == 2);
}

TEST_CASE("AnimatedList removeAt moves the entry into removingEntry until its animation finishes") {
    underhood::AnimatedList<int> list;
    list.insertAt(0, 7);
    REQUIRE(list.removingEntry() == nullptr);

    list.removeAt(0);
    REQUIRE(list.empty());
    REQUIRE(list.removingEntry() != nullptr);
    REQUIRE(list.removingEntry()->value == 7);

    list.update(1.0f);  // well past the 0.30s animation duration
    REQUIRE(list.removingEntry() == nullptr);
}

TEST_CASE("AnimatedList full() reports capacity correctly") {
    underhood::AnimatedList<int> list;
    REQUIRE_FALSE(list.full(2));
    list.insertAt(0, 1);
    REQUIRE_FALSE(list.full(2));
    list.insertAt(1, 2);
    REQUIRE(list.full(2));
}

TEST_CASE("AnimatedList removeAt while a previous removal is still animating replaces it immediately") {
    underhood::AnimatedList<int> list;
    list.insertAt(0, 1);
    list.insertAt(1, 2);

    list.removeAt(1);  // removingEntry = 2, mid-animation
    REQUIRE(list.removingEntry()->value == 2);

    list.removeAt(0);  // removingEntry_ overwritten per spec policy
    REQUIRE(list.removingEntry()->value == 1);
}

#include <catch2/catch_test_macros.hpp>

#include "underhood/animation.hpp"

TEST_CASE("Animator reports finished progress before start() is called") {
    underhood::Animator animator;
    REQUIRE(animator.progress() == 1.0f);
    REQUIRE_FALSE(animator.isAnimating());
}

TEST_CASE("Animator progresses from 0 toward 1 after start()") {
    underhood::Animator animator;
    animator.start();
    REQUIRE(animator.isAnimating());
    REQUIRE(animator.progress() == 0.0f);

    animator.update(0.15f);  // halfway through the 0.30s duration
    REQUIRE(animator.isAnimating());
    REQUIRE(animator.progress() > 0.0f);
    REQUIRE(animator.progress() < 1.0f);

    animator.update(0.20f);  // pushes elapsed past 0.30s total
    REQUIRE(animator.progress() == 1.0f);
    REQUIRE_FALSE(animator.isAnimating());
}

TEST_CASE("Animator start() restarts an already-finished animation") {
    underhood::Animator animator;
    animator.start();
    animator.update(1.0f);  // finishes it
    REQUIRE_FALSE(animator.isAnimating());

    animator.start();
    REQUIRE(animator.isAnimating());
    REQUIRE(animator.progress() == 0.0f);
}

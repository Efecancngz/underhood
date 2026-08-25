#include "underhood/animation.hpp"

#include <algorithm>

namespace underhood {

void Animator::start() {
    elapsed_ = 0.0f;
    active_ = true;
}

void Animator::update(float deltaTime) {
    if (!active_) return;
    elapsed_ += deltaTime;
    if (elapsed_ >= kDurationSeconds) {
        elapsed_ = kDurationSeconds;
        active_ = false;
    }
}

float Animator::progress() const {
    float linear = elapsed_ / kDurationSeconds;
    linear = std::min(1.0f, std::max(0.0f, linear));
    // Ease-out (quadratic): fast start, settles smoothly toward 1.
    return 1.0f - (1.0f - linear) * (1.0f - linear);
}

bool Animator::isAnimating() const {
    return active_;
}

}  // namespace underhood

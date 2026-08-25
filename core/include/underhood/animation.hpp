#pragma once

namespace underhood {

// Simple ease-out time-based progress tracker used by IOperationalModule
// implementations to animate insert/remove transitions. Owns no rendering
// state itself -- callers read progress() each frame and use it to
// interpolate their own box positions/opacity.
class Animator {
public:
    void start();
    void update(float deltaTime);
    float progress() const;
    bool isAnimating() const;

private:
    static constexpr float kDurationSeconds = 0.30f;
    float elapsed_ = kDurationSeconds;  // starts "finished" until start() is called
    bool active_ = false;
};

}  // namespace underhood

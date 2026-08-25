#include "move_semantics_module.hpp"

#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string MoveSemanticsModule::name() const {
    return "move_semantics";
}

std::string MoveSemanticsModule::codeSnippet() const {
    return "Resource r1(initial_value);\n"
           "Resource r2(std::move(r1));\n"
           "r2.setValue(new_value);";
}

std::vector<underhood::Parameter> MoveSemanticsModule::parameters() const {
    return {{"initial_value", 3, 1, 100}, {"new_value", 9, 1, 100}};
}

void MoveSemanticsModule::reset(const std::vector<underhood::Parameter>& params) {
    int initialValue = params.size() > 0 ? params[0].value : 3;
    newValue_ = params.size() > 1 ? params[1].value : 9;
    phase_ = 0;
    highlightedLine_ = 1;
    r1Value_ = initialValue;
    r2Value_ = std::nullopt;
    r1JustChanged_ = false;
    r2JustChanged_ = false;
}

bool MoveSemanticsModule::step() {
    r1JustChanged_ = false;
    r2JustChanged_ = false;
    if (phase_ == 0) {
        r2Value_ = r1Value_;
        r1Value_ = std::nullopt;
        r1JustChanged_ = true;
        r2JustChanged_ = true;
        phase_ = 1;
        highlightedLine_ = 2;
        return true;
    }
    if (phase_ == 1) {
        r2Value_ = newValue_;
        r2JustChanged_ = true;
        phase_ = 2;
        highlightedLine_ = 3;
        return true;
    }
    return false;
}

int MoveSemanticsModule::currentHighlightedLine() const {
    return highlightedLine_;
}

void MoveSemanticsModule::render(underhood::Canvas& canvas) const {
    auto r1State = r1JustChanged_
                       ? underhood::BoxState::JustChanged
                       : (r1Value_ ? underhood::BoxState::Owned : underhood::BoxState::Empty);
    canvas.drawBox(50, 50, 120, 60, r1Value_ ? ("r1: " + std::to_string(*r1Value_)) : "r1: (empty)",
                   r1State);

    auto r2State = r2JustChanged_
                       ? underhood::BoxState::JustChanged
                       : (r2Value_ ? underhood::BoxState::Owned : underhood::BoxState::Empty);
    canvas.drawBox(250, 50, 120, 60, r2Value_ ? ("r2: " + std::to_string(*r2Value_)) : "r2: (empty)",
                   r2State);

    // The resource only ever moves one way, r1 -> r2 -- draw the arrow once
    // it has (r2 holds a value), flashed the same JustChanged color as the
    // boxes on the step it happens.
    if (r2Value_.has_value()) {
        auto arrowState = (r1JustChanged_ || r2JustChanged_) ? underhood::BoxState::JustChanged
                                                               : underhood::BoxState::Owned;
        canvas.drawArrow(170, 80, 250, 80, arrowState);
    }
}

std::optional<int> MoveSemanticsModule::r1Value() const {
    return r1Value_;
}

std::optional<int> MoveSemanticsModule::r2Value() const {
    return r2Value_;
}

}  // namespace underhood::modules

namespace {
struct MoveSemanticsRegistrar {
    MoveSemanticsRegistrar() {
        underhood::ModuleRegistry::instance().registerModule("move_semantics", []() {
            return std::make_unique<underhood::modules::MoveSemanticsModule>();
        });
    }
};
const MoveSemanticsRegistrar moveSemanticsRegistrar;
}  // namespace

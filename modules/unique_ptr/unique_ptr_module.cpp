#include "unique_ptr_module.hpp"

#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string UniquePtrModule::name() const {
    return "unique_ptr";
}

std::string UniquePtrModule::codeSnippet() const {
    return "std::unique_ptr<int> a(new int(initial_value));\n"
           "std::unique_ptr<int> b = std::move(a);\n"
           "b.reset();";
}

std::vector<underhood::Parameter> UniquePtrModule::parameters() const {
    return {{"initial_value", 42, 1, 100}};
}

void UniquePtrModule::reset(const std::vector<underhood::Parameter>& params) {
    int initialValue = params.empty() ? 42 : params[0].value;
    phase_ = 0;
    highlightedLine_ = 1;
    aValue_ = initialValue;
    bValue_ = std::nullopt;
    aJustChanged_ = false;
    bJustChanged_ = false;
}

bool UniquePtrModule::step() {
    aJustChanged_ = false;
    bJustChanged_ = false;
    if (phase_ == 0) {
        bValue_ = aValue_;
        aValue_ = std::nullopt;
        aJustChanged_ = true;
        bJustChanged_ = true;
        phase_ = 1;
        highlightedLine_ = 2;
        return true;
    }
    if (phase_ == 1) {
        bValue_ = std::nullopt;
        bJustChanged_ = true;
        phase_ = 2;
        highlightedLine_ = 3;
        return true;
    }
    return false;
}

int UniquePtrModule::currentHighlightedLine() const {
    return highlightedLine_;
}

void UniquePtrModule::render(underhood::Canvas& canvas) const {
    auto stateFor = [](bool hasValue, bool justChanged) {
        if (justChanged) return underhood::BoxState::JustChanged;
        return hasValue ? underhood::BoxState::Owned : underhood::BoxState::Empty;
    };
    canvas.drawBox(50, 50, 120, 60, aValue_ ? ("a: " + std::to_string(*aValue_)) : "a: (empty)",
                   stateFor(aValue_.has_value(), aJustChanged_));
    canvas.drawBox(250, 50, 120, 60, bValue_ ? ("b: " + std::to_string(*bValue_)) : "b: (empty)",
                   stateFor(bValue_.has_value(), bJustChanged_));
}

bool UniquePtrModule::ownedByA() const {
    return aValue_.has_value();
}

bool UniquePtrModule::ownedByB() const {
    return bValue_.has_value();
}

std::optional<int> UniquePtrModule::currentValue() const {
    return aValue_.has_value() ? aValue_ : bValue_;
}

}  // namespace underhood::modules

namespace {
struct UniquePtrRegistrar {
    UniquePtrRegistrar() {
        underhood::ModuleRegistry::instance().registerModule("unique_ptr", []() {
            return std::make_unique<underhood::modules::UniquePtrModule>();
        });
    }
};
const UniquePtrRegistrar uniquePtrRegistrar;
}  // namespace

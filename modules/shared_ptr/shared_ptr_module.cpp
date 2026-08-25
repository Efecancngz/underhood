#include "shared_ptr_module.hpp"

#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string SharedPtrModule::name() const {
    return "shared_ptr";
}

std::string SharedPtrModule::codeSnippet() const {
    return "std::shared_ptr<int> a = std::make_shared<int>(initial_value);\n"
           "std::shared_ptr<int> b = a;\n"
           "a.reset();\n"
           "b.reset();";
}

std::vector<underhood::Parameter> SharedPtrModule::parameters() const {
    return {{"initial_value", 10, 1, 100}};
}

void SharedPtrModule::reset(const std::vector<underhood::Parameter>& params) {
    value_ = params.empty() ? 10 : params[0].value;
    phase_ = 0;
    highlightedLine_ = 1;
    aAlive_ = true;
    bAlive_ = false;
    refCount_ = 1;
    refCountJustChanged_ = false;
    aJustChanged_ = false;
    bJustChanged_ = false;
}

bool SharedPtrModule::step() {
    refCountJustChanged_ = false;
    aJustChanged_ = false;
    bJustChanged_ = false;
    if (phase_ == 0) {
        bAlive_ = true;
        refCount_ = 2;
        bJustChanged_ = true;
        refCountJustChanged_ = true;
        phase_ = 1;
        highlightedLine_ = 2;
        return true;
    }
    if (phase_ == 1) {
        aAlive_ = false;
        refCount_ = 1;
        aJustChanged_ = true;
        refCountJustChanged_ = true;
        phase_ = 2;
        highlightedLine_ = 3;
        return true;
    }
    if (phase_ == 2) {
        bAlive_ = false;
        refCount_ = 0;
        bJustChanged_ = true;
        refCountJustChanged_ = true;
        phase_ = 3;
        highlightedLine_ = 4;
        return true;
    }
    return false;
}

int SharedPtrModule::currentHighlightedLine() const {
    return highlightedLine_;
}

void SharedPtrModule::render(underhood::Canvas& canvas) const {
    auto stateFor = [](bool alive, bool justChanged) {
        if (justChanged) return underhood::BoxState::JustChanged;
        return alive ? underhood::BoxState::Owned : underhood::BoxState::Empty;
    };

    auto refState = stateFor(refCount_ > 0, refCountJustChanged_);
    canvas.drawBox(50, 50, 160, 60, "refcount: " + std::to_string(refCount_), refState);

    auto aState = stateFor(aAlive_, aJustChanged_);
    canvas.drawBox(50, 150, 120, 60, aAlive_ ? ("a -> " + std::to_string(value_)) : "a: (empty)",
                   aState);

    auto bState = stateFor(bAlive_, bJustChanged_);
    canvas.drawBox(250, 150, 120, 60, bAlive_ ? ("b -> " + std::to_string(value_)) : "b: (empty)",
                   bState);

    // a and b both hold a reference into the same refcount/value while
    // alive -- draw both arrows pointing up into the shared box instead of
    // implying either one "owns" the other.
    if (aAlive_) {
        canvas.drawArrow(110, 150, 110, 110, aState);
    }
    if (bAlive_) {
        canvas.drawArrow(310, 150, 170, 110, bState);
    }
}

int SharedPtrModule::refCount() const {
    return refCount_;
}

bool SharedPtrModule::aAlive() const {
    return aAlive_;
}

bool SharedPtrModule::bAlive() const {
    return bAlive_;
}

}  // namespace underhood::modules

namespace {
struct SharedPtrRegistrar {
    SharedPtrRegistrar() {
        underhood::ModuleRegistry::instance().registerModule(
            "shared_ptr", []() { return std::make_unique<underhood::modules::SharedPtrModule>(); },
            "Smart Pointers");
    }
};
const SharedPtrRegistrar sharedPtrRegistrar;
}  // namespace

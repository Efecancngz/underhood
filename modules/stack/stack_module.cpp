#include "stack_module.hpp"

#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string StackModule::name() const {
    return "stack";
}

std::string StackModule::codeSnippet() const {
    return "";
}

std::vector<underhood::IOperationalModule::Operation> StackModule::operations() const {
    return {{"Push", true}, {"Pop", false}};
}

bool StackModule::canPerform(const std::string& operationLabel) const {
    if (operationLabel == "Push") {
        return !entries_.full(kMaxSize);
    }
    if (operationLabel == "Pop") {
        return !entries_.empty();
    }
    return false;
}

namespace {
constexpr std::size_t kMaxHistoryEntries = 10;

void appendHistory(std::vector<std::string>& history, std::string entry) {
    history.push_back(std::move(entry));
    if (history.size() > kMaxHistoryEntries) {
        history.erase(history.begin());
    }
}
}  // namespace

void StackModule::performOperation(const std::string& operationLabel, int value) {
    if (operationLabel == "Push" && !entries_.full(kMaxSize)) {
        entries_.insertAt(entries_.size(), value);
        appendHistory(history_, "Push " + std::to_string(value));
    } else if (operationLabel == "Pop" && !entries_.empty()) {
        int poppedValue = entries_.entries().back().value;
        entries_.removeAt(entries_.size() - 1);
        appendHistory(history_, "Pop -> " + std::to_string(poppedValue));
    }
}

std::vector<std::string> StackModule::history() const {
    return history_;
}

void StackModule::update(float deltaTime) {
    entries_.update(deltaTime);
}

void StackModule::clear() {
    entries_ = underhood::AnimatedList<int>();
    history_.clear();
}

std::size_t StackModule::size() const {
    return entries_.size();
}

int StackModule::topValue() const {
    return entries_.entries().back().value;
}

void StackModule::render(underhood::Canvas& canvas) const {
    constexpr int kBoxWidth = 100;
    constexpr int kBoxHeight = 36;
    constexpr int kBoxSpacing = 6;
    constexpr int kBaseX = 170;
    constexpr int kBaseY = 230;  // bottom of the stack; grows upward

    auto drawEntry = [&](const underhood::AnimatedList<int>::Entry& entry, std::size_t stackIndex,
                          bool isTop) {
        float progress = entry.insertAnim.progress();
        int y = kBaseY - static_cast<int>(stackIndex) * (kBoxHeight + kBoxSpacing);
        // Slide in from above while inserting/removing.
        int slideOffset = static_cast<int>((1.0f - progress) * 40.0f);
        underhood::BoxState state = entry.insertAnim.isAnimating() ? underhood::BoxState::JustChanged
                                     : isTop                        ? underhood::BoxState::Owned
                                                                     : underhood::BoxState::Empty;
        canvas.drawBox(kBaseX, y - slideOffset, kBoxWidth, kBoxHeight, std::to_string(entry.value),
                        state);
    };

    const auto& entries = entries_.entries();
    for (std::size_t i = 0; i < entries.size(); ++i) {
        drawEntry(entries[i], i, i + 1 == entries.size());
    }
    if (const auto* removing = entries_.removingEntry()) {
        drawEntry(*removing, entries.size(), false);
    }
}

}  // namespace underhood::modules

namespace {
struct StackRegistrar {
    StackRegistrar() {
        underhood::ModuleRegistry::instance().registerModule(
            "stack", []() { return std::make_unique<underhood::modules::StackModule>(); },
            "Data Structures");
    }
};
const StackRegistrar stackRegistrar;
}  // namespace

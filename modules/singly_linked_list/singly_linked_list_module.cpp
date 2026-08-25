#include "singly_linked_list_module.hpp"

#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string SinglyLinkedListModule::name() const {
    return "linked_list_singly";
}

std::string SinglyLinkedListModule::codeSnippet() const {
    return "";
}

std::vector<underhood::IOperationalModule::Operation> SinglyLinkedListModule::operations() const {
    return {{"Insert Front", true}, {"Insert Back", true}, {"Delete Front", false}};
}

bool SinglyLinkedListModule::canPerform(const std::string& operationLabel) const {
    if (operationLabel == "Insert Front" || operationLabel == "Insert Back") {
        return !entries_.full(kMaxSize);
    }
    if (operationLabel == "Delete Front") {
        return !entries_.empty();
    }
    return false;
}

void SinglyLinkedListModule::performOperation(const std::string& operationLabel, int value) {
    if (operationLabel == "Insert Front" && !entries_.full(kMaxSize)) {
        entries_.insertAt(0, value);
        history_.append("Insert Front " + std::to_string(value));
    } else if (operationLabel == "Insert Back" && !entries_.full(kMaxSize)) {
        entries_.insertAt(entries_.size(), value);
        history_.append("Insert Back " + std::to_string(value));
    } else if (operationLabel == "Delete Front" && !entries_.empty()) {
        int frontVal = entries_.entries().front().value;
        entries_.removeAt(0);
        history_.append("Delete Front -> " + std::to_string(frontVal));
    }
}

std::vector<std::string> SinglyLinkedListModule::history() const {
    return history_.entries();
}

void SinglyLinkedListModule::update(float deltaTime) {
    entries_.update(deltaTime);
}

void SinglyLinkedListModule::clear() {
    entries_ = underhood::AnimatedList<int>();
    history_.clear();
}

std::size_t SinglyLinkedListModule::size() const {
    return entries_.size();
}

int SinglyLinkedListModule::frontValue() const {
    return entries_.entries().front().value;
}

void SinglyLinkedListModule::render(underhood::Canvas& canvas) const {
    constexpr int kAvailableWidth = 400;
    constexpr int kBaseX = 10;
    constexpr int kBoxHeight = 50;
    constexpr int kBaseY = 100;

    const auto& entries = entries_.entries();
    std::size_t slots = entries.size() + 2;  // +1 null terminator, +1 breathing room
    int cellWidth = kAvailableWidth / static_cast<int>(slots);
    int boxWidth = cellWidth > 20 ? cellWidth - 15 : cellWidth;  // leaves room for the arrow

    auto drawEntry = [&](const underhood::AnimatedList<int>::Entry& entry, std::size_t visualIndex,
                          bool isRemoving) {
        // A removing entry shares insertAnim with a fresh insert (see
        // AnimatedList::removeAt), so its progress() also runs 0->1 --
        // inverting it here makes the departing box start fully "arrived"
        // and animate OUT, instead of animating back IN on top of whatever
        // now occupies its old slot (visual index 0, the new head).
        float progress = isRemoving ? 1.0f - entry.insertAnim.progress() : entry.insertAnim.progress();
        int x = kBaseX + static_cast<int>(visualIndex) * cellWidth;
        int dropOffset = static_cast<int>((1.0f - progress) * 30.0f);
        underhood::BoxState state =
            entry.insertAnim.isAnimating() ? underhood::BoxState::JustChanged : underhood::BoxState::Owned;
        canvas.drawBox(x, kBaseY - dropOffset, boxWidth, kBoxHeight, std::to_string(entry.value), state,
                       progress);
    };

    for (std::size_t i = 0; i < entries.size(); ++i) {
        drawEntry(entries[i], i, /*isRemoving=*/false);
        int arrowStartX = kBaseX + static_cast<int>(i) * cellWidth + boxWidth;
        int arrowY = kBaseY + kBoxHeight / 2;
        canvas.drawArrow(arrowStartX, arrowY, kBaseX + static_cast<int>(i + 1) * cellWidth, arrowY,
                          underhood::BoxState::Owned);
    }
    if (const auto* removing = entries_.removingEntry()) {
        drawEntry(*removing, 0, /*isRemoving=*/true);
    }

    // "null" terminator after the last node.
    int nullX = kBaseX + static_cast<int>(entries.size()) * cellWidth;
    canvas.drawBox(nullX, kBaseY, boxWidth, kBoxHeight, "null", underhood::BoxState::Empty);
}

}  // namespace underhood::modules

namespace {
struct SinglyLinkedListRegistrar {
    SinglyLinkedListRegistrar() {
        underhood::ModuleRegistry::instance().registerModule(
            "linked_list_singly",
            []() { return std::make_unique<underhood::modules::SinglyLinkedListModule>(); },
            "Data Structures");
    }
};
const SinglyLinkedListRegistrar singlyLinkedListRegistrar;
}  // namespace

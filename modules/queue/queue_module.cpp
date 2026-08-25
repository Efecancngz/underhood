#include "queue_module.hpp"

#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string QueueModule::name() const {
    return "queue";
}

std::string QueueModule::codeSnippet() const {
    return "";
}

std::vector<underhood::IOperationalModule::Operation> QueueModule::operations() const {
    return {{"Enqueue", true}, {"Dequeue", false}};
}

bool QueueModule::canPerform(const std::string& operationLabel) const {
    if (operationLabel == "Enqueue") {
        return !entries_.full(kMaxSize);
    }
    if (operationLabel == "Dequeue") {
        return !entries_.empty();
    }
    return false;
}

void QueueModule::performOperation(const std::string& operationLabel, int value) {
    if (operationLabel == "Enqueue" && !entries_.full(kMaxSize)) {
        entries_.insertAt(entries_.size(), value);
        history_.append("Enqueue " + std::to_string(value));
    } else if (operationLabel == "Dequeue" && !entries_.empty()) {
        int frontVal = entries_.entries().front().value;
        entries_.removeAt(0);
        history_.append("Dequeue -> " + std::to_string(frontVal));
    }
}

std::vector<std::string> QueueModule::history() const {
    return history_.entries();
}

void QueueModule::update(float deltaTime) {
    entries_.update(deltaTime);
}

void QueueModule::clear() {
    entries_ = underhood::AnimatedList<int>();
    history_.clear();
}

std::size_t QueueModule::size() const {
    return entries_.size();
}

int QueueModule::frontValue() const {
    return entries_.entries().front().value;
}

void QueueModule::render(underhood::Canvas& canvas) const {
    constexpr int kAvailableWidth = 400;
    constexpr int kBaseX = 20;
    constexpr int kBoxHeight = 60;
    constexpr int kBaseY = 100;

    const auto& entries = entries_.entries();
    std::size_t slots = entries.size() + 1;  // +1 keeps room even when nearly full
    int cellWidth = kAvailableWidth / static_cast<int>(slots);
    int boxWidth = cellWidth > 20 ? cellWidth - 10 : cellWidth;

    auto drawEntry = [&](const underhood::AnimatedList<int>::Entry& entry, std::size_t visualIndex,
                          bool isFront, bool isRemoving) {
        // A removing entry shares insertAnim with a fresh insert (see
        // AnimatedList::removeAt), so its progress() also runs 0->1 --
        // inverting it here makes the departing box start fully "arrived"
        // and animate OUT, instead of animating back IN on top of whatever
        // now occupies its old slot.
        float progress = isRemoving ? 1.0f - entry.insertAnim.progress() : entry.insertAnim.progress();
        int x = kBaseX + static_cast<int>(visualIndex) * cellWidth;
        int dropOffset = static_cast<int>((1.0f - progress) * 30.0f);
        underhood::BoxState state = entry.insertAnim.isAnimating() ? underhood::BoxState::JustChanged
                                     : isFront                     ? underhood::BoxState::Owned
                                                                     : underhood::BoxState::Empty;
        canvas.drawBox(x, kBaseY - dropOffset, boxWidth, kBoxHeight, std::to_string(entry.value), state);
    };

    for (std::size_t i = 0; i < entries.size(); ++i) {
        drawEntry(entries[i], i, i == 0, /*isRemoving=*/false);
    }
    if (const auto* removing = entries_.removingEntry()) {
        drawEntry(*removing, 0, true, /*isRemoving=*/true);  // dequeue always removes index 0
    }

    canvas.drawText("front", kBaseX, kBaseY + kBoxHeight + 10);
    if (!entries.empty()) {
        int backX = kBaseX + static_cast<int>(entries.size() - 1) * cellWidth;
        canvas.drawText("back", backX, kBaseY + kBoxHeight + 10);
    }
}

}  // namespace underhood::modules

namespace {
struct QueueRegistrar {
    QueueRegistrar() {
        underhood::ModuleRegistry::instance().registerModule(
            "queue", []() { return std::make_unique<underhood::modules::QueueModule>(); },
            "Data Structures");
    }
};
const QueueRegistrar queueRegistrar;
}  // namespace

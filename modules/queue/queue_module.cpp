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

namespace {
constexpr std::size_t kMaxHistoryEntries = 10;

void appendHistory(std::vector<std::string>& history, std::string entry) {
    history.push_back(std::move(entry));
    if (history.size() > kMaxHistoryEntries) {
        history.erase(history.begin());
    }
}
}  // namespace

void QueueModule::performOperation(const std::string& operationLabel, int value) {
    if (operationLabel == "Enqueue" && !entries_.full(kMaxSize)) {
        entries_.insertAt(entries_.size(), value);
        appendHistory(history_, "Enqueue " + std::to_string(value));
    } else if (operationLabel == "Dequeue" && !entries_.empty()) {
        int frontVal = entries_.entries().front().value;
        entries_.removeAt(0);
        appendHistory(history_, "Dequeue -> " + std::to_string(frontVal));
    }
}

std::vector<std::string> QueueModule::history() const {
    return history_;
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
                          bool isFront) {
        float progress = entry.insertAnim.progress();
        int x = kBaseX + static_cast<int>(visualIndex) * cellWidth;
        int dropOffset = static_cast<int>((1.0f - progress) * 30.0f);
        underhood::BoxState state = entry.insertAnim.isAnimating() ? underhood::BoxState::JustChanged
                                     : isFront                     ? underhood::BoxState::Owned
                                                                     : underhood::BoxState::Empty;
        canvas.drawBox(x, kBaseY - dropOffset, boxWidth, kBoxHeight, std::to_string(entry.value), state);
    };

    for (std::size_t i = 0; i < entries.size(); ++i) {
        drawEntry(entries[i], i, i == 0);
    }
    if (const auto* removing = entries_.removingEntry()) {
        drawEntry(*removing, 0, true);  // dequeue always removes index 0
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

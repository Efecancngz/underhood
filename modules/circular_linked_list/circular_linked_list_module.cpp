#include "circular_linked_list_module.hpp"

#include <algorithm>
#include <memory>
#include <string>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string CircularLinkedListModule::name() const {
    return "linked_list_circular";
}

std::string CircularLinkedListModule::codeSnippet() const {
    return "";
}

std::vector<underhood::IOperationalModule::Operation> CircularLinkedListModule::operations() const {
    return {{"Insert", true}, {"Delete Front", false}};
}

bool CircularLinkedListModule::canPerform(const std::string& operationLabel) const {
    if (operationLabel == "Insert") {
        return !entries_.full(kMaxSize);
    }
    if (operationLabel == "Delete Front") {
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

void CircularLinkedListModule::performOperation(const std::string& operationLabel, int value) {
    if (operationLabel == "Insert" && !entries_.full(kMaxSize)) {
        entries_.insertAt(entries_.size(), value);  // always appends to the back
        appendHistory(history_, "Insert " + std::to_string(value));
    } else if (operationLabel == "Delete Front" && !entries_.empty()) {
        int frontVal = entries_.entries().front().value;
        entries_.removeAt(0);
        appendHistory(history_, "Delete Front -> " + std::to_string(frontVal));
    }
}

std::vector<std::string> CircularLinkedListModule::history() const {
    return history_;
}

void CircularLinkedListModule::update(float deltaTime) {
    entries_.update(deltaTime);
}

void CircularLinkedListModule::clear() {
    entries_ = underhood::AnimatedList<int>();
    history_.clear();
}

std::size_t CircularLinkedListModule::size() const {
    return entries_.size();
}

int CircularLinkedListModule::frontValue() const {
    return entries_.entries().front().value;
}

void CircularLinkedListModule::render(underhood::Canvas& canvas) const {
    constexpr int kAvailableWidth = 400;
    constexpr int kBaseX = 10;
    constexpr int kBoxHeight = 50;
    constexpr int kBaseY = 100;

    const auto& entries = entries_.entries();
    std::size_t slots = entries.size() + 1;
    int cellWidth = kAvailableWidth / static_cast<int>(std::max<std::size_t>(slots, 1));
    int boxWidth = cellWidth > 20 ? cellWidth - 15 : cellWidth;

    auto drawEntry = [&](const underhood::AnimatedList<int>::Entry& entry, std::size_t visualIndex) {
        float progress = entry.insertAnim.progress();
        int x = kBaseX + static_cast<int>(visualIndex) * cellWidth;
        int dropOffset = static_cast<int>((1.0f - progress) * 30.0f);
        underhood::BoxState state =
            entry.insertAnim.isAnimating() ? underhood::BoxState::JustChanged : underhood::BoxState::Owned;
        canvas.drawBox(x, kBaseY - dropOffset, boxWidth, kBoxHeight, std::to_string(entry.value), state);
    };

    for (std::size_t i = 0; i < entries.size(); ++i) {
        drawEntry(entries[i], i);
        if (i + 1 < entries.size()) {
            int arrowStartX = kBaseX + static_cast<int>(i) * cellWidth + boxWidth;
            int arrowY = kBaseY + kBoxHeight / 2;
            canvas.drawArrow(arrowStartX, arrowY, kBaseX + static_cast<int>(i + 1) * cellWidth, arrowY,
                              underhood::BoxState::Owned);
        }
    }
    if (const auto* removing = entries_.removingEntry()) {
        drawEntry(*removing, 0);
    }

    // v1 simplification: the wrap-around is a straight line to the first
    // box plus a text label, not a true curved arrow (Canvas::drawArrow only
    // draws straight lines; see the design spec's "Bilinen sadeleştirmeler").
    if (entries.size() >= 2) {
        int lastX = kBaseX + static_cast<int>(entries.size() - 1) * cellWidth + boxWidth / 2;
        int firstX = kBaseX + boxWidth / 2;
        int topY = kBaseY - 20;
        canvas.drawArrow(lastX, kBaseY, firstX, topY, underhood::BoxState::JustChanged);
        canvas.drawText("wraps to front", kBaseX, kBaseY + kBoxHeight + 10);
    } else if (entries.size() == 1) {
        canvas.drawText("wraps to front (self)", kBaseX, kBaseY + kBoxHeight + 10);
    }
}

}  // namespace underhood::modules

namespace {
struct CircularLinkedListRegistrar {
    CircularLinkedListRegistrar() {
        underhood::ModuleRegistry::instance().registerModule(
            "linked_list_circular",
            []() { return std::make_unique<underhood::modules::CircularLinkedListModule>(); },
            "Data Structures");
    }
};
const CircularLinkedListRegistrar circularLinkedListRegistrar;
}  // namespace

#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "underhood/animation.hpp"

namespace underhood {

// Generic ordered collection with per-entry insert/remove animation state,
// shared by all IOperationalModule data-structure modules (Stack, Queue,
// linked lists) so each doesn't reimplement the same bookkeeping. Insertion
// starts that entry's Animator; removal moves the entry into a single
// "removingEntry" slot with its own fade-out Animator instead of deleting
// it immediately, so render() can still draw it mid-animation.
template <typename T>
class AnimatedList {
public:
    struct Entry {
        T value;
        Animator insertAnim;
    };

    void insertAt(std::size_t index, T value) {
        Entry entry{std::move(value), Animator{}};
        entry.insertAnim.start();
        entries_.insert(entries_.begin() + static_cast<std::ptrdiff_t>(index), std::move(entry));
    }

    void removeAt(std::size_t index) {
        Entry removed = std::move(entries_[index]);
        entries_.erase(entries_.begin() + static_cast<std::ptrdiff_t>(index));
        removingEntry_ = std::move(removed);
        removingEntry_->insertAnim.start();
    }

    void update(float deltaTime) {
        for (auto& entry : entries_) {
            entry.insertAnim.update(deltaTime);
        }
        if (removingEntry_) {
            removingEntry_->insertAnim.update(deltaTime);
            if (!removingEntry_->insertAnim.isAnimating()) {
                removingEntry_.reset();
            }
        }
    }

    const std::vector<Entry>& entries() const { return entries_; }
    const Entry* removingEntry() const { return removingEntry_ ? &*removingEntry_ : nullptr; }
    std::size_t size() const { return entries_.size(); }
    bool empty() const { return entries_.empty(); }
    bool full(std::size_t maxSize) const { return entries_.size() >= maxSize; }

private:
    std::vector<Entry> entries_;
    std::optional<Entry> removingEntry_;
};

}  // namespace underhood

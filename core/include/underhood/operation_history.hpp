#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace underhood {

// Append-only operation log shared by IOperationalModule implementations
// (Stack, Queue, linked lists), capped at a fixed size (oldest entries drop
// first) so it never grows unbounded across a long session. Header-only,
// same pattern as AnimatedList (core/include/underhood/animated_list.hpp) --
// no .cpp/CMakeLists changes needed to add it.
class OperationHistory {
public:
    void append(std::string entry) {
        entries_.push_back(std::move(entry));
        if (entries_.size() > kMaxEntries) {
            entries_.erase(entries_.begin());
        }
    }

    void clear() { entries_.clear(); }

    const std::vector<std::string>& entries() const { return entries_; }

private:
    static constexpr std::size_t kMaxEntries = 10;
    std::vector<std::string> entries_;
};

}  // namespace underhood

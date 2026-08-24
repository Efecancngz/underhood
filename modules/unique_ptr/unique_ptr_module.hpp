#pragma once

#include <optional>

#include "underhood/canvas.hpp"
#include "underhood/simulation_module.hpp"

namespace underhood::modules {

class UniquePtrModule : public underhood::ISimulationModule {
public:
    std::string name() const override;
    std::string codeSnippet() const override;
    std::vector<underhood::Parameter> parameters() const override;
    void reset(const std::vector<underhood::Parameter>& params) override;
    bool step() override;
    int currentHighlightedLine() const override;
    void render(underhood::Canvas& canvas) const override;

    // Test-facing accessors (not part of ISimulationModule).
    bool ownedByA() const;
    bool ownedByB() const;
    std::optional<int> currentValue() const;

private:
    int phase_ = 0;
    int highlightedLine_ = 1;
    std::optional<int> aValue_;
    std::optional<int> bValue_;
    bool aJustChanged_ = false;
    bool bJustChanged_ = false;
};

}  // namespace underhood::modules

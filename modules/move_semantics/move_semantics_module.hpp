#pragma once

#include <optional>

#include "underhood/canvas.hpp"
#include "underhood/simulation_module.hpp"

namespace underhood::modules {

class MoveSemanticsModule : public underhood::IStepSimulationModule {
public:
    std::string name() const override;
    std::string codeSnippet() const override;
    std::vector<underhood::Parameter> parameters() const override;
    void reset(const std::vector<underhood::Parameter>& params) override;
    bool step() override;
    int currentHighlightedLine() const override;
    void render(underhood::Canvas& canvas) const override;

    std::optional<int> r1Value() const;
    std::optional<int> r2Value() const;

private:
    int phase_ = 0;
    int highlightedLine_ = 1;
    int newValue_ = 0;
    std::optional<int> r1Value_;
    std::optional<int> r2Value_;
    bool r1JustChanged_ = false;
    bool r2JustChanged_ = false;
};

}  // namespace underhood::modules

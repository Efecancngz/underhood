#pragma once

#include "underhood/canvas.hpp"
#include "underhood/simulation_module.hpp"

// Copy this folder to modules/<your_topic>/, rename the class and files,
// then implement each method below for your topic. See CONTRIBUTING.md
// for the full walkthrough.
namespace underhood::modules {

class TemplateModule : public underhood::IStepSimulationModule {
public:
    std::string name() const override;
    std::string codeSnippet() const override;
    std::vector<underhood::Parameter> parameters() const override;
    void reset(const std::vector<underhood::Parameter>& params) override;
    bool step() override;
    int currentHighlightedLine() const override;
    void render(underhood::Canvas& canvas) const override;

private:
    int phase_ = 0;
    int highlightedLine_ = 1;
};

}  // namespace underhood::modules

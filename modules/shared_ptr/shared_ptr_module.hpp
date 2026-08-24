#pragma once

#include "underhood/canvas.hpp"
#include "underhood/simulation_module.hpp"

namespace underhood::modules {

class SharedPtrModule : public underhood::ISimulationModule {
public:
    std::string name() const override;
    std::string codeSnippet() const override;
    std::vector<underhood::Parameter> parameters() const override;
    void reset(const std::vector<underhood::Parameter>& params) override;
    bool step() override;
    int currentHighlightedLine() const override;
    void render(underhood::Canvas& canvas) const override;

    int refCount() const;
    bool aAlive() const;
    bool bAlive() const;

private:
    int phase_ = 0;
    int highlightedLine_ = 1;
    int value_ = 0;
    int refCount_ = 0;
    bool aAlive_ = false;
    bool bAlive_ = false;
    bool refCountJustChanged_ = false;
    bool aJustChanged_ = false;
    bool bJustChanged_ = false;
};

}  // namespace underhood::modules

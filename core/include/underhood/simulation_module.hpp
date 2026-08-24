#pragma once

#include <string>
#include <vector>

#include "underhood/parameter.hpp"

namespace underhood {

class Canvas;  // defined in core/include/underhood/canvas.hpp (Task 6)

class ISimulationModule {
public:
    virtual ~ISimulationModule() = default;

    virtual std::string name() const = 0;
    virtual std::string codeSnippet() const = 0;
    virtual std::vector<Parameter> parameters() const = 0;
    virtual void reset(const std::vector<Parameter>& params) = 0;
    virtual bool step() = 0;
    virtual int currentHighlightedLine() const = 0;
    virtual void render(Canvas& canvas) const = 0;
};

}  // namespace underhood

#pragma once

#include <string>
#include <vector>

#include "underhood/parameter.hpp"

namespace underhood {

class Canvas;  // defined in core/include/underhood/canvas.hpp

enum class ModuleKind { Step, Operational };

// Common ground between the two module kinds below. ModuleRegistry::create()
// returns this type; the launcher checks kind() to decide which derived
// interface to cast to and which panel set to draw.
class ISimulationModule {
public:
    virtual ~ISimulationModule() = default;

    virtual std::string name() const = 0;
    // Fixed code snippet shown in the launcher's Code panel. Operational
    // modules return "" -- the launcher hides the Code panel's content
    // entirely when this is empty (their state speaks for itself; see
    // docs/superpowers/specs/2026-08-25-underhood-operational-modules-design.md).
    virtual std::string codeSnippet() const = 0;
    virtual void render(Canvas& canvas) const = 0;
    virtual ModuleKind kind() const = 0;
};

// A module that walks through one fixed, pre-scripted scenario one step()
// at a time (e.g. unique_ptr, shared_ptr, move_semantics). Behavior is
// unchanged from before this split -- existing modules only need their base
// class renamed from ISimulationModule to this.
class IStepSimulationModule : public ISimulationModule {
public:
    ModuleKind kind() const override { return ModuleKind::Step; }

    virtual std::vector<Parameter> parameters() const = 0;
    virtual void reset(const std::vector<Parameter>& params) = 0;
    virtual bool step() = 0;
    virtual int currentHighlightedLine() const = 0;
};

// A module the user drives with open-ended operations (Push/Pop,
// Enqueue/Dequeue, Insert/Delete) rather than a fixed scenario -- e.g. the
// Data Structures modules. See the design spec referenced above for the
// full rationale and the animation contract (update()).
class IOperationalModule : public ISimulationModule {
public:
    ModuleKind kind() const override { return ModuleKind::Operational; }

    struct Operation {
        std::string label;  // exact button text, e.g. "Push"
        bool takesValue;    // true: launcher shows a value input next to it
    };

    virtual std::vector<Operation> operations() const = 0;
    // Whether operationLabel can currently run (empty/full guards). The
    // launcher disables the corresponding button when this is false.
    virtual bool canPerform(const std::string& operationLabel) const = 0;
    virtual void performOperation(const std::string& operationLabel, int value) = 0;
    // Append-only log, oldest first. The launcher displays it reversed
    // (newest on top) and does not reverse this vector itself.
    virtual std::vector<std::string> history() const = 0;
    // Advances any in-flight insert/remove animations. Called every frame
    // by the launcher while this module is active, regardless of user input.
    virtual void update(float deltaTime) = 0;
    // Resets to empty instantly (no animation) and clears history().
    virtual void clear() = 0;
};

}  // namespace underhood

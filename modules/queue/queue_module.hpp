#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "underhood/animated_list.hpp"
#include "underhood/canvas.hpp"
#include "underhood/operation_history.hpp"
#include "underhood/simulation_module.hpp"

namespace underhood::modules {

class QueueModule : public underhood::IOperationalModule {
public:
    std::string name() const override;
    std::string codeSnippet() const override;
    void render(underhood::Canvas& canvas) const override;

    std::vector<Operation> operations() const override;
    bool canPerform(const std::string& operationLabel) const override;
    void performOperation(const std::string& operationLabel, int value) override;
    std::vector<std::string> history() const override;
    void update(float deltaTime) override;
    void clear() override;

    // Test-facing accessors (not part of IOperationalModule).
    std::size_t size() const;
    int frontValue() const;  // caller must check size() > 0 first

private:
    static constexpr std::size_t kMaxSize = 8;
    underhood::AnimatedList<int> entries_;
    underhood::OperationHistory history_;
};

}  // namespace underhood::modules

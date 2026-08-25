#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <memory>

#include "underhood/module_registry.hpp"

namespace {

class FakeModule : public underhood::IStepSimulationModule {
public:
    std::string name() const override { return "fake"; }
    std::string codeSnippet() const override { return "// fake"; }
    std::vector<underhood::Parameter> parameters() const override { return {}; }
    void reset(const std::vector<underhood::Parameter>&) override {}
    bool step() override { return false; }
    int currentHighlightedLine() const override { return 1; }
    void render(underhood::Canvas&) const override {}
};

}  // namespace

TEST_CASE("ModuleRegistry registers and creates modules by name") {
    auto& registry = underhood::ModuleRegistry::instance();
    registry.registerModule("fake_registry_test", []() {
        return std::make_unique<FakeModule>();
    });

    auto names = registry.moduleNames();
    REQUIRE(std::find(names.begin(), names.end(), "fake_registry_test") != names.end());

    auto instance = registry.create("fake_registry_test");
    REQUIRE(instance != nullptr);
    REQUIRE(instance->name() == "fake");
}

TEST_CASE("ModuleRegistry returns nullptr for an unknown module name") {
    auto& registry = underhood::ModuleRegistry::instance();
    REQUIRE(registry.create("does_not_exist_at_all") == nullptr);
}

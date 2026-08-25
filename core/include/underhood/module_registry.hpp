#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "underhood/simulation_module.hpp"

namespace underhood {

class ModuleRegistry {
public:
    using Factory = std::function<std::unique_ptr<ISimulationModule>()>;

    struct Category {
        std::string name;
        std::vector<std::string> moduleNames;
    };

    static ModuleRegistry& instance();

    // category groups modules in the launcher's sidebar (e.g. "Smart
    // Pointers", "Data Structures"). Defaults to "Simulations" so existing
    // callers/tests that don't care about grouping keep working.
    void registerModule(const std::string& name, Factory factory,
                         const std::string& category = "Simulations");
    std::vector<std::string> moduleNames() const;
    // Categories in first-registered order; each category's module names
    // alphabetically sorted. What the launcher's sidebar iterates over.
    std::vector<Category> categories() const;
    std::unique_ptr<ISimulationModule> create(const std::string& name) const;

    ModuleRegistry(const ModuleRegistry&) = delete;
    ModuleRegistry& operator=(const ModuleRegistry&) = delete;

private:
    ModuleRegistry() = default;

    std::map<std::string, Factory> factories_;
    std::map<std::string, std::string> categoryByModule_;
    std::vector<std::string> categoryOrder_;
};

}  // namespace underhood

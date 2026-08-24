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

    static ModuleRegistry& instance();

    void registerModule(const std::string& name, Factory factory);
    std::vector<std::string> moduleNames() const;
    std::unique_ptr<ISimulationModule> create(const std::string& name) const;

    ModuleRegistry(const ModuleRegistry&) = delete;
    ModuleRegistry& operator=(const ModuleRegistry&) = delete;

private:
    ModuleRegistry() = default;

    std::map<std::string, Factory> factories_;
};

}  // namespace underhood

#include "underhood/module_registry.hpp"

namespace underhood {

ModuleRegistry& ModuleRegistry::instance() {
    static ModuleRegistry registry;
    return registry;
}

void ModuleRegistry::registerModule(const std::string& name, Factory factory) {
    factories_[name] = std::move(factory);
}

std::vector<std::string> ModuleRegistry::moduleNames() const {
    std::vector<std::string> names;
    names.reserve(factories_.size());
    for (const auto& [name, factory] : factories_) {
        names.push_back(name);
    }
    return names;  // std::map keys are already sorted
}

std::unique_ptr<ISimulationModule> ModuleRegistry::create(const std::string& name) const {
    auto it = factories_.find(name);
    if (it == factories_.end()) {
        return nullptr;
    }
    return it->second();
}

}  // namespace underhood

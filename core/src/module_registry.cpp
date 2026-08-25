#include "underhood/module_registry.hpp"

#include <algorithm>

namespace underhood {

ModuleRegistry& ModuleRegistry::instance() {
    static ModuleRegistry registry;
    return registry;
}

void ModuleRegistry::registerModule(const std::string& name, Factory factory,
                                     const std::string& category) {
    factories_[name] = std::move(factory);
    categoryByModule_[name] = category;
    if (std::find(categoryOrder_.begin(), categoryOrder_.end(), category) == categoryOrder_.end()) {
        categoryOrder_.push_back(category);
    }
}

std::vector<std::string> ModuleRegistry::moduleNames() const {
    std::vector<std::string> names;
    names.reserve(factories_.size());
    for (const auto& [name, factory] : factories_) {
        names.push_back(name);
    }
    return names;  // std::map keys are already sorted
}

std::vector<ModuleRegistry::Category> ModuleRegistry::categories() const {
    std::vector<Category> result;
    result.reserve(categoryOrder_.size());
    for (const auto& categoryName : categoryOrder_) {
        Category category{categoryName, {}};
        for (const auto& [moduleName, moduleCategory] : categoryByModule_) {
            if (moduleCategory == categoryName) {
                category.moduleNames.push_back(moduleName);
            }
        }
        std::sort(category.moduleNames.begin(), category.moduleNames.end());
        result.push_back(std::move(category));
    }
    return result;
}

std::unique_ptr<ISimulationModule> ModuleRegistry::create(const std::string& name) const {
    auto it = factories_.find(name);
    if (it == factories_.end()) {
        return nullptr;
    }
    return it->second();
}

}  // namespace underhood

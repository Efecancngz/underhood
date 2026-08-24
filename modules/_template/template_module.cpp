#include "template_module.hpp"

#include <memory>

#include "underhood/module_registry.hpp"

namespace underhood::modules {

std::string TemplateModule::name() const {
    return "template";
}

std::string TemplateModule::codeSnippet() const {
    return "// Replace with the code snippet your simulation walks through.";
}

std::vector<underhood::Parameter> TemplateModule::parameters() const {
    return {};
}

void TemplateModule::reset(const std::vector<underhood::Parameter>& /*params*/) {
    phase_ = 0;
    highlightedLine_ = 1;
}

bool TemplateModule::step() {
    return false;  // replace with your step state machine
}

int TemplateModule::currentHighlightedLine() const {
    return highlightedLine_;
}

void TemplateModule::render(underhood::Canvas& canvas) const {
    canvas.drawText("Replace render() with your visualization.", 50, 50);
}

}  // namespace underhood::modules

// This module is not registered by default (modules/_template/ is not added
// to modules/CMakeLists.txt). Uncomment the block below once you rename
// this class and want it discoverable in the launcher menu:
//
// namespace {
// struct TemplateRegistrar {
//     TemplateRegistrar() {
//         underhood::ModuleRegistry::instance().registerModule("template", []() {
//             return std::make_unique<underhood::modules::TemplateModule>();
//         });
//     }
// };
// const TemplateRegistrar templateRegistrar;
// }  // namespace

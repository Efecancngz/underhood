#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"
#include "underhood/canvas.hpp"
#include "underhood/module_registry.hpp"
#include "underhood/simulation_module.hpp"

namespace {

enum class AppState { Menu, Simulating };

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::stringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

void drawCodePanel(const underhood::ISimulationModule& module) {
    ImGui::Begin("Code");
    auto lines = splitLines(module.codeSnippet());
    for (std::size_t i = 0; i < lines.size(); ++i) {
        int lineNumber = static_cast<int>(i) + 1;
        if (lineNumber == module.currentHighlightedLine()) {
            // Amber, matching Canvas's JustChanged box color (see canvas.cpp) --
            // the same color means "this is what just happened" in both panels.
            ImGui::TextColored(ImVec4(0.90f, 0.62f, 0.0f, 1.0f), "%s", lines[i].c_str());
        } else {
            ImGui::Text("%s", lines[i].c_str());
        }
    }
    ImGui::End();
}

void drawParameterPanel(std::vector<underhood::Parameter>& params, bool& resetRequested,
                         bool& stepRequested, bool& backRequested) {
    ImGui::Begin("Controls");
    for (auto& param : params) {
        ImGui::SliderInt(param.name.c_str(), &param.value, param.minValue, param.maxValue);
    }
    if (ImGui::Button("Reset")) {
        resetRequested = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Next Step")) {
        stepRequested = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Back to Menu")) {
        backRequested = true;
    }
    ImGui::End();
}

void drawThemePanel(bool& lightTheme, bool& themeChanged) {
    ImGui::Begin("Theme");
    if (ImGui::Checkbox("Light theme", &lightTheme)) {
        themeChanged = true;
    }
    ImGui::End();
}

}  // namespace

int main() {
    InitWindow(1024, 768, "underhood");
    SetTargetFPS(60);
    rlImGuiSetup(true);
    ImGui::StyleColorsDark();

    AppState state = AppState::Menu;
    std::unique_ptr<underhood::ISimulationModule> activeModule;
    std::vector<underhood::Parameter> activeParams;
    underhood::Canvas canvas;
    bool lightTheme = false;

    while (!WindowShouldClose()) {
        Color backgroundColor = lightTheme ? RAYWHITE : Color{24, 24, 24, 255};

        BeginDrawing();
        ClearBackground(backgroundColor);
        rlImGuiBegin();

        bool themeChanged = false;
        drawThemePanel(lightTheme, themeChanged);
        if (themeChanged) {
            if (lightTheme) {
                ImGui::StyleColorsLight();
            } else {
                ImGui::StyleColorsDark();
            }
            canvas.setDarkTheme(!lightTheme);
        }

        if (state == AppState::Menu) {
            ImGui::Begin("underhood");
            for (const auto& moduleName : underhood::ModuleRegistry::instance().moduleNames()) {
                if (ImGui::Button(moduleName.c_str())) {
                    activeModule = underhood::ModuleRegistry::instance().create(moduleName);
                    activeParams = activeModule->parameters();
                    activeModule->reset(activeParams);
                    state = AppState::Simulating;
                }
            }
            ImGui::End();
        } else {
            activeModule->render(canvas);
            drawCodePanel(*activeModule);

            bool resetRequested = false;
            bool stepRequested = false;
            bool backRequested = false;
            drawParameterPanel(activeParams, resetRequested, stepRequested, backRequested);

            if (resetRequested) {
                activeModule->reset(activeParams);
            }
            if (stepRequested) {
                activeModule->step();
            }
            if (backRequested) {
                activeModule.reset();
                state = AppState::Menu;
            }
        }

        rlImGuiEnd();
        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();
    return 0;
}

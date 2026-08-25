#include <cctype>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"  // DockBuilder* -- programmatic initial layout
#include "raylib.h"
#include "rlImGui.h"
#include "underhood/canvas.hpp"
#include "underhood/module_registry.hpp"
#include "underhood/simulation_module.hpp"
#include "underhood/theme.hpp"

namespace {

constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 800;
constexpr const char* kFontPath = UNDERHOOD_ASSETS_DIR "/fonts/Inter-Regular.ttf";

// Modules draw in this fixed logical coordinate space (chosen to comfortably
// fit every module's box layout, e.g. shared_ptr's rightmost box at x=370,
// y=210, with margin) regardless of the actual Visualization panel's pixel
// size. Canvas content is scaled+centered ("letterboxed") into whatever
// panel size is available -- see letterboxCamera() -- so modules never need
// to know or care how big the window is.
constexpr float kLogicalCanvasWidth = 440.0f;
constexpr float kLogicalCanvasHeight = 260.0f;

Camera2D letterboxCamera(int textureWidth, int textureHeight) {
    float scale = (textureWidth / kLogicalCanvasWidth < textureHeight / kLogicalCanvasHeight)
                      ? textureWidth / kLogicalCanvasWidth
                      : textureHeight / kLogicalCanvasHeight;
    float offsetX = (textureWidth - kLogicalCanvasWidth * scale) * 0.5f;
    float offsetY = (textureHeight - kLogicalCanvasHeight * scale) * 0.5f;
    Camera2D camera{};
    camera.offset = Vector2{offsetX, offsetY};
    camera.target = Vector2{0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = scale;
    return camera;
}

const char* kModulesPanel = "Modules";
const char* kCodePanel = "Code";
const char* kControlsPanel = "Controls";
const char* kVisualizationPanel = "Visualization";

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::stringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

// Programmatic initial layout: left sidebar (Modules), right side split into
// a Code strip on top and Controls/Visualization below it. Rebuilt only once
// -- io.IniFilename is null (see main()) so ImGui never has a stale layout
// to restore instead, and the user can still freely re-drag/resize panels
// within a session.
void BuildInitialLayout(ImGuiID dockspaceId) {
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

    ImGuiID rightId;
    ImGuiID sidebarId = ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.18f, nullptr, &rightId);

    ImGuiID bottomId;
    ImGuiID codeId = ImGui::DockBuilderSplitNode(rightId, ImGuiDir_Up, 0.32f, nullptr, &bottomId);

    ImGuiID visualizationId;
    ImGuiID controlsId =
        ImGui::DockBuilderSplitNode(bottomId, ImGuiDir_Left, 0.32f, nullptr, &visualizationId);

    ImGui::DockBuilderDockWindow(kModulesPanel, sidebarId);
    ImGui::DockBuilderDockWindow(kCodePanel, codeId);
    ImGui::DockBuilderDockWindow(kControlsPanel, controlsId);
    ImGui::DockBuilderDockWindow(kVisualizationPanel, visualizationId);
    ImGui::DockBuilderFinish(dockspaceId);
}

void DrawModulesPanel(const std::string& activeModuleName, bool& moduleClickedOut,
                       std::string& clickedModuleName, bool& lightTheme, bool& themeChanged) {
    ImGui::Begin(kModulesPanel);

    for (const auto& category : underhood::ModuleRegistry::instance().categories()) {
        std::string header = category.name;
        for (auto& c : header) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "%s", header.c_str());
        ImGui::Spacing();

        for (const auto& moduleName : category.moduleNames) {
            bool selected = (moduleName == activeModuleName);
            if (ImGui::Selectable(moduleName.c_str(), selected, 0, ImVec2(0, 36))) {
                moduleClickedOut = true;
                clickedModuleName = moduleName;
            }
        }
        ImGui::Spacing();
        ImGui::Spacing();
    }

    ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 50.0f);
    ImGui::Separator();
    if (ImGui::Checkbox("Light theme", &lightTheme)) {
        themeChanged = true;
    }
    ImGui::End();
}

void DrawCodePanel(const underhood::ISimulationModule* module) {
    ImGui::Begin(kCodePanel);
    if (module == nullptr) {
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled],
                            "Select a simulation from the left to see its code.");
    } else if (module->kind() == underhood::ModuleKind::Step) {
        auto stepModule = static_cast<const underhood::IStepSimulationModule*>(module);
        auto lines = splitLines(module->codeSnippet());
        for (std::size_t i = 0; i < lines.size(); ++i) {
            int lineNumber = static_cast<int>(i) + 1;
            if (lineNumber == stepModule->currentHighlightedLine()) {
                // Amber, matching Canvas's JustChanged box color -- the same
                // color means "this is what just happened" in both panels.
                ImGui::TextColored(ImVec4(0.90f, 0.62f, 0.0f, 1.0f), "%s", lines[i].c_str());
            } else {
                ImGui::Text("%s", lines[i].c_str());
            }
        }
    } else {
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled],
                            "Operational modules show their state in the visualization.");
    }
    ImGui::End();
}

void DrawControlsPanel(const underhood::ISimulationModule* module,
                        std::vector<underhood::Parameter>& params, bool& resetRequested,
                        bool& stepRequested) {
    ImGui::Begin(kControlsPanel);
    if (module == nullptr) {
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "No simulation selected.");
        ImGui::End();
        return;
    }

    if (module->kind() != underhood::ModuleKind::Step) {
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled],
                            "Operational modules have operation buttons in the visualization.");
        ImGui::End();
        return;
    }

    auto stepModule = static_cast<const underhood::IStepSimulationModule*>(module);
    for (auto& param : params) {
        ImGui::InputInt(param.name.c_str(), &param.value);
        if (param.value < param.minValue) param.value = param.minValue;
        if (param.value > param.maxValue) param.value = param.maxValue;
    }
    ImGui::Spacing();
    if (ImGui::Button("Reset", ImVec2(-1, 0))) {
        resetRequested = true;
    }
    if (ImGui::Button("Next Step", ImVec2(-1, 0))) {
        stepRequested = true;
    }
    ImGui::End();
}

// Renders the active module (if any) into canvasTexture at exactly the
// panel's current content-region size, then displays it -- 1:1, never
// scaled by ImGui/rlImGui, so text and box edges stay crisp regardless of
// window size. Recreates the texture only when the panel is actually
// resized (checked every frame, cheap: two int comparisons).
void DrawVisualizationPanel(underhood::ISimulationModule* module, underhood::Canvas& canvas,
                             const underhood::Palette& palette, RenderTexture2D& canvasTexture) {
    ImGui::Begin(kVisualizationPanel);
    ImVec2 avail = ImGui::GetContentRegionAvail();
    int wantWidth = static_cast<int>(avail.x);
    int wantHeight = static_cast<int>(avail.y);
    // Guard against 0-size (panel mid-drag/collapsed) and runaway textures.
    wantWidth = wantWidth < 64 ? 64 : (wantWidth > 4096 ? 4096 : wantWidth);
    wantHeight = wantHeight < 64 ? 64 : (wantHeight > 4096 ? 4096 : wantHeight);

    if (wantWidth != canvasTexture.texture.width || wantHeight != canvasTexture.texture.height) {
        UnloadRenderTexture(canvasTexture);
        canvasTexture = LoadRenderTexture(wantWidth, wantHeight);
    }

    BeginTextureMode(canvasTexture);
    ClearBackground(palette.background);
    if (module != nullptr) {
        Camera2D camera = letterboxCamera(wantWidth, wantHeight);
        BeginMode2D(camera);
        module->render(canvas);
        EndMode2D();
    }
    EndTextureMode();

    rlImGuiImageRenderTextureFit(&canvasTexture, true);
    ImGui::End();
}

}  // namespace

int main() {
    InitWindow(kWindowWidth, kWindowHeight, "underhood");
    SetTargetFPS(60);

    Font sharedFont = LoadFontEx(kFontPath, 32, nullptr, 0);
    SetTextureFilter(sharedFont.texture, TEXTURE_FILTER_BILINEAR);

    rlImGuiSetLoadFontsCallback([]() {
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromFileTTF(kFontPath, 18.0f);
    });
    rlImGuiSetup(true);

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = nullptr;  // fixed layout every run, see BuildInitialLayout

    bool lightTheme = false;
    underhood::ApplyTheme(!lightTheme);

    underhood::Canvas canvas;
    canvas.setDarkTheme(!lightTheme);
    canvas.setFont(sharedFont);

    RenderTexture2D canvasTexture = LoadRenderTexture(64, 64);  // resized on first frame

    std::unique_ptr<underhood::ISimulationModule> activeModule;
    std::string activeModuleName;
    std::vector<underhood::Parameter> activeParams;

    bool layoutBuilt = false;

    while (!WindowShouldClose()) {
        const underhood::Palette& palette = underhood::GetPalette(!lightTheme);

        BeginDrawing();
        ClearBackground(palette.background);
        rlImGuiBegin();

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                      ImGuiWindowFlags_NoBringToFrontOnFocus |
                                      ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
        ImGui::Begin("##DockHost", nullptr, hostFlags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspaceId = ImGui::GetID("UnderhoodDockspace");
        if (!layoutBuilt) {
            BuildInitialLayout(dockspaceId);
            layoutBuilt = true;
        }
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        ImGui::End();

        bool themeChanged = false;
        bool moduleClicked = false;
        std::string clickedModuleName;
        DrawModulesPanel(activeModuleName, moduleClicked, clickedModuleName, lightTheme, themeChanged);
        if (themeChanged) {
            underhood::ApplyTheme(!lightTheme);
            canvas.setDarkTheme(!lightTheme);
        }
        if (moduleClicked) {
            activeModule = underhood::ModuleRegistry::instance().create(clickedModuleName);
            activeModuleName = clickedModuleName;
            if (activeModule && activeModule->kind() == underhood::ModuleKind::Step) {
                auto stepModule = static_cast<underhood::IStepSimulationModule*>(activeModule.get());
                activeParams = stepModule->parameters();
                stepModule->reset(activeParams);
            }
        }

        DrawCodePanel(activeModule.get());

        bool resetRequested = false;
        bool stepRequested = false;
        DrawControlsPanel(activeModule.get(), activeParams, resetRequested, stepRequested);
        if (resetRequested && activeModule && activeModule->kind() == underhood::ModuleKind::Step) {
            auto stepModule = static_cast<underhood::IStepSimulationModule*>(activeModule.get());
            stepModule->reset(activeParams);
        }
        if (stepRequested && activeModule && activeModule->kind() == underhood::ModuleKind::Step) {
            auto stepModule = static_cast<underhood::IStepSimulationModule*>(activeModule.get());
            stepModule->step();
        }

        DrawVisualizationPanel(activeModule.get(), canvas, palette, canvasTexture);

        rlImGuiEnd();
        EndDrawing();
    }

    UnloadRenderTexture(canvasTexture);
    UnloadFont(sharedFont);
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}

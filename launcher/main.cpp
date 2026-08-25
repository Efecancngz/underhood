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

// Mirrors theme.cpp's internal ToImVec4 (Color 0-255 -> ImVec4 0-1) so the
// Code panel's "just changed" highlight can read the real active-theme
// accent instead of a hardcoded literal -- see DrawCodePanel.
ImVec4 ToImVec4(Color c) {
    return ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, 1.0f);
}

// Small, font-independent icon per module -- drawn with raw ImDrawList
// primitives (circles/rects/lines) instead of glyphs, so it never depends on
// which codepoints the bundled Inter font happens to cover. Each shape is a
// loose visual mnemonic for the concept (two circles for shared ownership,
// a closed triangle of dots for a circular list, etc).
void DrawModuleIcon(ImDrawList* drawList, ImVec2 topLeft, float size, const std::string& moduleName,
                     ImU32 color) {
    float cx = topLeft.x + size * 0.5f;
    float cy = topLeft.y + size * 0.5f;
    float r = size * 0.16f;

    if (moduleName == "unique_ptr") {
        drawList->AddCircleFilled(ImVec2(cx, cy), r, color);
    } else if (moduleName == "shared_ptr") {
        drawList->AddCircleFilled(ImVec2(cx - r * 0.7f, cy), r, color);
        drawList->AddCircleFilled(ImVec2(cx + r * 0.7f, cy), r, color);
    } else if (moduleName == "move_semantics") {
        drawList->AddTriangleFilled(ImVec2(topLeft.x + size * 0.25f, topLeft.y + size * 0.22f),
                                     ImVec2(topLeft.x + size * 0.25f, topLeft.y + size * 0.78f),
                                     ImVec2(topLeft.x + size * 0.8f, cy), color);
    } else if (moduleName == "stack") {
        float barHeight = size * 0.15f;
        float barWidth = size * 0.68f;
        float startX = topLeft.x + (size - barWidth) * 0.5f;
        for (int i = 0; i < 3; ++i) {
            float y = topLeft.y + size * 0.18f + i * (barHeight + size * 0.09f);
            drawList->AddRectFilled(ImVec2(startX, y), ImVec2(startX + barWidth, y + barHeight), color,
                                     1.5f);
        }
    } else if (moduleName == "queue") {
        float barWidth = size * 0.15f;
        float barHeight = size * 0.58f;
        float startY = topLeft.y + (size - barHeight) * 0.5f;
        for (int i = 0; i < 3; ++i) {
            float x = topLeft.x + size * 0.14f + i * (barWidth + size * 0.11f);
            drawList->AddRectFilled(ImVec2(x, startY), ImVec2(x + barWidth, startY + barHeight), color,
                                     1.5f);
        }
    } else if (moduleName == "linked_list_singly") {
        float dotR = size * 0.09f;
        float positions[3] = {0.2f, 0.5f, 0.8f};
        for (int i = 0; i < 3; ++i) {
            float x = topLeft.x + size * positions[i];
            drawList->AddCircleFilled(ImVec2(x, cy), dotR, color);
            if (i < 2) {
                float nextX = topLeft.x + size * positions[i + 1];
                drawList->AddLine(ImVec2(x + dotR, cy), ImVec2(nextX - dotR, cy), color, 1.5f);
            }
        }
    } else if (moduleName == "linked_list_circular") {
        float dotR = size * 0.09f;
        ImVec2 p0(cx, topLeft.y + size * 0.2f);
        ImVec2 p1(topLeft.x + size * 0.2f, topLeft.y + size * 0.78f);
        ImVec2 p2(topLeft.x + size * 0.8f, topLeft.y + size * 0.78f);
        drawList->AddLine(p0, p1, color, 1.5f);
        drawList->AddLine(p1, p2, color, 1.5f);
        drawList->AddLine(p2, p0, color, 1.5f);
        drawList->AddCircleFilled(p0, dotR, color);
        drawList->AddCircleFilled(p1, dotR, color);
        drawList->AddCircleFilled(p2, dotR, color);
    } else {
        drawList->AddRectFilled(ImVec2(topLeft.x + size * 0.25f, topLeft.y + size * 0.25f),
                                 ImVec2(topLeft.x + size * 0.75f, topLeft.y + size * 0.75f), color, 1.5f);
    }
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

// Draws one module row: a full-width invisible Selectable for hit-testing
// (so the whole row is clickable, not just the text) with the icon + label
// painted on top via the draw list -- avoids fighting ImGui's layout cursor
// to align an icon next to Selectable's own text.
void DrawModuleRow(const std::string& moduleName, bool selected, bool& moduleClickedOut,
                    std::string& clickedModuleName) {
    ImVec2 rowStart = ImGui::GetCursorScreenPos();
    if (ImGui::Selectable(("##sel_" + moduleName).c_str(), selected, 0, ImVec2(0, 32))) {
        moduleClickedOut = true;
        clickedModuleName = moduleName;
    }
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);
    DrawModuleIcon(drawList, ImVec2(rowStart.x + 8, rowStart.y + 7), 18.0f, moduleName, textColor);
    drawList->AddText(ImVec2(rowStart.x + 34, rowStart.y + 8), textColor, moduleName.c_str());
}

// Categories are individually collapsible (ImGui::CollapsingHeader owns its
// own open/closed state per session -- no manual bookkeeping needed) so a
// category the user isn't using right now doesn't eat vertical space.
void DrawModulesPanel(const std::string& activeModuleName, bool& moduleClickedOut,
                       std::string& clickedModuleName, std::string& clickedCategoryName) {
    ImGui::Begin(kModulesPanel);

    for (const auto& category : underhood::ModuleRegistry::instance().categories()) {
        std::string header = category.name;
        for (auto& c : header) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        if (ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Spacing();
            for (const auto& moduleName : category.moduleNames) {
                bool selected = (moduleName == activeModuleName);
                bool wasClicked = moduleClickedOut;
                DrawModuleRow(moduleName, selected, moduleClickedOut, clickedModuleName);
                if (!wasClicked && moduleClickedOut) {
                    clickedCategoryName = category.name;
                }
            }
            ImGui::Spacing();
        }
        ImGui::Spacing();
    }

    ImGui::End();
}

// Drawn inside the dock host's menu bar (see main()), so it always sits in
// a fixed strip above every panel regardless of how the user has resized or
// re-docked things. Owns the theme toggle too -- moved here from the bottom
// of the sidebar so it reads as a persistent app-level control, not a
// module-list footer.
void DrawTopBar(const std::string& activeModuleName, const std::string& activeCategoryName,
                 bool& lightTheme, bool& themeChanged) {
    ImGui::TextUnformatted("underhood");
    if (!activeModuleName.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], ">");
        ImGui::SameLine();
        ImGui::TextUnformatted(activeCategoryName.c_str());
        ImGui::SameLine();
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], ">");
        ImGui::SameLine();
        ImGui::TextUnformatted(activeModuleName.c_str());
    }

    float toggleWidth = 130.0f;
    ImGui::SameLine(ImGui::GetWindowWidth() - toggleWidth);
    if (ImGui::Checkbox("Light theme", &lightTheme)) {
        themeChanged = true;
    }
}

void DrawCodePanel(const underhood::ISimulationModule* module, const underhood::Palette& palette) {
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
                // Matches Canvas's JustChanged box color for the active
                // theme (accentJustChanged) -- the same color means "this
                // is what just happened" in both panels, in both themes.
                ImGui::TextColored(ToImVec4(palette.accentJustChanged), "%s", lines[i].c_str());
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

void DrawStepControls(std::vector<underhood::Parameter>& params, bool& resetRequested,
                       bool& stepRequested) {
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
}

// takesValue is currently informational only -- v1 shows one shared value
// input regardless of which operation the user is about to click, rather
// than hiding/graying it out per-button. Operations that ignore the value
// (e.g. "Pop") simply don't read it in performOperation().
void DrawOperationalControls(underhood::IOperationalModule* module, int& pendingValue) {
    ImGui::InputInt("Value", &pendingValue);
    ImGui::Spacing();

    for (const auto& operation : module->operations()) {
        bool enabled = module->canPerform(operation.label);
        ImGui::BeginDisabled(!enabled);
        if (ImGui::Button(operation.label.c_str(), ImVec2(-1, 0))) {
            module->performOperation(operation.label, pendingValue);
        }
        ImGui::EndDisabled();
    }

    ImGui::Spacing();
    if (ImGui::Button("Clear", ImVec2(-1, 0))) {
        module->clear();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "HISTORY");
    ImGui::BeginChild("##history", ImVec2(0, 160), true);
    auto history = module->history();
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        ImGui::TextUnformatted(it->c_str());
    }
    ImGui::EndChild();
}

void DrawControlsPanel(underhood::ISimulationModule* module, std::vector<underhood::Parameter>& params,
                        int& pendingValue, bool& resetRequested, bool& stepRequested) {
    ImGui::Begin(kControlsPanel);
    if (module == nullptr) {
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "No simulation selected.");
    } else if (module->kind() == underhood::ModuleKind::Step) {
        DrawStepControls(params, resetRequested, stepRequested);
    } else {
        DrawOperationalControls(static_cast<underhood::IOperationalModule*>(module), pendingValue);
    }
    ImGui::End();
}

// Landing state: instead of an empty canvas before any module is picked,
// show a card per module (grouped by category, same icon as the sidebar
// row) so the panel always has content and offers a second way to start,
// matching dsa-visualizer's home screen. Clicking a card reports out through
// the same (clicked, name, category) triple DrawModulesPanel uses, so
// main() handles both selection sources identically.
void DrawModuleCardGrid(bool& moduleClickedOut, std::string& clickedModuleName,
                         std::string& clickedCategoryName) {
    constexpr float kCardWidth = 150.0f;
    constexpr float kCardHeight = 92.0f;
    constexpr float kSpacing = 12.0f;

    ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled],
                        "Select a simulation from the left, or pick one below.");
    ImGui::Spacing();
    ImGui::Spacing();

    for (const auto& category : underhood::ModuleRegistry::instance().categories()) {
        std::string header = category.name;
        for (auto& c : header) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        ImGui::TextColored(ImGui::GetStyle().Colors[ImGuiCol_TextDisabled], "%s", header.c_str());
        ImGui::Spacing();

        float windowVisibleX2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        for (std::size_t i = 0; i < category.moduleNames.size(); ++i) {
            const std::string& moduleName = category.moduleNames[i];
            ImGui::PushID(moduleName.c_str());
            ImVec2 cardPos = ImGui::GetCursorScreenPos();
            if (ImGui::Button("##card", ImVec2(kCardWidth, kCardHeight))) {
                moduleClickedOut = true;
                clickedModuleName = moduleName;
                clickedCategoryName = category.name;
            }
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImU32 textColor = ImGui::GetColorU32(ImGuiCol_Text);
            DrawModuleIcon(drawList, ImVec2(cardPos.x + kCardWidth * 0.5f - 12.0f, cardPos.y + 16.0f),
                           24.0f, moduleName, textColor);
            ImVec2 textSize = ImGui::CalcTextSize(moduleName.c_str());
            drawList->AddText(ImVec2(cardPos.x + (kCardWidth - textSize.x) * 0.5f, cardPos.y + kCardHeight - 26.0f),
                              textColor, moduleName.c_str());
            ImGui::PopID();

            float lastCardX2 = ImGui::GetItemRectMax().x;
            float nextCardX2 = lastCardX2 + kSpacing + kCardWidth;
            if (i + 1 < category.moduleNames.size() && nextCardX2 < windowVisibleX2) {
                ImGui::SameLine(0.0f, kSpacing);
            }
        }
        ImGui::Spacing();
        ImGui::Spacing();
    }
}

// Renders the active module (if any) into canvasTexture at exactly the
// panel's current content-region size, then displays it -- 1:1, never
// scaled by ImGui/rlImGui, so text and box edges stay crisp regardless of
// window size. Recreates the texture only when the panel is actually
// resized (checked every frame, cheap: two int comparisons). When no module
// is active, shows the card grid instead (see DrawModuleCardGrid).
void DrawVisualizationPanel(underhood::ISimulationModule* module, underhood::Canvas& canvas,
                             const underhood::Palette& palette, RenderTexture2D& canvasTexture,
                             bool& moduleClickedOut, std::string& clickedModuleName,
                             std::string& clickedCategoryName) {
    ImGui::Begin(kVisualizationPanel);

    if (module == nullptr) {
        DrawModuleCardGrid(moduleClickedOut, clickedModuleName, clickedCategoryName);
        ImGui::End();
        return;
    }

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
    std::string activeCategoryName;
    std::vector<underhood::Parameter> activeParams;
    int pendingValue = 0;

    bool layoutBuilt = false;

    auto selectModule = [&](const std::string& name, const std::string& category) {
        if (name == activeModuleName) return;
        activeModule = underhood::ModuleRegistry::instance().create(name);
        activeModuleName = name;
        activeCategoryName = category;
        pendingValue = 0;
        if (activeModule && activeModule->kind() == underhood::ModuleKind::Step) {
            auto stepModule = static_cast<underhood::IStepSimulationModule*>(activeModule.get());
            activeParams = stepModule->parameters();
            stepModule->reset(activeParams);
        } else {
            activeParams.clear();
        }
    };

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
                                      ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground |
                                      ImGuiWindowFlags_MenuBar;
        ImGui::Begin("##DockHost", nullptr, hostFlags);
        ImGui::PopStyleVar(3);

        bool themeChanged = false;
        if (ImGui::BeginMenuBar()) {
            DrawTopBar(activeModuleName, activeCategoryName, lightTheme, themeChanged);
            ImGui::EndMenuBar();
        }
        if (themeChanged) {
            underhood::ApplyTheme(!lightTheme);
            canvas.setDarkTheme(!lightTheme);
        }

        ImGuiID dockspaceId = ImGui::GetID("UnderhoodDockspace");
        if (!layoutBuilt) {
            BuildInitialLayout(dockspaceId);
            layoutBuilt = true;
        }
        ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
        ImGui::End();

        bool moduleClicked = false;
        std::string clickedModuleName;
        std::string clickedCategoryName;
        DrawModulesPanel(activeModuleName, moduleClicked, clickedModuleName, clickedCategoryName);
        if (moduleClicked) {
            selectModule(clickedModuleName, clickedCategoryName);
        }

        DrawCodePanel(activeModule.get(), palette);

        bool resetRequested = false;
        bool stepRequested = false;
        DrawControlsPanel(activeModule.get(), activeParams, pendingValue, resetRequested, stepRequested);
        if (resetRequested && activeModule && activeModule->kind() == underhood::ModuleKind::Step) {
            auto stepModule = static_cast<underhood::IStepSimulationModule*>(activeModule.get());
            stepModule->reset(activeParams);
        }
        if (stepRequested && activeModule && activeModule->kind() == underhood::ModuleKind::Step) {
            auto stepModule = static_cast<underhood::IStepSimulationModule*>(activeModule.get());
            stepModule->step();
        }
        if (activeModule && activeModule->kind() == underhood::ModuleKind::Operational) {
            static_cast<underhood::IOperationalModule*>(activeModule.get())->update(GetFrameTime());
        }

        bool cardClicked = false;
        std::string cardModuleName;
        std::string cardCategoryName;
        DrawVisualizationPanel(activeModule.get(), canvas, palette, canvasTexture, cardClicked,
                                cardModuleName, cardCategoryName);
        if (cardClicked) {
            selectModule(cardModuleName, cardCategoryName);
        }

        rlImGuiEnd();
        EndDrawing();
    }

    UnloadRenderTexture(canvasTexture);
    UnloadFont(sharedFont);
    rlImGuiShutdown();
    CloseWindow();
    return 0;
}

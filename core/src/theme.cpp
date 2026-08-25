#include "underhood/theme.hpp"

#include "imgui.h"

namespace underhood {

namespace {

// Deliberately near-black/near-white rather than raylib's or ImGui's stock
// grays -- flat, high-contrast, minimal color use, closer to a data
// structure visualizer (visualgo.net, dsa-visualizer) than a typical UI
// mockup. Semantic accent colors (empty/owned/justChanged) are unchanged
// from the original palette -- only the chrome around them changed.
const Palette kDarkPalette{
    /*background=*/Color{10, 10, 10, 255},
    /*surface=*/Color{22, 22, 22, 255},
    /*surfaceRaised=*/Color{34, 34, 34, 255},
    /*border=*/Color{45, 45, 45, 255},
    /*textPrimary=*/Color{235, 235, 235, 255},
    /*textMuted=*/Color{145, 145, 145, 255},
    /*accentEmpty=*/Color{110, 110, 110, 255},
    /*accentOwned=*/Color{86, 182, 194, 255},
    /*accentJustChanged=*/Color{230, 159, 0, 255},
};

const Palette kLightPalette{
    /*background=*/Color{242, 242, 242, 255},
    /*surface=*/Color{255, 255, 255, 255},
    /*surfaceRaised=*/Color{232, 232, 232, 255},
    /*border=*/Color{210, 210, 210, 255},
    /*textPrimary=*/Color{25, 25, 25, 255},
    /*textMuted=*/Color{110, 110, 110, 255},
    /*accentEmpty=*/Color{170, 170, 170, 255},
    /*accentOwned=*/Color{0, 121, 140, 255},
    /*accentJustChanged=*/Color{204, 102, 0, 255},
};

ImVec4 ToImVec4(Color c, float alpha = 1.0f) {
    return ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, alpha);
}

}  // namespace

const Palette& GetPalette(bool dark) {
    return dark ? kDarkPalette : kLightPalette;
}

void ApplyTheme(bool dark) {
    const Palette& p = GetPalette(dark);
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text] = ToImVec4(p.textPrimary);
    colors[ImGuiCol_TextDisabled] = ToImVec4(p.textMuted);
    colors[ImGuiCol_WindowBg] = ToImVec4(p.background);
    colors[ImGuiCol_ChildBg] = ToImVec4(p.background);
    colors[ImGuiCol_PopupBg] = ToImVec4(p.surface);
    colors[ImGuiCol_Border] = ToImVec4(p.border);
    colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg] = ToImVec4(p.surface);
    colors[ImGuiCol_FrameBgHovered] = ToImVec4(p.surfaceRaised);
    colors[ImGuiCol_FrameBgActive] = ToImVec4(p.surfaceRaised);
    colors[ImGuiCol_TitleBg] = ToImVec4(p.surface);
    colors[ImGuiCol_TitleBgActive] = ToImVec4(p.surface);
    colors[ImGuiCol_TitleBgCollapsed] = ToImVec4(p.surface);
    colors[ImGuiCol_MenuBarBg] = ToImVec4(p.surface);
    colors[ImGuiCol_ScrollbarBg] = ToImVec4(p.background);
    colors[ImGuiCol_ScrollbarGrab] = ToImVec4(p.surfaceRaised);
    colors[ImGuiCol_ScrollbarGrabHovered] = ToImVec4(p.border);
    colors[ImGuiCol_ScrollbarGrabActive] = ToImVec4(p.accentOwned);
    colors[ImGuiCol_CheckMark] = ToImVec4(p.accentOwned);
    colors[ImGuiCol_SliderGrab] = ToImVec4(p.accentOwned);
    colors[ImGuiCol_SliderGrabActive] = ToImVec4(p.accentJustChanged);
    colors[ImGuiCol_Button] = ToImVec4(p.surfaceRaised);
    colors[ImGuiCol_ButtonHovered] = ToImVec4(p.border);
    colors[ImGuiCol_ButtonActive] = ToImVec4(p.accentOwned);
    // Header drives Selectable's "selected" background (the sidebar's active
    // module) -- tinted with the same teal used for BoxState::Owned, so
    // "this is the active one" reads the same way in the sidebar and canvas.
    colors[ImGuiCol_Header] = ToImVec4(p.accentOwned, 0.30f);
    colors[ImGuiCol_HeaderHovered] = ToImVec4(p.accentOwned, 0.16f);
    colors[ImGuiCol_HeaderActive] = ToImVec4(p.accentOwned, 0.45f);
    colors[ImGuiCol_Separator] = ToImVec4(p.border);
    colors[ImGuiCol_SeparatorHovered] = ToImVec4(p.accentOwned);
    colors[ImGuiCol_SeparatorActive] = ToImVec4(p.accentOwned);
    colors[ImGuiCol_ResizeGrip] = ToImVec4(p.border);
    colors[ImGuiCol_ResizeGripHovered] = ToImVec4(p.accentOwned, 0.7f);
    colors[ImGuiCol_ResizeGripActive] = ToImVec4(p.accentOwned);
    colors[ImGuiCol_Tab] = ToImVec4(p.surface);
    colors[ImGuiCol_TabHovered] = ToImVec4(p.surfaceRaised);
    colors[ImGuiCol_TabSelected] = ToImVec4(p.surfaceRaised);
    colors[ImGuiCol_TabDimmed] = ToImVec4(p.background);
    colors[ImGuiCol_TabDimmedSelected] = ToImVec4(p.surface);
    colors[ImGuiCol_DockingPreview] = ToImVec4(p.accentOwned, 0.4f);
    colors[ImGuiCol_DockingEmptyBg] = ToImVec4(p.background);
    colors[ImGuiCol_PlotLines] = ToImVec4(p.accentOwned);
    colors[ImGuiCol_PlotHistogram] = ToImVec4(p.accentOwned);
    colors[ImGuiCol_TextSelectedBg] = ToImVec4(p.accentOwned, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ToImVec4(p.accentJustChanged);
    colors[ImGuiCol_NavCursor] = ToImVec4(p.accentOwned);

    style.WindowRounding = 8.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 6.0f;
    style.WindowPadding = ImVec2(16.0f, 16.0f);
    style.FramePadding = ImVec2(10.0f, 8.0f);
    style.ItemSpacing = ImVec2(10.0f, 10.0f);
    style.ItemInnerSpacing = ImVec2(8.0f, 6.0f);
    style.IndentSpacing = 20.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.GrabMinSize = 10.0f;
    // Removes the small collapse/docking-menu triangle from every panel's
    // tab/title bar -- pure default-ImGui chrome, no purpose in a fixed
    // 4-panel layout the user never collapses or re-docks by hand.
    style.WindowMenuButtonPosition = ImGuiDir_None;
}

}  // namespace underhood

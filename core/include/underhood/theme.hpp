#pragma once

#include "raylib.h"

namespace underhood {

// Single source of truth for color values shared between ImGui chrome
// (panels, text, borders) and Canvas's raylib-drawn simulation boxes, so the
// two never drift into two different "dark themes" by accident.
struct Palette {
    Color background;    // window/dockspace background
    Color surface;       // card/panel fill
    Color surfaceRaised;  // hovered/active card fill
    Color border;         // panel/card outline
    Color textPrimary;
    Color textMuted;
    Color accentEmpty;        // Canvas BoxState::Empty
    Color accentOwned;        // Canvas BoxState::Owned
    Color accentJustChanged;  // Canvas BoxState::JustChanged
};

const Palette& GetPalette(bool dark);

// Sets ImGuiStyle colors, rounding, and spacing to match GetPalette(dark).
// Call after ImGui context creation (e.g. after rlImGuiSetup), and again
// whenever the theme toggles.
void ApplyTheme(bool dark);

}  // namespace underhood

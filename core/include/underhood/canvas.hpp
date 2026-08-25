#pragma once

#include <string>

#include "raylib.h"

namespace underhood {

enum class BoxState { Empty, Owned, JustChanged };

// Thin wrapper around raylib drawing primitives used by simulation modules.
// Keeping raylib calls behind this interface means module render() code
// only ever names Canvas, not raylib types directly. Box/text colors adapt
// to the active theme (see setDarkTheme, and underhood::GetPalette which is
// the single source of truth Canvas and the ImGui chrome both read from);
// the launcher owns clearing the window/render-texture background
// separately, in sync with the same theme flag.
class Canvas {
public:
    void setDarkTheme(bool dark);
    bool isDarkTheme() const;

    // Optional custom font for drawBox/drawText. If never called (font.texture.id
    // == 0), falls back to raylib's built-in default font.
    void setFont(Font font);

    // alpha (0..1) fades the whole box (fill, border, and label) -- used by
    // operational modules to fade a box in/out over its insert/remove
    // animation. Defaults to fully opaque so existing callers are unaffected.
    void drawBox(int x, int y, int width, int height, const std::string& label, BoxState state,
                 float alpha = 1.0f) const;
    void drawArrow(int x1, int y1, int x2, int y2, BoxState state) const;
    void drawText(const std::string& text, int x, int y) const;

private:
    bool darkTheme_ = true;
    Font font_{};
};

}  // namespace underhood

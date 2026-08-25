#include "underhood/canvas.hpp"

#include "raylib.h"
#include "underhood/theme.hpp"

namespace underhood {

namespace {

Color accentFor(BoxState state, const Palette& p) {
    switch (state) {
        case BoxState::Empty:
            return p.accentEmpty;
        case BoxState::Owned:
            return p.accentOwned;
        case BoxState::JustChanged:
            return p.accentJustChanged;
    }
    return p.accentEmpty;
}

constexpr float kBoxCornerRadius = 0.25f;  // roundness param for DrawRectangleRounded
constexpr int kLabelFontSize = 18;

}  // namespace

void Canvas::setDarkTheme(bool dark) {
    darkTheme_ = dark;
}

bool Canvas::isDarkTheme() const {
    return darkTheme_;
}

void Canvas::setFont(Font font) {
    font_ = font;
}

void Canvas::drawBox(int x, int y, int width, int height, const std::string& label,
                      BoxState state) const {
    const Palette& p = GetPalette(darkTheme_);
    Color accent = accentFor(state, p);
    Rectangle rect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(width),
                   static_cast<float>(height)};

    // Card look: filled surface (raised, so it reads against the near-black
    // canvas background) with a thicker accent-colored border carrying the
    // semantic state (gray/teal/amber), instead of a bare outline floating
    // on the raw background.
    DrawRectangleRounded(rect, kBoxCornerRadius, 12, p.surfaceRaised);
    DrawRectangleRoundedLinesEx(rect, kBoxCornerRadius, 12, 2.5f, accent);

    if (font_.texture.id != 0) {
        DrawTextEx(font_, label.c_str(), Vector2{static_cast<float>(x + 10), static_cast<float>(y + 10)},
                   static_cast<float>(kLabelFontSize), 0.0f, p.textPrimary);
    } else {
        DrawText(label.c_str(), x + 10, y + 10, kLabelFontSize, p.textPrimary);
    }
}

void Canvas::drawArrow(int x1, int y1, int x2, int y2) const {
    const Palette& p = GetPalette(darkTheme_);
    DrawLineEx(Vector2{static_cast<float>(x1), static_cast<float>(y1)},
               Vector2{static_cast<float>(x2), static_cast<float>(y2)}, 2.0f, p.accentOwned);
    DrawCircle(x2, y2, 4, p.accentOwned);
}

void Canvas::drawText(const std::string& text, int x, int y) const {
    const Palette& p = GetPalette(darkTheme_);
    if (font_.texture.id != 0) {
        DrawTextEx(font_, text.c_str(), Vector2{static_cast<float>(x), static_cast<float>(y)},
                   static_cast<float>(kLabelFontSize), 0.0f, p.textPrimary);
    } else {
        DrawText(text.c_str(), x, y, kLabelFontSize, p.textPrimary);
    }
}

}  // namespace underhood

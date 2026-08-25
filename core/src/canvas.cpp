#include "underhood/canvas.hpp"

#include <cmath>

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

struct LabelSplit {
    std::string prefix;
    std::string value;
    bool found;
};

// Looks for a ": " or "-> " separator so labels like "a: 42" or "a -> 42"
// can be drawn as a small muted prefix over a larger value instead of one
// flat line -- a cheap typographic hierarchy win that needs no changes to
// the modules building these strings. Labels without either separator
// (bare numbers, "null", etc.) report found=false and render unchanged.
LabelSplit splitLabel(const std::string& label) {
    std::size_t colonPos = label.find(':');
    std::size_t arrowPos = label.find("->");
    std::size_t sepPos = std::string::npos;
    std::size_t sepLen = 0;
    if (colonPos != std::string::npos && (arrowPos == std::string::npos || colonPos < arrowPos)) {
        sepPos = colonPos;
        sepLen = 1;
    } else if (arrowPos != std::string::npos) {
        sepPos = arrowPos;
        sepLen = 2;
    }
    if (sepPos == std::string::npos) {
        return {"", "", false};
    }
    std::string prefix = label.substr(0, sepPos + sepLen);
    std::string value = label.substr(sepPos + sepLen);
    std::size_t firstNonSpace = value.find_first_not_of(' ');
    if (firstNonSpace != std::string::npos) {
        value = value.substr(firstNonSpace);
    }
    return {prefix, value, true};
}

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

void Canvas::drawBox(int x, int y, int width, int height, const std::string& label, BoxState state,
                      float alpha) const {
    const Palette& p = GetPalette(darkTheme_);
    Color accent = Fade(accentFor(state, p), alpha);
    Color surface = Fade(p.surfaceRaised, alpha);
    Color textColor = Fade(p.textPrimary, alpha);
    Color mutedColor = Fade(p.textMuted, alpha);
    Rectangle rect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(width),
                   static_cast<float>(height)};

    // Card look: filled surface (raised, so it reads against the near-black
    // canvas background) with a thicker accent-colored border carrying the
    // semantic state (gray/teal/amber), instead of a bare outline floating
    // on the raw background.
    DrawRectangleRounded(rect, kBoxCornerRadius, 12, surface);
    DrawRectangleRoundedLinesEx(rect, kBoxCornerRadius, 12, 2.5f, accent);

    if (font_.texture.id == 0) {
        DrawText(label.c_str(), x + 10, y + 10, kLabelFontSize, textColor);
        return;
    }

    LabelSplit split = splitLabel(label);
    if (!split.found) {
        DrawTextEx(font_, label.c_str(), Vector2{static_cast<float>(x + 10), static_cast<float>(y + 10)},
                   static_cast<float>(kLabelFontSize), 0.0f, textColor);
        return;
    }

    // Two-line hierarchy: small muted prefix ("a:"/"a ->") over a larger,
    // more prominent value -- the number is what a viewer actually scans
    // for, the prefix is just context.
    float prefixSize = static_cast<float>(kLabelFontSize) * 0.72f;
    float valueSize = static_cast<float>(kLabelFontSize) * 1.15f;
    DrawTextEx(font_, split.prefix.c_str(), Vector2{static_cast<float>(x + 10), static_cast<float>(y + 8)},
               prefixSize, 0.0f, mutedColor);
    DrawTextEx(font_, split.value.c_str(),
               Vector2{static_cast<float>(x + 10), static_cast<float>(y + 8) + prefixSize + 2.0f}, valueSize,
               0.0f, textColor);
}

void Canvas::drawArrow(int x1, int y1, int x2, int y2, BoxState state) const {
    const Palette& p = GetPalette(darkTheme_);
    Color accent = accentFor(state, p);
    Vector2 start{static_cast<float>(x1), static_cast<float>(y1)};
    Vector2 end{static_cast<float>(x2), static_cast<float>(y2)};
    DrawLineEx(start, end, 2.5f, accent);

    // Real arrowhead (a small filled triangle oriented along the line)
    // instead of a plain dot at the endpoint -- reads as "points to" rather
    // than just marking where the line stops.
    Vector2 dir{end.x - start.x, end.y - start.y};
    float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (length < 0.0001f) {
        DrawCircle(x2, y2, 5, accent);
        return;
    }
    dir.x /= length;
    dir.y /= length;
    Vector2 perp{-dir.y, dir.x};
    constexpr float kHeadLength = 11.0f;
    constexpr float kHeadWidth = 6.0f;
    Vector2 base{end.x - dir.x * kHeadLength, end.y - dir.y * kHeadLength};
    Vector2 left{base.x + perp.x * kHeadWidth, base.y + perp.y * kHeadWidth};
    Vector2 right{base.x - perp.x * kHeadWidth, base.y - perp.y * kHeadWidth};
    DrawTriangle(end, left, right, accent);
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

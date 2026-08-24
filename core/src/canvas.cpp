#include "underhood/canvas.hpp"

#include "raylib.h"

namespace underhood {

namespace {

// A deliberate, muted palette (not raylib's loud defaults): gray for
// "nothing here," a calm teal for "holds a value," and amber for "this
// just changed." Distinct enough to stay legible without relying on hue
// alone -- the label text always states the state in words too.
Color boxColor(BoxState state, bool dark) {
    switch (state) {
        case BoxState::Empty:
            return dark ? Color{110, 110, 110, 255} : Color{170, 170, 170, 255};
        case BoxState::Owned:
            return dark ? Color{86, 182, 194, 255} : Color{0, 121, 140, 255};
        case BoxState::JustChanged:
            return dark ? Color{230, 159, 0, 255} : Color{204, 102, 0, 255};
    }
    return dark ? Color{110, 110, 110, 255} : Color{170, 170, 170, 255};
}

Color textColor(bool dark) {
    return dark ? RAYWHITE : Color{30, 30, 30, 255};
}

}  // namespace

void Canvas::setDarkTheme(bool dark) {
    darkTheme_ = dark;
}

bool Canvas::isDarkTheme() const {
    return darkTheme_;
}

void Canvas::drawBox(int x, int y, int width, int height, const std::string& label,
                      BoxState state) const {
    Color color = boxColor(state, darkTheme_);
    Rectangle rect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(width),
                   static_cast<float>(height)};
    DrawRectangleLinesEx(rect, 2.0f, color);
    DrawText(label.c_str(), x + 8, y + 8, 16, textColor(darkTheme_));
}

void Canvas::drawArrow(int x1, int y1, int x2, int y2) const {
    Color color = textColor(darkTheme_);
    DrawLine(x1, y1, x2, y2, color);
    DrawCircle(x2, y2, 4, color);
}

void Canvas::drawText(const std::string& text, int x, int y) const {
    DrawText(text.c_str(), x, y, 16, textColor(darkTheme_));
}

}  // namespace underhood

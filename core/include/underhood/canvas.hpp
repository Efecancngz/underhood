#pragma once

#include <string>

namespace underhood {

enum class BoxState { Empty, Owned, JustChanged };

// Thin wrapper around raylib drawing primitives used by simulation modules.
// Keeping raylib calls behind this interface means module render() code
// only ever names Canvas, not raylib types directly. Box/text colors adapt
// to the active theme (see setDarkTheme); the launcher owns clearing the
// window background separately, in sync with the same theme flag.
class Canvas {
public:
    void setDarkTheme(bool dark);
    bool isDarkTheme() const;

    void drawBox(int x, int y, int width, int height, const std::string& label,
                 BoxState state) const;
    void drawArrow(int x1, int y1, int x2, int y2) const;
    void drawText(const std::string& text, int x, int y) const;

private:
    bool darkTheme_ = true;
};

}  // namespace underhood

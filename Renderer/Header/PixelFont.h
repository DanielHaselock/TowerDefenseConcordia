#pragma once
#include <SDL.h>
#include <array>
#include <cstdint>
#include <string>

namespace Renderer
{
    /**
     * @class PixelFont
     * @brief Lightweight 5x7 bitmap font renderer built directly on SDL_RenderFillRect.
     *
     * Because the Renderer intentionally avoids an external TTF font pipeline,
     * PixelFont renders crisp uppercase/ASCII text (labels, buttons, status
     * banners, and grid markers) at any integer pixel scale.
     */
    class PixelFont
    {
    public:
        static constexpr int GLYPH_COLS = 5; ///< Width of one unscaled glyph in pixels.
        static constexpr int GLYPH_ROWS = 7; ///< Height of one unscaled glyph in pixels.
        static constexpr int GLYPH_GAP = 1;  ///< Horizontal gap between glyphs in unscaled pixels.

        /**
         * @brief Returns the 7-row 5-bit bitmap for a character (lowercase maps to uppercase).
         * @param ch ASCII character to look up.
         * @return Array of 7 row bitmasks where bit 4 is the leftmost column and bit 0 is the rightmost.
         */
        static std::array<uint8_t, GLYPH_ROWS> glyphFor(char ch);

        /**
         * @brief Computes the pixel width of a string rendered at the given integer scale.
         */
        static int measureWidth(const std::string& text, int scale = 1);

        /**
         * @brief Computes the pixel height of a single line rendered at the given integer scale.
         */
        static int measureHeight(int scale = 1);

        /**
         * @brief Draws a single-line string onto an SDL_Renderer with top-left corner at (x, y).
         */
        static void drawText(
            SDL_Renderer* renderer,
            const std::string& text,
            int x,
            int y,
            int scale,
            SDL_Color color);

        /**
         * @brief Draws text centered both horizontally and vertically inside the given rectangle.
         */
        static void drawCenteredText(
            SDL_Renderer* renderer,
            const std::string& text,
            const SDL_Rect& bounds,
            int scale,
            SDL_Color color);
    };
}

#include "PixelFont.h"
#include <cctype>

namespace Renderer
{
    std::array<uint8_t, PixelFont::GLYPH_ROWS> PixelFont::glyphFor(char ch)
    {
        const char upper = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
        switch (upper)
        {
        case 'A': return {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
        case 'B': return {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110};
        case 'C': return {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110};
        case 'D': return {0b11100, 0b10010, 0b10001, 0b10001, 0b10001, 0b10010, 0b11100};
        case 'E': return {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111};
        case 'F': return {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000};
        case 'G': return {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01110};
        case 'H': return {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
        case 'I': return {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
        case 'J': return {0b00111, 0b00010, 0b00010, 0b00010, 0b10010, 0b10010, 0b01100};
        case 'K': return {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001};
        case 'L': return {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111};
        case 'M': return {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001};
        case 'N': return {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001};
        case 'O': return {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
        case 'P': return {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000};
        case 'Q': return {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101};
        case 'R': return {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001};
        case 'S': return {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110};
        case 'T': return {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
        case 'U': return {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
        case 'V': return {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100};
        case 'W': return {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001};
        case 'X': return {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001};
        case 'Y': return {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100};
        case 'Z': return {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111};
        case '0': return {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110};
        case '1': return {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
        case '2': return {0b01110, 0b10001, 0b00001, 0b00110, 0b01000, 0b10000, 0b11111};
        case '3': return {0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110};
        case '4': return {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010};
        case '5': return {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110};
        case '6': return {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110};
        case '7': return {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000};
        case '8': return {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110};
        case '9': return {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100};
        case ':': return {0b00000, 0b00100, 0b00100, 0b00000, 0b00100, 0b00100, 0b00000};
        case '-': return {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000};
        case '+': return {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000};
        case '/': return {0b00001, 0b00010, 0b00010, 0b00100, 0b01000, 0b01000, 0b10000};
        case '|': return {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
        case '(': return {0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010};
        case ')': return {0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000};
        case '[': return {0b01110, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01110};
        case ']': return {0b01110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01110};
        case '!': return {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100};
        case '.': return {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00100};
        case ',': return {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00100, 0b01000};
        case '_': return {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111};
        case '<': return {0b00010, 0b00100, 0b01000, 0b10000, 0b01000, 0b00100, 0b00010};
        case '>': return {0b01000, 0b00100, 0b00010, 0b00001, 0b00010, 0b00100, 0b01000};
        default:  return {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};
        }
    }

    int PixelFont::measureWidth(const std::string& text, int scale)
    {
        if (text.empty() || scale <= 0)
        {
            return 0;
        }
        const int count = static_cast<int>(text.size());
        return count * GLYPH_COLS * scale + (count - 1) * GLYPH_GAP * scale;
    }

    int PixelFont::measureHeight(int scale)
    {
        return scale > 0 ? GLYPH_ROWS * scale : 0;
    }

    void PixelFont::drawText(
        SDL_Renderer* renderer,
        const std::string& text,
        int x,
        int y,
        int scale,
        SDL_Color color)
    {
        if (renderer == nullptr || text.empty() || scale <= 0)
        {
            return;
        }

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

        const int advance = (GLYPH_COLS + GLYPH_GAP) * scale;
        for (size_t idx = 0; idx < text.size(); ++idx)
        {
            const auto glyph = glyphFor(text[idx]);
            const int charX = x + static_cast<int>(idx) * advance;

            for (int row = 0; row < GLYPH_ROWS; ++row)
            {
                const uint8_t bits = glyph[row];
                if (bits == 0)
                {
                    continue;
                }
                for (int col = 0; col < GLYPH_COLS; ++col)
                {
                    if ((bits >> (GLYPH_COLS - 1 - col)) & 1)
                    {
                        SDL_Rect pixel{
                            charX + col * scale,
                            y + row * scale,
                            scale,
                            scale
                        };
                        SDL_RenderFillRect(renderer, &pixel);
                    }
                }
            }
        }
    }

    void PixelFont::drawCenteredText(
        SDL_Renderer* renderer,
        const std::string& text,
        const SDL_Rect& bounds,
        int scale,
        SDL_Color color)
    {
        const int w = measureWidth(text, scale);
        const int h = measureHeight(scale);
        const int x = bounds.x + (bounds.w - w) / 2;
        const int y = bounds.y + (bounds.h - h) / 2;
        drawText(renderer, text, x, y, scale, color);
    }
}

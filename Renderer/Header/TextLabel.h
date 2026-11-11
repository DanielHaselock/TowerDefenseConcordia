#pragma once
#include "Texture.h"
#include <string>

namespace Renderer
{
    /**
     * @class TextLabel
     * @brief Draws a fixed caption (e.g. "COINS", "HP", "WAVE") using PixelFont.
     *
     * Subclasses Texture purely so Window::render()'s dynamic_cast dispatch
     * (Renderer/Source/Window.cpp) picks it up for free; no image is ever
     * loaded, Render() is overridden to draw text instead.
     */
    class TextLabel : public Texture
    {
    public:
        TextLabel(int x, int y, std::string text, int scale, SDL_Color color);

        void Render(::SDL_Renderer* p_renderer) override;

        /// Changes the displayed caption.
        void setText(std::string text);

        /// Pixel width of the caption at its current scale.
        int getWidth() const;

        /// Pixel height of the caption at its current scale.
        int getHeight() const;

    private:
        std::string m_text;
        int m_scale;
        SDL_Color m_color;
    };
}

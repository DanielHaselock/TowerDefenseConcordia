#include "TextLabel.h"
#include "PixelFont.h"

namespace Renderer
{
    TextLabel::TextLabel(int x, int y, std::string text, int scale, SDL_Color color)
        : Texture(x, y, PixelFont::measureWidth(text, scale), PixelFont::measureHeight(scale)),
          m_text(std::move(text)),
          m_scale(scale),
          m_color(color)
    {
    }

    void TextLabel::Render(::SDL_Renderer* p_renderer)
    {
        if (p_renderer == nullptr)
        {
            return;
        }
        const SDL_Rect bounds = getScale();
        PixelFont::drawText(p_renderer, m_text, bounds.x, bounds.y, m_scale, m_color);
    }

    void TextLabel::setText(std::string text)
    {
        m_text = std::move(text);
        const SDL_Rect bounds = getScale();
        setScale(PixelFont::measureWidth(m_text, m_scale), PixelFont::measureHeight(m_scale));
        setPosition(bounds.x, bounds.y);
    }

    int TextLabel::getWidth() const
    {
        return PixelFont::measureWidth(m_text, m_scale);
    }

    int TextLabel::getHeight() const
    {
        return PixelFont::measureHeight(m_scale);
    }
}

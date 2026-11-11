#include "OutcomeOverlay.h"
#include "PixelFont.h"
#include "UiTheme.h"

namespace Renderer
{
    OutcomeOverlay::OutcomeOverlay(int x, int y, int w, int h, bool won)
        : Texture(x, y, w, h),
          m_won(won)
    {
    }

    void OutcomeOverlay::Render(::SDL_Renderer* p_renderer)
    {
        if (p_renderer == nullptr)
        {
            return;
        }

        const SDL_Rect bounds = getScale();

        SDL_SetRenderDrawBlendMode(p_renderer, SDL_BLENDMODE_BLEND);

        UiTheme::fillRect(p_renderer, bounds, SDL_Color{UiTheme::BackgroundDark.r, UiTheme::BackgroundDark.g, UiTheme::BackgroundDark.b, 215});

        const SDL_Color titleColor = m_won ? UiTheme::AccentGreen : UiTheme::AccentRed;
        const std::string title = m_won ? "YOU WIN" : "GAME OVER";

        const SDL_Rect titleBounds{bounds.x, bounds.y + bounds.h / 2 - 40, bounds.w, PixelFont::measureHeight(4)};
        PixelFont::drawCenteredText(p_renderer, title, titleBounds, 4, titleColor);

        const SDL_Rect subtitleBounds{bounds.x, bounds.y + bounds.h / 2 + 20, bounds.w, PixelFont::measureHeight(1)};
        PixelFont::drawCenteredText(p_renderer, "CLICK TO CONTINUE", subtitleBounds, 1, UiTheme::TextMuted);

        SDL_SetRenderDrawBlendMode(p_renderer, SDL_BLENDMODE_NONE);
    }
}

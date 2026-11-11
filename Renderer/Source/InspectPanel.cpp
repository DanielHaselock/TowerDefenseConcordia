#include "InspectPanel.h"
#include "PixelFont.h"
#include "UiTheme.h"

namespace Renderer
{
    InspectPanel::InspectPanel(int x, int y, int w, int h, std::string title, SDL_Color accent,
        std::vector<Row> rows, SDL_Point decimalPointOffset)
        : Texture(x, y, w, h),
          m_title(std::move(title)),
          m_accent(accent),
          m_rows(std::move(rows)),
          m_decimalPointOffset(decimalPointOffset)
    {
    }

    void InspectPanel::Render(::SDL_Renderer* p_renderer)
    {
        if (p_renderer == nullptr)
        {
            return;
        }

        const SDL_Rect bounds = getScale();

        UiTheme::fillRect(p_renderer, bounds, UiTheme::PanelDark);
        UiTheme::strokeRect(p_renderer, bounds, m_accent, 2);

        const SDL_Rect titleBounds{bounds.x, bounds.y + 6, bounds.w, PixelFont::measureHeight(1)};
        PixelFont::drawCenteredText(p_renderer, m_title, titleBounds, 1, m_accent);

        UiTheme::fillRect(p_renderer, {bounds.x + 8, bounds.y + 26, bounds.w - 16, 1}, UiTheme::BorderMuted);

        // These y-offsets mirror the fixed positions GameManager places the
        // corresponding NumberLabels at (panelY + each row's own offset).
        for (const Row& row : m_rows)
        {
            PixelFont::drawText(p_renderer, row.label, bounds.x + 10, bounds.y + row.yOffset, 1, UiTheme::TextMuted);
        }

        // Decimal point between two value columns (e.g. the tower rate-of-fire
        // row's whole and fraction labels); omitted when x is negative.
        if (m_decimalPointOffset.x >= 0)
        {
            UiTheme::fillRect(p_renderer,
                {bounds.x + m_decimalPointOffset.x, bounds.y + m_decimalPointOffset.y, 2, 2},
                UiTheme::TextPrimary);
        }
    }
}

#include "StyledButton.h"
#include "PixelFont.h"
#include "UiTheme.h"

namespace Renderer
{
    StyledButton::StyledButton(int x, int y, int w, int h, std::string label, ButtonVariant variant)
        : Texture(x, y, w, h),
          m_label(std::move(label)),
          m_variant(variant)
    {
    }

    void StyledButton::setHovered(bool hovered)
    {
        m_hovered = hovered;
    }

    void StyledButton::setCompanionLineHeight(int heightPx)
    {
        m_companionLineHeight = heightPx > 0 ? heightPx : 0;
    }

    namespace
    {
        // Top of the label+value block, so the two lines together sit centered
        // in the button rather than the label centering itself and pushing the
        // value into the lower half.
        int blockTopOffset(int buttonHeight, int companionHeight)
        {
            const int blockHeight =
                PixelFont::measureHeight(1) + StyledButton::kCompanionLineGapPx + companionHeight;
            return (buttonHeight - blockHeight) / 2;
        }
    }

    int StyledButton::companionLineOffsetY(int buttonHeight, int companionHeight)
    {
        return blockTopOffset(buttonHeight, companionHeight) +
            PixelFont::measureHeight(1) + kCompanionLineGapPx;
    }

    void StyledButton::Render(::SDL_Renderer* p_renderer)
    {
        if (p_renderer == nullptr)
        {
            return;
        }

        SDL_Color bg;
        SDL_Color border;
        switch (m_variant)
        {
        case ButtonVariant::Primary:
            bg = m_hovered ? UiTheme::PrimaryBgHover : UiTheme::PrimaryBg;
            border = UiTheme::PrimaryBorder;
            break;
        case ButtonVariant::Success:
            bg = m_hovered ? UiTheme::SuccessBgHover : UiTheme::SuccessBg;
            border = UiTheme::SuccessBorder;
            break;
        case ButtonVariant::Danger:
            bg = m_hovered ? UiTheme::DangerBgHover : UiTheme::DangerBg;
            border = UiTheme::DangerBorder;
            break;
        case ButtonVariant::Neutral:
        default:
            bg = m_hovered ? UiTheme::NeutralBgHover : UiTheme::NeutralBg;
            border = UiTheme::NeutralBorder;
            break;
        }

        const SDL_Rect bounds = getScale();
        UiTheme::fillRect(p_renderer, bounds, bg);
        UiTheme::strokeRect(p_renderer, bounds, border, 2);

        if (m_companionLineHeight > 0)
        {
            // The caller draws a value on the line below (see companionLineOffsetY),
            // so the label takes the top line of a centered two-line block instead
            // of centering across the full button height.
            const int textW = PixelFont::measureWidth(m_label, 1);
            const int textX = bounds.x + (bounds.w - textW) / 2;
            const int textY = bounds.y + blockTopOffset(bounds.h, m_companionLineHeight);
            PixelFont::drawText(p_renderer, m_label, textX, textY, 1, UiTheme::TextPrimary);
        }
        else
        {
            PixelFont::drawCenteredText(p_renderer, m_label, bounds, 1, UiTheme::TextPrimary);
        }
    }
}

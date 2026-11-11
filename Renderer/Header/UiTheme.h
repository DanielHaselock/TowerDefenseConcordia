#pragma once
#include <SDL.h>

namespace Renderer
{
    /**
     * @namespace UiTheme
     * @brief Shared navy/slate/gold palette and small drawing helpers used by every
     * hand-drawn UI surface in the game (the Map Editor and the in-game HUD).
     *
     * Centralizing these here (rather than duplicating them per screen) is what
     * keeps the editor and the gameplay HUD visually identical.
     */
    namespace UiTheme
    {
        constexpr SDL_Color BackgroundDark{20, 29, 38, 255};
        constexpr SDL_Color PanelDark{33, 47, 60, 255};
        constexpr SDL_Color PanelMed{48, 68, 86, 255};
        constexpr SDL_Color PanelLight{68, 94, 118, 255};
        constexpr SDL_Color BorderMuted{54, 76, 96, 255};
        constexpr SDL_Color BorderStrong{96, 128, 158, 255};

        constexpr SDL_Color AccentGold{255, 215, 0, 255};
        constexpr SDL_Color AccentBlue{108, 184, 238, 255};
        constexpr SDL_Color AccentRed{245, 92, 92, 255};
        constexpr SDL_Color AccentGreen{116, 240, 158, 255};

        constexpr SDL_Color TextPrimary{240, 246, 252, 255};
        constexpr SDL_Color TextMuted{156, 184, 206, 255};

        constexpr SDL_Color SuccessBg{36, 146, 78, 255};
        constexpr SDL_Color SuccessBgHover{46, 176, 96, 255};
        constexpr SDL_Color SuccessBorder{116, 240, 158, 255};

        constexpr SDL_Color DangerBg{68, 30, 36, 255};
        constexpr SDL_Color DangerBgHover{84, 38, 44, 255};
        constexpr SDL_Color DangerBorder{214, 86, 92, 255};

        constexpr SDL_Color PrimaryBg{42, 96, 140, 255};
        constexpr SDL_Color PrimaryBgHover{56, 122, 174, 255};
        constexpr SDL_Color PrimaryBorder{108, 184, 238, 255};

        constexpr SDL_Color NeutralBg{50, 66, 82, 255};
        constexpr SDL_Color NeutralBgHover{68, 88, 108, 255};
        constexpr SDL_Color NeutralBorder{104, 128, 152, 255};

        /// Returns true if (x, y) falls inside r (left/top inclusive, right/bottom exclusive).
        bool pointInRect(int x, int y, const SDL_Rect& r);

        /// Fills r with the given color on the given renderer.
        void fillRect(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color c);

        /// Draws a (possibly multi-pixel-thick) border tracing the inside edge of rect.
        void strokeRect(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color c, int thickness = 1);
    }
}

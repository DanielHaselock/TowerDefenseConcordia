#pragma once
#include "Texture.h"

namespace Renderer
{
    /**
     * @class OutcomeOverlay
     * @brief Full-screen win/lose banner shown once a game session ends.
     *
     * Draws a translucent dark fill (the frozen board stays faintly visible
     * underneath, matching the Map Editor's own SDL_BLENDMODE_BLEND usage)
     * plus a large centered title and a "click to continue" subtitle.
     */
    class OutcomeOverlay : public Texture
    {
    public:
        OutcomeOverlay(int x, int y, int w, int h, bool won);

        void Render(::SDL_Renderer* p_renderer) override;

    private:
        bool m_won;
    };
}

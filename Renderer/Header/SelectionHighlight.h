#pragma once
#include "Texture.h"

namespace Renderer
{
    /**
     * @class SelectionHighlight
     * @brief Border-only gold outline marking the currently selected tower's tile.
     *
     * Draws no fill so the tile sprite underneath stays visible, unlike a
     * StyledButton. Subclasses Texture purely to slot into Window::render().
     */
    class SelectionHighlight : public Texture
    {
    public:
        SelectionHighlight(int x, int y, int w, int h);

        void Render(::SDL_Renderer* p_renderer) override;
    };
}

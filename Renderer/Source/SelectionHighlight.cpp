#include "SelectionHighlight.h"
#include "UiTheme.h"

namespace Renderer
{
    SelectionHighlight::SelectionHighlight(int x, int y, int w, int h)
        : Texture(x, y, w, h)
    {
    }

    void SelectionHighlight::Render(::SDL_Renderer* p_renderer)
    {
        if (p_renderer == nullptr)
        {
            return;
        }
        UiTheme::strokeRect(p_renderer, getScale(), UiTheme::AccentGold, 3);
    }
}

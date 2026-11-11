#include "UiTheme.h"

namespace Renderer
{
    namespace UiTheme
    {
        bool pointInRect(int x, int y, const SDL_Rect& r)
        {
            return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
        }

        void fillRect(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color c)
        {
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            SDL_RenderFillRect(renderer, &rect);
        }

        void strokeRect(SDL_Renderer* renderer, const SDL_Rect& rect, SDL_Color c, int thickness)
        {
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            for (int i = 0; i < thickness; ++i)
            {
                SDL_Rect r{rect.x + i, rect.y + i, rect.w - 2 * i, rect.h - 2 * i};
                if (r.w > 0 && r.h > 0)
                {
                    SDL_RenderDrawRect(renderer, &r);
                }
            }
        }
    }
}

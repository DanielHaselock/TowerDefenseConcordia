#pragma once
#include "Texture.h"
#include <string>
#include <vector>

namespace Renderer
{
    /**
     * @class InspectPanel
     * @brief Procedurally-drawn background for a tower/critter inspection panel.
     *
     * Draws the panel frame, a title, and a caller-supplied list of row
     * captions (e.g. "LVL", "RANGE", ... for towers; "LEVEL", "HP", ... for
     * critters), plus an optional decimal point between two value columns
     * (used by the tower rate-of-fire row). The numeric values themselves
     * are drawn by separate NumberLabel instances positioned by
     * Driver/Source/GameManager.cpp at fixed offsets this panel's captions
     * are laid out to match.
     */
    class InspectPanel : public Texture
    {
    public:
        /// One caption row drawn on the panel; yOffset is measured from the panel's top edge.
        struct Row
        {
            std::string label;
            int yOffset;
        };

        /**
         * @param rows Caption rows drawn down the left side of the panel.
         * @param decimalPointOffset Panel-local (x, y) of an optional decimal point
         * drawn between two value columns; omit (default) for no decimal point.
         */
        InspectPanel(int x, int y, int w, int h, std::string title, SDL_Color accent,
            std::vector<Row> rows, SDL_Point decimalPointOffset = SDL_Point{-1, -1});

        void Render(::SDL_Renderer* p_renderer) override;

    private:
        std::string m_title;
        SDL_Color m_accent;
        std::vector<Row> m_rows;
        SDL_Point m_decimalPointOffset;
    };
}

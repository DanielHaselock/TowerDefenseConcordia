#pragma once
#include "Texture.h"
#include <string>

namespace Renderer
{
    /// Color treatment applied to a StyledButton, echoing the Map Editor's own buttons.
    enum class ButtonVariant
    {
        Primary, ///< Blue - main/affirmative action (e.g. Upgrade).
        Success, ///< Green - starts/advances the game (e.g. Start).
        Danger,  ///< Dark red - dismissive action (e.g. Cancel).
        Neutral  ///< Slate - secondary action (e.g. Sell).
    };

    /**
     * @class StyledButton
     * @brief A procedurally-drawn button: filled rect + border + centered PixelFont label.
     *
     * Subclasses Texture so Window::render()'s dynamic_cast dispatch picks it
     * up for free, and so Renderer::Button (whose click/show-hide logic is
     * typed Texture*) can wrap it exactly like it would a PNG-backed Texture.
     */
    class StyledButton : public Texture
    {
    public:
        StyledButton(int x, int y, int w, int h, std::string label, ButtonVariant variant);

        void Render(::SDL_Renderer* p_renderer) override;

        /// Updates the hover visual state; callers compare live mouse position against getScale().
        void setHovered(bool hovered);

        /**
         * @brief Reserves a line under the label for a companion value the caller
         * draws itself (e.g. the price under "UPGRADE", the wave number under "START").
         *
         * With a companion line the label is no longer centered on its own: label
         * and value are laid out as one vertically-centered block, so the pair reads
         * as balanced inside the button. Callers place the value at
         * companionLineOffsetY() below the button's top edge.
         *
         * @param heightPx Height of the companion value, or 0 for a label-only button.
         */
        void setCompanionLineHeight(int heightPx);

        /// Vertical gap (px) between the label and the companion value below it.
        static constexpr int kCompanionLineGapPx = 3;

        /**
         * @brief Offset (px) from the button's top edge to the top of its companion value.
         * @param buttonHeight Height of the button the value is drawn on.
         * @param companionHeight Height of the companion value itself.
         */
        static int companionLineOffsetY(int buttonHeight, int companionHeight);

    private:
        std::string m_label;
        ButtonVariant m_variant;
        bool m_hovered = false;
        int m_companionLineHeight = 0;
    };
}

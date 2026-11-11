#pragma once
#include "Observer.h"
#include "Player.h"
#include "Window.h"

namespace App
{
    /**
     * @class PlayerRenderObserver
     * @brief Displays the player's coin balance and health as number labels:
     * health flush in the bottom-left corner, coins in the empty space
     * beside the upgrade button.
     *
     * Player is the Observable subject: every ChangeCoins()/ChangeHealth()
     * notify() call runs update(), which rewrites both labels (only one
     * value actually changed, but re-drawing both is cheap and keeps this
     * observer simple). The labels are created through
     * Window::addNumberLabel(), so the Window owns them; this observer only
     * keeps pointers to them.
     */
    class PlayerRenderObserver : public Game::Observer<Player>
    {
    public:
        /** @brief Creates the player coin/health labels and subscribes to player updates. */
        PlayerRenderObserver(Player& player, ::Renderer::Window& window);

        /** @brief Refreshes both labels when the player's coins or health change. */
        void update(const Player& player) override;

    private:
        /// Coin counter drawn by the Window. Not owned here.
        ::Renderer::NumberLabel* coinLabel;

        /// Health counter drawn by the Window. Not owned here.
        ::Renderer::NumberLabel* healthLabel;

        /// "COINS" caption, drawn above coinLabel. Not owned here.
        ::Renderer::TextLabel* coinLabelText;

        /// "HP" caption, drawn immediately left of healthLabel. Not owned here.
        ::Renderer::TextLabel* healthLabelText;

        /// Re-anchors both labels and their text captions to their fixed positions.
        void placeLabels();
    };
}

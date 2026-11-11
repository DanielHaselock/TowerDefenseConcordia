#pragma once
#include "Observer.h"
#include "Tower.h"
#include "GameRenderer.h"

namespace App
{
    /**
     * @class TowerRenderSyncObserver
     * @brief Reacts to a Tower's state changes by swapping in the next-level
     * turret sprite on the tile it's built on.
     *
     * update() only receives a const Tower&, so it cannot call
     * Tower::setTexture() itself (that would mutate the Subject). It stores
     * the sprite GameRenderer::upgradeTurret() just revealed so the caller
     * (GameManager) can apply it back onto the Tower right after triggering
     * the upgrade - see getLastUpgradedTexture().
     */
    class TowerRenderSyncObserver : public Game::Observer<Game::Tower>
    {
    public:
        /** @brief Observes tower upgrades and stores the renderer used for their sprites. */
        TowerRenderSyncObserver(
            Game::Tower& tower,
            ::Renderer::GameRenderer& gameRenderer,
            int mapColumns = NUMB_COLUMNS);

        /** @brief Updates the displayed tower sprite after a tower state change. */
        void update(const Game::Tower& tower) override;

        /// The sprite produced by the most recent update() call.
        ::Renderer::spriteSheetTexture* getLastUpgradedTexture() const { return lastUpgradedTexture; }

    private:
        ::Renderer::GameRenderer& gameRenderer;
        int mapColumns;
        ::Renderer::spriteSheetTexture* lastUpgradedTexture = nullptr;
    };
}

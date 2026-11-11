#pragma once
#include "Observer.h"
#include "Map.h"
#include "GameRenderer.h"

namespace App
{
    /**
     * @class MapRenderSyncObserver
     * @brief Reacts to a Map's state changes by syncing the renderer's
     * turret sprites to match - showing one on every cell with a tower,
     * hiding it on every cell without - so GameManager no longer has to
     * call placeTurret()/removeTurret() itself next to every
     * placeTower()/removeTower() call.
     */
    class MapRenderSyncObserver : public Game::Observer<Game::Map>
    {
    public:
        /** @brief Registers for map changes and stores the renderer used to display them. */
        MapRenderSyncObserver(Game::Map& map, ::Renderer::GameRenderer& gameRenderer);

        /** @brief Synchronizes displayed tower sprites with the current map cells. */
        void update(const Game::Map& map) override;

    private:
        ::Renderer::GameRenderer& gameRenderer;
    };
}

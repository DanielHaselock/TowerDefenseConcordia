#include "MapRenderSyncObserver.h"
#include "DirectDammageTower.h"
#include "AoeTower.h"
#include "SlowTower.h"

namespace
{
    // Each tile pre-creates 9 turret sprites: 3 colors x 3 levels (see
    // RendererMap's constructor). Color 0/1/2 = DirectDammageTower/
    // AoeTower/SlowTower, matching GameManager::HandleMouseRelease's use of
    // getTurretTextures()[0]/[3]/[6] for each type's level-1 sprite.
    int colorIndexFor(const Game::Tower& tower)
    {
        if (dynamic_cast<const Game::DirectDammageTower*>(&tower)) return 0;
        if (dynamic_cast<const Game::AoeTower*>(&tower)) return 1;
        if (dynamic_cast<const Game::SlowTower*>(&tower)) return 2;
        return 0;
    }
}

App::MapRenderSyncObserver::MapRenderSyncObserver(
    Game::Map& map,
    ::Renderer::GameRenderer& gameRenderer)
    : Game::Observer<Game::Map>(map), gameRenderer(gameRenderer)
{
}

void App::MapRenderSyncObserver::update(const Game::Map& map)
{
    for (int row = 0; row < map.getRows(); ++row)
    {
        for (int col = 0; col < map.getCols(); ++col)
        {
            const Game::Cell& cell = map.getCell(row, col);
            auto* tile = gameRenderer.getGenerator()->getTiles()[row * map.getCols() + col];

            if (cell.hasTower && cell.tower != nullptr)
            {
                int turretType = colorIndexFor(*cell.tower) * 3 + cell.tower->getCurrentLevel();
                tile->PlaceTurret(turretType);
            }
            else
            {
                for (auto* turretTexture : tile->getTurretTextures())
                {
                    turretTexture->render = false;
                }
            }
        }
    }
}

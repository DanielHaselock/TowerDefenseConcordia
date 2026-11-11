#include "TowerRenderSyncObserver.h"

App::TowerRenderSyncObserver::TowerRenderSyncObserver(
    Game::Tower& tower,
    ::Renderer::GameRenderer& gameRenderer,
    int mapColumns)
    : Game::Observer<Game::Tower>(tower),
      gameRenderer(gameRenderer),
      mapColumns(mapColumns)
{
}

void App::TowerRenderSyncObserver::update(const Game::Tower& tower)
{
    lastUpgradedTexture = gameRenderer.upgradeTurret(tower.getRow() * mapColumns + tower.getCol());
}

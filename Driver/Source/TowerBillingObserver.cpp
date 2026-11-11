#include "TowerBillingObserver.h"

App::TowerBillingObserver::TowerBillingObserver(Game::Tower& tower, Player& player)
    : Game::Observer<Game::Tower>(tower), player(player)
{
}

void App::TowerBillingObserver::update(const Game::Tower& tower)
{
    player.ChangeCoins(-tower.getCurrentLevelCost());
}

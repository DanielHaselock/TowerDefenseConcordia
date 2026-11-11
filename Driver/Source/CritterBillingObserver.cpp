#include "CritterBillingObserver.h"

App::CritterBillingObserver::CritterBillingObserver(Game::Critter& critter, Player& player)
    : Game::Observer<Game::Critter>(critter), player(player)
{
}

void App::CritterBillingObserver::update(const Game::Critter& critter)
{
    if (critter.killed)
    {
        player.ChangeCoins(critter.getReward());
    }
    else if (critter.reachedEnd)
    {
        if (!healthPenaltyApplied)
        {
            healthPenaltyApplied = true;
            player.ChangeHealth(-1);
        }

        // getStrength() is a per-second rate; scale it by the elapsed time
        // of the Update() call that triggered this notify() so it drains
        // at the right rate regardless of framerate.
        player.ChangeCoins(-critter.getStrength() * critter.getLastDeltaTime());
    }
    else
    {
        healthPenaltyApplied = false;
    }
}

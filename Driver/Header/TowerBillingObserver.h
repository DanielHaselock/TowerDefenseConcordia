#pragma once
#include "Observer.h"
#include "Tower.h"
#include "Player.h"

namespace App
{
    /**
     * @class TowerBillingObserver
     * @brief Reacts to a Tower's state changes by charging the upgrade's
     * cost to the Player, so GameManager no longer applies that side effect
     * itself - it just triggers the upgrade and lets the attached observers
     * react to it.
     */
    class TowerBillingObserver : public Game::Observer<Game::Tower>
    {
    public:
        /** @brief Registers with the tower and keeps a reference to the player being charged. */
        TowerBillingObserver(Game::Tower& tower, Player& player);

        /** @brief Charges the player when the observed tower is upgraded. */
        void update(const Game::Tower& tower) override;

    private:
        Player& player;
    };
}

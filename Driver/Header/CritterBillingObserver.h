#pragma once
#include "Observer.h"
#include "Critter.h"
#include "Player.h"

namespace App
{
    /**
     * @class CritterBillingObserver
     * @brief Reacts to a Critter's state changes by crediting or charging
     * the Player, so GameManager no longer applies these side effects
     * itself - it just drives the critter's lifecycle (Update()/Death())
     * and lets the attached observers react to it.
     *
     * - Rewards the player once when a critter is killed (killed's
     *   false->true transition, notified from Critter::takeDamage()).
     * - Costs one health point the instant a critter reaches the exit.
     * - Charges the player coins once per frame while a critter idles at
     *   the exit (notified from Critter::Update()), until Death() resets it.
     */
    class CritterBillingObserver : public Game::Observer<Game::Critter>
    {
    public:
        /** @brief Observes the critter and stores the player whose coins/health are affected. */
        CritterBillingObserver(Game::Critter& critter, Player& player);

        /** @brief Applies coin/health rewards or losses in response to critter state changes. */
        void update(const Game::Critter& critter) override;

    private:
        Player& player;

        /// Guards against charging health more than once per trip to the exit.
        bool healthPenaltyApplied = false;
    };
}

#pragma once
#include "Observer.h"
#include "Critter.h"

namespace App
{
    /**
     * @class CritterRenderSyncObserver
     * @brief Mirrors a Critter's movement and spawn/death state onto its
     * sprite, so the game model never touches the renderer itself.
     *
     * Critter is the Observable subject: it notifies on activation,
     * deactivation, every Update() step, and reset after death. Each
     * notify() lands here, where the critter's current pixel position and
     * active flag are copied onto the spriteSheetTexture it was built with.
     */
    class CritterRenderSyncObserver : public Game::Observer<Game::Critter>
    {
    public:
        /** @brief Subscribes to changes in the given critter. */
        CritterRenderSyncObserver(Game::Critter& critter);

        /** @brief Synchronizes the critter sprite position and visibility with its state. */
        void update(const Game::Critter& critter) override;
    };
}

#include "CritterLoggingObserver.h"
#include <iostream>

namespace Game {

    void CritterLoggingObserver::update(const Critter& critter)
    {
        // Update() notifies once per frame while the critter is active;
        // print only when the state shown in the line has changed.
        if (critter.getRow() == lastRow && critter.getCol() == lastCol &&
            critter.getHitPoints() == lastHitPoints &&
            critter.getIsActive() == lastActive)
        {
            return;
        }

        lastRow = critter.getRow();
        lastCol = critter.getCol();
        lastHitPoints = critter.getHitPoints();
        lastActive = critter.getIsActive();

        std::cout << "[CritterLoggingObserver] " << critter.getType()
                   << " (" << static_cast<const void*>(&critter) << ")"
                   << " at (" << critter.getRow() << ", " << critter.getCol() << ") "
                   << "-> HP " << critter.getHitPoints() << "/" << critter.getMaxHitPoints()
                   << " | active: " << (critter.getIsActive() ? "yes" : "no")
                   << '\n';
    }

} // namespace Game

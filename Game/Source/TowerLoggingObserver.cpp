#include "TowerLoggingObserver.h"
#include <iostream>

namespace Game {

    void TowerLoggingObserver::update(const Tower& tower)
    {
        std::cout << "[TowerLoggingObserver] Tower at (" << tower.getRow() << ", " << tower.getCol() << ") "
                   << "-> level " << (tower.getCurrentLevel() + 1)
                   << " | range " << tower.getRange()
                   << " | power " << tower.getPower()
                   << " | rate of fire " << tower.getRateOfFire()
                   << '\n';
    }

} // namespace Game

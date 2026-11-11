#pragma once
#include "Observer.h"
#include "Tower.h"

namespace Game {

    /**
     * @class TowerLoggingObserver
     * @brief Prints a Tower's current state to standard output whenever it changes.
     */
    class TowerLoggingObserver : public Observer<Tower> {
    public:
        /** @brief Subscribes to changes in the given tower. */
        explicit TowerLoggingObserver(Tower& tower) : Observer<Tower>(tower) {}

        /** @brief Logs the tower after a state change. */
        void update(const Tower& tower) override;
    };

} // namespace Game

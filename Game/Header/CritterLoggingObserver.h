#pragma once
#include "Observer.h"
#include "Critter.h"

namespace Game {

    /**
     * @class CritterLoggingObserver
     * @brief Prints a Critter's current state to standard output whenever it
     * changes.
     *
     * Critter::Update() notifies every frame while a critter walks, so the
     * observer remembers the last state it logged and only prints when a
     * field shown in the line (tile, hit points, active flag) changes
     */
    class CritterLoggingObserver : public Observer<Critter> {
    public:
        /** @brief Subscribes to changes in the given critter. */
        explicit CritterLoggingObserver(Critter& critter) : Observer<Critter>(critter) {}

        /** @brief Logs relevant critter changes while avoiding repeated identical messages. */
        void update(const Critter& critter) override;

    private:
        int lastRow = -1;      ///< Tile row of the previously logged state.
        int lastCol = -1;      ///< Tile column of the previously logged state.
        int lastHitPoints = -1; ///< Hit points of the previously logged state.
        bool lastActive = false; ///< Active flag of the previously logged state.
    };

} // namespace Game

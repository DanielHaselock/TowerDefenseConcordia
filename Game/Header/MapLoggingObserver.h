#pragma once
#include "Observer.h"
#include "Map.h"

namespace Game {

    /**
     * @class MapLoggingObserver
     * @brief Prints a text view of a Map's grid to standard output whenever it changes.
     */
    class MapLoggingObserver : public Observer<Map> {
    public:
        /** @brief Subscribes to changes in the given map. */
        explicit MapLoggingObserver(Map& map) : Observer<Map>(map) {}

        /** @brief Logs the map after a state change. */
        void update(const Map& map) override;
    };

} // namespace Game

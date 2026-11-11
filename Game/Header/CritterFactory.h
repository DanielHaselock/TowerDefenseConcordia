#ifndef CRITTER_FACTORY_H
#define CRITTER_FACTORY_H

#include <memory>
#include <string>
#include "Critter.h"

namespace Game
{
    /**
     * @enum CritterType
     * @brief Identifies the concrete critter type to build. Extend this enum
     * (and the switch in CritterFactory::create) when adding new critter types.
     */
    enum class CritterType
    {
        Walker,
        Speedy,
        Boss
    };

    /**
     * @class CritterFactory
     * @brief Factory Method for critter creation. Centralizing creation here
     * means the (future) Critter Group Generator never needs to know the
     * concrete critter classes directly, and adding a new critter type later
     * only requires a new class + one new case here.
     */
    class CritterFactory
    {
    public:
        /**
         * @brief Builds a new critter of the given type at the given level.
         *
         * @param type Which concrete critter class to instantiate.
         * @param level Difficulty level used to scale the critter's stats.
         * @return Owning pointer to the newly created critter, or nullptr if
         *         type is not recognized.
         */
        static std::unique_ptr<Critter> create(CritterType type, int level = 1);
    };
}
#endif // CRITTER_FACTORY_H

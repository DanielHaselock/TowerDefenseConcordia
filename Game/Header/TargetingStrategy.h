#pragma once
#include <memory>
#include <vector>
#include "Critter.h"

namespace Game {

/**
 * @brief The targeting rules a Tower can use to choose among several critters in range.
 *
 * The values are declared in cycling order: Tower::cycleTargetingStrategy()
 * steps through them one by one and wraps back to NearestToTower after Weakest.
 */
enum class TargetingStrategyType
{
    NearestToTower, ///< The critter closest to the tower.
    NearestToExit,  ///< The critter with the least path left to walk before the exit.
    Strongest,      ///< The critter with the most hit points left.
    Weakest         ///< The critter with the fewest hit points left.
};

/**
 * @class TargetingStrategy
 * @brief Strategy interface: how a tower picks one target out of the critters it could shoot.
 *
 * A tower works in two steps. It first finds the candidates (living critters
 * inside its range, which is the same for every tower), then asks its
 * TargetingStrategy to choose one of them. Only that second step varies, so
 * it is the part pulled out into this interface: a tower can swap its
 * strategy at any time without any other tower code changing.
 *
 * Every concrete strategy is stateless, and breaks ties in favour of the
 * critter that comes first in the candidate list.
 */
class TargetingStrategy
{
public:
    virtual ~TargetingStrategy() = default;

    /**
     * @brief Chooses the critter to shoot at.
     *
     * @param candidates Living critters already known to be within range. Not owned.
     * @param towerRow Grid row of the tower choosing.
     * @param towerCol Grid column of the tower choosing.
     * @return The chosen critter (the first one wins a tie), or nullptr if candidates is empty.
     */
    virtual Critter* chooseTarget(const std::vector<Critter*>& candidates, int towerRow, int towerCol) const = 0;

    /// Short human-readable name, shown in the GUI (e.g. "Weakest").
    virtual const char* getName() const = 0;

    /// Which of the four rules this strategy implements.
    virtual TargetingStrategyType getType() const = 0;
};

/**
 * @brief Builds the strategy matching the given type.
 *
 * Used both to give each tower type its default strategy and to cycle a
 * tower through all of them.
 */
std::unique_ptr<TargetingStrategy> makeTargetingStrategy(TargetingStrategyType type);

} // namespace Game

#pragma once
#include "TargetingStrategy.h"

namespace Game {

/**
 * @class WeakestStrategy
 * @brief Targets the critter with the fewest hit points left, so towers finish off critters that are already close to dying.
 */
class WeakestStrategy : public TargetingStrategy
{
public:
    Critter* chooseTarget(const std::vector<Critter*>& candidates, int towerRow, int towerCol) const override;
    const char* getName() const override;
    TargetingStrategyType getType() const override;
};

} // namespace Game

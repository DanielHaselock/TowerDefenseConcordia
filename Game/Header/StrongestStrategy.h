#pragma once
#include "TargetingStrategy.h"

namespace Game {

/**
 * @class StrongestStrategy
 * @brief Targets the critter with the most hit points left.
 */
class StrongestStrategy : public TargetingStrategy
{
public:
    Critter* chooseTarget(const std::vector<Critter*>& candidates, int towerRow, int towerCol) const override;
    const char* getName() const override;
    TargetingStrategyType getType() const override;
};

} // namespace Game

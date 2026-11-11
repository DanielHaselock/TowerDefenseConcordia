#pragma once
#include "TargetingStrategy.h"

namespace Game {

/**
 * @class NearestToExitStrategy
 * @brief Targets the critter closest to the exit, measured as the distance it still has to walk along its path (Critter::getDistanceToExit()).
 */
class NearestToExitStrategy : public TargetingStrategy
{
public:
    Critter* chooseTarget(const std::vector<Critter*>& candidates, int towerRow, int towerCol) const override;
    const char* getName() const override;
    TargetingStrategyType getType() const override;
};

} // namespace Game

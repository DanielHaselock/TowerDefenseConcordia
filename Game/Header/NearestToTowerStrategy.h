#pragma once
#include "TargetingStrategy.h"

namespace Game {

/**
 * @class NearestToTowerStrategy
 * @brief Targets the critter closest to the tower, measured as grid distance (the same row/column metric Tower uses for its range check).
 */
class NearestToTowerStrategy : public TargetingStrategy
{
public:
    Critter* chooseTarget(const std::vector<Critter*>& candidates, int towerRow, int towerCol) const override;
    const char* getName() const override;
    TargetingStrategyType getType() const override;
};

} // namespace Game

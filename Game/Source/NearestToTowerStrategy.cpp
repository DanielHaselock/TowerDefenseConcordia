#include "NearestToTowerStrategy.h"

namespace Game {

Critter* NearestToTowerStrategy::chooseTarget(const std::vector<Critter*>& candidates, int towerRow, int towerCol) const
{
    Critter* best = nullptr;
    int bestDistanceSquared = 0;

    for (Critter* critter : candidates)
    {
        int dRow = critter->getRow() - towerRow;
        int dCol = critter->getCol() - towerCol;
        int distanceSquared = dRow * dRow + dCol * dCol;

        // Strict comparison: on a tie the earlier critter keeps the target.
        if (best == nullptr || distanceSquared < bestDistanceSquared)
        {
            best = critter;
            bestDistanceSquared = distanceSquared;
        }
    }

    return best;
}

const char* NearestToTowerStrategy::getName() const { return "Near Tower"; }

TargetingStrategyType NearestToTowerStrategy::getType() const { return TargetingStrategyType::NearestToTower; }

} // namespace Game

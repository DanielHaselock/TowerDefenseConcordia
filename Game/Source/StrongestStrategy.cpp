#include "StrongestStrategy.h"

namespace Game {

Critter* StrongestStrategy::chooseTarget(const std::vector<Critter*>& candidates, int, int) const
{
    Critter* best = nullptr;

    for (Critter* critter : candidates)
    {
        // Strict comparison: on a tie the earlier critter keeps the target.
        if (best == nullptr || critter->getHitPoints() > best->getHitPoints())
        {
            best = critter;
        }
    }

    return best;
}

const char* StrongestStrategy::getName() const { return "Strongest"; }

TargetingStrategyType StrongestStrategy::getType() const { return TargetingStrategyType::Strongest; }

} // namespace Game

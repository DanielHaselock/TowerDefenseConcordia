#include "WeakestStrategy.h"

namespace Game {

Critter* WeakestStrategy::chooseTarget(const std::vector<Critter*>& candidates, int, int) const
{
    Critter* best = nullptr;

    for (Critter* critter : candidates)
    {
        // Strict comparison: on a tie the earlier critter keeps the target.
        if (best == nullptr || critter->getHitPoints() < best->getHitPoints())
        {
            best = critter;
        }
    }

    return best;
}

const char* WeakestStrategy::getName() const { return "Weakest"; }

TargetingStrategyType WeakestStrategy::getType() const { return TargetingStrategyType::Weakest; }

} // namespace Game

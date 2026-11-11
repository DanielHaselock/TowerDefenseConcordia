#include "NearestToExitStrategy.h"

namespace Game {

Critter* NearestToExitStrategy::chooseTarget(const std::vector<Critter*>& candidates, int, int) const
{
    Critter* best = nullptr;
    float bestDistance = 0.0f;

    for (Critter* critter : candidates)
    {
        float distance = critter->getDistanceToExit();

        // Strict comparison: on a tie the earlier critter keeps the target.
        if (best == nullptr || distance < bestDistance)
        {
            best = critter;
            bestDistance = distance;
        }
    }

    return best;
}

const char* NearestToExitStrategy::getName() const { return "Near Exit"; }

TargetingStrategyType NearestToExitStrategy::getType() const { return TargetingStrategyType::NearestToExit; }

} // namespace Game

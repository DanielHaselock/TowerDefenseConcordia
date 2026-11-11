#include "TargetingStrategy.h"
#include "NearestToTowerStrategy.h"
#include "NearestToExitStrategy.h"
#include "StrongestStrategy.h"
#include "WeakestStrategy.h"

#include <stdexcept>

namespace Game {

std::unique_ptr<TargetingStrategy> makeTargetingStrategy(TargetingStrategyType type)
{
    switch (type)
    {
    case TargetingStrategyType::NearestToTower:
        return std::make_unique<NearestToTowerStrategy>();
    case TargetingStrategyType::NearestToExit:
        return std::make_unique<NearestToExitStrategy>();
    case TargetingStrategyType::Strongest:
        return std::make_unique<StrongestStrategy>();
    case TargetingStrategyType::Weakest:
        return std::make_unique<WeakestStrategy>();
    }
    throw std::invalid_argument("makeTargetingStrategy: unknown TargetingStrategyType");
}

} // namespace Game

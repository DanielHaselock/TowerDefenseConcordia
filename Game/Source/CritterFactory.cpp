#include "CritterFactory.h"

#include "BossCritter.h"
#include "SpeedyCritter.h"
#include "WalkerCritter.h"

namespace Game
{
    std::unique_ptr<Critter> CritterFactory::create(CritterType type, int level)
    {
        switch (type)
        {
        case CritterType::Walker:
            return std::make_unique<Game::WalkerCritter>(level);
        case CritterType::Speedy:
            return std::make_unique<Game::SpeedyCritter>(level);
        case CritterType::Boss:
            return std::make_unique<Game::BossCritter>(level);
        default:
            return nullptr;
        }
    }
}

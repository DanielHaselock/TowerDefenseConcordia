#include "AoeTower.h"

namespace Game {

AoeTower::AoeTower(int row, int col, ::Renderer::spriteSheetTexture* p_texture)
    : Tower(row, col, /*cost=*/LEVEL_CONFIGS[0].levelCost, /*currLvl=*/0, p_texture, TargetingStrategyType::Strongest) {
    levels = LEVEL_CONFIGS;
}

TowerType AoeTower::getType() const
{
    return TowerType::AoE;
}

int AoeTower::TryUpgrade(int amountOfMoney)
{
    if (canUpgrade() && amountOfMoney >= getUpgradeCost())
    {
        upgrade();
        return getUpgradeCost();
    }

    return 0;
}

void AoeTower::shoot(int initalX, int initialY, int angle, Critter& target) {
    if (bulletRequest != nullptr)
        return;

    // The bullet carries this level's blast radius; on impact the game
    // loop applies the splash through applySplash() below.
    int power = levels[currentLevel].power;
    float blastRadius = levels[currentLevel].specialMultiplier;

    bulletRequest = new BulletRequest(power, 500, initalX, initialY, angle, &target, 1, blastRadius);
}

void AoeTower::applySplash(const std::vector<Critter*>& critters, const std::vector<std::pair<int, int>>& cells, int damage)
{
    for (Critter* critter : critters)
    {
        if (critter->isDead())
        {
            continue;
        }

        for (const auto& cell : cells)
        {
            if (critter->getRow() == cell.first && critter->getCol() == cell.second)
            {
                critter->takeDamage(damage);
                break;
            }
        }
    }
}

} // namespace Game

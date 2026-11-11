#include "SlowTower.h"

namespace Game {

    SlowTower::SlowTower(int row, int col, ::Renderer::spriteSheetTexture* p_texture)
        : Tower(row, col, /*cost=*/LEVEL_CONFIGS[0].levelCost, /*currLvl=*/0, p_texture, TargetingStrategyType::NearestToExit) {
        levels = LEVEL_CONFIGS;
    }

    TowerType SlowTower::getType() const
    {
        return TowerType::Slow;
    }

    int SlowTower::TryUpgrade(int amountOfMoney)
    {
        if (canUpgrade() && amountOfMoney >= getUpgradeCost())
        {
            upgrade();
            return getUpgradeCost();
        }

        return 0;
    }

    void SlowTower::shoot(int initalX, int initialY, int angle, Critter& target)
    {
        if (bulletRequest != nullptr)
            return;

        // Direct damage only ever affects its single target; the full critter
        // list exists for other tower types (e.g. area-of-effect) that need it.
        int power = levels[currentLevel].power;
        bulletRequest = new BulletRequest(power, 500, initalX, initialY, angle, &target, 2);
    }

} // namespace Game

#pragma once
#include "Tower.h"

namespace Game {

/**
 * @class DirectDammageTower
 * @brief The baseline tower type: fires a single bullet that deals direct
 * damage straight to its target. No splash, no special effects.
 */
class DirectDammageTower : public Tower {
public:
    /// Constructs a level-1 direct-damage tower at the given grid position.
    explicit DirectDammageTower(int row, int col, ::Renderer::spriteSheetTexture* p_texture);

    /// Stats for levels 1-3. Range/power/rate of fire all improve as the
    /// tower is upgraded.
    inline static const std::vector<TowerLevel> LEVEL_CONFIGS = {
    TowerLevel(1, 10, 2, 1, 1.0f),
    TowerLevel(2, 10, 3, 15, 1.2f),
    TowerLevel(3, 10, 5, 20, 1.5f)
    };

    /// Attempts to upgrade the tower using DirectDammageTower::LEVEL_CONFIGS; see Tower::TryUpgrade.
    int TryUpgrade(int amountOfMoney) override;

    /// Reports TowerType::Direct; see Tower::getType.
    TowerType getType() const override;

protected:
    /// Queues a bullet request carrying this tower's current power and the
    /// "no special effect" code; when the bullet lands, it simply damages
    /// its single target with no splash or slow.
    void shoot(int initalX, int initialY, int angle, Critter& target) override;
};

} // namespace Game

#pragma once
#include "Tower.h"

namespace Game {

/**
 * @class SlowTower
 * @brief Tower that deals light damage but also slows its target down,
 * giving other towers more time to finish it off.
 *
 * For simplicity, the slow effect is a permanent reduction of the
 * critter's speed rather than one that wears off after a duration.
 * Timed effects would need each critter to track its own "slow expires
 * at" timer, which belongs to the game loop (not yet implemented) rather
 * than the tower itself.
 */
class SlowTower : public Tower {
public:
    /// Constructs a level-1 slow tower at the given grid position.
    explicit SlowTower(int row, int col, ::Renderer::spriteSheetTexture* p_texture);

    /// Stats for levels 1-3. specialMultiplier holds the fraction of speed
    /// left after being hit (e.g. 0.5 = half speed) for that level.
    inline static const std::vector<TowerLevel> LEVEL_CONFIGS =
    {
        // level, upgradeCost, range, power, rateOfFire, specialMultiplier(=speed fraction kept)
        TowerLevel(1, 15,  2, 1, 1.0f, 0.66f),
        TowerLevel(2, 20,  5, 4, 0.9f, 0.50f),
        TowerLevel(3, 30, 6, 5, 0.8f, 0.33f)
    };

    /// Attempts to upgrade the tower using SlowTower::LEVEL_CONFIGS; see Tower::TryUpgrade.
    int TryUpgrade(int amountOfMoney) override;

    /// Reports TowerType::Slow; see Tower::getType.
    TowerType getType() const override;

protected:
    /// Queues a bullet request carrying this tower's current power and the
    /// "slow" effect code; when the bullet lands, it damages its target and
    /// halves the target's speed (see Critter::halfSpeed()).
    void shoot(int initalX, int initialY, int angle, Critter& target) override;
};

} // namespace Game

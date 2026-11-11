#pragma once
#include <utility>
#include "Tower.h"

namespace Game {

/**
 * @class AoeTower
 * @brief Tower whose bullets explode on impact: every critter caught
 * within a blast radius around the primary target takes damage, not just
 * the target itself. Good against groups of critters walking close
 * together.
 */
class AoeTower : public Tower {
public:
    /// Constructs a level-1 AoE tower at the given grid position.
    explicit AoeTower(int row, int col, ::Renderer::spriteSheetTexture* p_texture);

    /// Stats for levels 1-3. specialMultiplier holds how many path cells
    /// (including the impact cell itself) the blast covers at that level.
    inline static const std::vector<TowerLevel> LEVEL_CONFIGS = {
        // level, upgradeCost, range, power, rateOfFire, specialMultiplier(=blast cell count)
        TowerLevel(1, 15,  3, 2,  1.5f, 1.0f),
        TowerLevel(2, 20,  3, 12, 1.4f, 2.0f),
        TowerLevel(3, 25, 4, 16, 1.3f, 3.0f)
    };

    /// Attempts to upgrade the tower using AoeTower::LEVEL_CONFIGS; see Tower::TryUpgrade.
    int TryUpgrade(int amountOfMoney) override;

    /**
     * @brief Applies one AoE impact: damages every living critter standing on one of the given cells.
     *
     * The cell list is the blast's actual footprint (see
     * GameManager::MainLoop(), which walks the critters' path outward from
     * the impact tile rather than using a 2D radius, so the footprint
     * always follows the corridor). takeDamage() itself guards against
     * rewarding an overkilled critter twice.
     *
     * @param critters All critters currently on the map (not owned).
     * @param cells Grid (row, col) cells caught in the blast.
     * @param damage Damage dealt to every critter caught in the blast.
     */
    static void applySplash(const std::vector<Critter*>& critters, const std::vector<std::pair<int, int>>& cells, int damage);

    /// Reports TowerType::AoE; see Tower::getType.
    TowerType getType() const override;

protected:
    /// Queues a bullet request carrying this tower's current power and the
    /// "AoE" effect code; when the bullet lands, it creates a HitRequest so
    /// the game loop can apply splash damage to critters near the impact
    /// tile, instead of only hurting the single selected target.
    void shoot(int initalX, int initialY, int angle, Critter& target) override;
};

} // namespace Game

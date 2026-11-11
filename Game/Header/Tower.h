#pragma once
#include <memory>
#include <vector>
#include "Critter.h"
#include "Bullet.h"
#include "Observer.h"
#include "TargetingStrategy.h"

namespace Renderer
{
    class spriteSheetTexture;
}

namespace Game {

    /**
 * @brief Stats for a single tower upgrade level.
 *
 * Each Tower subclass owns a small list of these (one per level) describing
 * how its cost, range, power, etc. grow as it gets upgraded.
 */
    struct TowerLevel
    {
        int level;        ///< Level number, starting at 1.
        int levelCost;  ///< Coins needed to reach this level from the previous one.
        int range;        ///< How far (in grid tiles) the tower can detect critters.
        int power;        ///< Damage dealt per shot at this level.
        float rateOfFire; ///< Cooldown between shots, in seconds (lower = fires more often).

        /// Meaning depends on the tower type: e.g. blast radius for an
        /// area-of-effect tower, or the speed fraction left after a slow effect.
        float specialMultiplier = 1.0f;

        TowerLevel(int lvl, int cost, int range, int power, float rateOfFire, float specialMultiplier = 1.0f)
            : level(lvl), levelCost(cost), range(range), power(power), rateOfFire(rateOfFire), specialMultiplier(specialMultiplier) {
        }
    };

/**
 * @brief Which kind of tower a Tower is.
 *
 * Lets code that only sees a Tower* branch on the kind without a cast:
 * the inspection UI picks the matching panel with it, for example.
 */
enum class TowerType
{
    Direct, ///< Single-target damage tower.
    Slow,   ///< Tower that also slows what it hits.
    AoE     ///< Tower whose shots splash around the impact point.
};

/**
 * @class Tower
 * @brief Abstract base class for every tower type in the game.
 *
 * A Tower is bought at a fixed row/col on the map grid for an initial cost, can be
 * upgraded through a sequence of levels (each more expensive, but with
 * better range/power/rate of fire), and can be sold at any time for a
 * refund that is a percentage of everything spent on it, where the
 * percentage shrinks as the game advances through the early waves.
 *
 * During gameplay, Update() is called every frame: the tower looks for
 * critters within its range, picks one to target, and fires at it once its
 * cooldown allows. Which critter it picks when several are in range is
 * delegated to its TargetingStrategy (Strategy pattern), which the player
 * can change at any time. What "firing" actually does (plain damage,
 * splash damage, slowing, ...) is left to subclasses via shoot(), since
 * that is the only part that differs between tower types.
 */
class Tower : public Observable<Tower> {
public:
    /**
     * @brief Constructs a tower at the given grid position.
     *
     * cost is the number of coins required to buy the tower at level 1;
     * each subclass passes the level 1 entry of its own LEVEL_CONFIGS so
     * the level table stays the single source of truth for tower prices.
     * currLvl is the starting index into the levels vector (0 = level 1).
     * type is the targeting strategy the tower starts with.
     */
    Tower(int row, int col, int cost, int currLvl, ::Renderer::spriteSheetTexture* p_texture, TargetingStrategyType type);

    // ---- Upgrade logic ----

    /// True if the tower has not yet reached its final level.
    bool canUpgrade() const;

    /// Coins required to upgrade to the next level.
    /// @throws std::logic_error if the tower is already at its max level.
    int getUpgradeCost() const;

    /// Coins it cost to reach the tower's current level. Meant to be read by
    /// an observer reacting to upgrade()'s notify(), since by the time that
    /// fires currentLevel already points at the level just paid for.
    int getCurrentLevelCost() const;

    /// Advances the tower to the next level.
    /// @throws std::logic_error if the tower is already at its max level.
    void upgrade();

    /**
     * @brief Coins refunded if the tower is sold during the given wave.
     *
     * The refund is a percentage of everything the player has spent on
     * this tower (buy cost plus every upgrade paid for so far), where the
     * percentage depends on the wave the sale happens in: 80% during wave
     * 1, dropping to 55% from wave 6 onward (see GetRefundPercent()).
     *
     * @param wave Current wave number, as reported by WaveManager.
     */
    int getRefundValue(int wave) const;

    /**
     * @brief Refund percentage applied to a tower sold in the given wave.
     *
     * Hardcoded table: wave 1 = 80, wave 2 = 75, wave 3 = 70, wave 4 = 65,
     * wave 5 = 60, wave 6 and later = 55. Wave numbers below 1 are treated
     * as wave 1, so selling before the first wave refunds 80%.
     *
     * @param wave Wave number the tower is being sold in.
     * @return Refund percentage, out of 100.
     */
    static int GetRefundPercent(int wave);

    // ---- Simple state getters ----

    /// Grid row the tower was built at.
    int getRow() const;

    /// Grid column the tower was built at.
    int getCol() const;

    /// Coins it originally cost to buy this tower.
    int getBuyingCost() const;

    /// Current level index (0 = level 1, 1 = level 2, ...).
    int getCurrentLevel() const;

    /// Detection radius, in grid tiles, at the current level.
    int getRange() const;

    /// Damage dealt per shot at the current level.
    int getPower() const;

    /// Seconds the tower must wait between shots at the current level.
    float getRateOfFire() const;

    /// Which kind of tower this is; each subclass reports its own type.
    virtual TowerType getType() const = 0;

    // ---- Combat logic ----

    /**
     * @brief Runs one "tick" of the tower's behavior.
     *
     * Detects critters within range, lets the tower's targeting strategy
     * choose one of them as the target, and shoots it if the tower isn't still on cooldown. Does nothing if
     * no critter is in range or the tower recently fired.
     *
     * @param currentTime Elapsed game time in seconds, as tracked by the
     *        main game loop. Used together with the tower's rate of fire
     *        to know when it is allowed to shoot again.
     * @param critters All critters currently on the map. Towers never take
     *        ownership of critters, they only read/damage them through
     *        this list.
     */
    void operate(float currentTime, const std::vector<Critter*>& critters);

    /**
     * @brief Per-frame tower behavior: aim, and fire when ready.
     *
     * Picks a critter in range via selectTarget(). If none is
     * found, idles by slowly rotating the turret texture. Otherwise rotates
     * the turret to face the target and, once the tower's current level
     * rate of fire has elapsed, calls shoot() to queue a new bullet request.
     *
     * @param deltatime Seconds elapsed since the last frame.
     * @param critters All critters currently on the map.
     */
    void Update(float deltatime, const std::vector<Critter*>& critters);

    /// Pending bullet request created by the last shoot() call, or nullptr
    /// if none is pending (consumed by processShoot()).
    BulletRequest* getBulletRequest() { return bulletRequest; }

    /// Replaces the sprite used to render this tower's turret.
    void setTexture(::Renderer::spriteSheetTexture*);

    /// Clears the current bullet request after it has been consumed (e.g.
    /// turned into an actual Bullet), freeing it and resetting the pointer.
    void processShoot();

    /// Prints this tower's row and column to standard output, for debugging.
    void printDetails();

    /// Releases the tower's pending bullet request. Called before the tower itself is destroyed.
    void DestroyTower();

    /**
     * @brief Attempts to upgrade the tower if the player can afford it.
     *
     * @param amountOfMoney Coins currently available to spend.
     * @return Coins actually spent (the upgrade cost) if the upgrade
     *         succeeded, or 0 if the tower was already maxed out or the
     *         player couldn't afford it.
     */
    virtual int TryUpgrade(int amountOfMoney) = 0;

    /// Releases the tower's pending bullet request, if any.
    virtual ~Tower();

    // ---- Targeting strategy ----

    /// Replaces how this tower picks among several critters in range. A null
    /// strategy is ignored, so the tower always has one.
    void setTargetingStrategy(std::unique_ptr<TargetingStrategy> strategy);

    /// The strategy this tower currently uses to choose its target.
    const TargetingStrategy& getTargetingStrategy() const;

    /// Switches to the next strategy in TargetingStrategyType order, wrapping
    /// from the last one back to the first. Not an upgrade, so observers are
    /// not notified.
    void cycleTargetingStrategy();

    /// Grants the unit tests (TowerTest) access to the protected shoot() hook.
    friend class TowerTest;

protected:
    /// True if critter is within the tower's current range.
    bool isInRange(const Critter& critter) const;

    /**
     * @brief Picks which critter to shoot at.
     *
     * Collects every living critter within range, then lets the current
     * targeting strategy choose one of them. Dead critters and critters
     * out of range never reach the strategy.
     *
     * @return Pointer to the chosen critter, or nullptr if none are in range.
     */
    Critter* selectTarget(const std::vector<Critter*>& critters) const;

    /**
     * @brief Applies this tower's bullet effect to a target.
     *
     * This is the one method each concrete tower type must implement, since
     * it is the only thing that differs between tower types: a direct-damage
     * tower just hurts the target, an area-of-effect tower also damages
     * nearby critters, a slow tower reduces the target's speed, etc.
     *
     * @param initalX Pixel X the fired bullet should spawn at (the tower's current position).
     * @param initialY Pixel Y the fired bullet should spawn at (the tower's current position).
     * @param angle Initial rotation, in degrees, applied to the fired bullet's sprite.
     * @param target The critter selected by selectTarget().
     */
    virtual void shoot(int initalX, int initialY, int angle, Critter& target) = 0;

    int row;
    int col;
    int buyingCost; ///< Coins the player paid to buy this tower at level 1.

    std::vector<TowerLevel> levels; ///< Per-level stats (cost, range, power, rate of fire, special multiplier), set by each subclass.
    int currentLevel; ///< Index into levels for the tower's current level (0 = level 1).

    float lastShotTime; ///< Cooldown accumulator; seconds elapsed since the tower was last allowed to shoot.

    ::Renderer::spriteSheetTexture* m_texture = nullptr; ///< Sprite used to render this tower's turret.

    double textureAngle = 0; ///< Current rotation, in degrees, applied to the turret sprite.
    double baseTurnSpeed = 100; ///< Degrees per second the turret idles/rotates at when no target is in range.
    BulletRequest* bulletRequest = nullptr; ///< Pending request describing the bullet this tower just fired, consumed by processShoot().

    std::unique_ptr<TargetingStrategy> targetingStrategy; ///< How this tower picks among critters in range; never null.

    bool canShoot = true; ///< True once the fixed cooldown has elapsed and the tower is allowed to fire again.
};

} // namespace Game

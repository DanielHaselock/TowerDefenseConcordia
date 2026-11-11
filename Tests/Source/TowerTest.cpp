#include "Tower.h"
#include "DirectDammageTower.h"
#include "AoeTower.h"
#include "SlowTower.h"
#include "Critter.h"
#include "Texture.h"
#include "Observer.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#else
#include <catch2/catch.hpp>
#endif

#if __has_include(<catch2/catch_approx.hpp>)
using Catch::Approx;
#else
using Catch::Detail::Approx;
#endif

namespace Game
{
    /// Minimal concrete Critter so towers have a target to shoot at.
    class TowerTestCritter : public Critter
    {
    public:
        explicit TowerTestCritter(::Renderer::spriteSheetTexture* texture)
            : Critter(10, 2, 40, 1, 5, 0, 0, texture)
        {
        }

        std::string getType() const override { return "TowerTest"; }
        std::unique_ptr<Critter> clone() const override { return std::make_unique<TowerTestCritter>(*this); }
        void Update(float, const std::vector<::Renderer::RendererMap*>&) override {}
        void setLevel(int p_level) override { level = p_level; }
    };

    /// Friend access (granted by Tower.h) so the tests can invoke the
    /// protected shoot() hook directly, in isolation from Update(). The
    /// parameter is a base Tower& so name lookup resolves to Tower::shoot for
    /// the access check (TowerTest is a friend of Tower) while the call still
    /// virtual-dispatches to the concrete tower's override.
    class TowerTest
    {
    public:
        static void shoot(Tower& tower, int x, int y, int angle, Critter& target)
        {
            tower.shoot(x, y, angle, target);
        }
    };

    /// Records every notify() call a Tower sends it, and the Tower passed in.
    class RecordingTowerObserver : public Observer<Tower>
    {
    public:
        explicit RecordingTowerObserver(Tower& tower) : Observer<Tower>(tower) {}

        int callCount = 0;
        const Tower* lastTower = nullptr;

        void update(const Tower& tower) override
        {
            ++callCount;
            lastTower = &tower;
        }
    };
}

using Game::DirectDammageTower;
using Game::AoeTower;
using Game::SlowTower;

TEST_CASE("DirectDammageTower starts at level one with its documented stats", "[Tower]")
{
    DirectDammageTower tower(2, 5, nullptr);

    CHECK(tower.getRow() == 2);
    CHECK(tower.getCol() == 5);
    CHECK(tower.getBuyingCost() == 10);
    CHECK(tower.getCurrentLevel() == 0);
    CHECK(tower.getRange() == 2);
    CHECK(tower.getPower() == 1);
    CHECK(tower.getRateOfFire() == Approx(1.0f));
}

TEST_CASE("Every tower subclass reports its TowerType, also through a base pointer", "[Tower]")
{
    DirectDammageTower direct(0, 0, nullptr);
    SlowTower slow(0, 0, nullptr);
    AoeTower aoe(0, 0, nullptr);

    CHECK(direct.getType() == Game::TowerType::Direct);
    CHECK(slow.getType() == Game::TowerType::Slow);
    CHECK(aoe.getType() == Game::TowerType::AoE);

    // The inspection UI only ever holds Tower*, so the virtual dispatch
    // has to pick the same types.
    Game::Tower* towers[] = { &direct, &slow, &aoe };
    CHECK(towers[0]->getType() == Game::TowerType::Direct);
    CHECK(towers[1]->getType() == Game::TowerType::Slow);
    CHECK(towers[2]->getType() == Game::TowerType::AoE);
}

TEST_CASE("Tower upgrades raise range, power and rate of fire per level", "[Tower]")
{
    DirectDammageTower tower(0, 0, nullptr);

    REQUIRE(tower.canUpgrade());
    // Upgrading to level 2 costs its configured levelCost.
    REQUIRE(tower.getUpgradeCost() == 10);
    tower.upgrade();
    CHECK(tower.getCurrentLevel() == 1);
    CHECK(tower.getRange() == 3);
    CHECK(tower.getPower() == 15);
    CHECK(tower.getRateOfFire() == Approx(1.2f));

    tower.upgrade();
    CHECK(tower.getCurrentLevel() == 2);
    CHECK(tower.getRange() == 5);
    CHECK(tower.getPower() == 20);
    CHECK(tower.getRateOfFire() == Approx(1.5f));
}

TEST_CASE("Tower cannot be upgraded past its final level", "[Tower]")
{
    DirectDammageTower tower(0, 0, nullptr);
    tower.upgrade();
    tower.upgrade();

    REQUIRE_FALSE(tower.canUpgrade());
    CHECK(tower.getUpgradeCost() == 0);
    // upgrade() past the max level is a no-op.
    tower.upgrade();
    CHECK(tower.getCurrentLevel() == 2);
}

TEST_CASE("Tower TryUpgrade only spends coins when the upgrade is affordable", "[Tower]")
{
    DirectDammageTower tower(0, 0, nullptr);

    CHECK(tower.TryUpgrade(9) == 0);      // cannot afford the 10-coin upgrade
    CHECK(tower.getCurrentLevel() == 0);

    // Each successful upgrade returns the cost of the NEXT level (the cost the
    // player would pay to upgrade again). At the final level there is no next
    // level, so the return falls through to 0.
    CHECK(tower.TryUpgrade(10) == 10);    // L1 -> L2, reports L2's cost (10)
    CHECK(tower.getCurrentLevel() == 1);
    CHECK(tower.TryUpgrade(10) == 0);     // L2 -> L3 (max), no next level to price
    CHECK(tower.getCurrentLevel() == 2);

    CHECK(tower.TryUpgrade(999) == 0);    // already maxed out
}

TEST_CASE("Tower refund percent follows the hardcoded wave table", "[Tower][Refund]")
{
    CHECK(DirectDammageTower::GetRefundPercent(1) == 80);
    CHECK(DirectDammageTower::GetRefundPercent(2) == 75);
    CHECK(DirectDammageTower::GetRefundPercent(3) == 70);
    CHECK(DirectDammageTower::GetRefundPercent(4) == 65);
    CHECK(DirectDammageTower::GetRefundPercent(5) == 60);
    CHECK(DirectDammageTower::GetRefundPercent(6) == 55);

    // Past wave 6 the percentage stays at its floor, and wave numbers
    // below 1 (selling before the first wave) are treated as wave 1.
    CHECK(DirectDammageTower::GetRefundPercent(7) == 55);
    CHECK(DirectDammageTower::GetRefundPercent(100) == 55);
    CHECK(DirectDammageTower::GetRefundPercent(0) == 80);
}

TEST_CASE("Tower refund is the wave percentage of everything invested", "[Tower][Refund]")
{
    DirectDammageTower tower(0, 0, nullptr);

    // Level 1 direct tower: 10 coins invested, 80% back during wave 1.
    CHECK(tower.getRefundValue(1) == 8);

    // After one upgrade the player has spent 10 + 10, and wave 2 pays 75%.
    tower.upgrade();
    CHECK(tower.getRefundValue(2) == 15);

    // Fully upgraded: 10 + 10 + 10 invested, 55% from wave 6 on.
    tower.upgrade();
    CHECK(tower.getRefundValue(6) == 16);

    // The refund is rounded down and never exceeds what was invested.
    SlowTower slow(0, 0, nullptr);
    CHECK(slow.getRefundValue(1) == 12);   // 15 invested, 80% = 12
    CHECK(slow.getRefundValue(6) == 8);    // 15 invested, 55% = 8 (8.25 rounded down)

    AoeTower aoe(0, 0, nullptr);
    CHECK(aoe.getRefundValue(1) == 12);    // 15 invested, 80% = 12

    for (int wave = 1; wave <= 10; ++wave)
    {
        CHECK(aoe.getRefundValue(wave) <= aoe.getBuyingCost());
    }
}

TEST_CASE("AoeTower and SlowTower carry their own level tables", "[Tower]")
{
    AoeTower aoe(1, 1, nullptr);
    CHECK(aoe.getBuyingCost() == 15);
    CHECK(aoe.getRange() == 3);
    CHECK(aoe.getPower() == 2);
    CHECK(aoe.getRateOfFire() == Approx(1.5f));

    aoe.upgrade();
    CHECK(aoe.getRange() == 3);
    CHECK(aoe.getPower() == 12);
    CHECK(aoe.getRateOfFire() == Approx(1.4f));

    aoe.upgrade();
    CHECK(aoe.getRange() == 4);
    CHECK(aoe.getPower() == 16);
    CHECK(aoe.getRateOfFire() == Approx(1.3f));

    SlowTower slow(2, 3, nullptr);
    CHECK(slow.getBuyingCost() == 15);
    CHECK(slow.getRange() == 2);
    CHECK(slow.getPower() == 1);
    CHECK(slow.getRateOfFire() == Approx(1.0f));

    slow.upgrade();
    CHECK(slow.getRange() == 5);
    CHECK(slow.getPower() == 4);
    CHECK(slow.getRateOfFire() == Approx(0.9f));

    slow.upgrade();
    CHECK(slow.getRange() == 6);
    CHECK(slow.getPower() == 5);
    CHECK(slow.getRateOfFire() == Approx(0.8f));
}

TEST_CASE("Tower shoot queues a bullet request with the current level's power", "[Tower][Bullet]")
{
    ::Renderer::spriteSheetTexture targetTex;
    Game::TowerTestCritter critter(&targetTex);
    critter.setPosition(2, 3);

    ::Renderer::spriteSheetTexture towerTex;
    DirectDammageTower tower(0, 0, &towerTex);

    REQUIRE(tower.getBulletRequest() == nullptr);
    Game::TowerTest::shoot(tower, 0, 0, 45, critter);

    Game::BulletRequest* req = tower.getBulletRequest();
    REQUIRE(req != nullptr);
    CHECK(req->damage == 1);          // level-1 power
    CHECK(req->speed == Approx(500.0f));
    CHECK(req->initalX == 0);
    CHECK(req->initialY == 0);
    CHECK(req->angle == 45);
    CHECK(req->target == &critter);
    CHECK(req->effect == 0);          // no special effect

    // A second shoot() while a request is pending is ignored.
    Game::TowerTest::shoot(tower, 0, 0, 0, critter);
    CHECK(tower.getBulletRequest() == req);

    // processShoot() frees the pending request.
    tower.processShoot();
    CHECK(tower.getBulletRequest() == nullptr);
}

TEST_CASE("AoeTower shoot requests the AoE effect and SlowTower requests the slow effect", "[Tower][Bullet]")
{
    ::Renderer::spriteSheetTexture targetTex;
    Game::TowerTestCritter critter(&targetTex);

    ::Renderer::spriteSheetTexture aoeTex;
    AoeTower aoe(0, 0, &aoeTex);
    Game::TowerTest::shoot(aoe, 1, 2, 0, critter);
    REQUIRE(aoe.getBulletRequest() != nullptr);
    CHECK(aoe.getBulletRequest()->damage == 2);   // level-1 power
    CHECK(aoe.getBulletRequest()->effect == 1);   // AoE
    CHECK(aoe.getBulletRequest()->blastRadius == Approx(1.0f));   // level-1 blast radius

    aoe.processShoot();
    REQUIRE(aoe.getBulletRequest() == nullptr);

    // Upgrading widens the blast the next shot will carry.
    aoe.upgrade();
    Game::TowerTest::shoot(aoe, 1, 2, 0, critter);
    REQUIRE(aoe.getBulletRequest() != nullptr);
    CHECK(aoe.getBulletRequest()->damage == 12);  // level-2 power
    CHECK(aoe.getBulletRequest()->blastRadius == Approx(2.0f));   // level-2 blast radius
    aoe.processShoot();

    ::Renderer::spriteSheetTexture slowTex;
    SlowTower slow(0, 0, &slowTex);
    Game::TowerTest::shoot(slow, 1, 2, 0, critter);
    REQUIRE(slow.getBulletRequest() != nullptr);
    CHECK(slow.getBulletRequest()->damage == 1);  // level-1 power
    CHECK(slow.getBulletRequest()->effect == 2);  // slow
    CHECK(slow.getBulletRequest()->blastRadius == Approx(0.0f));  // no blast
}

TEST_CASE("AoeTower applySplash damages every living critter standing on the blast cells", "[Tower][AoE]")
{
    ::Renderer::spriteSheetTexture tex;

    Game::TowerTestCritter atImpact(&tex);
    atImpact.setPosition(4, 4);      // on impact cell
    atImpact.setIsActive(true);

    Game::TowerTestCritter neighbour(&tex);
    neighbour.setPosition(4, 5);     // on adjacent path cell in blast
    neighbour.setIsActive(true);

    Game::TowerTestCritter outside(&tex);
    outside.setPosition(4, 6);       // outside blast cells
    outside.setIsActive(true);

    Game::TowerTestCritter pooled(&tex);
    pooled.setPosition(4, 4);        // on blast cell but not spawned (isDead)
    // left inactive on purpose

    std::vector<Game::Critter*> critters{ &atImpact, &neighbour, &outside, &pooled };
    std::vector<std::pair<int, int>> blastCells{ {4, 4}, {4, 5} };

    AoeTower::applySplash(critters, blastCells, 10);

    CHECK(atImpact.getHitPoints() == 0);    // 10 - 10
    CHECK(neighbour.getHitPoints() == 0);   // 10 - 10
    CHECK(outside.getHitPoints() == 10);    // untouched
    CHECK(pooled.getHitPoints() == 10);     // skipped, not spawned

    // A 1-cell footprint (level-1 AoE) only hits critters on the impact cell itself.
    ::Renderer::spriteSheetTexture tex2;
    Game::TowerTestCritter adjacentCritter(&tex2);
    adjacentCritter.setPosition(4, 5);
    adjacentCritter.setIsActive(true);

    AoeTower::applySplash({ &adjacentCritter }, { {4, 4} }, 10);
    CHECK(adjacentCritter.getHitPoints() == 10);
}

TEST_CASE("Tower Update idles (rotates) when no critter is in range", "[Tower]")
{
    ::Renderer::spriteSheetTexture towerTex;
    towerTex.m_pos_scale.x = 10.0f;
    towerTex.m_pos_scale.y = 20.0f;
    DirectDammageTower tower(5, 5, &towerTex);

    // A critter far outside range (tower range 2) must be ignored:
    // the turret idles, rotating by baseTurnSpeed * deltatime.
    ::Renderer::spriteSheetTexture critterTex;
    Game::TowerTestCritter critter(&critterTex);
    critter.setPosition(50, 50);

    std::vector<Game::Critter*> targets{ &critter };
    float before = towerTex.m_angle;
    tower.Update(0.5f, targets);
    float expected = before + 0.5f * 100.0f;
    if (expected > 360.0f)
        expected -= 360.0f;
    CHECK(towerTex.m_angle == Approx(expected));
    CHECK(tower.getBulletRequest() == nullptr);   // nothing was fired
}

TEST_CASE("Tower Update fires at an in-range critter and waits out the level's rate of fire", "[Tower][Bullet]")
{
    ::Renderer::spriteSheetTexture towerTex;
    towerTex.m_pos_scale.x = 0.0f;
    towerTex.m_pos_scale.y = 0.0f;
    DirectDammageTower tower(5, 5, &towerTex);

    ::Renderer::spriteSheetTexture critterTex;
    critterTex.m_pos_scale.x = 1.0f;
    critterTex.m_pos_scale.y = 1.0f;
    Game::TowerTestCritter critter(&critterTex);
    critter.setPosition(5, 6);   // distance 1, inside range 2
    critter.setIsActive(true);   // a live critter, so selectTarget() picks it

    std::vector<Game::Critter*> targets{ &critter };

    // First update: target found, canShoot starts true -> shoot() is called.
    tower.Update(0.1f, targets);
    REQUIRE(tower.getBulletRequest() != nullptr);
    CHECK(tower.getBulletRequest()->damage == 1);

    // While on cooldown no new request is queued. The cooldown is the
    // level's rate of fire (1.0 s for a level-1 direct tower); the flag
    // flips once the accumulator reaches it, and the re-fire lands on the
    // *next* frame (Update() tests canShoot before it accumulates).
    // shoot() refuses to queue a second bullet while one is still pending, so
    // the first request must be consumed before a second one can be queued.
    tower.processShoot();
    REQUIRE(tower.getBulletRequest() == nullptr);
    tower.Update(0.5f, targets);   // 0.5 s elapsed: still cooling down
    CHECK(tower.getBulletRequest() == nullptr);
    tower.Update(0.5f, targets);   // 1.0 s: rate of fire reached, canShoot becomes true...
    CHECK(tower.getBulletRequest() == nullptr);   // ...but the shot waits a frame
    tower.Update(0.1f, targets);   // now canShoot -> fires again
    Game::BulletRequest* second = tower.getBulletRequest();
    CHECK(second != nullptr);
    // The address may coincide with the freed first request (allocator
    // reuse), so assert it is a freshly-queued request, not the same object
    // identity, by checking it is non-null and distinct from the (freed) first.
    // The meaningful guarantee: a new request was produced after the cooldown.
    CHECK(second->damage == 1);
    tower.processShoot();
    CHECK(tower.getBulletRequest() == nullptr);
}

TEST_CASE("Tower Update uses the AoE level's slower 1.5 second rate of fire", "[Tower][Bullet]")
{
    ::Renderer::spriteSheetTexture towerTex;
    towerTex.m_pos_scale.x = 0.0f;
    towerTex.m_pos_scale.y = 0.0f;
    AoeTower tower(5, 5, &towerTex);   // level 1 rate of fire: 1.5 s

    ::Renderer::spriteSheetTexture critterTex;
    critterTex.m_pos_scale.x = 1.0f;
    critterTex.m_pos_scale.y = 1.0f;
    Game::TowerTestCritter critter(&critterTex);
    critter.setPosition(5, 6);
    critter.setIsActive(true);

    std::vector<Game::Critter*> targets{ &critter };

    tower.Update(0.1f, targets);
    REQUIRE(tower.getBulletRequest() != nullptr);
    tower.processShoot();

    // 1.4 s after the shot the tower is still cooling down; a direct
    // tower would already be ready at 1.0 s.
    tower.Update(1.0f, targets);
    CHECK(tower.getBulletRequest() == nullptr);
    tower.Update(0.4f, targets);
    CHECK(tower.getBulletRequest() == nullptr);

    tower.Update(0.1f, targets);   // 1.5 s: canShoot flips...
    CHECK(tower.getBulletRequest() == nullptr);
    tower.Update(0.1f, targets);   // ...and the shot comes the next frame
    CHECK(tower.getBulletRequest() != nullptr);
    tower.processShoot();
}

TEST_CASE("Upgraded towers fire on their level's faster cadence", "[Tower][Bullet]")
{
    ::Renderer::spriteSheetTexture towerTex;
    towerTex.m_pos_scale.x = 0.0f;
    towerTex.m_pos_scale.y = 0.0f;
    SlowTower tower(5, 5, &towerTex);

    ::Renderer::spriteSheetTexture critterTex;
    critterTex.m_pos_scale.x = 1.0f;
    critterTex.m_pos_scale.y = 1.0f;
    Game::TowerTestCritter critter(&critterTex);
    critter.setPosition(5, 6);
    critter.setIsActive(true);

    std::vector<Game::Critter*> targets{ &critter };

    tower.Update(0.1f, targets);
    REQUIRE(tower.getBulletRequest() != nullptr);
    tower.processShoot();

    // Level 1 slow towers wait 1.0 s; at 0.5 s they are still cooling down.
    tower.Update(0.5f, targets);
    CHECK(tower.getBulletRequest() == nullptr);

    // Upgrade to level 3 (rate of fire 0.8 s). The cooldown already paid
    // at level 1 does not carry over: the next shot comes 0.8 s after the
    // last one, not 1.0 s.
    tower.upgrade();
    tower.upgrade();
    tower.Update(0.3f, targets);   // 0.8 s total since the last shot
    CHECK(tower.getBulletRequest() == nullptr);   // flag flips this frame...
    tower.Update(0.1f, targets);   // ...shot lands next frame
    CHECK(tower.getBulletRequest() != nullptr);
    tower.processShoot();
}

TEST_CASE("Tower notifies attached observers when it upgrades", "[Tower][Observer]")
{
    DirectDammageTower tower(0, 0, nullptr);
    Game::RecordingTowerObserver observer(tower);

    tower.upgrade();
    CHECK(observer.callCount == 1);
    CHECK(observer.lastTower == &tower);

    tower.upgrade();
    CHECK(observer.callCount == 2);
}

TEST_CASE("Tower upgrade past max level does not notify", "[Tower][Observer]")
{
    DirectDammageTower tower(0, 0, nullptr);
    Game::RecordingTowerObserver observer(tower);

    tower.upgrade(); // L1 -> L2
    tower.upgrade(); // L2 -> L3 (max)
    REQUIRE(observer.callCount == 2);

    tower.upgrade(); // already maxed out: canUpgrade() is false, upgrade() is a no-op
    CHECK(observer.callCount == 2); // unchanged
}

TEST_CASE("Tower getCurrentLevelCost reflects the cost of the level just reached", "[Tower][Observer]")
{
    // AoeTower's three levels have distinct costs (15, 20, 25), unlike
    // DirectDammageTower's (10, 10, 10), making the per-level value visible.
    AoeTower tower(0, 0, nullptr);
    CHECK(tower.getCurrentLevelCost() == 15);

    tower.upgrade();
    CHECK(tower.getCurrentLevelCost() == 20);

    tower.upgrade();
    CHECK(tower.getCurrentLevelCost() == 25);
}

TEST_CASE("Tower stops notifying a detached observer", "[Tower][Observer]")
{
    DirectDammageTower tower(0, 0, nullptr);
    Game::RecordingTowerObserver observer(tower);

    tower.upgrade();
    REQUIRE(observer.callCount == 1);

    tower.detach(&observer);
    tower.upgrade();
    CHECK(observer.callCount == 1);
}

namespace
{
    /// A live critter at the given cell with the given hit points.
    struct TargetingFixture
    {
        ::Renderer::spriteSheetTexture texture;
        Game::TowerTestCritter critter;

        TargetingFixture(int row, int col, int hitPoints, float x, float y)
            : critter(&texture)
        {
            texture.m_pos_scale.x = x;
            texture.m_pos_scale.y = y;
            critter.setPosition(row, col);
            critter.setIsActive(true);
            critter.takeDamage(10 - hitPoints); // TowerTestCritter starts with 10 hit points
        }
    };
}

TEST_CASE("Each tower type starts with its own default targeting strategy", "[Tower][Strategy]")
{
    DirectDammageTower direct(0, 0, nullptr);
    SlowTower slow(0, 0, nullptr);
    AoeTower aoe(0, 0, nullptr);

    CHECK(direct.getTargetingStrategy().getType() == Game::TargetingStrategyType::Weakest);
    CHECK(slow.getTargetingStrategy().getType() == Game::TargetingStrategyType::NearestToExit);
    CHECK(aoe.getTargetingStrategy().getType() == Game::TargetingStrategyType::Strongest);
}

TEST_CASE("Tower Update aims at whichever critter its targeting strategy chooses", "[Tower][Strategy]")
{
    // Tower at (5, 5), range 2. Critter positions / hit points:
    //   near:   (5, 6) 6 HP  - closest to the tower, middle HP
    //   weak:   (6, 6) 2 HP  - weakest
    //   strong: (4, 4) 9 HP  - strongest
    TargetingFixture near(5, 6, 6, 1.0f, 1.0f);
    TargetingFixture weak(6, 6, 2, 1.0f, 1.0f);
    TargetingFixture strong(4, 4, 9, 1.0f, 1.0f);
    std::vector<Game::Critter*> targets{ &strong.critter, &weak.critter, &near.critter };

    auto firstTarget = [&](Game::TargetingStrategyType type) {
        ::Renderer::spriteSheetTexture towerTex;
        DirectDammageTower tower(5, 5, &towerTex);
        tower.setTargetingStrategy(Game::makeTargetingStrategy(type));
        tower.Update(0.1f, targets);
        REQUIRE(tower.getBulletRequest() != nullptr);
        return tower.getBulletRequest()->target;
    };

    CHECK(firstTarget(Game::TargetingStrategyType::Weakest) == &weak.critter);
    CHECK(firstTarget(Game::TargetingStrategyType::Strongest) == &strong.critter);
    CHECK(firstTarget(Game::TargetingStrategyType::NearestToTower) == &near.critter);
}

TEST_CASE("Changing a tower's strategy changes its next target", "[Tower][Strategy]")
{
    TargetingFixture weak(5, 6, 2, 1.0f, 1.0f);
    TargetingFixture strong(6, 6, 9, 1.0f, 1.0f);
    std::vector<Game::Critter*> targets{ &weak.critter, &strong.critter };

    ::Renderer::spriteSheetTexture towerTex;
    DirectDammageTower tower(5, 5, &towerTex);

    tower.Update(0.1f, targets);
    REQUIRE(tower.getBulletRequest() != nullptr);
    CHECK(tower.getBulletRequest()->target == &weak.critter);
    tower.processShoot();

    // Wait out the cooldown (the shot lands a frame after canShoot flips).
    tower.Update(1.0f, targets);
    tower.setTargetingStrategy(Game::makeTargetingStrategy(Game::TargetingStrategyType::Strongest));
    tower.Update(0.1f, targets);
    REQUIRE(tower.getBulletRequest() != nullptr);
    CHECK(tower.getBulletRequest()->target == &strong.critter);
    tower.processShoot();
}

TEST_CASE("Tower setTargetingStrategy ignores a null strategy", "[Tower][Strategy]")
{
    DirectDammageTower tower(0, 0, nullptr);
    tower.setTargetingStrategy(nullptr);
    CHECK(tower.getTargetingStrategy().getType() == Game::TargetingStrategyType::Weakest);
}

TEST_CASE("Tower cycleTargetingStrategy visits all four strategies and wraps", "[Tower][Strategy]")
{
    DirectDammageTower tower(0, 0, nullptr);
    tower.setTargetingStrategy(Game::makeTargetingStrategy(Game::TargetingStrategyType::NearestToTower));

    std::vector<Game::TargetingStrategyType> seen;
    for (int i = 0; i < 4; ++i)
    {
        tower.cycleTargetingStrategy();
        seen.push_back(tower.getTargetingStrategy().getType());
    }

    CHECK(seen[0] == Game::TargetingStrategyType::NearestToExit);
    CHECK(seen[1] == Game::TargetingStrategyType::Strongest);
    CHECK(seen[2] == Game::TargetingStrategyType::Weakest);
    CHECK(seen[3] == Game::TargetingStrategyType::NearestToTower); // wrapped around
}

TEST_CASE("Changing the targeting strategy does not notify tower observers", "[Tower][Strategy][Observer]")
{
    // Observers treat a Tower notification as an upgrade (billing charges for it).
    DirectDammageTower tower(0, 0, nullptr);
    Game::RecordingTowerObserver observer(tower);

    tower.cycleTargetingStrategy();
    tower.setTargetingStrategy(Game::makeTargetingStrategy(Game::TargetingStrategyType::Strongest));

    CHECK(observer.callCount == 0);
}

TEST_CASE("Dead and out-of-range critters never reach the targeting strategy", "[Tower][Strategy]")
{
    // The ideal pick for every strategy is excluded: a stronger critter that
    // is out of range, and a weaker one that is not active.
    TargetingFixture inRange(5, 6, 5, 1.0f, 1.0f);
    TargetingFixture outOfRange(50, 50, 10, 1.0f, 1.0f);
    TargetingFixture dead(5, 5, 1, 1.0f, 1.0f);
    dead.critter.setIsActive(false);
    std::vector<Game::Critter*> targets{ &outOfRange.critter, &dead.critter, &inRange.critter };

    for (auto type : { Game::TargetingStrategyType::NearestToTower, Game::TargetingStrategyType::NearestToExit,
                       Game::TargetingStrategyType::Strongest, Game::TargetingStrategyType::Weakest })
    {
        ::Renderer::spriteSheetTexture towerTex;
        DirectDammageTower tower(5, 5, &towerTex);
        tower.setTargetingStrategy(Game::makeTargetingStrategy(type));
        tower.Update(0.1f, targets);
        REQUIRE(tower.getBulletRequest() != nullptr);
        CHECK(tower.getBulletRequest()->target == &inRange.critter);
    }
}

#include "CritterFactory.h"
#include "WalkerCritter.h"
#include "SpeedyCritter.h"
#include "BossCritter.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

using namespace Game;

// ──────────────────────────────────────────────────────────────────────────────
//  create() — returns a concrete critter of the requested type
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("CritterFactory::create returns a Walker for CritterType::Walker", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Walker, 1);
    REQUIRE(c != nullptr);
    CHECK(c->getType() == "Walker");
}

TEST_CASE("CritterFactory::create returns a Speedy for CritterType::Speedy", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Speedy, 1);
    REQUIRE(c != nullptr);
    CHECK(c->getType() == "Speedy");
}

TEST_CASE("CritterFactory::create returns a Boss for CritterType::Boss", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Boss, 1);
    REQUIRE(c != nullptr);
    CHECK(c->getType() == "Boss");
}

// ──────────────────────────────────────────────────────────────────────────────
//  Level-1 base stats
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("Walker level 1 base stats", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Walker, 1);
    REQUIRE(c != nullptr);
    CHECK(c->getHitPoints()    == 5);
    CHECK(c->getMaxHitPoints() == 5);
    CHECK(c->getStrength()     == 2);
    CHECK(c->getSpeed()        == 50);
    CHECK(c->getBaseSpeed()    == 50);
    CHECK(c->getReward()       == 5);
    CHECK(c->getLevel()        == 1);
}

TEST_CASE("Speedy level 1 base stats", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Speedy, 1);
    REQUIRE(c != nullptr);
    CHECK(c->getHitPoints()    == 3);
    CHECK(c->getMaxHitPoints() == 3);
    CHECK(c->getStrength()     == 1);
    CHECK(c->getSpeed()        == 150);
    CHECK(c->getBaseSpeed()    == 150);
    CHECK(c->getReward()       == 8);
    CHECK(c->getLevel()        == 1);
}

TEST_CASE("Boss level 1 base stats", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Boss, 1);
    REQUIRE(c != nullptr);
    CHECK(c->getHitPoints()    == 30);
    CHECK(c->getMaxHitPoints() == 30);
    CHECK(c->getStrength()     == 6);
    CHECK(c->getSpeed()        == 25);
    CHECK(c->getBaseSpeed()    == 25);
    CHECK(c->getReward()       == 40);
    CHECK(c->getLevel()        == 1);
}

// ──────────────────────────────────────────────────────────────────────────────
//  Level scaling — HP / strength / reward scale linearly; speed is type-dependent
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("Walker level 3 scales HP, strength and reward but not speed", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Walker, 3);
    REQUIRE(c != nullptr);
    CHECK(c->getHitPoints()    == 15);  // 5  × 3
    CHECK(c->getStrength()     == 6);   // 2  × 3
    CHECK(c->getSpeed()        == 50);  // unchanged
    CHECK(c->getReward()       == 15);  // 5  × 3
    CHECK(c->getLevel()        == 3);
}

TEST_CASE("Speedy level 3 scales HP, strength and reward", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Speedy, 3);
    REQUIRE(c != nullptr);
    CHECK(c->getHitPoints()    == 9);   // 3  × 3
    CHECK(c->getStrength()     == 3);   // 1  × 3
    CHECK(c->getSpeed()        == 150); // unchanged at construction
    CHECK(c->getReward()       == 24);  // 8  × 3
    CHECK(c->getLevel()        == 3);
}

TEST_CASE("Boss level 2 scales HP, strength and reward", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Boss, 2);
    REQUIRE(c != nullptr);
    CHECK(c->getHitPoints()    == 60);  // 30 × 2
    CHECK(c->getStrength()     == 12);  // 6  × 2
    CHECK(c->getSpeed()        == 25);  // unchanged
    CHECK(c->getReward()       == 80);  // 40 × 2
    CHECK(c->getLevel()        == 2);
}

// ──────────────────────────────────────────────────────────────────────────────
//  setLevel() — rescales HP, strength and reward; Speedy also caps speed at min(1, level/2)
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("Walker::setLevel rescales HP, strength and reward", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Walker, 1);
    REQUIRE(c != nullptr);
    c->setLevel(4);
    CHECK(c->getHitPoints()    == 20);  // 5 × 4
    CHECK(c->getMaxHitPoints() == 20);
    CHECK(c->getStrength()     == 8);   // 2 × 4
    CHECK(c->getSpeed()        == 50);  // unchanged
    CHECK(c->getReward()       == 20);  // 5 × 4
}

TEST_CASE("Speedy::setLevel(1) caps speed at 75 (min(1, 1/2) × 150)", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Speedy, 1);
    REQUIRE(c != nullptr);
    c->setLevel(1);
    CHECK(c->getHitPoints() == 3);
    CHECK(c->getSpeed()     == 75);   // 150 × 0.5 (rescaled base_speed)
    // getBaseSpeed() returns the base-class baseSpeed int, which setLevel
    // does not rescale — only getSpeed() tracks the new base_speed value.
    CHECK(c->getBaseSpeed() == 150);
}

TEST_CASE("Speedy::setLevel(2) restores full speed (min(1, 2/2) × 150)", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Speedy, 2);
    REQUIRE(c != nullptr);
    c->setLevel(2);
    CHECK(c->getSpeed()     == 150);
    CHECK(c->getBaseSpeed() == 150);
}

TEST_CASE("Speedy::setLevel(5) holds at full speed (cap at 1.0)", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Speedy, 5);
    REQUIRE(c != nullptr);
    c->setLevel(5);
    CHECK(c->getSpeed()     == 150);
    CHECK(c->getBaseSpeed() == 150);
    CHECK(c->getHitPoints() == 15);  // 3 × 5
}

TEST_CASE("Boss::setLevel rescales HP, strength and reward", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Boss, 1);
    REQUIRE(c != nullptr);
    c->setLevel(3);
    CHECK(c->getHitPoints()    == 90);  // 30 × 3
    CHECK(c->getMaxHitPoints() == 90);
    CHECK(c->getStrength()     == 18);  // 6 × 3
    CHECK(c->getSpeed()        == 25);  // unchanged
    CHECK(c->getReward()       == 120); // 40 × 3
}

// ──────────────────────────────────────────────────────────────────────────────
//  Clone
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("clone returns a new critter with the same type and stats", "[CritterFactory]")
{
    auto c = CritterFactory::create(CritterType::Walker, 2);
    REQUIRE(c != nullptr);
    auto c2 = c->clone();
    REQUIRE(c2 != nullptr);
    CHECK(c2->getType() == "Walker");
    CHECK(c2->getHitPoints() == 10);
    CHECK(c2->getStrength()  == 4);
}

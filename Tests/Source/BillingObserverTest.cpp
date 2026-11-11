#include "Player.h"
#include "TowerBillingObserver.h"
#include "CritterBillingObserver.h"
#include "DirectDammageTower.h"
#include "WalkerCritter.h"
#include "Texture.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

using Game::DirectDammageTower;
using Game::WalkerCritter;

TEST_CASE("TowerBillingObserver charges the player the cost of the level just reached", "[Observer][Billing]")
{
    App::Player player;
    DirectDammageTower tower(0, 0, nullptr);
    App::TowerBillingObserver observer(tower, player);

    float before = player.GetCoins();
    tower.upgrade(); // L1 -> L2, costs LEVEL_CONFIGS[1].levelCost (10)
    CHECK(player.GetCoins() == before - 10);

    before = player.GetCoins();
    tower.upgrade(); // L2 -> L3, costs LEVEL_CONFIGS[2].levelCost (10)
    CHECK(player.GetCoins() == before - 10);
}

TEST_CASE("TowerBillingObserver charges nothing when upgrade is a no-op past max level", "[Observer][Billing]")
{
    DirectDammageTower tower(0, 0, nullptr);
    App::Player player;
    App::TowerBillingObserver observer(tower, player);

    tower.upgrade();
    tower.upgrade(); // now maxed out
    float before = player.GetCoins();

    tower.upgrade(); // no-op: canUpgrade() is false, upgrade() returns before notify()
    CHECK(player.GetCoins() == before); // unchanged
}

TEST_CASE("CritterBillingObserver rewards the player when a critter is killed", "[Observer][Billing]")
{
    ::Renderer::spriteSheetTexture texture;
    WalkerCritter critter(1, 0, 0, &texture); // level 1: reward 5

    App::Player player;
    App::CritterBillingObserver observer(critter, player);

    float before = player.GetCoins();
    critter.takeDamage(999); // lethal hit
    CHECK(player.GetCoins() == before + 5);
}

TEST_CASE("CritterBillingObserver does not reward twice for overkill hits on an already-dead critter", "[Observer][Billing]")
{
    ::Renderer::spriteSheetTexture texture;
    WalkerCritter critter(1, 0, 0, &texture);

    App::Player player;
    App::CritterBillingObserver observer(critter, player);

    critter.takeDamage(999); // kills it, rewards once
    float afterFirstKill = player.GetCoins();

    critter.takeDamage(999); // overkill: already killed, must not notify/reward again
    CHECK(player.GetCoins() == afterFirstKill);
}

TEST_CASE("CritterBillingObserver charges strength-per-second while idling at the exit", "[Observer][Billing]")
{
    ::Renderer::spriteSheetTexture texture;
    WalkerCritter critter(1, 0, 0, &texture); // level 1: strength 2

    App::Player player;
    App::CritterBillingObserver observer(critter, player);

    critter.reachedEnd = true;
    critter.lastDeltaTime = 0.5f; // half a second since the last Update()

    float before = player.GetCoins();
    critter.setIsActive(true); // any call that notifies while reachedEnd is set

    CHECK(player.GetCoins() == before - (2 * 0.5f)); // strength(2) * lastDeltaTime(0.5)
}

TEST_CASE("CritterBillingObserver costs one health point the instant a critter reaches the exit", "[Observer][Billing]")
{
    ::Renderer::spriteSheetTexture texture;
    WalkerCritter critter(1, 0, 0, &texture);

    App::Player player;
    App::CritterBillingObserver observer(critter, player);

    critter.reachedEnd = true;
    critter.lastDeltaTime = 0.0f; // isolate the health cost from the coin drain

    int before = player.GetHealth();
    critter.setIsActive(true); // notifies while reachedEnd is set

    CHECK(player.GetHealth() == before - 1);
}

TEST_CASE("CritterBillingObserver only costs health once per trip to the exit", "[Observer][Billing]")
{
    ::Renderer::spriteSheetTexture texture;
    WalkerCritter critter(1, 0, 0, &texture);

    App::Player player;
    App::CritterBillingObserver observer(critter, player);

    critter.reachedEnd = true;
    critter.lastDeltaTime = 0.0f;

    critter.setIsActive(true); // first leak: costs 1 health
    int afterFirstLeak = player.GetHealth();

    critter.setIsActive(true); // still idling at the exit: no further cost
    CHECK(player.GetHealth() == afterFirstLeak);

    // Once the critter is no longer at the exit (e.g. Death()/resetCritter()
    // clears reachedEnd for a later wave) and leaks again, it must cost
    // health again.
    critter.reachedEnd = false;
    critter.setIsActive(false); // notifies with reachedEnd cleared

    critter.reachedEnd = true;
    critter.setIsActive(true);
    CHECK(player.GetHealth() == afterFirstLeak - 1);
}

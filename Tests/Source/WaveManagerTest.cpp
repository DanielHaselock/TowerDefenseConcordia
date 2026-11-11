#include "WaveManager.h"
#include "Critter.h"
#include "WalkerCritter.h"
#include "BossCritter.h"
#include "Texture.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

#include <cstddef>
#include <vector>

namespace
{
    /// Pool critter with its own (headless-safe) sprite, so that
    /// WaveManager can activate it via setIsActive() without a null texture.
    class PoolCritter : public Game::Critter
    {
    public:
        ::Renderer::spriteSheetTexture tex;

        explicit PoolCritter(int hitPoints = 10, int level = 1)
            : Critter(hitPoints, 2, 40, level, 5, 0, 0, &tex)
        {
        }

        std::string getType() const override { return "Pool"; }
        std::unique_ptr<Game::Critter> clone() const override { return std::make_unique<PoolCritter>(*this); }
        void Update(float, const std::vector<::Renderer::RendererMap*>&) override {}
        void setLevel(int p_level) override { level = p_level; }
    };

    /// Pool sized to the full layout WaveManager expects:
    /// 50 walkers, then 50 speedy, then 15 bosses.
    std::vector<PoolCritter*> makeCritterPool()
    {
        std::vector<PoolCritter*> pool;
        pool.reserve(MAX_WALKER_POOL + MAX_SPEED_POOL + MAX_BOSS_POOL);
        for (std::size_t i = 0; i < MAX_WALKER_POOL + MAX_SPEED_POOL + MAX_BOSS_POOL; ++i)
        {
            pool.push_back(new PoolCritter());
        }
        return pool;
    }

    int countActive(const std::vector<PoolCritter*>& pool)
    {
        int active = 0;
        for (auto* critter : pool)
        {
            if (critter->getIsActive())
            {
                ++active;
            }
        }
        return active;
    }
}

namespace App
{
    /// Access to WaveManager's private precomputed stats.
    class WaveManagerTest
    {
    public:
        static std::size_t statsSize(const WaveManager& wm) { return wm.stats.size(); }
        static WaveStats statsAt(const WaveManager& wm, std::size_t index) { return wm.stats.at(index); }
        static int currentWave(const WaveManager& wm) { return wm.currWave; }
        static WaveState state(const WaveManager& wm) { return wm.waveState; }
    };
}

using App::WaveManagerTest;

TEST_CASE("WaveManager precomputes 100 waves at construction", "[WaveManager]")
{
    auto pool = makeCritterPool();
    std::vector<Game::Critter*> raw(pool.size());
    for (std::size_t i = 0; i < pool.size(); ++i)
    {
        raw[i] = pool[i];
    }
    App::WaveManager wm(raw);

    CHECK(WaveManagerTest::statsSize(wm) == 100);
    CHECK(WaveManagerTest::currentWave(wm) == 0);
    CHECK(WaveManagerTest::state(wm) == App::WaveState::waiting);

    for (auto* c : pool)
    {
        delete c;
    }
}

TEST_CASE("WaveManager wave composition follows the documented formulas", "[WaveManager]")
{
    auto pool = makeCritterPool();
    std::vector<Game::Critter*> raw(pool.size());
    for (std::size_t i = 0; i < pool.size(); ++i)
    {
        raw[i] = pool[i];
    }
    App::WaveManager wm(raw);

    // Wave n: total = 5 + 2n; boss = min(50%, 1%*n); speedy = min(40%, 10% + 2%*n);
    // walkers make up the remainder.
    struct Expected
    {
        std::size_t index; // 0-based stats index
        int walker;
        int speedy;
        int boss;
    };

    const Expected expectations[] = {
        {0, 7, 0, 0},    // wave 1: total 7, no boss/speedy
        {2, 10, 1, 0},  // wave 3: total 11, boss int(11*0.03)=0, speedy int(11*0.16)=1
        {9, 16, 7, 2},  // wave 10: total 25, boss int(25*0.10)=2, speedy int(25*0.30)=7
        {24, 20, 22, 13}, // wave 25: total 55, boss int(55*0.25)=13, speedy int(55*0.40)=22
        {49, 11, 42, 52} // wave 50: total 105, both probabilities at their caps
    };

    for (const auto& e : expectations)
    {
        INFO("wave " << (e.index + 1));
        App::WaveStats s = WaveManagerTest::statsAt(wm, e.index);
        CHECK(s.amountWalker == e.walker);
        CHECK(s.amountSpeedy == e.speedy);
        CHECK(s.amountBoss == e.boss);
        // The per-type counts must always sum to the wave total.
        CHECK(s.amountWalker + s.amountSpeedy + s.amountBoss == 5 + 2 * static_cast<int>(e.index + 1));
    }

    // GetWaveStats() reports the current wave (index 0 until the first IncreaseWave).
    App::WaveStats first = wm.GetWaveStats();
    CHECK(first.amountWalker == 7);
    CHECK(first.amountSpeedy == 0);
    CHECK(first.amountBoss == 0);

    for (auto* c : pool)
    {
        delete c;
    }
}

TEST_CASE("WaveManager raises the pool level every third wave", "[WaveManager]")
{
    auto pool = makeCritterPool();
    std::vector<Game::Critter*> raw(pool.size());
    for (std::size_t i = 0; i < pool.size(); ++i)
    {
        raw[i] = pool[i];
    }
    App::WaveManager wm(raw);

    // Level-ups happen on calls 1, 4, 7, ... (when currWave % 3 == 0),
    // setting the pool to level currWave/3 + 1.
    wm.IncreaseWave();
    REQUIRE(pool[0]->getLevel() == 1);
    REQUIRE(pool[60]->getLevel() == 1);

    wm.IncreaseWave();
    wm.IncreaseWave();
    CHECK(pool[0]->getLevel() == 1); // not yet on a level-up wave

    wm.IncreaseWave();
    CHECK(pool[0]->getLevel() == 2);
    CHECK(pool[100]->getLevel() == 2); // every critter in the pool is rescaled

    wm.IncreaseWave();
    wm.IncreaseWave();
    CHECK(pool[0]->getLevel() == 2);

    wm.IncreaseWave();
    CHECK(pool[0]->getLevel() == 3);

    for (auto* c : pool)
    {
        delete c;
    }
}

TEST_CASE("WaveManager level-ups rescale real critters' strength and reward", "[WaveManager][Critter]")
{
    ::Renderer::spriteSheetTexture tex;
    std::vector<Game::Critter*> pool;
    pool.push_back(new Game::WalkerCritter(1, 0, 0, &tex));
    pool.push_back(new Game::BossCritter(1, 0, 0, &tex));
    App::WaveManager wm(pool);

    // Built at level 1: walker 5 HP / 2 strength / 5 reward, boss 30 / 6 / 40.
    CHECK(pool[0]->getStrength() == 2);
    CHECK(pool[0]->getReward() == 5);

    // The level-up on the fourth IncreaseWave (currWave 3) lifts the whole pool to level 2.
    wm.IncreaseWave();
    wm.IncreaseWave();
    wm.IncreaseWave();
    wm.IncreaseWave();

    CHECK(pool[0]->getLevel() == 2);
    CHECK(pool[0]->getHitPoints() == 10);   // walker: 5 HP per level
    CHECK(pool[0]->getStrength() == 4);     // walker: 2 strength per level
    CHECK(pool[0]->getReward() == 10);      // walker: 5 reward per level

    CHECK(pool[1]->getLevel() == 2);
    CHECK(pool[1]->getHitPoints() == 60);   // boss: 30 HP per level
    CHECK(pool[1]->getStrength() == 12);    // boss: 6 strength per level
    CHECK(pool[1]->getReward() == 80);      // boss: 40 reward per level

    for (auto* c : pool)
    {
        delete c;
    }
}

TEST_CASE("WaveManager CheckWaveEnd reports whether every critter is dead", "[WaveManager]")
{
    auto pool = makeCritterPool();
    std::vector<Game::Critter*> raw(pool.size());
    for (std::size_t i = 0; i < pool.size(); ++i)
    {
        raw[i] = pool[i];
    }
    App::WaveManager wm(raw);

    CHECK(wm.CheckWaveEnd()); // an unspawned pool is entirely "dead"

    pool[0]->setIsActive(true);
    CHECK_FALSE(wm.CheckWaveEnd());

    pool[0]->setIsActive(false);
    CHECK(wm.CheckWaveEnd());

    for (auto* c : pool)
    {
        delete c;
    }
}

TEST_CASE("WaveManager CheckWaveEnd treats a leaked critter as resolved even while still active", "[WaveManager]")
{
    auto pool = makeCritterPool();
    std::vector<Game::Critter*> raw(pool.size());
    for (std::size_t i = 0; i < pool.size(); ++i)
    {
        raw[i] = pool[i];
    }
    App::WaveManager wm(raw);

    // Still active and not dead, but it has reached the exit: a leaked
    // critter shouldn't block the wave from being reported cleared while it
    // keeps draining coins in the background (see GameManager::MainLoop()).
    pool[0]->setIsActive(true);
    pool[0]->reachedEnd = true;
    CHECK(wm.CheckWaveEnd());

    for (auto* c : pool)
    {
        delete c;
    }
}

TEST_CASE("WaveManager walks through waiting, spawning and playing for the first spawned wave", "[WaveManager]")
{
    auto pool = makeCritterPool();
    std::vector<Game::Critter*> raw(pool.size());
    for (std::size_t i = 0; i < pool.size(); ++i)
    {
        raw[i] = pool[i];
    }
    App::WaveManager wm(raw);

    REQUIRE(WaveManagerTest::state(wm) == App::WaveState::waiting);

    // Between waves the manager holds until the player asks for the next
    // wave; frames alone never start it and never activate a critter.
    wm.Update(0.5f);
    wm.Update(0.5f);
    wm.Update(0.5f);
    REQUIRE(wm.IsWaitingForPlayer());
    REQUIRE(countActive(pool) == 0);

    wm.StartNextWave();
    REQUIRE(WaveManagerTest::state(wm) == App::WaveState::spawning);
    REQUIRE(WaveManagerTest::currentWave(wm) == 1);

    // The first spawned wave is wave 2 (IncreaseWave() runs before the first
    // spawn, so currWave is already 1): total 9, 8 walkers + 1 speedy,
    // one critter activated every 0.5 s.
    App::WaveStats stats = wm.GetWaveStats();
    REQUIRE(stats.amountWalker == 8);
    REQUIRE(stats.amountSpeedy == 1);
    REQUIRE(stats.amountBoss == 0);

    for (int i = 0; i < 8; ++i)
    {
        wm.Update(0.5f);
        REQUIRE(countActive(pool) == i + 1);
        REQUIRE(WaveManagerTest::state(wm) == App::WaveState::spawning);
    }

    // The 9th spawn activates the final critter but the state stays in
    // spawning: SpawnCritter() only flips to playing on the *next* call, when
    // amountCritterSpawned < total is false (the else branch).
    wm.Update(0.5f);
    REQUIRE(countActive(pool) == 9);
    REQUIRE(WaveManagerTest::state(wm) == App::WaveState::spawning);

    // The next spawn call sees the wave complete and switches to playing.
    wm.Update(0.5f);
    REQUIRE(WaveManagerTest::state(wm) == App::WaveState::playing);

    // While critters are still alive the wave keeps playing.
    wm.Update(1.0f);
    REQUIRE(WaveManagerTest::state(wm) == App::WaveState::playing);

    // Once every critter is dead the wave ends and the manager waits again.
    for (auto* c : pool)
    {
        c->setIsActive(false);
    }
    wm.Update(0.1f);
    CHECK(WaveManagerTest::state(wm) == App::WaveState::waiting);

    // ...and it keeps waiting, no matter how many frames pass.
    wm.Update(1.0f);
    wm.Update(1.0f);
    CHECK(wm.IsWaitingForPlayer());
    CHECK(countActive(pool) == 0);

    // The player starts the next wave: currWave was 1, so it becomes 2.
    wm.StartNextWave();
    CHECK(WaveManagerTest::state(wm) == App::WaveState::spawning);
    CHECK(WaveManagerTest::currentWave(wm) == 2);
    CHECK(!wm.IsWaitingForPlayer());

    // StartNextWave is a no-op once the wave is underway.
    wm.StartNextWave();
    CHECK(WaveManagerTest::currentWave(wm) == 2);

    for (auto* c : pool)
    {
        delete c;
    }
}

TEST_CASE("WaveManager GetCurrentWave tracks the wave counter", "[WaveManager]")
{
    auto pool = makeCritterPool();
    std::vector<Game::Critter*> raw(pool.size());
    for (std::size_t i = 0; i < pool.size(); ++i)
    {
        raw[i] = pool[i];
    }
    App::WaveManager waveManager(raw);

    // No wave has started yet.
    CHECK(waveManager.GetCurrentWave() == 0);

    waveManager.IncreaseWave();
    CHECK(waveManager.GetCurrentWave() == 1);

    waveManager.IncreaseWave();
    CHECK(waveManager.GetCurrentWave() == 2);

    for (auto* critter : pool)
    {
        delete critter;
    }
}

TEST_CASE("WaveManager GetRandomNumber stays within the inclusive range", "[WaveManager]")
{
    auto pool = makeCritterPool();
    std::vector<Game::Critter*> raw(pool.size());
    for (std::size_t i = 0; i < pool.size(); ++i)
    {
        raw[i] = pool[i];
    }
    App::WaveManager wm(raw);

    const int a = 3;
    const int b = 5;
    int minSeen = b;
    int maxSeen = a;

    for (int i = 0; i < 2000; ++i)
    {
        int value = wm.GetRandomNumber(a, b);
        CHECK(value >= a);
        CHECK(value <= b);
        minSeen = std::min(minSeen, value);
        maxSeen = std::max(maxSeen, value);
    }

    // Over 2000 draws all values of the small range must appear at least once.
    CHECK(minSeen == a);
    CHECK(maxSeen == b);

    for (auto* c : pool)
    {
        delete c;
    }
}

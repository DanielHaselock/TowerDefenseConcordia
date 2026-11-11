#include "Critter.h"
#include "WalkerCritter.h"
#include "SpeedyCritter.h"
#include "BossCritter.h"
#include "Texture.h"
#include "RendererMap.h"
#include "Vector4.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

#include <cmath>
#include <memory>
#include <vector>

namespace Game
{
    /// Minimal concrete Critter so the abstract base class can be exercised directly.
    class TestCritter : public Critter
    {
    public:
        explicit TestCritter(int hitPoints = 10, int strength = 2, int speed = 40,
                             int level = 1, int reward = 5, int spawnRow = 0,
                             int spawnCol = 0, ::Renderer::spriteSheetTexture* texture = nullptr)
            : Critter(hitPoints, strength, speed, level, reward, spawnRow, spawnCol, texture)
        {
        }

        std::string getType() const override { return "Test"; }
        std::unique_ptr<Critter> clone() const override { return std::make_unique<TestCritter>(*this); }
        void Update(float deltaTime, const std::vector<::Renderer::RendererMap*>& tiles) override
        {
            Critter::Update(deltaTime, tiles);
        }
        void setLevel(int p_level) override { level = p_level; }
    };

    /// Records every notify() call a Critter sends it.
    class RecordingCritterObserver : public Observer<Critter>
    {
    public:
        explicit RecordingCritterObserver(Critter& critter) : Observer<Critter>(critter) {}

        int callCount = 0;

        void update(const Critter&) override
        {
            ++callCount;
        }

    };
    // Builds a headless renderer grid so Critter pathing tests can exercise
    // real RendererMap tile lookups without creating an SDL window.
    class RendererTileGrid
    {
    public:
        RendererTileGrid(int rows, int columns)
        {
            // Store tiles in row-major order, matching Map's flat path indices.
            for (int row = 0; row < rows; ++row)
            {
                for (int column = 0; column < columns; ++column)
                {
                    const int cellNumber = row * columns + column;
                    const Math::Vec4<int> position(column * 10, row * 10, 10, 10);
                    auto* ground = texture.useSpecificSpriteSheet(0, 0, 1, 1, position);
                    auto* infected = texture.useSpecificSpriteSheet(0, 0, 1, 1, position);
                    // RendererMap references these sprites; texture owns them.
                    ownedTiles.emplace_back(std::make_unique<::Renderer::RendererMap>(
                        ::Renderer::TileType::scenery, ground, infected, &texture, 0, cellNumber));
                    tiles.emplace_back(ownedTiles.back().get());
                }
            }
        }

        // Keep the texture alive longer than RendererMap and its sprite references.
        ::Renderer::Texture texture{ 0, 0, 0, 0 };
        // Own tiles separately while exposing the raw-pointer view used by Critter.
        std::vector<std::unique_ptr<::Renderer::RendererMap>> ownedTiles;
        std::vector<::Renderer::RendererMap*> tiles;
    };

    /// Base-class accessors. The members they read are private to Critter.
    class CritterTest
    {
    public:
        static int hitPoints(const Critter& c) { return c.getHitPoints(); }
        static int maxHitPoints(const Critter& c) { return c.getMaxHitPoints(); }
        static int strength(const Critter& c) { return c.getStrength(); }
        static int speed(const Critter& c) { return c.getSpeed(); }
        static int baseSpeed(const Critter& c) { return c.getBaseSpeed(); }
        static int level(const Critter& c) { return c.getLevel(); }
        static int reward(const Critter& c) { return c.getReward(); }
        static int row(const Critter& c) { return c.getRow(); }
        static int col(const Critter& c) { return c.getCol(); }
        static bool active(Critter& c) { return c.getIsActive(); }
        static bool dead(const Critter& c) { return c.isDead(); }
        static void takeDamage(Critter& c, int amount) { c.takeDamage(amount); }
        static void setLevel(Critter& c, int lvl) { c.setLevel(lvl); }
        static void setPosition(Critter& c, int r, int cl) { c.setPosition(r, cl); }
    };
}

using Game::CritterTest;

TEST_CASE("WalkerCritter base stats match its documented values", "[Critter][Walker]")
{
    ::Renderer::spriteSheetTexture texture;
    Game::WalkerCritter walker(1, 2, 3, &texture);

    CHECK(walker.getType() == "Walker");
    CHECK(CritterTest::hitPoints(walker) == 5);
    CHECK(CritterTest::maxHitPoints(walker) == 5);
    CHECK(CritterTest::strength(walker) == 2);
    CHECK(CritterTest::speed(walker) == 50);
    CHECK(CritterTest::level(walker) == 1);
    CHECK(CritterTest::reward(walker) == 5);
    CHECK(CritterTest::row(walker) == 2);
    CHECK(CritterTest::col(walker) == 3);
}

TEST_CASE("WalkerCritter scales HP, strength and reward linearly with level", "[Critter][Walker]")
{
    ::Renderer::spriteSheetTexture texture;
    Game::WalkerCritter walker(1, 0, 0, &texture);

    // Speed must stay at the type's base speed regardless of level.
    CritterTest::setLevel(walker, 5);

    // WalkerCritter::setLevel rescales hit points, strength and reward by
    // the level; speed is not part of the walker's level scaling.
    CHECK(CritterTest::hitPoints(walker) == 25);
    CHECK(CritterTest::maxHitPoints(walker) == 25);
    CHECK(CritterTest::strength(walker) == 10);
    CHECK(CritterTest::reward(walker) == 25);
    CHECK(CritterTest::speed(walker) == 50);
    CHECK(CritterTest::level(walker) == 5);
}

TEST_CASE("SpeedyCritter base stats match its documented values", "[Critter][Speedy]")
{
    ::Renderer::spriteSheetTexture texture;
    Game::SpeedyCritter speedy(1, 1, 2, &texture);

    CHECK(speedy.getType() == "Speedy");
    CHECK(CritterTest::hitPoints(speedy) == 3);
    CHECK(CritterTest::strength(speedy) == 1);
    CHECK(CritterTest::speed(speedy) == 150);
    CHECK(CritterTest::reward(speedy) == 8);
    CHECK(CritterTest::row(speedy) == 1);
    CHECK(CritterTest::col(speedy) == 2);
}

TEST_CASE("SpeedyCritter setLevel ramps speed to full base speed by level 2", "[Critter][Speedy]")
{
    ::Renderer::spriteSheetTexture texture;
    Game::SpeedyCritter speedy(1, 0, 0, &texture);

    CritterTest::setLevel(speedy, 1);
    REQUIRE(CritterTest::hitPoints(speedy) == 3);
    REQUIRE(CritterTest::strength(speedy) == 1);
    REQUIRE(CritterTest::reward(speedy) == 8);
    REQUIRE(CritterTest::speed(speedy) == 75); // min(1, 1/2) factor

    CritterTest::setLevel(speedy, 2);
    REQUIRE(CritterTest::hitPoints(speedy) == 6);
    REQUIRE(CritterTest::strength(speedy) == 2);
    REQUIRE(CritterTest::reward(speedy) == 16);
    REQUIRE(CritterTest::speed(speedy) == 150); // capped at BASE_SPEED

    CritterTest::setLevel(speedy, 6);
    REQUIRE(CritterTest::hitPoints(speedy) == 18);
    REQUIRE(CritterTest::strength(speedy) == 6);
    REQUIRE(CritterTest::reward(speedy) == 48);
    REQUIRE(CritterTest::speed(speedy) == 150);
}

TEST_CASE("BossCritter base stats match its documented values", "[Critter][Boss]")
{
    ::Renderer::spriteSheetTexture texture;
    Game::BossCritter boss(1, 4, 5, &texture);

    CHECK(boss.getType() == "Boss");
    CHECK(CritterTest::hitPoints(boss) == 30);
    CHECK(CritterTest::strength(boss) == 6);
    CHECK(CritterTest::speed(boss) == 25);
    CHECK(CritterTest::reward(boss) == 40);

    CritterTest::setLevel(boss, 2);
    CHECK(CritterTest::hitPoints(boss) == 60);
    CHECK(CritterTest::maxHitPoints(boss) == 60);
    CHECK(CritterTest::strength(boss) == 12);
    CHECK(CritterTest::reward(boss) == 80);
    CHECK(CritterTest::level(boss) == 2);
}

TEST_CASE("Critter constructor stores its base stats and spawn position", "[Critter]")
{
    Game::TestCritter critter(42, 3, 60, 4, 21, 7, 8);

    CHECK(CritterTest::hitPoints(critter) == 42);
    CHECK(CritterTest::maxHitPoints(critter) == 42);
    CHECK(CritterTest::strength(critter) == 3);
    CHECK(CritterTest::speed(critter) == 60);
    CHECK(CritterTest::baseSpeed(critter) == 60);
    CHECK(CritterTest::level(critter) == 4);
    CHECK(CritterTest::reward(critter) == 21);
    CHECK(CritterTest::row(critter) == 7);
    CHECK(CritterTest::col(critter) == 8);
}

TEST_CASE("Critter takeDamage reduces hit points", "[Critter]")
{
    Game::TestCritter critter;

    CritterTest::takeDamage(critter, 4);
    CHECK(CritterTest::hitPoints(critter) == 6);

    CritterTest::takeDamage(critter, 3);
    CHECK(CritterTest::hitPoints(critter) == 3);
}

TEST_CASE("Critter takeDamage ignores negative amounts", "[Critter]")
{
    Game::TestCritter critter;

    CritterTest::takeDamage(critter, -10);
    CHECK(CritterTest::hitPoints(critter) == 10);
}

TEST_CASE("Critter takeDamage past zero flags the critter as killed", "[Critter]")
{
    Game::TestCritter critter;

    CritterTest::takeDamage(critter, 10);
    CHECK(critter.killed);
    CHECK(CritterTest::dead(critter)); // a fresh critter is inactive, so isDead() is already true

    // Documented discrepancy: the header says takeDamage clamps hit points at 0,
    // but the implementation only sets the killed flag - overkill leaves
    // negative hit points. A developer should reconcile the docs with the code.
    CritterTest::takeDamage(critter, 10);
    CHECK(critter.getHitPoints() == -10);
}

TEST_CASE("Critter setPosition updates its grid coordinates", "[Critter]")
{
    Game::TestCritter critter;

    CritterTest::setPosition(critter, 3, 4);
    CHECK(CritterTest::row(critter) == 3);
    CHECK(CritterTest::col(critter) == 4);
}

TEST_CASE("Critter path setup uses row-major tile indices on a non-square map", "[Critter][path]")
{
    const int rows = 3;
    const int columns = 5;
    Game::RendererTileGrid grid(rows, columns);
    ::Renderer::spriteSheetTexture critterTexture;
    Game::TestCritter critter(10, 2, 40, 1, 5, 0, 0, &critterTexture);

    // Set the critter's current tile to flat index 14 (next move is to tile 13).
    critter.setPath(grid.tiles, {14, 13}, columns);

    // flat index 14 is row 2, column 4 in 3x5 grid
    CHECK(CritterTest::row(critter) == 2);
    CHECK(CritterTest::col(critter) == 4);
    CHECK(critter.currentTile == grid.tiles[14]);
}

TEST_CASE("Critter changes to the next row-major path tile when crossing rows", "[Critter][path]")
{
    const int rows = 4;
    const int columns = 7;
    Game::RendererTileGrid grid(rows, columns);
    ::Renderer::spriteSheetTexture critterTexture;
    Game::TestCritter critter(10, 2, 40, 1, 5, 0, 0, &critterTexture);
    // indices 20 and 27 are vertically adjacent in column 6 (4x7 grid)
    critter.setPath(grid.tiles, {20, 27}, columns);
    critter.setIsActive(true);

    critter.Update(0.1f, grid.tiles);
    REQUIRE(critter.currentTile == grid.tiles[20]);

    critter.Update(0.1f, grid.tiles);

    CHECK(CritterTest::row(critter) == 3);
    CHECK(CritterTest::col(critter) == 6);
    CHECK(critter.currentTile == grid.tiles[27]);
}

TEST_CASE("Critter halfSpeed halves the speed once and does not stack", "[Critter]")
{
    Game::TestCritter critter; // speed 40

    critter.halfSpeed();
    REQUIRE(CritterTest::speed(critter) == 20);
    REQUIRE(critter.speedHalfed);

    critter.halfSpeed();
    CHECK(CritterTest::speed(critter) == 20); // repeated hits must not stack the slow
    CHECK(critter.speedHalfed);
}

TEST_CASE("Critter clone produces an independent copy of the same type", "[Critter]")
{
    Game::TestCritter critter(20, 2, 40, 1, 5, 0, 0);
    CritterTest::takeDamage(critter, 5);

    std::unique_ptr<Game::Critter> copy = critter.clone();

    REQUIRE(copy != nullptr);
    CHECK(copy->getType() == "Test");
    CHECK(CritterTest::hitPoints(*copy) == 15);

    // The clone must not share state with the original.
    CritterTest::takeDamage(*copy, 5);
    CHECK(CritterTest::hitPoints(critter) == 15);
    CHECK(CritterTest::hitPoints(*copy) == 10);
}

TEST_CASE("Concrete critters clone as their own type", "[Critter][clone]")
{
    ::Renderer::spriteSheetTexture texture;

    Game::WalkerCritter walker(2, 0, 0, &texture);
    std::unique_ptr<Game::Critter> walkerCopy = walker.clone();
    CHECK(walkerCopy->getType() == "Walker");
    CHECK(CritterTest::hitPoints(*walkerCopy) == 10);

    Game::SpeedyCritter speedy(3, 0, 0, &texture);
    std::unique_ptr<Game::Critter> speedyCopy = speedy.clone();
    CHECK(speedyCopy->getType() == "Speedy");
    CHECK(CritterTest::hitPoints(*speedyCopy) == 9);

    Game::BossCritter boss(2, 0, 0, &texture);
    std::unique_ptr<Game::Critter> bossCopy = boss.clone();
    CHECK(bossCopy->getType() == "Boss");
    CHECK(CritterTest::hitPoints(*bossCopy) == 60);
}

TEST_CASE("Critter takeDamage notifies on every hit up to and including the kill", "[Critter][Observer]")
{
    Game::TestCritter critter; // 10 HP
    Game::RecordingCritterObserver observer(critter);

    CritterTest::takeDamage(critter, 4); // non-lethal: HP changed, must notify
    CHECK(observer.callCount == 1);

    CritterTest::takeDamage(critter, 6); // lethal: HP reaches 0, killed flips true
    CHECK(observer.callCount == 2);
    CHECK(critter.killed);
}

TEST_CASE("Critter takeDamage does not notify again for overkill hits on an already-dead critter", "[Critter][Observer]")
{
    Game::TestCritter critter; // 10 HP
    Game::RecordingCritterObserver observer(critter);

    CritterTest::takeDamage(critter, 10); // kills it
    REQUIRE(observer.callCount == 1);

    // Overlapping hits (e.g. AOE splash) landing after the kill must not
    // notify again, even though hit points still go further negative.
    CritterTest::takeDamage(critter, 10);
    CHECK(observer.callCount == 1); // unchanged
    CHECK(critter.getHitPoints() == -10);
}

TEST_CASE("Critter setIsActive notifies observers", "[Critter][Observer]")
{
    ::Renderer::spriteSheetTexture texture;
    Game::TestCritter critter(10, 2, 40, 1, 5, 0, 0, &texture);
    Game::RecordingCritterObserver observer(critter);

    critter.setIsActive(true);
    CHECK(observer.callCount == 1);

    critter.setIsActive(false);
    CHECK(observer.callCount == 2);
}

TEST_CASE("Critter stops notifying a detached observer", "[Critter][Observer]")
{
    Game::TestCritter critter;
    Game::RecordingCritterObserver observer(critter);

    CritterTest::takeDamage(critter, 1);
    REQUIRE(observer.callCount == 1);

    critter.detach(&observer);
    CritterTest::takeDamage(critter, 1);
    CHECK(observer.callCount == 1); // unchanged
}

TEST_CASE("Critter without a path has no distance left to the exit", "[Critter][path]")
{
    Game::TestCritter critter;
    CHECK(critter.getDistanceToExit() == 0.0f);
}

TEST_CASE("Critter distance to the exit shrinks all the way along the path", "[Critter][path]")
{
    // One row of four tiles, 10 px apart: the path is 30 px long.
    Game::RendererTileGrid grid(1, 4);
    ::Renderer::spriteSheetTexture critterTexture;
    Game::TestCritter critter(10, 2, 40, 1, 5, 0, 0, &critterTexture);
    critter.setPath(grid.tiles, {0, 1, 2, 3}, 4);
    critter.setIsActive(true);

    float previous = critter.getDistanceToExit();
    CHECK(std::abs(previous - 30.0f) < 0.001f);

    // Every step moves the critter forward (or, for the frame spent
    // switching destination, leaves it where it is), so the distance never
    // grows and ends close to zero.
    bool shrankAtLeastOnce = false;
    // Small steps (0.4 px each), like the real game's frame times.
    for (int step = 0; step < 200; ++step)
    {
        critter.Update(0.01f, grid.tiles);
        float current = critter.getDistanceToExit();
        CHECK(current <= previous + 0.001f);
        shrankAtLeastOnce = shrankAtLeastOnce || current < previous - 0.001f;
        previous = current;
    }

    CHECK(shrankAtLeastOnce);
    CHECK(previous < 1.0f);
}

TEST_CASE("Critter further along its path is closer to the exit", "[Critter][path]")
{
    Game::RendererTileGrid grid(1, 6);
    ::Renderer::spriteSheetTexture textureA;
    ::Renderer::spriteSheetTexture textureB;
    Game::TestCritter leader(10, 2, 40, 1, 5, 0, 0, &textureA);
    Game::TestCritter follower(10, 2, 40, 1, 5, 0, 0, &textureB);
    leader.setPath(grid.tiles, {0, 1, 2, 3, 4, 5}, 6);
    follower.setPath(grid.tiles, {0, 1, 2, 3, 4, 5}, 6);
    leader.setIsActive(true);
    follower.setIsActive(true);

    for (int step = 0; step < 100; ++step)
    {
        leader.Update(0.01f, grid.tiles);
    }
    for (int step = 0; step < 30; ++step)
    {
        follower.Update(0.01f, grid.tiles);
    }

    CHECK(leader.getDistanceToExit() < follower.getDistanceToExit());
}

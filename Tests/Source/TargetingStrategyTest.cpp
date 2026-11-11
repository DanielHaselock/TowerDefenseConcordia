#include "TargetingStrategy.h"
#include "NearestToTowerStrategy.h"
#include "NearestToExitStrategy.h"
#include "StrongestStrategy.h"
#include "WeakestStrategy.h"
#include "Critter.h"
#include "RendererMap.h"
#include "Texture.h"
#include "Vector4.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

#include <memory>
#include <stdexcept>
#include <vector>

namespace Game
{
    /// Minimal concrete Critter for exercising the strategies on hand-built candidate lists.
    class StrategyTestCritter : public Critter
    {
    public:
        StrategyTestCritter(int hitPoints, int row, int col)
            : Critter(hitPoints, 2, 40, 1, 5, row, col, nullptr)
        {
        }

        std::string getType() const override { return "StrategyTest"; }
        std::unique_ptr<Critter> clone() const override { return std::make_unique<StrategyTestCritter>(*this); }
        void setLevel(int p_level) override { level = p_level; }
    };

    /// Headless row of tiles 10 px apart, so critters can be given a real path
    /// (and so a distance left to the exit) without an SDL window.
    class StrategyTestTiles
    {
    public:
        explicit StrategyTestTiles(int columns)
        {
            for (int column = 0; column < columns; ++column)
            {
                const Math::Vec4<int> position(column * 10, 0, 10, 10);
                auto* ground = texture.useSpecificSpriteSheet(0, 0, 1, 1, position);
                auto* infected = texture.useSpecificSpriteSheet(0, 0, 1, 1, position);
                ownedTiles.emplace_back(std::make_unique<::Renderer::RendererMap>(
                    ::Renderer::TileType::scenery, ground, infected, &texture, 0, column));
                tiles.emplace_back(ownedTiles.back().get());
            }
        }

        ::Renderer::Texture texture{ 0, 0, 0, 0 };
        std::vector<std::unique_ptr<::Renderer::RendererMap>> ownedTiles;
        std::vector<::Renderer::RendererMap*> tiles;
    };
}

using Game::StrategyTestCritter;
using Game::TargetingStrategyType;

TEST_CASE("WeakestStrategy picks the critter with the fewest hit points", "[Strategy]")
{
    StrategyTestCritter a(8, 0, 0), b(3, 0, 0), c(5, 0, 0);
    std::vector<Game::Critter*> candidates{ &a, &b, &c };

    CHECK(Game::WeakestStrategy().chooseTarget(candidates, 0, 0) == &b);
}

TEST_CASE("StrongestStrategy picks the critter with the most hit points", "[Strategy]")
{
    StrategyTestCritter a(8, 0, 0), b(3, 0, 0), c(12, 0, 0), d(5, 0, 0);
    std::vector<Game::Critter*> candidates{ &a, &b, &c, &d };

    CHECK(Game::StrongestStrategy().chooseTarget(candidates, 0, 0) == &c);
}

TEST_CASE("NearestToTowerStrategy picks the critter closest to the tower", "[Strategy]")
{
    StrategyTestCritter far(5, 0, 6), near(5, 4, 5), mid(5, 2, 7);
    std::vector<Game::Critter*> candidates{ &far, &near, &mid };

    // Tower at (5, 5): squared distances are 26, 1 and 8.
    CHECK(Game::NearestToTowerStrategy().chooseTarget(candidates, 5, 5) == &near);
}

TEST_CASE("NearestToExitStrategy picks the critter with the least path left", "[Strategy]")
{
    Game::StrategyTestTiles grid(6);
    const std::vector<int> path{ 0, 1, 2, 3, 4, 5 };

    StrategyTestCritter back(5, 0, 0), front(5, 0, 0), middle(5, 0, 0);
    for (StrategyTestCritter* critter : { &back, &front, &middle })
    {
        critter->setPath(grid.tiles, path, 6);
        critter->setIsActive(true);
    }
    for (int step = 0; step < 100; ++step) front.Update(0.01f, grid.tiles);
    for (int step = 0; step < 50; ++step) middle.Update(0.01f, grid.tiles);

    REQUIRE(front.getDistanceToExit() < middle.getDistanceToExit());
    REQUIRE(middle.getDistanceToExit() < back.getDistanceToExit());

    // The one furthest along wins whatever its position in the list, and
    // whatever its hit points or distance from the tower.
    std::vector<Game::Critter*> candidates{ &back, &front, &middle };
    CHECK(Game::NearestToExitStrategy().chooseTarget(candidates, 0, 0) == &front);

    candidates = { &front, &back, &middle };
    CHECK(Game::NearestToExitStrategy().chooseTarget(candidates, 99, 99) == &front);
}

TEST_CASE("Every strategy returns the first critter on a tie", "[Strategy]")
{
    // Same hit points, same cell and no path: nothing tells them apart.
    StrategyTestCritter a(5, 1, 1), b(5, 1, 1), c(5, 1, 1);
    std::vector<Game::Critter*> candidates{ &a, &b, &c };

    CHECK(Game::WeakestStrategy().chooseTarget(candidates, 0, 0) == &a);
    CHECK(Game::StrongestStrategy().chooseTarget(candidates, 0, 0) == &a);
    CHECK(Game::NearestToTowerStrategy().chooseTarget(candidates, 0, 0) == &a);
    CHECK(Game::NearestToExitStrategy().chooseTarget(candidates, 0, 0) == &a);

    candidates = { &c, &a, &b };
    CHECK(Game::WeakestStrategy().chooseTarget(candidates, 0, 0) == &c);
    CHECK(Game::StrongestStrategy().chooseTarget(candidates, 0, 0) == &c);
    CHECK(Game::NearestToTowerStrategy().chooseTarget(candidates, 0, 0) == &c);
    CHECK(Game::NearestToExitStrategy().chooseTarget(candidates, 0, 0) == &c);
}

TEST_CASE("Every strategy returns nullptr for an empty candidate list", "[Strategy]")
{
    const std::vector<Game::Critter*> none;

    CHECK(Game::WeakestStrategy().chooseTarget(none, 0, 0) == nullptr);
    CHECK(Game::StrongestStrategy().chooseTarget(none, 0, 0) == nullptr);
    CHECK(Game::NearestToTowerStrategy().chooseTarget(none, 0, 0) == nullptr);
    CHECK(Game::NearestToExitStrategy().chooseTarget(none, 0, 0) == nullptr);
}

TEST_CASE("A single candidate is always chosen", "[Strategy]")
{
    StrategyTestCritter only(5, 3, 3);
    std::vector<Game::Critter*> candidates{ &only };

    for (auto type : { TargetingStrategyType::NearestToTower, TargetingStrategyType::NearestToExit,
                       TargetingStrategyType::Strongest, TargetingStrategyType::Weakest })
    {
        CHECK(Game::makeTargetingStrategy(type)->chooseTarget(candidates, 0, 0) == &only);
    }
}

TEST_CASE("makeTargetingStrategy builds the strategy matching each type", "[Strategy]")
{
    auto nearestTower = Game::makeTargetingStrategy(TargetingStrategyType::NearestToTower);
    auto nearestExit = Game::makeTargetingStrategy(TargetingStrategyType::NearestToExit);
    auto strongest = Game::makeTargetingStrategy(TargetingStrategyType::Strongest);
    auto weakest = Game::makeTargetingStrategy(TargetingStrategyType::Weakest);

    CHECK(nearestTower->getType() == TargetingStrategyType::NearestToTower);
    CHECK(nearestExit->getType() == TargetingStrategyType::NearestToExit);
    CHECK(strongest->getType() == TargetingStrategyType::Strongest);
    CHECK(weakest->getType() == TargetingStrategyType::Weakest);

    CHECK(std::string(nearestTower->getName()) == "Near Tower");
    CHECK(std::string(nearestExit->getName()) == "Near Exit");
    CHECK(std::string(strongest->getName()) == "Strongest");
    CHECK(std::string(weakest->getName()) == "Weakest");
}

TEST_CASE("makeTargetingStrategy rejects an unknown strategy type", "[Strategy]")
{
    CHECK_THROWS_AS(Game::makeTargetingStrategy(static_cast<TargetingStrategyType>(-1)), std::invalid_argument);
}

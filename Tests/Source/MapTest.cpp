#include "Map.h"
#include "DirectDammageTower.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

#include <stdexcept>
#include <utility>
#include <vector>

namespace Game
{
    /// Records every notify() call a Map sends it.
    class RecordingMapObserver : public Observer<Map>
    {
    public:
        explicit RecordingMapObserver(Map& map) : Observer<Map>(map) {}

        int callCount = 0;

        void update(const Map&) override
        {
            ++callCount;
        }
    };

    class MapTest
    {
    public:
        static bool findEntryAndExit(
            Map& map,
            std::pair<int, int>& entry,
            std::pair<int, int>& exit)
        {
            return map.findEntryAndExit(entry, exit);
        }

    };
}

TEST_CASE("Map generates a connected path between its entry and exit", "[Map]")
{
    const std::pair<int, int> dimensions[] = {
        {1, 2},
        {2, 1},
        {1, 8},
        {8, 1},
        {3, 7},
        {7, 3},
        {6, 6}
    };

    for (const auto& [rows, cols] : dimensions)
    {
        DYNAMIC_SECTION(rows << " rows by " << cols << " columns")
        {
            Game::Map map(rows, cols);
            std::pair<int, int> entry;
            std::pair<int, int> exit;
            // Each generated map must have exactly one entry and exit.
            REQUIRE(Game::MapTest::findEntryAndExit(map, entry, exit));
            // The entry and exit must remain in the first and last columns.
            CHECK(entry.second == 0);
            CHECK(exit.second == cols - 1);
            // The path connecting the endpoints must be valid.
            CHECK(map.isValid());
        }
    }
}

TEST_CASE("Map regeneration replaces its path with a valid generated map", "[Map]")
{
    Game::Map map(5, 7);

    std::pair<int, int> entry;
    std::pair<int, int> exit;
    // The initially generated map must have valid endpoints.
    REQUIRE(Game::MapTest::findEntryAndExit(map, entry, exit));
    // Endpoints must be in the expected columns and connected by a valid path.
    CHECK(entry.second == 0);
    CHECK(exit.second == map.getCols() - 1);
    CHECK(map.isValid());

    map.generateRandomMap();
    // Regeneration must produce endpoints in the expected columns.
    REQUIRE(Game::MapTest::findEntryAndExit(map, entry, exit));
    CHECK(entry.second == 0);
    CHECK(exit.second == map.getCols() - 1);
    // The regenerated map must still have a valid connecting path.
    CHECK(map.isValid());
}

TEST_CASE("Map rejects dimensions that cannot form distinct endpoints", "[Map]")
{
    // Invalid dimensions must be rejected during map construction.
    CHECK_THROWS_AS(Game::Map(0, 4), std::invalid_argument);
    CHECK_THROWS_AS(Game::Map(4, 0), std::invalid_argument);
    CHECK_THROWS_AS(Game::Map(-1, 4), std::invalid_argument);
    CHECK_THROWS_AS(Game::Map(4, -1), std::invalid_argument);
    CHECK_THROWS_AS(Game::Map(1, 1), std::invalid_argument);
}

TEST_CASE("Editable maps start as scenery and accept a connected route", "[Map][Editor]")
{
    Game::Map map(2, 4, Game::Map::InitialLayout::Scenery);

    for (int row = 0; row < map.getRows(); ++row)
    {
        for (int column = 0; column < map.getCols(); ++column)
        {
            CHECK(map.getCell(row, column).type == Game::CellType::Scenery);
        }
    }

    map.setCellType(1, 0, Game::CellType::Entry);
    map.setCellType(1, 1, Game::CellType::Path);
    map.setCellType(1, 2, Game::CellType::Path);
    map.setCellType(1, 3, Game::CellType::Exit);

    REQUIRE(map.isValid());
    // correct path uses flat index (index = current_row * cols + current_col)
    CHECK((map.getCorrectPath() == std::vector<int>{4, 5, 6, 7}));
    CHECK(map.getEntryCell() == std::make_pair(1, 0));
    CHECK(map.getExitCell() == std::make_pair(1, 3));
}

TEST_CASE("Editable map is invalid when path is broken", "[Map][Editor]")
{
    Game::Map map(2, 4, Game::Map::InitialLayout::Scenery);
    map.setCellType(0, 0, Game::CellType::Entry);
    map.setCellType(0, 1, Game::CellType::Path);
    map.setCellType(0, 2, Game::CellType::Scenery);  // breaks the path
    map.setCellType(0, 3, Game::CellType::Exit);
    CHECK_FALSE(map.isValid());
    CHECK(map.getCorrectPath().empty());
}

TEST_CASE("Editable map validation requires exactly one entry and exit", "[Map][Editor]")
{
    Game::Map map(3, 4, Game::Map::InitialLayout::Scenery);
    map.setCellType(1, 0, Game::CellType::Entry);
    map.setCellType(1, 1, Game::CellType::Path);
    map.setCellType(1, 2, Game::CellType::Path);
    map.setCellType(1, 3, Game::CellType::Exit);
    REQUIRE(map.isValid());

    map.setCellType(2, 0, Game::CellType::Entry);
    CHECK_FALSE(map.isValid());

    map.setCellType(2, 0, Game::CellType::Scenery);
    map.setCellType(2, 3, Game::CellType::Exit);
    CHECK_FALSE(map.isValid());
}

TEST_CASE("Editable map cell updates notify observers and check coordinates", "[Map][Editor][Observer]")
{
    Game::Map map(2, 3, Game::Map::InitialLayout::Scenery);
    Game::RecordingMapObserver observer(map);

    map.setCellType(0, 0, Game::CellType::Entry);
    CHECK(observer.callCount == 1);
    CHECK_THROWS_AS(map.setCellType(-1, 0, Game::CellType::Path), std::out_of_range);
    CHECK(observer.callCount == 1);
}

TEST_CASE("Map accepts an entry outside the first column when connected to exit", "[Map][Editor]")
{
    Game::Map map(3, 4, Game::Map::InitialLayout::Scenery);
    map.setCellType(1, 1, Game::CellType::Entry);
    map.setCellType(1, 2, Game::CellType::Path);
    map.setCellType(1, 3, Game::CellType::Exit);

    REQUIRE(map.isValid());
    CHECK((map.getCorrectPath() == std::vector<int>{5, 6, 7}));
}

TEST_CASE("Map accepts an exit outside the last column and interior endpoints when connected", "[Map][Editor]")
{
    Game::Map map(4, 5, Game::Map::InitialLayout::Scenery);
    map.setCellType(2, 3, Game::CellType::Entry);
    map.setCellType(2, 2, Game::CellType::Path);
    map.setCellType(1, 2, Game::CellType::Path);
    map.setCellType(1, 1, Game::CellType::Exit);

    REQUIRE(map.isValid());
    CHECK(map.getEntryCell() == std::make_pair(2, 3));
    CHECK(map.getExitCell() == std::make_pair(1, 1));
    CHECK((map.getCorrectPath() == std::vector<int>{13, 12, 7, 6}));
}

TEST_CASE("Map validate reports specific endpoint and connectivity diagnostics", "[Map][Editor]")
{
    Game::Map map(3, 4, Game::Map::InitialLayout::Scenery);
    CHECK(map.validate() == Game::Map::ValidationStatus::MissingEntry);

    map.setCellType(0, 0, Game::CellType::Entry);
    CHECK(map.validate() == Game::Map::ValidationStatus::MissingExit);

    map.setCellType(2, 0, Game::CellType::Entry);
    CHECK(map.validate() == Game::Map::ValidationStatus::MultipleEntries);

    map.setCellType(2, 0, Game::CellType::Scenery);
    map.setCellType(0, 3, Game::CellType::Exit);
    CHECK(map.validate() == Game::Map::ValidationStatus::DisconnectedPath);

    map.setCellType(2, 3, Game::CellType::Exit);
    CHECK(map.validate() == Game::Map::ValidationStatus::MultipleExits);

    map.setCellType(2, 3, Game::CellType::Scenery);
    map.setCellType(0, 1, Game::CellType::Path);
    map.setCellType(0, 2, Game::CellType::Path);
    CHECK(map.validate() == Game::Map::ValidationStatus::Valid);
}

TEST_CASE("Map resize and clear update grid dimensions and notify observers", "[Map][Editor][Observer]")
{
    Game::Map map(5, 30, Game::Map::InitialLayout::Scenery);
    CHECK(map.getRows() == 5);
    CHECK(map.getCols() == 30);

    map.setCellType(2, 4, Game::CellType::Entry);
    map.setCellType(2, 5, Game::CellType::Path);

    Game::RecordingMapObserver observer(map);
    map.resize(10, 10, true);
    CHECK(observer.callCount == 1);
    CHECK(map.getRows() == 10);
    CHECK(map.getCols() == 10);
    CHECK(map.getCell(2, 4).type == Game::CellType::Entry);
    CHECK(map.getCell(2, 5).type == Game::CellType::Path);

    map.clear();
    CHECK(observer.callCount == 2);
    CHECK(map.getCell(2, 4).type == Game::CellType::Scenery);
    CHECK_THROWS_AS(map.resize(1, 1), std::invalid_argument);
}

TEST_CASE("Map rejects a map with no entry or exit", "[Map]")
{
    Game::Map map(3, 4, Game::Map::InitialLayout::Scenery);
    std::pair<int, int> entry;
    std::pair<int, int> exit;

    // Endpoint discovery and overall validity both fail when neither endpoint exists.
    CHECK_FALSE(Game::MapTest::findEntryAndExit(map, entry, exit));
    CHECK_FALSE(map.isValid());
}

TEST_CASE("Map rejects endpoints that are not connected by a path", "[Map]")
{
    Game::Map map(3, 4, Game::Map::InitialLayout::Scenery);
    map.setCellType(1, 0, Game::CellType::Entry);
    map.setCellType(1, 3, Game::CellType::Exit);

    // Endpoints in valid columns are invalid when no traversable path connects them.
    CHECK_FALSE(map.isValid());
}

TEST_CASE("Map setCellType rejects invalid coordinates", "[Map]")
{
    Game::Map map(3, 4);

    // Coordinates outside either map dimension must throw std::out_of_range.
    CHECK_THROWS_AS(
        map.setCellType(-1, 0, Game::CellType::Path),
        std::out_of_range);
    CHECK_THROWS_AS(
        map.setCellType(3, 0, Game::CellType::Path),
        std::out_of_range);
    CHECK_THROWS_AS(
        map.setCellType(0, -1, Game::CellType::Path),
        std::out_of_range);
    CHECK_THROWS_AS(
        map.setCellType(0, 4, Game::CellType::Path),
        std::out_of_range);
}

TEST_CASE("Map regeneration notifies observers", "[Map][Observer]")
{
    Game::Map map(3, 4);
    Game::RecordingMapObserver observer(map);

    map.generateRandomMap();
    CHECK(observer.callCount == 1);
}

TEST_CASE("Map placeTower notifies observers and marks the cell occupied", "[Map][Observer]")
{
    Game::Map map(3, 4, Game::Map::InitialLayout::Scenery);

    Game::RecordingMapObserver observer(map);

    Game::DirectDammageTower tower(1, 2, nullptr);
    REQUIRE(map.placeTower(&tower, 1, 2));
    CHECK(observer.callCount == 1);
    CHECK(map.getCell(1, 2).hasTower);
    CHECK(map.getCell(1, 2).tower == &tower);
}

TEST_CASE("Map placeTower does not notify when the cell cannot accept a tower", "[Map][Observer]")
{
    Game::Map map(3, 4, Game::Map::InitialLayout::Scenery);
    map.setCellType(0, 0, Game::CellType::Path); // not buildable

    Game::RecordingMapObserver observer(map);

    Game::DirectDammageTower tower(0, 0, nullptr);
    CHECK_FALSE(map.placeTower(&tower, 0, 0));
    CHECK(observer.callCount == 0);
}

TEST_CASE("Map placeTower refuses an already-occupied cell and does not notify again", "[Map][Observer]")
{
    Game::Map map(3, 4, Game::Map::InitialLayout::Scenery);

    Game::RecordingMapObserver observer(map);

    Game::DirectDammageTower firstTower(0, 0, nullptr);
    REQUIRE(map.placeTower(&firstTower, 0, 0));
    REQUIRE(observer.callCount == 1);

    Game::DirectDammageTower secondTower(0, 0, nullptr);
    CHECK_FALSE(map.placeTower(&secondTower, 0, 0)); // cell already has a tower
    CHECK(observer.callCount == 1); // unchanged - no notify on failure
}

TEST_CASE("Map removeTower notifies observers and clears the cell", "[Map][Observer]")
{
    Game::Map map(3, 4, Game::Map::InitialLayout::Scenery);

    Game::DirectDammageTower tower(0, 0, nullptr);
    REQUIRE(map.placeTower(&tower, 0, 0));

    // Constructed after placement, to isolate removeTower()'s own notify().
    Game::RecordingMapObserver observer(map);

    REQUIRE(map.removeTower(0, 0));
    CHECK(observer.callCount == 1);
    CHECK_FALSE(map.getCell(0, 0).hasTower);
    CHECK(map.getCell(0, 0).tower == nullptr);
}

TEST_CASE("Map removeTower on an empty cell does not notify", "[Map][Observer]")
{
    Game::Map map(3, 4, Game::Map::InitialLayout::Scenery);

    Game::RecordingMapObserver observer(map);

    CHECK_FALSE(map.removeTower(0, 0)); // nothing there to remove
    CHECK(observer.callCount == 0);
}

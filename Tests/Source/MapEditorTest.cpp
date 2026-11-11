#include "MapEditor.h"
#include "PixelFont.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

TEST_CASE("MapEditor builds custom maps of user-specified dimensions and validates connectivity", "[MapEditor][Map]")
{
    Renderer::Window window;
    App::InputManager input;
    App::MapEditor editor(window, &input, 10, 10);

    CHECK(editor.getMap().getRows() == 10);
    CHECK(editor.getMap().getCols() == 10);
    CHECK_FALSE(editor.tryStartGame());
    CHECK(editor.getValidationStatus() == Game::Map::ValidationStatus::MissingEntry);

    // Resize to 5x12
    REQUIRE(editor.setDimensions(5, 12));
    CHECK(editor.getMap().getRows() == 5);
    CHECK(editor.getMap().getCols() == 12);

    // Place Entry and Exit on custom cells and connect them with Path cells
    editor.paintCell(2, 3, Game::CellType::Entry);
    CHECK(editor.getValidationStatus() == Game::Map::ValidationStatus::MissingExit);
    CHECK_FALSE(editor.tryStartGame());

    editor.paintCell(2, 6, Game::CellType::Exit);
    CHECK(editor.getValidationStatus() == Game::Map::ValidationStatus::DisconnectedPath);
    CHECK_FALSE(editor.tryStartGame());

    editor.paintCell(2, 4, Game::CellType::Path);
    editor.paintCell(2, 5, Game::CellType::Path);

    CHECK(editor.getValidationStatus() == Game::Map::ValidationStatus::Valid);
    REQUIRE(editor.tryStartGame());
    CHECK(editor.isReadyToPlay());

    std::unique_ptr<Game::Map> createdMap = editor.takeMap();
    REQUIRE(createdMap != nullptr);
    CHECK(createdMap->getRows() == 5);
    CHECK(createdMap->getCols() == 12);
    CHECK(createdMap->isValid());
}

TEST_CASE("MapEditor enforces 2x2 minimum and 12x12 maximum dimensions", "[MapEditor][Map]")
{
    Renderer::Window window;
    App::MapEditor editor(window, nullptr, 1, 30);

    CHECK(editor.getMap().getRows() == 2);
    CHECK(editor.getMap().getCols() == 12);

    // Already at (2, 12), so clamping (0, 99) keeps dimensions unchanged
    CHECK_FALSE(editor.setDimensions(0, 99));

    REQUIRE(editor.setDimensions(6, 6));
    REQUIRE(editor.setDimensions(1, 1));
    CHECK(editor.getMap().getRows() == 2);
    CHECK(editor.getMap().getCols() == 2);

    REQUIRE(editor.setDimensions(25, 40));
    CHECK(editor.getMap().getRows() == 12);
    CHECK(editor.getMap().getCols() == 12);
}

TEST_CASE("MapEditor relocates Entry and Exit cleanly on single-click placement", "[MapEditor]")
{
    Renderer::Window window;
    App::MapEditor editor(window, nullptr, 6, 6);

    editor.paintCell(1, 1, Game::CellType::Entry);
    CHECK(editor.getMap().getCell(1, 1).type == Game::CellType::Entry);

    // Clicking a different cell with Entry moves Entry there and reverts the old cell to Scenery
    editor.paintCell(4, 2, Game::CellType::Entry);
    CHECK(editor.getMap().getCell(1, 1).type == Game::CellType::Scenery);
    CHECK(editor.getMap().getCell(4, 2).type == Game::CellType::Entry);

    // Clicking the same Entry cell toggles it back to Scenery
    editor.paintCell(4, 2, Game::CellType::Entry);
    CHECK(editor.getMap().getCell(4, 2).type == Game::CellType::Scenery);
}

TEST_CASE("MapEditor creates valid random maps for arbitrary user-specified sizes", "[MapEditor][Map]")
{
    Renderer::Window window;
    App::MapEditor editor(window, nullptr, 5, 12);

    editor.createRandomMap();
    CHECK(editor.getValidationStatus() == Game::Map::ValidationStatus::Valid);
    REQUIRE(editor.tryStartGame());

    std::unique_ptr<Game::Map> randomMap = editor.takeMap();
    REQUIRE(randomMap != nullptr);
    CHECK(randomMap->getRows() == 5);
    CHECK(randomMap->getCols() == 12);
    CHECK(randomMap->isValid());
}

TEST_CASE("PixelFont measures glyph dimensions consistently", "[Renderer][PixelFont]")
{
    CHECK(Renderer::PixelFont::measureWidth("", 2) == 0);
    CHECK(Renderer::PixelFont::measureWidth("A", 2) == 10);
    CHECK(Renderer::PixelFont::measureWidth("AB", 2) == 22);
    CHECK(Renderer::PixelFont::measureHeight(2) == 14);
    CHECK(Renderer::PixelFont::glyphFor('a') == Renderer::PixelFont::glyphFor('A'));
}

#include "MapGenerator.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

TEST_CASE("MapGenerator places non-square maps with rows vertical and columns horizontal", "[MapGenerator]")
{
    constexpr int rows = 3;
    constexpr int columns = 5;
    constexpr int menuOffset = 80;
    constexpr int tileWidth = 112;
    constexpr int tileHeight = 133;

    const SDL_FRect topLeft = Renderer::MapGenerator::CalculateTileRect(
        0, 0, menuOffset, tileWidth, tileHeight);
    const SDL_FRect topRight = Renderer::MapGenerator::CalculateTileRect(
        0, columns - 1, menuOffset, tileWidth, tileHeight);
    const SDL_FRect bottomLeft = Renderer::MapGenerator::CalculateTileRect(
        rows - 1, 0, menuOffset, tileWidth, tileHeight);

    // Five columns share the available width; three rows share the available height.
    CHECK(Renderer::MapGenerator::CalculateTileWidth(columns, menuOffset, 640) == tileWidth);
    CHECK(Renderer::MapGenerator::CalculateTileHeight(rows, menuOffset, 480) == tileHeight);
    // Columns move right while staying on the same row.
    CHECK(topLeft.x == menuOffset);
    CHECK(topRight.x == menuOffset + (columns - 1) * tileWidth);
    CHECK(topRight.y == topLeft.y);
    // Rows move down while staying in the same column.
    CHECK(bottomLeft.x == topLeft.x);
    CHECK(bottomLeft.y == (rows - 1) * tileHeight);
}

TEST_CASE("MapGenerator keeps cells square and fills the board edge-to-edge where possible", "[MapGenerator]")
{
    constexpr int menuOffset = 80;
    constexpr int winW = 640;
    constexpr int winH = 640;

    SECTION("Square 12x12 map fills 100% of the 560x560 board edge-to-edge with zero border gap")
    {
        const SDL_FRect first = Renderer::MapGenerator::CalculateCenteredSquareTileRect(
            0, 0, 12, 12, menuOffset, winW, winH);
        const SDL_FRect last = Renderer::MapGenerator::CalculateCenteredSquareTileRect(
            11, 11, 12, 12, menuOffset, winW, winH);

        CHECK(first.x == 80.0f);
        CHECK(first.y == 0.0f);
        CHECK(first.w == first.h);
        CHECK(last.x + last.w == 640.0f);
        CHECK(last.y + last.h == 560.0f);
    }

    SECTION("Wide 6x12 map fills width completely and centers vertically with equal grey borders")
    {
        const SDL_FRect first = Renderer::MapGenerator::CalculateCenteredSquareTileRect(
            0, 0, 6, 12, menuOffset, winW, winH);
        const SDL_FRect last = Renderer::MapGenerator::CalculateCenteredSquareTileRect(
            5, 11, 6, 12, menuOffset, winW, winH);

        CHECK(first.w == first.h);
        CHECK(first.x == 80.0f);
        CHECK(last.x + last.w == 640.0f);
        CHECK(first.y == 140.0f);
        CHECK(last.y + last.h == 420.0f);
    }

    SECTION("Tall 12x6 map fills height completely and centers horizontally with equal grey borders")
    {
        const SDL_FRect first = Renderer::MapGenerator::CalculateCenteredSquareTileRect(
            0, 0, 12, 6, menuOffset, winW, winH);
        const SDL_FRect last = Renderer::MapGenerator::CalculateCenteredSquareTileRect(
            11, 5, 12, 6, menuOffset, winW, winH);

        CHECK(first.w == first.h);
        CHECK(first.y == 0.0f);
        CHECK(last.y + last.h == 560.0f);
        CHECK(first.x == 220.0f);
        CHECK(last.x + last.w == 500.0f);
    }
}

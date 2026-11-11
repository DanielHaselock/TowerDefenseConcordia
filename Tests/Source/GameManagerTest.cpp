#include "GameManager.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

#include <memory>
#include <stdexcept>

TEST_CASE("GameManager rejects a null selected map", "[GameManager][Map]")
{
    Renderer::Window window;

    CHECK_THROWS_AS(
        App::GameManager(window, nullptr, std::unique_ptr<Game::Map>{}),
        std::invalid_argument);
}

TEST_CASE("GameManager rejects an invalid selected map", "[GameManager][Map]")
{
    Renderer::Window window;
    auto invalidMap = std::make_unique<Game::Map>(
        3,
        4,
        Game::Map::InitialLayout::Scenery);

    CHECK_THROWS_AS(
        App::GameManager(window, nullptr, std::move(invalidMap)),
        std::invalid_argument);
}

TEST_CASE("choosePanelCorner prefers a corner clear of subject and exit", "[GameManager][Inspection]")
{
    // Panel corners of a 560x400 play area: top-right, bottom-right,
    // top-left, bottom-left.
    const SDL_FRect corners[4] = {
        {390.0f, 0.0f, 170.0f, 176.0f},
        {390.0f, 224.0f, 170.0f, 176.0f},
        {0.0f, 0.0f, 170.0f, 176.0f},
        {0.0f, 224.0f, 170.0f, 176.0f},
    };

    // Subject mid-board, exit clear of every corner: first corner wins.
    const SDL_FRect subject{200.0f, 150.0f, 62.0f, 44.0f};
    const SDL_FRect exit{300.0f, 180.0f, 62.0f, 44.0f};
    const SDL_FRect chosen = App::GameManager::choosePanelCorner(subject, exit, corners);
    CHECK(chosen.x == 390.0f);
    CHECK(chosen.y == 0.0f);
}

TEST_CASE("choosePanelCorner skips a corner covering the subject", "[GameManager][Inspection]")
{
    const SDL_FRect corners[4] = {
        {390.0f, 0.0f, 170.0f, 176.0f},
        {390.0f, 224.0f, 170.0f, 176.0f},
        {0.0f, 0.0f, 170.0f, 176.0f},
        {0.0f, 224.0f, 170.0f, 176.0f},
    };

    // Subject sits in the top-right corner; the next clear corner wins.
    const SDL_FRect subject{400.0f, 10.0f, 62.0f, 44.0f};
    const SDL_FRect exit{300.0f, 180.0f, 62.0f, 44.0f};
    const SDL_FRect chosen = App::GameManager::choosePanelCorner(subject, exit, corners);
    CHECK(chosen.x == 390.0f);
    CHECK(chosen.y == 224.0f);
}

TEST_CASE("choosePanelCorner avoids the exit when it can", "[GameManager][Inspection]")
{
    const SDL_FRect corners[4] = {
        {390.0f, 0.0f, 170.0f, 176.0f},
        {390.0f, 224.0f, 170.0f, 176.0f},
        {0.0f, 0.0f, 170.0f, 176.0f},
        {0.0f, 224.0f, 170.0f, 176.0f},
    };

    // Exit covers the two right corners: the panel hops to top-left.
    const SDL_FRect subject{200.0f, 150.0f, 62.0f, 44.0f};
    const SDL_FRect exit{450.0f, 100.0f, 62.0f, 200.0f};
    const SDL_FRect chosen = App::GameManager::choosePanelCorner(subject, exit, corners);
    CHECK(chosen.x == 0.0f);
    CHECK(chosen.y == 0.0f);
}

TEST_CASE("choosePanelCorner keeps the subject visible over the exit", "[GameManager][Inspection]")
{
    const SDL_FRect corners[4] = {
        {390.0f, 0.0f, 170.0f, 176.0f},
        {390.0f, 224.0f, 170.0f, 176.0f},
        {0.0f, 0.0f, 170.0f, 176.0f},
        {0.0f, 224.0f, 170.0f, 176.0f},
    };

    // The subject covers both top corners and the exit covers both
    // bottom corners: no corner is clear of both, so the panel takes the
    // first corner that at least leaves the subject visible, even though
    // it covers the exit.
    const SDL_FRect subject{0.0f, 0.0f, 560.0f, 176.0f};
    const SDL_FRect exit{0.0f, 224.0f, 560.0f, 176.0f};
    const SDL_FRect chosen = App::GameManager::choosePanelCorner(subject, exit, corners);
    CHECK(chosen.x == 390.0f);
    CHECK(chosen.y == 224.0f);
}

TEST_CASE("choosePanelCorner falls back to the first corner when nothing is clear", "[GameManager][Inspection]")
{
    const SDL_FRect corners[4] = {
        {390.0f, 0.0f, 170.0f, 176.0f},
        {390.0f, 224.0f, 170.0f, 176.0f},
        {0.0f, 0.0f, 170.0f, 176.0f},
        {0.0f, 224.0f, 170.0f, 176.0f},
    };

    // A subject spanning the whole board covers every corner: the panel
    // has nowhere safe, so it parks in the first corner anyway.
    const SDL_FRect subject{0.0f, 0.0f, 560.0f, 400.0f};
    const SDL_FRect exit{300.0f, 180.0f, 62.0f, 44.0f};
    const SDL_FRect chosen = App::GameManager::choosePanelCorner(subject, exit, corners);
    CHECK(chosen.x == 390.0f);
    CHECK(chosen.y == 0.0f);
}

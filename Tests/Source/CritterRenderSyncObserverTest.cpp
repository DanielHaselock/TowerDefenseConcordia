#include "Critter.h"
#include "WalkerCritter.h"
#include "RendererMap.h"
#include "Texture.h"
#include "Vector4.h"
#include "CritterRenderSyncObserver.h"

#include <memory>
#include <vector>

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

namespace
{
    // Minimal 1-row tile grid with 10x10 tiles at (col*10, row*10), enough
    // for real RendererMap tile lookups without creating an SDL window
    // (same approach as CritterTest's grid helper).
    class RendererTileGrid
    {
    public:
        RendererTileGrid(int rows, int columns)
        {
            for (int row = 0; row < rows; ++row)
            {
                for (int column = 0; column < columns; ++column)
                {
                    const int cellNumber = row * columns + column;
                    const Math::Vec4<int> position(column * 10, row * 10, 10, 10);
                    auto* ground = texture.useSpecificSpriteSheet(0, 0, 1, 1, position);
                    auto* infected = texture.useSpecificSpriteSheet(0, 0, 1, 1, position);
                    ownedTiles.emplace_back(std::make_unique<::Renderer::RendererMap>(
                        ::Renderer::TileType::path, ground, infected, &texture, 0, cellNumber));
                    tiles.emplace_back(ownedTiles.back().get());
                }
            }
        }

        ::Renderer::Texture texture{ 0, 0, 0, 0 };
        std::vector<std::unique_ptr<::Renderer::RendererMap>> ownedTiles;
        std::vector<::Renderer::RendererMap*> tiles;
    };
}

TEST_CASE("CritterRenderSyncObserver mirrors critter movement onto its sprite", "[Critter][Observer][Render]")
{
    RendererTileGrid grid(1, 3);
    ::Renderer::spriteSheetTexture sprite;
    Game::WalkerCritter walker(1, 0, 0, &sprite);
    walker.setPath(grid.tiles, { 0, 1, 2 }, 3);

    App::CritterRenderSyncObserver render(walker);

    walker.setIsActive(true);
    // Activation positions the sprite at the spawn tile.
    CHECK(sprite.m_pos_scale.x == Approx(0.0f));
    CHECK(sprite.render == true);

    // Walker speed is 50 px/s; one 0.1 s step moves it 5 px along the path.
    walker.Update(0.1f, grid.tiles);
    CHECK(sprite.m_pos_scale.x == Approx(walker.getPixelPosx()));
    CHECK(sprite.m_pos_scale.x == Approx(5.0f));
    CHECK(sprite.m_pos_scale.y == Approx(walker.getPixelPosy()));

    walker.Update(0.1f, grid.tiles);
    CHECK(sprite.m_pos_scale.x == Approx(walker.getPixelPosx()));
    CHECK(sprite.m_pos_scale.x > 5.0f);
}

TEST_CASE("CritterRenderSyncObserver shows the sprite on spawn and hides it on death", "[Critter][Observer][Render]")
{
    RendererTileGrid grid(1, 3);
    ::Renderer::spriteSheetTexture sprite;
    Game::WalkerCritter walker(1, 0, 0, &sprite);
    walker.setPath(grid.tiles, { 0, 1, 2 }, 3);

    App::CritterRenderSyncObserver render(walker);

    // Pool critters start deactivated before any wave spawns them.
    walker.setIsActive(false);
    CHECK(sprite.render == false);

    walker.setIsActive(true);
    CHECK(sprite.render == true);

    walker.takeDamage(1000);
    walker.Death();
    CHECK(sprite.render == false);
    // Death resets the critter, and the observer moves the sprite back to
    // the spawn tile with it.
    CHECK(sprite.m_pos_scale.x == Approx(0.0f));

    walker.setIsActive(true);
    CHECK(sprite.render == true);
}

TEST_CASE("Critter no longer touches its sprite; a spriteless critter still runs", "[Critter][Observer][Render]")
{
    RendererTileGrid grid(1, 3);
    Game::WalkerCritter walker(1, 0, 0, nullptr);
    walker.setPath(grid.tiles, { 0, 1, 2 }, 3);

    App::CritterRenderSyncObserver render(walker);

    // Critter used to dereference its texture in Update(), setIsActive() and
    // resetCritter(); with rendering moved to the observer, a critter built
    // without a sprite walks, dies and resets without crashing, and the
    // observer simply has nothing to sync.
    walker.setIsActive(true);
    walker.Update(0.1f, grid.tiles);
    CHECK(walker.getPixelPosx() == Approx(5.0f));

    walker.takeDamage(1000);
    walker.Death();
    CHECK(walker.getIsActive() == false);
}

TEST_CASE("Destroying the observer stops sprite syncs without affecting the critter", "[Critter][Observer][Render]")
{
    RendererTileGrid grid(1, 3);
    ::Renderer::spriteSheetTexture sprite;
    Game::WalkerCritter walker(1, 0, 0, &sprite);
    walker.setPath(grid.tiles, { 0, 1, 2 }, 3);

    {
        App::CritterRenderSyncObserver render(walker);
        walker.setIsActive(true);
        walker.Update(0.1f, grid.tiles);
        CHECK(sprite.m_pos_scale.x == Approx(5.0f));
    }

    // The observer detached itself; the critter keeps moving, the sprite
    // stays where it was last synced.
    walker.Update(0.1f, grid.tiles);
    CHECK(walker.getPixelPosx() == Approx(10.0f));
    CHECK(sprite.m_pos_scale.x == Approx(5.0f));
}

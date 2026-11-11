#include "Bullet.h"
#include "Critter.h"
#include "Texture.h"

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

namespace Game
{
    /// Concrete Critter so a Bullet has a target to home in on / hit.
    /// The Critter base ctor only *stores* the texture pointer (it never
    /// derefs it), so a null texture is safe for construction and Hit().
    class BulletTestCritter : public Critter
    {
    public:
        explicit BulletTestCritter(::Renderer::spriteSheetTexture* texture = nullptr)
            : Critter(10, 2, 50, 1, 5, 0, 0, texture)
        {
        }
        std::string getType() const override { return "BulletTest"; }
        std::unique_ptr<Critter> clone() const override
        {
            return std::make_unique<BulletTestCritter>(*this);
        }
        void Update(float, const std::vector<::Renderer::RendererMap*>&) override {}
        void setLevel(int p_level) override { level = p_level; }
    };

    /// Friend (granted by Bullet.h) to expose the protected m_damage / m_speed
    /// members that Shoot() stores, so the tests can assert on them.
    class BulletTest
    {
    public:
        static float damage(const Bullet& b) { return b.m_damage; }
        static float speed(const Bullet& b) { return b.m_speed; }
    };
}

using Game::Bullet;
using Game::BulletTestCritter;

TEST_CASE("Bullet constructor stores the texture and hides it (render=false)", "[Bullet]")
{
    ::Renderer::spriteSheetTexture tex;
    REQUIRE(tex.render == true);

    Bullet bullet(&tex);
    CHECK(bullet.GetActive() == false);   // not in flight until Shoot()
    CHECK(tex.render == false);           // ctor hides the sprite
}

TEST_CASE("Bullet Shoot arms the bullet: position, angle, visible, active, stats", "[Bullet]")
{
    ::Renderer::spriteSheetTexture tex;
    tex.m_pos_scale.x = 0.0f;
    tex.m_pos_scale.y = 0.0f;
    Bullet bullet(&tex);

    BulletTestCritter target;
    bullet.Shoot(7, 400.0f, 10, 20, 30, &target, 0);

    CHECK(bullet.GetActive() == true);
    CHECK(tex.render == true);
    CHECK(tex.m_pos_scale.x == Approx(10.0f));
    CHECK(tex.m_pos_scale.y == Approx(20.0f));
    CHECK(tex.m_angle == Approx(30.0f));
    CHECK(Game::BulletTest::damage(bullet) == 7);
    CHECK(Game::BulletTest::speed(bullet) == Approx(400.0f));
}

TEST_CASE("Bullet Update homes toward the target without overshooting it", "[Bullet]")
{
    ::Renderer::spriteSheetTexture tex;
    tex.m_pos_scale.x = 0.0f;
    tex.m_pos_scale.y = 0.0f;
    Bullet bullet(&tex);

    ::Renderer::spriteSheetTexture targetTex;
    targetTex.m_pos_scale.x = 50.0f;
    targetTex.m_pos_scale.y = 0.0f;
    BulletTestCritter target(&targetTex);

    bullet.Shoot(5, 100.0f, 0, 0, 0, &target, 0);
    REQUIRE(bullet.GetActive());

    bullet.Update(0.1f);   // moveAmount = 100 * 0.1 = 10

    // Still in flight: it moved 10px toward the target but is 40px away, so
    // it has not yet reached the (moveAmount + 5) hit threshold.
    CHECK(bullet.GetActive() == true);
    CHECK(tex.m_pos_scale.x == Approx(10.0f));
    CHECK(tex.m_pos_scale.y == Approx(0.0f));
    // Sprites point "up", so the reported angle is direction + 90 degrees.
    CHECK(tex.m_angle == Approx(90.0f));
}

TEST_CASE("Bullet Update hits when it reaches the target", "[Bullet]")
{
    ::Renderer::spriteSheetTexture tex;
    tex.m_pos_scale.x = 0.0f;
    tex.m_pos_scale.y = 0.0f;
    Bullet bullet(&tex);

    ::Renderer::spriteSheetTexture targetTex;
    targetTex.m_pos_scale.x = 20.0f;
    targetTex.m_pos_scale.y = 0.0f;
    BulletTestCritter target(&targetTex);

    bullet.Shoot(5, 100.0f, 0, 0, 0, &target, 0);
    REQUIRE(bullet.GetActive());

    bullet.Update(0.1f);   // moveAmount = 10; 20 <= 10 + 5 is false -> in flight

    // Second update from x=10: distance 10, moveAmount 10 -> 10 <= 15 -> Hit
    bullet.Update(0.1f);
    CHECK(bullet.GetActive() == false);
    CHECK(target.getHitPoints() == 5);   // 10 - 5
}

TEST_CASE("Bullet Hit deals its damage to the target and deactivates", "[Bullet]")
{
    ::Renderer::spriteSheetTexture tex;
    Bullet bullet(&tex);

    BulletTestCritter target;   // 10 hp
    bullet.Shoot(15, 500.0f, 0, 0, 0, &target, 0);

    bullet.Hit();
    CHECK(bullet.GetActive() == false);
    CHECK(target.getHitPoints() == -5);  // 10 - 15 (no clamping)
}

TEST_CASE("Bullet with the slow effect halves the target speed exactly once", "[Bullet]")
{
    ::Renderer::spriteSheetTexture tex;
    Bullet bullet(&tex);

    BulletTestCritter target;   // base speed 50
    CHECK(target.getSpeed() == 50);

    bullet.Shoot(1, 500.0f, 0, 0, 0, &target, 2);  // effect 2 = slow
    bullet.Hit();

    CHECK(target.getSpeed() == 25);   // 50 / 2
    CHECK(target.speedHalfed == true);

    // A second slow hit must not halve it again (halfSpeed is non-stacking).
    bullet.Shoot(1, 500.0f, 0, 0, 0, &target, 2);
    bullet.Hit();
    CHECK(target.getSpeed() == 25);
}

TEST_CASE("Bullet with the AoE effect publishes a HitRequest at the impact tile", "[Bullet]")
{
    ::Renderer::spriteSheetTexture tex;
    Bullet bullet(&tex);

    BulletTestCritter target;
    target.setPosition(4, 6);

    bullet.Shoot(12, 500.0f, 0, 0, 0, &target, 1, 1.5f);  // effect 1 = AoE, blast radius 1.5 tiles
    REQUIRE(bullet.GetHitRequest() == nullptr);

    bullet.Hit();

    // An AoE bullet does not damage its target on Hit(): the splash over
    // the impact tile covers the target too, and the game loop applies
    // it once from the HitRequest.
    CHECK(target.getHitPoints() == 10);

    Game::HitRequest* hr = bullet.GetHitRequest();
    REQUIRE(hr != nullptr);
    CHECK(hr->hitRow == 4);
    CHECK(hr->hitCol == 6);
    CHECK(hr->aoeDamage == 12);
    CHECK(hr->blastRadius == Approx(1.5f));

    bullet.DeleteHitRequest();
    CHECK(bullet.GetHitRequest() == nullptr);
}

TEST_CASE("Bullet ignores negative damage and deactivates on hit", "[Bullet]")
{
    ::Renderer::spriteSheetTexture tex;
    Bullet bullet(&tex);

    BulletTestCritter target;   // 10 hp
    bullet.Shoot(-3, 500.0f, 0, 0, 0, &target, 0);
    bullet.Hit();

    CHECK(bullet.GetActive() == false);
    CHECK(target.getHitPoints() == 10);   // negative damage is a no-op
}

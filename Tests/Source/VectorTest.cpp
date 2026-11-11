#include "Vector3.h"
#include "Vector4.h"

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

using Math::Vec3;
using Math::Vec4;

// ──────────────────────────────────────────────────────────────────────────────
//  Vec3 — construction & accessors
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("Vec3 default constructor yields (0,0,0)", "[Math][Vec3]")
{
    Vec3<float> v;
    CHECK(v.X() == 0.0f);
    CHECK(v.Y() == 0.0f);
    CHECK(v.Z() == 0.0f);
}

TEST_CASE("Vec3 single-argument constructor fills all components", "[Math][Vec3]")
{
    Vec3<float> v(2.0f);
    CHECK(v.X() == 2.0f);
    CHECK(v.Y() == 2.0f);
    CHECK(v.Z() == 2.0f);
}

TEST_CASE("Vec3 three-argument constructor sets each component", "[Math][Vec3]")
{
    Vec3<float> v(1.0f, 2.0f, 3.0f);
    CHECK(v.X() == 1.0f);
    CHECK(v.Y() == 2.0f);
    CHECK(v.Z() == 3.0f);
}

TEST_CASE("Vec3 copy constructor preserves components", "[Math][Vec3]")
{
    Vec3<float> a(1.0f, 2.0f, 3.0f);
    Vec3<float> b(a);
    CHECK(b.X() == 1.0f);
    CHECK(b.Y() == 2.0f);
    CHECK(b.Z() == 3.0f);
}

TEST_CASE("vec3 is a typedef of Vec3<float>", "[Math][Vec3]")
{
    Math::vec3 v(1.0f, 2.0f, 3.0f);
    CHECK(v.X() == 1.0f);
    CHECK(v.Y() == 2.0f);
    CHECK(v.Z() == 3.0f);
}

// ──────────────────────────────────────────────────────────────────────────────
//  Vec3 — magnitude, dot & cross products, normalisation
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("Vec3 length and length2 on a 3-4-5 vector", "[Math][Vec3]")
{
    Vec3<float> v(3.0f, 4.0f, 0.0f);
    CHECK(v.length() == Approx(5.0f));
    CHECK(v.length2() == Approx(25.0f));
}

TEST_CASE("Vec3 length2 equals the square of length", "[Math][Vec3]")
{
    Vec3<float> v(1.0f, 2.0f, 2.0f);
    const float len = v.length();
    CHECK(len == Approx(3.0f));
    CHECK(v.length2() == Approx(len * len));
}

TEST_CASE("Vec3 dotProduct of perpendicular vectors is zero", "[Math][Vec3]")
{
    Vec3<float> a(1.0f, 0.0f, 0.0f);
    Vec3<float> b(0.0f, 1.0f, 0.0f);
    CHECK(a.dotProduct(b) == Approx(0.0f));
}

TEST_CASE("Vec3 dotProduct of parallel vectors is the product of magnitudes", "[Math][Vec3]")
{
    Vec3<float> a(2.0f, 0.0f, 0.0f);
    Vec3<float> b(5.0f, 0.0f, 0.0f);
    CHECK(a.dotProduct(b) == Approx(10.0f));
}

TEST_CASE("Vec3 crossProduct of basis vectors follows the right-hand rule", "[Math][Vec3]")
{
    // x × y = z
    Vec3<float> x(1.0f, 0.0f, 0.0f);
    Vec3<float> y(0.0f, 1.0f, 0.0f);
    Vec3<float> z(0.0f, 0.0f, 1.0f);

    CHECK(x.crossProduct(y).Z() == Approx(1.0f));
    CHECK(y.crossProduct(z).X() == Approx(1.0f));
    CHECK(z.crossProduct(x).Y() == Approx(1.0f));

    // Reversing the order flips the sign.
    CHECK(y.crossProduct(x).Z() == Approx(-1.0f));
}

TEST_CASE("Vec3 crossProduct is zero for parallel vectors", "[Math][Vec3]")
{
    Vec3<float> a(2.0f, 0.0f, 0.0f);
    Vec3<float> b(5.0f, 0.0f, 0.0f);
    Vec3<float> c = a.crossProduct(b);
    CHECK(c.X() == Approx(0.0f));
    CHECK(c.Y() == Approx(0.0f));
    CHECK(c.Z() == Approx(0.0f));
}

TEST_CASE("Vec3 normalized has unit length", "[Math][Vec3]")
{
    Vec3<float> v(3.0f, 4.0f, 0.0f);
    Vec3<float> n = v.normalized();
    CHECK(n.length() == Approx(1.0f));
    CHECK(n.X() == Approx(0.6f));
    CHECK(n.Y() == Approx(0.8f));
    CHECK(n.Z() == Approx(0.0f));
}

// ──────────────────────────────────────────────────────────────────────────────
//  Vec3 — arithmetic operators
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("Vec3 operator+ and operator- are componentwise", "[Math][Vec3]")
{
    Vec3<float> a(1.0f, 2.0f, 3.0f);
    Vec3<float> b(4.0f, 5.0f, 6.0f);

    Vec3<float> sum = a + b;
    CHECK(sum.X() == Approx(5.0f));
    CHECK(sum.Y() == Approx(7.0f));
    CHECK(sum.Z() == Approx(9.0f));

    Vec3<float> diff = b - a;
    CHECK(diff.X() == Approx(3.0f));
    CHECK(diff.Y() == Approx(3.0f));
    CHECK(diff.Z() == Approx(3.0f));
}

TEST_CASE("Vec3 unary minus negates every component", "[Math][Vec3]")
{
    Vec3<float> a(1.0f, -2.0f, 3.0f);
    Vec3<float> neg = -a;
    CHECK(neg.X() == Approx(-1.0f));
    CHECK(neg.Y() == Approx(2.0f));
    CHECK(neg.Z() == Approx(-3.0f));
}

TEST_CASE("Vec3 operator* scales by a scalar", "[Math][Vec3]")
{
    Vec3<float> v(1.0f, 2.0f, 3.0f);
    Vec3<float> s = v * 2.0f;
    CHECK(s.X() == Approx(2.0f));
    CHECK(s.Y() == Approx(4.0f));
    CHECK(s.Z() == Approx(6.0f));
}

TEST_CASE("Vec3 operator/ divides by a scalar", "[Math][Vec3]")
{
    Vec3<float> v(2.0f, 4.0f, 6.0f);
    Vec3<float> d = v / 2.0f;
    CHECK(d.X() == Approx(1.0f));
    CHECK(d.Y() == Approx(2.0f));
    CHECK(d.Z() == Approx(3.0f));
}

TEST_CASE("Vec3 free operator* scales by a leading double", "[Math][Vec3]")
{
    Vec3<float> v(1.0f, 2.0f, 3.0f);
    Vec3<float> s = 2.0 * v;
    CHECK(s.X() == Approx(2.0f));
    CHECK(s.Y() == Approx(4.0f));
    CHECK(s.Z() == Approx(6.0f));
}

TEST_CASE("Vec3::lerp interpolates between two vectors", "[Math][Vec3]")
{
    Vec3<float> a(0.0f, 0.0f, 0.0f);
    Vec3<float> b(10.0f, 20.0f, 30.0f);

    Vec3<float> mid = Vec3<float>::lerp(a, b, 0.5f);
    CHECK(mid.X() == Approx(5.0f));
    CHECK(mid.Y() == Approx(10.0f));
    CHECK(mid.Z() == Approx(15.0f));

    // Endpoints: t=0 gives a, t=1 gives b.
    Vec3<float> t0 = Vec3<float>::lerp(a, b, 0.0f);
    CHECK(t0.X() == Approx(0.0f));
    Vec3<float> t1 = Vec3<float>::lerp(a, b, 1.0f);
    CHECK(t1.Y() == Approx(20.0f));
}

// ──────────────────────────────────────────────────────────────────────────────
//  Vec4 — construction & accessors
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("Vec4 default constructor yields (0,0,0,0)", "[Math][Vec4]")
{
    Vec4<float> v;
    CHECK(v.X() == 0.0f);
    CHECK(v.Y() == 0.0f);
    CHECK(v.Z() == 0.0f);
    CHECK(v.W() == 0.0f);
}

TEST_CASE("Vec4 single-argument constructor sets x=y=z and w=0", "[Math][Vec4]")
{
    Vec4<float> v(3.0f);
    CHECK(v.X() == 3.0f);
    CHECK(v.Y() == 3.0f);
    CHECK(v.Z() == 3.0f);
    CHECK(v.W() == 0.0f);
}

TEST_CASE("Vec4 four-argument constructor sets every component", "[Math][Vec4]")
{
    Vec4<float> v(1.0f, 2.0f, 3.0f, 4.0f);
    CHECK(v.X() == 1.0f);
    CHECK(v.Y() == 2.0f);
    CHECK(v.Z() == 3.0f);
    CHECK(v.W() == 4.0f);
}

TEST_CASE("Vec4 constructor from a Vec3 defaults w to 0", "[Math][Vec4]")
{
    Vec3<float> p(1.0f, 2.0f, 3.0f);
    Vec4<float> v(p);
    CHECK(v.X() == 1.0f);
    CHECK(v.Y() == 2.0f);
    CHECK(v.Z() == 3.0f);
    CHECK(v.W() == 0.0f);
}

// ──────────────────────────────────────────────────────────────────────────────
//  Vec4 — magnitude, dot & cross (documented to ignore / drop w)
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("Vec4 length and length2 consider only x, y, z (not w)", "[Math][Vec4]")
{
    // Same x,y,z as a 3-4-0 vector; w is deliberately different and must be ignored.
    Vec4<float> v(3.0f, 4.0f, 0.0f, 999.0f);
    CHECK(v.length() == Approx(5.0f));
    CHECK(v.length2() == Approx(25.0f));
}

TEST_CASE("Vec4 dotProduct considers only x, y, z (not w)", "[Math][Vec4]")
{
    Vec4<float> a(1.0f, 2.0f, 3.0f, 100.0f);
    Vec4<float> b(4.0f, 5.0f, 6.0f, 200.0f);
    // 1*4 + 2*5 + 3*6 = 32 (w components ignored)
    CHECK(a.dotProduct(b) == Approx(32.0f));
}

TEST_CASE("Vec4 crossProduct yields w=0", "[Math][Vec4]")
{
    Vec4<float> x(1.0f, 0.0f, 0.0f, 0.0f);
    Vec4<float> y(0.0f, 1.0f, 0.0f, 0.0f);
    Vec4<float> c = x.crossProduct(y);
    CHECK(c.Z() == Approx(1.0f));
    CHECK(c.W() == Approx(0.0f));
}

// ──────────────────────────────────────────────────────────────────────────────
//  Vec4 — arithmetic operators (w behaviour matches the header contract)
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("Vec4 operator+ adds every component including w", "[Math][Vec4]")
{
    Vec4<float> a(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4<float> b(10.0f, 20.0f, 30.0f, 40.0f);
    Vec4<float> s = a + b;
    CHECK(s.X() == Approx(11.0f));
    CHECK(s.Y() == Approx(22.0f));
    CHECK(s.Z() == Approx(33.0f));
    CHECK(s.W() == Approx(44.0f));
}

TEST_CASE("Vec4 operator- subtracts every component including w", "[Math][Vec4]")
{
    Vec4<float> a(10.0f, 20.0f, 30.0f, 40.0f);
    Vec4<float> b(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4<float> d = a - b;
    CHECK(d.X() == Approx(9.0f));
    CHECK(d.Y() == Approx(18.0f));
    CHECK(d.Z() == Approx(27.0f));
    CHECK(d.W() == Approx(36.0f));
}

TEST_CASE("Vec4 unary minus drops w (returns a 3-component vector)", "[Math][Vec4]")
{
    Vec4<float> a(1.0f, 2.0f, 3.0f, 99.0f);
    Vec4<float> neg = -a;
    CHECK(neg.X() == Approx(-1.0f));
    CHECK(neg.Y() == Approx(-2.0f));
    CHECK(neg.Z() == Approx(-3.0f));
    CHECK(neg.W() == Approx(0.0f));
}

TEST_CASE("Vec4 operator* scales every component including w", "[Math][Vec4]")
{
    Vec4<float> v(1.0f, 2.0f, 3.0f, 4.0f);
    Vec4<float> s = v * 2.0f;
    CHECK(s.X() == Approx(2.0f));
    CHECK(s.Y() == Approx(4.0f));
    CHECK(s.Z() == Approx(6.0f));
    CHECK(s.W() == Approx(8.0f));
}

TEST_CASE("Vec4 operator/ drops w (returns a 3-component vector)", "[Math][Vec4]")
{
    Vec4<float> v(2.0f, 4.0f, 6.0f, 100.0f);
    Vec4<float> d = v / 2.0f;
    CHECK(d.X() == Approx(1.0f));
    CHECK(d.Y() == Approx(2.0f));
    CHECK(d.Z() == Approx(3.0f));
    CHECK(d.W() == Approx(0.0f));
}

// ──────────────────────────────────────────────────────────────────────────────
//  Vec4 — homogenisation & normalisation
// ──────────────────────────────────────────────────────────────────────────────

TEST_CASE("Vec4 homogenized divides x, y, z by w", "[Math][Vec4]")
{
    Vec4<float> v(2.0f, 4.0f, 6.0f, 2.0f);
    Vec3<float> h = v.homogenized();
    CHECK(h.X() == Approx(1.0f));
    CHECK(h.Y() == Approx(2.0f));
    CHECK(h.Z() == Approx(3.0f));
}

TEST_CASE("Vec4 homogenized with w=0 yields the zero vector (no division by zero)", "[Math][Vec4]")
{
    Vec4<float> v(1.0f, 2.0f, 3.0f, 0.0f);
    Vec3<float> h = v.homogenized();
    CHECK(h.X() == Approx(0.0f));
    CHECK(h.Y() == Approx(0.0f));
    CHECK(h.Z() == Approx(0.0f));
}

TEST_CASE("Vec4 normalized returns a unit-length vector with w=0", "[Math][Vec4]")
{
    Vec4<float> v(3.0f, 4.0f, 0.0f, 7.0f);
    Vec4<float> n = v.normalized();
    CHECK(n.X() == Approx(0.6f));
    CHECK(n.Y() == Approx(0.8f));
    CHECK(n.Z() == Approx(0.0f));
    CHECK(n.W() == Approx(0.0f));
}

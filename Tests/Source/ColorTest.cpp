#include "Color.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

TEST_CASE("Color default constructor produces a black transparent color", "[Color]")
{
    Math::Color c;

    CHECK(c.R() == 0);
    CHECK(c.B() == 0);
    CHECK(c.G() == 0);
    CHECK(c.A() == 0);
}

TEST_CASE("Color stores its channels from the four-argument constructor", "[Color]")
{
    Math::Color c(10, 20, 30, 255);

    CHECK(c.R() == 10);
    CHECK(c.G() == 20);
    CHECK(c.B() == 30);
    CHECK(c.A() == 255);
}

TEST_CASE("Color setColor replaces all channels", "[Color]")
{
    Math::Color c(1, 2, 3, 4);

    c.setColor(11, 22, 33, 244);

    CHECK(c.R() == 11);
    CHECK(c.G() == 22);
    CHECK(c.B() == 33);
    CHECK(c.A() == 244);
}

TEST_CASE("Color copy constructor copies all channels", "[Color]")
{
    Math::Color original(12, 34, 56, 78);
    Math::Color copy(original);

    CHECK(copy.R() == 12);
    CHECK(copy.G() == 34);
    CHECK(copy.B() == 56);
    CHECK(copy.A() == 78);
}

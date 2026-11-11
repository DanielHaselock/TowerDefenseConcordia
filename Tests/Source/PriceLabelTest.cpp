#include "PriceLabel.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

TEST_CASE("PriceLabel segment masks match a seven-segment display", "[PriceLabel]")
{
    // Bit order: bit0 top, bit1 top-left, bit2 top-right, bit3 middle,
    // bit4 bottom-left, bit5 bottom-right, bit6 bottom.
    CHECK(Renderer::PriceLabel::segmentsFor('0') == 0b1110111);
    CHECK(Renderer::PriceLabel::segmentsFor('1') == 0b0100100);
    CHECK(Renderer::PriceLabel::segmentsFor('2') == 0b1011101);
    CHECK(Renderer::PriceLabel::segmentsFor('3') == 0b1101101);
    CHECK(Renderer::PriceLabel::segmentsFor('4') == 0b0101110);
    CHECK(Renderer::PriceLabel::segmentsFor('5') == 0b1101011);
    CHECK(Renderer::PriceLabel::segmentsFor('6') == 0b1111011);
    CHECK(Renderer::PriceLabel::segmentsFor('7') == 0b0100101);
    CHECK(Renderer::PriceLabel::segmentsFor('8') == 0b1111111);
    CHECK(Renderer::PriceLabel::segmentsFor('9') == 0b1101111);
    CHECK(Renderer::PriceLabel::segmentsFor('x') == 0);
}

TEST_CASE("PriceLabel width grows with the number of digits", "[PriceLabel]")
{
    Renderer::PriceLabel label(0, 0, 5);
    CHECK(label.getWidth() == 12);

    label.setValue(15);
    CHECK(label.getWidth() == 12 + 4 + 12);

    label.setValue(120);
    CHECK(label.getWidth() == 3 * 12 + 2 * 4);
}

TEST_CASE("PriceLabel starts rendered and can be hidden", "[PriceLabel]")
{
    Renderer::PriceLabel label(0, 0, 10);
    CHECK(label.shouldRender());

    label.setRender(false);
    CHECK_FALSE(label.shouldRender());
}

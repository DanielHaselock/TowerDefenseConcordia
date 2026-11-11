#include "NumberLabel.h"

#if __has_include(<catch2/catch_test_macros.hpp>)
#include <catch2/catch_test_macros.hpp>
#else
#include <catch2/catch.hpp>
#endif

TEST_CASE("NumberLabel segment masks match a seven-segment display", "[NumberLabel]")
{
    // Bit order: bit0 top, bit1 top-left, bit2 top-right, bit3 middle,
    // bit4 bottom-left, bit5 bottom-right, bit6 bottom.
    CHECK(Renderer::NumberLabel::segmentsFor('0') == 0b1110111);
    CHECK(Renderer::NumberLabel::segmentsFor('1') == 0b0100100);
    CHECK(Renderer::NumberLabel::segmentsFor('2') == 0b1011101);
    CHECK(Renderer::NumberLabel::segmentsFor('3') == 0b1101101);
    CHECK(Renderer::NumberLabel::segmentsFor('4') == 0b0101110);
    CHECK(Renderer::NumberLabel::segmentsFor('5') == 0b1101011);
    CHECK(Renderer::NumberLabel::segmentsFor('6') == 0b1111011);
    CHECK(Renderer::NumberLabel::segmentsFor('7') == 0b0100101);
    CHECK(Renderer::NumberLabel::segmentsFor('8') == 0b1111111);
    CHECK(Renderer::NumberLabel::segmentsFor('9') == 0b1101111);
    CHECK(Renderer::NumberLabel::segmentsFor('x') == 0);
}

TEST_CASE("NumberLabel size grows with the number of digits", "[NumberLabel]")
{
    Renderer::NumberLabel label(0, 0, 5);
    CHECK(label.getWidth() == 12);
    CHECK(label.getHeight() == 20);

    label.setValue(30);
    CHECK(label.getWidth() == 12 + 4 + 12);

    label.setValue(100);
    CHECK(label.getWidth() == 3 * 12 + 2 * 4);
}

TEST_CASE("NumberLabel starts rendered and can be hidden", "[NumberLabel]")
{
    Renderer::NumberLabel label(0, 0, 30);
    CHECK(label.shouldRender());

    label.setRender(false);
    CHECK_FALSE(label.shouldRender());
}

#include "NumberLabel.h"
#include "PixelFont.h"
#include <string>

Renderer::NumberLabel::NumberLabel(int x, int y, int value, Math::Vec3<int> color)
	: m_x(x), m_y(y), m_value(value), m_color(color)
{
}

int Renderer::NumberLabel::segmentsFor(char digit)
{
	// Bit order: bit0 top, bit1 top-left, bit2 top-right, bit3 middle,
	// bit4 bottom-left, bit5 bottom-right, bit6 bottom.
	switch (digit)
	{
	case '0': return 0b1110111;
	case '1': return 0b0100100;
	case '2': return 0b1011101;
	case '3': return 0b1101101;
	case '4': return 0b0101110;
	case '5': return 0b1101011;
	case '6': return 0b1111011;
	case '7': return 0b0100101;
	case '8': return 0b1111111;
	case '9': return 0b1101111;
	default:  return 0;
	}
}

void Renderer::NumberLabel::Render(::SDL_Renderer* p_renderer)
{
	// Draws with PixelFont (matching the Map Editor's text everywhere else),
	// but keeps each digit in its original DIGIT_WIDTH/GAP slot so getWidth()
	// (and the seven-segment constants/segmentsFor() the test suite pins)
	// stay unchanged.
	const std::string digits = std::to_string(m_value);
	const SDL_Color color{
		static_cast<Uint8>(m_color.X()),
		static_cast<Uint8>(m_color.Y()),
		static_cast<Uint8>(m_color.Z()),
		255 };

	constexpr int scale = 2;
	const int glyphW = PixelFont::measureWidth("0", scale);
	const int glyphH = PixelFont::measureHeight(scale);

	for (size_t d = 0; d < digits.size(); ++d)
	{
		const int slotX = m_x + (int)d * (DIGIT_WIDTH + GAP);
		const int drawX = slotX + (DIGIT_WIDTH - glyphW) / 2;
		const int drawY = m_y + (DIGIT_HEIGHT - glyphH) / 2;
		PixelFont::drawText(p_renderer, std::string(1, digits[d]), drawX, drawY, scale, color);
	}
}

void Renderer::NumberLabel::setRender(bool p_render)
{
	m_shouldRender = p_render;
}

void Renderer::NumberLabel::setValue(int value)
{
	m_value = value;
}

void Renderer::NumberLabel::setPosition(int x, int y)
{
	m_x = x;
	m_y = y;
}

int Renderer::NumberLabel::getWidth() const
{
	const int digits = (int)std::to_string(m_value).size();
	return digits * DIGIT_WIDTH + (digits - 1) * GAP;
}

int Renderer::NumberLabel::getHeight() const
{
	return DIGIT_HEIGHT;
}

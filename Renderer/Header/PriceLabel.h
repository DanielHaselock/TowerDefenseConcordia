#pragma once
#include <SDL.h>
#include <string>
#include "AbstractRenderable.h"

namespace Renderer
{
	/**
	 * @class PriceLabel
	 * @brief Draws a small integer with PixelFont, in each digit's original seven-segment slot.
	 *
	 * Drawn with PixelFont (matching the Map Editor's text everywhere else),
	 * but each digit keeps its original DIGIT_WIDTH/GAP slot so getWidth()
	 * (and the seven-segment constants/segmentsFor() the test suite pins)
	 * stay unchanged. The label sets its own draw color inside Render(), so
	 * Window::render() draws it without extra color setup.
	 */
	class PriceLabel : public AbstractRenderable
	{
	public:
		/// Creates a price label whose leftmost glyph starts at (x, y), showing the given value.
		PriceLabel(int x, int y, int value);

		/// Draws the current value with PixelFont in the fixed price color.
		virtual void Render(::SDL_Renderer* p_renderer) override;

		/// Enables or disables rendering of this label.
		virtual void setRender(bool p_render) override;

		/// Changes the displayed value.
		void setValue(int value);

		/// Moves the label so its leftmost digit starts at (x, y).
		void setPosition(int x, int y);

		/// Total pixel width of the current value's digits (digits plus inter-digit gaps).
		int getWidth() const;

		/// Pixel height of the digits.
		int getHeight() const;

		/**
		 * @brief Returns the lit-segment bitmask of one displayable digit.
		 * @param digit Character '0'-'9'; any other character lights nothing.
		 * @return Bitmask: bit0 top, bit1 top-left, bit2 top-right, bit3 middle,
		 *         bit4 bottom-left, bit5 bottom-right, bit6 bottom.
		 */
		static int segmentsFor(char digit);

	private:
		int m_x; ///< Screen x of the leftmost glyph.
		int m_y; ///< Screen y of the top of the glyphs.
		std::string m_text; ///< Value currently displayed.

		static constexpr int DIGIT_WIDTH = 12;  ///< Width of one digit in pixels.
		static constexpr int DIGIT_HEIGHT = 20; ///< Height of one digit in pixels.
		static constexpr int SEGMENT = 3;       ///< Thickness of each segment in pixels.
		static constexpr int GAP = 4;           ///< Horizontal space between two digits.
	};
}

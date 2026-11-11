#pragma once
#include <SDL.h>
#include "AbstractRenderable.h"
#include "Vector3.h"

namespace Renderer
{
	/**
	 * @class NumberLabel
	 * @brief Draws a small integer as seven-segment digits, like a calculator display.
	 *
	 * The Renderer has no font pipeline, so numeric readouts (e.g. the
	 * player's coin counter) are drawn as filled rectangles: each digit
	 * lights a subset of seven segments. The label sets its own draw color
	 * inside Render(), so Window::render() draws it without extra setup.
	 */
	class NumberLabel : public AbstractRenderable
	{
	public:
		/// Creates a number label whose leftmost digit starts at (x, y), showing the given value, drawn in the given RGB color (defaults to the original gold).
		NumberLabel(int x, int y, int value, Math::Vec3<int> color = Math::Vec3<int>(255, 215, 0));

		/// Draws the current value as seven-segment digits in the fixed label color.
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
		 * @brief Returns the lit-segment bitmask of one decimal digit.
		 * @param digit Character '0'-'9'; any other character lights nothing.
		 * @return Bitmask: bit0 top, bit1 top-left, bit2 top-right, bit3 middle,
		 *         bit4 bottom-left, bit5 bottom-right, bit6 bottom.
		 */
		static int segmentsFor(char digit);

	private:
		int m_x; ///< Screen x of the leftmost digit.
		int m_y; ///< Screen y of the top of the digits.
		int m_value; ///< Integer currently displayed.
		Math::Vec3<int> m_color; ///< RGB color the digits are drawn in.

		static constexpr int DIGIT_WIDTH = 12;  ///< Width of one digit in pixels.
		static constexpr int DIGIT_HEIGHT = 20; ///< Height of one digit in pixels.
		static constexpr int SEGMENT = 3;       ///< Thickness of each segment in pixels.
		static constexpr int GAP = 4;           ///< Horizontal space between two digits.
	};
}

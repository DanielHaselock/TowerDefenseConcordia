#pragma once

namespace Math
{
	/**
	 * @class Color
	 * @brief An RGBA color value.
	 */
	class Color
	{

	public:
		/** @brief Creates a color with the default component values. */
		Color();

		/** @brief Creates a color from its red, green, blue, and alpha components. */
		Color(int p_r, int p_g, int p_b, int p_a);

		/** @brief Copies another color. */
		Color(Color& other);

		/** @brief Creates a color from its red, green, blue, and alpha components. */
		void setColor(int p_r, int p_g, int p_b, int p_a);

		/** @brief Returns the red component. */
		int R() { return r; };

		/** @brief Returns the green component. */
		int G() { return g; };

		/** @brief Returns the blue component. */
		int B() { return b; };

		/** @brief Returns the alpha component. */
		int A() { return a; };



	private:

		int r, g, b, a;

	};
}

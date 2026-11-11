#pragma once
#include "SDL.h"
#include "AbstractRenderable.h"
#include "Color.h"
#include "Vector4.h"

namespace Renderer
{
	/**
	 * @class Rectangle
	 * @brief A solid-color, axis-aligned rectangle that can be rendered by Window.
	 *
	 * Used for simple UI elements such as menu backgrounds. Position/size
	 * is stored as (x, y, w, h) and color as RGBA; Render() fills the
	 * rectangle on the given SDL_Renderer.
	 */
	class Rectangle : public AbstractRenderable
	{
	public:
		/// Creates a rectangle at (x, y) with size (z, w) used as (width, height).
		Rectangle(int x, int y, int z, int w);

		/// Creates a rectangle from a (x, y, w, h) vector.
		Rectangle(Math::Vec4<int> vec);

		/// Sets the rectangle's fill color (RGBA).
		void setColor(int r, int g, int b, int a);

		/// Fills this rectangle's bounds on the given renderer using its current color.
		virtual void Render(::SDL_Renderer* p_renderer) override;

		/// Enables or disables rendering of this rectangle.
		virtual void setRender(bool p_render) override;

		/// Returns a mutable reference to this rectangle's fill color.
		Math::Color& getColor();

	private:

		Math::Vec4<int> m_size; ///< Bounds of the rectangle as (x, y, width, height).

		Math::Color color; ///< Fill color (RGBA) used when rendering.
	};

}

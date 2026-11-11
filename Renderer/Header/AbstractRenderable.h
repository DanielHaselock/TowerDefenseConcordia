#pragma once
#include <SDL.h>

namespace Renderer
{
	/**
	 * @class AbstractRenderable
	 * @brief Abstract class extended by anything that can be drawn by Window::render().
	 *
	 * Window keeps a list of AbstractRenderable pointers (textures and rectangles)
	 * and, for each one whose p_shouldRender flag is set, calls Render()
	 * once per frame. Implementations are responsible for their own SDL
	 * draw calls.
	 */
	class AbstractRenderable
	{
	public:
		virtual ~AbstractRenderable() = default;

		/// Issues the SDL draw call(s) needed to display this object using the given renderer.
		virtual void Render(::SDL_Renderer* p_renderer) = 0;

		/// Enables or disables rendering of this object (sets p_shouldRender, and any related sub-state).
		virtual void setRender(bool p_render) = 0;

		/** @brief Returns whether this object is currently marked for rendering. */
		bool shouldRender() const { return m_shouldRender; }

	protected:
		/** @brief Controls whether the renderer draws this object. */
		bool m_shouldRender = true;
	};

}
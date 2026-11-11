#pragma once
#include "SDL.h"

namespace Renderer
{
	/**
	 * @class Renderer
	 * @brief Thin wrapper around the raw SDL_Renderer used to draw each frame.
	 *
	 * Owns the SDL_Renderer created for a given SDL_Window and exposes the
	 * small set of operations (clear, draw, present, set color) that
	 * Window needs to run its render loop.
	 */
	class Renderer
	{
	public:

		/// Creates the SDL_Renderer for the given window and initializes SDL_image (PNG support).
		void createRenderer(SDL_Window* p_window);

		/// Presents the back buffer, displaying everything drawn this frame.
		void refreshRenderer();

		/// Clears the renderer to the current draw color, ready for a new frame.
		void clearRenderer();

		/// Draws a single pixel at (x, y) using the current draw color.
		void drawPoint(int x, int y);

		/// Sets the renderer's draw color, used by subsequent fill/draw calls.
		void setColor(int r, int g, int b, int a);

		/// Resets the draw color to opaque white (the default used between draws).
		void setColor(); //Default

		/// Destroys the underlying SDL_Renderer.
		void quit();

		/// Returns the raw SDL_Renderer handle, for calls that need direct SDL access.
		SDL_Renderer* getRenderer();

	private:
		SDL_Renderer* m_renderer = nullptr; ///< Owned native SDL renderer handle.

	};

}
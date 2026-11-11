#pragma once
#include "Window.h"
#include <functional>
#include <vector>
namespace App
{
	/**
	 * @class InputManager
	 * @brief Translates raw window/mouse state into game-level callbacks.
	 *
	 * InputManager does not own any game logic itself: callers (e.g.
	 * GameManager) register callbacks for click, release, and move events
	 * via the Attach*Event() methods, and InputManager invokes them each
	 * frame based on the Window's current mouse state.
	 */
	class InputManager
	{
	public:

		/// Creates an input manager with no callbacks attached yet.
		InputManager();

		/**
		 * @brief Polls the window for pending OS/SDL events.
		 * @param window Window whose events should be polled.
		 * @return True if at least one event was processed this frame,
		 *         false if there were none.
		 */
		bool pollEvents(::Renderer::Window& window);

		/**
		 * @brief Dispatches the appropriate attached callbacks based on the window's current mouse state.
		 *
		 * Invokes every click callback if the mouse button was just
		 * pressed, every move callback if the button state hasn't
		 * changed this frame, or every release callback otherwise.
		 * @param window Window whose mouse state drives the dispatch.
		 */
		void manageMouse(::Renderer::Window& window);

		/// Registers a callback to be invoked when the mouse button is pressed.
		void AttachClickEvent(std::function<void()> function);

		/// Registers a callback to be invoked when the mouse button is released.
		void AttachReleaseEvent(std::function<void()> function);

		/// Registers a callback to be invoked while the mouse moves without a button state change.
		void AttachMoveEvent(std::function<void()> function);

		/// Removes all registered mouse callbacks (used when transitioning from editor to gameplay).
		void clearCallbacks();
	private:

		std::vector<std::function<void()>> mouseMoveFunctions; ///< Callbacks invoked by manageMouse() on mouse movement.
		std::vector<std::function<void()>> mouseClickFunctions; ///< Callbacks invoked by manageMouse() on mouse button press.
		std::vector<std::function<void()>> mouseReleaseFunctions; ///< Callbacks invoked by manageMouse() on mouse button release.

	};
}
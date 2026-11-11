#pragma once
#include "SDL.h"

namespace Renderer
{
	/**
	 * @class Mouse
	 * @brief Plain snapshot of the current mouse state.
	 *
	 * Updated every frame by Window::handleEvent() while polling SDL events.
	 */
	class Mouse
	{
	public:
		/// Returns the current horizontal mouse position in window coordinates.
		int getX() const;

		/// Returns the current vertical mouse position in window coordinates.
		int getY() const;

		/// Returns whether a mouse button is currently being held down.
		bool isPressed() const;

		/// Returns the last SDL mouse event type that updated this state.
		Uint32 getType() const;

		/// Returns whether the mouse state changed during the current frame.
		bool hasStateChanged() const;

		/// Sets the mouse cursor position in window coordinates.
		/// @param x Horizontal position.
		/// @param y Vertical position.
		void setPosition(int x, int y);

		/// Sets whether a button is currently pressed.
		/// @param pressed True when a mouse button is held down.
		void setPressed(bool pressed);

		/// Sets the SDL event type associated with the most recent mouse update.
		/// @param type SDL mouse event type, such as SDL_MOUSEMOTION or SDL_MOUSEBUTTONDOWN.
		void setType(Uint32 type);

		/// Marks whether the mouse state changed during the current frame.
		/// @param changed True if the current event poll produced a mouse state update.
		void setStateChanged(bool changed);

		/// Returns the SDL button identifier (e.g. SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT) from the latest button event.
		Uint8 getButton() const;

		/// Records which mouse button triggered the latest button event.
		void setButton(Uint8 button);

		/// True if a mouse button was pressed during the current frame's event poll.
		bool wasJustPressed() const;

		/// Marks whether a mouse button was pressed during the current frame's event poll.
		void setJustPressed(bool justPressed);

		/// True if a mouse button was released during the current frame's event poll.
		bool wasJustReleased() const;

		/// Marks whether a mouse button was released during the current frame's event poll.
		void setJustReleased(bool justReleased);

	private:
		int m_x = 0;		///< Current mouse position in window coordinates.
		int m_y = 0;		///< Current mouse position in window coordinates.
		bool m_isPressed = false;	///< True while a mouse button is held down.
		Uint32 m_type = 0;	///< SDL event type that last updated this state (e.g. SDL_MOUSEMOTION).
		bool m_stateChanged = false;	///< True for the frame a mouse button was pressed or released.
		Uint8 m_button = SDL_BUTTON_LEFT; ///< Button identifier from the most recent press/release event.
		bool m_justPressed = false; ///< True if SDL_MOUSEBUTTONDOWN occurred during this frame's poll.
		bool m_justReleased = false; ///< True if SDL_MOUSEBUTTONUP occurred during this frame's poll.
	};
}

#pragma once
#include "Texture.h"
namespace Renderer
{
	/// Identifies which visual state (and which texture/sprite slot) a Button is currently showing.
	enum ButtonSprite
	{
		BUTTON_SPRITE_MOUSE_OUT = 0, ///< Default appearance, mouse not over the button.
		BUTTON_SPRITE_MOUSE_OVER_MOTION = 1, ///< Mouse is hovering over the button.
		BUTTON_SPRITE_MOUSE_DOWN = 2, ///< Mouse button is held down over the button.
		BUTTON_SPRITE_MOUSE_UP = 3, ///< Mouse button was just released over the button.
		BUTTON_SPRITE_TOTAL = 4 ///< Number of defined button states.
	};

	/**
	 * @class Button
	 * @brief Clickable UI element backed by one or more Textures, with simple hit-testing.
	 *
	 * A Button has a position/size on screen and a set of textures (either
	 * separate Texture objects, one per ButtonSprite state, or sprites
	 * within a single sprite-sheet Texture, see setUseSpriteSheet()).
	 * handlePress() checks whether the current mouse position is inside the
	 * button's bounds and flags it as pressed; callers are expected to poll
	 * m_pressed and reset it as needed. The Button does not own the
	 * textures passed to it (they are typically owned by Window).
	 */
	class Button
	{
	public:
		/// Creates an empty button with no texture assigned yet.
		Button();

		/// Creates a button using the given texture as both its current and first texture.
		Button(Texture* texture);

		/// Clears the list of tracked textures. Does not delete them (the Button does not own its textures).
		~Button();

		/// Moves the button (and its texture/sprite) to (x, y).
		void setPosition(int x, int y);

		/// Resizes the button (and its texture/sprite) to (w, h).
		void setScale(int w, int h);

		/// Checks the current mouse position against the button's bounds and updates m_pressed via changebutton().
		void handlePress();

		/// Sets m_pressed to true if inside is true (the button was hit).
		void changebutton(bool inside);

		/**
		 * @brief Swaps the visible texture/sprite to match mCurrentSprite.
		 *
		 * When using a sprite sheet, enables only the sprite at index
		 * mCurrentSprite and disables the rest. Otherwise, hides every
		 * texture in m_listTextures and shows/selects the one at index
		 * mCurrentSprite.
		 */
		void changeTexture();

		/// Adds another texture/state to this button's texture list, positioning and scaling it to match the button.
		void addTexture(Texture* p_newTexture);

		/// Hit-tests (x, y) against the current sprite-sheet entry's destination rectangle.
		bool checkSpriteSheetTexture(int x, int y);

		/// Hit-tests (x, y) against the current texture's destination rectangle.
		bool checkRegularTexture(int x, int y);

		/// Sets the button's current texture pointer (ignored if p_text is null).
		void setTexture(Texture* p_text);

		/// Switches the button between sprite-sheet mode and separate-texture mode.
		void setUseSpriteSheet(bool p_sprite);

		/**
		 * @brief Activates or deactivates the button and updates which texture/sprite is visible accordingly.
		 * @param p_active True to show the button's current sprite/texture, false to hide it.
		 */
		void setButtonActive(bool p_active);

		/// Returns the texture currently assigned to this button.
		Texture* getCurrentTexture();

		bool m_active = false; ///< Whether the button is currently active/visible.

		bool m_pressed = false; ///< Set by handlePress()/changebutton() when the button was clicked; callers should read and reset it.

	private:

		int m_x = 0; ///< Current x position of the button.
		int m_y = 0; ///< Current y position of the button.

		int m_w = 0; ///< Current width of the button.
		int m_h = 0; ///< Current height of the button.

		Texture* m_currentTexture = nullptr; ///< Texture (or sprite-sheet source) currently displayed. Not owned by Button.

		std::vector<Texture*> m_listTextures; ///< All textures registered for this button's states. Not owned by Button.

		ButtonSprite mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT; ///< Visual state currently selected (and sprite-sheet index when applicable).

		bool m_useSpriteSheet = false; ///< True if this button's states are sprites within one sheet texture, rather than separate Texture objects.
	};
}

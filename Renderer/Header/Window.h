#pragma once
#include "Renderer.h"
#include "Texture.h"
#include "Shape.h"
#include "PriceLabel.h"
#include "NumberLabel.h"
#include "TextLabel.h"
#include "StyledButton.h"
#include "InspectPanel.h"
#include "OutcomeOverlay.h"
#include "SelectionHighlight.h"
#include "Vector4.h"
#include "Mouse.h"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640
namespace Renderer
{
	/**
	 * @class Window
	 * @brief Owns the SDL window/renderer and every texture and rectangle drawn in it.
	 *
	 * Window is the single entry point used by the rest of the game to create
	 * the SDL window, poll input events, register textures/rectangles to be
	 * drawn, and render each frame. It owns the lifetime of every
	 * Renderer::Texture, Renderer::Rectangle, Renderer::PriceLabel and
	 * Renderer::NumberLabel created through addTexture(), addRect(),
	 * addPriceLabel() and addNumberLabel(): they are destroyed by quit() (or
	 * the Window's own destruction sequence), so callers should not delete
	 * them manually.
	 */
	class Window
	{
	public:
		/// Initializes SDL video, creates the native SDL_Window, and sets up the underlying Renderer.
		void createWindowRenderer();

		/// Polls and dispatches every pending SDL event for this frame via handleEvent().
		/// @return true if at least one event was processed, false if there were none.
		bool pollEvents();

		/// Clears the renderer, draws every renderable in m_renderTargets that should render, and presents the frame.
		void render();

		/// Dispatches a single SDL event to the appropriate handler (quit, keyboard, mouse).
		void handleEvent(SDL_Event p_event);

		/// Records a key-down event; also triggers quit() when the key is Escape.
		void handleKeyboardDown(SDL_KeyboardEvent p_event);

		/// Records a key-up event.
		void handleKeyboardUp(SDL_KeyboardEvent p_event);

		/// Frees all textures, rectangles and render targets, then tears down the renderer, window, and SDL itself.
		void quit();

		/// True once the user has requested to quit (window close or Escape key).
		bool getQuit();

		/// Sets whether the application should quit on the next loop check.
		void setQuit(bool p_quit);

		/// Toggles whether the texture at index numb in m_Textures is rendered.
		void selectTexture(int numb, bool p_render);

		/**
		 * @brief Loads an image from disk as a new Texture and registers it for rendering.
		 *
		 * The Window takes ownership of the returned Texture: it is tracked in
		 * m_Textures/m_renderTargets and freed by quit().
		 * @param p_path Path to the image file to load.
		 * @param p_scale Optional destination rectangle (x, y, w, h); if its magnitude is 0, the loaded image's own size is used.
		 * @param p_colorToIgnore Optional color to treat as transparent (color key).
		 * @return Pointer to the newly created, Window-owned Texture.
		 */
		::Renderer::Texture* addTexture(std::string p_path, Math::Vec4<int> p_scale = Math::Vec4<int>(), Math::Vec3<int> p_colorToIgnore = Math::Vec3<int>::error);

		/**
		 * @brief Creates a new solid-color Rectangle and registers it for rendering.
		 *
		 * The Window takes ownership of the returned Rectangle: it is tracked
		 * in m_recs/m_renderTargets and freed by quit().
		 * @param p_scale Rectangle bounds as (x, y, w, h).
		 * @param p_color Fill color as (r, g, b, a).
		 * @return Pointer to the newly created, Window-owned Rectangle.
		 */
		::Renderer::Rectangle* addRect(Math::Vec4<int> p_scale, Math::Vec4<int> p_color);

		/**
	 * @brief Creates a new PriceLabel and registers it for rendering.
	 *
	 * The Window takes ownership of the returned PriceLabel: it is tracked
	 * in m_priceLabels/m_renderTargets and freed by quit().
	 * @param x Screen x of the leftmost digit.
	 * @param y Screen y of the top of the digits.
	 * @param value Integer to display as seven-segment digits.
	 * @return Pointer to the newly created, Window-owned PriceLabel.
	 */
	::Renderer::PriceLabel* addPriceLabel(int x, int y, int value);

	/**
	 * @brief Creates a new NumberLabel and registers it for rendering.
	 *
	 * The Window takes ownership of the returned NumberLabel: it is
	 * tracked in m_numberLabels/m_renderTargets and freed by quit().
	 * @param x Screen x of the leftmost digit.
	 * @param y Screen y of the top of the digits.
	 * @param value Integer to display as seven-segment digits.
	 * @param color RGB color the digits are drawn in (defaults to gold).
	 * @return Pointer to the newly created, Window-owned NumberLabel.
	 */
	::Renderer::NumberLabel* addNumberLabel(int x, int y, int value, Math::Vec3<int> color = Math::Vec3<int>(255, 215, 0));

		/**
		 * @brief Creates a new procedurally-drawn text caption and registers it for rendering.
		 *
		 * Window takes ownership (freed alongside every other Texture by
		 * quit()/ClearSession(), since TextLabel subclasses Texture).
		 */
		::Renderer::TextLabel* addTextLabel(int x, int y, std::string text, int scale, SDL_Color color);

		/**
		 * @brief Creates a new procedurally-drawn button and registers it for rendering.
		 *
		 * Window takes ownership (StyledButton subclasses Texture, so it is
		 * tracked/freed exactly like one).
		 */
		::Renderer::StyledButton* addStyledButton(int x, int y, int w, int h, std::string label, ::Renderer::ButtonVariant variant);

		/**
		 * @brief Creates a new procedurally-drawn inspection panel background and registers it for rendering.
		 * @param rows Caption rows drawn down the left side of the panel.
		 * @param decimalPointOffset Panel-local (x, y) of an optional decimal point between
		 * two value columns (e.g. the tower rate-of-fire row); omit for no decimal point.
		 */
		::Renderer::InspectPanel* addInspectPanel(int x, int y, int w, int h, std::string title, SDL_Color accent,
			std::vector<::Renderer::InspectPanel::Row> rows, SDL_Point decimalPointOffset = SDL_Point{-1, -1});

		/// Creates a new full-screen win/lose overlay and registers it for rendering.
		::Renderer::OutcomeOverlay* addOutcomeOverlay(int x, int y, int w, int h, bool won);

		/// Creates a new border-only selection highlight and registers it for rendering.
		::Renderer::SelectionHighlight* addSelectionHighlight(int x, int y, int w, int h);

		/// Enables rendering for the rectangle at index numb in m_recs.
		void showRect(int numb);

		/// Disables rendering for every registered render target (textures and rectangles).
		void resetRenderTargets();

		/**
		 * @brief Frees every texture/rect/label created so far, emptying the Window back to a blank slate.
		 *
		 * Same cleanup as quit(), minus tearing down the SDL renderer/window
		 * themselves - so a new game session can immediately start adding
		 * fresh textures/rects/labels into this same Window (e.g. for a
		 * "click to restart" flow or transitioning from MapEditor to GameManager).
		 */
		void ClearSession();

		/// Registers an external AbstractRenderable in draw order without taking ownership.
		void attachRenderable(::Renderer::AbstractRenderable* renderable);

		/// Removes an external AbstractRenderable previously added via attachRenderable().
		void detachRenderable(::Renderer::AbstractRenderable* renderable);

		Mouse m_mouse; ///< Latest polled mouse state.

		std::vector<SDL_Keycode> keysPressedDown; ///< Keys that went down during the current frame's event poll.
		std::vector<SDL_Keycode> keysPressedUp; ///< Keys that went up during the current frame's event poll.

	private:
		SDL_Window* m_window; ///< Native SDL window handle, owned by this Window.
		SDL_Event m_event; ///< Scratch event struct reused by pollEvents().



		Renderer m_renderer; ///< Wrapper around the SDL_Renderer used to draw every frame.
		std::vector<::Renderer::Texture*> m_Textures; ///< All textures created via addTexture(), owned by this Window.
		std::vector<::Renderer::Rectangle*> m_recs; ///< All rectangles created via addRect(), owned by this Window.
		std::vector<::Renderer::PriceLabel*> m_priceLabels; ///< All price labels created via addPriceLabel(), owned by this Window.
		std::vector<::Renderer::NumberLabel*> m_numberLabels; ///< All number labels created via addNumberLabel(), owned by this Window.
		std::vector<::Renderer::AbstractRenderable*> m_renderTargets; ///< Every renderable (textures, rectangles, price labels and number labels), in draw order.

		bool m_quit = false; ///< Set once the application should exit.


		/// Deletes and clears every texture owned by this Window.
		void freeTextures();

		/// Deletes and clears every rectangle owned by this Window.
		void freeRects();

		/// Deletes and clears every price label owned by this Window.
		void freePriceLabels();

		/// Deletes and clears every number label owned by this Window.
		void freeNumberLabels();

		/// Clears the render target list (the objects themselves are owned/freed elsewhere).
		void freeRenderTargets();
	};
}
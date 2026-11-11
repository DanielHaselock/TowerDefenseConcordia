#pragma once 
#include "Window.h"
//#include "EntityManager.h"
#include "InputManager.h"
//#include "FileHandler.h"
#include "GameManager.h"
#include "MapEditor.h"
namespace App
{
	/**
	 * @class Application
	 * @brief Owns and drives the top-level editor-to-gameplay loop.
	 *
	 * Application opens the window into the interactive MapEditor first so
	 * the player can build a custom map or generate a random map of a chosen
	 * size. Once a valid map is confirmed, Application transitions cleanly
	 * into GameManager for gameplay until the window is closed.
	 */
	class Application
	{
	public:
		/// Creates the game window, input manager, and initial MapEditor screen.
		Application();

		/**
		 * @brief Runs the editor and gameplay loop until the window is closed.
		 *
		 * Drives MapEditor until a valid map is confirmed, then tears down the
		 * editor scene and launches GameManager with the selected map.
		 */
		void mainLoop();

		/// Frees the owned map editor, input manager, and game manager.
		~Application();

	private:
		Renderer::Window window;  ///< The game's window and renderer.

		InputManager* inputManager = nullptr; ///< Owned; translates raw input into actions.
		MapEditor* mapEditor = nullptr; ///< Owned while in the map creation phase; null once gameplay begins.
		GameManager* gameManager = nullptr; ///< Owned once gameplay begins; drives overall game state/logic.
		
	};
}
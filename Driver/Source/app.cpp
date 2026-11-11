#include "app.h"
#include <chrono>
#include <memory>

App::Application::Application()
{
	window.createWindowRenderer();
	inputManager = new App::InputManager();
	mapEditor = new App::MapEditor(window, inputManager);
}


void App::Application::mainLoop()
{
	using clock = std::chrono::high_resolution_clock;

	auto previousTime = clock::now();

	while (!window.getQuit())
    {
		auto currentTime = clock::now();
		std::chrono::duration<float> elapsed = currentTime - previousTime;
		float deltaTime = elapsed.count();
		previousTime = currentTime;

		if (!window.getQuit())
		{
			window.render();

			if (mapEditor != nullptr)
			{
				mapEditor->MainLoop(deltaTime);

				if (inputManager->pollEvents(window))
				{
					inputManager->manageMouse(window);
					mapEditor->HandleKeyboard(window);
				}

				if (mapEditor->isReadyToPlay() && !window.getQuit())
				{
					std::unique_ptr<Game::Map> selectedMap = mapEditor->takeMap();
					delete mapEditor;
					mapEditor = nullptr;

					window.ClearSession();
					inputManager->clearCallbacks();
					gameManager = new App::GameManager(window, inputManager, std::move(selectedMap));
				}
			}
			else if (gameManager != nullptr)
			{
				gameManager->MainLoop(deltaTime);

				if (gameManager->ShouldRestart())
				{
					// Tear down the finished session and return to the Map Editor
					// so the player can build or randomize a fresh map.
					delete gameManager;
					gameManager = nullptr;
					delete inputManager;
					window.ClearSession();

					inputManager = new App::InputManager();
					mapEditor = new App::MapEditor(window, inputManager);
				}
				else if (inputManager->pollEvents(window))
				{
					inputManager->manageMouse(window);
				}
			}
		}
	}

	window.quit();
}

App::Application::~Application()
{
	delete mapEditor;
	delete inputManager;
	delete gameManager;
}
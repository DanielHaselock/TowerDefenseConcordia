#include "InputManager.h"
#include <functional>

App::InputManager::InputManager()
{
}

bool App::InputManager::pollEvents(::Renderer::Window& window)
{
    bool shouldCheckEvents = window.pollEvents();
    if (!shouldCheckEvents)
        return false;

    return true;
}

void App::InputManager::manageMouse(::Renderer::Window& window)
{
        const bool pressedThisFrame = window.m_mouse.wasJustPressed() ||
            (window.m_mouse.isPressed() && window.m_mouse.hasStateChanged());
        const bool releasedThisFrame = window.m_mouse.wasJustReleased() ||
            (!window.m_mouse.isPressed() && window.m_mouse.hasStateChanged());

        if (pressedThisFrame)
        {
            for (const std::function<void()>& func : mouseClickFunctions)
            {
                std::invoke(func);
            }
        }

        if (!window.m_mouse.hasStateChanged())
        {
            for (const std::function<void()>& func : mouseMoveFunctions)
            {
                std::invoke(func);
            }
        }

        if (releasedThisFrame)
        {
            for (const std::function<void()>& func : mouseReleaseFunctions)
            {
                std::invoke(func);
            }
        }
}

void App::InputManager::AttachClickEvent(std::function<void()> function)
{
    mouseClickFunctions.emplace_back(function);
}

void App::InputManager::AttachMoveEvent(std::function<void()> function)
{
    mouseMoveFunctions.emplace_back(function);
}

void App::InputManager::AttachReleaseEvent(std::function<void()> function)
{
    mouseReleaseFunctions.emplace_back(function);
}

void App::InputManager::clearCallbacks()
{
    mouseMoveFunctions.clear();
    mouseClickFunctions.clear();
    mouseReleaseFunctions.clear();
}
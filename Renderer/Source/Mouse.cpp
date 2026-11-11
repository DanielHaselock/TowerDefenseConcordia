#include "Mouse.h"

int Renderer::Mouse::getX() const
{
	return m_x;
}

int Renderer::Mouse::getY() const
{
	return m_y;
}

bool Renderer::Mouse::isPressed() const
{
	return m_isPressed;
}

Uint32 Renderer::Mouse::getType() const
{
	return m_type;
}

bool Renderer::Mouse::hasStateChanged() const
{
	return m_stateChanged;
}

void Renderer::Mouse::setPosition(int x, int y)
{
	m_x = x;
	m_y = y;
}

void Renderer::Mouse::setPressed(bool pressed)
{
	m_isPressed = pressed;
}

void Renderer::Mouse::setType(Uint32 type)
{
	m_type = type;
}

void Renderer::Mouse::setStateChanged(bool changed)
{
	m_stateChanged = changed;
}

Uint8 Renderer::Mouse::getButton() const
{
	return m_button;
}

void Renderer::Mouse::setButton(Uint8 button)
{
	m_button = button;
}

bool Renderer::Mouse::wasJustPressed() const
{
	return m_justPressed;
}

void Renderer::Mouse::setJustPressed(bool justPressed)
{
	m_justPressed = justPressed;
}

bool Renderer::Mouse::wasJustReleased() const
{
	return m_justReleased;
}

void Renderer::Mouse::setJustReleased(bool justReleased)
{
	m_justReleased = justReleased;
}

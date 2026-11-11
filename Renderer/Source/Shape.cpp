#include "Shape.h"
#include <cmath>

Renderer::Rectangle::Rectangle(int x, int y, int z, int w)
{
    m_size.X() = x;
    m_size.Y() = y;
    m_size.Z() = z;
    m_size.W() = w;
}

Renderer::Rectangle::Rectangle(Math::Vec4<int> vec)
{
    m_size.X() = vec.X();
    m_size.Y() = vec.Y();
    m_size.Z() = vec.Z();
    m_size.W() = vec.W();
}

void Renderer::Rectangle::setColor(int r, int g, int b, int a)
{
    color.setColor(r, g, b, a);
}

void Renderer::Rectangle::Render(::SDL_Renderer* p_renderer)
{
    SDL_Rect fillRect = { m_size.X(), m_size.Y(), m_size.Z(), m_size.W() };
    SDL_RenderFillRect(p_renderer, &fillRect);
}

void Renderer::Rectangle::setRender(bool p_render)
{
    m_shouldRender = p_render;
}

Math::Color& ::Renderer::Rectangle::getColor()
{
    return color;
}

#include "Renderer.h"
#include <stdio.h>
#include "SDL_image.h"
void Renderer::Renderer::createRenderer(SDL_Window* p_window)
{
	m_renderer = SDL_CreateRenderer(p_window, -1, 0);

	if (!m_renderer)
	{
		printf("Renderer could not initialize! SDL_Error: %s\n", SDL_GetError());
	}

	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
	}
}

void Renderer::Renderer::refreshRenderer()
{
	SDL_RenderPresent(m_renderer);
}

void Renderer::Renderer::clearRenderer()
{
	SDL_RenderClear(m_renderer);
}

void Renderer::Renderer::drawPoint(int x, int y)
{
	SDL_RenderDrawPoint(m_renderer, x, y);
}

void Renderer::Renderer::setColor(int r, int g, int b, int a)
{
	SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
}

void Renderer::Renderer::setColor()
{
	SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
}

void Renderer::Renderer::quit()
{
	SDL_DestroyRenderer(m_renderer);
}

SDL_Renderer* Renderer::Renderer::getRenderer()
{
	return m_renderer;
}

#include "MapGenerator.h"
#include "UiTheme.h"
#include "Vector4.h"
#include <cmath>
#include <stdexcept>

namespace
{
	// SDL_Color -> Math::Vec4<int>, so MapGenerator's flat-color panels draw
	// from the same navy/slate palette as the Map Editor and the rest of the
	// in-game HUD (see Renderer::UiTheme) instead of their own grey literals.
	Math::Vec4<int> toVec4(SDL_Color c)
	{
		return Math::Vec4<int>(c.r, c.g, c.b, c.a);
	}
}

Renderer::MapGenerator::MapGenerator(Window& window, int startingPixel, int p_rows, int p_columns, int max_width, int max_height, std::vector<int>& cellValues)
{
	// Fill the board backdrop so any unused space around a non-square map
	// stays visually distinct from the side/bottom menu panels.
	::Renderer::Rectangle* playAreaBackground = window.addRect(
		Math::Vec4<int>(startingPixel, 0, max_width - startingPixel, max_height - startingPixel),
		toVec4(UiTheme::BackgroundDark));
	::Renderer::Rectangle* sideMenuBackground = window.addRect(Math::Vec4<int>(0, 0, startingPixel, WINDOW_HEIGHT), toVec4(UiTheme::PanelDark));
	::Renderer::Rectangle* bottomMenuBackground = window.addRect(Math::Vec4<int>(0, WINDOW_HEIGHT - startingPixel, WINDOW_WIDTH, startingPixel), toVec4(UiTheme::PanelDark));
	(void)playAreaBackground;
	(void)sideMenuBackground;
	(void)bottomMenuBackground;

	// Thin dividers between the menus and the play area, echoing the Map Editor's own panel seams.
	window.addRect(Math::Vec4<int>(startingPixel - 1, 0, 1, WINDOW_HEIGHT), toVec4(UiTheme::BorderMuted));
	window.addRect(Math::Vec4<int>(0, WINDOW_HEIGHT - startingPixel - 1, WINDOW_WIDTH, 1), toVec4(UiTheme::BorderMuted));

	const int squareSize = CalculateSquareTileSize(p_rows, p_columns, startingPixel, max_width, max_height);
	tileWidth = squareSize;
	tileHeight = squareSize;

	menuTileSize = std::min((float)WINDOW_WIDTH / 8, (float)WINDOW_HEIGHT / 6);

	// Bordered "shop" card behind the 3 tower icons + their price labels,
	// echoing the Map Editor's own sidebar section cards. Registered (and so
	// drawn) before the shared sprite-sheet texture below, so it sits underneath.
	{
		const int iconTop = WINDOW_HEIGHT / 20;
		const int iconBottom = WINDOW_HEIGHT / 20 + WINDOW_HEIGHT / 2 + static_cast<int>(menuTileSize);
		const int cardTop = iconTop - 12;
		const int cardBottom = iconBottom + 32; // room for the last icon's price label
		const int cardLeft = 6;
		const int cardWidth = startingPixel - 2 * cardLeft;

		window.addRect(
			Math::Vec4<int>(cardLeft - 2, cardTop - 2, cardWidth + 4, cardBottom - cardTop + 4),
			toVec4(UiTheme::BorderStrong));
		window.addRect(
			Math::Vec4<int>(cardLeft, cardTop, cardWidth, cardBottom - cardTop),
			toVec4(UiTheme::PanelMed));
	}

	//Menu Generation

	::Renderer::Texture* texture = window.addTexture("Assets/smallSpriteSheetV2.png", Math::Vec4<int>(), Math::Vec3<int>(32, 32, 32));
	//Green
	menuSprites.emplace_back(texture->useSpecificSpriteSheet(255, 255, 64, 64, Math::Vec4<int>(0, WINDOW_HEIGHT / 20, menuTileSize, menuTileSize)));
	//Red
	menuSprites.emplace_back(texture->useSpecificSpriteSheet(319, 255, 64, 64, Math::Vec4<int>(0, (WINDOW_HEIGHT / 20 + WINDOW_HEIGHT / 4), menuTileSize, menuTileSize)));
	//Blue
	menuSprites.emplace_back(texture->useSpecificSpriteSheet(191, 255, 64, 64, Math::Vec4<int>(0, (WINDOW_HEIGHT / 20 + WINDOW_HEIGHT / 2), menuTileSize, menuTileSize)));

	// Build tiles in row-major order, matching the backend's flattened cell list.
	for (int row = 0; row < p_rows; ++row)
	{
		for (int column = 0; column < p_columns; ++column)
		{
			// A flat row-major index advances across columns, then continues on the next row.
			const int cellNumber = row * p_columns + column;
			const int text = GetTexture(cellValues[cellNumber]);
			// Rows determine screen y; columns determine screen x; cells stay square and centered.
			const SDL_FRect rect = CalculateCenteredSquareTileRect(
				row, column, p_rows, p_columns, startingPixel, max_width, max_height);
			const Math::Vec4<int> tileScale(
				static_cast<int>(rect.x),
				static_cast<int>(rect.y),
				static_cast<int>(rect.w),
				static_cast<int>(rect.h));
			::Renderer::RendererMap* tile = new ::Renderer::RendererMap(::Renderer::TileType::scenery,
				texture->useSpecificSpriteSheet(text, 0, 64, 64, tileScale),
				texture->useSpecificSpriteSheet(191, 0, 64, 64, tileScale),
				texture,
				row * column,
				cellNumber);
			m_tiles.emplace_back(tile);
		}
	}
}

int Renderer::MapGenerator::CalculateTileWidth(int p_columns, int startingPixel, int max_width)
{
	int amount = max_width - startingPixel;
	int width = amount / p_columns;
	return width;
}

int Renderer::MapGenerator::CalculateTileHeight(int p_rows, int startingPixel, int max_height)
{
	int amount = max_height - startingPixel;
	return amount / p_rows;
}

int Renderer::MapGenerator::CalculateSquareTileSize(int p_rows, int p_columns, int startingPixel, int max_width, int max_height)
{
	const float availWidth = static_cast<float>(max_width - startingPixel);
	const float availHeight = static_cast<float>(max_height - startingPixel);
	const float exactSize = std::min(availWidth / p_columns, availHeight / p_rows);
	return std::max(1, static_cast<int>(std::lround(exactSize)));
}

SDL_FRect Renderer::MapGenerator::CalculateTileRect(int row, int column, int startingPixel, int tileWidth, int tileHeight)
{
	return SDL_FRect{
		// The menu offset reserves the left side; columns advance horizontally from there.
		static_cast<float>(startingPixel + column * tileWidth),
		// Rows advance vertically from the top of the window.
		static_cast<float>(row * tileHeight),
		static_cast<float>(tileWidth),
		static_cast<float>(tileHeight)
	};
}

SDL_FRect Renderer::MapGenerator::CalculateCenteredSquareTileRect(
	int row,
	int column,
	int p_rows,
	int p_columns,
	int startingPixel,
	int max_width,
	int max_height)
{
	const float availWidth = static_cast<float>(max_width - startingPixel);
	const float availHeight = static_cast<float>(max_height - startingPixel);
	const float exactTileSize = std::max(1.0f, std::min(availWidth / p_columns, availHeight / p_rows));
	const float totalGridW = exactTileSize * p_columns;
	const float totalGridH = exactTileSize * p_rows;
	const float originX = static_cast<float>(startingPixel) + (availWidth - totalGridW) * 0.5f;
	const float originY = (availHeight - totalGridH) * 0.5f;

	const int x0 = static_cast<int>(std::lround(originX + column * exactTileSize));
	const int x1 = static_cast<int>(std::lround(originX + (column + 1) * exactTileSize));
	const int y0 = static_cast<int>(std::lround(originY + row * exactTileSize));
	const int y1 = static_cast<int>(std::lround(originY + (row + 1) * exactTileSize));

	return SDL_FRect{
		static_cast<float>(x0),
		static_cast<float>(y0),
		static_cast<float>(std::max(1, x1 - x0)),
		static_cast<float>(std::max(1, y1 - y0))
	};
}

int Renderer::MapGenerator::GetTexture(int cellType)
{
	// Each case returns the x-offset, in pixels, of that cell type's ground tile within the shared sprite sheet.
	switch (cellType)
	{
	case 0:
		return 383;
		break;
	case 1:
		return 319;
		break;

	case 2:
		return 255;
		break;

	case 3:
		return 446;
		break;

	default:
        throw std::invalid_argument("Invalid cell type");
	}
}

Renderer::RendererMap* Renderer::MapGenerator::GetClosestTile(int x, int y)
{
	::Renderer::RendererMap* closestTile = nullptr;
	// Compares squared distances to avoid an unnecessary sqrt per tile; fine since we only need the ordering.
	float closestDistanceSquared = std::numeric_limits<float>::max();

	for (::Renderer::RendererMap* tile : m_tiles)
	{
		SDL_FRect rect = tile->getTilePos();

		float centerX = rect.x + rect.w * 0.5f;
		float centerY = rect.y + rect.h * 0.5f;

		float dx = x - centerX;
		float dy = y - centerY;

		float distanceSquared = dx * dx + dy * dy;

		if (distanceSquared < closestDistanceSquared)
		{
			closestDistanceSquared = distanceSquared;
			closestTile = tile;
		}
	}

	return closestTile;
}

Renderer::MapGenerator::~MapGenerator()
{
	for (int i = 0; i < m_tiles.size(); ++i)
	{
		delete m_tiles[i];
	}

}

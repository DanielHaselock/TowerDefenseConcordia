#pragma once
#include "Window.h"
#include "RendererMap.h"
namespace Renderer
{
	/**
	 * @class MapGenerator
	 * @brief Builds and owns the grid of renderable map tiles and the side-menu sprites.
	 *
	 * On construction, lays out the side/bottom menu background rectangles,
	 * computes each tile's pixel size from the window dimensions and the
	 * reserved menu width (startingPixel), creates the three menu tower
	 * icons, and creates one Renderer::RendererMap per grid cell using the
	 * cell types supplied in cellValues. MapGenerator owns every tile it
	 * creates and deletes them in its destructor.
	 */
	class MapGenerator
	{
		public:

			/**
			 * @brief Lays out the menu UI and builds every map tile for a p_rows x p_columns grid.
			 * @param window Window used to create the background rectangles, menu textures and tile textures.
			 * @param startingPixel Pixel width/height reserved for the side/bottom menu, and the left offset where the grid begins.
			 * @param p_rows Number of tile rows.
			 * @param p_columns Number of tile columns.
			 * @param max_width Total usable width (typically WINDOW_WIDTH) the grid must fit into.
			 * @param max_height Total usable height (typically WINDOW_HEIGHT) the grid must fit into.
			 * @param cellValues Flattened grid of cell type codes (0-3) used to pick each tile's texture via GetTexture().
			 */
			MapGenerator(Window& window, int startingPixel, int p_rows, int p_columns, int max_width, int max_height, std::vector<int>& cellValues);

			/// Computes the pixel width of one tile so that p_columns tiles fit in (max_width - startingPixel).
			static int CalculateTileWidth(int p_columns, int startingPixel, int max_width);

			/// Computes the pixel height of one tile so that p_rows tiles fit in (max_height - startingPixel).
			static int CalculateTileHeight(int p_rows, int startingPixel, int max_height);

			/// Computes the uniform square side length (width == height) so a p_rows x p_columns grid fits in the play area without distortion.
			static int CalculateSquareTileSize(int p_rows, int p_columns, int startingPixel, int max_width, int max_height);

			/// Returns the screen rectangle for a cell, with rows vertical and columns horizontal.
			static SDL_FRect CalculateTileRect(int row, int column, int startingPixel, int tileWidth, int tileHeight);

			/// Returns the centered square screen rectangle for (row, column) inside the play area.
			static SDL_FRect CalculateCenteredSquareTileRect(int row, int column, int p_rows, int p_columns, int startingPixel, int max_width, int max_height);

			/// Returns the pixel size used for the square menu tower icons.
			int GetMenuTileSize() { return menuTileSize; }

			/// Maps a cell type code (0-3) to the sprite-sheet x-offset of its ground texture.
			int GetTexture(int cellType);

			/// Returns the tile whose center is closest to the given pixel coordinates (used for mouse hover/placement).
			RendererMap* GetClosestTile(int x, int y);

			/// Returns the three side-menu tower icon sprites (green/red/blue).
			std::vector<spriteSheetTexture*>& getMenuSprites() {return menuSprites;}

			/// Returns every tile generated for the grid, in row-major order.
			std::vector<RendererMap*> getTiles() {return m_tiles;}

			/// Returns the computed pixel width of a single tile.
			int getWidth() { return tileWidth; }
			/// Returns the computed pixel height of a single tile.
			int getHeight() { return tileHeight; }

			/// Deletes every tile owned by this generator.
			~MapGenerator();
	private:
		std::vector<RendererMap*> m_tiles; ///< All grid tiles created by this generator, owned here.
		std::vector<spriteSheetTexture*> menuSprites; ///< The three side-menu tower icon sprites.

		int menuTileSize; ///< Pixel size (width == height) used for menu icons.

		int tileWidth, tileHeight = 0; ///< Computed pixel size of a single grid tile. Note: only tileHeight gets the "= 0" default.
	};
}
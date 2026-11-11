#pragma once
#include "Window.h"
#include "Texture.h"
#include "MapGenerator.h"

namespace Renderer
{
	/**
	 * @class GameRenderer
	 * @brief Top-level facade coordinating the map, menu, and spawned sprites for a running game.
	 *
	 * GameRenderer is the main entry point the gameplay code talks to: it
	 * creates the MapGenerator and shared sprite-sheet texture, exposes
	 * convenience methods to spawn critter/bullet sprites, place/upgrade/
	 * remove turrets on tiles, and track which tile the mouse is currently
	 * closest to. It owns the MapGenerator (deleted in its destructor);
	 * mainTexture/cancelTexture/upgradeTexture are created through
	 * Window::addTexture() and therefore remain owned by the Window.
	 */
	class GameRenderer
	{
	public:
		/**
		 * @brief Builds the map grid/menu (via MapGenerator) and loads the shared sprite sheet plus cancel/upgrade icons.
		 * @param window Window used to create textures and background rectangles.
		 * @param rows Number of grid rows.
		 * @param columns Number of grid columns.
		 * @param cellValues Flattened grid of cell type codes used to build each tile.
		 * @param towerPrices Purchase price of each tower type, index-aligned with the side-menu icons; drawn under each icon.
		 */
		void CreateGame(Window& window, int rows, int columns, std::vector<int>& cellValues, const std::vector<int>& towerPrices);

		/// Updates currClosestTile to the tile nearest the given mouse position; called once per frame.
		void MainLoop(int mouseX, int mouseY, float deltatime);

		/// Returns the tile closest to the last position passed to MainLoop().
		RendererMap* getClosestTile() { return currClosestTile; }

		/// Returns how many side-menu tower icons exist.
		int GetAmountMenuSprites(){ return mapGenerator->getMenuSprites().size(); }

		/// Enables or disables rendering of one sprite in the shared mainTexture sprite sheet (e.g. a menu icon being dragged).
		void setSpriteSheetRender(int numb, bool render);

		/// Returns the destination rectangle of the given side-menu icon.
		SDL_FRect getSpriteRect(int numb) { return mapGenerator->getMenuSprites()[numb]->m_pos_scale; }

		/// Centers the drag-preview sprites (the 3 tower icons) on the given mouse position.
		void handleMouseMove(int x, int y);

		/// Hides the dragged menu sprite after the mouse is released over the map.
		void handleMouseRelease(int x, int y, int spritePressed);

		/// Places a turret of the given type/color on the tile currently closest to the mouse.
		void placeTurret(int spriteToShow);

		/// Upgrades the turret currently placed on the given tile to its next level.
		/// @return The newly visible (upgraded) turret sprite, or nullptr if the tile has no turret.
		::Renderer::spriteSheetTexture* upgradeTurret(int tileNumb);

		/// Hides every turret sprite on the given tile (used when a tower is sold/destroyed).
		void removeTurret(int tileNumb);

		/// Returns the flattened grid index of the tile closest to the given position.
		int GetClosestCellNumber(int x, int y);

		/// Marks the tile at (row, col) as infected, applying a damage-over-time visual (e.g. for an area-of-effect tower).
		void setAOECell(int row, int col, int damageOverTime);

		/// Updates hover visuals on the 4 styled HUD buttons (cancel/upgrade/sell/start) against the live mouse position.
		void updateButtonHover(int mouseX, int mouseY);

		/// Spawns a new sprite for a Walker critter and returns it for the caller to position/track.
		spriteSheetTexture* SpawnWalker(Window& window);

		/// Spawns a new sprite for a Speedy critter and returns it for the caller to position/track.
		spriteSheetTexture* SpawnSpeed(Window& window);

		/// Spawns a new sprite for a Boss critter and returns it for the caller to position/track.
		spriteSheetTexture* SpawnBoss(Window& window);

		/// Spawns a new bullet sprite and returns it for the caller to position/track.
		spriteSheetTexture* SpawnBullet(Window& window);

		/// Returns the shared "cancel" icon texture (e.g. for dismissing the selected-tower UI).
		::Renderer::Texture* getCancelTexture() { return cancelTexture; }

		/// Returns the "upgrade" icon texture.
		::Renderer::StyledButton* getUpgradeTexture() { return upgradeTexture; }

		/// Returns the shared "sell" icon texture (sells the selected tower for a refund).
		::Renderer::StyledButton* getSellTexture() { return sellTexture; }

		/// Returns the "target" HUD button (cycles the selected tower's targeting strategy).
		::Renderer::StyledButton* getTargetTexture() { return targetTexture; }

		/// Returns the inspection panel background drawn while a direct tower is selected.
		::Renderer::Texture* getInspectDirectTexture() { return inspectDirectTexture; }

		/// Returns the inspection panel background drawn while a slow tower is selected.
		::Renderer::Texture* getInspectSlowTexture() { return inspectSlowTexture; }

		/// Returns the inspection panel background drawn while an AoE tower is selected.
		::Renderer::Texture* getInspectAoeTexture() { return inspectAoeTexture; }

		/// Returns the inspection panel background drawn while a Walker critter is selected.
		::Renderer::Texture* getInspectWalkerTexture() { return inspectWalkerTexture; }

		/// Returns the inspection panel background drawn while a Speedy critter is selected.
		::Renderer::Texture* getInspectSpeedyTexture() { return inspectSpeedyTexture; }

		/// Returns the inspection panel background drawn while a Boss critter is selected.
		::Renderer::Texture* getInspectBossTexture() { return inspectBossTexture; }

		/// Returns the bordered Walker sprite swapped in while a Walker critter is selected.
		::Renderer::spriteSheetTexture* getSelectedWalkerSprite() { return selectedWalkerSprite; }

		/// Returns the bordered Speedy sprite swapped in while a Speedy critter is selected.
		::Renderer::spriteSheetTexture* getSelectedSpeedySprite() { return selectedSpeedySprite; }

		/// Returns the bordered Boss sprite swapped in while a Boss critter is selected.
		::Renderer::spriteSheetTexture* getSelectedBossSprite() { return selectedBossSprite; }

		/// Returns the gold border marker drawn on the tile of the currently selected tower.
		::Renderer::Texture* getSelectionHighlightTexture() { return selectionHighlightTexture; }

		/// Returns the shared "Start" button texture used between waves.
		::Renderer::StyledButton* getStartTexture() { return startTexture; }

		/// Returns the underlying MapGenerator owning the grid tiles and menu sprites.
		MapGenerator* getGenerator() { return mapGenerator; }

		/// Deletes the owned MapGenerator.
		~GameRenderer();

	private:
		MapGenerator* mapGenerator = nullptr;///< Grid of tiles and side-menu sprites, owned by this GameRenderer.
		::Renderer::Texture* mainTexture = nullptr; ///< Shared sprite sheet used for menu icons, critters and bullets. Owned by Window.

		::Renderer::StyledButton* cancelTexture = nullptr; ///< "Cancel" HUD button. Owned by Window.
		::Renderer::StyledButton* upgradeTexture = nullptr; ///< "Upgrade" HUD button. Owned by Window.
		::Renderer::StyledButton* sellTexture = nullptr; ///< "Sell" HUD button. Owned by Window.
		::Renderer::StyledButton* targetTexture = nullptr; ///< "Target" HUD button, cycles the selected tower's targeting strategy. Owned by Window.
		::Renderer::InspectPanel* inspectDirectTexture = nullptr; ///< Inspection panel background for a selected direct tower. Owned by Window.
		::Renderer::InspectPanel* inspectSlowTexture = nullptr; ///< Inspection panel background for a selected slow tower. Owned by Window.
		::Renderer::InspectPanel* inspectAoeTexture = nullptr; ///< Inspection panel background for a selected AoE tower. Owned by Window.
		::Renderer::InspectPanel* inspectWalkerTexture = nullptr; ///< Inspection panel background for a selected Walker critter. Owned by Window.
		::Renderer::InspectPanel* inspectSpeedyTexture = nullptr; ///< Inspection panel background for a selected Speedy critter. Owned by Window.
		::Renderer::InspectPanel* inspectBossTexture = nullptr; ///< Inspection panel background for a selected Boss critter. Owned by Window.
		::Renderer::Texture* selectedCritterTexture = nullptr; ///< Sprite sheet holding the bordered (selected) variants of the critter sprites. Owned by Window.
		::Renderer::spriteSheetTexture* selectedWalkerSprite = nullptr; ///< Bordered Walker sprite; owned by selectedCritterTexture.
		::Renderer::spriteSheetTexture* selectedSpeedySprite = nullptr; ///< Bordered Speedy sprite; owned by selectedCritterTexture.
		::Renderer::spriteSheetTexture* selectedBossSprite = nullptr; ///< Bordered Boss sprite; owned by selectedCritterTexture.
		::Renderer::SelectionHighlight* selectionHighlightTexture = nullptr; ///< Gold border marker shown on the selected tower's tile. Owned by Window.
		::Renderer::StyledButton* startTexture = nullptr; ///< "Start" HUD button, shown between waves. Owned by Window.

		std::vector<::Renderer::spriteSheetTexture*> critterSprites; ///< Sprites spawned for active critters. Owned by mainTexture.
		std::vector<::Renderer::spriteSheetTexture*> bulletSprites; ///< Sprites spawned for active bullets. Owned by mainTexture.

		RendererMap* currClosestTile = nullptr; ///< Tile closest to the mouse as of the last MainLoop() call. Not owned here.
		int columns = 0; ///< Column count used for row-major cell indexing.
	};
}
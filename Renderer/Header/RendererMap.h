#pragma once
#include "Texture.h"

#define NUMB_ROWS 9
#define NUMB_COLUMNS 9

namespace Renderer
{
	/// Classifies a map tile as buildable scenery (towers can be placed) or a critter path (towers cannot be placed).
	enum TileType
	{
		scenery = 0, ///< Tile can have a turret placed on it.
		path = 1 ///< Tile is part of the critter path; no turret can be placed on it.
	};


	/**
	 * @struct TileClickRequest
	 * @brief Signals that the tile holding a placed turret was clicked.
	 *
	 * Created by RendererMap::checkSelectTile() when the mouse clicks a tile
	 * that already has a visible turret sprite; the owning code polls
	 * RendererMap::clickRequest to detect and consume it (e.g. to open an
	 * upgrade/sell UI for that tile).
	 */
	struct TileClickRequest
	{
		int tileNumber; ///< Cell number (see RendererMap::getCellNumber()) of the clicked tile.

		/// Records which tile (by cell number) was clicked.
		TileClickRequest(int t) : tileNumber(t){}
	};

	/**
	 * @class RendererMap
	 * @brief Visual representation of a single grid tile: its ground sprite, infection overlay, and turret sprites.
	 *
	 * Each RendererMap owns a set of pre-created turret sprites (3 colors x
	 * 3 levels) carved out of the shared map texture, all hidden until
	 * PlaceTurret() reveals one; UpgradeTurret() then swaps the visible one
	 * for the next level. infect()/Update() drive a temporary "infected"
	 * visual state (used for area-of-effect damage) that reverts
	 * automatically after maxInfectedTime seconds. checkSelectTile() detects
	 * mouse clicks on a tile that currently has a turret, recording the
	 * result in clickRequest for the owner to consume.
	 */
	class RendererMap
	{
	public:

		/**
		 * @brief Builds the tile's ground/infected sprites and its full set of hidden turret sprites.
		 * @param p_type Whether this tile is scenery (buildable) or part of the critter path.
		 * @param p_texture Sprite for this tile's normal ground appearance. Not owned (owned by p_main_texture).
		 * @param p_infectedtexture Sprite shown instead while the tile is infect()-ed. Not owned (owned by p_main_texture).
		 * @param p_main_texture Shared sprite-sheet texture used to carve out this tile's turret sprites.
		 * @param textureNumb Sprite-sheet x-offset used to pick the ground texture (see MapGenerator::GetTexture()).
		 * @param p_cellNumber Flattened grid index identifying this tile (row-major).
		 */
		RendererMap(TileType p_type, spriteSheetTexture* p_texture, spriteSheetTexture* p_infectedtexture, Texture* p_main_texture, int textureNumb, int p_cellNumber); //texure is generated in app

		/// Reveals the turret sprite for the given turret type/color, if this tile is scenery.
		void PlaceTurret(int turretType);

		/// Advances the currently visible turret to its next upgrade sprite, hiding the old one.
		/// @return The newly visible turret sprite, or nullptr if no turret is currently placed.
		::Renderer::spriteSheetTexture* UpgradeTurret();

		/// Returns this tile's on-screen destination rectangle (position and size).
		SDL_FRect& getTilePos() { return m_texture->m_pos_scale; }

		/// Returns this tile's ground sprite.
		spriteSheetTexture* getSpriteTexture() { return m_texture; }

		/// Returns all turret sprites (one per color/level combination) available to be placed on this tile.
		std::vector<spriteSheetTexture*>& getTurretTextures() { return m_turretTextures; }

		/// Returns this tile's flattened grid index.
		int getCellNumber() { return m_cellNumber; }

		/// Advances the infected-overlay timer, reverting to the normal ground sprite once maxInfectedTime has elapsed.
		void Update(float deltatime);

		/// Switches this tile to its infected visual (hides the ground sprite, shows the infected overlay) and records the damage amount.
		void infect(int damage);

		/// If (x, y) is inside this tile and it currently shows a turret, creates a TileClickRequest in clickRequest.
		void checkSelectTile(int x, int y);

		bool isInfected = false; ///< True while the infected overlay is being shown.
		int infectedDamage = 0; ///< Damage-over-time amount recorded by the most recent infect() call.
		TileClickRequest* clickRequest = nullptr; ///< Set by checkSelectTile() when this tile (with a turret) is clicked; owner is expected to consume/delete it.
	private:

		Texture* m_mainTexture = nullptr; ///< Shared sprite-sheet texture this tile's turret sprites were carved from. Not owned.
		int m_mainTextureNumb = 0; ///< Sprite-sheet x-offset used for this tile's ground texture.
		spriteSheetTexture* m_texture = nullptr; ///< Normal ground sprite for this tile. Not owned by this class.
		spriteSheetTexture* m_infectedtexture = nullptr; ///< Overlay sprite shown while isInfected is true. Not owned by this class.
		std::vector<spriteSheetTexture*> m_turretTextures; ///< All turret sprites (3 colors x 3 levels) available for this tile, owned by m_mainTexture.

		TileType m_type; ///< Whether this tile is buildable scenery or part of the critter path.

		int m_cellNumber = 0; ///< Flattened grid index identifying this tile.
		float currInfectedTime = 0; ///< Seconds elapsed since infect() was last called.
		float maxInfectedTime = 3; ///< Duration, in seconds, the infected overlay remains visible.

	};
}

#pragma once
#include "InputManager.h"
#include "GameRenderer.h"
#include "WaveManager.h"
#include "Map.h"
#include "Game.h"
#include "Tower.h"
#include "Bullet.h"
#include "Player.h"
#include "Button.h"
#include "TowerLoggingObserver.h"
#include "MapLoggingObserver.h"
#include "CritterLoggingObserver.h"
#include "CritterRenderSyncObserver.h"
#include "PlayerRenderObserver.h"
#include "TowerBillingObserver.h"
#include "CritterBillingObserver.h"
#include "MapRenderSyncObserver.h"
#include "TowerRenderSyncObserver.h"
#include <memory>

#define MAX_BULLET_POOL 20

namespace App
{
	/**
	 * @class GameManager
	 * @brief Owns and drives the whole gameplay session for one map.
	 *
	 * GameManager wires itself up to the InputManager's mouse callbacks,
	 * builds the Map and the renderer-side grid, pre-allocates pools of
	 * critters and bullets, and owns the Player, the WaveManager, and every
	 * Tower that gets bought. Each frame, MainLoop() drives the whole
	 * session: UI buttons, tile selection, critter/bullet/tower updates,
	 * and wave progression.
	 */
	/** @brief Terminal state of a game session: still playing, won, or lost. */
	enum class GameOutcome
	{
		None,
		Won,
		Lost
	};

	class GameManager
	{
	public:
		/**
		 * @brief Builds the game world and wires up input callbacks.
		 *
		 * Registers mouse press/move/release handlers with the input
		 * manager, generates the Map, hands its cell layout to the
		 * renderer, pre-spawns the critter and bullet pools, creates the
		 * WaveManager (seeded with the critter pool), and spawns the
		 * upgrade/cancel UI buttons.
		 * @param window Window providing the live mouse state and used to
		 *        create renderer-owned textures.
		 * @param p_inputManager Input manager whose click/move/release
		 *        events this GameManager subscribes to.
		 */
		GameManager(::Renderer::Window& window, InputManager* p_inputManager);

		/**
		 * @brief Starts a game on a caller-selected map.
		 * @param selectedMap An editable map that has been configured and validated.
		 * @throws std::invalid_argument If the map is null or invalid.
		 */
		GameManager(
			::Renderer::Window& window,
			InputManager* p_inputManager,
			std::unique_ptr<Game::Map> selectedMap);

		/**
		 * @brief Handles a mouse-button-down event.
		 *
		 * Finds the closest tower-shop sprite to the cursor (if any) and,
		 * when close enough, starts "holding" that tower type for
		 * placement. Also forwards the press to the clicked map tile (for
		 * tower or critter selection) and to the upgrade/cancel buttons.
		 */
		void HandleMousePress();

		/// Forwards the current mouse position to the renderer so hover feedback can be drawn.
		void HandleMouseMove();

		/**
		 * @brief Handles a mouse-button-up event, completing a tower placement.
		 *
		 * If no tower is currently being held, does nothing. Otherwise,
		 * finds the map cell closest to the cursor and, if the player can
		 * afford the held tower type and the cell allows a tower, builds
		 * the corresponding Tower there, deducts its cost from the
		 * player's coins, and asks the renderer to display the new turret.
		 */
		void HandleMouseRelease();

		/**
		 * @brief Checks whether the player can afford the tower type currently being held.
		 * @return True if the player's coins cover the level-0 cost of the
		 *         tower type identified by currSpritePressed.
		 */
		bool canPlaceTurret();

		/**
		 * @brief Picks the inspection-panel corner that best avoids the board's key rectangles.
		 * @param subjectRect The selected element's rectangle; the panel must not cover it.
		 * @param exitRect The exit tile; the panel should preferably not cover it either.
		 * @param corners Four candidate corners: top-right, bottom-right, top-left, bottom-left.
		 * @return First corner covering neither; failing that, the first covering only the
		 *         exit; failing that, the first corner.
		 */
		static SDL_FRect choosePanelCorner(const SDL_FRect& subjectRect, const SDL_FRect& exitRect, const SDL_FRect corners[4]);

		/**
		 * @brief True once the player has clicked anywhere after a win/loss.
		 *
		 * Set by HandleMousePress() the moment a click lands while outcome
		 * is non-None; Application polls this each frame and, once true,
		 * tears this GameManager down and builds a fresh one to restart the
		 * session.
		 */
		bool ShouldRestart() const { return shouldRestart; }

		/// Pre-allocates the pool of walker critters, binds them to the map's generated path, and marks them inactive until spawned.
		void SpawnWalker(::Renderer::Window& window);

		/// Pre-allocates the pool of speedy critters, binds them to the map's generated path, and marks them inactive until spawned.
		void SpawnSpeed(::Renderer::Window& window);

		/// Pre-allocates the pool of boss critters, binds them to the map's generated path, and marks them inactive until spawned.
		void SpawnBoss(::Renderer::Window& window);

		/// Pre-allocates the pool of reusable bullets shared by every tower.
		void SpawnBullets(::Renderer::Window& window);

		/// Creates and positions the upgrade and cancel UI buttons, initially inactive.
		void SpawnButtons();

		/// Click-handling hook reserved for future use; not currently wired to any input event.
		void OnClick();

		/// Mouse-move hook reserved for future use; not currently wired to any input event.
		void OnMouseMove();

		/**
		 * @brief Advances the whole game session by one frame.
		 *
		 * Updates the renderer, resolves pending upgrade/cancel button
		 * presses on the selected tower or critter, detects map tiles that
		 * were clicked (clicking a critter selects it for live inspection;
		 * otherwise clicking a tower's cell selects the tower and shows the
		 * upgrade/cancel buttons), updates every critter (rewarding or
		 * penalizing the player on kill/leak), updates every bullet (applying
		 * AOE hits as they land), lets every tower operate and fire (pulling
		 * a free bullet from the pool when a tower requests one), and finally
		 * updates the WaveManager.
		 *
		 * Once the session has a GameOutcome other than None (the player
		 * has won or lost), every step above is skipped - the board stays
		 * frozen as a backdrop behind the win/loss overlay - except the
		 * initial renderer update, so hover feedback keeps working.
		 * @param deltaTime Time elapsed since the previous frame, in seconds.
		 */
		void MainLoop(float deltaTime);

		/// Bullet-spawning hook reserved for future use; not currently wired to any caller (see SpawnBullets() for the actual pool setup).
		void SpawnBullet();

		/**
		 * @brief Finds a bullet in the pool that isn't currently in flight.
		 * @return Pointer to the first inactive bullet, or nullptr if every
		 *         bullet in the pool is currently active.
		 */
		Game::Bullet* findValidBullet();

		/// Destroys every owned tower, critter, bullet, UI button, the wave manager, and the map.
		~GameManager();

		/// Shared queue of deferred actions requested elsewhere in the game, reserved for future use.
		static std::vector<std::function<void()>> requestedActions;

	private:
		// Groups the map's logging and renderer-sync observers so they detach
		// automatically when the map's observer set is destroyed.
		struct MapObserverSet
		{
			Game::MapLoggingObserver logging;
			App::MapRenderSyncObserver rendering;

			MapObserverSet(Game::Map& map, ::Renderer::GameRenderer& gameRenderer)
				: logging(map), rendering(map, gameRenderer) {}
		};

		// Groups one tower's logging, billing, and renderer-sync observers.
		struct TowerObserverSet
		{
			// Used to find this tower's render observer during upgrades and
			// to destroy the observers before the tower is deleted.
			Game::Tower* subject;
			Game::TowerLoggingObserver logging;
			App::TowerBillingObserver billing;
			App::TowerRenderSyncObserver rendering;

			TowerObserverSet(
				Game::Tower& tower,
				Player& player,
				::Renderer::GameRenderer& gameRenderer,
				int mapColumns)
				: subject(&tower),
				  logging(tower),
				  billing(tower, player),
				  rendering(tower, gameRenderer, mapColumns) {}
		};

		// Groups one critter's logging and player-billing observers.
		struct CritterObserverSet
		{
			Game::CritterLoggingObserver logging;
			App::CritterBillingObserver billing;
			CritterRenderSyncObserver render;

			CritterObserverSet(Game::Critter& critter, Player& player)
				: logging(critter), billing(critter, player), render(critter) {}
		};

		// Prices the sell button's refund digits for the given tower at
		// the current wave; called on selection and every frame a tower
		// stays selected so the preview tracks wave changes.
		void priceSellLabel(Game::Tower* tower);

		// Shows the inspection panel for the given tower: the panel image
		// matching its type plus seven-segment labels for level, range,
		// power, rate of fire and the next upgrade cost. The panel parks
		// in a corner of the play area, picking one that covers neither
		// the tower's own tile nor the exit tile when that is possible.
		void showTowerInspection(Game::Tower* tower, const SDL_FRect& towerTileRect);

		// Hides the inspection panel and all of its value labels.
		void hideTowerInspection();

		// Shows the tower's current targeting strategy name on the target
		// button and on the inspection panel's last line.
		void refreshTargetLabels(Game::Tower* tower);

		// Fills the four inspection-panel corner candidates (top-right,
		// bottom-right, top-left, bottom-left) for the current play area
		// and returns the exit tile's rectangle.
		SDL_FRect panelCorners(SDL_FRect corners[4]);

		// Dismisses the selected-tower UI (buttons, price labels, panel,
		// tile highlight) and clears the selection; the tower itself stays
		// on the map.
		void clearTowerSelection();

		// Returns the active critter whose tile-sized sprite covers the
		// given screen point, or nullptr when no critter is there.
		Game::Critter* findCritterAt(int x, int y);

		// Positions the critter inspection panel (type-matched image plus
		// seven-segment labels for level, hp, max hp, speed, reward and
		// steal) in a corner clear of the critter and, when possible, of
		// the exit; also moves the selection outline onto the critter.
		// Called on selection and every frame it stays selected, so the
		// outline follows the critter, the stats stay live, and the panel
		// hops corners when the critter walks into its corner.
		void updateCritterInspection(Game::Critter* critter);

		// Hides the critter panel, labels and outline, and clears the
		// critter selection. The shared cancel button and outline stay
		// visible when a tower is selected.
		void clearCritterSelection();

		/// Swaps the critter's sprite for its bordered variant so the selection marker follows it automatically.
		void showCritterSelectedSprite(Game::Critter* critter);

		/// Puts the critter's original sprite back after inspection ends.
		void restoreCritterSprite(Game::Critter* critter);

		::Renderer::GameRenderer gameRenderer; ///< Owns the renderer-side sprites/tiles mirroring this GameManager's game state.
		InputManager* inputManager = nullptr;///< Input manager this GameManager subscribes its mouse handlers to.
		WaveManager* waveManager = nullptr;///< Drives critter wave spawning over time; seeded with the critter pool.

		bool b_clicked = false; ///< Unused click-state flag.
		::Renderer::Mouse* mouse; ///< Pointer to the window's live mouse state.
		int currSpritePressed = 0; ///< Index of the tower-shop sprite currently selected/held for placement.
		bool isHoldingTurret = false; ///< True while a tower is being held after a shop click, awaiting placement on release.
		std::unique_ptr<Game::Map> map; ///< The selected grid/path that gameplay takes place on.

		Game::Tower* currentTowerSelected = nullptr; ///< Tower currently selected on the map (for upgrade/cancel), or nullptr if none.
		Game::Critter* currentCritterSelected = nullptr; ///< Critter currently selected for inspection; its panel follows it every frame. Selection is mutually exclusive with the tower selection.
		::Renderer::spriteSheetTexture* critterOriginalTexture = nullptr; ///< Sprite the selected critter wore before the bordered variant was swapped in; restored on deselect.

		std::vector<Game::Tower*> towers; ///< Every tower currently placed on the map, owned by this GameManager.
		std::vector<Game::Critter*> critters; ///< Pre-allocated pool of every critter (walker, speedy, boss), owned by this GameManager.
		std::vector<Game::Bullet*> bullets; ///< Pre-allocated pool of reusable bullets, owned by this GameManager.
		Player player; ///< Tracks the human player's coin balance.

		::Renderer::Button* upgradeButton = nullptr; ///< UI button shown while a tower is selected, to upgrade it.
		::Renderer::Button* cancelButton = nullptr; ///< UI button shown while a tower is selected, to dismiss the selection without selling.
		::Renderer::Button* sellButton = nullptr; ///< UI button shown while a tower is selected, to sell it for a wave-based refund.
		::Renderer::Button* targetButton = nullptr; ///< UI button shown while a tower is selected; each press switches the tower to its next targeting strategy.
		::Renderer::TextLabel* targetNameLabel = nullptr; ///< Name of the selected tower's targeting strategy, shown on the target button. Owned by the Window.
		::Renderer::TextLabel* inspectTargetLabel = nullptr; ///< "TARGET: <strategy>" line at the bottom of the tower inspection panel. Owned by the Window.
		::Renderer::Button* startButton = nullptr; ///< UI button shown between waves; pressing it starts the next wave.
		::Renderer::PriceLabel* startWaveLabel = nullptr; ///< Wave number shown on the start button ("Start n"). Owned by the Window.
		::Renderer::PriceLabel* upgradePriceLabel = nullptr; ///< Upgrade cost shown on the upgrade button while a tower is selected. Owned by the Window.
		::Renderer::TextLabel* maxUpgradeTexture = nullptr; ///< "MAX" caption drawn instead of the price label when the selected tower is maxed out. Owned by the Window.
		::Renderer::PriceLabel* sellPriceLabel = nullptr; ///< Refund amount shown on the sell button while a tower is selected. Owned by the Window.
		::Renderer::NumberLabel* inspectLevelLabel = nullptr; ///< Current level shown on the tower inspection panel. Owned by the Window.
		::Renderer::NumberLabel* inspectRangeLabel = nullptr; ///< Current range shown on the tower inspection panel. Owned by the Window.
		::Renderer::NumberLabel* inspectPowerLabel = nullptr; ///< Current damage per shot shown on the tower inspection panel. Owned by the Window.
		::Renderer::NumberLabel* inspectRateWholeLabel = nullptr; ///< Whole part of the current rate of fire, left of the panel's fixed decimal point. Owned by the Window.
		::Renderer::NumberLabel* inspectRateFractionLabel = nullptr; ///< One-decimal part of the current rate of fire, right of the panel's decimal point. Owned by the Window.
		::Renderer::NumberLabel* inspectCritterLevelLabel = nullptr; ///< Difficulty level shown on the critter inspection panel. Owned by the Window.
		::Renderer::NumberLabel* inspectCritterHpLabel = nullptr; ///< Current hit points shown on the critter inspection panel, refreshed every frame. Owned by the Window.
		::Renderer::NumberLabel* inspectCritterMaxHpLabel = nullptr; ///< Maximum hit points shown on the critter inspection panel. Owned by the Window.
		::Renderer::NumberLabel* inspectCritterSpeedLabel = nullptr; ///< Current speed shown on the critter inspection panel (reflects active slows). Owned by the Window.
		::Renderer::NumberLabel* inspectCritterRewardLabel = nullptr; ///< Coins awarded on kill, shown on the critter inspection panel. Owned by the Window.
		::Renderer::NumberLabel* inspectCritterStrengthLabel = nullptr; ///< Coins stolen per tick at the exit, shown on the critter inspection panel. Owned by the Window.

		GameOutcome outcome = GameOutcome::None; ///< Set once the player has won or lost; freezes MainLoop() once non-None.
		bool shouldRestart = false; ///< Set by HandleMousePress() on a click after outcome is non-None; see ShouldRestart().
		::Renderer::OutcomeOverlay* gameOverTexture = nullptr; ///< Full-screen "Game Over" overlay, shown once outcome becomes Lost. Owned by the Window.
		::Renderer::OutcomeOverlay* youWinTexture = nullptr; ///< Full-screen "You Win" overlay, shown once outcome becomes Won. Owned by the Window.
		::Renderer::NumberLabel* waveLabel = nullptr; ///< Shows the current wave number, stacked above the coin/health counters. Owned by the Window.
		::Renderer::TextLabel* waveLabelText = nullptr; ///< "WAVE" caption, drawn immediately left of waveLabel. Owned by the Window.
		::Renderer::PriceLabel* inspectUpgradeLabel = nullptr; ///< Cost of the next upgrade shown on the panel; MAX when the tower is maxed out. Owned by the Window.
		::Renderer::TextLabel* inspectMaxTexture = nullptr; ///< "MAX" caption drawn instead of the upgrade cost row when the inspected tower is maxed out. Owned by the Window.

		// Each set owns observers attached to its corresponding subject.
		// They are explicitly cleared before their subjects are destroyed.
		std::unique_ptr<MapObserverSet> mapObserverSet;
		std::vector<std::unique_ptr<TowerObserverSet>> towerObserverSets;
		std::vector<std::unique_ptr<CritterObserverSet>> critterObserverSets;

		// Attached to player; declared after it so it is destroyed (and
		// detaches) before the Player it observes.
		std::unique_ptr<PlayerRenderObserver> playerObserver;
	};
}
#include "GameManager.h"
#include "UiTheme.h"
#include "PixelFont.h"
#include <cmath>
#include <stdexcept>

#include "DirectDammageTower.h"
#include "AoeTower.h"
#include "SlowTower.h"
#include <chrono>
#include "WalkerCritter.h"
#include "SpeedyCritter.h"
#include "BossCritter.h"
#include <iostream>

using namespace Game;

namespace
{
	// Geometry of the HUD button row sitting under the map. The buttons are
	// positioned in SpawnButtons(); the value labels drawn on them (upgrade
	// price, refund, upcoming wave) are placed against the same rectangles.
	constexpr int kHudButtonW = WINDOW_WIDTH / 8;
	constexpr int kHudButtonH = 60;
	constexpr int kHudButtonY = WINDOW_HEIGHT - 70;
	constexpr int kTargetButtonX = WINDOW_WIDTH / 2 - 132;
	constexpr int kUpgradeButtonX = WINDOW_WIDTH / 2 - 50;
	constexpr int kSellButtonX = WINDOW_WIDTH / 2 + 32;
	constexpr int kCancelButtonX = WINDOW_WIDTH / 2 + 140;
	constexpr int kStartButtonX = WINDOW_WIDTH / 2 + 230;

	// Centers a value under a button's label, on the line StyledButton keeps
	// free for it, so label and value read as one centered block. Takes any
	// label type with getWidth()/getHeight()/setPosition() - PriceLabel for
	// a price/refund/wave number, or TextLabel for the "MAX" caption shown
	// on an upgrade button in place of a price.
	template <typename Label>
	void placeOnCompanionLine(Label* label, int buttonX)
	{
		label->setPosition(
			buttonX + (kHudButtonW - label->getWidth()) / 2,
			kHudButtonY + ::Renderer::StyledButton::companionLineOffsetY(kHudButtonH, label->getHeight()));
	}
}

App::GameManager::GameManager(::Renderer::Window &window, InputManager *p_inputManager)
	: GameManager(
		  window,
		  p_inputManager,
		  std::make_unique<Map>(NUMB_ROWS, NUMB_COLUMNS))
{
}

App::GameManager::GameManager(
	::Renderer::Window &window,
	InputManager *p_inputManager,
	std::unique_ptr<Game::Map> selectedMap)
	: inputManager(p_inputManager)
{
	if (!selectedMap)
	{
		throw std::invalid_argument("A map must be selected before starting a game");
	}
	if (!selectedMap->isValid())
	{
		throw std::invalid_argument("The selected map is invalid");
	}
	map = std::move(selectedMap);

	mouse = &window.m_mouse;

	std::function<void()> func = [this]()
	{
		HandleMousePress();
	};

	inputManager->AttachClickEvent(func);

	std::function<void()> func2 = [this]()
	{
		HandleMouseMove();
	};

	inputManager->AttachMoveEvent(func2);

	std::function<void()> func3 = [this]()
	{
		HandleMouseRelease();
	};

	inputManager->AttachReleaseEvent(func3);

	mapObserverSet = std::make_unique<MapObserverSet>(*map, gameRenderer);

	std::vector<int> correctPath = map->getCorrectPath();

	std::vector<int> cellValues;

	for (auto cells : map->getCells())
	{
		cellValues.push_back(static_cast<int>(cells->type));
	}

	// Prices shown in the side menu; index-aligned with the menu icons
	// (direct, aoe, slow) and matching what HandleMouseRelease() charges.
	std::vector<int> towerPrices = {
		DirectDammageTower::LEVEL_CONFIGS[0].levelCost,
		AoeTower::LEVEL_CONFIGS[0].levelCost,
		SlowTower::LEVEL_CONFIGS[0].levelCost
	};

	gameRenderer.CreateGame(window, map->getRows(), map->getCols(), cellValues, towerPrices);

	SpawnWalker(window);
	SpawnSpeed(window);
	SpawnBoss(window);

	SpawnBullets(window);

	waveManager = new WaveManager(critters);

	SpawnButtons();

	// Upgrade price label, hidden until a tower is selected;
	// positioned relative to the upgrade button (see SpawnButtons).
	upgradePriceLabel = window.addPriceLabel(0, 0, 0);
	upgradePriceLabel->setRender(false);

	// "MAX" caption replacing the price label when the selected tower is
	// already at max level.
	maxUpgradeTexture = window.addTextLabel(0, 0, "MAX", 2, ::Renderer::UiTheme::AccentGold);
	maxUpgradeTexture->setRender(false);

	// Refund amount shown on the sell button while a tower is selected;
	// the value is refreshed on every selection (see the click handling
	// in MainLoop).
	sellPriceLabel = window.addPriceLabel(0, 0, 0);
	sellPriceLabel->setRender(false);

	// Inspection panel value labels, hidden until a tower is selected
	// (positioned against the panel image in showTowerInspection).
	inspectLevelLabel = window.addNumberLabel(0, 0, 0);
	inspectRangeLabel = window.addNumberLabel(0, 0, 0);
	inspectPowerLabel = window.addNumberLabel(0, 0, 0);
	inspectRateWholeLabel = window.addNumberLabel(0, 0, 0);
	inspectRateFractionLabel = window.addNumberLabel(0, 0, 0);
	inspectUpgradeLabel = window.addPriceLabel(0, 0, 0);
	inspectLevelLabel->setRender(false);
	inspectRangeLabel->setRender(false);
	inspectPowerLabel->setRender(false);
	inspectRateWholeLabel->setRender(false);
	inspectRateFractionLabel->setRender(false);
	inspectUpgradeLabel->setRender(false);

	// Critter inspection value labels, hidden until a critter is selected
	// (positioned against the panel image in updateCritterInspection).
	inspectCritterLevelLabel = window.addNumberLabel(0, 0, 0);
	inspectCritterHpLabel = window.addNumberLabel(0, 0, 0);
	inspectCritterMaxHpLabel = window.addNumberLabel(0, 0, 0);
	inspectCritterSpeedLabel = window.addNumberLabel(0, 0, 0);
	inspectCritterRewardLabel = window.addNumberLabel(0, 0, 0);
	inspectCritterStrengthLabel = window.addNumberLabel(0, 0, 0);
	inspectCritterLevelLabel->setRender(false);
	inspectCritterHpLabel->setRender(false);
	inspectCritterMaxHpLabel->setRender(false);
	inspectCritterSpeedLabel->setRender(false);
	inspectCritterRewardLabel->setRender(false);
	inspectCritterStrengthLabel->setRender(false);
	// "MAX" caption replacing the panel's upgrade cost row for a maxed tower;
	// scale 2 to match the NumberLabel/PriceLabel digits beside it.
	inspectMaxTexture = window.addTextLabel(0, 0, "MAX", 2, ::Renderer::UiTheme::AccentGold);
	inspectMaxTexture->setRender(false);

	// Current targeting strategy: on the target button's companion line, and
	// as a last line on the inspection panel (positioned in showTowerInspection).
	targetNameLabel = window.addTextLabel(0, 0, "", 1, ::Renderer::UiTheme::AccentGold);
	targetNameLabel->setRender(false);
	inspectTargetLabel = window.addTextLabel(0, 0, "", 1, ::Renderer::UiTheme::TextMuted);
	inspectTargetLabel->setRender(false);

	// Wave number shown on the start button between waves; positioned
	// relative to the button each time it is shown (see MainLoop).
	startWaveLabel = window.addPriceLabel(0, 0, 1);
	startWaveLabel->setRender(false);

	// Each of these three buttons shows a value under its label, so the label
	// gives up the full-height centering and shares a centered two-line block
	// with it. (The upgrade button drops back to a centered "UPGRADE" alone
	// when the selected tower is maxed out and has no price to show.)
	gameRenderer.getUpgradeTexture()->setCompanionLineHeight(upgradePriceLabel->getHeight());
	gameRenderer.getSellTexture()->setCompanionLineHeight(sellPriceLabel->getHeight());
	gameRenderer.getTargetTexture()->setCompanionLineHeight(targetNameLabel->getHeight());
	gameRenderer.getStartTexture()->setCompanionLineHeight(startWaveLabel->getHeight());

	// Coin counter in the window's bottom-left corner; attaching it to the
	// player makes every ChangeCoins() redraw the label through the
	// observer pattern.
	playerObserver = std::make_unique<PlayerRenderObserver>(player, window);

	// Wave counter, flush in the bottom-left corner below the health
	// counter (y=424, see PlayerRenderObserver). "WAVE" is a PixelFont
	// caption placed left of the number.
	waveLabel = window.addNumberLabel(0, 0, waveManager->GetCurrentWave(), Math::Vec3<int>(255, 255, 255));
	const int waveY = WINDOW_HEIGHT - 8 - waveLabel->getHeight();
	waveLabelText = window.addTextLabel(0, 0, "WAVE", 1, ::Renderer::UiTheme::TextMuted);
	waveLabelText->setPosition(8, waveY + (waveLabel->getHeight() - waveLabelText->getHeight()) / 2);
	waveLabel->setPosition(8 + waveLabelText->getWidth() + 4, waveY);

	// Win/loss overlays, added last so they draw on top of everything else
	// (Window draws its render targets in registration order). Hidden until
	// MainLoop() sets a terminal GameOutcome.
	gameOverTexture = window.addOutcomeOverlay(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, false);
	youWinTexture = window.addOutcomeOverlay(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, true);
	gameOverTexture->setRender(false);
	youWinTexture->setRender(false);
}

void App::GameManager::HandleMousePress()
{
	if (outcome != GameOutcome::None)
	{
		// Game over: any click anywhere restarts instead of acting on the
		// (now frozen) board underneath the overlay.
		shouldRestart = true;
		return;
	}

	int minDiffNumb = -1;
	float closestDistanceSquared = std::numeric_limits<float>::max();
	// Squared-distance threshold (20px) a click must be within to count as hitting a shop sprite.
	float minDistanceSquared = 400;

	// Find the shop sprite whose center is closest to the click, within the threshold above.
	for (int i = 0; i < gameRenderer.GetAmountMenuSprites(); ++i)
	{
		SDL_FRect rect = gameRenderer.getSpriteRect(i);

		float centerX = rect.x + rect.w * 0.5f;
		float centerY = rect.y + rect.h * 0.5f;

		float dx = mouse->getX() - centerX;
		float dy = mouse->getY() - centerY;

		float distanceSquared = dx * dx + dy * dy;

		if (distanceSquared < closestDistanceSquared && distanceSquared < minDistanceSquared)
		{
			closestDistanceSquared = distanceSquared;
			minDiffNumb = i;
		}
	}

	if (minDiffNumb != -1)
	{
		isHoldingTurret = true;
		currSpritePressed = minDiffNumb;
		gameRenderer.setSpriteSheetRender(currSpritePressed, true);
	}

	int closestCellNumber = gameRenderer.GetClosestCellNumber(mouse->getX(), mouse->getY());
	gameRenderer.getGenerator()->getTiles()[closestCellNumber]->checkSelectTile(mouse->getX(), mouse->getY());

	// A critter under the cursor wins the click: dismiss any selected-tower
	// UI and inspect the critter instead. Its panel is refreshed every frame
	// in MainLoop, so it follows the critter as it walks. Critters walk
	// path tiles, which never raise a tile clickRequest (those fire only for
	// tiles holding a turret), so the hit-test has to happen here.
	Game::Critter *clickedCritter = findCritterAt(mouse->getX(), mouse->getY());
	if (clickedCritter != nullptr)
	{
		clearTowerSelection();
		clearCritterSelection();
		currentCritterSelected = clickedCritter;
		showCritterSelectedSprite(clickedCritter);
		cancelButton->setButtonActive(true);
		updateCritterInspection(clickedCritter);
	}

	upgradeButton->handlePress();
	sellButton->handlePress();
	targetButton->handlePress();
	cancelButton->handlePress();
	startButton->handlePress();
}

void App::GameManager::HandleMouseMove()
{
	gameRenderer.handleMouseMove(mouse->getX(), mouse->getY());
	gameRenderer.updateButtonHover(mouse->getX(), mouse->getY());
}

void App::GameManager::HandleMouseRelease()
{
	if (!isHoldingTurret)
	{
		return;
	}

	isHoldingTurret = false;

	int closestCellNumber = gameRenderer.GetClosestCellNumber(mouse->getX(), mouse->getY());

	std::vector<Cell *> cells = map->getCells();

	gameRenderer.handleMouseRelease(mouse->getX(), mouse->getY(), currSpritePressed);

	if (!canPlaceTurret())
		return;

	const SDL_FRect& targetRect = gameRenderer.getGenerator()->getTiles()[closestCellNumber]->getTilePos();
	if (mouse->getX() < targetRect.x || mouse->getX() >= targetRect.x + targetRect.w ||
		mouse->getY() < targetRect.y || mouse->getY() >= targetRect.y + targetRect.h)
	{
		return;
	}

	if (cells[closestCellNumber]->canPlaceTower())
	{
		int row = closestCellNumber / map->getCols();
		int col = closestCellNumber % map->getCols();

		// currSpritePressed selects which tower type was held from the shop;
		// closestCellNumber is converted from a flat grid index back to row/col.
		Tower *tower = nullptr;
		switch (currSpritePressed)
		{
		case 0:
			tower = new DirectDammageTower(row, col, gameRenderer.getClosestTile()->getTurretTextures()[0]);
			player.ChangeCoins(-DirectDammageTower::LEVEL_CONFIGS[0].levelCost);
			break;
		case 1:
			tower = new AoeTower(row, col, gameRenderer.getClosestTile()->getTurretTextures()[3]);
			player.ChangeCoins(-AoeTower::LEVEL_CONFIGS[0].levelCost);
			break;
		case 2:
			tower = new SlowTower(row, col, gameRenderer.getClosestTile()->getTurretTextures()[6]);
			player.ChangeCoins(-SlowTower::LEVEL_CONFIGS[0].levelCost);
			break;
		default:
			break;
		}

		if (tower != nullptr)
		{
			towerObserverSets.push_back(
				std::make_unique<TowerObserverSet>(*tower, player, gameRenderer, map->getCols()));

			// map->placeTower() notifies the map render observer, which shows
			// the turret sprite on this cell - no manual placeTurret() call
			// needed here anymore.
			map->placeTower(tower, row, col);
			towers.emplace_back(tower);
		}
	}
}

bool App::GameManager::canPlaceTurret()
{
	switch (currSpritePressed)
	{
	case 0:
		return player.GetCoins() >= DirectDammageTower::LEVEL_CONFIGS[0].levelCost;
	case 1:
		return player.GetCoins() >= AoeTower::LEVEL_CONFIGS[0].levelCost;
	case 2:
		return player.GetCoins() >= SlowTower::LEVEL_CONFIGS[0].levelCost;
	default:
		throw std::out_of_range("Invalid sprite index");
	}
}

void App::GameManager::SpawnWalker(::Renderer::Window &window)
{
	for (int i = 0; i < MAX_WALKER_POOL; ++i)
	{
		auto spritetexture = gameRenderer.SpawnWalker(window);
		WalkerCritter *critter = new WalkerCritter(1, 0, 0, spritetexture);
		critter->setPath(gameRenderer.getGenerator()->getTiles(), map->getCorrectPath(), map->getCols());
		critterObserverSets.push_back(std::make_unique<CritterObserverSet>(*critter, player));
		critter->setIsActive(false);
		critters.emplace_back(critter);
	}
}

void App::GameManager::SpawnSpeed(::Renderer::Window &window)
{
	for (int i = 0; i < MAX_SPEED_POOL; ++i)
	{
		auto spritetexture = gameRenderer.SpawnSpeed(window);
		SpeedyCritter *critter = new SpeedyCritter(1, 0, 0, spritetexture);
		critter->setPath(gameRenderer.getGenerator()->getTiles(), map->getCorrectPath(), map->getCols());
		critterObserverSets.push_back(std::make_unique<CritterObserverSet>(*critter, player));
		critter->setIsActive(false);
		critters.emplace_back(critter);
	}
}

void App::GameManager::SpawnBoss(::Renderer::Window &window)
{
	for (int i = 0; i < MAX_BOSS_POOL; ++i)
	{
		auto spritetexture = gameRenderer.SpawnBoss(window);
		BossCritter *critter = new BossCritter(1, 0, 0, spritetexture);
		critter->setPath(gameRenderer.getGenerator()->getTiles(), map->getCorrectPath(), map->getCols());
		critterObserverSets.push_back(std::make_unique<CritterObserverSet>(*critter, player));
		critter->setIsActive(false);
		critters.emplace_back(critter);
	}
}

void App::GameManager::SpawnBullets(::Renderer::Window &window)
{
	for (int i = 0; i < MAX_BULLET_POOL; ++i)
	{
		auto spritetexture = gameRenderer.SpawnBullet(window);
		Bullet *bullet = new Bullet(spritetexture);
		bullets.emplace_back(bullet);
	}
}

void App::GameManager::SpawnButtons()
{
	upgradeButton = new ::Renderer::Button(gameRenderer.getUpgradeTexture());
	cancelButton = new ::Renderer::Button(gameRenderer.getCancelTexture());
	sellButton = new ::Renderer::Button(gameRenderer.getSellTexture());
	targetButton = new ::Renderer::Button(gameRenderer.getTargetTexture());
	startButton = new ::Renderer::Button(gameRenderer.getStartTexture());

	// Target, upgrade, sell and cancel sit in a row below the map while a tower
	// is selected; sell goes in the middle so selling is never mistaken
	// for the cancel at the far right. The start button waits at the far
	// right end of the row, clear of the selection buttons, and only
	// shows between waves.
	upgradeButton->setPosition(kUpgradeButtonX, kHudButtonY);
	sellButton->setPosition(kSellButtonX, kHudButtonY);
	targetButton->setPosition(kTargetButtonX, kHudButtonY);
	cancelButton->setPosition(kCancelButtonX, kHudButtonY);
	startButton->setPosition(kStartButtonX, kHudButtonY);

	upgradeButton->setScale(kHudButtonW, kHudButtonH);
	sellButton->setScale(kHudButtonW, kHudButtonH);
	targetButton->setScale(kHudButtonW, kHudButtonH);
	cancelButton->setScale(kHudButtonW, kHudButtonH);
	startButton->setScale(kHudButtonW, kHudButtonH);

	upgradeButton->setButtonActive(false);
	sellButton->setButtonActive(false);
	targetButton->setButtonActive(false);
	cancelButton->setButtonActive(false);
	startButton->setButtonActive(false);
}

namespace
{
	// True when the two rectangles share at least one pixel.
	bool rectsOverlap(const SDL_FRect &a, const SDL_FRect &b)
	{
		return a.x < b.x + b.w && b.x < a.x + a.w &&
			a.y < b.y + b.h && b.y < a.y + a.h;
	}

	// Returns up to cellCount (row, col) path cells centered on the impact
	// tile, growing outward along the critters' walking path instead of a
	// 2D radius - so e.g. a 1-cell blast is just the impact tile, not its
	// orthogonal neighbors too.
	std::vector<std::pair<int, int>> BlastFootprint(const std::vector<int> &path, int columns, int hitRow, int hitCol, int cellCount)
	{
		int hitIndex = -1;
		for (size_t i = 0; i < path.size(); ++i)
		{
			if (path[i] / columns == hitRow && path[i] % columns == hitCol)
			{
				hitIndex = static_cast<int>(i);
				break;
			}
		}

		std::vector<std::pair<int, int>> cells;
		cells.emplace_back(hitRow, hitCol);

		if (hitIndex == -1)
			return cells;

		int forward = hitIndex + 1;
		int backward = hitIndex - 1;
		while (static_cast<int>(cells.size()) < cellCount && (forward < static_cast<int>(path.size()) || backward >= 0))
		{
			if (forward < static_cast<int>(path.size()))
			{
				cells.emplace_back(path[forward] / columns, path[forward] % columns);
				++forward;
				if (static_cast<int>(cells.size()) == cellCount)
					break;
			}
			if (backward >= 0)
			{
				cells.emplace_back(path[backward] / columns, path[backward] % columns);
				--backward;
			}
		}

		return cells;
	}
}

void App::GameManager::priceSellLabel(Game::Tower *tower)
{
	// The refund depends on the wave, so it is recomputed while the
	// tower stays selected, not just when it is picked.
	sellPriceLabel->setValue(tower->getRefundValue(waveManager->GetCurrentWave()));
	placeOnCompanionLine(sellPriceLabel, kSellButtonX);
}

SDL_FRect App::GameManager::panelCorners(SDL_FRect corners[4])
{
	const auto &tiles = gameRenderer.getGenerator()->getTiles();

	const int menuOffset = WINDOW_WIDTH / 8;
	int minX = menuOffset;
	int maxX = WINDOW_WIDTH;
	int minY = 0;
	int maxY = WINDOW_HEIGHT - menuOffset;
	for (auto tile : tiles)
	{
		const SDL_FRect &r = tile->getTilePos();
		if (r.x < minX) minX = (int)r.x;
		if (r.x + r.w > maxX) maxX = (int)(r.x + r.w);
		if (r.y < minY) minY = (int)r.y;
		if (r.y + r.h > maxY) maxY = (int)(r.y + r.h);
	}

	const std::pair<int, int> exitCell = map->getExitCell();
	const SDL_FRect &exitRect =
		tiles[exitCell.first * map->getCols() + exitCell.second]->getTilePos();

	const int panelW = 170;
	const int panelH = 176;
	corners[0] = {(float)(maxX - panelW), (float)minY, (float)panelW, (float)panelH};
	corners[1] = {(float)(maxX - panelW), (float)(maxY - panelH), (float)panelW, (float)panelH};
	corners[2] = {(float)minX, (float)minY, (float)panelW, (float)panelH};
	corners[3] = {(float)minX, (float)(maxY - panelH), (float)panelW, (float)panelH};

	return exitRect;
}

SDL_FRect App::GameManager::choosePanelCorner(const SDL_FRect &subjectRect, const SDL_FRect &exitRect, const SDL_FRect corners[4])
{
	// Corners are tried in the order given (top-right, bottom-right,
	// top-left, bottom-left); the first covering neither the subject nor
	// the exit wins. If every corner that leaves the subject visible
	// would cover the exit, one of those is used - the subject always
	// outranks the exit.
	const SDL_FRect *chosen = nullptr;
	const SDL_FRect *subjectSafe = nullptr;
	for (int i = 0; i < 4; ++i)
	{
		const SDL_FRect &corner = corners[i];
		if (rectsOverlap(corner, subjectRect))
			continue;
		if (subjectSafe == nullptr)
			subjectSafe = &corner;
		if (!rectsOverlap(corner, exitRect))
		{
			chosen = &corner;
			break;
		}
	}
	if (chosen == nullptr)
		chosen = subjectSafe != nullptr ? subjectSafe : &corners[0];
	return *chosen;
}

void App::GameManager::showTowerInspection(Game::Tower *tower, const SDL_FRect &towerTileRect)
{
	// The panel image carries the tower name and the row labels; only the
	// values are drawn at runtime, as seven-segment digits.
	// Textures draw at the image's native size, so all three panels get
	// the same position and only the matching type is shown.

	SDL_FRect corners[4];
	const SDL_FRect exitRect = panelCorners(corners);
	const SDL_FRect chosen = choosePanelCorner(towerTileRect, exitRect, corners);

	const int panelX = (int)chosen.x;
	const int panelY = (int)chosen.y;

	gameRenderer.getInspectDirectTexture()->setPosition(panelX, panelY);
	gameRenderer.getInspectSlowTexture()->setPosition(panelX, panelY);
	gameRenderer.getInspectAoeTexture()->setPosition(panelX, panelY);
	gameRenderer.getInspectDirectTexture()->setRender(tower->getType() == Game::TowerType::Direct);
	gameRenderer.getInspectSlowTexture()->setRender(tower->getType() == Game::TowerType::Slow);
	gameRenderer.getInspectAoeTexture()->setRender(tower->getType() == Game::TowerType::AoE);

	// These offsets line the value digits up with the row captions and
	// decimal point InspectPanel::Render draws at the matching y-offsets.

	// getCurrentLevel() is a 0-based index; players count levels from 1.
	inspectLevelLabel->setValue(tower->getCurrentLevel() + 1);
	inspectLevelLabel->setPosition(panelX + 146, panelY + 40);
	inspectLevelLabel->setRender(true);

	inspectRangeLabel->setValue(tower->getRange());
	inspectRangeLabel->setPosition(panelX + 146, panelY + 64);
	inspectRangeLabel->setRender(true);

	inspectPowerLabel->setValue(tower->getPower());
	inspectPowerLabel->setPosition(panelX + 158 - inspectPowerLabel->getWidth(), panelY + 88);
	inspectPowerLabel->setRender(true);

	// Rate of fire always shows one decimal (e.g. 1.2 s). The panel bakes
	// the dot, so the whole and fractional digits get fixed slots on
	// either side of it.
	int rateTenths = static_cast<int>(lroundf(tower->getRateOfFire() * 10.0f));
	inspectRateWholeLabel->setValue(rateTenths / 10);
	inspectRateWholeLabel->setPosition(panelX + 118, panelY + 112);
	inspectRateWholeLabel->setRender(true);
	inspectRateFractionLabel->setValue(rateTenths % 10);
	inspectRateFractionLabel->setPosition(panelX + 142, panelY + 112);
	inspectRateFractionLabel->setRender(true);

	inspectTargetLabel->setText(std::string("TARGET: ") + tower->getTargetingStrategy().getName());
	// Upgrade digits end at +156 and the panel at +176; +162 leaves the 7px line ~6px clear on each side.
	inspectTargetLabel->setPosition(panelX + 10, panelY + 162);
	inspectTargetLabel->setRender(true);

	if (tower->canUpgrade())
	{
		inspectUpgradeLabel->setValue(tower->getUpgradeCost());
		inspectUpgradeLabel->setPosition(panelX + 158 - inspectUpgradeLabel->getWidth(), panelY + 136);
		inspectUpgradeLabel->setRender(true);
		inspectMaxTexture->setRender(false);
	}
	else
	{
		inspectUpgradeLabel->setRender(false);
		inspectMaxTexture->setPosition(panelX + 158 - inspectMaxTexture->getWidth(), panelY + 136);
		inspectMaxTexture->setRender(true);
	}
}

void App::GameManager::hideTowerInspection()
{
	gameRenderer.getInspectDirectTexture()->setRender(false);
	gameRenderer.getInspectSlowTexture()->setRender(false);
	gameRenderer.getInspectAoeTexture()->setRender(false);

	inspectLevelLabel->setRender(false);
	inspectRangeLabel->setRender(false);
	inspectPowerLabel->setRender(false);
	inspectRateWholeLabel->setRender(false);
	inspectRateFractionLabel->setRender(false);
	inspectUpgradeLabel->setRender(false);
	inspectMaxTexture->setRender(false);
	inspectTargetLabel->setRender(false);
}

void App::GameManager::refreshTargetLabels(Game::Tower *tower)
{
	const char *name = tower->getTargetingStrategy().getName();

	targetNameLabel->setText(name);
	placeOnCompanionLine(targetNameLabel, kTargetButtonX);
	targetNameLabel->setRender(true);

	inspectTargetLabel->setText(std::string("TARGET: ") + name);
}

void App::GameManager::clearTowerSelection()
{
	upgradeButton->setButtonActive(false);
	sellButton->setButtonActive(false);
	targetButton->setButtonActive(false);
	targetNameLabel->setRender(false);
	cancelButton->setButtonActive(false);
	upgradePriceLabel->setRender(false);
	maxUpgradeTexture->setRender(false);
	sellPriceLabel->setRender(false);
	hideTowerInspection();
	gameRenderer.getSelectionHighlightTexture()->setRender(false);
	currentTowerSelected = nullptr;
}

Game::Critter *App::GameManager::findCritterAt(int x, int y)
{
	const int tileW = gameRenderer.getGenerator()->getWidth();
	const int tileH = gameRenderer.getGenerator()->getHeight();

	// Critter sprites are tile-sized and drawn at their interpolated
	// pixel position, so the clickable area is that sprite's rectangle.
	for (auto critter : critters)
	{
		if (!critter->getIsActive())
			continue;

		const float cx = critter->getPixelPosx();
		const float cy = critter->getPixelPosy();

		if (x >= cx && x < cx + tileW && y >= cy && y < cy + tileH)
			return critter;
	}
	return nullptr;
}

void App::GameManager::updateCritterInspection(Game::Critter *critter)
{
	SDL_FRect corners[4];
	const SDL_FRect exitRect = panelCorners(corners);

	// The critter's sprite rectangle - one tile wide/tall at its current
	// interpolated pixel position - is the panel's occlusion test subject.
	const SDL_FRect critterRect = {
		critter->getPixelPosx(),
		critter->getPixelPosy(),
		(float)gameRenderer.getGenerator()->getWidth(),
		(float)gameRenderer.getGenerator()->getHeight(),
	};

	// Re-picked every frame: the panel hops to the next clear corner as
	// soon as the critter walks into its current one.
	const SDL_FRect chosen = choosePanelCorner(critterRect, exitRect, corners);
	const int panelX = (int)chosen.x;
	const int panelY = (int)chosen.y;

	// The panel image carries the critter type name and the row labels;
	// all three panels share the position and only the type-matched one
	// draws.
	const std::string type = critter->getType();
	gameRenderer.getInspectWalkerTexture()->setPosition(panelX, panelY);
	gameRenderer.getInspectSpeedyTexture()->setPosition(panelX, panelY);
	gameRenderer.getInspectBossTexture()->setPosition(panelX, panelY);
	gameRenderer.getInspectWalkerTexture()->setRender(type == "Walker");
	gameRenderer.getInspectSpeedyTexture()->setRender(type == "Speedy");
	gameRenderer.getInspectBossTexture()->setRender(type == "Boss");

	// Row offsets line the digits up with the labels baked into the panel
	// images (see tools_dev/gen_critter_panels.py). Values are
	// right-aligned on the panel's value column and read fresh every
	// frame, so hp tracks damage taken and speed reflects active slows.
	inspectCritterLevelLabel->setValue(critter->getLevel());
	inspectCritterLevelLabel->setPosition(panelX + 158 - inspectCritterLevelLabel->getWidth(), panelY + 40);
	inspectCritterLevelLabel->setRender(true);

	inspectCritterHpLabel->setValue(critter->getHitPoints());
	inspectCritterHpLabel->setPosition(panelX + 158 - inspectCritterHpLabel->getWidth(), panelY + 62);
	inspectCritterHpLabel->setRender(true);

	inspectCritterMaxHpLabel->setValue(critter->getMaxHitPoints());
	inspectCritterMaxHpLabel->setPosition(panelX + 158 - inspectCritterMaxHpLabel->getWidth(), panelY + 84);
	inspectCritterMaxHpLabel->setRender(true);

	inspectCritterSpeedLabel->setValue(critter->getSpeed());
	inspectCritterSpeedLabel->setPosition(panelX + 158 - inspectCritterSpeedLabel->getWidth(), panelY + 106);
	inspectCritterSpeedLabel->setRender(true);

	inspectCritterRewardLabel->setValue(critter->getReward());
	inspectCritterRewardLabel->setPosition(panelX + 158 - inspectCritterRewardLabel->getWidth(), panelY + 128);
	inspectCritterRewardLabel->setRender(true);

	inspectCritterStrengthLabel->setValue(critter->getStrength());
	inspectCritterStrengthLabel->setPosition(panelX + 158 - inspectCritterStrengthLabel->getWidth(), panelY + 150);
	inspectCritterStrengthLabel->setRender(true);
}

void App::GameManager::clearCritterSelection()
{
	if (currentCritterSelected != nullptr)
	{
		restoreCritterSprite(currentCritterSelected);
	}
	currentCritterSelected = nullptr;

	// hide cancel button if no tower is selected (cancel button is used for both critters and towers)
	if (currentTowerSelected == nullptr)
		cancelButton->setButtonActive(false);

	gameRenderer.getInspectWalkerTexture()->setRender(false);
	gameRenderer.getInspectSpeedyTexture()->setRender(false);
	gameRenderer.getInspectBossTexture()->setRender(false);

	inspectCritterLevelLabel->setRender(false);
	inspectCritterHpLabel->setRender(false);
	inspectCritterMaxHpLabel->setRender(false);
	inspectCritterSpeedLabel->setRender(false);
	inspectCritterRewardLabel->setRender(false);
	inspectCritterStrengthLabel->setRender(false);
}

void App::GameManager::showCritterSelectedSprite(Game::Critter *critter)
{
	if (critter->getTexture() == nullptr)
	{
		// Headless critters have no sprite to swap; inspection still works.
		return;
	}

	const std::string type = critter->getType();
	::Renderer::spriteSheetTexture *bordered =
		type == "Walker" ? gameRenderer.getSelectedWalkerSprite()
		: type == "Speedy" ? gameRenderer.getSelectedSpeedySprite()
		: gameRenderer.getSelectedBossSprite();

	critterOriginalTexture = critter->getTexture();
	// The render observer only syncs whichever sprite the critter wears, so
	// the original has to be parked manually before the swap.
	critterOriginalTexture->render = false;
	critter->setTexture(bordered);
	bordered->setPosition(critter->getPixelPosx(), critter->getPixelPosy());
	bordered->render = true;
}

void App::GameManager::restoreCritterSprite(Game::Critter *critter)
{
	if (critterOriginalTexture == nullptr)
	{
		return;
	}

	critter->getTexture()->render = false;
	critter->setTexture(critterOriginalTexture);
	// A critter that died while selected must not have its sprite redrawn;
	// the observer resumes maintaining it on the next active frame.
	critterOriginalTexture->render = critter->getIsActive();
	critterOriginalTexture = nullptr;
}

void App::GameManager::MainLoop(float deltaTime)
{
	gameRenderer.MainLoop(mouse->getX(), mouse->getY(), deltaTime);
	// Game renderer

	if (outcome != GameOutcome::None)
	{
		// Game over: freeze the board as a backdrop behind the win/loss
		// overlay. No more tower placement/upgrades, critters, bullets, or
		// wave progression.
		return;
	}

	if (currentTowerSelected != nullptr)
		priceSellLabel(currentTowerSelected);

	// Buttons
	if (currentTowerSelected)
	{
		if (sellButton->m_pressed)
		{
			upgradeButton->setButtonActive(false);
			sellButton->setButtonActive(false);
			targetButton->setButtonActive(false);
			targetNameLabel->setRender(false);
			cancelButton->setButtonActive(false);
			upgradePriceLabel->setRender(false);
			maxUpgradeTexture->setRender(false);
			sellPriceLabel->setRender(false);
			hideTowerInspection();
			gameRenderer.getSelectionHighlightTexture()->setRender(false);

			// Selling pays back the current wave's percentage of everything
			// the player invested in this tower (see Tower::getRefundValue).
			int refund = currentTowerSelected->getRefundValue(waveManager->GetCurrentWave());
			player.ChangeCoins(+refund);

			// map->removeTower() notifies the map render observer, which
			// hides this cell's turret sprite - no manual removeTurret()
			// call needed here anymore.
			map->removeTower(currentTowerSelected->getRow(), currentTowerSelected->getCol());

			// remove from list
			towers.erase(std::remove(towers.begin(), towers.end(), currentTowerSelected), towers.end());
			// Destroy this tower's observers before deleting the tower so their
			// destructors can safely detach from the still-live subject.
			towerObserverSets.erase(
				std::remove_if(
					towerObserverSets.begin(),
					towerObserverSets.end(),
					[this](const std::unique_ptr<TowerObserverSet> &observerSet)
					{
						return observerSet->subject == currentTowerSelected;
					}),
				towerObserverSets.end());
			currentTowerSelected->DestroyTower();
			delete currentTowerSelected;
			currentTowerSelected = nullptr;
		}
		else if (cancelButton->m_pressed)
		{
			// Cancel only dismisses the selected-tower UI; the tower stays
			// on the map. Removing a tower for a refund is the sell
			// button's job, handled above.
			clearTowerSelection();
		}
		else if (targetButton->m_pressed)
		{
			// Only changes how the tower picks among critters in range: no
			// notify(), so nothing is charged, and the tower stays selected.
			currentTowerSelected->cycleTargetingStrategy();
			refreshTargetLabels(currentTowerSelected);
		}
		else if (upgradeButton->m_pressed)
		{
			std::cout << "UPGRADING TOWER " << std::endl;
			upgradeButton->setButtonActive(false);
			sellButton->setButtonActive(false);
			targetButton->setButtonActive(false);
			targetNameLabel->setRender(false);
			cancelButton->setButtonActive(false);
			upgradePriceLabel->setRender(false);
			maxUpgradeTexture->setRender(false);
			sellPriceLabel->setRender(false);
			hideTowerInspection();
			gameRenderer.getSelectionHighlightTexture()->setRender(false);

			if (currentTowerSelected->canUpgrade() && player.GetCoins() >= currentTowerSelected->getUpgradeCost())
			{
				// TryUpgrade() calls Tower::upgrade(), which notify()s this
				// tower's observers: billing charges the cost to player, and
				// the render observer reveals the next-level turret sprite.
				currentTowerSelected->TryUpgrade(player.GetCoins());
				auto observerSet = std::find_if(
					towerObserverSets.begin(),
					towerObserverSets.end(),
					[this](const std::unique_ptr<TowerObserverSet> &candidate)
					{
						return candidate->subject == currentTowerSelected;
					});
				if (observerSet == towerObserverSets.end())
				{
					throw std::logic_error("Selected tower has no observer set");
				}
				currentTowerSelected->setTexture(
					(*observerSet)->rendering.getLastUpgradedTexture());
			}

			currentTowerSelected = nullptr;
		}
	}

	if (currentCritterSelected != nullptr && cancelButton->m_pressed)
		clearCritterSelection();

	cancelButton->m_pressed = false;
	sellButton->m_pressed = false;
	targetButton->m_pressed = false;
	upgradeButton->m_pressed = false;

	// The start button only reacts while the wave manager is holding
	// between waves; the number on it names the wave about to begin.
	if (startButton->m_pressed && waveManager->IsWaitingForPlayer())
	{
		waveManager->StartNextWave();
		startButton->setButtonActive(false);
		startWaveLabel->setRender(false);
	}
	startButton->m_pressed = false;

	for (auto tile : gameRenderer.getGenerator()->getTiles())
	{
		tile->Update(deltaTime);

		if (tile->clickRequest != nullptr)
		{
			Tower *foundtower = nullptr;

			// A tile was clicked: find the tower (if any) built on that cell and select it.
			for (auto tower : towers)
			{
				if (tower->getRow() == tile->getCellNumber() / map->getCols() &&
					tower->getCol() == tile->getCellNumber() % map->getCols())
				{
					currentTowerSelected = tower;
					tower->printDetails();

					upgradeButton->setButtonActive(true);
					sellButton->setButtonActive(true);
					targetButton->setButtonActive(true);
					cancelButton->setButtonActive(true);
					refreshTargetLabels(tower);

					// Upgrade cost as gold digits centered on the lower half
					// of the upgrade button; a maxed-out tower shows the MAX
					// caption in the same spot instead.
					if (tower->canUpgrade())
					{
						upgradePriceLabel->setValue(tower->getUpgradeCost());
						placeOnCompanionLine(upgradePriceLabel, kUpgradeButtonX);
						upgradePriceLabel->setRender(true);
						maxUpgradeTexture->setRender(false);
						gameRenderer.getUpgradeTexture()->setCompanionLineHeight(upgradePriceLabel->getHeight());
					}
					else
					{
						upgradePriceLabel->setRender(false);
						placeOnCompanionLine(maxUpgradeTexture, kUpgradeButtonX);
						maxUpgradeTexture->setRender(true);
						gameRenderer.getUpgradeTexture()->setCompanionLineHeight(maxUpgradeTexture->getHeight());
					}

					// Show what selling this tower right now would pay,
					// as gold digits on the sell button.
					priceSellLabel(tower);
					sellPriceLabel->setRender(true);

					// Same click that opens the upgrade/cancel buttons
					// also opens the inspection panel for this tower.
					const SDL_FRect& tileRect = tile->getTilePos();
					showTowerInspection(tower, tileRect);

					// Ring this tower's tile so it is obvious on the map
					// which tower the upgrade/cancel buttons act on.
					::Renderer::Texture* highlight = gameRenderer.getSelectionHighlightTexture();
					highlight->setPosition((int)tileRect.x, (int)tileRect.y);
					highlight->setScale((int)tileRect.w, (int)tileRect.h);
					highlight->setRender(true);

					foundtower = tower;
				}
			}

			currentTowerSelected = foundtower;

			// Tower selection and critter selection are mutually exclusive.
			clearCritterSelection();

			delete tile->clickRequest;
			tile->clickRequest = nullptr;
		}
	}

	for (auto critter : critters)
	{
		// Update() already notified the critter billing observer above: a
		// lethal hit (via takeDamage()) rewards coins once, and reaching the
		// exit costs one health point once, then drains coins every frame
		// the critter idles there (Player::ChangeCoins() floors at 0). A
		// leaking critter is only recycled (removed from play) once it has
		// drained every coin the player has left, regardless of health - so
		// it keeps stealing for as long as there's anything left to steal,
		// then is swept away.
		critter->Update(deltaTime, gameRenderer.getGenerator()->getTiles());

		if (critter->killed || (critter->reachedEnd && player.GetCoins() <= 0))
		{
			critter->Death();
		}
	}

	// A selected critter is tracked every frame: the outline follows its
	// sprite, the panel re-picks its corner as the critter walks, and the
	// stat values are re-read so hp reflects damage taken mid-frame. A
	// critter that dies or is recycled mid-selection dismisses the panel.
	if (currentCritterSelected != nullptr)
	{
		if (!currentCritterSelected->getIsActive())
			clearCritterSelection();
		else
			updateCritterInspection(currentCritterSelected);
	}

	for (auto bullet : bullets)
	{
		bullet->Update(deltaTime);

		auto hitrq = bullet->GetHitRequest();

		if (hitrq)
		{
			// The blast's footprint is N path cells (N = this shot's level)
			// centered on the impact tile, following the critters' actual
			// walking path rather than a 2D radius - so a 1-cell blast is
			// just the impact tile, not its orthogonal neighbors too.
			auto cells = BlastFootprint(map->getCorrectPath(), map->getCols(), hitrq->hitRow, hitrq->hitCol, static_cast<int>(hitrq->blastRadius));

			Game::AoeTower::applySplash(critters, cells, static_cast<int>(hitrq->aoeDamage));

			for (const auto &cell : cells)
			{
				gameRenderer.setAOECell(cell.first, cell.second, 0);
			}

			bullet->DeleteHitRequest();
		}
	}

	for (Tower *tower : towers)
	{
		tower->Update(deltaTime, critters);

		if (tower->getBulletRequest() != nullptr)
		{
			Bullet *validBullet = findValidBullet();
			if (validBullet == nullptr)
				return;

			BulletRequest *rq = tower->getBulletRequest();
			validBullet->Shoot(rq->damage, rq->speed, rq->initalX, rq->initialY, rq->angle, rq->target, rq->effect, rq->blastRadius);

			tower->processShoot();
		}
	}

	waveManager->Update(deltaTime);

	// While the wave manager waits for the player, the start button names
	// the wave that is about to begin: "Start 1", "Start 2", ...
	if (waveManager->IsWaitingForPlayer())
	{
		if (!startButton->m_active)
		{
			startButton->setButtonActive(true);
		}
		startWaveLabel->setValue(waveManager->GetCurrentWave() + 1);
		placeOnCompanionLine(startWaveLabel, kStartButtonX);
		startWaveLabel->setRender(true);
	}

	waveLabel->setValue(waveManager->GetCurrentWave());

	// Check for a new terminal state after this frame's updates: a leak may
	// have just exhausted the player's health, or a wave clear may have just
	// returned WaveManager to waiting on/after the winning wave.
	if (player.IsDefeated())
	{
		outcome = GameOutcome::Lost;
		gameOverTexture->setRender(true);
		std::cout << "GAME OVER - you lost after " << waveManager->GetCurrentWave() << " waves." << std::endl;
	}
	else if (waveManager->IsWaitingForPlayer() && waveManager->GetCurrentWave() >= WIN_WAVE_COUNT)
	{
		outcome = GameOutcome::Won;
		youWinTexture->setRender(true);
		std::cout << "YOU WIN! Cleared " << waveManager->GetCurrentWave() << " waves." << std::endl;
	}
}

Game::Bullet *App::GameManager::findValidBullet()
{
	for (auto bullet : bullets)
	{
		if (bullet->GetActive() == false)
			return bullet;
	}

	return nullptr;
}

App::GameManager::~GameManager()
{
	critterObserverSets.clear();
	towerObserverSets.clear();
	mapObserverSet.reset();

	for (int i = 0; i < towers.size(); ++i)
	{
		delete towers[i];
	}

	for (int i = 0; i < critters.size(); ++i)
	{
		delete critters[i];
	}

	for (int i = 0; i < bullets.size(); ++i)
	{
		delete bullets[i];
	}

	delete upgradeButton;
	delete cancelButton;
	delete sellButton;
	delete targetButton;
	delete startButton;

	delete waveManager;

}

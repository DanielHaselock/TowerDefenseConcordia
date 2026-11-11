#include "GameRenderer.h"
#include "UiTheme.h"
#include <stdexcept>

void Renderer::GameRenderer::CreateGame(Window& window, int rows, int columns, std::vector<int>& cellValues, const std::vector<int>& towerPrices)
{
	this->columns = columns;
	int startingPixel = WINDOW_WIDTH / 8;

	mapGenerator = new MapGenerator(window, startingPixel, rows, columns, WINDOW_WIDTH, WINDOW_HEIGHT, cellValues);

	selectionHighlightTexture = window.addSelectionHighlight(0, 0, 0, 0);

	mainTexture = window.addTexture("Assets/smallSpriteSheetV2.png", Math::Vec4<int>(), Math::Vec3<int>(32, 32, 32));
	mainTexture->useSpecificSpriteSheet(255, 255, 64, 64, Math::Vec4<int>(startingPixel - WINDOW_WIDTH / 10, WINDOW_HEIGHT / 8, mapGenerator->GetMenuTileSize(), mapGenerator->GetMenuTileSize()));
	mainTexture->useSpecificSpriteSheet(319, 255, 64, 64, Math::Vec4<int>(startingPixel - WINDOW_WIDTH / 10, (WINDOW_HEIGHT / 20 + WINDOW_HEIGHT / 4), mapGenerator->GetMenuTileSize(), mapGenerator->GetMenuTileSize()));
	mainTexture->useSpecificSpriteSheet(191, 255, 64, 64, Math::Vec4<int>(startingPixel - WINDOW_WIDTH / 10, (WINDOW_HEIGHT / 20 + WINDOW_HEIGHT / 2), mapGenerator->GetMenuTileSize(), mapGenerator->GetMenuTileSize()));

	mainTexture->setAlpha(150.0f);

	//Menu sprites
	mainTexture->getSpriteSheet()[0]->render = false;
	mainTexture->getSpriteSheet()[1]->render = false;
	mainTexture->getSpriteSheet()[2]->render = false;

	// Show each menu icon's purchase price just below the icon; towerPrices is
	// index-aligned with the menu icons (direct, aoe, slow).
	const std::vector<spriteSheetTexture*>& menuSprites = mapGenerator->getMenuSprites();

	if (towerPrices.size() != menuSprites.size())
	{
		throw std::invalid_argument("Each tower menu icon must have a price");
	}

	for (size_t i = 0; i < menuSprites.size() && i < towerPrices.size(); ++i)
	{
		const SDL_FRect& icon = menuSprites[i]->m_pos_scale;
		::Renderer::PriceLabel* label = window.addPriceLabel((int)icon.x, (int)(icon.y + icon.h + 4), towerPrices[i]);
		label->setPosition((int)(icon.x + (icon.w - label->getWidth()) / 2), (int)(icon.y + icon.h + 4));
	}

	upgradeTexture = window.addStyledButton(0, 0, 0, 0, "UPGRADE", ::Renderer::ButtonVariant::Primary);
	sellTexture = window.addStyledButton(0, 0, 0, 0, "SELL", ::Renderer::ButtonVariant::Neutral);
	targetTexture = window.addStyledButton(0, 0, 0, 0, "TARGET", ::Renderer::ButtonVariant::Neutral);
	cancelTexture = window.addStyledButton(0, 0, 0, 0, "CANCEL", ::Renderer::ButtonVariant::Danger);
	startTexture = window.addStyledButton(0, 0, 0, 0, "START", ::Renderer::ButtonVariant::Success);
	// The upgrade/sell/target/start buttons each carry a value under their label
	// (price, refund, current targeting strategy, upcoming wave); GameManager owns those labels and sets
	// the companion line height so the two lines center as one block.

	constexpr int inspectPanelW = 170;
	constexpr int inspectPanelH = 176;

	// Row offsets mirror the fixed positions GameManager::showTowerInspection
	// places the matching NumberLabels at (panelY + 40/64/88/112/136), with
	// each caption drawn 6px below its value's top so the pixel-font text
	// lines up with the (taller) seven-segment-style digits beside it.
	const std::vector<::Renderer::InspectPanel::Row> towerRows{
		{"LVL", 46}, {"RANGE", 70}, {"DMG", 94}, {"RATE", 118}, {"UPGRADE", 142}};
	// Decimal point between the rate-of-fire whole (x+118) and fraction (x+142) labels.
	const SDL_Point rateDecimalPoint{135, 128};

	inspectDirectTexture = window.addInspectPanel(0, 0, inspectPanelW, inspectPanelH, "DIRECT TOWER", UiTheme::AccentGold, towerRows, rateDecimalPoint);
	inspectSlowTexture = window.addInspectPanel(0, 0, inspectPanelW, inspectPanelH, "SLOW TOWER", UiTheme::AccentBlue, towerRows, rateDecimalPoint);
	inspectAoeTexture = window.addInspectPanel(0, 0, inspectPanelW, inspectPanelH, "AOE TOWER", UiTheme::AccentRed, towerRows, rateDecimalPoint);

	// Row offsets mirror GameManager::updateCritterInspection's NumberLabel
	// positions (panelY + 40/62/84/106/128/150), same "caption 6px below its
	// value" convention as the tower rows above.
	const std::vector<::Renderer::InspectPanel::Row> critterRows{
		{"LEVEL", 46}, {"HP", 68}, {"MAX HP", 90}, {"SPEED", 112}, {"REWARD", 134}, {"STEAL", 156}};

	inspectWalkerTexture = window.addInspectPanel(0, 0, inspectPanelW, inspectPanelH, "WALKER", UiTheme::AccentGreen, critterRows);
	inspectSpeedyTexture = window.addInspectPanel(0, 0, inspectPanelW, inspectPanelH, "SPEEDY", UiTheme::AccentBlue, critterRows);
	inspectBossTexture = window.addInspectPanel(0, 0, inspectPanelW, inspectPanelH, "BOSS", UiTheme::AccentRed, critterRows);

	cancelTexture->setRender(false);
	upgradeTexture->setRender(false);
	sellTexture->setRender(false);
	inspectDirectTexture->setRender(false);
	inspectSlowTexture->setRender(false);
	inspectAoeTexture->setRender(false);
	inspectWalkerTexture->setRender(false);
	inspectSpeedyTexture->setRender(false);
	inspectBossTexture->setRender(false);
	selectionHighlightTexture->setRender(false);

	// Bordered critter variants, swapped into a critter while it is selected
	// (see GameManager::showCritterSelectedSprite). Same sheet layout as
	// mainTexture; kept fully opaque so the border reads clearly.
	selectedCritterTexture = window.addTexture("Assets/selectedSpriteSheetV2.png", Math::Vec4<int>(), Math::Vec3<int>(32, 32, 32));
	const Math::Vec4<int> critterDest(0, WINDOW_HEIGHT / 8, mapGenerator->getWidth(), mapGenerator->getHeight());
	selectedWalkerSprite = selectedCritterTexture->useSpecificSpriteSheet(0, 255, 64, 64, critterDest);
	selectedSpeedySprite = selectedCritterTexture->useSpecificSpriteSheet(128, 255, 64, 64, critterDest);
	selectedBossSprite = selectedCritterTexture->useSpecificSpriteSheet(128, 319, 64, 64, critterDest);
	selectedWalkerSprite->render = false;
	selectedSpeedySprite->render = false;
	selectedBossSprite->render = false;
	startTexture->setRender(false);
}

void Renderer::GameRenderer::MainLoop(int mouseX, int mouseY, float deltatime)
{
	currClosestTile = mapGenerator->GetClosestTile(mouseX, mouseY);
}

void Renderer::GameRenderer::setSpriteSheetRender(int numb, bool render)
{
	mainTexture->getSpriteSheet()[numb]->render = render;
}

void Renderer::GameRenderer::handleMouseMove(int x, int y)
{
	mainTexture->getSpriteSheet()[0]->setPosition(x - mainTexture->getSpriteSheet()[0]->m_pos_scale.w / 2, y - mainTexture->getSpriteSheet()[0]->m_pos_scale.h / 2);
	mainTexture->getSpriteSheet()[1]->setPosition(x - mainTexture->getSpriteSheet()[0]->m_pos_scale.w / 2, y - mainTexture->getSpriteSheet()[0]->m_pos_scale.h / 2);
	mainTexture->getSpriteSheet()[2]->setPosition(x - mainTexture->getSpriteSheet()[0]->m_pos_scale.w / 2, y - mainTexture->getSpriteSheet()[0]->m_pos_scale.h / 2);
}

void Renderer::GameRenderer::handleMouseRelease(int x, int y, int spritePressed)
{
	mainTexture->getSpriteSheet()[spritePressed]->render = false;
}

void Renderer::GameRenderer::placeTurret(int spriteToShow)
{
	currClosestTile->PlaceTurret(spriteToShow);
}

::Renderer::spriteSheetTexture* Renderer::GameRenderer::upgradeTurret(int tileNumb)
{
	return mapGenerator->getTiles()[tileNumb]->UpgradeTurret();
}

void Renderer::GameRenderer::removeTurret(int tileNumb)
{
	for (auto turretText : mapGenerator->getTiles()[tileNumb]->getTurretTextures())
	{
		turretText->render = false;
	}
}

int Renderer::GameRenderer::GetClosestCellNumber(int x, int y)
{
	return currClosestTile->getCellNumber();
}

Renderer::spriteSheetTexture* Renderer::GameRenderer::SpawnWalker(Window& window)
{
	critterSprites.emplace_back(mainTexture->useSpecificSpriteSheet(0, 255, 64, 64, Math::Vec4<int>(0, WINDOW_HEIGHT / 8, mapGenerator->getWidth(), mapGenerator->getHeight())));
	return critterSprites[critterSprites.size() - 1];
}

Renderer::spriteSheetTexture* Renderer::GameRenderer::SpawnSpeed(Window& window)
{
	critterSprites.emplace_back(mainTexture->useSpecificSpriteSheet(128, 255, 64, 64, Math::Vec4<int>(0, WINDOW_HEIGHT / 8, mapGenerator->getWidth(), mapGenerator->getHeight())));
	return critterSprites[critterSprites.size() - 1];
}

Renderer::spriteSheetTexture* Renderer::GameRenderer::SpawnBoss(Window& window)
{
	critterSprites.emplace_back(mainTexture->useSpecificSpriteSheet(128, 319, 64, 64, Math::Vec4<int>(0, WINDOW_HEIGHT / 8, mapGenerator->getWidth(), mapGenerator->getHeight())));
	return critterSprites[critterSprites.size() - 1];
}

Renderer::spriteSheetTexture* Renderer::GameRenderer::SpawnBullet(Window& window)
{
	bulletSprites.emplace_back(mainTexture->useSpecificSpriteSheet(383, 255, 64, 64, Math::Vec4<int>(0, WINDOW_HEIGHT / 8, mapGenerator->getWidth(), mapGenerator->getHeight())));
	return bulletSprites[bulletSprites.size() - 1];
}

void Renderer::GameRenderer::setAOECell(int row, int col, int damageOverTime)
{
	mapGenerator->getTiles()[row * columns + col]->infect(damageOverTime);
}

void Renderer::GameRenderer::updateButtonHover(int mouseX, int mouseY)
{
	upgradeTexture->setHovered(UiTheme::pointInRect(mouseX, mouseY, upgradeTexture->getScale()));
	sellTexture->setHovered(UiTheme::pointInRect(mouseX, mouseY, sellTexture->getScale()));
	targetTexture->setHovered(UiTheme::pointInRect(mouseX, mouseY, targetTexture->getScale()));
	cancelTexture->setHovered(UiTheme::pointInRect(mouseX, mouseY, cancelTexture->getScale()));
	startTexture->setHovered(UiTheme::pointInRect(mouseX, mouseY, startTexture->getScale()));
}

Renderer::GameRenderer::~GameRenderer()
{
	delete mapGenerator;
}

#include "PlayerRenderObserver.h"
#include "UiTheme.h"

namespace
{
    const int LABEL_MARGIN = 8;
    const int CAPTION_GAP = 4; // between a caption ("HP", "COINS") and its number

    // Red, so losing health reads as a warning distinct from the gold coin counter.
    const Math::Vec3<int> HEALTH_COLOR(220, 40, 40);

    // Coins live centered between the left side-menu (ends at x=80) and the
    // first HUD button, the target button (starts at x=188, see
    // kTargetButtonX in GameManager), caption stacked above the number.
    const int COIN_AREA_LEFT = 80;
    const int COIN_AREA_RIGHT = 188;
    const int COIN_BLOCK_GAP = 4;
}

App::PlayerRenderObserver::PlayerRenderObserver(Player& player, ::Renderer::Window& window)
    : Game::Observer<Player>(player)
{
    coinLabel = window.addNumberLabel(0, 0, static_cast<int>(player.GetCoins()));
    healthLabel = window.addNumberLabel(0, 0, player.GetHealth(), HEALTH_COLOR);

    coinLabelText = window.addTextLabel(0, 0, "COINS", 1, ::Renderer::UiTheme::TextMuted);
    healthLabelText = window.addTextLabel(0, 0, "HP", 1, ::Renderer::UiTheme::TextMuted);

    placeLabels();
}

void App::PlayerRenderObserver::update(const Player& player)
{
    // Coins are tracked as float (leak penalties drain fractionally); the
    // counter shows the whole coins the player actually has.
    coinLabel->setValue(static_cast<int>(player.GetCoins()));
    healthLabel->setValue(player.GetHealth());
    placeLabels();
}

void App::PlayerRenderObserver::placeLabels()
{
    // Health sits above the wave counter in the bottom-left corner (wave
    // is flush to the corner - see GameManager's waveY). Each number
    // starts right after its caption, vertically centered against the digits.
    const int healthY = WINDOW_HEIGHT - 56;
    const int healthTextWidth = healthLabelText->getWidth();

    healthLabelText->setPosition(LABEL_MARGIN, healthY + (healthLabel->getHeight() - healthLabelText->getHeight()) / 2);
    healthLabel->setPosition(LABEL_MARGIN + healthTextWidth + CAPTION_GAP, healthY);

    // Re-centered every call since the number's width changes with its digit count.
    const int bottomStripTop = WINDOW_HEIGHT - 80;
    const int coinAreaCenterX = (COIN_AREA_LEFT + COIN_AREA_RIGHT) / 2;
    const int coinTextHeight = coinLabelText->getHeight();
    const int coinBlockHeight = coinTextHeight + COIN_BLOCK_GAP + coinLabel->getHeight();
    const int coinBlockTop = bottomStripTop + (80 - coinBlockHeight) / 2;

    coinLabelText->setPosition(coinAreaCenterX - coinLabelText->getWidth() / 2, coinBlockTop);
    coinLabel->setPosition(coinAreaCenterX - coinLabel->getWidth() / 2, coinBlockTop + coinTextHeight + COIN_BLOCK_GAP);
}

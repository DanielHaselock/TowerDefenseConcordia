#include "CritterRenderSyncObserver.h"
#include "Texture.h"

App::CritterRenderSyncObserver::CritterRenderSyncObserver(Game::Critter& critter)
    : Game::Observer<Game::Critter>(critter)
{
}

void App::CritterRenderSyncObserver::update(const Game::Critter& critter)
{
    ::Renderer::spriteSheetTexture* sprite = critter.getTexture();
    if (sprite == nullptr)
    {
        // Critters built without a sprite (e.g. headless tests) simply have
        // nothing to sync; the model state is still valid.
        return;
    }

    sprite->setPosition(critter.getPixelPosx(), critter.getPixelPosy());
    sprite->render = critter.getIsActive();
}

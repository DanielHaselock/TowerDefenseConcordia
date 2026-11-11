#include "BossCritter.h"

#include <iostream>
#include <ostream>

namespace {
    constexpr int BASE_HP = 30;
    constexpr int BASE_STRENGTH = 6;
    constexpr int BASE_SPEED = 25;
    constexpr int BASE_REWARD = 40;
}

Game::BossCritter::BossCritter(int level, int spawnRow, int spawnCol, Renderer::spriteSheetTexture* texture)
    : Critter(BASE_HP * level, BASE_STRENGTH * level, BASE_SPEED,
              level, BASE_REWARD * level, spawnRow, spawnCol, texture) {}

std::string Game::BossCritter::getType() const {
    return "Boss";
}

std::unique_ptr<Game::Critter> Game::BossCritter::clone() const {
    return std::make_unique<BossCritter>(*this);
}

void Game::BossCritter::Update(float deltaTime, const std::vector<::Renderer::RendererMap*>& m_tiles)
{
    ::Game::Critter::Update(deltaTime, m_tiles);
}

void Game::BossCritter::setLevel(int p_level)
{
    maxHitPoints = hitPoints = BASE_HP * p_level;
    strength = BASE_STRENGTH * p_level;
    reward = BASE_REWARD * p_level;
    level = p_level;
}

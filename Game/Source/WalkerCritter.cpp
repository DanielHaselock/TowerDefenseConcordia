#include <iostream>
#include <ostream>

#include "WalkerCritter.h"
#include "Texture.h"
namespace {
    constexpr int BASE_HP = 5;
    constexpr int BASE_STRENGTH = 2;
    constexpr int BASE_SPEED = 50;
    constexpr int BASE_REWARD = 5;
}

Game::WalkerCritter::WalkerCritter(int level, int spawnRow, int spawnCol, ::Renderer::spriteSheetTexture* texture)
    : Critter(BASE_HP * level, BASE_STRENGTH * level, BASE_SPEED,
              level, BASE_REWARD * level, spawnRow, spawnCol, texture) 
{
    base_speed = BASE_SPEED;
}

std::string Game::WalkerCritter::getType() const {
    return "Walker";
}

std::unique_ptr<Game::Critter> Game::WalkerCritter::clone() const {
    return std::make_unique<WalkerCritter>(*this);
}

void Game::WalkerCritter::Update(float deltaTime, const std::vector<::Renderer::RendererMap*>& m_tiles)
{
    ::Game::Critter::Update(deltaTime, m_tiles);
}

void Game::WalkerCritter::setLevel(int p_level)
{
    maxHitPoints = hitPoints = BASE_HP * p_level;
    strength = BASE_STRENGTH * p_level;
    reward = BASE_REWARD * p_level;
    level = p_level;
}

#include "SpeedyCritter.h"

#include <iostream>
#include <ostream>
#include "Texture.h"

namespace {
    constexpr int BASE_HP = 3;
    constexpr int BASE_STRENGTH = 1;
    constexpr int BASE_SPEED = 150;
    constexpr int BASE_REWARD = 8;
}

Game::SpeedyCritter::SpeedyCritter(int level, int spawnRow, int spawnCol, ::Renderer::spriteSheetTexture* texture)
    : Critter(BASE_HP * level, BASE_STRENGTH * level, BASE_SPEED,
              level, BASE_REWARD * level, spawnRow, spawnCol, texture) 
{
    base_speed = BASE_SPEED;
    speed = base_speed;
}

std::string Game::SpeedyCritter::getType() const {
    return "Speedy";
}

std::unique_ptr<Game::Critter> Game::SpeedyCritter::clone() const {
    return std::make_unique<SpeedyCritter>(*this);
}

void Game::SpeedyCritter::Update(float deltaTime, const std::vector<::Renderer::RendererMap*>& m_tiles)
{  
    ::Game::Critter::Update(deltaTime, m_tiles);
}

void Game::SpeedyCritter::setLevel(int p_level)
{
    maxHitPoints = hitPoints = BASE_HP * p_level;
    strength = BASE_STRENGTH * p_level;
    reward = BASE_REWARD * p_level;
    level = p_level;
    // Speed ramps up linearly until level 2, then holds at full BASE_SPEED
    // for all higher levels (the min(1, level/2) factor caps at 1).
    base_speed = BASE_SPEED * std::min((float)1, (float)p_level / 2);

    speed = base_speed;
    if (speedHalfed)
    {
        speed /= 2;
    }
}

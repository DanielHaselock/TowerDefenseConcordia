#include "Critter.h"
#include "Map.h"
#include "RendererMap.h"
#include <cmath>
#include <iostream>
Game::Critter::Critter(int hitPoints, int strength, int speed, int level, int reward, int spawnRow, int spawnCol, Renderer::spriteSheetTexture* texture)
    : hitPoints(hitPoints), maxHitPoints(hitPoints), strength(strength),
      speed(speed), baseSpeed(speed), level(level), reward(reward), row(spawnRow), col(spawnCol), m_texture(texture){}

void Game::Critter::setPath(std::vector<Renderer::RendererMap*> m_tiles, const std::vector<int>& p_correctPath, int columns)
{
    positions.clear();
    m_correctPath = p_correctPath;
    m_columns = columns;

    for (int tile : p_correctPath)
    {
        positions.emplace_back((float)m_tiles[tile]->getSpriteTexture()->m_pos_scale.x, (float)m_tiles[tile]->getSpriteTexture()->m_pos_scale.y);
    }

    pixelPosX = positions[currentPos].first;
    pixelPosY = positions[currentPos].second;

    destinationX = positions[currentPos + 1].first;
    destinationY = positions[currentPos + 1].second;

    row = m_correctPath[currentPos] / m_columns;
    col = m_correctPath[currentPos] % m_columns;

    currentTile = m_tiles[m_correctPath[currentPos]];
}

void Game::Critter::resetCritter()
{
    isActive = false;
    killed = false;
    reachedEnd = false;
    hitPoints = maxHitPoints;
    currentPos = 0;

    pixelPosX = positions[currentPos].first;
    pixelPosY = positions[currentPos].second;

    destinationX = positions[currentPos + 1].first;
    destinationY = positions[currentPos + 1].second;

    row = m_correctPath[currentPos] / m_columns;
    col = m_correctPath[currentPos] % m_columns;
    notify();
}

void Game::Critter::takeDamage(int amount)
{
    if (amount < 0) return;

    // Overkill is allowed to drive hitPoints further negative (existing,
    // tested behavior) - but only notify while this hit is the one that
    // first kills the critter. Without this guard, further hits landing
    // on an already-dead critter (e.g. overlapping AOE splash in the same
    // frame) would notify again, and an observer reacting to a kill could
    // reward it more than once for the same death.
    bool wasAlreadyKilled = killed;

    hitPoints -= amount;
    if (hitPoints <= 0)
    {
        killed = true;
    }

    if (!wasAlreadyKilled)
    {
        notify();
    }
}

void Game::Critter::Death()
{
    resetCritter();
}

bool Game::Critter::isDead() const {
    return !isActive;
}

void Game::Critter::Update(float deltaTime, const std::vector<::Renderer::RendererMap*>& m_tiles)
{
    if (!isActive)
        return;

    lastDeltaTime = deltaTime;

    if (speedHalfed)
    {
        speedHalfTime += deltaTime;

        if (speedHalfTime >= maxSpeedHalfTime)
        {
            speed = base_speed;
            speedHalfed = false;
        }
    }

    int diffX = destinationX - pixelPosX;
    int diffY = destinationY - pixelPosY;

    int distanceX = abs(destinationX - positions[currentPos].first);
    int distanceY = abs(destinationY - positions[currentPos].second);

    if (diffX != 0)
    {
        pixelPosX += diffX / abs(diffX) * speed * deltaTime;
    }
    if (diffY != 0)
    {
        pixelPosY += diffY / abs(diffY) * speed * deltaTime;
    }

    // Switch to the next tile's row/col once 75% of the way across the
    // current tile, rather than waiting to fully reach the pixel destination.
    if (!changedSquare && (currentPos + 1) < positions.size() && abs(diffX) <= ((float)distanceX / 2 + (float)distanceX / 4) && abs(diffY) <= (float)distanceY / 2 + (float)distanceY / 4)
    {
        changedSquare = true;
        const int nextTile = m_correctPath[currentPos + 1];
        row = nextTile / m_columns;
        col = nextTile % m_columns;
        
        currentTile = m_tiles[nextTile];
        timeToDamage = 0;
    }

    // Infected tiles only hurt critters when they actually carry damage;
    // AoE blasts infect tiles purely as a visual footprint.
    if (currentTile->isInfected && currentTile->infectedDamage > 0)
    {
        timeToDamage += deltaTime;

        if (timeToDamage >= 1)
        {
            takeDamage(currentTile->infectedDamage);
            timeToDamage = 0;
        }

    }

    if (diffX == 0 && diffY == 0)
    {
        if ((currentPos + 1) < positions.size())
        {
            currentPos++;
            destinationX = positions[currentPos].first;
            destinationY = positions[currentPos].second;
        }
        else
        {
            // Stays true every frame the critter is left active at the
            // exit; the end-of-Update() notify() then fires once per frame,
            // which is exactly the "continuous per-frame" signal the billing
            // observer needs to keep draining coins. It stops the moment
            // Death() clears isActive, since Update() then returns before
            // reaching here.
            reachedEnd = true;
        }

        changedSquare = false;
    }

    // Movement is a state change too: notify once per step so the
    // render-sync observer keeps the sprite at the critter's new pixel
    // position (replacing the direct sprite call this method used to make).
    notify();
}

float Game::Critter::getDistanceToExit() const
{
    if (positions.empty())
    {
        return 0.0f;
    }

    // Update() starts out walking toward positions[1] with currentPos still
    // 0, and from then on walks toward positions[currentPos]; so the index of
    // the tile being walked toward is currentPos, but never below 1.
    size_t next = static_cast<size_t>(currentPos < 1 ? 1 : currentPos);
    if (next >= positions.size())
    {
        next = positions.size() - 1;
    }

    float dx = positions[next].first - pixelPosX;
    float dy = positions[next].second - pixelPosY;
    float remaining = std::sqrt(dx * dx + dy * dy);

    for (size_t i = next; i + 1 < positions.size(); ++i)
    {
        float segX = static_cast<float>(positions[i + 1].first - positions[i].first);
        float segY = static_cast<float>(positions[i + 1].second - positions[i].second);
        remaining += std::sqrt(segX * segX + segY * segY);
    }

    return remaining;
}

int Game::Critter::getHitPoints() const { return hitPoints; }
int Game::Critter::getMaxHitPoints() const { return maxHitPoints; }
int Game::Critter::getStrength() const { return strength; }
int Game::Critter::getSpeed() const { return speed; }
int Game::Critter::getBaseSpeed() const { return baseSpeed; }
int Game::Critter::getLevel() const { return level; }
int Game::Critter::getReward() const { return reward; }
int Game::Critter::getRow() const { return row; }
int Game::Critter::getCol() const { return col; }

void Game::Critter::setPosition(int newRow, int newCol) {
    row = newRow;
    col = newCol;
}

void Game::Critter::setPixelPosition(int x, int y)
{
    pixelPosX = x;
    pixelPosY = y;
}

void Game::Critter::setDestination(int x, int y)
{
    destinationX = x;
    destinationY = y;
}

void Game::Critter::setIsActive(bool p_active)
{
    isActive = p_active;
    notify();
}

void Game::Critter::halfSpeed()
{
    if (!speedHalfed)
    {
        speed = speed / 2;
        speedHalfed = true;
    }

    speedHalfTime = 0;
}

#include <algorithm>
#include <array>
#include <stdexcept>
#include "Tower.h"
#include "Texture.h"
#include <iostream>

#include <cmath>
#include <numbers>
constexpr float PI = 3.14159265358979323846f;


namespace Game {

    Tower::Tower(int row, int col, int cost, int currLvl, ::Renderer::spriteSheetTexture* p_texture, TargetingStrategyType type)
        : row(row), col(col), buyingCost(cost),
          currentLevel(currLvl),
          // Start far enough in the past that the tower can fire immediately
          // instead of waiting out a fake cooldown on its very first operate().
          lastShotTime(0), m_texture(p_texture),
          targetingStrategy(makeTargetingStrategy(type)){}

    // ---- Upgrade logic ----

    bool Tower::canUpgrade() const {
        return currentLevel + 1 < static_cast<int>(levels.size());
    }

    int Tower::getUpgradeCost() const {
        if (!canUpgrade())
        {
            return 0;
        }
        return levels[currentLevel + 1].levelCost;
    }

    int Tower::getCurrentLevelCost() const {
        return levels[currentLevel].levelCost;
    }

    void Tower::upgrade() {
        if (!canUpgrade()) {
            return;
        }
        currentLevel++;
        notify();
    }

    int Tower::getRefundValue(int wave) const {
        // Everything the player has spent on this tower: the buy cost
        // (level 1's entry) plus every upgrade level paid for so far.
        int invested = 0;
        for (int lvl = 0; lvl <= currentLevel; ++lvl)
        {
            invested += levels[lvl].levelCost;
        }

        // Integer division rounds the refund down, so a sale can never
        // pay out more than the wave's percentage of what was invested.
        return invested * GetRefundPercent(wave) / 100;
    }

    int Tower::GetRefundPercent(int wave) {
        // Selling early gives most of the money back; from wave 6 on the
        // refund bottoms out at 55% and stays there.
        static const int percentByWave[6] = { 80, 75, 70, 65, 60, 55 };

        if (wave < 1)
        {
            wave = 1;
        }
        if (wave > 6)
        {
            wave = 6;
        }

        return percentByWave[wave - 1];
    }

    // ---- Getters ----

    int Tower::getRow() const { return row; }
    int Tower::getCol() const { return col; }
    int Tower::getBuyingCost() const { return buyingCost; }
    int Tower::getCurrentLevel() const { return currentLevel; }
    int Tower::getRange() const { return levels[currentLevel].range; }
    int Tower::getPower() const { return levels[currentLevel].power; }
    float Tower::getRateOfFire() const { return levels[currentLevel].rateOfFire; }

    // ---- Combat logic ----

    bool Tower::isInRange(const Critter& critter) const {
        int dRow = critter.getRow() - row;
        int dCol = critter.getCol() - col;
        int distanceSquared = dRow * dRow + dCol * dCol;

        int range = levels[currentLevel].range;
        return distanceSquared <= range * range;
    }

    Critter* Tower::selectTarget(const std::vector<Critter*>& critters) const {
        // Finding the candidates is the same for every tower; only the
        // choice among them is up to the strategy.
        std::vector<Critter*> candidates;

        for (Critter* critter : critters) {
            if (critter->isDead() || !isInRange(*critter)) {
                continue; // skip dead critters and ones outside our range
            }
            candidates.push_back(critter);
        }

        return targetingStrategy->chooseTarget(candidates, row, col);
    }

    void Tower::setTargetingStrategy(std::unique_ptr<TargetingStrategy> strategy) {
        if (strategy) {
            targetingStrategy = std::move(strategy);
        }
    }

    const TargetingStrategy& Tower::getTargetingStrategy() const {
        return *targetingStrategy;
    }

    void Tower::cycleTargetingStrategy() {
        static constexpr std::array strategies{
            TargetingStrategyType::NearestToTower,
            TargetingStrategyType::NearestToExit,
            TargetingStrategyType::Strongest,
            TargetingStrategyType::Weakest
        };
        const auto current = std::find(strategies.begin(), strategies.end(), targetingStrategy->getType());
        const auto next = (current + 1 == strategies.end()) ? strategies.begin() : current + 1;
        targetingStrategy = makeTargetingStrategy(*next);
    }

    void Tower::DestroyTower()
    {
        delete bulletRequest;
    }

    void Tower::operate(float currentTime, const std::vector<Critter*>& critters) {
        // Still on cooldown: too soon since the last shot.
        //if (currentTime - lastShotTime < levels[currentLevel].rateOfFire) {
        //    return;
        //}

        //Critter* target = selectTarget(critters);
        //if (target == nullptr) {
        //    return; // nothing in range to shoot at
        //}

        //shoot(,*target);
        //lastShotTime = currentTime;
    }

    void Tower::Update(float deltatime, const std::vector<Critter*>& critters)
    {
        //// Still on cooldown: too soon since the last shot.
        //if (deltatime - lastShotTime < levels[currentLevel].rateOfFire) {
        //    return;
        //}

        Critter* target = selectTarget(critters);
        if (target == nullptr) 
        {
            textureAngle += deltatime * baseTurnSpeed;

            if (textureAngle > 360)
            {
                textureAngle -= 360;
            }

            m_texture->m_angle = textureAngle;

            return; // nothing in range to shoot at
        }
        else
        {
            // Turret center
            float turretX = m_texture->m_pos_scale.x;
            float turretY = m_texture->m_pos_scale.y;

            // Critter center
            SDL_FRect targetRect = target->getTexture()->m_pos_scale;

            float targetX = targetRect.x;
            float targetY = targetRect.y;

            // Direction
            float dx = targetX - turretX;
            float dy = targetY - turretY;

            // Convert direction to degrees
            textureAngle = std::atan2(dy, dx) * 180.0f / PI;
            textureAngle += 90; //image is offset incorrectly
            m_texture->m_angle = textureAngle;

            if (canShoot)
            {
                shoot(turretX, turretY, textureAngle, *target);
                canShoot = false;
                lastShotTime = 0;
            }
            else
            {
                // Cooldown between shots is the current level's rate of
                // fire, in seconds; upgraded towers fire on their own
                // cadence (slow towers faster, AoE towers slower).
                lastShotTime += deltatime;

                if (lastShotTime >= levels[currentLevel].rateOfFire)
                {
                    canShoot = true;
                }
            }

        }
    }

    void Tower::setTexture(::Renderer::spriteSheetTexture* p_text)
    {
        m_texture = p_text;
    }

    void Tower::processShoot()
    {
        delete bulletRequest;
        bulletRequest = nullptr;
    }

    void Tower::printDetails()
    {
        std::cout << "TOWER: ROW: " << row << " COL: " << col << std::endl;
    }

    Tower::~Tower()
    {
        if (bulletRequest)
            delete bulletRequest;

        bulletRequest = nullptr;
    }

} // namespace Game

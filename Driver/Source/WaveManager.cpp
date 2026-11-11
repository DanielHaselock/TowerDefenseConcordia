#include "WaveManager.h"
#include <algorithm>
#include <iostream>
#include <random>

App::WaveManager::WaveManager(std::vector<Game::Critter*> p_critters) : critters(p_critters)
{
    waveState = App::WaveState::waiting;

    stats.reserve(100);

    for (int wave = 1; wave <= 100; ++wave)
    {
        WaveStats waveStats{};

        // Total enemies increases with wave
        int totalEnemies = 5 + wave * 2;

        // Probabilities gradually change with wave
        float bossChance =
            std::min(0.50f, wave * 0.01f);

        float speedyChance =
            std::min(0.40f, 0.1f + wave * 0.02f);

        float walkerChance =
            1.0f - speedyChance - bossChance;

        waveStats.amountBoss =
            static_cast<int>(totalEnemies * bossChance);

        waveStats.amountSpeedy =
            static_cast<int>(totalEnemies * speedyChance);

        waveStats.amountWalker =
            totalEnemies
            - waveStats.amountSpeedy
            - waveStats.amountBoss;

        stats.push_back(waveStats);
    }
}

void App::WaveManager::IncreaseWave()
{
    if (currWave % 3 == 0)
    {
        for (auto critter : critters)
        {
            critter->setLevel((currWave / 3) + 1);
        }
    }

    currWave++;
}

App::WaveStats App::WaveManager::GetWaveStats()
{
    return stats[currWave];
}

void App::WaveManager::Update(float deltatime)
{
    if (waveState == App::WaveState::playing)
    {
        CheckWaveEnd();
        ShouldEndWave();
        return;
    }
    if (waveState == App::WaveState::spawning)
    {
        currSpawnTime += deltatime;

        if (currSpawnTime >= critterSpawnTime)
        {
            currSpawnTime = 0;
            SpawnCritter();
        }
    }
    else if (waveState == App::WaveState::waiting)
    {
        // Between waves the manager holds here until the player presses
        // the start button, which calls StartNextWave().
    }
}

void App::WaveManager::StartNextWave()
{
    if (waveState != App::WaveState::waiting)
    {
        return;
    }

    waveState = App::WaveState::spawning;
    IncreaseWave();
    amountCritterSpawned = 0;
}

bool App::WaveManager::IsWaitingForPlayer() const
{
    return waveState == App::WaveState::waiting;
}

int App::WaveManager::GetCurrentWave() const
{
    return currWave;
}

void App::WaveManager::ShouldEndWave()
{
    if (!CheckWaveEnd())
    {
        return;
    }

    waveState = App::WaveState::waiting;
}

bool App::WaveManager::CheckWaveEnd()
{
    for (auto critter : critters)
    {
        if (!critter->isDead() && !critter->reachedEnd)
            return false;
    }

    return true;
}

void App::WaveManager::SpawnCritter()
{
    auto stats = GetWaveStats();
    
    if (amountCritterSpawned <
        stats.amountWalker +
        stats.amountSpeedy +
        stats.amountBoss)
    {
        std::vector<int> availableTypes;

        if (amountWalkerSpawned < stats.amountWalker)
            availableTypes.push_back(0);

        if (amountSpeedSpawned < stats.amountSpeedy)
            availableTypes.push_back(1);

        if (amountBossSpawned < stats.amountBoss)
            availableTypes.push_back(2);

        int randomIndex =
            GetRandomNumber(0, static_cast<int>(availableTypes.size()) - 1);

        int type = availableTypes[randomIndex];

        // The shared critter pool is laid out as walkers, then speedy critters,
        // then bosses (see GameManager::SpawnWalker/SpawnSpeed/SpawnBoss), so each
        // type's next free slot is offset by the size of the pools before it.
        if (type == 0)
        {
            critters[amountWalkerSpawned]->setIsActive(true);
            amountWalkerSpawned++;
        }
        else if (type == 1)
        {
            critters[MAX_WALKER_POOL + amountSpeedSpawned]
                ->setIsActive(true);

            amountSpeedSpawned++;
        }
        else if (type == 2)
        {
            critters[MAX_WALKER_POOL +
                MAX_SPEED_POOL +
                amountBossSpawned]
                ->setIsActive(true);

            amountBossSpawned++;
        }

        amountCritterSpawned++;
    }
    else
    {
        amountBossSpawned = amountSpeedSpawned = amountWalkerSpawned = amountCritterSpawned = 0;
        waveState = App::WaveState::playing;
    }
}

int App::WaveManager::GetRandomNumber(int a, int b)
{
    std::random_device rd;

    // 2. Initialize the standard mersenne_twister_engine with the seed
    std::mt19937 gen(rd());

    // 3. Define the inclusive range [0, 2]
    std::uniform_int_distribution<int> distribution(a, b);

    // 4. Generate the random number
    int random_num = distribution(gen);

    return random_num;
}

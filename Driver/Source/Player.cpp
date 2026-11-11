#include "Player.h"
#include <iostream>

App::Player::Player() : coins(STARTING_COINS), health(STARTING_HEALTH)
{
}

void App::Player::ChangeCoins(float diff)
{
	if (coins + diff < 0)
	{
		coins = 0;
	}
	else
	{
		coins += diff;
	}

	std::cout << "COINS: " << coins << std::endl;

	notify();
}

void App::Player::ChangeHealth(int diff)
{
	if (health + diff < 0)
	{
		health = 0;
	}
	else
	{
		health += diff;
	}

	std::cout << "HEALTH: " << health << std::endl;

	notify();
}

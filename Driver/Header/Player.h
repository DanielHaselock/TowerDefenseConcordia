#pragma once
#include "Observer.h"

#define STARTING_COINS 30

/// How many critters may reach the exit before the player loses. Kept as a
/// plain constant (rather than derived/configurable) so it is trivial to
/// retune while the loss condition is being tested.
#define STARTING_HEALTH 10

namespace App
{
	/**
	 * @class Player
	 * @brief Tracks the human player's coin balance and health.
	 *
	 * The player starts with STARTING_COINS coins. Coins are spent buying
	 * and upgrading towers, gained by killing critters, and drained while a
	 * critter idles at the exit (see GameManager::MainLoop()).
	 *
	 * Health is a separate pool from coins: it starts at STARTING_HEALTH and
	 * loses one point the instant a critter reaches the exit (regardless of
	 * the player's coin balance). Reaching zero health means the player has
	 * lost (see IsDefeated()).
	 *
	 * Player is an Observable subject: every ChangeCoins()/ChangeHealth()
	 * notifies all attached Observer<Player> instances (e.g. the on-screen
	 * coin/health counters, see PlayerRenderObserver).
	 */
	class Player : public Game::Observable<Player>
	{
	public:
		/// Creates a player with the starting coin balance (STARTING_COINS) and health (STARTING_HEALTH).
		Player();

		/**
		 * @brief Adjusts the player's coin balance.
		 * @param diff Amount to add to the balance; negative to spend coins,
		 *        positive to award coins.
		 */
		void ChangeCoins(float diff);

		/** @brief Returns the current coin balance. */
		float GetCoins() const { return coins; }

		/**
		 * @brief Adjusts the player's health.
		 * @param diff Amount to add to health; negative when a critter leaks.
		 */
		void ChangeHealth(int diff);

		/** @brief Returns the current health. */
		int GetHealth() const { return health; }

		/** @brief True once health has reached zero. */
		bool IsDefeated() const { return health <= 0; }

	private:
		float coins;
		int health;

	};
}

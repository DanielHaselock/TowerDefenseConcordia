#pragma once
#include <vector>
#include "Critter.h"

#define MAX_WALKER_POOL 50
#define MAX_SPEED_POOL 50
#define MAX_BOSS_POOL 15

/// The wave the player must clear to win. Kept as a plain, easily-edited
/// constant (set low for testing purposes right now). Must stay <= 100: the
/// WaveManager constructor only precomputes stats for waves 1-100.
#define WIN_WAVE_COUNT 5

namespace App
{
	/// How many critters of each type make up a given wave.
	struct WaveStats
	{
		int amountWalker;
		int amountSpeedy;
		int amountBoss;
	};

	/** @brief The phase a wave is currently in. */
	enum WaveState
	{
		waiting = 0,  ///< Between waves; stays here until the player starts the next wave via StartNextWave().
		spawning = 1, ///< Actively activating this wave's critters from the pool, one at a time.
		playing = 2,  ///< Every critter for this wave has been spawned; waiting for them all to die or leak before the next wave.
	};

	/**
	 * @class WaveManager
	 * @brief Drives critter wave spawning over time.
	 *
	 * On construction, pre-computes the walker/speedy/boss composition of
	 * up to 100 waves, with total critter count and the boss/speedy
	 * proportions both increasing with wave number. Each frame, Update()
	 * advances a small state machine (waiting -> spawning -> playing ->
	 * waiting) that activates pooled critters one at a time while
	 * spawning, and waits for the wave to be cleared before starting the
	 * next one. Between waves the machine holds in waiting until the
	 * player starts the next wave with StartNextWave(). WaveManager does
	 * not own the critters: it operates on the shared pool passed in by
	 * GameManager, assumed to be laid out as walkers first, then speedy
	 * critters, then bosses.
	 */
	class WaveManager
	{
	public:

		/**
		 * @brief Precomputes the per-wave critter composition for up to 100 waves.
		 * @param p_critters Shared, pre-allocated pool of critters (not
		 *        owned by WaveManager), laid out as walkers first, then
		 *        speedy critters, then bosses, matching GameManager's
		 *        spawn order.
		 */
		WaveManager(std::vector<Game::Critter*> p_critters);

		/**
		 * @brief Moves on to the next wave.
		 *
		 * Every third wave, also raises the level of every critter in the
		 * pool (used to scale critter strength/speed over time).
		 */
		void IncreaseWave();

		/// Gets the precomputed walker/speedy/boss composition for the current wave.
		WaveStats GetWaveStats();

		/**
		 * @brief Starts the next wave, if the manager is waiting for the player.
		 *
		 * Switches the state machine from waiting to spawning and calls
		 * IncreaseWave(). Ignored in any other state.
		 */
		void StartNextWave();

		/**
		 * @brief Reports whether the manager is holding between waves.
		 * @return True while the machine waits for the player to start the
		 *         next wave; the start button is shown while this holds.
		 */
		bool IsWaitingForPlayer() const;

		/**
		 * @brief Number of the wave currently being played.
		 *
		 * 0 before the first wave has started, 1 during the first wave,
		 * and so on. Used by the tower sell refund to pick the refund
		 * percentage for the current wave (see Game::Tower::GetRefundPercent),
		 * and by the start button to name the wave about to begin.
		 */
		int GetCurrentWave() const;

		/**
		 * @brief Advances the wave state machine by one frame.
		 *
		 * While waiting, does nothing: the machine stays there until the
		 * player calls StartNextWave(). While spawning, periodically
		 * activates the next critter via SpawnCritter(). While playing,
		 * checks whether the wave has been cleared and switches back to
		 * waiting if so.
		 * @param deltatime Time elapsed since the previous frame, in seconds.
		 */
		void Update(float deltatime);

		/// Switches back to the waiting state once CheckWaveEnd() reports the wave is cleared.
		void ShouldEndWave();

		/**
		 * @brief Checks whether every critter in the pool is resolved.
		 *
		 * A critter is resolved once it's dead (isDead()) or has reached the
		 * exit (reachedEnd) - the latter so a wave can be reported cleared
		 * without waiting for a leaked critter's coin drain to finish; it
		 * keeps draining/stays active in the background until GameManager
		 * recycles it.
		 * @return True if every critter in the pool is resolved.
		 */
		bool CheckWaveEnd();

		/**
		 * @brief Activates the next critter for the current wave.
		 *
		 * Picks a random type among those that haven't yet reached their
		 * quota for this wave (per GetWaveStats()), then activates the
		 * next inactive pooled critter of that type. Once every critter
		 * for the wave has been spawned, resets the per-type spawn
		 * counters and switches the wave state to playing.
		 */
		void SpawnCritter();

		/// Returns a uniformly distributed random integer in the inclusive range [a, b].
		int GetRandomNumber(int a, int b);

		friend class WaveManagerTest;

	private:

		int currWave = 0; ///< Index of the current wave into stats (0-based).
		std::vector<WaveStats> stats; ///< Precomputed critter composition for each wave, indexed by currWave.
		std::vector<Game::Critter*> critters; ///< Shared critter pool (not owned), laid out as walkers, then speedy, then bosses.

		WaveState waveState; ///< Current phase of the wave state machine.
		float critterSpawnTime = 0.5f; ///< Seconds to wait between activating consecutive critters while spawning.
		float currSpawnTime = 0;       ///< Elapsed time, in seconds, since the last critter was spawned.

		int amountCritterSpawned = 0; ///< Total critters activated so far for the current wave, across all types.
		int amountWalkerSpawned = 0;  ///< Walker critters activated so far for the current wave; also the index of the next walker to activate in the pool.
		int amountSpeedSpawned = 0;   ///< Speedy critters activated so far for the current wave; also the offset of the next speedy critter to activate in the pool.
		int amountBossSpawned = 0;    ///< Boss critters activated so far for the current wave; also the offset of the next boss to activate in the pool.
	};
}
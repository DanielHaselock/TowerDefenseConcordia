#pragma once

namespace Renderer
{
	class spriteSheetTexture;
}

namespace Game
{
	class Critter;

	/**
	 * @struct BulletRequest
	 * @brief Describes a bullet a Tower wants to fire, before an actual
	 * Bullet object is spawned for it.
	 *
	 * Towers don't create Bullet objects directly; shoot() populates one of
	 * these instead, which the game loop reads via Tower::getBulletRequest()
	 * to spawn the actual bullet, then clears via Tower::processShoot().
	 */
	struct BulletRequest
	{
		int damage; ///< Damage the bullet deals on impact.
		float speed; ///< Travel speed of the bullet, in pixels/second.
		int initalX; ///< Pixel X the bullet should spawn at (the tower's position).
		int initialY; ///< Pixel Y the bullet should spawn at (the tower's position).
		int angle; ///< Initial rotation, in degrees, applied to the bullet sprite.
		Critter* target; ///< Critter the bullet should home in on.
		int effect; ///< Special effect applied on hit: 0 = none, 1 = AoE, 2 = slow.
		float blastRadius; ///< AoE blast radius in grid tiles; 0 for towers without an AoE effect.

		/// Constructs a bullet request with all fields set explicitly.
		BulletRequest(int d, float s, int x, int y, int a, Critter* t, int e, float r = 0.0f)
			: damage(d), speed(s), initalX(x), initialY(y), angle(a), target(t), effect(e), blastRadius(r) {}
	};

	/**
	 * @struct HitRequest
	 * @brief Describes the point of impact of a bullet, for effects (like
	 * area-of-effect damage) that need to act on the location rather than
	 * just the bullet's original target.
	 *
	 * Populated by Bullet::Hit() when an AoE bullet lands; the game loop
	 * reads it via Bullet::GetHitRequest() to apply splash damage around
	 * the impact tile, then clears it via Bullet::DeleteHitRequest().
	 */
	struct HitRequest
	{
		int hitRow; ///< Grid row the bullet hit.
		int hitCol; ///< Grid column the bullet hit.
		float aoeDamage; ///< Damage to apply to critters caught in the blast.
		float blastRadius; ///< Blast radius in grid tiles; critters this close to the impact tile take the damage.

		/// Constructs a hit request at the given grid position with the given damage and blast radius.
		HitRequest(int r, int c, int dmg, float radius = 0.0f) : hitRow(r), hitCol(c), aoeDamage(dmg), blastRadius(radius) {}
	};

	/**
	 * @class Bullet
	 * @brief A single in-flight projectile fired by a Tower.
	 *
	 * A Bullet is spawned from a Tower's BulletRequest, then homes in on
	 * its target critter every frame until it gets close enough to Hit()
	 * it, applying damage and (depending on its effect) a slow or an
	 * area-of-effect HitRequest for the game loop to resolve.
	 */
	class Bullet
	{
		/// Grants the unit tests (BulletTest) access to the protected m_damage /
		/// m_speed members so they can assert the stats stored by Shoot().
		friend class BulletTest;

	public:

		/// Constructs an (initially inactive, hidden) bullet using the given sprite.
		Bullet(::Renderer::spriteSheetTexture* p_texture);

		/**
		 * @brief Activates the bullet and aims it at a target.
		 *
		 * @param p_damage Damage dealt to the target on impact.
		 * @param p_speed Travel speed, in pixels/second.
		 * @param initalX Pixel X to spawn the bullet at.
		 * @param initialY Pixel Y to spawn the bullet at.
		 * @param angle Initial sprite rotation, in degrees.
		 * @param target Critter the bullet should home in on.
		 * @param p_effect Special effect applied on hit: 0 = none, 1 = AoE, 2 = slow.
		 * @param p_blastRadius AoE blast radius in grid tiles (AoE effect only; 0 otherwise).
		 */
		void Shoot(int p_damage, float p_speed, int initalX, int initialY, int angle, Critter* target, int p_effect, float p_blastRadius = 0.0f);

		/**
		 * @brief Moves the bullet toward its target by one frame's worth of
		 * travel, rotating its sprite to face the target.
		 *
		 * Automatically calls Hit() once the bullet is close enough to (or
		 * would overshoot) its target. Does nothing if the bullet is
		 * inactive or has no target.
		 *
		 * @param deltatime Seconds elapsed since the last frame.
		 */
		void Update(float deltatime);

		/**
		 * @brief Resolves the bullet's impact: damages the target, applies
		 * the bullet's effect (halves the target's speed, or queues an AoE
		 * HitRequest), hides the bullet and deactivates it.
		 *
		 * An AoE bullet does not damage its target here: the impact tile
		 * and blast radius travel in the HitRequest, and the game loop
		 * applies the splash damage to every critter caught in the blast,
		 * the target included. That keeps a critter at ground zero from
		 * being hit twice for the same shot.
		 */
		void Hit();

		/// True while the bullet is in flight (active and rendered).
		bool GetActive() { return isActive; }

		/// Pending area-of-effect hit, or nullptr if this bullet has no AoE
		/// effect or hasn't landed yet.
		HitRequest* GetHitRequest() { return hitRequest; }

		/// Frees and clears the pending hit request after it has been handled.
		void DeleteHitRequest();

		/// Destroys the bullet, freeing any pending hit request.
		~Bullet();

	private:
		float m_damage, m_speed; ///< Damage dealt and travel speed of this bullet.
		float m_blastRadius = 0.0f; ///< AoE blast radius in grid tiles, stored by Shoot().
		Critter* m_target = nullptr; ///< Critter this bullet is homing in on.
		HitRequest* hitRequest = nullptr; ///< Pending AoE impact info, set by Hit() when effect == 1.
		int effect = 0; ///< Special effect applied on hit: 0 = none, 1 = AoE, 2 = slow.

		bool isActive = false; ///< True while the bullet is in flight.

		::Renderer::spriteSheetTexture* m_texture; ///< Sprite used to render this bullet.
	};
}
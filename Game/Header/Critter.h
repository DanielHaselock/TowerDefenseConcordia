#ifndef CRITTER_H
#define CRITTER_H

#include <memory>
#include <string>
#include <vector>
#include "Observer.h"

namespace Renderer
{
    class spriteSheetTexture;
    class RendererMap;
}

namespace Game {
    class Cell;

    /**
     * @class Critter
     * @brief Abstract base class representing a critter in the tower defense game.
     *
     * A Critter walks from the map's entry point to its exit point. It can be
     * damaged by towers, dies when its hit points reach zero (granting the
     * player a coin reward), and steals coins from the player if it reaches
     * the exit point. Concrete critter types (Walker, Speedy, Boss, ...)
     * customize the base stats and identify themselves via getType().
     */
    class Critter : public Observable<Critter> {

    private:
        bool isActive = false; ///< True while the critter is spawned and alive on the map; false before spawn and after death/reset.

    protected:
        int hitPoints;      ///< Current hit points.
        int maxHitPoints;    ///< Hit points at full health (used e.g. for UI/health bars).
        int strength;        ///< Coins stolen per "tick" if the critter reaches the exit.
        float speed;            ///< Current movement speed (higher = moves further per turn).
        int baseSpeed;        ///< Speed before any slow effects (used so slows don't stack).
        int level;             ///< Difficulty level, scales stats for later waves.
        int reward;             ///< Coins awarded to the player when this critter dies.

        int row;                 ///< Current row position on the map grid.
        int col;                  ///< Current column position on the map grid.

        float pixelPosX; ///< Current pixel X position on screen, interpolated between path tiles.
        float pixelPosY; ///< Current pixel Y position on screen, interpolated between path tiles.

        float destinationX; ///< Pixel X of the next path tile being walked toward.
        float destinationY; ///< Pixel Y of the next path tile being walked toward.

        std::vector<std::pair<int, int>> positions; ///< Pixel coordinates of each tile along the critter's path, in order.
        std::vector<int> m_correctPath; ///< Grid tile indices making up the critter's path from spawn to exit.
        int m_columns = 0; ///< Number of map columns used to decode row-major path indices.
        int currentPos = 0; ///< Index into positions/m_correctPath of the tile currently being walked toward.
        ::Renderer::spriteSheetTexture* m_texture = nullptr; ///< Sprite used to render this critter.


        //Stats
        float base_speed = 0; ///< Per-type base speed (set by subclasses); used to restore speed once a temporary slow effect ends.

    public:
        /**
         * @brief Constructs a critter with the given base stats and spawn location.
         *
         * @param hitPoints Starting (and max) hit points.
         * @param strength Coins stolen per "tick" if this critter reaches the exit.
         * @param speed Initial movement speed.
         * @param level Difficulty level used to scale stats.
         * @param reward Coins awarded to the player when this critter dies.
         * @param spawnRow Grid row the critter starts at.
         * @param spawnCol Grid column the critter starts at.
         * @param texture Sprite used to render this critter.
         */
        Critter(int hitPoints, int strength, int speed, int level, int reward, int spawnRow, int spawnCol, ::Renderer::spriteSheetTexture* texture);

        /**
         * @brief Assigns the sequence of map tiles this critter will walk along.
         *
         * Converts the tile-index path into pixel positions and initializes the
         * critter's current position, destination and current tile accordingly.
         *
         * @param m_tiles All renderer tiles on the map, indexed by tile number.
         * @param p_correctPath Ordered tile indices from spawn to exit.
         * @param columns Number of columns in the map.
         */
        void setPath(std::vector<::Renderer::RendererMap*> m_tiles, const std::vector<int>& p_correctPath, int columns);

        /// Returns the critter to its initial state (full health, hidden,
        /// back at the start of its path) so it can be reused for a later wave.
        void resetCritter();

        virtual ~Critter() = default;

        /// Applies damage to the critter, clamping hit points at 0.
        virtual void takeDamage(int amount);

        /// True once hit points have reached zero.
        virtual bool isDead() const;

        /// Moves the critter one step. Real pathing depends on the Map (Part 1);
        /// here it just documents the interface each critter must support.
        virtual void Update(float deltaTime, const std::vector<::Renderer::RendererMap*>& m_tiles);

        /// Returns a human-readable type name, e.g. "Walker".
        virtual std::string getType() const = 0;

        /// Polymorphic copy, used by the (future) Critter Group Generator.
        virtual std::unique_ptr<Critter> clone() const = 0;

        // Getters
        int getHitPoints() const;
        /** @brief Returns the maximum health of this critter. */
        int getMaxHitPoints() const;
        /// Coins stolen per tick if this critter reaches the exit.
        int getStrength() const;
        /** @brief Returns the current movement speed. */
        int getSpeed() const;
        /// Speed before any slow effects.
        int getBaseSpeed() const;
        /** @brief Returns the critter difficulty level. */
        int getLevel() const;
        /// Coins awarded to the player when this critter dies.
        int getReward() const;
        /** @brief Returns the current grid row. */
        int getRow() const;
        /** @brief Returns the current grid column. */
        int getCol() const;
        /// Current pixel X on screen, interpolated between path tiles.
        float getPixelPosx() const { return pixelPosX; };
        /// Current pixel Y on screen, interpolated between path tiles.
        float getPixelPosy() const { return pixelPosY; };
        /// True while the critter is spawned and alive on the map.
        bool getIsActive() const { return isActive; }

        /**
         * @brief Distance, in pixels, the critter still has to walk along its path to reach the exit.
         *
         * The straight-line distance to the tile it is walking toward plus the
         * length of every remaining path segment after it, so the value only
         * ever shrinks as the critter advances. Used by the "nearest to exit"
         * targeting strategy. Returns 0 for a critter that has no path set.
         */
        float getDistanceToExit() const;
        // Setters
        void setPosition(int newRow, int newCol);

        /** @brief Updates the critter position in screen pixels. */
        void setPixelPosition(int x, int y);

        /// Sets the pixel coordinates the critter is currently walking toward.
        void setDestination(int x, int y);
        /// Activates or deactivates the critter and notifies observers,
        /// which sync the sprite's visibility (see CritterRenderSyncObserver).
        void setIsActive(bool p_active);

        /// Halves the critter's current speed, if not already slowed, and
        /// resets the slow timer so repeated hits don't stack.
        void halfSpeed();
        bool speedHalfed = false; ///< True while the critter's speed is currently halved by a slow effect.
        float speedHalfTime = 0; ///< Seconds elapsed since the current slow effect was applied.
        float maxSpeedHalfTime = 3; ///< Duration, in seconds, a slow effect lasts before speed is restored.

        /// Rescales this critter's stats (hit points, strength, reward and
        /// the level field itself) for the given difficulty level.
        /// Implemented by each concrete critter type.
        virtual void setLevel(int p_level) = 0;

        /// The sprite this critter is rendered with. Owned by the renderer
        /// (GameRenderer/Window); read-only for the model and its observers.
        ::Renderer::spriteSheetTexture* getTexture() const { return m_texture; }
        /// Replaces the sprite used to render this critter (used to show the bordered variant while selected).
        void setTexture(::Renderer::spriteSheetTexture* texture) { m_texture = texture; }

        ::Renderer::RendererMap* currentTile = nullptr; ///< Map tile the critter currently occupies; used to check for infected-tile damage.

        bool reachedEnd = false; ///< True once the critter has walked past the final tile on its path.
        bool killed = false; ///< True once hit points have reached zero (critter should be removed/rewarded).
        bool changedSquare = false; ///< True for the brief period right after crossing into a new tile, used to avoid re-triggering tile-change logic every frame.

        float timeToDamage = 0; ///< Seconds accumulated while standing on an infected tile, used to apply damage once per second.

        float lastDeltaTime = 0.0f; ///< Elapsed time of the most recent Update() call; read by observers that need to scale a per-second rate (e.g. the exit "steals coins" rate, via getStrength()) into a per-frame amount.
        /// Elapsed time of the most recent Update() call.
        float getLastDeltaTime() const { return lastDeltaTime; }

        /// Called when the critter dies; currently just resets it back to its initial unspawned state.
        void Death();

    };
}

#endif // CRITTER_H

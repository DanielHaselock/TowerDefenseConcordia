#ifndef BOSS_H
#define BOSS_H

#include "Critter.h"

namespace Game {
    /**
     * @class BossCritter
     * @brief Slow, tanky critter with high HP and strength. Dangerous if it
     * reaches the exit point, since it steals coins at a high rate.
     */
    class BossCritter : public Critter {
    public:
        /**
         * @brief Constructs a Boss critter scaled to the given level,
         * spawning at the given grid position.
         *
         * @param level Difficulty level; scales hit points, strength and reward.
         * @param spawnRow Grid row to spawn at.
         * @param spawnCol Grid column to spawn at.
         * @param texture Sprite used to render this critter.
         */
        explicit BossCritter(int level = 1, int spawnRow = 0, int spawnCol = 0, ::Renderer::spriteSheetTexture* texture = nullptr);

        /// Returns "Boss".
        std::string getType() const override;
        /// Polymorphic copy of this Boss critter.
        std::unique_ptr<Critter> clone() const override;
        /// Moves the critter one step along its path (see Critter::Update).
        void Update(float deltaTime, const std::vector<::Renderer::RendererMap*>& m_tiles) override;

        /// Rescales hit points (and max hit points), strength and reward
        /// for the given level.
        void setLevel(int p_level) override;
    };
}

#endif // BOSS_H

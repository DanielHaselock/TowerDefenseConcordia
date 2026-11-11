#ifndef SPEEDY_H
#define SPEEDY_H

#include "Critter.h"

namespace Game {
    /**
     * @class SpeedyCritter
     * @brief Fast, fragile critter. Low HP and strength, high speed — it tries
     * to rush past towers rather than tank damage.
     */
    class SpeedyCritter : public Critter {
    public:
        /**
         * @brief Constructs a Speedy critter scaled to the given level,
         * spawning at the given grid position.
         *
         * @param level Difficulty level; scales hit points, strength and reward.
         * @param spawnRow Grid row to spawn at.
         * @param spawnCol Grid column to spawn at.
         * @param texture Sprite used to render this critter.
         */
        explicit SpeedyCritter(int level = 1, int spawnRow = 0, int spawnCol = 0, ::Renderer::spriteSheetTexture* texture = nullptr);

        /// Returns "Speedy".
        std::string getType() const override;
        /// Polymorphic copy of this Speedy critter.
        std::unique_ptr<Critter> clone() const override;
        /// Moves the critter one step along its path (see Critter::Update).
        void Update(float deltaTime, const std::vector<::Renderer::RendererMap*>& m_tiles) override;

        /// Rescales hit points, strength and reward for the given level,
        /// and ramps base speed (speed ramps up until level 2, then holds
        /// steady), reapplying any currently active slow effect.
        void setLevel(int p_level) override;
    };
}

#endif // SPEEDY_H

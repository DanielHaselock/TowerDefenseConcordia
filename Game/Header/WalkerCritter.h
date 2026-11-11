#ifndef WALKER_H
#define WALKER_H

#include "Critter.h"

namespace Game {
    /**
     * @class WalkerCritter
     * @brief Balanced, "default" critter type. Moderate HP, speed and strength.
     */
    class WalkerCritter : public Critter {
    public:
        /**
         * @brief Constructs a Walker critter scaled to the given level,
         * spawning at the given grid position.
         *
         * @param level Difficulty level; scales hit points, strength and reward.
         * @param spawnRow Grid row to spawn at.
         * @param spawnCol Grid column to spawn at.
         * @param texture Sprite used to render this critter.
         */
        explicit WalkerCritter(int level = 1, int spawnRow = 0, int spawnCol = 0, ::Renderer::spriteSheetTexture* texture = nullptr);

        /// Returns "Walker".
        std::string getType() const override;
        /// Polymorphic copy of this Walker critter.
        std::unique_ptr<Critter> clone() const override;
        /// Moves the critter one step along its path (see Critter::Update).
        void Update(float deltaTime, const std::vector<::Renderer::RendererMap*>& m_tiles) override;

        /// Rescales hit points (and max hit points), strength and reward
        /// for the given level.
        void setLevel(int p_level) override;
    };
}

#endif // WALKER_H

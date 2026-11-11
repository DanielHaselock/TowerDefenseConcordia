#pragma once
#include "Tower.h"

namespace Game
{

    /** @brief The role of a cell in a map. */
    enum class CellType
    {
        Entry,   ///< The critter spawn cell; traversable by critters.
        Scenery, ///< A cell where towers can be placed.
        Path,    ///< A traversable cell where towers cannot be placed.
        Exit     ///< The critter destination cell; traversable by critters.
    };

    /** @brief Stores the type assigned to one map cell. */
    struct Cell
    {
        CellType type; ///< The cell's role in the map.

        /** @brief Creates a cell with the given type. */
        Cell(CellType type) : type(type) {}

        /** @brief Checks whether critters can traverse this cell. */
        bool isPath() const
        {
            return type != CellType::Scenery;
        }

        /** @brief Checks whether a tower can be placed on this cell. */
        bool canPlaceTower() const
        {
            return type == CellType::Scenery && !hasTower;
        }

        bool hasTower = false;

        Tower* tower = nullptr;
    };


} // namespace Game
#pragma once

#include <utility>
#include <vector>
#include "Game.h"
#include "Observer.h"

namespace Game
{

    /**
     * @brief A rectangular grid containing scenery and a path from entry to exit.
     *
     * Maps can start with a random route or as all-scenery layouts for editing.
     * Valid routes require a unique entry cell and a unique exit cell connected
     * by traversable path cells.
     */
    class Map : public Observable<Map>
    {
    public:
        /** @brief Determines whether a new map starts empty or randomly generated. */
        enum class InitialLayout
        {
            Random,
            Scenery
        };

        /** @brief Detailed outcome of validating a map's endpoints and path connectivity. */
        enum class ValidationStatus
        {
            Valid,            ///< Exactly one entry and one exit exist, connected by path cells.
            MissingEntry,     ///< No entry cell has been placed on the map.
            MultipleEntries,  ///< More than one entry cell exists on the map.
            MissingExit,      ///< No exit cell has been placed on the map.
            MultipleExits,    ///< More than one exit cell exists on the map.
            DisconnectedPath  ///< Unique entry and exit exist, but no path connects them.
        };

        /**
         * @brief Creates a map with the requested initial layout.
         * @param rows Number of rows in the map (must be positive)
         * @param cols Number of columns in the map (must be positive)
         * @param initialLayout Random preserves the generated-map behavior;
         *        Scenery creates an editable blank map.
         * @throws std::invalid_argument If either dimension is invalid or the map is 1x1.
         */
        Map(int rows, int cols, InitialLayout initialLayout = InitialLayout::Random);

        /** @brief Gets the number of rows. */
        int getRows() const;

        /** @brief Gets the number of columns. */
        int getCols() const;

        /**
         * @brief Gets a cell by its zero-based row and column.
         * @throws std::out_of_range If the coordinates are outside the map.
         */
        const Cell &getCell(int row, int column) const;

        /** @brief Gets the full grid of cells. */
        std::vector<Cell*> getCells();

        /**
         * @brief Sets the role of a cell while editing a map.
         * @throws std::out_of_range If the coordinates are outside the map.
         *
         * Editing invalidates the previously validated path. Call isValid()
         * before using the map for gameplay.
         */
        void setCellType(int row, int column, CellType type);

        /**
         * @brief Resets every cell on the map to Scenery and notifies observers.
         */
        void clear();

        /**
         * @brief Resizes the map grid to new dimensions and notifies observers.
         * @param newRows New number of rows (must be positive).
         * @param newCols New number of columns (must be positive).
         * @param preserveCells When true, preserves cell types in the overlapping region.
         * @throws std::invalid_argument If either dimension is invalid or the map is 1x1.
         */
        void resize(int newRows, int newCols, bool preserveCells = true);

        /**
         * @brief Validates endpoint uniqueness and BFS path connectivity, returning a diagnostic status.
         * @return ValidationStatus::Valid when playable, or the specific reason validation failed.
         */
        ValidationStatus validate();

        /**
         * @brief Checks endpoint uniqueness and path connectivity using BFS.
         * @return True if both aspects pass, false otherwise.
         *
         * It uses validate() to verify endpoints and populate correctPath.
         */
        bool isValid();

        /**
         * @brief Gets the entry cell's coordinates as (row, column).
         * @throws std::logic_error If the map does not contain exactly one entry and one exit.
         */
        std::pair<int, int> getEntryCell();

        /**
         * @brief Gets the exit cell's coordinates as (row, column).
         * @throws std::logic_error If the map does not contain exactly one entry and one exit.
         */
        std::pair<int, int> getExitCell();

        /**
         * @brief Generates the map with a random connected path.
         * @return A read-only reference to the generated grid.
         *
         * The entry is in the first column and the exit is in the last column.
         */
        const std::vector<std::vector<Cell>> &generateRandomMap();

        /** @brief Prints each cell's enum value (integer value). Each row of the map is on a separate line. */
        void display() const;

        /**
         * @brief Gets the generated entry-to-exit path.
         * @return Reference to the path, stored as a flat list of
         *         row-major cell indices (row * cols + column) in order
         *         from the entry cell to the exit cell.
         */
        std::vector<int>& getCorrectPath() { return correctPath; }

        /**
         * @brief Places a tower on a scenery cell.
         * @return False if the coordinates are not a free scenery cell.
         */
        bool placeTower(Tower* tower, int row, int column);

        /**
         * @brief Removes whichever tower occupies a cell, if any.
         * @return False if the cell did not have a tower.
         */
        bool removeTower(int row, int column);

    private:
        friend class MapTest;

        /** @brief Changes a cell type without notifying observers. */
        void assignCellType(int row, int column, CellType type);

        /** @brief Throws if the supplied row and column are outside the grid. */
        void checkCoordinates(int row, int column) const;

        /**
         * @brief Finds the unique entry and exit cells.
         * @param entry Receives the entry coordinates as (row, column).
         * @param exit Receives the exit coordinates as (row, column).
         * @return True if exactly one entry and one exit exist, false otherwise.
         */
        bool findEntryAndExit(std::pair<int, int> &entry, std::pair<int, int> &exit);

        int rows;
        int cols;

        /** @brief The map grid stored as a 2D vector of cell values. */
        std::vector<std::vector<Cell>> cells;

        std::vector<int> correctPath; ///< Flat row-major indices of the entry-to-exit path, set by isValid().
    };

} 
#include "Map.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>

namespace Game
{
    Map::Map(int rows, int cols, InitialLayout initialLayout)
        : rows(rows),
          cols(cols)
    {
        if (rows <= 0 || cols <= 0)
        {
            throw std::invalid_argument("Map rows and columns must be positive");
        }
        if (rows == 1 && cols == 1)
        {
            throw std::invalid_argument("A map needs at least two cells for distinct entry and exit points");
        }

        switch (initialLayout)
        {
        case InitialLayout::Random:
            generateRandomMap();
            break;
        case InitialLayout::Scenery:
            cells.assign(rows, std::vector<Cell>(cols, Cell(CellType::Scenery)));
            break;
        default:
            throw std::invalid_argument("Unknown initial map layout");
        }
    }

    int Map::getRows() const
    {
        return rows;
    }

    int Map::getCols() const
    {
        return cols;
    }

    const Cell &Map::getCell(int row, int column) const
    {
        checkCoordinates(row, column);
        return cells[row][column];
    }

    std::vector<Cell*> Map::getCells()
    {
        std::vector<Cell*> cells1d;

        for (auto& row : cells)
        {
            for (auto& cell : row)
            {
                cells1d.emplace_back(&cell);
            }
        }

        return cells1d;
    }

    std::pair<int, int> Map::getEntryCell()
    {
        std::pair<int, int> entry;
        std::pair<int, int> exit;
        if (!findEntryAndExit(entry, exit))
        {
            throw std::logic_error("Map must contain exactly one entry and one exit cell");
        }
        return entry;
    }

    std::pair<int, int> Map::getExitCell()
    {
        std::pair<int, int> entry;
        std::pair<int, int> exit;
        if (!findEntryAndExit(entry, exit))
        {
            throw std::logic_error("Map must contain exactly one entry and one exit cell");
        }
        return exit;
    }

    void Map::display() const
    {
        for (int row = 0; row < rows; ++row)
        {
            for (int column = 0; column < cols; ++column)
            {
                const CellType type = cells[row][column].type;
                std::cout << static_cast<int>(type);
                if (column + 1 < cols)
                {
                    std::cout << ' ';
                }
            }
            std::cout << '\n';
        }
    }

    void Map::setCellType(int row, int column, CellType type)
    {
        checkCoordinates(row, column);
        cells[row][column].type = type;
        correctPath.clear();
        notify();
    }

    void Map::clear()
    {
        cells.assign(rows, std::vector<Cell>(cols, Cell(CellType::Scenery)));
        correctPath.clear();
        notify();
    }

    void Map::resize(int newRows, int newCols, bool preserveCells)
    {
        if (newRows <= 0 || newCols <= 0)
        {
            throw std::invalid_argument("Map rows and columns must be positive");
        }
        if (newRows == 1 && newCols == 1)
        {
            throw std::invalid_argument("A map needs at least two cells for distinct entry and exit points");
        }

        std::vector<std::vector<Cell>> newCells(
            newRows,
            std::vector<Cell>(newCols, Cell(CellType::Scenery)));

        if (preserveCells)
        {
            const int commonRows = std::min(rows, newRows);
            const int commonCols = std::min(cols, newCols);
            for (int r = 0; r < commonRows; ++r)
            {
                for (int c = 0; c < commonCols; ++c)
                {
                    newCells[r][c].type = cells[r][c].type;
                }
            }
        }

        rows = newRows;
        cols = newCols;
        cells = std::move(newCells);
        correctPath.clear();
        notify();
    }

    void Map::assignCellType(int row, int column, CellType type)
    {
        checkCoordinates(row, column);
        cells[row][column].type = type;
    }

    void Map::checkCoordinates(int row, int column) const
    {
        if (row < 0 || row >= rows || column < 0 || column >= cols)
        {
            throw std::out_of_range("Map cell coordinates are out of bounds");
        }
    }

    bool Map::findEntryAndExit(std::pair<int, int>& entry,
        std::pair<int, int>& exit)
    {
        int entryCount = 0;
        int exitCount = 0;

        std::pair<int, int> foundEntry;
        std::pair<int, int> foundExit;

        for (int row = 0; row < rows; ++row)
        {
            for (int column = 0; column < cols; ++column)
            {
                if (cells[row][column].type == CellType::Entry)
                {
                    ++entryCount;
                    foundEntry = std::make_pair(row, column);
                }
                else if (cells[row][column].type == CellType::Exit)
                {
                    ++exitCount;
                    foundExit = std::make_pair(row, column);
                }
            }
        }

        if (entryCount != 1 || exitCount != 1)
        {
            return false;
        }

        entry = foundEntry;
        exit = foundExit;

        return true;
    }

    Map::ValidationStatus Map::validate()
    {
        correctPath.clear();

        int entryCount = 0;
        int exitCount = 0;
        std::pair<int, int> entry;
        std::pair<int, int> exit;

        for (int row = 0; row < rows; ++row)
        {
            for (int column = 0; column < cols; ++column)
            {
                if (cells[row][column].type == CellType::Entry)
                {
                    ++entryCount;
                    entry = std::make_pair(row, column);
                }
                else if (cells[row][column].type == CellType::Exit)
                {
                    ++exitCount;
                    exit = std::make_pair(row, column);
                }
            }
        }

        if (entryCount == 0)
        {
            return ValidationStatus::MissingEntry;
        }
        if (entryCount > 1)
        {
            return ValidationStatus::MultipleEntries;
        }
        if (exitCount == 0)
        {
            return ValidationStatus::MissingExit;
        }
        if (exitCount > 1)
        {
            return ValidationStatus::MultipleExits;
        }

        const int entryRow = entry.first;
        const int entryColumn = entry.second;
        const int exitRow = exit.first;
        const int exitColumn = exit.second;

        std::queue<std::pair<int, int>> cellsToVisit;

        std::vector<std::vector<bool>> visited(
            rows,
            std::vector<bool>(cols, false)
        );

        // Stores the previous cell used to reach each cell
        std::vector<std::vector<std::pair<int, int>>> parent(
            rows,
            std::vector<std::pair<int, int>>(
                cols,
                std::make_pair(-1, -1)
            )
        );

        cellsToVisit.push(std::make_pair(entryRow, entryColumn));
        visited[entryRow][entryColumn] = true;

        // Up, down, left, right
        const std::pair<int, int> offsets[] = {
            std::make_pair(-1, 0),
            std::make_pair(1, 0),
            std::make_pair(0, -1),
            std::make_pair(0, 1)
        };

        // Start BFS from the entry cell
        while (!cellsToVisit.empty())
        {
            const std::pair<int, int> current = cellsToVisit.front();
            cellsToVisit.pop();

            // Exit found
            if (current.first == exitRow &&
                current.second == exitColumn)
            {
                std::pair<int, int> currentPathCell = current;

                // Work backwards from exit to entry
                while (currentPathCell != entry)
                {
                    const int index =
                        currentPathCell.first * cols +
                        currentPathCell.second;

                    correctPath.emplace_back(index);

                    currentPathCell =
                        parent[currentPathCell.first][currentPathCell.second];
                }

                // Add entry
                correctPath.emplace_back(
                    entryRow * cols + entryColumn
                );

                // Reverse so path becomes entry -> exit
                std::reverse(
                    correctPath.begin(),
                    correctPath.end()
                );

                return ValidationStatus::Valid;
            }

            for (const std::pair<int, int>& offset : offsets)
            {
                const int nextRow =
                    current.first + offset.first;

                const int nextColumn =
                    current.second + offset.second;

                // Only add valid path cells
                if (nextRow >= 0 && nextRow < rows &&
                    nextColumn >= 0 && nextColumn < cols &&
                    !visited[nextRow][nextColumn] &&
                    cells[nextRow][nextColumn].isPath())
                {
                    visited[nextRow][nextColumn] = true;

                    parent[nextRow][nextColumn] = current;

                    cellsToVisit.push(
                        std::make_pair(nextRow, nextColumn)
                    );
                }
            }
        }

        // No connected path found
        correctPath.clear();
        return ValidationStatus::DisconnectedPath;
    }

    bool Map::isValid()
    {
        return validate() == ValidationStatus::Valid;
    }

    // Restriction: Entry is in the first column and exit is in the last column.
    const std::vector<std::vector<Cell>> &Map::generateRandomMap()
    {
        // seed the pseudorandom generator, which supplies the random row and path choices below
        std::random_device randomDevice;
        std::mt19937 generator(randomDevice());
        std::uniform_int_distribution<int> rowDistribution(0, rows - 1);

        // reset all cells to scenery so this method can also regenerate an existing map.
        cells.assign(rows, std::vector<Cell>(cols, Cell(CellType::Scenery)));

        const int entryRow = rowDistribution(generator);
        const int entryColumn = 0;
        int exitRow = rowDistribution(generator);
        if (cols == 1)
        {
            // (rows - 2) prevents out of bounds error if entry and exit cells are the same
            std::uniform_int_distribution<int> exitRowDistribution(0, rows - 2);
            exitRow = exitRowDistribution(generator);
            // exit and entry cannot be in the same row and same col
            if (exitRow == entryRow)
            {
                ++exitRow;
            }
        }
        const int exitColumn = cols - 1;

        assignCellType(entryRow, entryColumn, CellType::Entry);

        // start from entry and randomly choose vertical or horizontal moves when both are possible
        int currentRow = entryRow;
        int currentColumn = entryColumn;
        // continue moving until we reach the exit cell
        while (currentRow != exitRow || currentColumn != exitColumn)
        {
            const bool canMoveVertically = currentRow != exitRow;
            const bool canMoveHorizontally = currentColumn != exitColumn;
            bool moveVertically = canMoveVertically;

            if (canMoveVertically && canMoveHorizontally)
            {
                // choose vertical with 50% probability, otherwise choose horizontal
                std::bernoulli_distribution chooseVertical(0.5);
                moveVertically = chooseVertical(generator);
            }

            if (moveVertically)
            {
                currentRow += (exitRow > currentRow) ? 1 : -1;
            }
            else
            {
                currentColumn += (exitColumn > currentColumn) ? 1 : -1;
            }

            if (currentRow == exitRow && currentColumn == exitColumn)
            {
                assignCellType(currentRow, currentColumn, CellType::Exit);
            }
            else
            {
                assignCellType(currentRow, currentColumn, CellType::Path);
            }
        }

        // safety check before we return the cells
        if (!isValid())
        {
            throw std::logic_error("Random map generation produced an invalid map");
        }

        notify();

        return cells;
    }

    bool Map::placeTower(Tower* tower, int row, int column)
    {
        checkCoordinates(row, column);
        Cell& cell = cells[row][column];

        if (!cell.canPlaceTower())
        {
            return false;
        }

        cell.hasTower = true;
        cell.tower = tower;
        notify();
        return true;
    }

    bool Map::removeTower(int row, int column)
    {
        checkCoordinates(row, column);
        Cell& cell = cells[row][column];

        if (!cell.hasTower)
        {
            return false;
        }

        cell.hasTower = false;
        cell.tower = nullptr;
        notify();
        return true;
    }

}
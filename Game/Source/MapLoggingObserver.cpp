#include "MapLoggingObserver.h"
#include <iostream>

namespace Game {

    void MapLoggingObserver::update(const Map& map)
    {
        std::cout << "[MapLoggingObserver] Map changed:\n";

        for (int row = 0; row < map.getRows(); ++row)
        {
            for (int column = 0; column < map.getCols(); ++column)
            {
                const Cell& cell = map.getCell(row, column);
                char symbol;

                if (cell.hasTower)
                {
                    symbol = 'T';
                }
                else
                {
                    switch (cell.type)
                    {
                    case CellType::Entry:
                        symbol = 'E';
                        break;
                    case CellType::Exit:
                        symbol = 'X';
                        break;
                    case CellType::Path:
                        symbol = '.';
                        break;
                    default:
                        symbol = '#';
                        break;
                    }
                }

                std::cout << symbol << ' ';
            }
            std::cout << '\n';
        }
    }

} // namespace Game

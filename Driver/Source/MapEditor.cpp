#include "MapEditor.h"
#include "MapGenerator.h"
#include "PixelFont.h"
#include "UiTheme.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

using ::Renderer::UiTheme::fillRect;
using ::Renderer::UiTheme::pointInRect;
using ::Renderer::UiTheme::strokeRect;

namespace
{
    constexpr int TOP_BAR_H = 38;
    constexpr int SIDEBAR_W = 164;
    constexpr int BOTTOM_BAR_H = 30;

    constexpr SDL_Rect STATUS_PILL_RECT{144, 6, 486, 26};

    constexpr SDL_Rect ROWS_MINUS_RECT{44, 60, 24, 24};
    constexpr SDL_Rect ROWS_INPUT_RECT{71, 60, 54, 24};
    constexpr SDL_Rect ROWS_PLUS_RECT{128, 60, 24, 24};

    constexpr SDL_Rect COLS_MINUS_RECT{44, 90, 24, 24};
    constexpr SDL_Rect COLS_INPUT_RECT{71, 90, 54, 24};
    constexpr SDL_Rect COLS_PLUS_RECT{128, 90, 24, 24};

    constexpr SDL_Rect RANDOM_BTN_RECT{10, 152, 144, 28};
    constexpr SDL_Rect CLEAR_BTN_RECT{10, 185, 144, 26};

    constexpr SDL_Rect BRUSH_RECTS[4] = {
        {10, 239, 144, 34}, // Entry
        {10, 278, 144, 34}, // Exit
        {10, 317, 144, 34}, // Path
        {10, 356, 144, 34}  // Scenery
    };

    constexpr Game::CellType BRUSH_TYPES[4] = {
        Game::CellType::Entry,
        Game::CellType::Exit,
        Game::CellType::Path,
        Game::CellType::Scenery
    };

    const char* const BRUSH_LABELS[4] = {
        "[1] ENTRY",
        "[2] EXIT",
        "[3] PATH",
        "[4] SCENERY"
    };

    constexpr SDL_Rect PLAY_BTN_RECT{10, WINDOW_HEIGHT - 76, 144, 64};

    int groundSpriteXFor(Game::CellType type)
    {
        switch (type)
        {
        case Game::CellType::Entry:   return 383;
        case Game::CellType::Scenery: return 319;
        case Game::CellType::Path:    return 255;
        case Game::CellType::Exit:    return 446;
        }
        return 319;
    }

    SDL_Color fallbackTileColor(Game::CellType type)
    {
        switch (type)
        {
        case Game::CellType::Entry:   return {120, 146, 156, 255};
        case Game::CellType::Scenery: return {46, 178, 102, 255};
        case Game::CellType::Path:    return {184, 128, 72, 255};
        case Game::CellType::Exit:    return {222, 202, 164, 255};
        }
        return {46, 178, 102, 255};
    }

    const char* cellTypeName(Game::CellType type)
    {
        switch (type)
        {
        case Game::CellType::Entry:   return "ENTRY";
        case Game::CellType::Scenery: return "SCENERY";
        case Game::CellType::Path:    return "PATH";
        case Game::CellType::Exit:    return "EXIT";
        }
        return "SCENERY";
    }

    int digitFromKey(SDL_Keycode key)
    {
        if (key >= SDLK_0 && key <= SDLK_9)
        {
            return static_cast<int>(key - SDLK_0);
        }
        switch (key)
        {
        case SDLK_KP_0: return 0;
        case SDLK_KP_1: return 1;
        case SDLK_KP_2: return 2;
        case SDLK_KP_3: return 3;
        case SDLK_KP_4: return 4;
        case SDLK_KP_5: return 5;
        case SDLK_KP_6: return 6;
        case SDLK_KP_7: return 7;
        case SDLK_KP_8: return 8;
        case SDLK_KP_9: return 9;
        default:        return -1;
        }
    }
}

namespace App
{
    MapEditor::MapEditor(
        ::Renderer::Window& window,
        InputManager* p_inputManager,
        int initialRows,
        int initialCols)
        : window(window),
          inputManager(p_inputManager)
    {
        initialRows = std::clamp(initialRows, MIN_DIMENSION, MAX_DIMENSION);
        initialCols = std::clamp(initialCols, MIN_DIMENSION, MAX_DIMENSION);

        map = std::make_unique<Game::Map>(
            initialRows,
            initialCols,
            Game::Map::InitialLayout::Scenery);

        loggingObserver = std::make_unique<Game::MapLoggingObserver>(*map);
        syncObserver = std::make_unique<EditorMapObserver>(*map, *this);

        // Load the shared ground tile sheet (kept hidden on Window so only
        // EditorCanvas draws tile sub-rects dynamically).
        tileSheetTexture = window.addTexture(
            "Assets/smallSpriteSheetV2.png",
            Math::Vec4<int>(),
            Math::Vec3<int>(32, 32, 32));
        tileSheetTexture->setRender(false);

        canvas = std::make_unique<EditorCanvas>(*this);
        window.attachRenderable(canvas.get());

        if (inputManager != nullptr)
        {
            inputManager->AttachClickEvent([this]() { HandleMousePress(); });
            inputManager->AttachMoveEvent([this]() { HandleMouseMove(); });
            inputManager->AttachReleaseEvent([this]() { HandleMouseRelease(); });
        }

        refreshValidation();
    }

    MapEditor::~MapEditor()
    {
        window.detachRenderable(canvas.get());
        syncObserver.reset();
        loggingObserver.reset();
    }

    void MapEditor::MainLoop(float deltaTime)
    {
        cursorBlinkTimer += deltaTime;
        if (cursorBlinkTimer >= 1.0f)
        {
            cursorBlinkTimer -= 1.0f;
        }

        if (invalidFlashTimer > 0.0f)
        {
            invalidFlashTimer = std::max(0.0f, invalidFlashTimer - deltaTime);
        }
    }

    void MapEditor::onMapChanged()
    {
        refreshValidation();
    }

    void MapEditor::refreshValidation()
    {
        if (!map)
        {
            return;
        }

        validationStatus = map->validate();
        const int totalCells = map->getRows() * map->getCols();
        onCorrectPathMask.assign(totalCells, false);

        const std::vector<int>& path = map->getCorrectPath();
        validatedPathLength = static_cast<int>(path.size());
        for (int idx : path)
        {
            if (idx >= 0 && idx < totalCells)
            {
                onCorrectPathMask[idx] = true;
            }
        }
    }

    bool MapEditor::setDimensions(int newRows, int newCols)
    {
        newRows = std::clamp(newRows, MIN_DIMENSION, MAX_DIMENSION);
        newCols = std::clamp(newCols, MIN_DIMENSION, MAX_DIMENSION);

        if (newRows == map->getRows() && newCols == map->getCols())
        {
            return false;
        }

        map->resize(newRows, newCols, true);

        if (hoverRow >= map->getRows() || hoverCol >= map->getCols())
        {
            hoverRow = -1;
            hoverCol = -1;
        }
        return true;
    }

    void MapEditor::commitFocusedField()
    {
        if (focusedField == ActiveField::None)
        {
            return;
        }

        const ActiveField field = focusedField;
        focusedField = ActiveField::None;

        if (typedDigits.empty())
        {
            return;
        }

        int parsed = 0;
        try
        {
            parsed = std::stoi(typedDigits);
        }
        catch (...)
        {
            typedDigits.clear();
            return;
        }
        typedDigits.clear();

        parsed = std::clamp(parsed, MIN_DIMENSION, MAX_DIMENSION);
        if (field == ActiveField::Rows)
        {
            setDimensions(parsed, map->getCols());
        }
        else if (field == ActiveField::Cols)
        {
            setDimensions(map->getRows(), parsed);
        }
    }

    void MapEditor::createRandomMap()
    {
        commitFocusedField();
        map->generateRandomMap();
    }

    void MapEditor::clearCustomMap()
    {
        commitFocusedField();
        map->clear();
    }

    void MapEditor::selectBrush(Game::CellType brush)
    {
        selectedBrush = brush;
    }

    void MapEditor::paintCell(int row, int col, Game::CellType type)
    {
        if (row < 0 || row >= map->getRows() || col < 0 || col >= map->getCols())
        {
            return;
        }

        const Game::CellType current = map->getCell(row, col).type;

        if (type == Game::CellType::Entry || type == Game::CellType::Exit)
        {
            if (current == type)
            {
                map->setCellType(row, col, Game::CellType::Scenery);
                return;
            }

            // Relocate any previous endpoint of the same kind so moving Entry or Exit
            // takes a single click.
            for (int r = 0; r < map->getRows(); ++r)
            {
                for (int c = 0; c < map->getCols(); ++c)
                {
                    if ((r != row || c != col) && map->getCell(r, c).type == type)
                    {
                        map->setCellType(r, c, Game::CellType::Scenery);
                    }
                }
            }

            map->setCellType(row, col, type);

            // Helpful workflow progression: after placing Entry, if Exit is not yet
            // placed, keep Entry or advance cleanly? Keeping the user's selected brush
            // predictable avoids surprises, while keyboard [1]-[4] or brush buttons remain instant.
            return;
        }

        if (current != type)
        {
            map->setCellType(row, col, type);
        }
    }

    bool MapEditor::tryStartGame()
    {
        commitFocusedField();
        refreshValidation();
        if (validationStatus == Game::Map::ValidationStatus::Valid)
        {
            readyToPlay = true;
            return true;
        }

        invalidFlashTimer = 0.65f;
        return false;
    }

    std::unique_ptr<Game::Map> MapEditor::takeMap()
    {
        if (!map || !map->isValid())
        {
            throw std::logic_error("Cannot take an invalid or null map from MapEditor");
        }

        window.detachRenderable(canvas.get());
        syncObserver.reset();
        loggingObserver.reset();
        readyToPlay = false;
        return std::move(map);
    }

    MapEditor::GridLayout MapEditor::computeGridLayout() const
    {
        constexpr int padding = 8;
        const int canvasX = SIDEBAR_W + padding;
        const int canvasY = TOP_BAR_H + padding;
        const int canvasW = WINDOW_WIDTH - SIDEBAR_W - 2 * padding;
        const int canvasH = WINDOW_HEIGHT - TOP_BAR_H - BOTTOM_BAR_H - 2 * padding;

        const int r = std::max(1, map->getRows());
        const int c = std::max(1, map->getCols());

        const int tileSize = std::max(1, std::min(canvasW / c, canvasH / r));
        const int tileW = tileSize;
        const int tileH = tileSize;
        const int totalW = tileW * c;
        const int totalH = tileH * r;

        GridLayout layout;
        layout.tileW = tileW;
        layout.tileH = tileH;
        layout.totalW = totalW;
        layout.totalH = totalH;
        layout.originX = canvasX + (canvasW - totalW) / 2;
        layout.originY = canvasY + (canvasH - totalH) / 2;
        return layout;
    }

    bool MapEditor::screenToCell(int screenX, int screenY, int& outRow, int& outCol) const
    {
        const GridLayout layout = computeGridLayout();
        if (screenX < layout.originX || screenX >= layout.originX + layout.totalW ||
            screenY < layout.originY || screenY >= layout.originY + layout.totalH)
        {
            return false;
        }

        outCol = (screenX - layout.originX) / layout.tileW;
        outRow = (screenY - layout.originY) / layout.tileH;
        return outRow >= 0 && outRow < map->getRows() &&
               outCol >= 0 && outCol < map->getCols();
    }

    void MapEditor::HandleMousePress()
    {
        mouseX = window.m_mouse.getX();
        mouseY = window.m_mouse.getY();
        const Uint8 button = window.m_mouse.getButton();

        // Check numeric input boxes first before committing focus.
        if (button == SDL_BUTTON_LEFT)
        {
            if (pointInRect(mouseX, mouseY, ROWS_INPUT_RECT))
            {
                if (focusedField != ActiveField::Rows)
                {
                    commitFocusedField();
                    focusedField = ActiveField::Rows;
                    typedDigits.clear();
                    cursorBlinkTimer = 0.0f;
                }
                return;
            }
            if (pointInRect(mouseX, mouseY, COLS_INPUT_RECT))
            {
                if (focusedField != ActiveField::Cols)
                {
                    commitFocusedField();
                    focusedField = ActiveField::Cols;
                    typedDigits.clear();
                    cursorBlinkTimer = 0.0f;
                }
                return;
            }
        }

        // Clicking anywhere outside the focused input commits its pending value.
        commitFocusedField();

        if (button == SDL_BUTTON_LEFT)
        {
            if (pointInRect(mouseX, mouseY, ROWS_MINUS_RECT))
            {
                if (map->getRows() > MIN_DIMENSION)
                {
                    setDimensions(map->getRows() - 1, map->getCols());
                }
                return;
            }
            if (pointInRect(mouseX, mouseY, ROWS_PLUS_RECT))
            {
                if (map->getRows() < MAX_DIMENSION)
                {
                    setDimensions(map->getRows() + 1, map->getCols());
                }
                return;
            }
            if (pointInRect(mouseX, mouseY, COLS_MINUS_RECT))
            {
                if (map->getCols() > MIN_DIMENSION)
                {
                    setDimensions(map->getRows(), map->getCols() - 1);
                }
                return;
            }
            if (pointInRect(mouseX, mouseY, COLS_PLUS_RECT))
            {
                if (map->getCols() < MAX_DIMENSION)
                {
                    setDimensions(map->getRows(), map->getCols() + 1);
                }
                return;
            }

            if (pointInRect(mouseX, mouseY, RANDOM_BTN_RECT))
            {
                createRandomMap();
                return;
            }
            if (pointInRect(mouseX, mouseY, CLEAR_BTN_RECT))
            {
                clearCustomMap();
                return;
            }

            for (int i = 0; i < 4; ++i)
            {
                if (pointInRect(mouseX, mouseY, BRUSH_RECTS[i]))
                {
                    selectBrush(BRUSH_TYPES[i]);
                    return;
                }
            }

            if (pointInRect(mouseX, mouseY, PLAY_BTN_RECT))
            {
                tryStartGame();
                return;
            }
        }

        int cellRow = -1;
        int cellCol = -1;
        if (screenToCell(mouseX, mouseY, cellRow, cellCol))
        {
            hoverRow = cellRow;
            hoverCol = cellCol;
            isPainting = true;
            paintModeErase = (button == SDL_BUTTON_RIGHT);

            if (paintModeErase)
            {
                paintCell(cellRow, cellCol, Game::CellType::Scenery);
            }
            else
            {
                paintCell(cellRow, cellCol, selectedBrush);
            }
        }
    }

    void MapEditor::HandleMouseMove()
    {
        mouseX = window.m_mouse.getX();
        mouseY = window.m_mouse.getY();

        int cellRow = -1;
        int cellCol = -1;
        if (screenToCell(mouseX, mouseY, cellRow, cellCol))
        {
            hoverRow = cellRow;
            hoverCol = cellCol;

            if (isPainting && window.m_mouse.isPressed())
            {
                if (paintModeErase)
                {
                    paintCell(cellRow, cellCol, Game::CellType::Scenery);
                }
                else if (selectedBrush == Game::CellType::Path ||
                         selectedBrush == Game::CellType::Scenery)
                {
                    paintCell(cellRow, cellCol, selectedBrush);
                }
            }
        }
        else
        {
            hoverRow = -1;
            hoverCol = -1;
        }
    }

    void MapEditor::HandleMouseRelease()
    {
        isPainting = false;
        paintModeErase = false;
    }

    void MapEditor::HandleKeyboard(::Renderer::Window& win)
    {
        for (SDL_Keycode key : win.keysPressedDown)
        {
            if (focusedField != ActiveField::None)
            {
                const int digit = digitFromKey(key);
                if (digit >= 0)
                {
                    if (typedDigits.size() < 2)
                    {
                        if (!(typedDigits.empty() && digit == 0))
                        {
                            typedDigits.push_back(static_cast<char>('0' + digit));
                        }
                    }
                    continue;
                }

                if (key == SDLK_BACKSPACE)
                {
                    if (!typedDigits.empty())
                    {
                        typedDigits.pop_back();
                    }
                    continue;
                }

                if (key == SDLK_TAB)
                {
                    const ActiveField next =
                        (focusedField == ActiveField::Rows) ? ActiveField::Cols : ActiveField::Rows;
                    commitFocusedField();
                    focusedField = next;
                    typedDigits.clear();
                    cursorBlinkTimer = 0.0f;
                    continue;
                }

                if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
                {
                    commitFocusedField();
                    continue;
                }

                if (key == SDLK_UP)
                {
                    const ActiveField current = focusedField;
                    commitFocusedField();
                    if (current == ActiveField::Rows && map->getRows() < MAX_DIMENSION)
                    {
                        setDimensions(map->getRows() + 1, map->getCols());
                    }
                    else if (current == ActiveField::Cols && map->getCols() < MAX_DIMENSION)
                    {
                        setDimensions(map->getRows(), map->getCols() + 1);
                    }
                    focusedField = current;
                    continue;
                }

                if (key == SDLK_DOWN)
                {
                    const ActiveField current = focusedField;
                    commitFocusedField();
                    if (current == ActiveField::Rows && map->getRows() > MIN_DIMENSION)
                    {
                        setDimensions(map->getRows() - 1, map->getCols());
                    }
                    else if (current == ActiveField::Cols && map->getCols() > MIN_DIMENSION)
                    {
                        setDimensions(map->getRows(), map->getCols() - 1);
                    }
                    focusedField = current;
                    continue;
                }
            }

            // Global shortcuts when not typing digits into a size field
            switch (key)
            {
            case SDLK_1:
            case SDLK_KP_1:
                selectBrush(Game::CellType::Entry);
                break;
            case SDLK_2:
            case SDLK_KP_2:
                selectBrush(Game::CellType::Exit);
                break;
            case SDLK_3:
            case SDLK_KP_3:
                selectBrush(Game::CellType::Path);
                break;
            case SDLK_4:
            case SDLK_KP_4:
                selectBrush(Game::CellType::Scenery);
                break;
            case SDLK_r:
                createRandomMap();
                break;
            case SDLK_c:
                clearCustomMap();
                break;
            case SDLK_TAB:
                focusedField = ActiveField::Rows;
                typedDigits.clear();
                cursorBlinkTimer = 0.0f;
                break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                tryStartGame();
                break;
            default:
                break;
            }
        }
    }

    std::string MapEditor::statusBannerText() const
    {
        switch (validationStatus)
        {
        case Game::Map::ValidationStatus::Valid:
            return "VALID MAP (" + std::to_string(validatedPathLength) + " PATH CELLS) - READY TO PLAY!";
        case Game::Map::ValidationStatus::MissingEntry:
            return "INVALID: PLACE AN ENTRY POINT [1]";
        case Game::Map::ValidationStatus::MultipleEntries:
            return "INVALID: ONLY ONE ENTRY POINT ALLOWED";
        case Game::Map::ValidationStatus::MissingExit:
            return "INVALID: PLACE AN EXIT POINT [2]";
        case Game::Map::ValidationStatus::MultipleExits:
            return "INVALID: ONLY ONE EXIT POINT ALLOWED";
        case Game::Map::ValidationStatus::DisconnectedPath:
            return "INVALID: CONNECT ENTRY TO EXIT WITH PATH [3]";
        }
        return "";
    }

    void MapEditor::EditorCanvas::Render(::SDL_Renderer* renderer)
    {
        if (renderer == nullptr || !editor.map)
        {
            return;
        }

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        // 1. Canvas & panel backgrounds
        fillRect(renderer, {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT}, {20, 29, 38, 255});
        fillRect(renderer, {0, 0, WINDOW_WIDTH, TOP_BAR_H}, {31, 45, 58, 255});
        fillRect(renderer, {0, TOP_BAR_H, SIDEBAR_W, WINDOW_HEIGHT - TOP_BAR_H}, {33, 47, 60, 255});
        fillRect(
            renderer,
            {SIDEBAR_W, WINDOW_HEIGHT - BOTTOM_BAR_H, WINDOW_WIDTH - SIDEBAR_W, BOTTOM_BAR_H},
            {26, 38, 49, 255});

        // Subtle panel dividers
        fillRect(renderer, {0, TOP_BAR_H - 1, WINDOW_WIDTH, 1}, {54, 76, 96, 255});
        fillRect(renderer, {SIDEBAR_W - 1, TOP_BAR_H, 1, WINDOW_HEIGHT - TOP_BAR_H}, {54, 76, 96, 255});
        fillRect(
            renderer,
            {SIDEBAR_W, WINDOW_HEIGHT - BOTTOM_BAR_H, WINDOW_WIDTH - SIDEBAR_W, 1},
            {48, 68, 86, 255});

        // 2. Top Header Bar & Live Validation Banner
        ::Renderer::PixelFont::drawText(
            renderer, "MAP EDITOR", 12, 12, 2, {255, 215, 0, 255});

        const bool isValid = (editor.validationStatus == Game::Map::ValidationStatus::Valid);
        const bool flashingInvalid = (!isValid && editor.invalidFlashTimer > 0.0f);

        SDL_Color pillBg = isValid
            ? SDL_Color{24, 72, 48, 255}
            : (flashingInvalid ? SDL_Color{132, 34, 42, 255} : SDL_Color{68, 30, 36, 255});
        SDL_Color pillBorder = isValid
            ? SDL_Color{72, 218, 126, 255}
            : (flashingInvalid ? SDL_Color{255, 130, 130, 255} : SDL_Color{214, 86, 92, 255});
        SDL_Color pillText = isValid
            ? SDL_Color{185, 255, 210, 255}
            : SDL_Color{255, 215, 215, 255};

        fillRect(renderer, STATUS_PILL_RECT, pillBg);
        strokeRect(renderer, STATUS_PILL_RECT, pillBorder, flashingInvalid ? 2 : 1);
        ::Renderer::PixelFont::drawCenteredText(
            renderer, editor.statusBannerText(), STATUS_PILL_RECT, 1, pillText);

        // 3. Left Sidebar: Map Size Section
        const SDL_Color sectionColor{156, 184, 206, 255};
        ::Renderer::PixelFont::drawText(renderer, "MAP SIZE (2-12)", 10, 46, 1, sectionColor);

        auto drawStepButton = [&](const SDL_Rect& rect, const char* symbol, bool enabled)
        {
            const bool hovered = enabled && pointInRect(editor.mouseX, editor.mouseY, rect);
            SDL_Color bg = !enabled
                ? SDL_Color{38, 48, 58, 255}
                : (hovered ? SDL_Color{68, 94, 118, 255} : SDL_Color{48, 68, 86, 255});
            SDL_Color border = !enabled
                ? SDL_Color{62, 76, 90, 255}
                : SDL_Color{96, 128, 158, 255};
            SDL_Color fg = !enabled
                ? SDL_Color{96, 110, 124, 255}
                : SDL_Color{240, 246, 252, 255};

            fillRect(renderer, rect, bg);
            strokeRect(renderer, rect, border, 1);
            ::Renderer::PixelFont::drawCenteredText(renderer, symbol, rect, 2, fg);
        };

        auto drawNumberBox = [&](const SDL_Rect& rect, int currentValue, ActiveField field)
        {
            const bool focused = (editor.focusedField == field);
            const bool hovered = pointInRect(editor.mouseX, editor.mouseY, rect);

            SDL_Color bg = focused
                ? SDL_Color{20, 32, 44, 255}
                : (hovered ? SDL_Color{32, 46, 60, 255} : SDL_Color{24, 36, 48, 255});
            SDL_Color border = focused
                ? SDL_Color{255, 215, 0, 255}
                : (hovered ? SDL_Color{130, 168, 200, 255} : SDL_Color{82, 110, 136, 255});

            fillRect(renderer, rect, bg);
            strokeRect(renderer, rect, border, focused ? 2 : 1);

            std::string shown = (focused && !editor.typedDigits.empty())
                ? editor.typedDigits
                : std::to_string(currentValue);

            if (focused && editor.cursorBlinkTimer < 0.55f)
            {
                shown += "_";
            }

            SDL_Color textColor = (focused && editor.typedDigits.empty())
                ? SDL_Color{180, 204, 224, 255}
                : SDL_Color{255, 232, 120, 255};
            ::Renderer::PixelFont::drawCenteredText(renderer, shown, rect, 2, textColor);
        };

        const int rows = editor.map->getRows();
        const int cols = editor.map->getCols();

        ::Renderer::PixelFont::drawText(renderer, "ROWS", 10, 69, 1, {230, 238, 246, 255});
        drawStepButton(ROWS_MINUS_RECT, "-", rows > MIN_DIMENSION);
        drawNumberBox(ROWS_INPUT_RECT, rows, ActiveField::Rows);
        drawStepButton(ROWS_PLUS_RECT, "+", rows < MAX_DIMENSION);

        ::Renderer::PixelFont::drawText(renderer, "COLS", 10, 99, 1, {230, 238, 246, 255});
        drawStepButton(COLS_MINUS_RECT, "-", cols > MIN_DIMENSION);
        drawNumberBox(COLS_INPUT_RECT, cols, ActiveField::Cols);
        drawStepButton(COLS_PLUS_RECT, "+", cols < MAX_DIMENSION);

        ::Renderer::PixelFont::drawText(
            renderer, "CLICK BOX TO TYPE", 24, 120, 1, {124, 150, 172, 255});

        fillRect(renderer, {10, 133, 144, 1}, {50, 70, 88, 255});

        // 4. Generator / Reset Section
        ::Renderer::PixelFont::drawText(renderer, "GENERATOR", 10, 140, 1, sectionColor);

        {
            const bool hoverRandom = pointInRect(editor.mouseX, editor.mouseY, RANDOM_BTN_RECT);
            fillRect(
                renderer,
                RANDOM_BTN_RECT,
                hoverRandom ? SDL_Color{56, 122, 174, 255} : SDL_Color{42, 96, 140, 255});
            strokeRect(renderer, RANDOM_BTN_RECT, {108, 184, 238, 255}, 1);
            ::Renderer::PixelFont::drawCenteredText(
                renderer, "RANDOM MAP [R]", RANDOM_BTN_RECT, 1, {245, 250, 255, 255});
        }

        {
            const bool hoverClear = pointInRect(editor.mouseX, editor.mouseY, CLEAR_BTN_RECT);
            fillRect(
                renderer,
                CLEAR_BTN_RECT,
                hoverClear ? SDL_Color{68, 88, 108, 255} : SDL_Color{50, 66, 82, 255});
            strokeRect(renderer, CLEAR_BTN_RECT, {104, 128, 152, 255}, 1);
            ::Renderer::PixelFont::drawCenteredText(
                renderer, "CLEAR GRID [C]", CLEAR_BTN_RECT, 1, {226, 236, 245, 255});
        }

        fillRect(renderer, {10, 219, 144, 1}, {50, 70, 88, 255});

        // 5. Brush Palette Section
        ::Renderer::PixelFont::drawText(renderer, "CELL TYPES", 10, 226, 1, sectionColor);

        SDL_Texture* rawSheet =
            (editor.tileSheetTexture != nullptr) ? editor.tileSheetTexture->getRawTexture() : nullptr;

        auto drawCellVisual = [&](const SDL_Rect& dst, Game::CellType type, bool showOnRoute)
        {
            if (rawSheet != nullptr)
            {
                SDL_Rect src{groundSpriteXFor(type), 0, 64, 64};
                SDL_RenderCopy(renderer, rawSheet, &src, &dst);
            }
            else
            {
                fillRect(renderer, dst, fallbackTileColor(type));
            }

            if (type == Game::CellType::Entry)
            {
                strokeRect(renderer, dst, {46, 232, 126, 255}, std::min(2, std::max(1, dst.w / 8)));
                if (dst.w >= 8 && dst.h >= 9)
                {
                    const int sc = (dst.w >= 22 && dst.h >= 22) ? 2 : 1;
                    const int bw = std::min(dst.w - 2, sc * 7 + 4);
                    const int bh = std::min(dst.h - 2, sc * 7 + 4);
                    SDL_Rect badge{dst.x + (dst.w - bw) / 2, dst.y + (dst.h - bh) / 2, bw, bh};
                    fillRect(renderer, badge, {18, 112, 58, 220});
                    ::Renderer::PixelFont::drawCenteredText(
                        renderer, "E", dst, sc, {255, 255, 255, 255});
                }
            }
            else if (type == Game::CellType::Exit)
            {
                strokeRect(renderer, dst, {245, 92, 92, 255}, std::min(2, std::max(1, dst.w / 8)));
                if (dst.w >= 8 && dst.h >= 9)
                {
                    const int sc = (dst.w >= 22 && dst.h >= 22) ? 2 : 1;
                    const int bw = std::min(dst.w - 2, sc * 7 + 4);
                    const int bh = std::min(dst.h - 2, sc * 7 + 4);
                    SDL_Rect badge{dst.x + (dst.w - bw) / 2, dst.y + (dst.h - bh) / 2, bw, bh};
                    fillRect(renderer, badge, {156, 36, 42, 220});
                    ::Renderer::PixelFont::drawCenteredText(
                        renderer, "X", dst, sc, {255, 255, 255, 255});
                }
            }
            else if (type == Game::CellType::Path && showOnRoute)
            {
                const int dotSize = std::clamp(std::min(dst.w, dst.h) / 4, 2, 8);
                SDL_Rect dot{
                    dst.x + (dst.w - dotSize) / 2,
                    dst.y + (dst.h - dotSize) / 2,
                    dotSize,
                    dotSize
                };
                fillRect(renderer, dot, {255, 225, 90, 220});
            }
        };

        for (int i = 0; i < 4; ++i)
        {
            const SDL_Rect& rect = BRUSH_RECTS[i];
            const Game::CellType brushType = BRUSH_TYPES[i];
            const bool active = (editor.selectedBrush == brushType);
            const bool hovered = pointInRect(editor.mouseX, editor.mouseY, rect);

            SDL_Color bg = active
                ? SDL_Color{56, 80, 102, 255}
                : (hovered ? SDL_Color{46, 64, 82, 255} : SDL_Color{38, 54, 70, 255});
            SDL_Color border = active
                ? SDL_Color{255, 215, 0, 255}
                : SDL_Color{76, 100, 124, 255};

            fillRect(renderer, rect, bg);
            strokeRect(renderer, rect, border, active ? 2 : 1);

            SDL_Rect swatch{rect.x + 6, rect.y + 5, 24, 24};
            drawCellVisual(swatch, brushType, false);
            strokeRect(renderer, swatch, {20, 28, 36, 200}, 1);

            SDL_Color labelColor = active
                ? SDL_Color{255, 230, 110, 255}
                : SDL_Color{232, 240, 248, 255};
            ::Renderer::PixelFont::drawText(
                renderer, BRUSH_LABELS[i], rect.x + 38, rect.y + 14, 1, labelColor);
        }

        // 6. Play Game Button
        {
            const bool hoverPlay = pointInRect(editor.mouseX, editor.mouseY, PLAY_BTN_RECT);
            SDL_Color bg = isValid
                ? (hoverPlay ? SDL_Color{46, 176, 96, 255} : SDL_Color{36, 146, 78, 255})
                : SDL_Color{48, 54, 62, 255};
            SDL_Color border = isValid
                ? SDL_Color{116, 240, 158, 255}
                : SDL_Color{84, 94, 106, 255};
            SDL_Color titleColor = isValid
                ? SDL_Color{255, 255, 255, 255}
                : SDL_Color{135, 146, 158, 255};
            SDL_Color subColor = isValid
                ? SDL_Color{205, 255, 222, 255}
                : SDL_Color{120, 130, 142, 255};

            fillRect(renderer, PLAY_BTN_RECT, bg);
            strokeRect(renderer, PLAY_BTN_RECT, border, 2);

            SDL_Rect topHalf{PLAY_BTN_RECT.x, PLAY_BTN_RECT.y + 8, PLAY_BTN_RECT.w, 26};
            SDL_Rect bottomHalf{PLAY_BTN_RECT.x, PLAY_BTN_RECT.y + 36, PLAY_BTN_RECT.w, 18};
            ::Renderer::PixelFont::drawCenteredText(renderer, "PLAY GAME", topHalf, 2, titleColor);
            ::Renderer::PixelFont::drawCenteredText(
                renderer,
                isValid ? "[ENTER] START" : "MAP NOT VALID",
                bottomHalf,
                1,
                subColor);
        }

        // 7. Interactive Map Grid Canvas
        const GridLayout layout = editor.computeGridLayout();

        // Outer frame around the grid
        SDL_Rect outerFrame{
            layout.originX - 2,
            layout.originY - 2,
            layout.totalW + 4,
            layout.totalH + 4
        };
        strokeRect(renderer, outerFrame, isValid ? SDL_Color{72, 188, 118, 220} : SDL_Color{74, 98, 122, 220}, 2);

        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < cols; ++c)
            {
                SDL_Rect cellRect{
                    layout.originX + c * layout.tileW,
                    layout.originY + r * layout.tileH,
                    layout.tileW,
                    layout.tileH
                };

                const int flatIdx = r * cols + c;
                const bool onRoute =
                    isValid &&
                    flatIdx < static_cast<int>(editor.onCorrectPathMask.size()) &&
                    editor.onCorrectPathMask[flatIdx];

                drawCellVisual(cellRect, editor.map->getCell(r, c).type, onRoute);

                // Crisp grid line around each cell when tiles are at least 4px
                if (layout.tileW >= 4 && layout.tileH >= 4)
                {
                    strokeRect(renderer, cellRect, {16, 24, 32, 95}, 1);
                }
            }
        }

        // Hover cursor outline on active grid cell
        if (editor.hoverRow >= 0 && editor.hoverRow < rows &&
            editor.hoverCol >= 0 && editor.hoverCol < cols)
        {
            SDL_Rect hoverRect{
                layout.originX + editor.hoverCol * layout.tileW,
                layout.originY + editor.hoverRow * layout.tileH,
                layout.tileW,
                layout.tileH
            };
            strokeRect(renderer, hoverRect, {255, 235, 120, 245}, 2);
        }

        // 8. Bottom Info Bar
        std::string leftInfo;
        if (editor.hoverRow >= 0 && editor.hoverCol >= 0)
        {
            const Game::CellType hoveredType =
                editor.map->getCell(editor.hoverRow, editor.hoverCol).type;
            leftInfo = "CELL (" + std::to_string(editor.hoverRow) + ", " +
                       std::to_string(editor.hoverCol) + "): " + cellTypeName(hoveredType);
        }
        else
        {
            leftInfo = "GRID: " + std::to_string(rows) + " X " + std::to_string(cols) +
                       " (" + std::to_string(rows * cols) + " CELLS)";
        }

        ::Renderer::PixelFont::drawText(
            renderer,
            leftInfo,
            SIDEBAR_W + 10,
            WINDOW_HEIGHT - BOTTOM_BAR_H + 11,
            1,
            {195, 214, 230, 255});

        const std::string rightHint = "LMB/DRAG: PAINT | RMB: SCENERY";
        const int hintW = ::Renderer::PixelFont::measureWidth(rightHint, 1);
        ::Renderer::PixelFont::drawText(
            renderer,
            rightHint,
            WINDOW_WIDTH - hintW - 10,
            WINDOW_HEIGHT - BOTTOM_BAR_H + 11,
            1,
            {138, 164, 186, 255});

        // Restore SDL's default blend mode so gameplay UI rectangles (which pass alpha=1
        // expecting opaque rendering) draw solid grey rather than transparent.
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    }
}

#pragma once
#include "AbstractRenderable.h"
#include "InputManager.h"
#include "Map.h"
#include "MapLoggingObserver.h"
#include "Observer.h"
#include "RendererMap.h"
#include "Window.h"
#include <memory>
#include <string>
#include <vector>

namespace App
{
    /**
     * @class MapEditor
     * @brief Interactive graphical Map Editor presented before gameplay begins.
     *
     * Allows the player to specify custom map dimensions (from 2x2 up to 12x12),
     * paint Entry, Exit, Path, and Scenery cells directly on the grid, or
     * generate a random valid map of the chosen dimensions. All grid mutations
     * and connectivity checks delegate to Game::Map, while an Observer<Game::Map>
     * keeps the editor's validation state and path preview synchronized.
     */
    class MapEditor
    {
    public:
        static constexpr int MIN_DIMENSION = 2;  ///< Smallest allowed row or column count.
        static constexpr int MAX_DIMENSION = 12; ///< Largest allowed row or column count.

        /**
         * @brief Constructs the Map Editor UI, attaches its render view to Window, and wires mouse callbacks.
         * @param window Game window owning the renderer and input state.
         * @param p_inputManager Optional input manager to attach mouse press/move/release callbacks to.
         * @param initialRows Initial number of grid rows (defaults to NUMB_ROWS).
         * @param initialCols Initial number of grid columns (defaults to NUMB_COLUMNS).
         */
        MapEditor(
            ::Renderer::Window& window,
            InputManager* p_inputManager,
            int initialRows = NUMB_ROWS,
            int initialCols = NUMB_COLUMNS);

        /// Detaches the editor's render view from the Window and cleans up observers.
        ~MapEditor();

        /// Advances editor animations/timers (cursor blink, invalid-play flash) by deltaTime seconds.
        void MainLoop(float deltaTime);

        /// Handles mouse button presses on sidebar controls and grid cells.
        void HandleMousePress();

        /// Handles mouse motion for hover highlights and click-and-drag cell painting.
        void HandleMouseMove();

        /// Ends any active click-and-drag cell painting stroke.
        void HandleMouseRelease();

        /// Processes keyboard shortcuts and numeric text entry for Rows/Cols inputs.
        void HandleKeyboard(::Renderer::Window& window);

        /**
         * @brief Resizes the underlying map to the requested dimensions (clamped to valid bounds).
         * @return True if the dimensions changed, false if unchanged.
         */
        bool setDimensions(int newRows, int newCols);

        /// Generates a random connected map at the current dimensions via Game::Map::generateRandomMap().
        void createRandomMap();

        /// Resets all cells in the current map to Scenery via Game::Map::clear().
        void clearCustomMap();

        /// Selects which CellType brush subsequent grid clicks will apply.
        void selectBrush(Game::CellType brush);

        /// Returns the currently selected CellType brush.
        Game::CellType getSelectedBrush() const { return selectedBrush; }

        /**
         * @brief Paints a single cell on the map using Game::Map::setCellType().
         *
         * Placing Entry or Exit relocates any previous endpoint of the same kind
         * to Scenery (or toggles it back to Scenery if clicking the same cell),
         * keeping single-click endpoint placement effortless.
         */
        void paintCell(int row, int col, Game::CellType type);

        /**
         * @brief Validates the map via Game::Map::validate() and marks the editor ready if valid.
         * @return True if the map is valid and ready to launch gameplay, false otherwise.
         */
        bool tryStartGame();

        /// True once the user has confirmed a valid map to play.
        bool isReadyToPlay() const { return readyToPlay; }

        /// Latest diagnostic status produced by Game::Map::validate().
        Game::Map::ValidationStatus getValidationStatus() const { return validationStatus; }

        /// Read-only access to the map currently being edited.
        const Game::Map& getMap() const { return *map; }

        /**
         * @brief Detaches editor observers and transfers ownership of the validated map to the caller.
         */
        std::unique_ptr<Game::Map> takeMap();

    private:
        enum class ActiveField
        {
            None,
            Rows,
            Cols
        };

        /**
         * @brief Observer notified whenever Game::Map mutates so validation and path preview stay in sync.
         */
        class EditorMapObserver : public Game::Observer<Game::Map>
        {
        public:
            EditorMapObserver(Game::Map& subject, MapEditor& owner)
                : Game::Observer<Game::Map>(subject), owner(owner) {}

            void update(const Game::Map&) override
            {
                owner.onMapChanged();
            }

        private:
            MapEditor& owner;
        };

        /**
         * @brief Self-contained SDL2 renderable drawing the Map Editor sidebar, status bar, and tile grid.
         */
        class EditorCanvas : public ::Renderer::AbstractRenderable
        {
        public:
            explicit EditorCanvas(const MapEditor& editor) : editor(editor) {}

            void Render(::SDL_Renderer* p_renderer) override;
            void setRender(bool p_render) override { m_shouldRender = p_render; }

        private:
            const MapEditor& editor;
        };

        struct GridLayout
        {
            int originX = 0;
            int originY = 0;
            int tileW = 1;
            int tileH = 1;
            int totalW = 1;
            int totalH = 1;
        };

        void onMapChanged();
        void refreshValidation();
        void commitFocusedField();
        GridLayout computeGridLayout() const;
        bool screenToCell(int screenX, int screenY, int& outRow, int& outCol) const;
        std::string statusBannerText() const;

        ::Renderer::Window& window;
        InputManager* inputManager = nullptr;
        ::Renderer::Texture* tileSheetTexture = nullptr;

        std::unique_ptr<Game::Map> map;
        std::unique_ptr<Game::MapLoggingObserver> loggingObserver;
        std::unique_ptr<EditorMapObserver> syncObserver;
        std::unique_ptr<EditorCanvas> canvas;

        Game::CellType selectedBrush = Game::CellType::Entry;
        Game::Map::ValidationStatus validationStatus = Game::Map::ValidationStatus::MissingEntry;
        std::vector<bool> onCorrectPathMask;
        int validatedPathLength = 0;

        ActiveField focusedField = ActiveField::None;
        std::string typedDigits;

        bool isPainting = false;
        bool paintModeErase = false;
        bool readyToPlay = false;

        int mouseX = 0;
        int mouseY = 0;
        int hoverRow = -1;
        int hoverCol = -1;

        float cursorBlinkTimer = 0.0f;
        float invalidFlashTimer = 0.0f;
    };
}

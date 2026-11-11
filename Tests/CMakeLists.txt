file(GLOB_RECURSE TestSource ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)

find_package(Catch2 REQUIRED)

# WaveManager, Player, and the Driver-side critter observers are compiled
# into the Main executable (Driver/Source), not into GameLib, so the test
# target compiles those sources directly. Only these specific files are
# pulled in - not the rest of Driver, which carries the game's main() and
# other app-specific (SDL/GameRenderer-dependent) state.
add_executable(Tests
    ${TestSource}
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/WaveManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/Player.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/TowerBillingObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/CritterBillingObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/CritterRenderSyncObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/GameManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/InputManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/MapRenderSyncObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/PlayerRenderObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/TowerRenderSyncObserver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Source/MapEditor.cpp
)

target_include_directories(Tests PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../Driver/Header>
)

target_link_libraries(Tests PRIVATE GameLib Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(Tests)

#Renderer

file(GLOB_RECURSE RendererSource ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)
file(GLOB_RECURSE RendererHeader ${CMAKE_CURRENT_SOURCE_DIR}/Header/*.h)
add_library(RendererLib SHARED ${RendererSource} ${RendererHeader})

set_property(TARGET RendererLib PROPERTY POSITION_INDEPENDENT_CODE 1) # for shared libraries only

# SDL2 (SDL2::SDL2) and SDL2_image (SDL2_image::SDL2_image) come from the
# system install - found by find_package() in the root CMakeLists.txt.

#LINKING

target_link_libraries(RendererLib
	PUBLIC
    SDL2::SDL2
    SDL2_image::SDL2_image
	MathLib
)

target_include_directories(RendererLib
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Header>
		$<INSTALL_INTERFACE:Header>
		MathLib
)

#Doxygen
set_property(
    GLOBAL APPEND
    PROPERTY TD_DOCUMENTATION_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/Header"
)


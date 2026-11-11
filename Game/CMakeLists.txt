#Game

file(GLOB_RECURSE GameSource ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)
file(GLOB_RECURSE GameHeader ${CMAKE_CURRENT_SOURCE_DIR}/Header/*.h)
add_library(GameLib SHARED ${GameSource} ${GameHeader})

set_property(TARGET GameLib PROPERTY POSITION_INDEPENDENT_CODE 1)

target_include_directories(GameLib
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Header>
		$<INSTALL_INTERFACE:Header>
		RendererLib
		MathLib
)

target_link_libraries(GameLib
	PUBLIC
	RendererLib
	MathLib
)

#Doxygen
set_property(
    GLOBAL APPEND
    PROPERTY TD_DOCUMENTATION_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/Header"
)


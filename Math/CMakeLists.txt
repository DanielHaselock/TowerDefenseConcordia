#Math

file(GLOB_RECURSE MathSource ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)
file(GLOB_RECURSE MathHeader ${CMAKE_CURRENT_SOURCE_DIR}/Header/*.h)
add_library(MathLib SHARED ${MathSource} ${MathHeader})

set_property(TARGET MathLib PROPERTY POSITION_INDEPENDENT_CODE 1) # for shared libraries only

target_include_directories(MathLib
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Header>
		$<INSTALL_INTERFACE:Header>

	PRIVATE

)

target_link_libraries(MathLib
	PUBLIC

	PRIVATE

)

#Doxygen
set_property(
    GLOBAL APPEND
    PROPERTY TD_DOCUMENTATION_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/Header"
)



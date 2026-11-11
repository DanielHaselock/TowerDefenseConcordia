#Root

cmake_minimum_required(VERSION 3.18.4)
# Allow older third-party CMake package files to work with CMake 4+
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(MAIN_NAME Main)

# ============================================================
# Directories
# ============================================================

set(ExternalDir ${CMAKE_CURRENT_SOURCE_DIR}/External) #For dependencies
set(ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Assets) #For images
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

# ============================================================
# Options
# ============================================================

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON) # makes shared libs work
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
option(BUILD_DOCUMENTATION "Build Doxygen documentation" ON)
option(BUILD_TESTS "Build and enable project tests" OFF)

project(TowerDefense)

# ============================================================
# SDL2 - installed on the system
# ============================================================
# The build assumes SDL2 and SDL2_image are already installed on the
# system. On Ubuntu/Debian (including WSL):
#   sudo apt install libsdl2-dev libsdl2-image-dev
# The SDL2::SDL2 and SDL2_image::SDL2_image imported targets are created
# here and linked by the Renderer (see Renderer/CMakeLists.txt).
# SDL2_image's Debian package provides pkg-config metadata rather than a
# CMake package config, so keep a pkg-config fallback for that target.
find_package(SDL2 REQUIRED)
find_package(SDL2_image QUIET CONFIG)
if(NOT TARGET SDL2_image::SDL2_image)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)
    add_library(SDL2_image::SDL2_image INTERFACE IMPORTED)
    set_property(
        TARGET SDL2_image::SDL2_image
        PROPERTY INTERFACE_LINK_LIBRARIES PkgConfig::SDL2_IMAGE
    )
endif()
message(STATUS "Using system SDL2 ${SDL2_VERSION} (found at ${SDL2_DIR})")

add_subdirectory(Driver)
add_subdirectory(Renderer)
add_subdirectory(Game)
add_subdirectory(Math)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(Tests)
endif()

# ============================================================
# Doxygen
# ============================================================

if(BUILD_DOCUMENTATION)

    find_package(Doxygen)

    if(DOXYGEN_FOUND)

        message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")

        set(DOXYGEN_PROJECT_NAME "TowerDefense")
        set(DOXYGEN_PROJECT_BRIEF "Tower Defense Project")

        set(
            DOXYGEN_OUTPUT_DIRECTORY
            "${CMAKE_CURRENT_BINARY_DIR}/docs"
        )

        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_GENERATE_LATEX NO)

        set(DOXYGEN_EXTRACT_ALL YES)
        set(DOXYGEN_RECURSIVE YES)

        # Main page (game rules / design overview), not tied to any library's sources
        set_property(
            GLOBAL APPEND
            PROPERTY TD_DOCUMENTATION_DIRS
            "${CMAKE_CURRENT_SOURCE_DIR}/Docs"
        )

        # Get documentation directories registered by subprojects
        get_property(
            DOCUMENTATION_DIRS
            GLOBAL
            PROPERTY TD_DOCUMENTATION_DIRS
        )

        message(STATUS "Doxygen directories: ${DOCUMENTATION_DIRS}")

        doxygen_add_docs(
            docs
            ${DOCUMENTATION_DIRS}
            COMMENT "Generating TowerDefense documentation"
        )
		if(WIN32)
			add_custom_command(
				TARGET docs
				POST_BUILD
				COMMAND cmd /c start "" "${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html"
				COMMENT "Opening documentation"
				VERBATIM
			)
		elseif(APPLE)
			add_custom_command(
				TARGET docs
				POST_BUILD
				COMMAND open "${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html"
				COMMENT "Opening documentation"
				VERBATIM
			)
		elseif(DEFINED ENV{WSL_DISTRO_NAME})

			find_program(WSLVIEW wslview)

			if(WSLVIEW)
				add_custom_command(
					TARGET docs
					POST_BUILD
					COMMAND ${WSLVIEW} "${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html"
					COMMENT "Opening documentation"
					VERBATIM
				)
			endif()
		elseif(UNIX)
			add_custom_command(
				TARGET docs
				POST_BUILD
				COMMAND xdg-open "${DOXYGEN_OUTPUT_DIRECTORY}/html/index.html"
				COMMENT "Opening documentation"
				VERBATIM
			)
		endif()

    else()

        message(WARNING "Doxygen not found. Please install Doxygen.")

    endif()

endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${MAIN_NAME})
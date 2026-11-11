#Driver

file(GLOB_RECURSE MainSource CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp")
file(GLOB_RECURSE MainHeader CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Header/*.h")

add_executable(${MAIN_NAME} ${MainSource} ${MainHeader})

target_link_libraries(${MAIN_NAME} PRIVATE
    RendererLib
    GameLib
)

target_include_directories(${MAIN_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Header"
)

# Doxygen
set_property(GLOBAL APPEND PROPERTY TD_DOCUMENTATION_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/Header"
)

# Run with the executable's directory as the working directory.
set_target_properties(${MAIN_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${MAIN_NAME}>"
)

# Copy project DLLs next to Main.exe.
foreach(project_lib IN ITEMS GameLib RendererLib MathLib)
    add_custom_command(TARGET ${MAIN_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:${project_lib}>"
            "$<TARGET_FILE_DIR:${MAIN_NAME}>"
        VERBATIM
    )
endforeach()

# Copy Assets next to Main.exe.
add_custom_command(TARGET ${MAIN_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ASSET_DIR}"
        "$<TARGET_FILE_DIR:${MAIN_NAME}>/Assets"
    COMMENT "Copying Assets folder"
    VERBATIM
)

# Copy vcpkg runtime DLLs, including transitive dependencies such as
# libpng16.dll and zlib1.dll. Debug and Release have different DLL directories.
if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_RUNTIME_BIN
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/$<IF:$<CONFIG:Debug>,debug/bin,bin>"
    )

    add_custom_command(TARGET ${MAIN_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${VCPKG_RUNTIME_BIN}"
            "$<TARGET_FILE_DIR:${MAIN_NAME}>"
        COMMENT "Copying vcpkg runtime DLLs (SDL2, SDL2_image, libpng, etc.)"
        VERBATIM
    )
else()
    message(WARNING
        "vcpkg directory or triplet not available: SDL2 runtime DLLs may not be copied. "
        "Configure with -DCMAKE_TOOLCHAIN_FILE=.../vcpkg.cmake and "
        "-DVCPKG_TARGET_TRIPLET=x64-windows."
    )
endif()
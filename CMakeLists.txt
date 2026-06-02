cmake_minimum_required(VERSION 3.22)

project(BVHView VERSION 1.0.1 LANGUAGES C CXX)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel)
endif()

if(NOT WIN32)
    message(FATAL_ERROR "BVHView_Cpp only supports Windows")
endif()

enable_language(RC)

set(BVHVIEW_RAYLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/raylib")
set(BVHVIEW_RAYGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/raygui")

if(NOT EXISTS "${BVHVIEW_RAYLIB_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "raylib submodule is missing; run: git submodule update --init --recursive")
endif()

if(NOT EXISTS "${BVHVIEW_RAYGUI_DIR}/src/raygui.h")
    message(FATAL_ERROR "raygui submodule is missing; run: git submodule update --init --recursive")
endif()

set(BUILD_EXAMPLES OFF CACHE BOOL "Build raylib examples" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build raylib as a shared library" FORCE)
set(PLATFORM Desktop CACHE STRING "Build raylib desktop platform" FORCE)
add_subdirectory("${BVHVIEW_RAYLIB_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/raylib" EXCLUDE_FROM_ALL)

set(BVHVIEW_SOURCES
    src/main.cpp
    src/core/args.cpp
    src/core/math_utils.cpp
    src/camera/orbit_camera.cpp
    src/bvh/bvh_data.cpp
    src/animation/transform_data.cpp
    src/animation/character_data.cpp
    src/render/geometry.cpp
    src/render/capsule_data.cpp
    src/render/shader.cpp
    src/render/model.cpp
    src/render/drawing.cpp
    src/ui/file_dialog_scale.cpp
    src/ui/ui_scale.cpp
    src/ui/viewer_settings.cpp
    src/ui/viewer_ui.cpp
    src/app/application.cpp
    bvhview.rc
)

add_executable(bvhview WIN32 ${BVHVIEW_SOURCES})

target_compile_features(bvhview PRIVATE cxx_std_17)
set_target_properties(bvhview PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(bvhview PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${BVHVIEW_RAYLIB_DIR}/src"
    "${BVHVIEW_RAYGUI_DIR}/src"
)

target_link_libraries(bvhview PRIVATE raylib)

set_target_properties(bvhview PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/app"
)

if(MSVC)
    target_compile_options(bvhview PRIVATE /W4)
    target_link_options(bvhview PRIVATE /ENTRY:mainCRTStartup)
else()
    target_compile_options(bvhview PRIVATE -Wall)
endif()

target_compile_definitions(bvhview PRIVATE
    $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:NDEBUG>
)

if(NOT MSVC)
    target_compile_options(bvhview PRIVATE
        $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-O3>
    )
endif()

include(GNUInstallDirs)
install(TARGETS bvhview RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

cmake_minimum_required(VERSION 3.16)
project(MigAlleyRevisited LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------------------
# Dependencies via pkg-config (ensures 32-bit libs are found)
# -------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)
pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
pkg_check_modules(VULKAN REQUIRED vulkan)

# Common settings for all in-house targets
add_library(ProjectDefaults INTERFACE)

if(NOT MSVC)
    # Force 32-bit build on GCC/Clang, and disable enum comparison warnings
    message(STATUS "Forcing 32-bit build: adding -m32 to CMAKE_CXX_FLAGS")
    target_compile_options(ProjectDefaults INTERFACE -m32 -fms-extensions -Wno-enum-compare)
    target_link_options(ProjectDefaults INTERFACE -m32)
endif()

target_include_directories(ProjectDefaults INTERFACE
    ${CMAKE_SOURCE_DIR}/Includes
    ${SDL2_INCLUDE_DIRS}
    ${SDL2TTF_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${GTK2_INCLUDE_DIRS}
    ${VULKAN_INCLUDE_DIRS}
)

target_link_libraries(ProjectDefaults INTERFACE
    ${SDL2_LIBRARIES}
    ${SDL2TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
    ${GTK2_LIBRARIES}
    ${VULKAN_LIBRARIES}
)

target_compile_definitions(ProjectDefaults INTERFACE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# -------------------------------------------------------------------
# Add subprojects
# -------------------------------------------------------------------
add_subdirectory(AI)
add_subdirectory(AIRCRAFT)
add_subdirectory(3D)
add_subdirectory(Comms)
add_subdirectory(Files)
add_subdirectory(Graphics)
add_subdirectory(Hardware)
add_subdirectory(Input)
add_subdirectory(Math)
add_subdirectory(Missman)
add_subdirectory(Model)
add_subdirectory(Movecode)
add_subdirectory(Bfields)
add_subdirectory(CollisionsEtAl)
add_subdirectory(Overlay)
add_subdirectory(Assembler)
add_subdirectory(GENERAL)
add_subdirectory(UI)
add_subdirectory(MigAlley)

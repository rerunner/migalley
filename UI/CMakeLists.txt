cmake_minimum_required(VERSION 3.16)
project(UI LANGUAGES CXX)

# Build as a static library
add_library(UI STATIC
    MiniUI.cpp
    uid_reverse.cpp
    MiniUI.h   # optional, for IDE visibility
)

# Inherit common includes/defines
target_link_libraries(UI PRIVATE ProjectDefaults)

# Extra include dirs specific to UI
target_include_directories(UI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../MigAlley
)

cmake_minimum_required(VERSION 3.16)
project(Overlay LANGUAGES CXX)

add_library(Overlay STATIC
    # Overlay source files
    ../3D/MONOTXT.CPP
    ../3D/OVERLAY.CPP
    ../Missman/NODEREV.CPP
)

target_link_libraries(Overlay PRIVATE ProjectDefaults)

target_include_directories(Overlay
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../3D
        ${CMAKE_CURRENT_SOURCE_DIR}/../Missman
)

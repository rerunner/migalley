cmake_minimum_required(VERSION 3.16)
project(CollisionsEtAl LANGUAGES CXX)

add_library(CollisionsEtAl STATIC
    # CollisionsEtAl source files
    ../3D/JIMCOL.CPP
    ../3D/PARTICLE.CPP
    ../3D/TILEMAKE.CPP
    ../Graphics/WINSMACK.CPP
    000TOOLS.CPP
)

target_link_libraries(CollisionsEtAl PRIVATE ProjectDefaults)

# Inherit common includes/defines
target_link_libraries(CollisionsEtAl PRIVATE ProjectDefaults)

# Extra include dirs specific to CollisionsEtAl
target_include_directories(CollisionsEtAl
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../3D
        ${CMAKE_CURRENT_SOURCE_DIR}/../Graphics
)

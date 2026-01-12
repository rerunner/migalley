cmake_minimum_required(VERSION 3.16)
project(Missman LANGUAGES CXX)

add_library(Missman STATIC
    # Missman source files
    FORMATN.CPP
    MISSINIT.CPP
    NODE.CPP
    PACKAGES.CPP
    SAVEGAME.CPP
)

target_link_libraries(Missman PRIVATE ProjectDefaults)


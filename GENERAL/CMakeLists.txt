cmake_minimum_required(VERSION 3.16)
project(General LANGUAGES CXX)

add_library(General STATIC
    # General source files
    MALLOC.CPP
    MigCstring.cpp
    STATIC.CPP
    WINERROR.CPP
)

target_link_libraries(General PRIVATE ProjectDefaults)


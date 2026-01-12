cmake_minimum_required(VERSION 3.16)
project(Files LANGUAGES CXX)

add_library(Files STATIC
    # Files source files
    FILEMAN.CPP
    LOADLIST.CPP
    WINFILE.CPP
)

target_link_libraries(Files PRIVATE ProjectDefaults)


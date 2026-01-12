cmake_minimum_required(VERSION 3.16)
project(Math LANGUAGES CXX)

add_library(Math STATIC
    # Math source files
    MATH.CPP
    MATRIX.CPP
)

target_link_libraries(Math PRIVATE ProjectDefaults)


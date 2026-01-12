cmake_minimum_required(VERSION 3.16)
project(Graphics LANGUAGES CXX)

add_library(Graphics STATIC
    # Graphics source files
    GRAFPRIM.CPP
    LBM.CPP
    POLYGON.CPP
    TGA.CPP
    WRAPPER.CPP


)

target_link_libraries(Graphics PRIVATE ProjectDefaults)


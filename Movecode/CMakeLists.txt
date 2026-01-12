cmake_minimum_required(VERSION 3.16)
project(Movecode LANGUAGES CXX)

add_library(Movecode STATIC
    # Movecode source files
    AUTOMOVE.CPP
    FLYINIT.CPP
    FLYMODEL.CPP
    KEYFLY.CPP
    MOVEALL.CPP
    WPACTION.CPP
)

target_link_libraries(Movecode PRIVATE ProjectDefaults)


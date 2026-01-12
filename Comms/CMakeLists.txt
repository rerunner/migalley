cmake_minimum_required(VERSION 3.16)
project(Comms LANGUAGES CXX)

add_library(Comms STATIC
    # Comms source files
    AGGRGTOR.CPP
    COMMS.CPP
    REPLAY.CPP
    WINMOVE.CPP
)

target_link_libraries(Comms PRIVATE ProjectDefaults)


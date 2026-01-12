cmake_minimum_required(VERSION 3.16)
project(Input LANGUAGES CXX)

add_library(Input STATIC
    # Input source files
    ANALOGUE.CPP
    KEYTESTS.CPP
)

target_link_libraries(Input PRIVATE ProjectDefaults)


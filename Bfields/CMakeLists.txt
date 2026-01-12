cmake_minimum_required(VERSION 3.16)
project(Bfields LANGUAGES CXX)

# Build as a static library
add_library(Bfields STATIC
    GLOBREFS.CPP
    MAPCODE.CPP
    MESSAGES.CPP
    PERSONS2.CPP
    PERSONS3.CPP
    PERSONS4.CPP
    PERSONS7.CPP
    QMISS.CPP
    SAVEBIN.CPP
    TANK.CPP
)

# Inherit common includes and definitions
target_link_libraries(Bfields PRIVATE ProjectDefaults)


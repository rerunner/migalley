cmake_minimum_required(VERSION 3.16)
project(Model LANGUAGES CXX)

add_library(Model STATIC
    # Model source files
    ACMAI.CPP
    ACMMAN.CPP
    ACMMODEL.CPP
    ACMSIMPL.CPP
    AERODEV.CPP
    CTRL.CPP
    CURVES.CPP
    ELMT.CPP
    ENGINE.CPP
    GEAR.CPP
    MODEL.CPP
    MODINT.CPP
    MODVEC.CPP
    SKY.CPP
)

target_link_libraries(Model PRIVATE ProjectDefaults)


cmake_minimum_required(VERSION 3.16)
project(Hardware LANGUAGES CXX)

add_library(Hardware STATIC
    # Hardware source files
    CONFIG.CPP
    DDRWINIT.CPP
    HARDPUB.CPP
    HARDWIN.CPP
    MESSENGN.CPP
    MILES.CPP
    RCHATTER.CPP
    SFONTS.CPP
    WIN3D.CPP
    direct_3d.cpp
    XX_ScreenFlip_Vulkan.cpp
)

target_link_libraries(Hardware PRIVATE ProjectDefaults)


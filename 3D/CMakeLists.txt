cmake_minimum_required(VERSION 3.16)
project(My3D LANGUAGES CXX)

# Build as a static library (consistent with solution usage)
add_library(My3D STATIC
    3DCODE.CPP
    3DCOM.CPP
    ANMRADIO.CPP
    BTREE.CPP
    COLLIDED.CPP
    IMAGEMAP.CPP
    LANDSCAP.CPP
    MIGLAND.CPP
    SHAPES.CPP
    SHPBLD.CPP
    TRANSITE.CPP
    VIEWSEL.CPP
    WEAPPAK.CPP
    WORLD.CPP
)

target_link_libraries(My3D PRIVATE ProjectDefaults)


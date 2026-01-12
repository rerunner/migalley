cmake_minimum_required(VERSION 3.16)
project(MigAlley LANGUAGES CXX)

if (WIN32)
    target_sources(MigAlley PRIVATE MIG.RC)
endif()

add_executable(MigAlley
    GeneratedResources.cpp
    MigCString.cpp
    Comit_e.cpp
    Damage.cpp
    FRAG.CPP
    Fullpane.cpp
    MigAlley.cpp
    MRESULT.CPP
    Sdetail.cpp
    stdafx.cpp
    WEAPONS.CPP
    WinMig3D.CPP
    MIG.RC
)

target_precompile_headers(MigAlley PRIVATE stdafx.h)

# Inherit defaults
target_link_libraries(MigAlley PRIVATE ProjectDefaults)

# Extra include dirs specific to MigAlley
target_include_directories(MigAlley PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../UI
    C:/DX81SDK/include
)

# Internal libraries
target_link_libraries(MigAlley PRIVATE
    My3D
    Assembler
    Overlay
    AI
    Bfields
    Comms
    Files
    General
    Graphics
    Input
    Math
    Movecode
    Missman
    Hardware
    CollisionsEtAl
    Model
    Aircraft
    UI
)

# External libraries via find_package

target_link_libraries(MigAlley PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES}
)

if (WIN32)
# Windows system libraries
target_link_libraries(MigAlley PRIVATE
    kernel32 user32 gdi32 comdlg32 winspool winmm shell32 comctl32
    ole32 oleaut32 uuid rpcrt4 advapi32 wsock32 wininet
    opengl32 d3dx dplayx dsound dinput dxguid ddraw
)
endif()

if (MSVC)
    target_link_options(MigAlley PRIVATE
        $<$<CONFIG:Debug>:/NODEFAULTLIB:libcmtd.lib>
    )
endif()


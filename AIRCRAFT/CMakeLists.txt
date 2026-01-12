cmake_minimum_required(VERSION 3.16)
project(Aircraft LANGUAGES CXX)

# Build as a static library
add_library(Aircraft STATIC
    AI_F51D.CPP
    AI_F80C.CPP
    AI_F84E.CPP
    AI_F86E.CPP
    AI_MIG15.CPP
    AI_YAK9.CPP
    CD_COMM.CPP
    CD_F51D.CPP
    CD_F80C.CPP
    CD_F84E.CPP
    CD_F86E.CPP
    CD_MIG15.CPP
    CD_YAK9.CPP
    DT_F51D.CPP
    DT_F80C.CPP
    DT_F84E.CPP
    DT_F86E.CPP
    DT_MIG15.CPP
    DT_OTHER.CPP
    DT_YAK9.CPP
)

target_link_libraries(Aircraft PRIVATE ProjectDefaults)


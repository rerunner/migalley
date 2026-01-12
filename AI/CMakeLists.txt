cmake_minimum_required(VERSION 3.16)
project(AI LANGUAGES CXX)

add_library(AI STATIC
    # AI source files
    AAA.CPP
    MSGAI.CPP
    SPOTTED.CPP
    USERMSG.CPP
)

target_link_libraries(AI PRIVATE ProjectDefaults)


cmake_minimum_required(VERSION 3.16)

# Only enable the MASM language when building with MSVC. On Linux/GCC we
# do not have MASM, so keep the project language to CXX which is available.
if (MSVC)
    project(Assembler LANGUAGES CXX ASM_MASM)
else()
    project(Assembler LANGUAGES CXX)
endif()

# Build as a static library
add_library(Assembler STATIC)

# If building with MSVC, add the MASM source. On other platforms add a tiny
# C++ stub so that CMake can determine the link language for the target.
if (MSVC)
    target_sources(Assembler PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../Graphics/GRAFPASM.ASM
    )
else()
    # Provide a trivial C++ translation unit so the static library has a
    # defined language on non-MSVC platforms (avoids "Cannot determine link
    # language" errors).
    target_sources(Assembler PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stub_empty.cpp)
endif()

# Inherit common includes/defines
target_link_libraries(Assembler PRIVATE ProjectDefaults)

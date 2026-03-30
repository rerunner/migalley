# Mig Alley Linux Port

A complete native Linux port of Rowan’s **Mig Alley** (1999).  
All original Windows dependencies have been removed or replaced with modern, cross‑platform components.  
The objective is long‑term preservation, maintainability, and future expansion of the dynamic campaign engine.

## Overview

This project contains a full platform migration of Mig Alley from Windows to Linux.  
The codebase no longer depends on Win32, DirectX, DirectInput, or Miles Sound System.  
All required subsystems have been rewritten or adapted to use portable libraries and modern APIs.

## Major Changes

### Platform and System Layer
- Replacement of the MFC layer with a custom implementation compatible with the original message routing model  
- Reimplementation of required Win32 API functions  
- Removal of all Windows‑specific code paths  
- Native Linux build using standard toolchains

### Graphics
- 2D rendering implemented with SDL2  
- UI rendering implemented with SDL2  
- 3D engine rewritten to use Vulkan
- All legacy Direct3D 5/6 fixed‑function code replaced with a modern rendering backend

### Input
- DirectInput replaced with SDL input  
- Support for mouse, keyboard, and joystick through SDL’s unified input system  
- Axis, button, and hat mappings aligned with original behavior

### Audio
- Miles Sound System API ported to use SDL_mixer  
- Channel management and playback timing matched to original logic

### Dependencies
The Linux version depends only on:
- SDL2, SDL_mixer and SDL_ttf  
- Vulkan loader and driver
- nlohmann C++ json support

No Windows DLLs or compatibility layers are required.

## Build Instructions

### Requirements
- C++ compiler with C++17 support  
- CMake  
- SDL2 development packages  
- SDL_mixer development packages  
- Vulkan SDK or system Vulkan headers and loader

### Build
```
mkdir build
cd build
cmake ..
make
```

## Current Status

The game is fully playable on Linux.  
All subsystems have been ported.  
The campaign, AI, UI, and mission logic behave as in the original release.

## Roadmap

### Short Term
- Codebase cleanup  
- Validation of campaign logic  
- Regression testing against the Windows version

### Medium Term
- Reactive multiplayer
- Improvements to the dynamic campaign engine  
- More autonomous ground and air operations  
- Better integration between strategic and tactical layers

### Long Term
- Tools for mission analysis and debugging

## License

This project uses the original license provided by Rowan Software when the source code was released.

## Acknowledgments

Original game by Rowan Software.  
This project aims to preserve and extend the technical and historical value of the simulation.

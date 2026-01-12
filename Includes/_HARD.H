#include "WIN32_COMPAT.H" //RERUN
#include "SDL.h"
#include "SDL_ttf.h" //RERUN
#include "SDL_opengl.h"
#include "GL/glu.h"
#define DIRECTINPUT_VERSION 0x700 // RERUN
#include "dinput_stub.h"

#define DIRECTDRAW_VERSION 0x0600
#include 	"DOSDEFS.H"
#include	<stdio.h>
#include	<string.h>
#include	"MONOTXT.H"
#if defined (__MSVC__)
#include	"MSSW.H"
#else
// No Miles sound system on linux
#include "mssw_compat.h" // stubbing MSSW.H, not MILES.H, keep that one
#endif


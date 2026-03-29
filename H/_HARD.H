#include "SDL.h"
#include "SDL_ttf.h" //RERUN
#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "MIG.H"
#include "MIGVIEW.H"
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


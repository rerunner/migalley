#pragma optimize( "g", off )
//RERUN #ifdef _DEBUG
//RERUN #define new DEBUG_NEW
//RERUN #undef THIS_FILE
//RERUN #define THIS_FILE __FILE__
//RERUN #define	THIS_FILE_DEFINED
//RERUN #endif

#define F_SOUNDS
#include "DOSDEFS.H"
//RERUN #include "files.g"
#include "FILES.H"

#include	<afxmt.h>
#define DIRECTINPUT_VERSION 0x0700 // RERUN
#include	"dinput_stub.h"
#include	<mmsystem.h>
#include	"STUB3D.H"
#include "DISPLAY.H"
#include "STDAFX.H"
#include	"FLYINIT.H"
#include	"DEBRIEF.H"
#include "MISSSUB.H"
CString	MakePilotName(PilotName& name);

#include "ENUMBITS.M"

MATHABLE(SquadSelect)
MATHABLE(Debrief::Types)
MATHABLE(Debrief::PlayerTypes)
#ifndef _DEBUG
#pragma message(__HERE__ "Non-debug version: global optimisation enabled when safe!")
#pragma optimize( "g", on )
#endif




#define	F_COMMON
#define	F_GRAFIX
//#define	WINMOVE_Included
#include	"DOSDEFS.H"
#include	<stdio.h>
#ifdef __MSVC__
#  include <io.h>
#else
#  include <unistd.h>   // for close(), read(), write(), access(), etc.
#  include <fcntl.h>    // for open() flags
#  include <sys/stat.h> // for chmod(), stat()
#  define _open   open
#  define _close  close
#  define _read   read
#  define _write  write
#  define _access access
#  define _chmod  chmod
#endif
#include	<stdarg.h>
#include	<stdlib.h>
#include	<string.h>
#include	"MODFILE.H"
#include	"MYANGLES.H"
#include	"WORLDINC.H"
#include	"ANIMDATA.H"

#include	"UNIQUEID.H"
#include	"WORLD.H"
#include	"FILEMAN.H"
#include	"MAKEBF.H"
#define		TEXTREF_NOTYPES
#include	"TEXTREF.H"
#include	"PERSONS2.H"
#include	<string.h>

#include	"MYERROR.H"
#include	"SHAPES.H"
#include	"MYMATH.H"
#include	"FLYMODEL.H"
#include	"MISSMAN2.H"
//MATHABLE	(ANGLES);

#include	"BITFIELD.H"
#include	"3DCOM.H"
#include	"MYTIME.H"
#include	"FLYMODEL.H"

#include	"AI.H"

#include	"VIEWSEL.H"

#include	"LANDSCAP.H"

#include	"SAVEGAME.H"
#include	"AAA.H"
#include	"SHPINSTR.H"
#include	"STUB3D.H"
//TempCode ARM 12Sep96 #include 	<windows.h>											//ARM 09Aug96
//TempCode ARM 12Sep96 #include 	<windowsx.h>										//ARM 09Aug96
//TempCode ARM 12Sep96 #include 	<mmsystem.h>										//ARM 09Aug96
//TempCode ARM 12Sep96 #include	"c:\dxsdk\sdk\inc\dplay.h"							//ARM 09Aug96
#include	"WINMOVE.H"

#include	"MODINST.H"
//#include	"replay.h"

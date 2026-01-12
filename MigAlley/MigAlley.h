#ifndef MIGALLEY_H
#define MIGALLEY_H

//#include "resource.h"
#include "WIN32_COMPAT.H" //RERUN
#include "CSTRING.H" //RERUN
#include "mfc/RESOURCE.H"
#include "RESOURCE.H"

int miginit();
int migselect(int choice);
int migfly();
int migdestroy();

inline	CString	LoadResString(int resnum)
{
	CString s;
	s.LoadString(resnum);
	return s;
};
#define	RESSTRING(name)	LoadResString(IDS_##name)

extern HWND global_hWnd;

enum	GameStates	{ TITLE, QUICK, HOT, CAMP, WAR, MATCH, REPLAYLOAD };

#endif //MIGALLEY_H

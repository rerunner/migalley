// MigAlley.cpp : Defines the entry point for the console application.
//
//RERUN #include "STDAFX.H"
#include "WIN32_COMPAT.H" //RERUN
#include <iostream>
#include <SDL.h>
#include "SDL_syswm.h"
#include "SDL_opengl.h"//TEST
#include <SDL_ttf.h> //RERUN
#define DIRECTINPUT_VERSION 0x700 // RERUN
#include "dinput_stub.h"
#include "MigAlley.h"
#include "DOSDEFS.H" //RERUN
#include "WINMOVE.H"
#include "WinMig3D.H" //RERUN
#include "FULLPANE.H" //RERUN
#include "GAMESET.H" //RERUN
#include "PERSONS2.H" //RERUN
#include "GLOBREFS.H" //RERUN
#include "GAMESET.H" //RERUN
#include "MILES.H" //RERUN
#define QUICKQUICKQUICK //RERUN
#include "SQUICK1.H" //RERUN
#include "OVERLAY.H" //RERUN for MAP
#include "MiniUI.h" //RERUN
#include "GeneratedResources.h" // RERUN for linux

#define MAX_LOADSTRING 100

// Global Variables:
HWND global_hWnd; //RERUN
RFullPanelDial* m_pfullpane; //RERUN
UniqueID trg_uid; //RERUN
Inst3d*	tmpinst; //RERUN
View3d*	tmpview;//RERUN
bool 	tempblockkeys; //RERUN
extern SWord	winmode_w, winmode_h; //RERUN

//RERUN
void	DPlay::InitMessages()
{
	for (int n = 0; n<NUMRADIOMESSAGES; n++)
		strcpy(Messages[mySlot][n], RESSTRING(L_RADIOMSG0 + n));
}
//RERUN

//////////////////////////

View3d*	Launch3d(bool flag) // flag means "wasrunning"
{
	//PROBLEM!!!! MUST NOT do this when running 3d in a window!!!!!!!!!!!!!
	//	gameSettings.m_bFullScreenMode = false;
	if (gameSettings.m_bFullScreenMode)
	{
#if defined (__MSVC__)
		MoveWindow(global_hWnd, 0, 0, winmode_w, winmode_h, true);
#else
	SDL_SetWindowPosition(global_hWnd, 0, 0);
	SDL_SetWindowSize(global_hWnd, winmode_w, winmode_h);
#endif
	}

	if (tmpinst == NULL)
	{
		if (trg_uid == UID_Null)
		{
			Persons4::ShutDownMapWorld();
			{
				GR_GlobalSkillMin = SKILL_NOVICE;						//RDH 01Oct96
				GR_GlobalSkillMax = SKILL_HERO;							//RDH 01Oct96

				GR_NAT_ENEMY = NAT_RED;
				GR_NAT_FRIEND = NAT_BLUE;
			}
			tmpinst = new Inst3d;
		}
		else
		{
			Persons4::InitViewFromMap(trg_uid);
			tmpinst = new Inst3d(true);
		}
	}
	if (!tmpview)
	{
		tmpview = new View3d(tmpinst, global_hWnd);
		if (trg_uid == UID_Null)	
			tmpview->MakeInteractive(WinMode::WIN, *(RECT*)NULL, false, flag);
		else
			tmpview->MakeInteractive(WinMode::WIN, *(RECT*)NULL, true);
	}

	//#pragma message("MigAlley.cpp - paused for profiling")
//#ifdef NDEBUG
	if (trg_uid == UID_Null)											  //DAW 18/02/00
		tmpinst->Paused(FALSE);										  //DAW 18/02/00
//#endif
	tempblockkeys = false;
	return tmpview;
}

extern	UWord	DoSmack(int windowhandle);
extern	void	CloseSmack();
void SelectIntro(FullScreen * startscreen)
{
	startscreen = &RFullPanelDial::introsmack;
	m_pfullpane->IntroSmackInit();
#if 0 // Movie
	while (DoSmack((int)global_hWnd) > 0) {} //RERUN, move this to a timer event loop
	CloseSmack();
#endif
	Save_Data.InitPreferences((int)Master_3d.winst); //Call this when skipping IntroSmackInit

	startscreen = &RFullPanelDial::title;
	m_pfullpane->LaunchMain(startscreen);
}

void SelectHotShot(FullScreen * startscreen)
{
	startscreen = &RFullPanelDial::quickmissionflight;
	m_pfullpane->SetUpHotShot(startscreen);
}

void SelectReplay(FullScreen * startscreen)
{
	startscreen = &RFullPanelDial::replayload;
	m_pfullpane->InitReplay(startscreen);
}

void SelectQuickMission(FullScreen * startscreen)
{
	startscreen = &RFullPanelDial::quickmission;
	m_pfullpane->QuickViewInit();
	CSQuick1::quickdef = CSQuick1::quickmissions[CSQuick1::currquickmiss];
	m_pfullpane->SetQuickState(startscreen);
}

void SelectCampaign(FullScreen * startscreen)
{
	startscreen = &RFullPanelDial::campaignselect;
	m_pfullpane->SetCampState(startscreen);
	m_pfullpane->CampaignSelectInit();
#if 0 // Movie
	m_pfullpane->StartCampSmacker(startscreen);
	while (DoSmack((int)global_hWnd) > 0) {} //RERUN, move this to a timer event loop
	CloseSmack();
#endif
	m_pfullpane->StartCampBackground(startscreen);
	m_pfullpane->StartCampObjectives(startscreen);
	m_pfullpane->LaunchMapFirstTime(startscreen);
}

void SelectFullWar(FullScreen * startscreen)
//&campstart, &RFullPanelDial::SetUpFullWar
{
	startscreen = &RFullPanelDial::campstart;
	m_pfullpane->SetUpFullWar(startscreen);
	m_pfullpane->CampaignSelectInit();
#if 0 // Movie
	m_pfullpane->StartCampSmacker(startscreen);
	while (DoSmack((int)global_hWnd) > 0) {} //RERUN, move this to a timer event loop
	CloseSmack();
#endif
	m_pfullpane->StartCampBackground(startscreen);
	m_pfullpane->StartCampObjectives(startscreen);
	m_pfullpane->LaunchMapFirstTime(startscreen);
}

void SelectSoftwareGraphics(bool selection)
{
	Save_Data.fSoftware = selection;
	if (Save_Data.fSoftware)
	{
		Save_Data.textureQuality = 4;
		Save_Data.filtering = 0;
		//Save_Data.detail_3d %= DETAIL3D_INCONSEQUENTIALS;
		Save_Data.detail_3d |= DETAIL3D_INCONSEQUENTIALS;
		Save_Data.detail_3d %= DETAIL3D_GROUNDSHADING;
		//Save_Data.cockpit3Ddetail %= COCK3D_SKYIMAGES;
		Save_Data.cockpit3Ddetail |= COCK3D_SKYIMAGES;
		Save_Data.gamedifficulty %= GD_GEFFECTS;
		Save_Data.gamedifficulty %= GD_INJURYEFFECTS;
		Save_Data.gamedifficulty %= GD_WHITEOUT;
		Save_Data.gamedifficulty |= GD_PERIPHERALVISION;
		//Save_Data.gamedifficulty %= GD_DISPLAYMESSAGES;
		//Save_Data.infoLineCount = 0;
	}
}

#if defined (__MSVC__)
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#if 0
	miginit();
	migselect(0); //0 = hotshot
	migfly();
#else
	UIMain(argc, argv);
#endif
	return 0;
}

int miginit()
{
	if (SDL_Init(SDL_INIT_VIDEO| SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) != 0){
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}
	TTF_Init();

	//Use OpenGL 2.1
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

	//Now create a window
	winmode_w = 1366; //1920;
	winmode_h = 768; //1080;
	CString winTitle =  RESSTRING(MIGALLEY);
	printf("The window title = %s\n", winTitle.c_str());
	SDL_Window *win = SDL_CreateWindow(winTitle.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
#ifdef NDEBUG // RERUN, maybe
		0, 0,
		SDL_WINDOW_FULLSCREEN |
#else
		winmode_w, winmode_h,
		SDL_WINDOW_SHOWN |
#endif
		SDL_WINDOW_VULKAN);


	Master_3d.SdlWinInst = win;
	Master_3d.SdlMigTexture = NULL;

	SDL_SysWMinfo SysInfo; //Will hold our MigWindow information
	SDL_VERSION(&SysInfo.version); //Set SDL version

	if (SDL_GetWindowWMInfo(win, &SysInfo) <= 0) {
		printf("%s \n", SDL_GetError());
		return false;
	}

#if defined (__MSVC__)
	HWND WindowHandle = SysInfo.info.win.window; //There it is, Win32 handle
#else
	HWND WindowHandle = win;
#endif
	printf("%d\n", (int)WindowHandle);

	global_hWnd = WindowHandle; //RERUN

#if defined (__MSVC__)
	HINSTANCE hInst = GetModuleHandle(NULL); //Get HINSTANCE from running exe
#else
	HINSTANCE hInst = nullptr;
#endif
	Master_3d.Init(hInst, global_hWnd);

	/////////////////////////////////
	tempblockkeys = true;
	for (int i = 0; i < Master_3d.NUM_EVENTS; i++)
		Master_3d.htable[i] = CreateEvent(0, 0, 0, 0);

	// for tracking the idle time state
	BOOL bIdle = TRUE;
	LONG lIdleCount = 0;
	/////////////////////////////////
	m_pfullpane = new RFullPanelDial; // must create it on the heap
	FullScreen * startscreen = NULL;
	SelectIntro(startscreen);

	return 0;
}

int migselect(int choice)
{
//	m_pfullpane = new RFullPanelDial; // must create it on the heap
	FullScreen * startscreen = NULL;
//	SelectIntro(startscreen);

	gameSettings.m_dwWidth = Save_Data.displayW = winmode_w;
	gameSettings.m_dwHeight = Save_Data.displayH = winmode_h;
	SelectSoftwareGraphics(true); // software graphics versus opengl (eventually)

	switch (choice)
	{
	case 0:
		SelectHotShot(startscreen);
		break;
	case 1:
		SelectQuickMission(startscreen);
		break;
	case 2:
		SelectCampaign(startscreen);
		break;
	case 3:
		SelectFullWar(startscreen);
		break;
	default:
		break;
	}
	return 0;
}

//////////////////////////////////
#include <SDL2/SDL.h>
#include <unordered_map>

inline int SDLScancodeToDIK(SDL_Scancode sc)
{
    static const std::unordered_map<SDL_Scancode, int> map = {
        {SDL_SCANCODE_ESCAPE, DIK_ESCAPE},
        {SDL_SCANCODE_1, DIK_1}, {SDL_SCANCODE_2, DIK_2},
        {SDL_SCANCODE_3, DIK_3}, {SDL_SCANCODE_4, DIK_4},
        {SDL_SCANCODE_5, DIK_5}, {SDL_SCANCODE_6, DIK_6},
        {SDL_SCANCODE_7, DIK_7}, {SDL_SCANCODE_8, DIK_8},
        {SDL_SCANCODE_9, DIK_9}, {SDL_SCANCODE_0, DIK_0},
        {SDL_SCANCODE_MINUS, DIK_MINUS}, {SDL_SCANCODE_EQUALS, DIK_EQUALS},
        {SDL_SCANCODE_BACKSPACE, DIK_BACK},
        {SDL_SCANCODE_TAB, DIK_TAB},
        {SDL_SCANCODE_Q, DIK_Q}, {SDL_SCANCODE_W, DIK_W},
        {SDL_SCANCODE_E, DIK_E}, {SDL_SCANCODE_R, DIK_R},
        {SDL_SCANCODE_T, DIK_T}, {SDL_SCANCODE_Y, DIK_Y},
        {SDL_SCANCODE_U, DIK_U}, {SDL_SCANCODE_I, DIK_I},
        {SDL_SCANCODE_O, DIK_O}, {SDL_SCANCODE_P, DIK_P},
        {SDL_SCANCODE_RETURN, DIK_RETURN},
        {SDL_SCANCODE_LCTRL, DIK_LCONTROL},
        {SDL_SCANCODE_A, DIK_A}, {SDL_SCANCODE_S, DIK_S},
        {SDL_SCANCODE_D, DIK_D}, {SDL_SCANCODE_F, DIK_F},
        {SDL_SCANCODE_G, DIK_G}, {SDL_SCANCODE_H, DIK_H},
        {SDL_SCANCODE_J, DIK_J}, {SDL_SCANCODE_K, DIK_K},
        {SDL_SCANCODE_L, DIK_L},
        {SDL_SCANCODE_LSHIFT, DIK_LSHIFT},
        {SDL_SCANCODE_Z, DIK_Z}, {SDL_SCANCODE_X, DIK_X},
        {SDL_SCANCODE_C, DIK_C}, {SDL_SCANCODE_V, DIK_V},
        {SDL_SCANCODE_B, DIK_B}, {SDL_SCANCODE_N, DIK_N},
        {SDL_SCANCODE_M, DIK_M},
        {SDL_SCANCODE_SPACE, DIK_SPACE},
        {SDL_SCANCODE_CAPSLOCK, DIK_CAPSLOCK},
        {SDL_SCANCODE_F1, DIK_F1}, {SDL_SCANCODE_F2, DIK_F2},
        {SDL_SCANCODE_F3, DIK_F3}, {SDL_SCANCODE_F4, DIK_F4},
        {SDL_SCANCODE_F5, DIK_F5}, {SDL_SCANCODE_F6, DIK_F6},
        {SDL_SCANCODE_F7, DIK_F7}, {SDL_SCANCODE_F8, DIK_F8},
        {SDL_SCANCODE_F9, DIK_F9}, {SDL_SCANCODE_F10, DIK_F10},
        {SDL_SCANCODE_F11, DIK_F11}, {SDL_SCANCODE_F12, DIK_F12},
        {SDL_SCANCODE_LEFTBRACKET, DIK_LBRACKET},
        {SDL_SCANCODE_RIGHTBRACKET, DIK_RBRACKET},
        {SDL_SCANCODE_SEMICOLON, DIK_SEMICOLON},
        {SDL_SCANCODE_APOSTROPHE, DIK_APOSTROPHE},
        {SDL_SCANCODE_GRAVE, DIK_GRAVE},
        {SDL_SCANCODE_BACKSLASH, DIK_BACKSLASH},
        {SDL_SCANCODE_COMMA, DIK_COMMA},
        {SDL_SCANCODE_PERIOD, DIK_PERIOD},
        {SDL_SCANCODE_SLASH, DIK_SLASH},
        {SDL_SCANCODE_RSHIFT, DIK_RSHIFT},
        {SDL_SCANCODE_LALT, DIK_LALT},
        {SDL_SCANCODE_NUMLOCKCLEAR, DIK_NUMLOCK},
        {SDL_SCANCODE_SCROLLLOCK, DIK_SCROLL},
        {SDL_SCANCODE_KP_7, DIK_NUMPAD7},
        {SDL_SCANCODE_KP_8, DIK_NUMPAD8},
        {SDL_SCANCODE_KP_9, DIK_NUMPAD9},
        {SDL_SCANCODE_KP_MINUS, DIK_SUBTRACT},
        {SDL_SCANCODE_KP_4, DIK_NUMPAD4},
        {SDL_SCANCODE_KP_5, DIK_NUMPAD5},
        {SDL_SCANCODE_KP_6, DIK_NUMPAD6},
        {SDL_SCANCODE_KP_PLUS, DIK_ADD},
        {SDL_SCANCODE_KP_1, DIK_NUMPAD1},
        {SDL_SCANCODE_KP_2, DIK_NUMPAD2},
        {SDL_SCANCODE_KP_3, DIK_NUMPAD3},
        {SDL_SCANCODE_KP_0, DIK_NUMPAD0},
        {SDL_SCANCODE_KP_PERIOD, DIK_DECIMAL},
        {SDL_SCANCODE_KP_ENTER, DIK_NUMPADENTER},
        {SDL_SCANCODE_RCTRL, DIK_RCONTROL},
        {SDL_SCANCODE_KP_DIVIDE, DIK_DIVIDE},
        {SDL_SCANCODE_PRINTSCREEN, DIK_SYSRQ},
        {SDL_SCANCODE_RALT, DIK_RALT},
        {SDL_SCANCODE_HOME, DIK_HOME},
        {SDL_SCANCODE_UP, DIK_UP},
        {SDL_SCANCODE_PAGEUP, DIK_PRIOR},
        {SDL_SCANCODE_LEFT, DIK_LEFT},
        {SDL_SCANCODE_RIGHT, DIK_RIGHT},
        {SDL_SCANCODE_END, DIK_END},
        {SDL_SCANCODE_DOWN, DIK_DOWN},
        {SDL_SCANCODE_PAGEDOWN, DIK_NEXT},
        {SDL_SCANCODE_INSERT, DIK_INSERT},
        {SDL_SCANCODE_DELETE, DIK_DELETE},
        {SDL_SCANCODE_LGUI, DIK_LWIN},
        {SDL_SCANCODE_RGUI, DIK_RWIN},
        {SDL_SCANCODE_APPLICATION, DIK_APPS}
    };

    auto it = map.find(sc);
    return (it != map.end()) ? it->second : -1;
}

//////////////////////////////////

int migfly()
{
	m_pfullpane->StartFlying();
	View3d* myView = Launch3d(false);

	SDL_Event event;
	bool running = true;
	while (running)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				running = false;
			if (event.type == SDL_KEYDOWN) {
 			   int dik = SDLScancodeToDIK(event.key.keysym.scancode);
    			if (dik != -1) myView->inst->OnKeyDown(dik);
			}
			if (event.type == SDL_KEYUP) {
    			int dik = SDLScancodeToDIK(event.key.keysym.scancode);
    			if (dik != -1) myView->inst->OnKeyUp(dik);
			}
		}
		if (myView->drawloop((LPVOID) myView) == 1) {
			running = false;
		}
	}

	migdestroy();
	return 0;
}

int migdestroy()
{
	//Clean up our objects and quit
	SDL_DestroyTexture(Master_3d.SdlMigTexture);
	Master_3d.SdlMigTexture = nullptr;
	SDL_DestroyWindow(Master_3d.SdlWinInst);
	Master_3d.SdlWinInst = nullptr;
	global_hWnd = nullptr;
	TTF_Quit();
	SDL_Quit();
	return 0;
}

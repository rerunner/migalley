/*
	 MiG Alley
	 Copyright (C) 1998, 1999, 2000, 2001 Empire Interactive (Europe) Ltd,
	 677 High Road, North Finchley, London N12 0DA

	 Please see the document licence.doc for the full licence agreement

2. LICENCE
 2.1 	
 	Subject to the provisions of this Agreement we now grant to you the 
 	following rights in respect of the Source Code:
  2.1.1 
  	the non-exclusive right to Exploit  the Source Code and Executable 
  	Code on any medium; and 
  2.1.2 
  	the non-exclusive right to create and distribute Derivative Works.
 2.2 	
 	Subject to the provisions of this Agreement we now grant you the
	following rights in respect of the Object Code:
  2.2.1 
	the non-exclusive right to Exploit the Object Code on the same
	terms and conditions set out in clause 3, provided that any
	distribution is done so on the terms of this Agreement and is
	accompanied by the Source Code and Executable Code (as
	applicable).

3. GENERAL OBLIGATIONS
 3.1 
 	In consideration of the licence granted in clause 2.1 you now agree:
  3.1.1 
	that when you distribute the Source Code or Executable Code or
	any Derivative Works to Recipients you will also include the
	terms of this Agreement;
  3.1.2 
	that when you make the Source Code, Executable Code or any
	Derivative Works ("Materials") available to download, you will
	ensure that Recipients must accept the terms of this Agreement
	before being allowed to download such Materials;
  3.1.3 
	that by Exploiting the Source Code or Executable Code you may
	not impose any further restrictions on a Recipient's subsequent
	Exploitation of the Source Code or Executable Code other than
	those contained in the terms and conditions of this Agreement;
  3.1.4 
	not (and not to allow any third party) to profit or make any
	charge for the Source Code, or Executable Code, any
	Exploitation of the Source Code or Executable Code, or for any
	Derivative Works;
  3.1.5 
	not to place any restrictions on the operability of the Source 
	Code;
  3.1.6 
	to attach prominent notices to any Derivative Works stating
	that you have changed the Source Code or Executable Code and to
	include the details anddate of such change; and
  3.1.7 
  	not to Exploit the Source Code or Executable Code otherwise than
	as expressly permitted by  this Agreement.

questions about this file may be asked at http://www.simhq.com/
*/

#include "_HARD.H"

//------------------------------------------------------------------------------
//Filename       win3d.cpp
//System         
//Author         Paul
//Date           Thu 30 Jan 1997
//Description    
//------------------------------------------------------------------------------

#define INWIN3D
#include	"DOSDEFS.H"
#define F_GRAFIX
#define F_SOUNDS
#define F_BATTLE
#include	"FILES.H"
#include "_HARD.H" // RERUN
#include "DISPLAY.H" // RERUN
#include	"MYERROR.H"
#include	"MYTIME.H"
#include	"WORLDINC.H"
#include	"LANDSCAP.H"
#include	"MIGLAND.H"
#include	"SAVEGAME.H"
#include 	"MISSMAN2.H"
#include	"VIEWSEL.H"
#include	"VERTEX.H"
#include	"3DDEFS.H"
#include	"3DCODE.H"
#include	"MYMATH.H"
#include	"HDATA.H"
#include	"HARDPASM.H"
#include	"WIN3D.H"
#include "MigOpenGL.h"
#include	"GAMESET.H"
#include	"TILEMAKE.H"
#include "AREATYPE.H"
#include "PROF.H"
#include "MILES.H" // RERUN
#include "WinMig3D.H" //SDL

#include "XX_ScreenFlip_Vulkan.h" // Vulkan

TTF_Font * sdlTextFont; //RERUN global font for text rendering

//RERUN #define DDSCAPS_LOCALVIDMEM 0x00000000l

UByte videoBuffer =0;	//OFF==0 ON==1;

bool mono3d=false;


inline void ZeroMem(void* pp,ULong ll)
{
#if defined (__MSVC__)
	_asm{
	push eax
	push edi
	push ebx
	push ecx
	xor	eax,eax;
	mov edi,pp;
	mov ebx,ll;
	mov	ecx,4;
	sub	ecx,edi;
	and	ecx,3;
	sub	ebx,ecx;
	rep	stosb;
	mov	ecx,ebx;
	shr	ecx,2;
	rep	stosd;
	and	ebx,3;
	mov	ecx,ebx;
	rep	stosb;
	pop ecx
	pop ebx
	pop edi
	pop eax
	}
#else
	std::memset(pp, 0, ll);
#endif
}

typedef	struct _DirectDraw DirectD,*DirectDP;

#include	"WIN3D.H"

#include	"IMAGEMAP.G"

extern	void fpSqrt(Float& );

#ifdef __MSVC__
inline Float fpSqrt(Float num)
{
    __asm
    {
		fld 	num
		fsqrt
		fstp	num
    }
	return num;
}
#else
inline Float fpSqrt(Float num)
{
	return std::sqrt(num);
}
#endif

extern	void fpSin_Cos(Angles angle, Float& sin_ang, Float& cos_ang);

#ifdef __MSVC__
inline void fpSin_Cos(Angles angle, Float& sin_ang, Float& cos_ang)
{
   __asm
    {
		push eax
		push edx
		push ebx
	   mov eax,angle;
	   mov edx,sin_ang;
	   mov ebx,cos_ang;
		and		eax,0x0000FFFF;
		fldpi;
		push	eax;
		fild	dword ptr ss:[esp];
		fmulp	st(1),st;
		mov		dword ptr ss:[esp],32768;
		fild	dword ptr ss:[esp];
		fdivp	st(1),st;
		fsincos;
		fstp	qword ptr ds:[ebx];
		fstp	qword ptr ds:[edx];
		add		esp,4;
		pop ebx
		pop edx
		pop eax
    }
}

#endif

#ifdef __MSVC__
inline void	ASM_Blat2(void* src,void* dst,ULong len)
{
	_asm{
	push	eax
	push	ecx
	push	esi
	push	edi
	mov		esi,src
	mov		edi,dst
	mov		ebx,len
	mov		ecx,4
	sub		ecx,edi
	and		ecx,3
	sub		ebx,ecx
	rep		movsb
	mov		ecx,ebx
	shr		ecx,2
	rep		movsd
	and		ebx,3
	mov		ecx,ebx
	rep		movsb
	pop		edi
	pop		esi
	pop		ecx
	pop		eax
	}
}
#endif

#if !defined(NDEBUG)
#define FLUSH(p1) Flush(p1)
#else
#define FLUSH(p1)
#endif

//------------------------------------------------------------------------------
//Procedure		GreyPalette
//Author		Paul.   
//Date			Wed 3 Mar 1999
//------------------------------------------------------------------------------
void GreyPalette(UByte* rgb)
{
	if (mono3d)
	{
 		UByte r=rgb[0];
		r=r<rgb[1]?rgb[1]:r;
		r=r<rgb[2]?rgb[2]:r;
		rgb[0]=rgb[1]=rgb[2]=r;
	}
}

void GreyPalette(UByte* r,UByte* g,UByte* b)
{
	if (mono3d)
	{
		UByte local[3];
		local[0]=*r;
		local[1]=*g;
		local[2]=*b;
		GreyPalette(local);
		*r=local[0];
		*g=local[0];
		*b=local[0];
	}
}

#include "MONOTXT.H"

extern ViewData	viewdata; //,saveviewdata;

//SLong	executecount;

#ifndef	__FAR_LAND__											//PD 22Aug97
const	int	maxLevel = 5;										//PD 22Aug97
#else															//PD 22Aug97
const	int	maxLevel = 6;										//PD 22Aug97
#endif															//PD 22Aug97

const	ULong	TWO_MEG = 2*1024*1024;
const	ULong	FOUR_MEG = 4*1024*1024;
const	ULong	EIGHT_MEG = 8*1024*1024;

const	Float	SZ_SCALE = 4.0;
const	Float	RHW_SCALE= 10.0;

int driverCount=0;
int numModes=0;

char dn00[64], dn01[64], dn02[64], dn03[64];
char dn04[64], dn05[64], dn06[64], dn07[64];
char dn08[64], dn09[64], dn0A[64], dn0B[64];
char dn0C[64], dn0D[64], dn0E[64], dn0F[64];

char* driverNames[]=
{
	dn00,dn01,dn02,dn03,
	dn04,dn05,dn06,dn07,
	dn08,dn09,dn0A,dn0B,
	dn0C,dn0D,dn0E,dn0F
};

ULong driverRAM[64];

SDrvrModes driverModes[128];

//------------------------------------------------------------------------------
//Procedure		GetDrivers
//Author		Paul.   
//Date			Wed 9 Dec 1998
//------------------------------------------------------------------------------
void GetDrivers(int& dc,char**& dn)
{
	dc=driverCount;
	dn=driverNames;
}

//------------------------------------------------------------------------------
//Procedure		GetModes
//Author		Paul.   
//Date			Wed 9 Dec 1998
//------------------------------------------------------------------------------
void GetModes(int& nm,SDrvrModes*& dm)
{
	nm=numModes;
	dm=driverModes;
}

//------------------------------------------------------------------------------
//Procedure		GetDriverAndMode
//Author		Paul.   
//Date			Thu 21 Jan 1999
//------------------------------------------------------------------------------
void GetDriverAndMode(int& driverIndex,int& modeIndex,int& this_width,int& this_height)
{
	SLong i,j,k;

	if (driverCount>0)
	{
		//there is more than just the primary
		//display driver

		for (i=driverCount;i-->=0;)
		{
		 	if (driverRAM[i]>=800*600*3*2)
				for (j=0,k=0;j<numModes;j++)
					if (driverModes[j].driverNo-1==i &&
						driverModes[j].displayBPP==16)
					{
						//k++;
						if (driverModes[j].displayWidth==800 &&
							driverModes[j].displayHeight==600)
						{
							driverIndex=i+1;
							modeIndex=k;
							this_width=800;
							this_height=600;
							return;
						}
						k++;
					}

			for (j=0,k=0;j<numModes;j++)
				if (driverModes[j].driverNo-1==i &&
					driverModes[j].displayBPP==16)
				{
					//k++;
					if (driverModes[j].displayWidth==640 &&
						driverModes[j].displayHeight==480)
					{
						driverIndex=i+1;
						modeIndex=k;
						this_width=640;
						this_height=480;
						return;
					}
					k++;
				}
		}
	}

	k=0;
	for (j=0;j<numModes;j++)
		if (driverModes[j].driverNo==0 &&
			driverModes[j].displayBPP==16)
		{
			//k++;
			if (driverModes[j].displayWidth==640 &&
				driverModes[j].displayHeight==480)
			{
				driverIndex=0;
				modeIndex=k;
				this_width=640;
				this_height=480;
				return;
			}
			k++;
		}

	k=0;
	for (j=0;j<numModes;j++)
		if (driverModes[j].driverNo==0 &&
			driverModes[j].displayBPP==16)
		{
			//k++;
			if (driverModes[j].displayWidth==512 &&
				driverModes[j].displayHeight==384)
			{
				driverIndex=0;
				modeIndex=k;
				this_width=512;
				this_height=384;
				return;
			}
			k++;
		}

	driverIndex=modeIndex=0;
	this_width=640;
	this_height=480;
}



#define CLIP_XVAL(xval)\
			if ((xval)<viewdata.clipLft)		(xval)=viewdata.clipLft;\
			else if ((xval)>viewdata.clipRgt)	(xval)=viewdata.clipRgt;	

#define CLIP_YVAL(yval)\
			if ((yval)<viewdata.clipTop)		(yval)=viewdata.clipTop;\
			else if ((yval)>viewdata.clipBot)	(yval)=viewdata.clipBot;

#define LCLIP_YVAL(yval)\
			if ((yval)<viewdata.clipTop+1.0)(yval)=viewdata.clipTop+1.0;\
			else if ((yval)>viewdata.clipBot-1.0)(yval)=viewdata.clipBot-1.0;

#define LCLIP_XVAL(xval)\
			if ((xval)<viewdata.clipLft+1.0)(xval)=viewdata.clipLft+1.0;\
			else if ((xval)>viewdata.clipRgt-1.0)(xval)=viewdata.clipRgt-1.0;

#define CLAMP(val)	if ((val)>-.00001 && (val)<.00001) (val)=.0

enum	WhichDirection											//PD 12Sep97
{
	wd_NULL = 	0x0000,
	wd_N	=	0x0001,		wd_S	=	0x0002,
	wd_E	=	0x0004,		wd_W	=	0x0008,
	wd_NW	=	wd_N+wd_W,	wd_NE	=	wd_N+wd_E,
	wd_SW	= 	wd_S+wd_W,	wd_SE	=	wd_S+wd_E
};

extern	SWord	winmode_w;
extern	SWord	winmode_h;
extern	SWord	winmode_bpp;

//------------------------------------------------------------------------------
//Procedure		D3DErrorCodes
//Author		Paul.   
//Date			Thu 6 Feb 1997
//
//Description	Translate random number into error string
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void D3DErrorCodes(const char* file,int lineno,HRESULT dderr)
{
}

//------------------------------------------------------------------------------
//Procedure		direct_draw
//Author		Paul.   
//Date			Fri 31 Jan 1997
//
//Description	Replacement for old 'DirectDrawCreate' call that
//				will scan available drivers for a 3D capeable one
//				and use that in preference to any others
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
CON	direct_draw::direct_draw()
{
	SLong q;
	HRESULT		dderrval;
	LPDIRECT3D	lpD3D;

	lockLevel=0;
	gotShifts=false;

	lpDD=NULL;
	lpD3D=NULL;
	NumDrivers=0;
	CurrDriver=0;
	softpallookup=NULL;

	//Search for a 3D capable DirectDraw hardware driver and create it.
#if 0 // RERUN
	dderrval = DirectDrawEnumerate(enumDDDrivers,(LPVOID)this);
#else
	dderrval = DD_OK;
#endif

	if (dderrval!=DD_OK)	lpDD=NULL;

	NumModes=0;

	for (q=0;q<NumDrivers && q<16;q++)
	{
		strcpy(driverNames[q],Drivers[q].About);
		driverRAM[q]=Drivers[q].vidMem;
	}
	driverCount=NumDrivers;

	//Scan for additional modes with no hardware support

	CurrDriver=-1;

	bPrimaryDisplayDriverDoesHw3D = FALSE;
	bButNotVeryWell	= FALSE;
	bFogVertexMode	= FALSE;									//PD 08Sep97
	bFogTableMode	= FALSE;									//PD 08Sep97

#if USE_SDL_PRESENTATION_PATH
	lpDD = SDL_CreateRenderer(Master_3d.SdlWinInst, -1, 0);
	if (lpDD == nullptr)
	{
		printf("SDL_CreateRenderer Failed!\n");
		assert(1);
	}
	
	lpDD2 = lpDD;
	winmode_bpp = 16; // winmode_w and winmode_h are set in MigAlley.cpp
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(lpDD, winmode_w, winmode_h);
	Master_3d.SdlMigTexture = SDL_CreateTexture(lpDD2, SDL_PIXELFORMAT_RGB565,
			SDL_TEXTUREACCESS_STREAMING,
			winmode_w, winmode_h); //a single SDL_Texture that will represent the screen.
#else
	lpDD2 = lpDD = nullptr;
	winmode_bpp = 16; // winmode_w and winmode_h are set in MigAlley.cpp

	// Think of Vulkan as a presenter, not a renderer.
	// 1) Create Vulkan instance, also
	//    - Create swapchain
	//    - Create one persistent VkImage for the game framebuffer

	Vulkan_Init(Master_3d.SdlWinInst);

	CreateRenderPass(); // RERUN CHECKED OK
	CreateFramebuffers();
	createDescriptorSetLayoutAndSampler();
    createPipelineLayout();

	Vulkan_CreatePipeline();

	// 2) Create Vulkan surface from SDL window
	Vulkan_CreateSDLSurface(Master_3d.SdlWinInst);
#endif

	sdlTextFont = TTF_OpenFont("/usr/share/fonts/TTF/DejaVuSansMono.ttf", 16);
	if (sdlTextFont == NULL)
	{
		printf("TTF_OpenFont Failed!\n");
		assert(1);
	}

	for (q=0;q<NumModes && q<128;q++)
	{
		driverModes[q].driverNo=Modes[q].Driver+1;
		driverModes[q].displayWidth=Modes[q].Width;
		driverModes[q].displayHeight=Modes[q].Height;
		driverModes[q].displayBPP=Modes[q].Bpp;
	}
	numModes=NumModes;

	//Determine which modes can be supported based on the
	//ammount of video memory available

	int counter;

	//Initially need a driver that supports the current 
	//windows mode front end of the game

	fullScreenRequired=false;

	//first, check for a full screen 640x480 
	//hardware accelerated mode NOT using primary
	//display device (ie 3dfx)

	BOOL bFullScreenRequested=gameSettings.m_bFullScreenMode;

	if (bFullScreenRequested)
	{
		gameSettings.m_dwWidth=Save_Data.displayW;
		gameSettings.m_dwHeight = Save_Data.displayH;
	}

	DWORD dwWidth=gameSettings.m_dwWidth;
	DWORD dwHeight=gameSettings.m_dwHeight;

	if (bFullScreenRequested)
	{
		for (counter=0;counter<NumModes;counter++){
			if (Modes[counter].Width==dwWidth &&
				Modes[counter].Height==dwHeight &&
				Modes[counter].Bpp==16 &&
				Modes[counter].Driver==Save_Data.dddriver){
				//now check for hardware support in this mode
				int possDriver=Modes[counter].Driver;
				if (Drivers[possDriver].bHardware3D)
				{
					fullScreenRequired=true;
					break;
				}
			}
		}
	}
	else counter=NumModes;

	if (counter==NumModes)
	{
		if (bFullScreenRequested)
		{
			for (counter=0;counter<NumModes;counter++)
			{
				if (Modes[counter].Width==dwWidth &&
					Modes[counter].Height==dwHeight &&
					Modes[counter].Bpp==16 &&
					Modes[counter].Driver==-1 &&
					bPrimaryDisplayDriverDoesHw3D)
				{
					fullScreenRequired=true;
					break;
				}
			}
		}

		if (counter==NumModes)
		{
			for (counter=0;counter<NumModes;counter++){
				if (Modes[counter].Width==winmode_w &&
					Modes[counter].Height==winmode_h &&
					Modes[counter].Bpp==winmode_bpp &&
					Modes[counter].Driver==-1)
					break;
			}
			if (counter==NumModes)
			{
				if (winmode_bpp==8){
					if (counter==D3DAPP_MAXD3DMODES)
						counter--;
					Modes[counter].Width=winmode_w;
					Modes[counter].Height=winmode_h;
					Modes[counter].Bpp=8;
					Modes[counter].Driver=-1;
					NumModes=counter+1;
				}
			}
		}
	}

	//Obtain a DirectDraw2 interface
	if (lpDD!=NULL)
	{
		lpDD2 = lpDD; 
	}
}

//------------------------------------------------------------------------------
//Procedure		enum3DDevices
//Author		Paul.   
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
HRESULT WINAPI
direct_draw::enum3DDevices(	LPGUID 			lpGuid,
							LPSTR 			lpDeviceDescription,
                      		LPSTR 			lpDeviceName,
							LPD3DDEVICEDESC lpHWDesc,
                      		LPD3DDEVICEDESC lpHELDesc,
							LPVOID 			lpContext)
{
#if 0 //RERUN not needed for SDL
	DirectDDP	directddp;

    directddp = (DirectDDP)lpContext;

    if (lpHWDesc->dcmColorModel)
	{
       	directddp->CanDoHardware3D=1;
		directddp->bPrimaryDisplayDriverDoesHw3D=TRUE;

		if (lpHWDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX)
		{
			directddp->bFogVertexMode=TRUE;
		}
		else if	(lpHWDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE)
		{
			directddp->bFogTableMode=TRUE;
		}
		//Alpha based transparency  / Color key test

		if (!directddp->bButNotVeryWell)
		{
		if ((lpHWDesc->dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) &&
			(lpHWDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA) &&
			(lpHWDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA))
		{
			directddp->bButNotVeryWell=FALSE;
		}
		else if (lpHWDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_TRANSPARENCY)//PD 08Oct97
		{
			directddp->bButNotVeryWell=FALSE;
		}
		else directddp->bButNotVeryWell=TRUE;
		}
    }
#endif
    return (D3DENUMRET_OK);
}

//------------------------------------------------------------------------------
//Procedure		direct_draw
//Author		Paul.   
//Date			Fri 31 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
DES direct_draw::~direct_draw()
{
	if (softpallookup) delete[]softpallookup,softpallookup=NULL;

	HRESULT	dderrval;

	RELEASE_RENDERER(lpDD2, dderrval);

	RELEASE_RENDERER(lpDD, dderrval);
}

//------------------------------------------------------------------------------
//Procedure		NameNuberedDriver
//Author		Paul.   
//Date			Fri 23 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
UByte*	direct_draw::NameNumberedDriver(SWord drvno)
{
	if (drvno<NumDrivers && drvno>=0)
		return ((UByte*)Drivers[drvno].About);
	 
	return (NULL);	
}

//------------------------------------------------------------------------------
//Procedure		ModeSupported
//Author		Paul.   
//Date			Thu 29 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool direct_draw::ModeSupported(ScreenRez* lpSRez)
{
	ULong	memForUse,memRequired;

	memForUse =	(lpSRez->driver==-1)?
				primaryDisplayDriverMem:
				Drivers[lpSRez->driver].vidMem;

	SLong	displayWidth, displayHeight, displayBytesPerPixel;

	displayWidth = (lpSRez->width>0)?lpSRez->width:-lpSRez->width;

	displayHeight = displayWidth * 3/4;

	displayBytesPerPixel = lpSRez->colourdepth>>3;

	if (displayBytesPerPixel==1)	return (TRUE);

	if (displayBytesPerPixel!=2)	return (FALSE);

	//Need at least 2 screens + Z-buffer

	memRequired = 2 * displayWidth * displayHeight * displayBytesPerPixel;

	memRequired+= 2 * displayWidth * displayHeight;

	//Need room for ground textures
	//And piloted a/c textures

	if (lpSRez->driver!=-1)
	{
		memRequired+= 1024*1024;								//PD 24Jun97
		memRequired += 1024*1024;								//PD 24Jun97
	}
	else
	{
		memRequired+=0x23E000;									//PD 08Sep97
	}

	if (memForUse>=memRequired)	return (TRUE);

	return (FALSE);
}

//------------------------------------------------------------------------------
//Procedure		DriverIsCapable
//Author		Paul.   
//Date			Thu 29 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool direct_draw::DriverIsCapable(SWord drvno)
{
	if (drvno==-1 && bPrimaryDisplayDriverDoesHw3D==TRUE && bButNotVeryWell==TRUE)
		return (FALSE);

	return (TRUE);
}

//------------------------------------------------------------------------------
//Procedure		CountDrivers
//Author		Paul.   
//Date			Tue 27 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
SWord direct_draw::CountDrivers()
{
	SWord	retval;

	retval = NumDrivers;

	for (int i=0;i<NumModes;i++)
		if (Modes[i].Driver==-1 && Modes[i].Bpp!=8)
		{
			retval -= 1000;
			break;
		}

	return (retval);
}

//------------------------------------------------------------------------------
//Procedure		DriverDoesHardware3D
//Author		Paul.   
//Date			Thu 27 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
BOOL direct_draw::DriverDoesHardware3D()
{
	int	driverno = Modes[CurrMode].Driver;

	if (driverno==-1)	return (bPrimaryDisplayDriverDoesHw3D);

	return (Drivers[driverno].bHardware3D);
}

//------------------------------------------------------------------------------
//Procedure		GetBltCaps
//Author		Paul.   
//Date			Tue 21 Jul 1998
//------------------------------------------------------------------------------
void direct_draw::GetBltCaps(bool& dab,bool& cbsm)
{
	int driverno=Modes[CurrMode].Driver;

	if (driverno==-1)
	{
		dab=(primaryDoesAsyncBlt!=0)?true:false;
		cbsm=(primaryCanBltSysMem!=0)?true:false;
	}
	else
	{
		dab=(Drivers[driverno].bDoesAsyncBlt!=0)?true:false;
		cbsm=(Drivers[driverno].bCanBltSysMem!=0)?true:false;
	}
}

//------------------------------------------------------------------------------
//Procedure		enumDDDrivers
//Author		Paul.   
//Date			Fri 31 Jan 1997
//
//Description	Callback function used during enumeration of DirectDraw drivers.
//				During enumeration, if a 3D capable hardware device is found, it 
//				is created and *(LPDIRECTDRAW*)lpContext is set to it.  
//				Otherwise, does nothing.
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
BOOL FAR PASCAL
direct_draw::enumDDDrivers(	GUID FAR* lpGUID,	LPSTR lpDriverDesc,
							LPSTR lpDriverName, LPVOID lpContext)
{
    return (DDENUMRET_OK);
}

//------------------------------------------------------------------------------
//Procedure		enumDisplayModes
//Author		Paul.   
//Date			Mon 3 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
HRESULT WINAPI
direct_draw::enumDisplayModes(	LPDDSURFACEDESC lpDDSurfaceDesc,
								LPVOID			lpContext)
{
	return (DDENUMRET_OK);
}

//------------------------------------------------------------------------------
//Procedure		GetModes
//Author		Paul.   
//Date			Tue 4 Feb 1997
//
//Description	Copy available mode list back to old
//				structure	
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void direct_draw::GetModes(DirectD& dd)
{
	dd.cModes=NumModes;

	for (int counter=0;counter<NumModes;counter++)
	{
		dd.DDModes[counter].width=Modes[counter].Width;
		dd.DDModes[counter].height=Modes[counter].Height;
		dd.DDModes[counter].colourdepth=Modes[counter].Bpp;
		dd.DDModes[counter].driver=Modes[counter].Driver;
	}
}

//------------------------------------------------------------------------------
//Procedure		SetMode
//Author		Paul.   
//Date			Mon 3 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns		TRUE = driver changed 
//				FALSE= driver *NOT* changed
//
//------------------------------------------------------------------------------
BOOL direct_draw::SetMode(int mode)
{
	return FALSE;
}

static TWODEFFECT tdeffect=TDE_UNIFORM;

#define _makeRecIndex(a,b)\
		(a)>=(b)?\
		((b)*recordsused+(a)):\
		((a)*recordsused+(b));

#define _makeSimpleIndex(a,b)\
		(b)*recordsused+(a);

///////////////////////////////////////////////////////////////////////////////
//
//	Clipping macros
//
///////////////////////////////////////////////////////////////////////////////

#define lrTest(testBx,testBz)\
		(testBx)>(testBz)?\
		CF3D_OFFRIGHT:\
		((testBx)<0.&&-(testBx)>(testBz))?\
		CF3D_OFFLEFT:\
		CF3D_NULL

#define tbTest(testBy,testBz)\
		(testBy)>(testBz)?\
		CF3D_OFFTOP:\
		((testBy)<0.&&-(testBy)>(testBz))?\
		CF3D_OFFBOTTOM:\
		CF3D_NULL


//------------------------------------------------------------------------------
//Procedure		clipRGBA
//Author		Paul.   
//Date			Mon 29 Jun 1998
//
//------------------------------------------------------------------------------
inline void clipRGBA(R3DCOLOR inc,R3DCOLOR outc,R3DCOLOR* clipc,double frac)
{
	if (inc==outc)
	{
		*clipc=inc;
	}
	else
	{
		SLong ifrac=SLong(frac*65536);

		//clip red component

		if (inc.r==outc.r)
			clipc->r=inc.r;
		else
		{
			SLong newr=SLong(inc.r)+(((SLong(outc.r)-SLong(inc.r))*ifrac)>>16);
			clipc->r=newr<0?UByte(0):newr>255?UByte(255):UByte(newr);
		}

		//clip green component

		if (inc.g==outc.g)
			clipc->g=inc.g;
		else
		{
			SLong newg=SLong(inc.g)+(((SLong(outc.g)-SLong(inc.g))*ifrac)>>16);
			clipc->g=newg<0?UByte(0):newg>255?UByte(255):UByte(newg);
		}

		//clip blue component

		if (inc.b==outc.b)
			clipc->b=inc.b;
		else
		{
			SLong newb=SLong(inc.b)+(((SLong(outc.b)-SLong(inc.b))*ifrac)>>16);
			clipc->b=newb<0?UByte(0):newb>255?UByte(255):UByte(newb);
		}

		//clip alpha component

		if (inc.a==outc.a)
			clipc->a=inc.a;
		else
		{
			SLong newa=SLong(inc.a)+(((SLong(outc.a)-SLong(inc.a))*ifrac)>>16);
			clipc->a=newa<0?UByte(0):newa>255?UByte(255):UByte(newa);
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		clipRGB
//Author		Paul.   
//Date			Mon 29 Jun 1998
//
//------------------------------------------------------------------------------
inline void clipRGB(R3DCOLOR inc,R3DCOLOR outc,R3DCOLOR* clipc,double frac)
{
	if (inc==outc)
	{
		*clipc=inc;
	}
	else
	{
		SLong ifrac=SLong(frac*65536);

		//clip red component

		if (inc.r==outc.r)
			clipc->r=inc.r;
		else
		{
			SLong newr=SLong(inc.r)+(((SLong(outc.r)-SLong(inc.r))*ifrac)>>16);
			clipc->r=newr<0?UByte(0):newr>255?UByte(255):UByte(newr);
		}

		//clip green component

		if (inc.g==outc.g)
			clipc->g=inc.g;
		else
		{
			SLong newg=SLong(inc.g)+(((SLong(outc.g)-SLong(inc.g))*ifrac)>>16);
			clipc->g=newg<0?UByte(0):newg>255?UByte(255):UByte(newg);
		}

		//clip blue component

		if (inc.b==outc.b)
			clipc->b=inc.b;
		else
		{
			SLong newb=SLong(inc.b)+(((SLong(outc.b)-SLong(inc.b))*ifrac)>>16);
			clipc->b=newb<0?UByte(0):newb>255?UByte(255):UByte(newb);
		}
	}
}



//------------------------------------------------------------------------------
//Procedure		DOCLIP
//Author		Paul.   
//Date			Tue 30 Jun 1998
//------------------------------------------------------------------------------
#define DOCLIP(srcpoly,srcindex,srcnext,dstpoly,dstindex,flag,cliprtn)\
if (!((srcpoly)[(srcindex)]->clipFlags&(flag)))\
{\
	(dstpoly)[(dstindex)++]=(srcpoly)[(srcindex)];\
	if ((srcpoly)[(srcnext)]->clipFlags&(flag))\
	{\
		(cliprtn)((srcpoly)[(srcindex)],(srcpoly)[(srcnext)],ptltemp);\
		if (!(ptltemp->clipFlags&CF3D_ALL))\
		{\
			*ptlvert=*ptltemp;\
			(dstpoly)[(dstindex)++]=ptlvert++;\
		}\
		else\
		{\
			(dstpoly)[(dstindex)++]=ptltemp++;\
		}\
	}\
}\
else if (!((srcpoly)[(srcnext)]->clipFlags&(flag)))\
{\
	(cliprtn)((srcpoly)[(srcnext)],(srcpoly)[(srcindex)],ptltemp);\
	if (!(ptltemp->clipFlags&CF3D_ALL))\
	{\
		*ptlvert=*ptltemp;\
		(dstpoly)[(dstindex)++]=ptlvert++;\
	}\
	else\
	{\
	  	(dstpoly)[(dstindex)++]=ptltemp++;\
	}\
}


//------------------------------------------------------------------------------
//Procedure		GetSurfaceDimensions
//Author		Jim Taylor
//Date			Mon 16 Nov 1998
//------------------------------------------------------------------------------
void direct_draw::GetSurfaceDimensions(DirectD* pdirectd,SLong& width,SLong& height)
{
	ROWANSURFACEDESC rsd;
	LockScr(pdirectd,&rsd);
	UnlockScr(pdirectd,&rsd);

	width = rsd.dwWidth = winmode_w;
	height = rsd.dwHeight = winmode_h;
}

extern int active_lines;

typedef struct tagSCREENRECORD
{
	struct tagSCREENRECORD* pNext;
	UByteP pBufferDat;
}
SCREENRECORD,*LPSCREENRECORD;

typedef struct tagVIDEOBUFFER
{
	enum {NUM_RECORDS=120};		//for 80Mb machine
//	enum {NUM_RECORDS=81};		//for 80Mb machine
//	enum {NUM_RECORDS=163};		//for 128Mb machine
//	enum {NUM_RECORDS=382};		//for 256Mb machine
	SLong noRecs;
	SLong frameCounter;
	SLong bufferSizeInBytes;
	SLong imageWidth;
	SLong imageHeight;
	ULong redMask;
	ULong greenMask;
	ULong blueMask;
	SLong redShift;
	SLong greenShift;
	SLong blueShift;
	SLong redBits;
	SLong greenBits;
	SLong blueBits;
	LPSCREENRECORD pHead,pTail;
	LPSCREENRECORD pBegin,pInsert;
}
VIDEOBUFFER,*LPVIDEOBUFFER;

static LPVIDEOBUFFER pVideoBuf=NULL;

//------------------------------------------------------------------------------
//Procedure		SetSmackerMode
//Author		Paul.   
//Date			Mon 15 Mar 1999
//------------------------------------------------------------------------------
void direct_draw::SetSmackerMode(bool flag,HWND)
{
}


void direct_draw::LockScr(struct _DirectDraw* pdirectd,ROWANSURFACEDESC* rsd)
{
	if (lockLevel==0)
	{
		HRESULT hr;
		DDSURFACEDESC sd;
		ZeroMem(&sd,sizeof(DDSURFACEDESC));
		sd.dwSize=sizeof(DDSURFACEDESC);

		if (pdirectd->lpDDSBack)
		{
			do{ hr = SDL_LockSurface(pdirectd->lpDDSBack); } while (hr != 0);
		}
		sd.lpSurface = pdirectd->lpDDSBack->pixels;
		sd.dwWidth = pdirectd->lpDDSBack->w; // 800;
		sd.dwHeight = pdirectd->lpDDSBack->h; // 600;
		sd.lPitch = pdirectd->lpDDSBack->pitch; //  3200; //??
		sd.ddpfPixelFormat.dwRGBBitCount = SDL_PIXELBITCOUNT; //RERUN actually 32

		rsdGlobal.lpSurface=sd.lpSurface;
		rsdGlobal.dwWidth=sd.dwWidth;
		rsdGlobal.dwHeight=sd.dwHeight;
		rsdGlobal.lPitch=sd.lPitch;
		rsdGlobal.dwRGBBitCount=sd.ddpfPixelFormat.dwRGBBitCount;
	}
	lockLevel++;
	*rsd=rsdGlobal;
}

void direct_draw::UnlockScr(struct _DirectDraw* pdirectd,ROWANSURFACEDESC* rsd)
{
	lockLevel--;
	if (lockLevel == 0)
	{
		if (pdirectd->lpDDSBack != NULL)
		{
			SDL_UnlockSurface(pdirectd->lpDDSBack);
		}
	}
}

void direct_draw::GetColourInfo(struct _DirectDraw* pdirectd,
								UWord& redMask,SWord& redShift1,SWord& redShift2,
								UWord& greenMask,SWord& greenShift1,SWord& greenShift2,
								UWord& blueMask,SWord& blueShift1,SWord& blueShift2)
{
	if (!gotShifts)
	{
		HRESULT hr;
		DDSURFACEDESC sd;
		ZeroMem(&sd,sizeof(DDSURFACEDESC));
		sd.dwSize=sizeof(DDSURFACEDESC);

		if (pdirectd->lpDDSBack != NULL)
		{
			do{ hr = SDL_LockSurface(pdirectd->lpDDSBack); } while (hr != 0);
			SDL_UnlockSurface(pdirectd->lpDDSBack);
		}
		sd.lpSurface = pdirectd->lpDDSBack->pixels;
		sd.dwWidth = pdirectd->lpDDSBack->w; // 800;
		sd.dwHeight = pdirectd->lpDDSBack->h; // 600;
		sd.lPitch = pdirectd->lpDDSBack->pitch; // 3200; //??
		sd.ddpfPixelFormat.dwRGBBitCount = SDL_PIXELBITCOUNT; //RERUN actually 32
		sd.ddpfPixelFormat.dwRBitMask = pdirectd->lpDDSBack->format->Rmask;
		sd.ddpfPixelFormat.dwGBitMask = pdirectd->lpDDSBack->format->Gmask;
		sd.ddpfPixelFormat.dwBBitMask = pdirectd->lpDDSBack->format->Bmask;

		DWORD	bits,shift,mask;

		for (shift = 0, mask = sd.ddpfPixelFormat.dwRBitMask; !(mask & 1); shift++, mask >>= 1);
		redShift1=swRedSh1=shift;
		redMask=swRedMask=mask;
		for (bits=0;mask&1;bits++,mask>>=1);
		redShift2=swRedSh2=8-bits;
		for (shift = 0, mask = sd.ddpfPixelFormat.dwGBitMask; !(mask & 1); shift++, mask >>= 1);
		greenShift1=swGreenSh1=shift;
		greenMask=swGreenMask=mask;
		for (bits=0;mask&1;bits++,mask>>=1);
		greenShift2=swGreenSh2=8-bits;
		for (shift = 0, mask = sd.ddpfPixelFormat.dwBBitMask; !(mask & 1); shift++, mask >>= 1);
		blueShift1=swBlueSh1=shift;
		blueMask=swBlueMask=mask;
		for (bits=0;mask&1;bits++,mask>>=1);
		blueShift2=swBlueSh2=8-bits;
		gotShifts=true;
	}
	else
	{
		redMask=swRedMask;
		greenMask=swGreenMask;
		blueMask=swBlueMask;

		redShift1=swRedSh1;
		greenShift1=swGreenSh1;
		blueShift1=swBlueSh1;

		redShift2=swRedSh2;
		greenShift2=swGreenSh2;
		blueShift2=swBlueSh2;
	}
}

void direct_draw::MakePaletteFadeTable(	struct _DirectDraw* pdirectd,
										UByte* psrcpal,						//source palette
										UWord* plut,						//dest for lookup table
										UByte targR,UByte targG,UByte targB)//target fade rgb
{
	//prepare lookup table on first call...
	if (!softpallookup)
	{
		UByte* palfade=softpallookup=new UByte[16*64*64];

		for (int i=63;i>=0;i--)
		{
			int index=i<<6;
			for (int j=63;j>=0;j--)
			{
				int index2=(index+j)<<4;

				//from i->j in 16 steps
				int src=j<<4,delta=i-j;
				for (int k=15;k>=0;k--)
				{
					palfade[index2+k]=UByte(src>>4);
					src+=delta;
				}
			}
		}
	}

	UWord redMask; SWord redShift1,redShift2;
	UWord greenMask; SWord greenShift1,greenShift2;
	UWord blueMask; SWord blueShift1,blueShift2;

	GetColourInfo(	pdirectd,
					redMask,redShift1,redShift2,
					greenMask,greenShift1,greenShift2,
					blueMask,blueShift1,blueShift2);

	UByte	*redbaseptr=softpallookup+(ULong(targR>>redShift2)<<4),
			*greenbaseptr=softpallookup+(ULong(targG>>greenShift2)<<4),
			*bluebaseptr=softpallookup+(ULong(targB>>blueShift2)<<4);

	UByte* srcRGB;
	int i;
	
	for (i=255,srcRGB=psrcpal+3*i;i>=0;i--,srcRGB-=3)
	{
		UWord*  newCol=(UWord*)(ULong(plut)+(i<<5));
		
		UByte	*redptr=redbaseptr+(ULong(srcRGB[1]>>redShift2)<<10),
				*greenptr=greenbaseptr+(ULong(srcRGB[2]>>greenShift2)<<10),
				*blueptr=bluebaseptr+(ULong(srcRGB[0]>>blueShift2)<<10);

		for (int k=0;k<16;k++)
		{
			UWord	redVal=*redptr++,
					greenVal=*greenptr++,
					blueVal=*blueptr++;

			*newCol++ =	((redVal&redMask)<<redShift1)+
						((greenVal&greenMask)<<greenShift1)+
						((blueVal&blueMask)<<blueShift1);
		}
	}
}

void direct_draw::MakeColourRangeFadeTable(	struct _DirectDraw* pdirectd,
											UWord* plut,
											UByte srcR,UByte srcG,UByte srcB,
											UByte dstR,UByte dstG,UByte dstB)
{
	//prepare lookup table on first call...
	if (!softpallookup)
	{
		UByte* palfade=softpallookup=new UByte[16*64*64];

		for (int i=63;i>=0;i--)
		{
			int index=i<<6;
			for (int j=63;j>=0;j--)
			{
				int index2=(index+j)<<4;

				//from i->j in 16 steps
				int src=j<<4,delta=i-j;
				for (int k=15;k>=0;k--)
				{
					palfade[index2+k]=UByte(src>>4);
					src+=delta;
				}
			}
		}
	}

	UWord redMask; SWord redShift1,redShift2;
	UWord greenMask; SWord greenShift1,greenShift2;
	UWord blueMask; SWord blueShift1,blueShift2;

	GetColourInfo(	pdirectd,
					redMask,redShift1,redShift2,
					greenMask,greenShift1,greenShift2,
					blueMask,blueShift1,blueShift2);
	
	UByte	*redptr=softpallookup+(ULong(srcR>>redShift2)<<10)+(ULong(dstR>>redShift2)<<4),
			*greenptr=softpallookup+(ULong(srcG>>greenShift2)<<10)+(ULong(dstG>>greenShift2)<<4),
			*blueptr=softpallookup+(ULong(srcB>>blueShift2)<<10)+(ULong(dstB>>blueShift2)<<4);

	for (int i=0;i<16;i++)
	{
		UWord	redVal=*redptr++,
				greenVal=*greenptr++,
				blueVal=*blueptr++;
		*plut++ =	((redVal&redMask)<<redShift1)+
					((greenVal&greenMask)<<greenShift1)+
					((blueVal&blueMask)<<blueShift1);
	}
}

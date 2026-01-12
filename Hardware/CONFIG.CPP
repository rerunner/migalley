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

//SDL #include "DDRAW.H" // RERUN
#include "DISPLAY.H" // RERUN
#include "SAVEGAME.H" // RERUN
#include "MONOTXT.H" // RERUN
#include "GAMESET.H" // RERUN
#include "WIN3D.H" // RERUN

extern void ZeroMem(void* pp, ULong ll); // RERUN
extern void GetModes(int& nm, SDrvrModes*& dm); // RERUN

//------------------------------------------------------------------------------
//Filename       config.cpp
//System         
//Author         Paul.   
//Date           Fri 23 Apr 1999
//Description    Contains processor ID code + DirectX capability tests
//------------------------------------------------------------------------------
#undef DDSCAPS_LOCALVIDMEM
#define DDSCAPS_LOCALVIDMEM 0x10000000l

#define HAS_NPU		0x01
#define CPU_MMX		0x02
#define	CPU_TS		0x04
#define CYRIX		0x08
#define NEC			0x10
#define NEXGEN		0x20
#define AMD			0x40
#define UMC			0x80

#define	CPUSUPP_FP		0x00000001
#define	CPUSUPP_V86X	0x00000002
#define	CPUSUPP_IOBP	0x00000004
#define	CPUSUPP_PSX		0x00000008
#define	CPUSUPP_TS		0x00000010
#define	CPUSUPP_PSMR	0x00000020
#define	CPUSUPP_PAX		0x00000040
#define	CPUSUPP_MCX		0x00000080
#define	CPUSUPP_CXG8	0x00000100
#define	CPUSUPP_APIC	0x00000200
#define	CPUSUPP_reserved 0x00000400
#define	CPUSUPP_SYSE_E	0x00000800
#define	CPUSUPP_MTRR	0x00001000
#define	CPUSUPP_PGX		0x00002000
#define	CPUSUPP_MCA		0x00004000
#define	CPUSUPP_CMOV	0x00008000
#define	CPUSUPP_MMX		0x00800000

#include <cpuid.h>
#include <stdint.h>
#include <x86intrin.h>   // for __rdtsc()

UWord cpu_id(void) {
    unsigned int eax, ebx, ecx, edx;
    UWord result = 0;

    // Get vendor string
    if (!__get_cpuid(0, &eax, &ebx, &ecx, &edx))
        return 0; // CPUID not supported (very old CPU)

    char vendor[13];
    ((unsigned int*)vendor)[0] = ebx;
    ((unsigned int*)vendor)[1] = edx;
    ((unsigned int*)vendor)[2] = ecx;
    vendor[12] = '\0';

    // Identify vendor
    if (!strncmp(vendor, "GenuineIntel", 12)) {
        // Intel
    } else if (!strncmp(vendor, "AuthenticAMD", 12)) {
        result |= AMD;
    } else if (!strncmp(vendor, "CyrixInstead", 12)) {
        result |= CYRIX;
    } else if (!strncmp(vendor, "NexGenDriven", 12)) {
        result |= NEXGEN;
    } else if (!strncmp(vendor, "UMC UMC UMC ", 12)) {
        result |= UMC;
    }

    // Get feature bits
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        unsigned family = (eax >> 8) & 0xf;
        result |= (family & 0xff); // put family code in low byte

        if (edx & (1 << 4))   // TSC
            result |= CPU_TS;
        if (edx & (1 << 23))  // MMX
            result |= CPU_MMX;
    }

    // Assume FPU present
    result |= HAS_NPU;

    return result;
}

//------------------------------------------------------------------------------
//Procedure		cpu_info
//Author		Paul.   
//Date			Fri 23 Apr 1999
//------------------------------------------------------------------------------
#include <time.h>

void cpu_info(void)
{
#if 0
    UWord flags = cpu_id();
    SWord clock = -1;

    if (flags & CPU_TS) {
        uint64_t start_tsc, end_tsc;
        struct timespec ts_start, ts_now;

        // record start time and TSC
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
        start_tsc = __rdtsc();

        // wait ~2 seconds
        do {
            clock_gettime(CLOCK_MONOTONIC, &ts_now);
        } while ((ts_now.tv_sec - ts_start.tv_sec) < 2);

        // record end TSC
        end_tsc = __rdtsc();

        // cycles elapsed in 2 seconds
        uint64_t ticks = end_tsc - start_tsc;

        // convert to MHz
        clock = (SWord)(ticks / (2 * 1000 * 1000));
    }

    // classify processor type
    ProcType pt;
    if (clock < 200 || clock > 2000 || (flags & (UMC << 8)))
        pt = PROC_SLOW;
    else if ((flags & 0xFF) >= 5 && clock <= 266)
        pt = PROC_MIN;
    else if ((flags & 0xFF) >= 6 && clock <= 300)
        pt = PROC_MED;
    else
        pt = PROC_MAX;
#endif
	ProcType pt = PROC_MAX; // RERUN Come on, this is not the 90s
    Save_Data.processorType = pt;
}

//Direct Draw/Direct 3D initialisation tests

#if 0 //SDLMOVE
BOOL FAR PASCAL EnumDrivers(GUID FAR* lpGUID,LPSTR,LPSTR,LPVOID lpContext)
{
	SDrivers *psd=(SDrivers*)lpContext,&sd=*psd;
    LPDIRECTDRAW pDD;
    LPDIRECTDRAW2 pDD2;
    DDCAPS DriverCaps,HELCaps;
	if (lpGUID!=NULL)
	{
		if (DirectDrawCreate(lpGUID,&pDD,NULL)==DD_OK)
		{
			if (pDD->QueryInterface(IID_IDirectDraw2,(LPVOID*)&pDD2)==DD_OK)
			{
			    ZeroMem(&DriverCaps,sizeof(DDCAPS));
				ZeroMem(&HELCaps,sizeof(DDCAPS));
	    		DriverCaps.dwSize=sizeof(DDCAPS);
		    	HELCaps.dwSize=sizeof(DDCAPS);
				if (pDD2->GetCaps(&DriverCaps,&HELCaps)==DD_OK && (DriverCaps.dwCaps&DDCAPS_3D))
				{
					int driver_index=sd.num_drivers;
					memcpy(sd.hard_guids+driver_index,lpGUID,sizeof(GUID));
					sd.num_drivers++;
				}
				pDD2->Release();
			}
			pDD->Release();
			if (sd.num_drivers==SDrivers::Max_Drivers) return DDENUMRET_CANCEL;
		}
	}
	return DDENUMRET_OK;
}


HRESULT WINAPI EnumDisplays(LPDDSURFACEDESC lpDDSurfaceDesc,LPVOID lpContext)
{
	if (lpDDSurfaceDesc->dwWidth>=512 &&
		//RERUN lpDDSurfaceDesc->dwWidth*3==lpDDSurfaceDesc->dwHeight*4 &&
		lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount==16)
	{
		SModeEnum *pme=(SModeEnum*)lpContext,&me=*pme;
		me.widths[me.num_modes]=lpDDSurfaceDesc->dwWidth;
		me.heights[me.num_modes] = lpDDSurfaceDesc->dwHeight;
		me.num_modes++;
		if (me.num_modes==SModeEnum::Max_Modes) return DDENUMRET_CANCEL;
	}
	return DDENUMRET_OK;
}

HRESULT WINAPI FindDevice(LPGUID lpGuid,LPSTR,LPSTR,LPD3DDEVICEDESC lpHWDesc,LPD3DDEVICEDESC,LPVOID lpContext)
{
    if (lpHWDesc->dcmColorModel)
	{
		GUID FAR** ppguid=(GUID FAR**)lpContext;
		*ppguid=lpGuid;
		return D3DENUMRET_CANCEL;
	}
	return D3DENUMRET_OK;
}
#endif

inline bool IsValidMode(SModeFlags* pmf,UWord w,UWord h)
{
//RERUN	if (w*3!=h*4) return false;
	for (int x=0;x<SModeFlags::Max_Modes;x++) if (pmf->widths[x]==w) return true;
	return false;
}

inline ULong TextureRAM(SModeFlags* pmf,UWord w)
{
	for (int x=0;x<SModeFlags::Max_Modes;x++)
		if (pmf->widths[x]==w) return pmf->freeTextureRam[x];
	return ULong(0);
}

#define TEXTURES_PAL8		0x00000001
#define TEXTURES_RGB16		0x00000002
#define TEXTURES_RGBAMASK	0x00000004
#define TEXTURES_RGBAFADE	0x00000008

#define MIN_TEXTURERAM_PAL8	2*1024*1024
#define	MIN_TEXTURERAM_RGB16 4*1024*1024

HRESULT CALLBACK CheckTextures(LPDDSURFACEDESC pddsd,LPVOID pContext)
{
	DWORD* pflags=(DWORD*)pContext;

    if (pddsd->ddpfPixelFormat.dwFlags&DDPF_PALETTEINDEXED8)
		*pflags|=TEXTURES_PAL8;
	else if (pddsd->ddpfPixelFormat.dwFlags&DDPF_RGB &&	pddsd->ddpfPixelFormat.dwRGBBitCount==16)
	{
		*pflags|=TEXTURES_RGB16;
		if (pddsd->ddpfPixelFormat.dwFlags&DDPF_ALPHAPIXELS)
		{
			*pflags|=TEXTURES_RGBAMASK;

			WORD bits=0;
			DWORD mask=pddsd->ddpfPixelFormat.dwRGBAlphaBitMask;
			while(mask) {mask=mask&(mask-1);bits++;}
			if (bits>1) *pflags|=TEXTURES_RGBAFADE;
		}
	}
	return DDENUMRET_OK;
}

void Configure(bool fFullConfig,HWND hWnd)
{
	HRESULT hr;
	SDrivers sd;
	GUID FAR* pguid;
	int i;
	Mono_Text.Print(0,UByteP("Configuring!!!!"));

	Save_Data.sd=sd;

	cpu_info();

	switch (Save_Data.processorType)
	{
	case PROC_SLOW:
	Save_Data.detail_3d%=DETAIL3D_AUTODETAIL;
	Save_Data.desiredfps=1;
	Save_Data.detail_3d%=DETAIL3D_GROUNDSHADING;
	Save_Data.detail_3d%=DETAIL3D_ITEMSHADING;
	Save_Data.detail_3d%=DETAIL3D_TRANSSMOKE;
	Save_Data.detail_3d%=DETAIL3D_INCONSEQUENTIALS;
	Save_Data.detail_3d%=DETAIL3D_ROUTES;
	Save_Data.detail_3d%=DETAIL3D_AIRCRAFTSHADOWS;
	Save_Data.detail_3d%=DETAIL3D_ITEMSHADOWS;
	Save_Data.detail_3d%=DETAIL3D_HORIZONDIST;
	Save_Data.detail_3d|=DETAIL3D_HORIZONFADE;
	Save_Data.detail_3d%=DETAIL3D_CONTOURDETAIL;
	Save_Data.cockpit3Ddetail%=COCK3D_SKYIMAGES;
	Save_Data.gamedifficulty%=GD_WEATHEREFFECTS;
	Save_Data.filtering=0;
	break;

	case PROC_MIN:
	Save_Data.detail_3d%=DETAIL3D_AUTODETAIL;
	Save_Data.desiredfps=2;
	Save_Data.detail_3d|=DETAIL3D_GROUNDSHADING;
	Save_Data.detail_3d%=DETAIL3D_ITEMSHADING;
	Save_Data.detail_3d%=DETAIL3D_TRANSSMOKE;
	Save_Data.detail_3d%=DETAIL3D_INCONSEQUENTIALS;
	Save_Data.detail_3d|=DETAIL3D_ROUTES;
	Save_Data.detail_3d|=DETAIL3D_AIRCRAFTSHADOWS;
	Save_Data.detail_3d%=DETAIL3D_ITEMSHADOWS;
	Save_Data.detail_3d%=DETAIL3D_HORIZONDIST;
	Save_Data.detail_3d|=DETAIL3D_HORIZONFADE;
	Save_Data.detail_3d%=DETAIL3D_CONTOURDETAIL;
	Save_Data.cockpit3Ddetail%=COCK3D_SKYIMAGES;
	Save_Data.gamedifficulty|=GD_WEATHEREFFECTS;
	Save_Data.filtering=1;
	break;

	case PROC_MED:
	Save_Data.detail_3d%=DETAIL3D_AUTODETAIL;
	Save_Data.desiredfps=3;
	Save_Data.detail_3d|=DETAIL3D_GROUNDSHADING;
	Save_Data.detail_3d|=DETAIL3D_ITEMSHADING;
	Save_Data.detail_3d|=DETAIL3D_TRANSSMOKE;
	Save_Data.detail_3d|=DETAIL3D_INCONSEQUENTIALS;
	Save_Data.detail_3d|=DETAIL3D_ROUTES;
	Save_Data.detail_3d|=DETAIL3D_AIRCRAFTSHADOWS;
	Save_Data.detail_3d|=DETAIL3D_ITEMSHADOWS;
	Save_Data.detail_3d|=DETAIL3D_HORIZONDIST;
	Save_Data.detail_3d%=DETAIL3D_HORIZONFADE;
	Save_Data.detail_3d|=DETAIL3D_CONTOURDETAIL;
	Save_Data.cockpit3Ddetail%=COCK3D_SKYIMAGES;
	Save_Data.gamedifficulty|=GD_WEATHEREFFECTS;
	Save_Data.filtering=2;
	break;

	case PROC_MAX:
	Save_Data.detail_3d%=DETAIL3D_AUTODETAIL;
	Save_Data.desiredfps=3;
	Save_Data.detail_3d|=DETAIL3D_GROUNDSHADING;
	Save_Data.detail_3d|=DETAIL3D_ITEMSHADING;
	Save_Data.detail_3d|=DETAIL3D_TRANSSMOKE;
	Save_Data.detail_3d|=DETAIL3D_INCONSEQUENTIALS;
	Save_Data.detail_3d|=DETAIL3D_ROUTES;
	Save_Data.detail_3d|=DETAIL3D_AIRCRAFTSHADOWS;
	Save_Data.detail_3d|=DETAIL3D_ITEMSHADOWS;
	Save_Data.detail_3d|=DETAIL3D_HORIZONDIST;
	Save_Data.detail_3d%=DETAIL3D_HORIZONFADE;
	Save_Data.detail_3d|=DETAIL3D_CONTOURDETAIL;
	Save_Data.cockpit3Ddetail|=COCK3D_SKYIMAGES;
	Save_Data.gamedifficulty|=GD_WEATHEREFFECTS;
	Save_Data.filtering=3;
	break;
	}

	Save_Data.gammacorrection=2;

	//select a good range of default graphics options
	
	//if there is a secondary hardware 3D device then select it
	//else if there is a primary 3D device then select it
	//else select the software driver

	SModeFlags *pdm=NULL;
	SDriverSettings *pds=NULL;

	if (sd.num_drivers>=2)
	{
		Save_Data.fSoftware=false;
		pdm=sd.hard_modes+1;
		pds=sd.hard_settings+1;
	}
	else if (sd.fFirstHardIsPrimary)
	{
		Save_Data.fSoftware=false;
		pdm=sd.hard_modes+0;
		pds=sd.hard_settings+0;
	}
	else
	{
		Save_Data.fSoftware=true;
		pdm=&sd.soft_modes;
	}

	//for software version select a mode based on the processor speed

	if (Save_Data.fSoftware)
	{
		UWord max_w;
		switch (Save_Data.processorType)
		{
		default:
		case PROC_SLOW: max_w=512; break;
		case PROC_MIN:	max_w=640; break;
		case PROC_MED:	max_w=640; break;
		case PROC_MAX:	max_w=800; break;
		}
		//try to select a mode no more than 'max_w' wide

		int nm,rez;
		SDrvrModes* dm;
		GetModes(nm, dm); // RERUN implemented in Win3D.cpp

		//SDL HACK START
		dm[0].displayBPP = 16;
		dm[0].displayHeight = 600;
		dm[0].displayWidth = 800;
		//SDL HACK END

		for (i=0,rez=0;i<nm;i++)
			if (dm[i].driverNo==0 &&
				dm[i].displayBPP==16 && 
				IsValidMode(pdm,dm[i].displayWidth,dm[i].displayHeight))
			{
				if (dm[i].displayWidth==max_w)
				{
					Save_Data.dddriver=-1;
					Save_Data.screenresolution=rez;
					gameSettings.m_dwWidth=Save_Data.displayW=dm[i].displayWidth;
					gameSettings.m_dwHeight=Save_Data.displayH=dm[i].displayHeight;
			 		Mono_Text.Print(Save_Data.displayH,UByteP("Save_Data.displayH (Soft)"));

				}
				rez++;
			}

		Save_Data.textureQuality=4;
		Save_Data.filtering=0;
		Save_Data.detail_3d%=DETAIL3D_INCONSEQUENTIALS;
		Save_Data.detail_3d%=DETAIL3D_GROUNDSHADING;
		Save_Data.cockpit3Ddetail%=COCK3D_SKYIMAGES;
	}
	else
	{
		UWord max_w;
		int nm,rez,dddriver;
		SDrvrModes* dm;
		GetModes(nm,dm);
		dddriver=(sd.num_drivers>=2)?1:0;

		max_w=(Save_Data.processorType!=PROC_MAX)?800:1024;

		for (i=0,rez=0;i<nm;i++)
			if (dm[i].driverNo==dddriver &&
				dm[i].displayBPP==16 &&
				IsValidMode(pdm,dm[i].displayWidth,dm[i].displayHeight))
			{
				if (dm[i].displayWidth==max_w)
				{
					//if texture ram available is at least enuogh for texture quality == 3
					//then choose this mode

					ULong texture_ram=TextureRAM(pdm,max_w);
					if (!pds->pal8) texture_ram>>=1;

					ULong min_texture_ram=0x200000;
					if (pds->filtering>=2)	min_texture_ram+=(min_texture_ram>>1);
					if (pds->filtering==3)	min_texture_ram+=(!pds->pal8)?49152*2:49152;

					if (texture_ram>=min_texture_ram)
					{
						Save_Data.dddriver=dddriver-1;
						Save_Data.screenresolution=rez;
						gameSettings.m_dwWidth=Save_Data.displayW=dm[i].displayWidth;
						gameSettings.m_dwHeight=Save_Data.displayH=dm[i].displayHeight;
				 		Mono_Text.Print(Save_Data.displayH,UByteP("Save_Data.displayH (1)"));

						Save_Data.textureQuality=(texture_ram>2*min_texture_ram)?4:3;
						break;
					}
				}
				rez++;
			}

		if (i==nm)		//FAILED!
		{
			max_w=(Save_Data.processorType!=PROC_MAX)?640:800;

			for (i=0,rez=0;i<nm;i++)
				if (dm[i].driverNo==dddriver &&
					dm[i].displayBPP==16 &&
					IsValidMode(pdm,dm[i].displayWidth,dm[i].displayHeight))
				{
					if (dm[i].displayWidth==max_w)
					{
						//if texture ram available is at least enuogh for texture quality == 3
						//then choose this mode

						ULong texture_ram=TextureRAM(pdm,max_w);
						if (!pds->pal8) texture_ram>>=1;

						ULong min_texture_ram=0x200000;
//TEMP						if (pds->filtering>=2)	min_texture_ram+=(min_texture_ram>>1);
//TEMP						if (pds->filtering==3)	min_texture_ram+=(!pds->pal8)?49152*2:49152;

						if (texture_ram>=min_texture_ram)
						{
							Save_Data.dddriver=dddriver-1;
							Save_Data.screenresolution=rez;
							gameSettings.m_dwWidth=Save_Data.displayW=dm[i].displayWidth;
							gameSettings.m_dwHeight=Save_Data.displayH=dm[i].displayHeight;
				 			Mono_Text.Print(Save_Data.displayH,UByteP("Save_Data.displayH (2)"));
							Save_Data.textureQuality=(texture_ram>2*min_texture_ram)?4:3;
							break;
						}
					}
					rez++;
				}

			if (i==nm)	//FAILED AGAIN!
			{
				max_w=(Save_Data.processorType!=PROC_MAX)?512:640;

				for (i=0,rez=0;i<nm;i++)
					if (dm[i].driverNo==dddriver &&
						dm[i].displayBPP==16 &&
						IsValidMode(pdm,dm[i].displayWidth,dm[i].displayHeight))
					{
						if (dm[i].displayWidth==max_w)
						{
							//if texture ram available is at least enuogh for texture quality == 3
							//then choose this mode

							ULong texture_ram=TextureRAM(pdm,max_w);
							if (!pds->pal8) texture_ram>>=1;

							ULong min_texture_ram=0x200000;
//TEMP							if (pds->filtering>=2)	min_texture_ram+=(min_texture_ram>>1);
//TEMP							if (pds->filtering==3)	min_texture_ram+=(!pds->pal8)?49152*2:49152;

							if (texture_ram>=min_texture_ram)
							{
								Save_Data.dddriver=dddriver-1;
								Save_Data.screenresolution=rez;
								gameSettings.m_dwWidth=Save_Data.displayW=dm[i].displayWidth;
								gameSettings.m_dwHeight=Save_Data.displayH=dm[i].displayHeight;
				 				Mono_Text.Print(Save_Data.displayH,UByteP("Save_Data.displayH (3)"));
								Save_Data.textureQuality=(texture_ram>2*min_texture_ram)?4:3;
								break;
							}
						}
						rez++;
					}		  

				if (i==nm)	//FAILED AGAIN - ONE LAST ATTEMPT @ HARDWARE MODE
				{
					max_w=640;

					for (i=0,rez=0;i<nm;i++)
			 			if (dm[i].driverNo==dddriver &&
							dm[i].displayBPP==16 &&
							IsValidMode(pdm,dm[i].displayWidth,dm[i].displayHeight))
						{
							if (dm[i].displayWidth==max_w)
							{
								//if texture ram available is at least enuogh for texture quality == 3
								//then choose this mode

								ULong texture_ram=TextureRAM(pdm,max_w);
								if (!pds->pal8) texture_ram>>=1;

								ULong min_texture_ram=0x200000;

								if (texture_ram>=min_texture_ram)
								{
									Save_Data.dddriver=dddriver-1;
									Save_Data.screenresolution=rez;
									gameSettings.m_dwWidth=Save_Data.displayW=dm[i].displayWidth;
									gameSettings.m_dwHeight=Save_Data.displayH=dm[i].displayHeight;
				 					Mono_Text.Print(Save_Data.displayH,UByteP("Save_Data.displayH (4)"));
									Save_Data.textureQuality=(texture_ram>2*min_texture_ram)?4:3;
									break;
								}
							}
							rez++;
						}

					if (i==nm)	//CHOOSE THE SOFTWARE DRIVER!
					{
						Save_Data.fSoftware=true;
						pdm=&sd.soft_modes;
						switch (Save_Data.processorType)
						{
						default:
						case PROC_SLOW: max_w=512; break;
						case PROC_MIN:	max_w=640; break;
						case PROC_MED:	max_w=640; break;
						case PROC_MAX:	max_w=800; break;
						}
						//try to select a mode no more than 'max_w' wide

						for (i=0,rez=0;i<nm;i++)
							if (dm[i].driverNo==-1 &&
								dm[i].displayBPP==16 && 
								IsValidMode(pdm,dm[i].displayWidth,dm[i].displayHeight))
							{
								if (dm[i].displayWidth==max_w)
								{
									Save_Data.dddriver=-1;
									Save_Data.screenresolution=rez;
									gameSettings.m_dwWidth=Save_Data.displayW=dm[i].displayWidth;
									gameSettings.m_dwHeight=Save_Data.displayH=dm[i].displayHeight;
				 					Mono_Text.Print(Save_Data.displayH,UByteP("Save_Data.displayH (S)"));
								}
								rez++;
							}

						Save_Data.textureQuality=4;
						Save_Data.filtering=0;
						Save_Data.detail_3d%=DETAIL3D_INCONSEQUENTIALS;
						Save_Data.detail_3d%=DETAIL3D_GROUNDSHADING;
						Save_Data.cockpit3Ddetail%=COCK3D_SKYIMAGES;
					}
				}
			}
		}
	}
}

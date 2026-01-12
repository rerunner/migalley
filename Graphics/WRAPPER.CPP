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

#include "_GRAP.H" //RERUN

//------------------------------------------------------------------------------
//Filename       wrapper.cpp
//System         
//Author         Paul.   
//Date           Mon 3 Jul 1995
//Description    Wrapper.CPP provides a wrapper layer around the hardware 
//			   primitives and graphics primitives libraries. This file 
//			   provides the routines we are familiar with and translation 
//			   from virtual resolution and 256 colours as well as providing 
//			   memory blocks on request from the lower level.
//------------------------------------------------------------------------------
#include	<stdlib.h>

#include	"DOSDEFS.H"
#include	"STUB3D.H"
#include 	"MYERROR.H"
#include	"PALETTE.H"
#include	"FONTS.H"
#include	"FILEMAN.H"
#include	"DISPLAY.H"
//#include	"HardPrim.H"
//#include	"Wrapper.H"
#include	"MYANGLES.H"
#include	"WORLDINC.H"
#include	"WORLD.H"
//#include	"Main.H"
#include	"IMAGEMAP.H"
#include	"HARDPASM.H"
#include	"SAVEGAME.H"

#include	"KEYTEST.H"
#include	"TEXTREF.H"
#include	"MOUSE.H"
#include	"MYTIME.H"
#include "POLYGON.H" // RERUN

//------------------------------------------------------------------------------
//Procedure		Graphic
//Author		Jim Taylor
//Date			Wed 13 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
CON		Graphic::Graphic(Bool& unp,IntensityIndex*	ii,void*&p,FontPtr& fp,FontPtr& mp):
	UseNewPal(unp),
	intensityindex(*ii),
	Palette(p),
	fontptr(fp),
	mousefontptr(mp)
{
};

void	MigDisplay::WindowMoved(LogicalPtr	oldplace,LogicalPtr newplace)
{
	FILE* fp;
Graphic*	window=windowlink;
if (oldplace!=newplace)

	while (window)
	{	//being longwinded here to ensure legality!
		window->logicalscreenptr=newplace+(window->logicalscreenptr-oldplace);
		window->BytesPerScanLine = BytesPerScanLine;
		window=window->windowlink;
	}
}

//------------------------------------------------------------------------------
//Procedure		MigWindow
//Author		Jim Taylor
//Date			Wed 13 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
CON	MigWindow::MigWindow(Bool& unp,IntensityIndex*	ii,void*&p,FontPtr& fp,FontPtr& mp):
		Graphic(unp,ii,p,fp,mp)
{
	master=this;
	windowlink=NULL;
};

//------------------------------------------------------------------------------
//Procedure		MigWindow
//Author		Jim Taylor
//Date			Wed 13 Dec 1995
//
//Description	Generate a clipping zone on the current display
//
//				Reposition the physical coords based on the logical coords movements
//				Regenerate the scale factors based on scale
//				Regenerate the logical coords using physical and scale
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
CON	MigWindow::MigWindow(const MigWindow& src,const InitWindowDef& pos,SWord scale,Bool nofitfatal):
		Graphic(src.UseNewPal,&src.intensityindex,src.Palette,src.fontptr,src.mousefontptr)
{	//MigWindow on existing screen
	master=src.master;
	windowlink=master->windowlink;
	master->windowlink=this;

	iffartnum		=src.iffartnum;
	ifflinenum		=src.ifflinenum;
	iffoffset		=src.iffoffset;
	logicalscreenptr=src.logicalscreenptr;
	BytesPerPixel	=src.BytesPerPixel;

	RedBits			=src.RedBits;								//PD 17Oct97
	GreenBits		=src.GreenBits;								//PD 17Oct97
	BlueBits 		=src.BlueBits;								//PD 17Oct97
	RedShift		=src.RedShift;								//PD 17Oct97
	GreenShift		=src.GreenShift;							//PD 17Oct97
	BlueShift 		=src.BlueShift;								//PD 17Oct97

	BytesPerScanLine=src.BytesPerScanLine;
	glasscolour		=src.glasscolour;
	colour			=src.colour;
	range			=src.range;
	fontptr			=src.fontptr;
	CharX			=src.CharX;
	CharY			=src.CharY;
	imageptr		=src.imageptr;
	shadetable		=src.shadetable;
	NumberOfScreens	=src.NumberOfScreens;
//TempCode DAW 08Feb96 	horiline		=src.horiline;


	PhysicalWidth=(pos.VirtualWidth)/src.virtualXscale;
	PhysicalHeight=(pos.VirtualHeight)/src.virtualYscale;
	PhysicalMinX=pos.VirtualMinX/src.virtualXscale;
	PhysicalMinY=pos.VirtualMinY/src.virtualYscale;
	PhysicalMaxX=PhysicalMinX+PhysicalWidth-1;
	PhysicalMaxY=PhysicalMinY+PhysicalHeight-1;
	if (	(PhysicalMinX<src.PhysicalMinX)
		||	(PhysicalMinY<src.PhysicalMinY)
		||	((PhysicalMinX+PhysicalWidth)>(src.PhysicalMinX+src.PhysicalWidth))
		||	((PhysicalMinY+PhysicalHeight)>(src.PhysicalMinY+src.PhysicalHeight))
		)
	{
		if (PhysicalMinX<src.PhysicalMinX)
		{
			PhysicalWidth-=src.PhysicalMinX-PhysicalMinX;
			PhysicalMinX=src.PhysicalMinX;
		}
		if (PhysicalMinY<src.PhysicalMinY)
		{
			PhysicalHeight-=src.PhysicalMinY-PhysicalMinY;
			PhysicalMinY=src.PhysicalMinY;
		}
		if ((PhysicalMinX+PhysicalWidth)>(src.PhysicalMinX+src.PhysicalWidth))
			PhysicalWidth=(src.PhysicalMinX+src.PhysicalWidth)-PhysicalMinX;
		if ((PhysicalMinY+PhysicalHeight)>(src.PhysicalMinY+src.PhysicalHeight))
			PhysicalHeight=(src.PhysicalMinY+src.PhysicalHeight)-PhysicalMinY;

		if (nofitfatal)
			_Error.EmitSysErr("MigWindow is outside source screen area");
	}
	virtualXscale=src.virtualXscale*scale;
	virtualYscale=src.virtualYscale*scale;

	VirtualWidth	=PhysicalWidth		*virtualXscale;
	VirtualHeight	=PhysicalHeight		*virtualYscale;
	VirtualMinX		=PhysicalMinX		*virtualXscale;
	VirtualMinY		=PhysicalMinY		*virtualYscale;
};

//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		MigWindow
//FIX10-2//Author		Jim Taylor
//FIX10-2//Date			Mon 2 Sep 1996
//FIX10-2//
//FIX10-2//Description	Shift origin of window by pixels
//FIX10-2//
//FIX10-2//Inputs		
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2CON	MigWindow::MigWindow(const MigWindow& src,const ScreenXY& delta,SWord scale):
//FIX10-2		Graphic(src.UseNewPal,&src.intensityindex,src.Palette,src.fontptr,src.mousefontptr)
//FIX10-2{	//MigWindow on existing screen
//FIX10-2	master=src.master//FIX10-2
//FIX10-2	windowlink=master->windowlink//FIX10-2
//FIX10-2	master->windowlink=this//FIX10-2
//FIX10-2
//FIX10-2	iffartnum		=src.iffartnum//FIX10-2
//FIX10-2	ifflinenum		=src.ifflinenum//FIX10-2
//FIX10-2	iffoffset		=src.iffoffset//FIX10-2
//FIX10-2	logicalscreenptr=src.logicalscreenptr//FIX10-2
//FIX10-2	BytesPerPixel	=src.BytesPerPixel//FIX10-2
//FIX10-2
//FIX10-2	RedBits			=src.RedBits//FIX10-2								//PD 17Oct97
//FIX10-2	GreenBits		=src.GreenBits//FIX10-2								//PD 17Oct97
//FIX10-2	BlueBits 		=src.BlueBits//FIX10-2								//PD 17Oct97
//FIX10-2	RedShift		=src.RedShift//FIX10-2								//PD 17Oct97
//FIX10-2	GreenShift		=src.GreenShift//FIX10-2							//PD 17Oct97
//FIX10-2	BlueShift 		=src.BlueShift//FIX10-2								//PD 17Oct97
//FIX10-2
//FIX10-2	BytesPerScanLine=src.BytesPerScanLine//FIX10-2
//FIX10-2	glasscolour		=src.glasscolour//FIX10-2
//FIX10-2	colour			=src.colour//FIX10-2
//FIX10-2	range			=src.range//FIX10-2
//FIX10-2	fontptr			=src.fontptr//FIX10-2
//FIX10-2	CharX			=src.CharX//FIX10-2
//FIX10-2	CharY			=src.CharY//FIX10-2
//FIX10-2	imageptr		=src.imageptr//FIX10-2
//FIX10-2	shadetable		=src.shadetable//FIX10-2
//FIX10-2	NumberOfScreens	=src.NumberOfScreens//FIX10-2
//FIX10-2//TempCode DAW 08Feb96 	horiline		=src.horiline//FIX10-2
//FIX10-2
//FIX10-2
//FIX10-2
//FIX10-2
//FIX10-2	PhysicalWidth	=src.PhysicalWidth//FIX10-2
//FIX10-2	PhysicalHeight	=src.PhysicalHeight//FIX10-2
//FIX10-2	PhysicalMinX	=src.PhysicalMinX//FIX10-2
//FIX10-2	PhysicalMinY	=src.PhysicalMinY//FIX10-2
//FIX10-2	PhysicalMaxX=PhysicalMinX+PhysicalWidth-1//FIX10-2
//FIX10-2	PhysicalMaxY=PhysicalMinY+PhysicalHeight-1//FIX10-2
//FIX10-2	virtualXscale	=src.virtualXscale//FIX10-2
//FIX10-2	virtualYscale	=src.virtualYscale//FIX10-2
//FIX10-2
//FIX10-2	int	dx=delta.x/virtualXscale//FIX10-2
//FIX10-2	int	dy=delta.y/virtualYscale//FIX10-2
//FIX10-2	PhysicalMinX+=dx//FIX10-2
//FIX10-2	PhysicalMinY+=dy//FIX10-2
//FIX10-2	logicalscreenptr-=dy*BytesPerScanLine+dx*BytesPerPixel//FIX10-2
//FIX10-2
//FIX10-2	virtualXscale*=scale//FIX10-2
//FIX10-2	virtualYscale*=scale//FIX10-2
//FIX10-2
//FIX10-2	VirtualWidth	=PhysicalWidth		*virtualXscale//FIX10-2
//FIX10-2	VirtualHeight	=PhysicalHeight		*virtualYscale//FIX10-2
//FIX10-2	VirtualMinX		=PhysicalMinX		*virtualXscale//FIX10-2
//FIX10-2	VirtualMinY		=PhysicalMinY		*virtualYscale//FIX10-2
//FIX10-2	PhysicalMaxX=PhysicalMinX+PhysicalWidth-1//FIX10-2
//FIX10-2	PhysicalMaxY=PhysicalMinY+PhysicalHeight-1//FIX10-2
//FIX10-2
//FIX10-2}//FIX10-2

CON	MigWindow::MigWindow(const MigWindow& src,const int deltax,const int deltay,SWord scale):
		Graphic(src.UseNewPal,&src.intensityindex,src.Palette,src.fontptr,src.mousefontptr)
{	//MigWindow on existing screen
	master=src.master;
	windowlink=master->windowlink;
	master->windowlink=this;

	iffartnum		=src.iffartnum;
	ifflinenum		=src.ifflinenum;
	iffoffset		=src.iffoffset;
	logicalscreenptr=src.logicalscreenptr;
	BytesPerPixel	=src.BytesPerPixel;

	RedBits			=src.RedBits;								//PD 17Oct97
	GreenBits		=src.GreenBits;								//PD 17Oct97
	BlueBits 		=src.BlueBits;								//PD 17Oct97
	RedShift		=src.RedShift;								//PD 17Oct97
	GreenShift		=src.GreenShift;							//PD 17Oct97
	BlueShift 		=src.BlueShift;								//PD 17Oct97

	BytesPerScanLine=src.BytesPerScanLine;
	glasscolour		=src.glasscolour;
	colour			=src.colour;
	range			=src.range;
	fontptr			=src.fontptr;
	CharX			=src.CharX;
	CharY			=src.CharY;
	imageptr		=src.imageptr;
	shadetable		=src.shadetable;
	NumberOfScreens	=src.NumberOfScreens;
//TempCode DAW 08Feb96 	horiline		=src.horiline;




	PhysicalWidth	=src.PhysicalWidth;
	PhysicalHeight	=src.PhysicalHeight;
	PhysicalMinX	=src.PhysicalMinX;
	PhysicalMinY	=src.PhysicalMinY;
	PhysicalMaxX=PhysicalMinX+PhysicalWidth-1;
	PhysicalMaxY=PhysicalMinY+PhysicalHeight-1;
	virtualXscale	=src.virtualXscale;
	virtualYscale	=src.virtualYscale;

	int	dx=deltax/virtualXscale;
	int	dy=deltay/virtualYscale;
	PhysicalMinX+=dx;
	PhysicalMinY+=dy;
	logicalscreenptr-=dy*BytesPerScanLine+dx*BytesPerPixel;

	virtualXscale*=scale;
	virtualYscale*=scale;

	VirtualWidth	=PhysicalWidth		*virtualXscale;
	VirtualHeight	=PhysicalHeight		*virtualYscale;
	VirtualMinX		=PhysicalMinX		*virtualXscale;
	VirtualMinY		=PhysicalMinY		*virtualYscale;
	PhysicalMaxX=PhysicalMinX+PhysicalWidth-1;
	PhysicalMaxY=PhysicalMinY+PhysicalHeight-1;

};
	
	
//------------------------------------------------------------------------------
//Procedure		MigWindow
//Author		Jim Taylor
//Date			Tue 19 Dec 1995
//
//Description	Create a virtual screen on the current window
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
CON	MigWindow::MigWindow(const MigWindow& src,WinShiftDef X,WinShiftDef Y,SWord scale):
		Graphic(src.UseNewPal,&src.intensityindex,src.Palette,src.fontptr,src.mousefontptr)
{
	master=src.master;
	windowlink=master->windowlink;
	master->windowlink=this;

	iffartnum		=src.iffartnum;
	ifflinenum		=src.ifflinenum;
	iffoffset		=src.iffoffset;
	BytesPerPixel	=src.BytesPerPixel;
	BytesPerScanLine=src.BytesPerScanLine;
	glasscolour		=src.glasscolour;
	colour			=src.colour;
	range			=src.range;
	CharX			=src.CharX;
	CharY			=src.CharY;
	imageptr		=src.imageptr;
	shadetable		=src.shadetable;
	NumberOfScreens	=src.NumberOfScreens;
//TempCode DAW 08Feb96 	horiline		=src.horiline;
	PhysicalWidth	=(src.PhysicalWidth);
	PhysicalHeight	=(src.PhysicalHeight);
	PhysicalMaxX=PhysicalMinX+PhysicalWidth-1;
	PhysicalMaxY=PhysicalMinY+PhysicalHeight-1;


	//Place screen origin on top LHS of window
	logicalscreenptr=	src.logicalscreenptr			
					 +	src.PhysicalMinX*BytesPerPixel
					 +	src.PhysicalMinY*BytesPerScanLine;

	//Now shift origin to requested place
	if (X==WINSH_MAX)
		PhysicalMinX=-PhysicalWidth;
	elseif (X==WINSH_MID)
		PhysicalMinX=-PhysicalWidth>>1;
	elseif (X==WINSH_MIN)
		PhysicalMinX=0;
	else
		PhysicalMinX=src.PhysicalMinX;


	if (Y==WINSH_MAX)
		PhysicalMinY=-PhysicalHeight;
	elseif (Y==WINSH_MID)
		PhysicalMinY=-PhysicalHeight>>1;
	elseif (Y==WINSH_MIN)
		PhysicalMinY=0;
	else
		PhysicalMinY=src.PhysicalMinY;
	PhysicalMaxX=PhysicalMinX+PhysicalWidth-1;
	PhysicalMaxY=PhysicalMinY+PhysicalHeight-1;

	//Place screen memory of window
	logicalscreenptr=	logicalscreenptr			
					 -	PhysicalMinX*BytesPerPixel
					 -	PhysicalMinY*BytesPerScanLine;

	virtualXscale=src.virtualXscale*scale;
	virtualYscale=src.virtualYscale*scale;

	VirtualWidth	=PhysicalWidth		*virtualXscale;
	VirtualHeight	=PhysicalHeight		*virtualYscale;
	VirtualMinX		=PhysicalMinX		*virtualXscale;
	VirtualMinY		=PhysicalMinY		*virtualYscale;
	

}

//------------------------------------------------------------------------------
//Procedure		MigWindow
//Author		Paul.   
//Date			Fri 28 Jun 1996
//
//Description	Make an imagemap look like a window
//				so we can draw to it
//
//Inputs		ptr to imagemap desc	
//
//Returns	
//
//------------------------------------------------------------------------------
CON	MigWindow::MigWindow(const MigWindow& src,ImageMap_Desc &image):
		Graphic(src.UseNewPal,&src.intensityindex,src.Palette,src.fontptr,src.mousefontptr)
{
	//Keep this to keep the destructor happy
	master=src.master;
	windowlink=master->windowlink;
	master->windowlink=this;

	PhysicalWidth = image.w;						//RJS 06Feb98
	PhysicalHeight= image.h;						//RJS 06Feb98

	logicalscreenptr=image.body;					//RJS 06Feb98
	iffartnum		=src.iffartnum;
	ifflinenum		=src.ifflinenum;
	iffoffset		=src.iffoffset;
	BytesPerPixel	=src.BytesPerPixel;
	BytesPerScanLine=PhysicalWidth;
	glasscolour		=src.glasscolour;
	colour			=src.colour;
	range			=src.range;
	fontptr			=src.fontptr;
	CharX			=src.CharX;
	CharY			=src.CharY;
	imageptr		=src.imageptr;
	shadetable		=src.shadetable;
	NumberOfScreens	=1;

	PhysicalMinX=0;
	PhysicalMinY=0;
	PhysicalMaxX=PhysicalMinX+PhysicalWidth-1;
	PhysicalMaxY=PhysicalMinY+PhysicalHeight-1;

	virtualXscale=src.virtualXscale;
	virtualYscale=src.virtualYscale;

	VirtualWidth	=PhysicalWidth		*virtualXscale;
	VirtualHeight	=PhysicalHeight		*virtualYscale;
	VirtualMinX		=PhysicalMinX		*virtualXscale;
	VirtualMinY		=PhysicalMinY		*virtualYscale;
}

//------------------------------------------------------------------------------
//Procedure		MigDisplay
//Author		Jim Taylor
//Date			Wed 13 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
#pragma  warning(disable:4355)
CON	MigDisplay::MigDisplay(View3d* v):
		MigWindow(this->UseNewPal,&this->intensityindex,Palette,fontptr,mousefontptr)
#pragma  warning(default:4355)
		{
			view3dwin=v;
			prevmodeAX=prevmodeBX=0;
			Palette=NULL;
			Graphic::shadetable=shadetable;
			logicalscreenptr=NULL;
			fontptr=NULL;
			mousefontptr = NULL;
			mouseimagenumber = 1;
			iffartnum=-1;
			currsmk=0;
			FillExpandBuff();
		};


DES	MigDisplay::~MigDisplay()
{

	ExitDirectDraw ();
	//RERUN UNSAFE: delete	Palette;
	delete[] static_cast<UByte*>(Palette); //RERUN: cross-fingers
	Palette=NULL;
}
//------------------------------------------------------------------------------
//Procedure		MigWindow
//Author		Jim Taylor
//Date			Wed 13 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
DES	MigWindow::~MigWindow()
{
Graphic*	chase=master;
	if (chase!=this)
		while (chase->windowlink!=this)
			chase=chase->windowlink;
//Old_Code DAW 01Feb96 	else
//Old_Code DAW 01Feb96 		assert((windowlink==NULL)&&("MigDisplay closed, but not all windows"));
	chase->windowlink=windowlink;
};

//------------------------------------------------------------------------------
//Procedure	MigWindow::Init
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#ifdef	STUB3d_defined
short	MigDisplay::Init(WinMode::Mode v,int L,int T,int R,int B)
{
	InitDisplayDef	mode={640,480,8,0};
	
	if 		(FindModeWin(mode))				// Returns TRUE
		LoadGraphicsLib(SelectLibWin());	// SelectLibWin() returns INVALIDFILENUM
	else
		_Error.EmitSysErr("Couldn't find match for mode!!!");

	SetGraphicsMode();

	PhysicalMaxX=PhysicalMinX+PhysicalWidth-1;
	PhysicalMaxY=PhysicalMinY+PhysicalHeight-1;

	return(TRUE);
}
#endif

//------------------------------------------------------------------------------
//Procedure	MigWindow::GetFontInfo
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MigWindow::GetFontInfo(short& width, short& delta,short& height)
{

	//Get info on the space character

	delta = (short)	((-fontptr->charentries[0].YShift)*virtualYscale);

	//t3 is a pointer to the fonts SIZETABLE & space is the first character

	width = (short)	(fontptr->charentries[0].NextXShift*virtualXscale);//RDH 20Dec95
	height = (short) (fontptr->charentries[0].Height*virtualYscale);
}

//------------------------------------------------------------------------------
//Procedure	MigWindow::GetCharacterWidth
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	Called by justification routines to estimate character positions
//
//Inputs	
//
//Returns	The NextXShift - Width is an internal thing...
//			Of course, really clever justification would use width for the last char
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
short MigWindow::GetCharacterWidth(short character)
{
	return(SWord)(fontptr->charentries[character-32].NextXShift*virtualXscale);
}

//------------------------------------------------------------------------------
void	FixupFontHeader(void*	&fontdata,ULong&)
{	//fixup offsets into pointers:

	MigFont*
	f=(MigFont*)	fontdata;
CharTableEntryPtr
	c=(f->charentries);
int	i;
int j = f->maxchar;
	for (i=0;i<j;i++)
		c[i].pixeldata =(void*)((ULong) c[i].pixeldata + (ULong) &c[j]);
  }

//------------------------------------------------------------------------------
//Procedure	MigWindow::RegisterFontActive
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	Sets up current font to be used
//
//Inputs		ScreenRecord, font filenumber
//
//Returns	width = width of space
//			delta = delta to top of space
//			height = height of space
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//short MigWindow::RegisterFontActive
//				(FileNum filenumber,short& width, short& delta, short& height)
//{
//static	fileblockptr	loadedhandle=NULL;
//
//	//Dispose old font if any
//
//	if(fontptr != NULL)	delete(loadedhandle);
//
//	//Load new font data
//
//	loadedhandle=new fileblock(filenumber,FixupFontHeader);
//
//	fontptr=(FontPtr)getdata(*loadedhandle);
//
////TempCode JIM 03Oct95 	if((screen->FontPtr = _DiskIO.LoadNumberedFile(filenumber))==NULL)
////TempCode JIM 03Oct95 		_Error.EmitSysErr(__FILE__":Failed to set new font\n");
//
//	//Get info on the space character
//
//	GetFontInfo(width,delta,height);
//
//	return(TRUE);
//}

//------------------------------------------------------------------------------
//Procedure		SetPixelWidth
//Author		Paul.   
//Date			Wed 3 Jan 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::SetPixelWidth(SWord &pixwid)
{
	Graphic::SetPixelWidth(pixwid);
}
void* MigWindow::GetPaletteTable()
{
	return Graphic::GetPaletteTable();
}

void* MigWindow::GetLandFadeTable()
{
	return Graphic::GetLandFadeTable();
}

void* MigWindow::GetSkyFadeTable()
{
	return Graphic::GetSkyFadeTable();
}

void* MigWindow::GetHorizonFadeTable()
{
	return Graphic::GetHorizonFadeTable();
}

void MigWindow::SelectPalette(SWord palNum)
{
	Graphic::SelectPalette(palNum);
}

UWord MigWindow::GetPaletteEntry(UWord c)
{
	return Graphic::GetPaletteEntry(c);
}

void MigWindow::SetPaletteEntry(UWord c,UWord v)
{
	Graphic::SetPaletteEntry(c,v);
}

//------------------------------------------------------------------------------
//Procedure		SetTransparency
//Author		Paul.   
//Date			Tue 2 Apr 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::SetTransparency(SWord flag)
{
	Graphic::SetTransparency(flag);
}

//------------------------------------------------------------------------------
//Procedure		SetLuminosity
//Author		Robert Slater
//Date			Mon 19 Jan 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::SetLuminosity(SWord flag)
{
	Graphic::SetLuminosity(flag);
}

//------------------------------------------------------------------------------
//Procedure		SetHeatHaze
//Author		Robert Slater
//Date			Fri 30 Jan 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::SetHeatHaze(SWord flag)
{
	Graphic::SetHeatHaze(flag);
}

//------------------------------------------------------------------------------
//Procedure	MigWindow::SetColour
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MigWindow::SetColour(Colour colour)							//PD 29Nov95
{
	Graphic::SetColour(colour);								//PD 29Nov95
}

//------------------------------------------------------------------------------
//Procedure	MigWindow::SetColour
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MigWindow::SetColour(Colour colour, ColourRange range)		//PD 29Nov95
{
		Graphic::SetColour(colour,range);						//PD 29Nov95
}

//------------------------------------------------------------------------------
//Procedure		SetGlassRange
//Author		Paul.   
//Date			Mon 1 Apr 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::SetGlassRange(Colour colour)
{
 	Graphic::SetGlassRange(colour);
}

//------------------------------------------------------------------------------
//Procedure	MigWindow::SetColour
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MigWindow::SetColour(ImageMapNumber imap)					//PD 29Nov95
{
	//Convert image number to a pointer to the image map data

	ImageMapDescPtr	imageptr;

	imageptr = Image_Map.GetImageMapPtr(imap);					//PD 29Nov95

	Graphic::SetColour(imageptr);								//PD 29Nov95
}

void MigWindow::SetColour(ROWANSURFACEDESC* rsd)
{
	Graphic::SetColour(rsd);
}

//------------------------------------------------------------------------------
//Procedure		SetColour
//Author		Paul.   
//Date			Fri 3 May 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::SetColour(ImageMapDescPtr imapptr)
{
	Graphic::SetColour(imapptr);
}

//TempCode JIM 12Dec95 //------------------------------------------------------------------------------
//TempCode JIM 12Dec95 //Procedure	MigWindow::CloneWindow
//TempCode JIM 12Dec95 //------------------------------------------------------------------------------
//TempCode JIM 12Dec95 //Author		Paul.   
//TempCode JIM 12Dec95 //Date		Mon 3 Jul 1995
//TempCode JIM 12Dec95 //Modified	
//TempCode JIM 12Dec95 //
//TempCode JIM 12Dec95 //Description	
//TempCode JIM 12Dec95 //
//TempCode JIM 12Dec95 //Inputs	
//TempCode JIM 12Dec95 //
//TempCode JIM 12Dec95 //Returns	
//TempCode JIM 12Dec95 //
//TempCode JIM 12Dec95 //Externals
//TempCode JIM 12Dec95 //------------------------------------------------------------------------------
//TempCode JIM 12Dec95 //------------------------------------------------------------------------------
//TempCode JIM 12Dec95 ScreenRecord *MigWindow::CloneWindow(ScreenRecord *screen)
//TempCode JIM 12Dec95 {
//TempCode JIM 12Dec95 	int	i;
//TempCode JIM 12Dec95 
//TempCode JIM 12Dec95 	char	*src,
//TempCode JIM 12Dec95 		*dest;
//TempCode JIM 12Dec95 
//TempCode JIM 12Dec95 	ScreenRecord	*newscreen;
//TempCode JIM 12Dec95 
//TempCode JIM 12Dec95 	if((newscreen = new ScreenRecord)==0)
//TempCode JIM 12Dec95 	{
//TempCode JIM 12Dec95 		return(NULL);
//TempCode JIM 12Dec95 	}
//TempCode JIM 12Dec95 
//TempCode JIM 12Dec95 	src = (char *)screen;
//TempCode JIM 12Dec95 	dest = (char *)newscreen;
//TempCode JIM 12Dec95 
//TempCode JIM 12Dec95 	for(i=sizeof(ScreenRecord);i;--i,*src++ = *dest++);
//TempCode JIM 12Dec95 
//TempCode JIM 12Dec95 	return(newscreen);
//TempCode JIM 12Dec95 }

//TempCode JIM 12Dec95 //------------------------------------------------------------------------------
//TempCode JIM 12Dec95 //Procedure	DestroyWindow
//TempCode JIM 12Dec95 //------------------------------------------------------------------------------
//TempCode JIM 12Dec95 //Author		Paul.   
//TempCode JIM 12Dec95 //Date		Mon 14 Aug 1995
//TempCode JIM 12Dec95 //Modified	
//TempCode JIM 12Dec95 //
//TempCode JIM 12Dec95 //Description	DEADED MOST OF THIS 'COS THEY ARE one-off SCREEN structures 
//TempCode JIM 12Dec95 //				rather than WINDOW structures.
//TempCode JIM 12Dec95 //				I am coming to the conclusion that we will need a hardware class 
//TempCode JIM 12Dec95 //				arround the window class, where the card-specific vars for the
//TempCode JIM 12Dec95 //				graphics driver go. 
//TempCode JIM 12Dec95 //				This will need deleting, and these bits 'o memory can be freed there.
//TempCode JIM 12Dec95 //
//TempCode JIM 12Dec95 //Inputs	
//TempCode JIM 12Dec95 //
//TempCode JIM 12Dec95 //Returns	
//TempCode JIM 12Dec95 //
//TempCode JIM 12Dec95 //Externals
//TempCode JIM 12Dec95 //------------------------------------------------------------------------------
//TempCode JIM 12Dec95 //------------------------------------------------------------------------------
//TempCode JIM 12Dec95 void MigWindow::DestroyWindow(ScreenRecord *screen)
//TempCode JIM 12Dec95 {
//TempCode JIM 12Dec95 //DeadCode JIM 09Nov95 	if(screen->LogicalScreenPtr!=NULL)	free(screen->LogicalScreenPtr);	//NOOO!! What if proper pseudo-screening using flat hardware?
//TempCode JIM 12Dec95 //DeadCode JIM 09Nov95 	if(screen->Palette!=NULL) free(screen->Palette);	//NOOOOOOO!! Windows share pallettes
//TempCode JIM 12Dec95 //DeadCode JIM 09Nov95 	if(screen->fontptr!=NULL) free(screen->FontPtr);	//NOOOOOOO!! Windows share fonts
//TempCode JIM 12Dec95 	screen->LogicalScreenPtr = screen->Palette = screen->fontptr = NULL;
//TempCode JIM 12Dec95 	delete(screen);
//TempCode JIM 12Dec95 }

//------------------------------------------------------------------------------
//Procedure	MigWindow::MouseRedraw
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MigDisplay::MouseRedraw()
{

}

//------------------------------------------------------------------------------
//Procedure	MigWindow::GetCharPos
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MigWindow::GetCharPos(short& cursorx, short& cursory)
{
	cursorx = (short) (CharX*virtualXscale);
	cursory = (short) (CharY*virtualYscale);
}

//------------------------------------------------------------------------------
SWord MigWindow::GetCharPos()					//JIM 09Nov95
{																//JIM 09Nov95
	return((short)(CharX*virtualXscale));		//JIM 10Nov95
}																//JIM 09Nov95

//------------------------------------------------------------------------------
//Procedure	MigWindow::SetCharPos
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MigWindow::SetCharPos(short cursorx, short cursory)
{
	CharX = (short)(cursorx/virtualXscale);
	CharY = (short)(cursory/virtualYscale);
}

//------------------------------------------------------------------------------
//Procedure	MigWindow::ScreenSwap
//------------------------------------------------------------------------------
//Author	Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	The many ways to swap the screen and hide the mouse redraw
//				Also incoorporates the 2cs delay
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void	MigDisplay::ScreenSwap()
{
	UByte		*mouseimage;

	LogicalPtr	oldscr=logicalscreenptr;

	if (NumberOfScreens==0)	//debug physical - need better test...
	{
//Nothing
	}

	ULong	counter = 1000000;

	switch (NumberOfScreens)
	{
	case 0:	//debug physical - need better test...

		MigDisplay::ScreenFlip();										//JIM 13Dec95
		if (UseNewPal)
			MigDisplay::PalChange();
		UseNewPal=FALSE;
		WindowMoved(oldscr,logicalscreenptr);
	break;
	case 1:		//single copy screen
		MigDisplay::ScreenFlip();										//JIM 13Dec95
		static char i;
		if ((i+=4)==0 || UseNewPal)
			MigDisplay::PalChange();
		UseNewPal=FALSE;
		WindowMoved(oldscr,logicalscreenptr);
	break;
	case 2:
		MigDisplay::ScreenFlip();										//JIM 13Dec95
		if (UseNewPal)
			MigDisplay::PalChange();
		UseNewPal=FALSE;
		WindowMoved(oldscr,logicalscreenptr);
	break;
	case 3:
		MigDisplay::ScreenFlip();										//JIM 13Dec95
		if (UseNewPal)
			MigDisplay::PalChange();
		UseNewPal=FALSE;
		WindowMoved(oldscr,logicalscreenptr);					//JIM 04Aug96
	break;
	}
}

//------------------------------------------------------------------------------
//Procedure		DoHardPoly
//Author		Paul.   
//Date			Fri 7 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::DoHardPoly(LPPOLYTYPE lpPolyType){return (HardPoly(lpPolyType));}

void MigDisplay::DoPushViewData() {PushViewData();}
void MigDisplay::DoPopViewData() {PopViewData();}
void MigDisplay::DoSet3DWin(SLong x,SLong y,SLong w,SLong h) {Set3DWin(x,y,w,h);}

bool MigDisplay::DoScanForBottomPixelsOfText(SWord a,SWord b,SWord c)
{
	return ScanForBottomPixelsOfText(a,b,c);
}

void MigDisplay::DoPushPull(UByte a) {PushPull(a);}

void	MigDisplay::DoMakePaletteFadeTable(UByte* a,UWord* b,UByte c,UByte d,UByte e)
{	MakePaletteFadeTable(a,b,c,d,e);}

void MigDisplay::DoMakeColourRangeFadeTable(UWord* a,UByte b,UByte c,UByte d,UByte e,UByte f,UByte g)
{
	MakeColourRangeFadeTable(a,b,c,d,e,f,g);
}

void MigDisplay::DoGetColourInfo(UWord& a,SWord& b,SWord& c,UWord& d,SWord& e,SWord& f,UWord& g,SWord& h,SWord& i)
{GetColourInfo(a,b,c,d,e,f,g,h,i);}

void	MigDisplay::DoLockScr(ROWANSURFACEDESC* a) {LockScr(a);}
void	MigDisplay::DoUnlockScr(ROWANSURFACEDESC* a) {UnlockScr(a);}

void MigDisplay::DoPutLine(SWord a,SWord b,SWord c,SWord d,Colour e){PutLine(a,b,c,d,e);}
void	MigDisplay::DoScreenSwap() {ScreenSwap();}
void	MigDisplay::DoCaptureScreen() {CaptureScreen();}
void	MigDisplay::DoSaveVideoBuffer() {SaveVideoBuffer();}
//------------------------------------------------------------------------------
//Procedure		DoGammaCorr
//Author		Paul.   
//Date			Wed 23 Dec 1998
//------------------------------------------------------------------------------
void MigDisplay::DoGammaCorr(UByte& r,UByte& g,UByte& b)
{GammaCorr(r,g,b);}

//------------------------------------------------------------------------------
//Procedure		DoSetEdgeTest
//Author		Paul.   
//Date			Thu 10 Dec 1998
//------------------------------------------------------------------------------
bool MigDisplay::DoSetEdgeTest(bool a) {return SetEdgeTest(a);}

//------------------------------------------------------------------------------
//Procedure		DoSetSunPos
//Author		Paul.   
//Date			Fri 27 Nov 1998
//------------------------------------------------------------------------------
void MigDisplay::DoSetSunPos(DoPointStruc& a){SetSunPos(a);}

//------------------------------------------------------------------------------
//Procedure		DoSetMipMap
//Author		Paul.   
//Date			Tue 24 Nov 1998
//------------------------------------------------------------------------------
bool MigDisplay::DoSetMipMap(bool a) {return SetMipMap(a);}

//------------------------------------------------------------------------------
//Procedure		DoPeripheralBlob
//Author		Paul.   
//Date			Mon 23 Nov 1998
//------------------------------------------------------------------------------
void MigDisplay::DoPeripheralBlob(Float* a,Float* b,Float* c,UByte d,SLong e)
{
	if (!Save_Data.fSoftware)	PeripheralBlob(a, b, c, d, e);
	else						POLYGON.PeripheralBlob(a, b, c, d, e);
}

void MigDisplay::DoSetFontColour(ULong a)
{SetFontColour(a);}

void MigDisplay::DoGetSurfaceDimensions(SLong& a,SLong& b)
{GetSurfaceDimensions(a,b);}

void MigDisplay::DoPutC(ImageMapDesc* a,DoPointStruc* b,bool fRefresh)
{PutC(a,b,fRefresh);}

void MigDisplay::DoPutA(ImageMapDesc* a,DoPointStruc* b)
{PutA(a,b);}

void MigDisplay::Do2DLine(UWord a,UWord b,UWord c,UWord d)
{D2DLine(a,b,c,d);}

//------------------------------------------------------------------------------
//Procedure		DoRenderTileToDDSurface
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
void MigDisplay::DoRenderTileToDDSurface(UByte* a,const HTEXT& b)
{RenderTileToDDSurface(a,b);}

//------------------------------------------------------------------------------
//Procedure		DoGetTextureScales
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
void MigDisplay::DoGetTextureScales(SLong& a,SLong& b,SLong& c,SLong& d,SLong& e,SLong& f)
{GetTextureScales(a,b,c,d,e,f);}

//------------------------------------------------------------------------------
//Procedure		DoFreeTexture
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
void	MigDisplay::DoFreeTexture(const HTEXT& a) {FreeTexture(a);}

//------------------------------------------------------------------------------
//Procedure		DoFreeTexture
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
void	MigDisplay::DoFreeTextures() {FreeTextures();}

//------------------------------------------------------------------------------
//Procedure		DoAllocateTexture
//Author		Paul.   
//Date			Mon 9 Nov 1998
//------------------------------------------------------------------------------
HTEXT	MigDisplay::DoAllocateTexture(UByte a) {return AllocateTexture(a);}

void MigDisplay::DoFlushAll(Bool f){FlushAll(f);}

SWord	MigDisplay::DoForceTQ(SWord a) {return ForceTQ(a);}

//------------------------------------------------------------------------------
//Procedure		DoCalcItemFog
//Author		Paul.   
//Date			Wed 9 Sep 1998
//------------------------------------------------------------------------------
void MigDisplay::DoCalcItemFog(bool a,Float& b,Float& c,Float& d)
{
	CalcItemFog(a,b,c,d);
}

bool  MigDisplay::DoSetTranspFade(bool a)
{
	return SetTranspFade(a);
}

bool	MigDisplay::DoTestAsyncBlt() {return TestAsyncBlt();}

void MigDisplay::DoCreateLandTBuffers() {CreateLandTBuffers();}
void*	MigDisplay::DoGetLandBufferPtr(SWord a) {return GetLandBufferPtr(a);}
void*	MigDisplay::DoGetLandPalLookup() {return GetLandPalLookup();}
void	MigDisplay::DoSetLandPalLookup(UByte* a) {SetLandPalLookup(a);}
SWord	MigDisplay::DoGetLandBpp() {return GetLandBpp();}

//------------------------------------------------------------------------------
//Procedure		DoNewTile
//Author		Paul.   
//Date			Mon 27 Jul 1998
//------------------------------------------------------------------------------
void MigDisplay::DoNewTile(SLong z,DoPointStruc* a,SLong b,UByte* c,SLong d)
{
	NewTile(z,a,b,c,d);
}

//------------------------------------------------------------------------------
//Procedure		DoCNewTile
//Author		Paul.   
//Date			Mon 27 Jul 1998
//------------------------------------------------------------------------------
void MigDisplay::DoCNewTile(SLong z,DoPointStruc* a,SLong b,UByte* c,SLong d)
{	
	CNewTile(z,a,b,c,d);
}

void	MigDisplay::DoBeginTextureScroll(const SWord a,void* b,void* c)
{
	BeginTextureScroll(a,b,c);
}
void	MigDisplay::DoWaitTextureScrollComplete(const SWord a)
{
	WaitTextureScrollComplete(a);
}

void	MigDisplay::DoPartialTextureUpload(const SWord a,void* b,void* c)
{
	PartialTextureUpload(a,b,c);
}
void	MigDisplay::DoShowLoaderArt(void* pArt)
{
	ShowLoaderArt(pArt);
}

void MigDisplay::DoFlushLandDraw() {FlushLandDraw();}
void MigDisplay::DoSetAspectRatio(Float ar,Float fov) {SetAspectRatio(ar,fov);}

//------------------------------------------------------------------------------
//Procedure		DoCLEB
//Author		Paul.   
//Date			Fri 19 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::DoCLEB() {CLEB();}
//------------------------------------------------------------------------------
//Procedure		DoAddTile
//Author		Paul.   
//Date			Fri 19 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::DoAddTile(ULong a,ULong aa,DoPointStruc* b,ULong c,UByte*& d)
{AddTile(a,aa,b,c,d);}
//------------------------------------------------------------------------------
//Procedure		DoCAddTile
//Author		Paul.   
//Date			Fri 19 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::DoCAddTile(ULong a,ULong aa,DoPointStruc* b,ULong c,UByte*& d)
{CAddTile(a,aa,b,c,d);}
//------------------------------------------------------------------------------
//Procedure		DoAddTile
//Author		Paul.   
//Date			Fri 19 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::DoNearAddTile(ULong a,ULong aa,DoPointStruc* b,ULong c,UByte*& d,ULong e)
{NearAddTile(a,aa,b,c,d,e);}
//------------------------------------------------------------------------------
//Procedure		DoCNearAddTile
//Author		Paul.   
//Date			Wed 24 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::DoCNearAddTile(ULong a,ULong aa,DoPointStruc* b,ULong c,UByte*& d,ULong e)
{CNearAddTile(a,aa,b,c,d,e);}

void	MigDisplay::DoCNearAddTileX(ULong a,ULong b,DoPointStruc* c,ULong d,UByte*& e,const HTEXT& f,UWord flag)
{CNearAddTileX(a,b,c,d,e,f,flag);}

void	MigDisplay::DoNearAddTileX(ULong a,ULong b,DoPointStruc* c,ULong d,UByte*& e,const HTEXT& f,UWord flag)
{NearAddTileX(a,b,c,d,e,f,flag);}

void	MigDisplay::DoNearAddDitherX(ULong a,ULong b,DoPointStruc* c,ULong d,UByte*& e)
{NearAddDitherX(a,b,c,d,e);}

//------------------------------------------------------------------------------
//Procedure		DoSmokedGlassBox
//Author		Paul    
//Date			Sun 7 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::DoSmokedGlassBox(SWord a,SWord b,SWord c,SWord d,ULong e)
{
	SmokedGlassBox(a,b,c,d,e);
}

//------------------------------------------------------------------------------
//Procedure		DoBeginFan
//Author		Paul.   
//Date			Tue 2 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::DoBeginFan(ULong hTexture) {BeginFan(hTexture);}
//------------------------------------------------------------------------------
//Procedure		DoBeginFan
//Author		Paul.   
//Date			Tue 2 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::DoBeginFan(ImageMapDesc* pImage) {BeginFan(pImage);}
//------------------------------------------------------------------------------
//Procedure		DoAddFanPoint
//Author		Paul.   
//Date			Tue 2 Jun 1998
//------------------------------------------------------------------------------
void MigDisplay::DoAddFanPoint(DoPointStruc* pdp,SLong vCnt,UByte*& pData)
{AddFanPoint(pdp,vCnt,pData);}
//------------------------------------------------------------------------------
//Procedure		DoUploadLandTexture
//Author		Paul.   
//Date			Mon 18 May 1998
//------------------------------------------------------------------------------
void MigDisplay::DoUploadLandTexture(ULong hTexture,ImageMapDesc* lpImage)
{UploadLandTexture(hTexture,lpImage);}
//------------------------------------------------------------------------------
//Procedure		DoSetClipState
//Author		Paul.   
//Date			Wed 29 Apr 1998
//------------------------------------------------------------------------------
bool MigDisplay::DoSetClipState(bool newState){return SetClipState(newState);}
//------------------------------------------------------------------------------
//Procedure		DoReleaseTextures
//Author		Paul.   
//Date			Tue 21 Apr 1998
//------------------------------------------------------------------------------
void MigDisplay::DoReleaseTextures(){ReleaseTextures();}
//------------------------------------------------------------------------------
//Procedure		DoSetTextureQuality
//Author		Paul.   
//Date			Mon 20 Apr 1998
//------------------------------------------------------------------------------
void MigDisplay::DoSetTextureQuality(bool b){SetTextureQuality(b);}
//------------------------------------------------------------------------------
//Procedure		DoSetRangeFog
//Author		Paul.   
//Date			Wed 8 Apr 1998
//------------------------------------------------------------------------------
bool MigDisplay::DoSetRangeFog(bool b){return SetRangeFog(b);}
//------------------------------------------------------------------------------
//Procedure		DoGetHorizonColour
//Author		Paul.   
//Date			Mon 6 Apr 1998
//------------------------------------------------------------------------------
void MigDisplay::DoGetHorizonColour(UByte& a,UByte& b,UByte& c)
{GetHorizonColour(a,b,c);}
//------------------------------------------------------------------------------
//Procedure		DoSetHorizonColour
//Author		Paul.   
//Date			Mon 6 Apr 1998
//------------------------------------------------------------------------------
void MigDisplay::DoSetHorizonColour(UByte a,UByte b,UByte c){SetHorizonColour(a,b,c);}
//------------------------------------------------------------------------------
//Procedure		DoSetShadeVals
//Author		Paul.   
//Date			Wed 1 Apr 1998
//------------------------------------------------------------------------------
void MigDisplay::DoSetShadeVals(SLong nr,SLong ng,SLong nb,SLong er,SLong eg,SLong eb){SetShadeVals(nr,ng,nb,er,eg,eb);}
//------------------------------------------------------------------------------
//Procedure		DoSetLShade
//Author		Paul.   
//Date			Wed 1 Apr 1998
//------------------------------------------------------------------------------
bool MigDisplay::DoSetLShade(bool b){return SetLShade(b);}
//------------------------------------------------------------------------------
//Procedure		DoSetGlobalAlpha
//Author		Paul.   
//Date			Fri 27 Mar 1998
//------------------------------------------------------------------------------
UByte MigDisplay::DoSetGlobalAlpha(UByte a)
{
	return SetGlobalAlpha(a);
}

//------------------------------------------------------------------------------
//Procedure		DoForceRender
//Author		Paul.   
//Date			Tue 24 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoForceRender()
{
	ForceRender();
}

//------------------------------------------------------------------------------
//Procedure		DomeHorizon
//Author		Paul.   
//Date			Wed 26 Nov 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoDomeHorizon(void* lpData,void* lpPolys)
{
	DomeHorizon(lpData,lpPolys);
}

//------------------------------------------------------------------------------
//Procedure		BackupLogicalPtr
//Author		Paul.   
//Date			Tue 21 Oct 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoBackupLogicalPtr() {BackupLogicalPtr();}		//PD 21Oct97
//------------------------------------------------------------------------------
//Procedure		ResetLogicalPtr
//Author		Paul.   
//Date			Tue 21 Oct 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoResetLogicalPtr() {ResetLogicalPtr();}			//PD 21Oct97

//------------------------------------------------------------------------------
//Procedure		DoSetFogState
//Author		Paul.   
//Date			Thu 9 Oct 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoSetFogState(Bool bFogState)
{
	SetFogState(bFogState);
}

//------------------------------------------------------------------------------
//Procedure		DoBody2Screen
//Author		Paul.   
//Date			Wed 25 Nov 1998
//------------------------------------------------------------------------------
void MigDisplay::DoBody2Screen(DoPointStruc& a)
{
	Body2Screen(a);
}

//------------------------------------------------------------------------------
//Procedure		DoSetFadeValue
//Author		Paul.   
//Date			Sat 7 Jun 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoSetFadeValue(UByte newValue)
{
 	SetFadeValue(newValue);
}

//------------------------------------------------------------------------------
//Procedure		DoEnableBackClip
//Author		Paul.   
//Date			Thu 5 Jun 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::DoEnableBackClip(Bool bFlag)
{
 	return (EnableBackClip(bFlag));
}	

//------------------------------------------------------------------------------
//Procedure		DoSetFade
//Author		Paul.   
//Date			Thu 5 Jun 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoSetFade(Colour trgCol,SLong step,TwoDeeEffect tdfx)
{
	SetFade(trgCol,step,tdfx);
}

//------------------------------------------------------------------------------
//Procedure		XModeSupported
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
Bool MigDisplay::XModeSupported(ScreenRez* lpSRez)
{
 	return (ModeSupported(lpSRez));
}

//------------------------------------------------------------------------------
//Procedure		XDriverIsCapable
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
Bool MigDisplay::XDriverIsCapable(SWord drvno)
{
 	return (DriverIsCapable(drvno));
}

//------------------------------------------------------------------------------
//Procedure		DoNameNumberedDriver
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
UByte* MigDisplay::DoNameNumberedDriver(SWord driverno)
{
 	return (NameNumberedDriver(driverno));
}

//------------------------------------------------------------------------------
//Procedure		DoCountDrivers
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
SWord	MigDisplay::DoCountDrivers()
{
	return (CountDrivers());
}

//------------------------------------------------------------------------------
//Procedure		DoHardLine
//Author		Paul.   
//Date			Mon 28 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::DoHardLine(LPLINETYPE lpLineType)
{
	return (HardLine(lpLineType));
}

//------------------------------------------------------------------------------
//Procedure		DoHardPoint
//Author		Paul.   
//Date			Mon 28 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::DoHardPoint(LPPOINTTYPE lpPointType)
{
	return (HardPoint(lpPointType));
}

//------------------------------------------------------------------------------
//Procedure		DoHardPlainRectFill
//Author		Paul.   
//Date			Fri 18 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::DoHardPlainRectFill(MigWindow*win,SWord x, SWord y, SWord w, SWord h)
{
 	return(HardPlainRectFill(win,x,y,w,h));
}

//------------------------------------------------------------------------------
//Procedure		DoSetViewParams
//Author		Paul.   
//Date			Fri 18 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoSetViewParams(ANGLES a,MigWindow* w,ViewPoint* v)
{
 	SetViewParams(a,w,v);
}

//------------------------------------------------------------------------------
//Procedure		DoSetZUpdate
//Author		Paul.   
//Date			Mon 7 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::DoSetZUpdate(Bool flag)
{
	return (SetZUpdate(flag));
}

//------------------------------------------------------------------------------
//Procedure		DoSetPlayerTextures
//Author		Paul.   
//Date			Thu 8 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::DoSetPlayerTextures(Bool flag)
{
	return (SetPlayerTextures(flag));
}

//------------------------------------------------------------------------------
//Procedure		DoingHardware3D
//Author		Paul.   
//Date			Tue 25 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::DoingHardware3D()
{
 	return (Hardware3D());
}

//------------------------------------------------------------------------------
//Procedure		DoSetMatrices
//Author		Paul.   
//Date			Mon 24 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool MigDisplay::DoSetMatrices(SLong viewrange,ANGLES viewang)
{
 	return (SetMatrices(viewrange,viewang));
}
//------------------------------------------------------------------------------
//Procedure		DoSetFog
//Author		Paul.   
//Date			Mon 24 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoSetFog(UByte r,UByte g,UByte b,SLong start,SLong end)
{
	if (!Save_Data.fSoftware)	SetFog(r,g,b,start,end);
	else						POLYGON.SetFog(r,g,b,start,end);
}

//------------------------------------------------------------------------------
//Procedure		DoSetFog
//Author		Paul.   
//Date			Fri 23 Oct 1998
//------------------------------------------------------------------------------
void	MigDisplay::DoSetFog(UByte r,UByte g,UByte b)
{
	if (!Save_Data.fSoftware)	SetFog(r,g,b);
	else						POLYGON.SetFog(r,g,b);
}

//------------------------------------------------------------------------------
//Procedure		DoGetFog
//Author		Paul.   
//Date			Fri 23 Oct 1998
//------------------------------------------------------------------------------
void	MigDisplay::DoGetFog(UByte& r,UByte& g,UByte& b,SLong& s,SLong& e)
{
	if (!Save_Data.fSoftware)	GetFog(r,g,b,s,e);
	else						POLYGON.GetFog(r,g,b,s,e);
}

//------------------------------------------------------------------------------
//Procedure		DoBeginScene
//Author		Paul.   
//Date			Mon 17 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoBeginScene()
{
	BeginScene();
}

//------------------------------------------------------------------------------
//Procedure		DoEndScene
//Author		Paul.   
//Date			Mon 17 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoEndScene()
{
	EndScene();
}

void MigDisplay::DoEnd3DScene()
{
	End3DScene();
}
//------------------------------------------------------------------------------
//Procedure		HClearScreen
//Author		Paul.   
//Date			Mon 10 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::HClearScreen()
{
 	HCls(FALSE);												//PD 10Jun97
}


//------------------------------------------------------------------------------
//Procedure	MigWindow::PlotLogicalPixel
//------------------------------------------------------------------------------
//Author	Paul.   
//Date		Mon 3 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
Colour	MigWindow::PlotLogicalPixel(SWord X, SWord Y,Colour C)//PD 29Nov95
{
	return(Graphic::PlotPixel(X/virtualXscale,Y/virtualYscale,C));//JIM 05Feb96
}

Colour	MigWindow::PlotLogicalPixel(SWord X, SWord Y)//PD 29Nov95
{
	return(Graphic::PlotPixel(X/virtualXscale,Y/virtualYscale));//JIM 05Feb96
}

//------------------------------------------------------------------------------
//Procedure	MigWindow::ClearScreen
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Wed 5 Jul 1995
//Modified	
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void MigWindow::ClearScreen(Colour colour)	//PD 29Nov95
{
	SetColour(colour);								//PD 29Nov95
	Graphic::ClearScreen();								//PD 29Nov95
}

//------------------------------------------------------------------------------
//Procedure		PlainLine
//Author		Paul.   
//Date			Tue 19 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::PlainLine(SLong x1,SLong y1,SLong x2,SLong y2)//PD 29Nov95
{
	switch (BytesPerPixel)
	{
		case 1:
	 	Graphic::PlainLine(	(SWord)(x1/virtualXscale), (SWord)(y1/virtualYscale),
						   	(SWord)(x2/virtualXscale), (SWord)(y2/virtualYscale));
		break;
		case 2:
	 	Graphic::PlainLine16(	(SWord)(x1/virtualXscale), (SWord)(y1/virtualYscale),
						   		(SWord)(x2/virtualXscale), (SWord)(y2/virtualYscale));
		break;
	}
}

//------------------------------------------------------------------------------
//Procedure		ClipLine
//Author		Martin Alderton
//Date			Fri 22 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::ClipLine(SLong x1,SLong y1,SLong x2,SLong y2)//PD 29Nov95
{
 	Graphic::ClipLine(	(SWord)(x1/virtualXscale), 	(SWord)(y1/virtualYscale),
					   	(SWord)(x2/virtualXscale),	(SWord)(y2/virtualYscale));
}
//------------------------------------------------------------------------------
//Procedure		ClipLine
//Author		Martin Alderton
//Date			Fri 22 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::ClipLine3D(SLong x1,SLong y1,SLong x2,SLong y2)//MGA19Feb96
{
 	Graphic::ClipLine3D(	x1, y1,
					   	x2,	y2,
						virtualXscale,
						virtualYscale);
}

//------------------------------------------------------------------------------
//Procedure		ClipLine3D
//Author		Robert Slater
//Date			Tue 25 Feb 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::ClipLine3D(Float x1,Float y1,Float x2,Float y2)
{
 	Graphic::ClipLine3D(x1, y1, x2,	y2,
						virtualXscale,
						virtualYscale);
}


//------------------------------------------------------------------------------
//Procedure		zclipline
//Author		Martin Alderton
//Date			Wed 6 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::zclipline(DoPointStruc& start_vertex, DoPointStruc& end_vertex)
{
	Graphic::zclipline(start_vertex,end_vertex);
}
//------------------------------------------------------------------------------
//Procedure		PlainCircle
//Author		Martin Alderton
//Date			Fri 22 Sep 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::PlainCircle(SLong radius, SLong x1,SLong y1)//PD 29Nov95
{	//should we scale radius by x or y? usually they are the same.
	Graphic::PlainCircle(	(SWord)	(radius/virtualXscale),
							(SWord)	(x1/virtualXscale),	(SWord)	(y1/virtualYscale));
}

//------------------------------------------------------------------------------
//Procedure		TransparentBlob
//Author		Paul.   
//Date			Mon 28 Jul 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::TransparentBlob(	SWord clipInfo,
								SWord intens,
								SLong radius,
								SLong x,
								SLong y)
{
 	Graphic::TransparentBlob(	clipInfo, intens,
								(SWord)(radius/virtualXscale),
								(SWord)(x/virtualXscale),
								(SWord)(y/virtualYscale));
}

//------------------------------------------------------------------------------
//Procedure		GetRealSxy
//Author		Paul.   
//Date			Thu 26 Oct 1995
//
//Description	Scales virtual screen coordinates to physical screen coordinates
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::GetTrunkSxy(SLong &screenx, SLong &screeny)		//PD 29Nov95
{
	screenx-=screenx%virtualXscale;
	screeny-=screeny%virtualYscale;
}

//------------------------------------------------------------------------------
//Procedure		GetScanLineAddr
//Author		Paul.   
//Date			Wed 3 Jan 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
HoriLineRtnPtr MigWindow::GetScanLineAddr(SCANLINETYPE type)
{
	return(Graphic::GetScanLineAddr(type));
}

//------------------------------------------------------------------------------
//Procedure		MakeIntensityData
//Author		Paul.   
//Date			Wed 8 Nov 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::MakeIntensityData()
{
	Graphic::MakeIntensityData();								//PD 29Nov95
}


//------------------------------------------------------------------------------
//Procedure		RectangleFill
//Author		Paul.   
//Date			Fri 18 Apr 1997
//
//Description	Plain colour rectangle fill (now tries hardware accel first
//				before doing a software version)
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::RectangleFill(SLong x,SLong y,ULong w,ULong h)
{
	SWord	sx,sy,sw,sh;

	sx = (SWord)(x/virtualXscale);
	sy = (SWord)(y/virtualYscale);
	sw = (SWord)(w/virtualXscale);
	sh = (SWord)(h/virtualYscale);

	if (!Master()->DoHardPlainRectFill(this,sx,sy,sw,sh))
 		Graphic::PlainRectangleFill(sx,sy,sw,sh);
}

//------------------------------------------------------------------------------
//Procedure		AilRectFill
//Author		Paul.   
//Date			Fri 21 Jun 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::AilRectFill(ULong x,ULong y,ULong w,ULong h)
{
 	Graphic::AilRectangleFill(
		(SWord)(x/virtualXscale),
		(SWord)(y/virtualYscale),
		(SWord)(w/virtualXscale),
		(SWord)(h/virtualYscale));
}

//------------------------------------------------------------------------------
//Procedure		GlassRectFill
//Author		Paul.   
//Date			Tue 2 Apr 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::GlassRectFill(ULong x,ULong y,ULong w,ULong h)
{
 	Graphic::GlassRectangleFill(
		(SWord)(x/virtualXscale),
		(SWord)(y/virtualYscale),
		(SWord)(w/virtualXscale),
		(SWord)(h/virtualYscale));
}

//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		conv
//FIX10-2//Author		Jim Taylor
//FIX10-2//Date			Wed 24 Jul 1996
//FIX10-2//
//FIX10-2//Description	Converts coords from 1 window to another
//FIX10-2//
//FIX10-2//Inputs		
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2ScreenXY	MigWindow::conv(ScreenXY srcxy,const Graphic* srcwin) const
//FIX10-2{
//FIX10-2	static SLong olddelta=0;
//FIX10-2	static	SWord	x=0,y=0;
//FIX10-2SLong	delta=(srcwin->logicalscreenptr-logicalscreenptr);
//FIX10-2
//FIX10-2	if (delta!=olddelta)
//FIX10-2		if (delta)
//FIX10-2		{
//FIX10-2			olddelta=delta;
//FIX10-2			y=delta/BytesPerScanLine;
//FIX10-2			x=delta%BytesPerScanLine;
//FIX10-2			x=x/BytesPerPixel;
//FIX10-2			y=y*virtualYscale;
//FIX10-2			x=x*virtualXscale;
//FIX10-2		}
//FIX10-2		else
//FIX10-2		{
//FIX10-2			olddelta=delta;
//FIX10-2		 	y=0;
//FIX10-2			x=0;
//FIX10-2		}
//FIX10-2	srcxy.x+=x;
//FIX10-2	srcxy.y+=y;
//FIX10-2	return(srcxy);
//FIX10-2}
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		inside
//FIX10-2//Author		Jim Taylor
//FIX10-2//Date			Tue 30 Jan 1996
//FIX10-2//
//FIX10-2//Description	Needs to cope with local window rescaling
//FIX10-2//				No, I don't know how I am going to do this!
//FIX10-2//
//FIX10-2//Inputs		mouse is master based
//FIX10-2//				box is window based
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2Bool	MigWindow::inside(const ScreenXY& mouse,const ScreenXY& boxxy,const ScreenWH& boxwh) const
//FIX10-2{
//FIX10-2	ScreenXY	cm=conv(mouse,master);
//FIX10-2
//FIX10-2	if (cm.x<boxxy.x)	return(FALSE);
//FIX10-2	if (cm.x>boxxy.x+boxwh.w)	return(FALSE);
//FIX10-2	if (cm.y<boxxy.y)	return(FALSE);
//FIX10-2	if (cm.y>boxxy.y+boxwh.h)	return(FALSE);
//FIX10-2	return(TRUE);
//FIX10-2}

//------------------------------------------------------------------------------
//Procedure		ScreenShot
//Author		Dave Whiteside
//Date			Mon 19 Feb 1996
//
//Description	take ScreenShot
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	MigWindow::ScreenShot()
{
	if (BytesPerPixel==1)
		Graphic::ScreenShot();
	else
		Graphic::ScreenShot16();
}

//------------------------------------------------------------------------------
//Procedure		ScreenShot16
//Author		Paul.   
//Date			Mon 21 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	MigWindow::ScreenShot16()
{
	Graphic::ScreenShot16();
}

//------------------------------------------------------------------------------
//Procedure		CopyP2L
//Author		Jim Taylor
//Date			Tue 27 Feb 1996
//
//Description	Copy region from physical to logical.
//				This is wonderfully dodgy code...
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Graphic::CopyP2L(SWord X,SWord Y,SWord W,SWord H)
{
		LogicalPtr	S=logicalscreenptr;
		S+=X*BytesPerPixel;
		S+=Y*BytesPerScanLine;
		return(((MigDisplay*)master)->CopyP2L(S,W,H));

}

//------------------------------------------------------------------------------
//Procedure		CopyP2L
//Author		Jim Taylor
//Date			Tue 27 Feb 1996
//
//Description	Copy region from physical to logical - may do nothing on single/debug
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	MigWindow::CopyP2L(SWord X,SWord Y,SWord W,SWord H)
{
 	return	Graphic::CopyP2L(
								(SWord)(X/virtualXscale),
								(SWord)(Y/virtualYscale),
								(SWord)(W/virtualXscale),
								(SWord)(H/virtualYscale));
}

//------------------------------------------------------------------------------
//Procedure		PlotPixel
//Author		Robert Slater
//Date			Thu 14 Mar 1996
//
//Description	Plots a pixel to the display with clipping
//
//Inputs		X, Y and new colour for the pixel
//
//Returns		Old colour of the pixel
//
//------------------------------------------------------------------------------
Colour	MigDisplay::PlotPixel(SWord	x, SWord	y, Colour	thecol)
{
	SWord		MinX,MaxX,MinY,MaxY;

	MinX = PhysicalMinX;
	MaxX = MinX + PhysicalWidth;

	MinY = PhysicalMinY;
	MaxY = MinY + PhysicalHeight;

	if(x >= MinX && x < MaxX && y >= MinY && y < MaxY)
		return((Colour)PlotPixelNoClip(x,y,thecol));

	return((Colour)-1);

}
//FIX10-2
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		PlotMouseScanline
//FIX10-2//Author		Robert Slater
//FIX10-2//Date			Mon 11 Mar 1996
//FIX10-2//
//FIX10-2//Description	Plots the mouse pointer
//FIX10-2//
//FIX10-2//Inputs		
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2void	MigDisplay::PlotMouseScanline(SWord	x, SWord	y, SWord	w, UByte	*&pos)
//FIX10-2{
//FIX10-2	int		code;
//FIX10-2
//FIX10-2	UByte	*image = pos;
//FIX10-2
//FIX10-2	SWord	endx = x + w;
//FIX10-2
//FIX10-2
//FIX10-2	while (x < endx)
//FIX10-2	{
//FIX10-2		code = *(image++);
//FIX10-2
//FIX10-2		if (code == 0x80)
//FIX10-2		{
//FIX10-2			x = x + 1 + *(image++);
//FIX10-2		}
//FIX10-2		elseif (code < 0x80)
//FIX10-2			{
//FIX10-2				while (code >= 0)
//FIX10-2				{
//FIX10-2					PlotPixel(x++, y, (Colour)*(image++));
//FIX10-2
//FIX10-2					code--;
//FIX10-2				}
//FIX10-2			}
//FIX10-2			else
//FIX10-2			{
//FIX10-2#ifdef __WATCOMC__
//FIX10-2				INT3();
//FIX10-2#else
//FIX10-2#ifdef __MSVC__
//FIX10-2				INT3;
//FIX10-2#endif
//FIX10-2#endif
//FIX10-2			}
//FIX10-2	}
//FIX10-2
//FIX10-2	pos = image;
//FIX10-2}
//FIX10-2
//FIX10-2
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		PlotAndSaveMouseScanline
//FIX10-2//Author		Robert Slater
//FIX10-2//Date			Mon 11 Mar 1996
//FIX10-2//
//FIX10-2//Description	Saves current image and plots the mouse pointer over it
//FIX10-2//
//FIX10-2//Inputs		
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2void	MigDisplay::PlotAndSaveMouseScanline(SWord	x, SWord	y, SWord	w, UByte	*&oldpos, UByte	*&newpos)
//FIX10-2{
//FIX10-2	int		code, gap, colour;
//FIX10-2	UByte	*newimage = newpos;
//FIX10-2	UByte	*oldimage = oldpos;
//FIX10-2	
//FIX10-2	SWord	endx = x + w;
//FIX10-2
//FIX10-2
//FIX10-2	while (x < endx)
//FIX10-2	{
//FIX10-2		code = *(oldimage++);
//FIX10-2
//FIX10-2		*(newimage++) = (UByte) code;
//FIX10-2
//FIX10-2		if (code == 0x80)
//FIX10-2		{
//FIX10-2			gap = *(oldimage++);
//FIX10-2
//FIX10-2			*(newimage++) = (UByte) gap;
//FIX10-2
//FIX10-2			x = x + gap + 1;
//FIX10-2		}
//FIX10-2		elseif (code < 0x80)
//FIX10-2			{
//FIX10-2				while (code >= 0)
//FIX10-2				{
//FIX10-2					colour = *(oldimage++);
//FIX10-2
//FIX10-2					*(newimage++) = (UByte) PlotPixel(x++, y, (Colour)colour);
//FIX10-2
//FIX10-2					code--;
//FIX10-2				}
//FIX10-2			}
//FIX10-2			else
//FIX10-2			{
//FIX10-2#ifdef __WATCOMC__
//FIX10-2				INT3();
//FIX10-2#else
//FIX10-2#ifdef __MSVC__
//FIX10-2				INT3;
//FIX10-2#endif
//FIX10-2#endif
//FIX10-2			}
//FIX10-2	}
//FIX10-2
//FIX10-2	newpos = newimage;
//FIX10-2	oldpos = oldimage;
//FIX10-2
//FIX10-2}
//FIX10-2
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		PlotMouseScanline
//FIX10-2//Author		Robert Slater
//FIX10-2//Date			Tue 12 Mar 1996
//FIX10-2//
//FIX10-2//Description	
//FIX10-2//
//FIX10-2//Inputs		
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2void	Graphic::PlotMouseScanline(SWord	x, SWord	y, SWord	w, UByte	*&pos)
//FIX10-2{
//FIX10-2	int		code;
//FIX10-2
//FIX10-2	UByte	*image = pos;
//FIX10-2
//FIX10-2	SWord	endx = x + w;
//FIX10-2
//FIX10-2
//FIX10-2	while (x < endx)
//FIX10-2	{
//FIX10-2		code = *(image++);
//FIX10-2
//FIX10-2		if (code == 0x80)
//FIX10-2		{
//FIX10-2			x = x + 1 + *(image++);
//FIX10-2		}
//FIX10-2		elseif (code < 0x80)
//FIX10-2			{
//FIX10-2				while (code >= 0)
//FIX10-2				{
//FIX10-2					PlotPixel(x++, y, (Colour)*(image++));
//FIX10-2
//FIX10-2					code--;
//FIX10-2				}
//FIX10-2			}
//FIX10-2			else
//FIX10-2			{
//FIX10-2#ifdef __WATCOMC__
//FIX10-2				INT3();
//FIX10-2#else
//FIX10-2#ifdef __MSVC__
//FIX10-2				INT3;
//FIX10-2#endif
//FIX10-2#endif
//FIX10-2			}
//FIX10-2	}
//FIX10-2
//FIX10-2	pos = image;
//FIX10-2}
//FIX10-2
//FIX10-2
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		PlotAndSaveMouseScanline
//FIX10-2//Author		Robert Slater
//FIX10-2//Date			Mon 11 Mar 1996
//FIX10-2//
//FIX10-2//Description	
//FIX10-2//
//FIX10-2//Inputs		
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2void	Graphic::PlotAndSaveMouseScanline(SWord	x, SWord	y, SWord	w, UByte	*&oldpos, UByte	*&newpos)
//FIX10-2{
//FIX10-2	int		code, colour;
//FIX10-2	UByte	*newimage = newpos;
//FIX10-2	UByte	*oldimage = oldpos;
//FIX10-2	
//FIX10-2	SWord	endx = x + w;
//FIX10-2
//FIX10-2
//FIX10-2	while (x < endx)
//FIX10-2	{
//FIX10-2		code = *(oldimage++);
//FIX10-2
//FIX10-2		*(newimage++) = (UByte) code;
//FIX10-2
//FIX10-2		if (code == 0x80)
//FIX10-2		{
//FIX10-2			x = x + 1 + (*(newimage++) = *(oldimage++));
//FIX10-2
//FIX10-2
//FIX10-2		}
//FIX10-2		elseif (code < 0x80)
//FIX10-2			{
//FIX10-2				while (code >= 0)
//FIX10-2				{
//FIX10-2					colour = *(oldimage++);
//FIX10-2
//FIX10-2					*(newimage++) = (UByte) PlotPixel(x++, y, (Colour)colour);
//FIX10-2
//FIX10-2					code--;
//FIX10-2				}
//FIX10-2			}
//FIX10-2			else
//FIX10-2			{
//FIX10-2#ifdef __WATCOMC__
//FIX10-2				INT3();
//FIX10-2#else
//FIX10-2#ifdef __MSVC__
//FIX10-2				INT3;
//FIX10-2#endif
//FIX10-2#endif
//FIX10-2			}
//FIX10-2	}
//FIX10-2
//FIX10-2	newpos = newimage;
//FIX10-2	oldpos = oldimage;
//FIX10-2
//FIX10-2}
//FIX10-2
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		PhysMouseMove
//FIX10-2//Author		Robert Slater
//FIX10-2//Date			Mon 11 Mar 1996
//FIX10-2//
//FIX10-2//Description	
//FIX10-2//
//FIX10-2//Inputs		
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2void	MigDisplay::PhysMouseMove(SWord	newx, SWord	newy, short	imagenum)
//FIX10-2{
//FIX10-2
//FIX10-2	if (BytesPerPixel!=1)
//FIX10-2//DeadCode JIM 19Jan98 	if (Whole_Screen->BytesPerPixel!=1)
//FIX10-2		return;
//FIX10-2	SWord		oldx, oldy, oldw, oldh;
//FIX10-2	SWord		neww, newh;
//FIX10-2	SWord		y, botnew, botold;
//FIX10-2	UByte		*tmpmouseshape, *tmpnewimage, *tmpoldimage;
//FIX10-2//	MouseSave	*oldpos, *newpos, *thru;
//FIX10-2
//FIX10-2
//FIX10-2	// Set up the new mouse information
//FIX10-2
//FIX10-2	
//FIX10-2	if (imagenum!=0)
//FIX10-2	{
//FIX10-2		SetMouseSaveInfo(1,imagenum,newx,newy);
//FIX10-2		tmpmouseshape = MousePixelData(imagenum);
//FIX10-2	}
//FIX10-2	else
//FIX10-2	{
//FIX10-2		SetMouseSaveInfo(1,mouseimagenumber,newx,newy);
//FIX10-2		tmpmouseshape = MousePixelData(mouseimagenumber);
//FIX10-2	}
//FIX10-2	oldpos = mousesave[0].Info;
//FIX10-2	newpos = mousesave[1].Info;
//FIX10-2
//FIX10-2
//FIX10-2	oldx = oldpos->XY.x;
//FIX10-2	oldy = oldpos->XY.y;
//FIX10-2	oldw = oldpos->WH.w;
//FIX10-2	oldh = oldpos->WH.h;
//FIX10-2
//FIX10-2	newx = newpos->XY.x;
//FIX10-2	newy = newpos->XY.y;
//FIX10-2	neww = newpos->WH.w;
//FIX10-2	newh = newpos->WH.h;
//FIX10-2
//FIX10-2	tmpnewimage = (UByte*) newpos->dataptr;
//FIX10-2	tmpoldimage = (UByte*) oldpos->dataptr;
//FIX10-2
//FIX10-2
//FIX10-2	// Check if new position is above old position on screen
//FIX10-2
//FIX10-2	botnew = newy + newh;
//FIX10-2	botold = oldy + oldh;			
//FIX10-2
//FIX10-2	if (newy < oldy)
//FIX10-2	{
//FIX10-2		y = newy;
//FIX10-2
//FIX10-2		if (botnew < oldy)
//FIX10-2		{
//FIX10-2			//	Case 1...
//FIX10-2
//FIX10-2
//FIX10-2			while (y < botnew)
//FIX10-2				PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2
//FIX10-2
//FIX10-2			// Restore old image...
//FIX10-2
//FIX10-2			y = oldy;
//FIX10-2
//FIX10-2			while (y < botold)
//FIX10-2				PlotMouseScanline(oldx, y++, oldw, tmpoldimage);
//FIX10-2		}
//FIX10-2		else
//FIX10-2		{
//FIX10-2			if (botnew < botold)
//FIX10-2			{
//FIX10-2				// 	Case 5...
//FIX10-2
//FIX10-2
//FIX10-2				while (y < oldy)
//FIX10-2					PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2
//FIX10-2				while (y < botnew)
//FIX10-2				{
//FIX10-2					PlotMouseScanline(oldx, y, oldw, tmpoldimage);
//FIX10-2					PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2				}
//FIX10-2
//FIX10-2				while (y < botold)
//FIX10-2					PlotMouseScanline(oldx, y++, oldw, tmpoldimage);
//FIX10-2			}
//FIX10-2			else
//FIX10-2			{
//FIX10-2				// 	Case 6...
//FIX10-2
//FIX10-2
//FIX10-2				while (y < oldy)
//FIX10-2					PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2
//FIX10-2				while (y < botold)
//FIX10-2				{
//FIX10-2					PlotMouseScanline(oldx, y, oldw, tmpoldimage);
//FIX10-2					PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2				}
//FIX10-2
//FIX10-2				while (y < botnew)
//FIX10-2					PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2			}
//FIX10-2		}
//FIX10-2	}
//FIX10-2	else
//FIX10-2	{
//FIX10-2		y = oldy;
//FIX10-2
//FIX10-2		if (newy > botold)
//FIX10-2		{
//FIX10-2			//	Case 2
//FIX10-2
//FIX10-2
//FIX10-2			while (y < botold)
//FIX10-2				PlotMouseScanline(oldx, y++, oldw, tmpoldimage);
//FIX10-2
//FIX10-2			y = newy;
//FIX10-2
//FIX10-2			while (y < botnew)
//FIX10-2				PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2
//FIX10-2		}
//FIX10-2		else
//FIX10-2		{
//FIX10-2			if (botold < botnew)
//FIX10-2			{
//FIX10-2				//	Case 4
//FIX10-2
//FIX10-2
//FIX10-2				while (y < newy)
//FIX10-2					PlotMouseScanline(oldx, y++, oldw, tmpoldimage);
//FIX10-2
//FIX10-2				while (y < botold)
//FIX10-2				{
//FIX10-2					PlotMouseScanline(oldx, y, oldw, tmpoldimage);
//FIX10-2					PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2				}
//FIX10-2
//FIX10-2				while (y < botnew)
//FIX10-2					PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2			}
//FIX10-2			else
//FIX10-2			{
//FIX10-2				//	Case 3
//FIX10-2
//FIX10-2
//FIX10-2				while (y < newy)
//FIX10-2					PlotMouseScanline(oldx, y++, oldw, tmpoldimage);
//FIX10-2
//FIX10-2				while (y < botnew)
//FIX10-2				{
//FIX10-2					PlotMouseScanline(oldx, y, oldw, tmpoldimage);
//FIX10-2					PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2				}
//FIX10-2
//FIX10-2				while (y < botold)
//FIX10-2					PlotMouseScanline(oldx, y++, oldw, tmpoldimage);
//FIX10-2			}
//FIX10-2		}
//FIX10-2	}
//FIX10-2
//FIX10-2	thru = mousesave[0].Info;
//FIX10-2	mousesave[0].Info = mousesave[1].Info;
//FIX10-2	mousesave[1].Info = thru;
//FIX10-2
//FIX10-2}
//FIX10-2
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		LogMouseDrawSave
//FIX10-2//Author		Robert Slater
//FIX10-2//Date			Mon 11 Mar 1996
//FIX10-2//
//FIX10-2//Description	Draws new image to logical screen and saves the old one	
//FIX10-2//
//FIX10-2//Inputs		The new image and mouse shape		
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2void	Graphic::LogMouseDrawSave(void	*MouseShape, MouseSave	*newpos)
//FIX10-2{
//FIX10-2	SWord	newx, newy, neww, newh;
//FIX10-2	SWord	y, botnew;
//FIX10-2	UByte	*tmpmouseshape, *tmpnewimage;
//FIX10-2
//FIX10-2	newx = newpos->XY.x;
//FIX10-2	newy = newpos->XY.y;
//FIX10-2	neww = newpos->WH.w;
//FIX10-2	newh = newpos->WH.h;
//FIX10-2
//FIX10-2	tmpmouseshape = (UByte*)	MouseShape;
//FIX10-2	tmpnewimage = (UByte*) newpos->dataptr;
//FIX10-2
//FIX10-2	y = newy;
//FIX10-2
//FIX10-2	botnew = newy + newh;
//FIX10-2
//FIX10-2	while (y < botnew)
//FIX10-2		PlotAndSaveMouseScanline(newx, y++, neww, tmpmouseshape, tmpnewimage);
//FIX10-2
//FIX10-2}
//FIX10-2
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		LogMouseRestore
//FIX10-2//Author		Robert Slater
//FIX10-2//Date			Mon 11 Mar 1996
//FIX10-2//
//FIX10-2//Description	Do mouse repair to logical screen
//FIX10-2//
//FIX10-2//Inputs		Mouse information
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2void	Graphic::LogMouseRestore(MouseSave	*oldpos)
//FIX10-2{
//FIX10-2	SWord	oldx, oldy, oldw, oldh;
//FIX10-2	SWord	y, botold;
//FIX10-2
//FIX10-2	UByte	*tmpoldimage;
//FIX10-2
//FIX10-2	Master()->LockBackScreen (TRUE,LOWLOCK);					//AMM 02May97
//FIX10-2
//FIX10-2	oldx = oldpos->XY.x;
//FIX10-2	oldy = oldpos->XY.y;
//FIX10-2	oldw = oldpos->WH.w;
//FIX10-2	oldh = oldpos->WH.h;
//FIX10-2
//FIX10-2	tmpoldimage = (UByte*) oldpos->dataptr;
//FIX10-2
//FIX10-2	y = oldy;
//FIX10-2
//FIX10-2	botold = oldy + oldh;
//FIX10-2
//FIX10-2
//FIX10-2	while (y < botold)
//FIX10-2		PlotMouseScanline(oldx, y++, oldw, tmpoldimage);
//FIX10-2
//FIX10-2	Master()->UnLockBackScreen (LOWLOCK);					//AMM 02May97
//FIX10-2}
//FIX10-2
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2//Procedure		LoadMouseImage
//FIX10-2//Author		Robert Slater
//FIX10-2//Date			Tue 12 Mar 1996
//FIX10-2//
//FIX10-2//Description	Load the mouse image as a font
//FIX10-2//
//FIX10-2//Inputs		
//FIX10-2//
//FIX10-2//Returns	
//FIX10-2//
//FIX10-2//------------------------------------------------------------------------------
//FIX10-2void	MigDisplay::LoadMouseImage(FileNum filenumber)
//FIX10-2{
//FIX10-2	static	fileblockptr	loadedhandle = NULL;
//FIX10-2	short					width, delta, height, x,y;
//FIX10-2	ULong					size;
//FIX10-2
//FIX10-2	//Dispose old font if any
//FIX10-2
//FIX10-2	if (mousefontptr != NULL)
//FIX10-2		delete(loadedhandle);
//FIX10-2
//FIX10-2
//FIX10-2	//Load new font data
//FIX10-2
//FIX10-2	loadedhandle = new fileblock(filenumber,FixupFontHeader);
//FIX10-2
//FIX10-2	mousefontptr = (FontPtr)	getdata(*loadedhandle);
//FIX10-2
//FIX10-2
//FIX10-2
//FIX10-2	width = (short)	(mousefontptr->charentries[0].NextXShift);
//FIX10-2	height = (short) (mousefontptr->charentries[0].Height);
//FIX10-2
//FIX10-2	size = width * height;
//FIX10-2
//FIX10-2	// Allocate 2 or more buffers for the mouse data...
//FIX10-2
//FIX10-2
//FIX10-2	mousesave[0].dataptr = new	char[size];
//FIX10-2	mousesave[0].datasize = size;
//FIX10-2
//FIX10-2	mousesave[1].dataptr = new	char[size];
//FIX10-2	mousesave[1].datasize = size;
//FIX10-2
//FIX10-2	if (NumberOfScreens > 2)
//FIX10-2	{
//FIX10-2		mousesave[2].dataptr = new char[size];
//FIX10-2		mousesave[2].datasize = size;
//FIX10-2	}
//FIX10-2
//FIX10-2
//FIX10-2
//FIX10-2}
//FIX10-2
//FIX10-2
//------------------------------------------------------------------------------
//Procedure		PrintIcon
//Author		Jim Taylor
//Date			Thu 18 Apr 1996
//
//Description	Print a character from the mouse font as an icon...
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	MigWindow::PrintIcon(int x,int y,int iconnum)
{
	if (iconnum>=mousefontptr->maxchar)
		iconnum=2;
CharTableEntryPtr
	cptr = & mousefontptr->charentries[iconnum];
	Graphic::PrintIcon(x/virtualXscale,y/virtualYscale,iconnum,cptr);
}

void	Graphic::PrintIcon(int sx,int sy,int iconnum,CharTableEntryPtr cptr)
{
	SWord	width,height;	//Width and height of the packed data
	UByte	*src;
	sx-=cptr->NextXShift;
	sy+=cptr->YShift;
	src = (UByte *) cptr->pixeldata;								//DAW 29Nov95
	
	width = cptr->Width;
	height = cptr->Height+sy;
	if (height>(PhysicalMinY+PhysicalHeight))
	{
		while(sy<PhysicalMinY)
		{
			src = Graphic::SkipRow(src,width);
			sy++;
		}
		while(sy<(PhysicalMinY+PhysicalHeight))
			src = Graphic::UnpackRow(src,width,sx,sy++,PhysicalMinX,PhysicalMinX+PhysicalWidth);
	}
	else
	{
		while(sy<PhysicalMinY)
		{
			src = Graphic::SkipRow(src,width);
			sy++;
		}
		while(sy<height)
			src = Graphic::UnpackRow(src,width,sx,sy++,PhysicalMinX,PhysicalMinX+PhysicalWidth);
	}
}

//------------------------------------------------------------------------------
//Procedure		RegisterPalette
//Author		Robert Slater
//Date			Tue 22 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::RegisterPalette(UByte* src,SLong length)
{
 	Graphic::RegisterPalette(src,length);
	Master()->HWRegisterPal(src,ULong(length));
}

//------------------------------------------------------------------------------
//Procedure		ReinitDisplay
//Author		Paul.   
//Date			Thu 3 Jul 1997
//
//Description	Resets logical ptr & palette after windows task switch
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigWindow::ReinitDisplay()
{
	Graphic::ReinitDisplay();
}
//------------------------------------------------------------------------------
//Procedure		DoTestHW3D
//Author		Paul.   
//Date			Tue 23 Sep 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MigDisplay::DoTestHW3D(MigWindow* w)
{
	TestHW3D(w);
}



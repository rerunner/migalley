//------------------------------------------------------------------------------
//Filename       shpbld.h
//System         
//Author         Paul.   
//Date           Mon 22 Jan 1996
//Description    
//------------------------------------------------------------------------------
#ifndef	SHPBLD_Included
#define	SHPBLD_Included

#define	DEFAULT_SHPBLD 0

	#include	"DOSDEFS.H"
	#include	"3DDEFS.H"
	#include	"MYANGLES.H"
	#include	"WORLDINC.H"
	#include	"ANIMDATA.H"

	#include	"3DINSTR.H"
	#include	"IMAGEMAP.H"

class	ShapeBuild
{
	UByteP	currentp;

	public:

		CON		ShapeBuild();
		DES	   ~ShapeBuild();

		UByteP	BuildCloud(ImageMapNumber, ImageMapNumber, SWord);
		void	DeleteCloud();

	private:

		void 	CreateDoPoint(UByteP& ,SWord ,SWord ,SWord , SWord );
		void 	CreateDoCreateBumpPoly(UByteP& ,ImageMapNumber );
		void 	CreateDoCopyBVert(UByteP& ,SWord , SWord , SWord );
		void 	CreateDoDrawBPoly(UByteP& );
		void 	CreateDoReturn(UByteP& );

		void 	CreateDoCreateIPoly(UByteP& ,ImageMapNumber );
		void 	CreateDoCopyIVert(UByteP& ,SWord , SWord , SWord );
		void 	CreateDoDrawIPoly(UByteP& );
	protected:
};

extern class ShapeBuild Shape_Build;

#endif

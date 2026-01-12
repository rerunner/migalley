//------------------------------------------------------------------------------
//Filename       planetyp.h
//System         
//Author         R. Hyde 
//Date           Thu 26 Oct 1995
//Description    Use I_planetype	to declare an instance
//				 Use name.I			to access the instance
// This module contains macros to initialise inherited classes "c" style.
// If the module is declared a second time, then this alternate construction 
// is made to allow instances to occur. 
// The re-caller has to undef PLANETYP_Included to avoid accidental double call
//------------------------------------------------------------------------------
#ifndef	PLANETYP_Included
#define	PLANETYP_Included
#include	"DOSDEFS.H"
//RERUN #include	"files.g"		//NON-GRAPHICS FILES				//RDH 16Jul96
#include	"FILES.H" // RERUN
#include	"TEXT.H"
#include	"MYANGLES.H"											//ARM 02Aug96
#include	"FLYINIT.H"											//ARM 02Aug96
#include	"WORLDINC.H"										//ARM 03Feb97

#ifdef	DEFAULT_PLANETYP
//
// If the module is declared a second time, then this alternate construction 
// is made to allow instances to occur.
//
	#define	emptybasestruct(subname,parentname)						\
	struct	II_##subname			{

	#define	basestruct(subname)							\
	struct	II_##subname			{

	#define	substruct(subname,parentname)				\
	struct	II_##subname			{					\
		II_##parentname	parentinst;

	#define	endstruct(subname)		};					\
	union	I_##subname									\
		{	II_##subname II;subname I;}

	#define	startinst(strucname,instname)	 			\
		II_##strucname	I_##instname	=	{

	#define	endinst(strucname,instname)	 	};			\
		strucname&	instname=*(strucname*)&I_##instname;

#else
//
// This definition defines substructures by simple inheritance
// That will not allow initialisation.
//
#include	"SHAPENUM.G"
	#define	emptybasestruct(subname,parentname) struct	subname : parentname	{

	#define	basestruct(subname) struct	subname					{

	#define	substruct(subname,parentname) struct	subname : parentname	{

	#define	endstruct(subname)		}

enum	ModelPatch	{PATCHWW1=1,PATCHWW2,PATCHWW3,PATCHMAX=0xffffffff};
enum	InstType	{lightinstrumtypes=0};
MINMAX(WeightEmpty,0,100000);
MINMAX(FuelLoad,0,2000000000);
MINMAX(FuelABoff,0,65535);
MINMAX(ThrustperStage,0,65535);
MINMAX(ABStages,0,65535);
MINMAX(MilMax,0,65535);
MINMAX(MaxTurnRate,0,65535);
MINMAX(MaxRollRate,0,65535);
MINMAX(MaxPitchRate,0,65535);
MINMAX(MaxAil,0,65535);
MINMAX(MaxAccel,0,262143);
MINMAX(RollRateAccel,0,65535);
MINMAX(LandingSpeed,0,65535);
MINMAX(MaxDiveRpm,0,65535);
MINMAX(MaxRpm,0,65535);
MINMAX(FreeRpm,0,65535);
MINMAX(MaxTorque,0,65535);
MINMAX(MinRpm,0,65535);
	MINMAX(BestClimbVel,0,65535);
	MINMAX(MaxDiveVel,0,65535);

	MINMAX(MaxVel,0,65535);
	MINMAX(MinVel,0,65535);
	MINMAX(CruiseVel,0,65535);
MINMAX(BestClimbAng,0,65536);	//CSB 28/09/98
MINMAX(BestClimbSpeed,0,65535);	//CSB 28/09/98
MINMAX(DiveVelFactor,0,1000);	//CSB 29/09/98

MINMAX(MaxVelSL_0km,0,65535);	//CSB 28/09/98
MINMAX(MaxVelSL_11km,0,65535);	//CSB 28/09/98
MINMAX(MaxVelSL_20km,0,65535);	//CSB 28/09/98
MINMAX(MaxVelDive_0km,0,65535);	//CSB 28/09/98
MINMAX(MaxVelDive_11km,0,65535);	//CSB 28/09/98
MINMAX(MaxVelDive_20km,0,65535);	//CSB 28/09/98

MINMAX(MovableFireZoneType,-32768,32767);
MINMAX(Damage,0,65535);
MINMAX(MoI,-32768,32767);
MINMAX(RotMoI,-32768,32767);
MINMAX(RotDirection,-32768,32767);
MINMAX(GyroEffectPerThou,-32768,32767);
MINMAX(CMAlfa,-32768,32767);
MINMAX(CMDelta,-32768,32767);
MINMAX(CMQ,-32768,32767);
MINMAX(CMO,-32768,32767);
MINMAX(CMGamma,-32768,32767);
MINMAX(CMPee,-32768,32767);
MINMAX(CM_acwb,-32768,32767);
MINMAX(XChord,-32768,32767);									//ARM 04Jun96
MINMAX(CoG_Neut,-1000,1000);
MINMAX(TailDistance,0,32767);
MINMAX(Wing_Incidence,0,65535);
MINMAX(Tail_Incidence,0,65535);
MINMAX(Ailmin,-32768,32767);
MINMAX(ABrake,0,65535);
MINMAX(TrimMin,-32768,32767);
MINMAX(TrimMax,-32768,32767);
MINMAX(FinSlope,-32768,32767);
MINMAX(ClFinNom,-32768,32767);
MINMAX(CLFinDenom,-32768,32767);
MINMAX(LiftFlapAngle,-32768,32767);
MINMAX(CDenom,-32768,32767);
MINMAX(Friction,-32768,32767);
MINMAX(Efff,-32768,32767);
MINMAX(MaxIRot,-32768,32767);
MINMAX(Abstage,0,32767);

enum	ACFlags	{hooka=1,bda=2,HUDINACTIVE=4,etca=8};
MINMAX(FightCeiling,	0,10000000);
MINMAX(Abs_Ceiling,		0,10000000);	//CSB 29/09/98
MINMAX(MuzzleVel,		0,10000);
#endif
#define	DEFAULT_PLANETYP 0

//TempCode JIM 23Jan96 basestruct(PlaneInit)	//stuff from planetype that is only used for initialisation

//TempCode JIM 23Jan96 isbasestruct(TextBlockKnown);
struct	DumbPlaneType;
struct	PlaneType;

basestruct(PlaneInit)
	WeightEmpty	weightempty;
	FuelLoad	maxintfuel;

	operator	DumbPlaneType* ()	{return(DumbPlaneType*)this;}
	operator	PlaneType* ()	{return(PlaneType*)this;}

endstruct(PlaneInit);

substruct(WheeledType,PlaneInit)
	ShapeNum	shadowshpno;
	int			planetext;	//	TextSnip0	planetext;
	ShapeNum	visible;
	FuelABoff	fuelaboff;
	
		MaxVel	 	maxvel;
		MinVel		minvel;
		CruiseVel	cruisevel;
		BestClimbVel	 	bestclimbvel;
	ANGLESInst	 	bestclimbpitch;
		MaxDiveVel	 	maxdivevel;
	MaxAccel	maxaccel;
endstruct(WheeledType);


substruct(DumbPlaneType,WheeledType)
	ShapeNum	cockpit1;										//PD 20Jun96
	ShapeNum	cockpit2;										//PD 20Jun96
	ShapeNum	maskpit1;										//PD 20Jun96
	ShapeNum	maskpit2;										//PD 20Jun96
	Float		cockscale;										//RJS 19Jan99
	ULong		phrasename;										//RJS 10Feb99
	AerobaticFactor	aerobaticfactor;
	ANGLESInst		deckangle;
	MaxRollRate	maxrollrate;
	MaxPitchRate	maxpitchrate;
	LandingSpeed	landingspeed;
	MaxDiveRpm	 	maxdiverpm;
	FreeRpm	 	freerpm;
	MaxRpm	 	maxrpm;
	MinRpm	 	minrpm;
	MovableFireZoneType	movablefirezonetype;
	Damage		damage;
	FightCeiling	fightceiling;
	Abs_Ceiling		abs_ceiling;
	Float	 	maxturnrateaoaSL;

	MuzzleVel		muzzlevel;
	FileNum			gunsnd0;
	FileNum			gunsnd1;
	FileNum			enginesnd;
	UByte			noGuns;										//RJS 29Mar99
	SWord			gundelay;									//RJS 12Apr99
	UWord			gunfrequency;								//RJS 31May99

       
endstruct(DumbPlaneType);

substruct(PlaneType,DumbPlaneType)

	InstType	insttype;
	ACFlags		acflags;

	BestClimbAng	bestclimbang[21];	//CSB 28/09/98
	BestClimbSpeed  bestclimbspeed[21];	//CSB 28/09/98
	DiveVelFactor	divevelfactor[9];	//CSB 29/09/98
	
	MaxVelSL_0km	maxvelsl_0km;	//CSB 28/09/98
	MaxVelSL_11km	maxvelsl_11km;	//CSB 28/09/98
	MaxVelSL_20km	maxvelsl_20km;	//CSB 28/09/98

	MaxVelDive_0km	maxveldive_0km;	//CSB 28/09/98
	MaxVelDive_11km	maxveldive_11km;	//CSB 28/09/98
	MaxVelDive_20km	maxveldive_20km;	//CSB 28/09/98

	ULong		version;

	void (*pSetupFunction) (AirStrucPtr const, ULong);		//ARM 28Jan97

endstruct(PlaneType);

typedef	PlaneType	*PTypePtr;

#undef	basestruct
#undef	emptybasestruct
#undef	substruct
#undef	endstruct

extern	PlaneInit*	Plane_Type_Translate[];

#endif


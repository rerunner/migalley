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

//Filename       flymodel.cpp
//System         
//Author         R. Hyde 
//Date           Mon 11 Sep 1995
//Description    
//------------------------------------------------------------------------------

#include "_MOVE.H" //RERUN

//#define		_NOFLIGHTMODEL_

#include	"DOSDEFS.H"
#define F_GRAFIX
#define F_COMMON
//RERUN #include	"files.g"
#include	"FILES.H"
#include	"WORLDINC.H"
#include	"PLANETYP.H"
#include	"MYANGLES.H"
#include	"ENUMBITS.M"
#include	"MYTIME.H"
//MATHABLE	(ANGLES)
#include	"FLYMODEL.H"
#include	"MYMATH.H"
#include	"VERTEX.H"
#include	"MATRIX.H"
#include	"TEXTREF.H"
#include	"VIEWSEL.H"
#include	"HARDPASM.H"
#include	"3DCOM.H"
#include	"LANDSCAP.H"
#include	"MISSMAN2.H"
#include	"SAVEGAME.H"
#include	"MILES.H"
#include	"WORLD.H"
#include	"TRANSITE.H"
#include	"COLLIDED.H"
#include	"SPEED.H"
#include "PERSONS2.H"
#include	"RANGES.H"
#include	"WINMOVE.H"
#include	"MODVEC.H"
#include	"MODEL.H"
#include	"SHAPES.H"
#include	"REPLAY.H"
#include "RCHATTER.H" // RERUN
//extern	ULong	GR_Quit3DNow;
extern DPlay _DPlay;

										//ARM 06Nov96
//-----------------------------------------------------------------------------------------
//1	Permanent notes on the flight model	
//
//
//	1	Units
//			 	vel,i_a_s			10 cm/s
//				vel_cm  			cm/s
//				vel_mm  			cm/s
//				WorldX,Y,Z			cm
//				forces				N
//				airdensity			g/m^3
//				sos					m/s
//				angles				all use full word
//				trig functions		signed word, 7fff = 1.0, 8000 = -1	
//
//	2	In the model, y is reversed:
//
//			cposy = -ControlledAC->World.Y; 		world y postion of ac
//			
//	3	Angles 					
//			    roll right wing down = +ve
//				hdg change to right  = +ve
//				pitch up			 = +ve
//				
//		When the ac is pointing above the velocity vector, then aoa = +ve
//		When the ac has a bigger heading than velocity vector, then slip = -ve
//
//
//


//-----------------------------------------------------------------------------------------
//2	Notes on relationship with rest of code
//
//
//	1	We calculate the position of a point on the aircraft. There are a number of
//		significant points:
//					pilot eye
//					centre around which aircraft turns
//					centre of drawn shape
//		At present we don't distinguish between two
//
//	2	Changing from auto to manual has changed, I will probably need to know
//		so that trim can be allowed
//
//	3	
// 
//	8	
//	 	if (ControlledAC2 == ControlledAC)
//				true for manual pilot
//
//------------------------------------------------------------------------------
//INSTANCE		Manual_Pilot
//Author		Jim Taylor
//Date			Tue 31 Oct 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ManualPilot	Manual_Pilot;

//------------------------------------------------------------------------------
//Procedure		ManualPilot	CONSTRUCTOR
//Author		R. Hyde 
//Date			Thu 26 Oct 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
	ManualPilot::ManualPilot()
{
	View_Point=NULL;
// 	machdata=Mach_Data;
//	airdensitydata=AirDensity_Data;
//	airdensityratio=AirDensity_Ratio;
//	shokdata=Shok_Data;
	ControlledAC2=NULL;

}

//------------------------------------------------------------------------------
//Procedure		Associate
//Author		Jim Taylor
//Date			Tue 31 Oct 1995
//
//Description	Associates specific 3d item to manual AutoMoveInfo.movecode.
//				May also be used to flag auto/manual switch,
//				so put any trim init code here!
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	ManualPilot::Associate	(
			AirStruc*	manualas,				//item to move
			CONTROLMODE		inmanual)		//TRUE	//flag for man/auto/auto accel
{
	if  (inmanual!=controlmode || ControlledAC2!=manualas)
	{
		controlmode=inmanual?MANUAL:AUTO;
		ControlledAC2=manualas;
		InitManualPilot(ControlledAC2);
	}
}
//JIM 15Oct96

void	ManualPilot::AutoToggle(CONTROLMODE dead)
{
	if (dead==PILOTDEAD)
		controlmode=PILOTDEAD;
	else
		if (controlmode!=PILOTDEAD)								//JIM 11Oct96
		{														//JIM 11Oct96
			ControlledAC2->FormationInfo.information=FALSE;
			ControlledAC2->FormationInfo.slowdownleader=FALSE;
			if (dead==TOGGLE)									//JIM 11Oct96
			{
				dead=controlmode==AUTO?MANUAL:AUTO;
				Associate(ControlledAC2,dead);
			}
			else AutoToggleAircraft(ControlledAC2,dead);					//rdh 6/4/98
		}														//JIM 11Oct96
}

//------------------------------------------------------------------------------
//Author		R. Hyde 
//Date		Wed 30 Aug 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	ManualPilot::MainManualPilot()
{
//TempCode PD 3Nov97 
//	return;
//TempCode PD 3Nov97 

//Add timing thing here	//CSB 02/03/99
	
	if (ControlledAC2)
	{
		WorldStuff &world = mobileitem::GetWorld();
		UWord	oldsector=	world.GetSector(ControlledAC2);

///////////////////////////////////////////
//tmp fix should not be running acm on player ac in final version//RDH 20Aug96
		if (controlmode == MANUAL)								//RDH 20Aug96
		{
			if (Persons2::PlayerGhostAC->FormationInfo.slowdownleader)			  //JIM 22/03/99
			{														  //JIM 22/03/99
				Persons2::PlayerGhostAC->FormationInfo.slowdownleader=FALSE;		  //JIM 22/03/99
				if (ControlledAC2->fly.thrustpercent>=90 && ControlledAC2->vel>1200)
					if (!ControlledAC2->TimeLeft())
					{
						AirStrucPtr found=NULL;
						for (AirStrucPtr l=Persons2::PlayerGhostAC;l;l=l->fly.nextflight)
						{
							for (AirStrucPtr w=l;w;w=w->Follower())
								if (!w->FormationInfo.information)
									if (w!=Persons2::PlayerGhostAC)
										found=w;
						}
						if (found)
							_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_TOOFAST, MSG_FORMATIONFLYING,found, NULL, ControlledAC2));
						ControlledAC2->SetManoeuvreTime();
					}
			}
			GetKeyCommon (ControlledAC2);				//RDH 15Dec95
			if (!_Replay.Playback)
			{
				_DPlay.AddDeltas(Persons2::PlayerSeenAC,FALSE); 				
#ifndef	_NOFLIGHTMODEL_
 				FlightModelFixedTime(Persons2::PlayerSeenAC);
#endif
 				Persons2::PlayerSeenAC->SetManualFlightParams ();

				_DPlay.AddDeltas(Persons2::PlayerSeenAC,TRUE);				
			}


			SHAPE.RipStores(ControlledAC2);						//RJS 03Jun98

//			ControlledAC2->CheckManualCourse();					//JIM 03Dec96
//being done by macca.
		}
		else if (controlmode==AUTOACCEL_WAYPT || controlmode==AUTOACCEL_FWD ||
				controlmode==AUTOACCEL_MSG || controlmode==AUTOACCEL_MISS ||
				controlmode==AUTOACCEL_HOME)
		{
			_DPlay.AddDeltas(Persons2::PlayerSeenAC,FALSE); 				//AMM 10Mar99
			AutoMoveCodeTypeSelect bupMoveCode=ControlledAC2->AutoMoveInfo.movecode;
			ControlledAC2->AutoMoveInfo.movecode=AUTO_ACCELERATE;
			mobileitem::MoveItem((MobileItemPtr)ControlledAC2,world);
			ControlledAC2->AutoMoveInfo.movecode=bupMoveCode;
			_DPlay.AddDeltas(Persons2::PlayerSeenAC,TRUE);				//AMM 10Mar99
		}
		else
		{
//			if (controlmode == MANUAL)								//RDH 20Aug96
//				GetStickKeys(ControlledAC2);
			if (!_Replay.Playback)
			{													
// this moves players ac if player is not in control i.e autospiral, autores etc
																
//				WorldStuff &world = mobileitem::GetWorld();		
																
				_DPlay.AddDeltas(ControlledAC2,FALSE);			
				mobileitem::MoveItem((MobileItemPtr)(ControlledAC2),world);
				_DPlay.AddDeltas(ControlledAC2,TRUE);
			}
			else												//AMM 31Aug98
			{													//AMM 31Aug98
// shouldnt ever get in here?
				INT3;

 			}													//AMM 31Aug98


//			View_Object->right_aoa = ControlledAC2->fly.aoa;
//			View_Object->left_aoa = ControlledAC2->fly.aoa;
		}

///////////////////////////////////////////

		if (ControlledAC2->uniqueID.bitfieldUid.changed)
		{
			UWord	newsector=	world.GetSector(ControlledAC2);
			if (newsector!=oldsector)
			{
				world.RemoveFromSector(ControlledAC2,oldsector);
				world.AddToWorld(ControlledAC2);
			}
			ControlledAC2->uniqueID.bitfieldUid.changed = FALSE;						//RDH 26Jun96
		}
 	}
//Add timing thing here	//CSB 02/03/99

}

//------------------------------------------------------------------------------
//Procedure		DeathSequenceOverride
//Author		Robert Slater
//Date			Wed 15 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	ManualPilot::DeathSequenceOverride(ItemPtr	itm, int	newmovecode,bool commscalled)//AMM 26Jan99
{

	Bool	wasset = FALSE;										//DAW 03Sep98
	if (itm->MigStatus.LaunchFlags.size >= AIRSTRUCSIZE)
	{
		AirStrucPtr		acptr = (AirStrucPtr) itm;
		MinAnimData*	mad = (MinAnimData*) acptr->Anim;
		{
			if (	(acptr->AutoMoveInfo.movecode != newmovecode)
				&&	(acptr->AutoMoveInfo.movecode != AUTO_NOPPILOT)					//RJS 27Aug98
				&&	(acptr->AutoMoveInfo.movecode != AUTO_HITWATER)					//RJS 27Aug98
				&&	(acptr->AutoMoveInfo.movecode != AUTO_CRASHSKID)					//RJS 27Aug98
				&&	(acptr->AutoMoveInfo.movecode != AUTO_CRASHROLL)					//RJS 27Aug98
				&&	(acptr->AutoMoveInfo.movecode != AUTO_SPIRAL2GROUND)	)		//DAW 03Sep98
			{

				if (itm == Persons2::PlayerSeenAC)						//RJS 30Oct98
				{
					controlmode=PILOTDEAD;
//Dead					acptr->MigStatus.LaunchFlags.deadtime = 1000;//Just to make sure...

					if (_Replay.Record)									//RJS 30Oct98
					{
						UByte	effect = RDEATH_NULL;

						switch (newmovecode)
						{
							case AUTO_HITWATER:
							case AUTO_CRASHSKID:
								effect = RDEATH_SKID;
								break;
							case AUTO_CRASHROLL:
								effect = RDEATH_ROLL;
								break;
							case AUTO_SPIRAL2GROUND:
							case AUTO_DEATHSEQUENCE:
								effect = RDEATH_DIVE;
								break;
						}

		 				_DPlay.NewSpecial(PIDC_DEATHMOVEEFFECT,effect,0,0);
					}
				}

				acptr->AutoMoveInfo.movecode = (AutoMoveCodeTypeSelect) newmovecode;
				wasset = TRUE;										//DAW 03Sep98

				if (!acptr->MigStatus.LaunchFlags.deadtime)						//RJS 17May99
				{
					ItemPtr	lasthitter = acptr;
					if (acptr->lasthitter.bitfieldUid.count)
					{
						lasthitter = (ItemPtr)Persons2::ConvertPtrUID((UniqueID)acptr->lasthitter.bitfieldUid.count);
						if (!lasthitter)  		//checks to see if it's an airstruc within ScoreSimpleItemKilled
							lasthitter = acptr;
					}

					if (_DPlay.Implemented)						//RJS 03Jun99
					{											//RJS 03Jun99
						if (_DPlay.GameType<COMMSQUICKMISSION)	//RJS 03Jun99
							_DPlay.SetScore(acptr,lasthitter);	//RJS 03Jun99
						else
						{
							if (_DPlay.GameType>TEAMPLAY)
								_DPlay.NewScoreClaim(lasthitter,acptr->uniqueID.bitfieldUid.count);
						}
					}
					else
					{
						if (lasthitter->MigStatus.LaunchFlags.size == AIRSTRUCSIZE)	//RJS 03Jun99
							itm->ScoreSimpleItemKilled(true,(AirStrucPtr)lasthitter);//RJS 25May99
					}

					acptr->BreakForm();								//RJS 17May99
				}
			}
			else
			{
				//It's ok to say you can't move anymore,
				//but not ok to say you can...
				if (newmovecode == AUTO_NOPPILOT)
					acptr->AutoMoveInfo.movecode = (AutoMoveCodeTypeSelect) newmovecode;
			}

			//Make sure it is dead!								
			ShapeDescPtr	sdptr = SHAPESTUFF.GetShapePtr(acptr->shape);	//RJS 17May99
			acptr->MigStatus.LaunchFlags.deadtime = sdptr->DeadTime;						//RJS 17May99
			acptr->MigStatus.LaunchFlags.deadscale = sdptr->DeadScale;						//RJS 17May99
	 		acptr->MigStatus.LaunchFlags.deaded = TRUE;									//RJS 17May99
			if (acptr == Persons2::PlayerSeenAC)				//RJS 09Jul99
			{
				Persons2::PlayerGhostAC->MigStatus.LaunchFlags.deadtime = sdptr->DeadTime;
				Persons2::PlayerGhostAC->MigStatus.LaunchFlags.deadscale = sdptr->DeadScale;
				Persons2::PlayerGhostAC->MigStatus.LaunchFlags.deaded = TRUE;
				Persons2::PlayerGhostAC->AutoMoveInfo.movecode=acptr->AutoMoveInfo.movecode;
			}
			
			mad->itemstate = DEAD;								//RJS 23May99

			AircraftAnimData* adptr = (AircraftAnimData*)acptr->Anim;//DAW 09Jun99
			adptr->shooting = 0;								//DAW 09Jun99
		}
	}
	return(wasset);
}
//------------------------------------------------------------------------------
//Procedure		FlightModelFixedTime
//Author		R. Hyde 
//Date			Sun 7 Apr 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	ManualPilot::FlightModelFixedTime(AirStrucPtr const ControlledAC2)
{
	UWord	loopcount;
	ANGLES	test;

//Old_Code RDH 21Oct96 	groundlevel = Land_Scape.GetGroundLevel(ControlledAC2);
	groundlevel = _Collide.LowestSafeAlt(ControlledAC2,
											ControlledAC2->pitch,
											ControlledAC2->roll);//RDH 21Oct96

//to allow for small changes in ground level add 2 cm			//RDH 29Nov96
	groundlevel +=2;											//RDH 29Nov96

		ControlledAC2->fly.pModel->Base (ControlledAC2);							//ARM 07Jan97
//need a test here for dead aircraft +++


}

//------------------------------------------------------------------------------
//Author		R. Hyde 
//Date		Wed 30 Aug 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	ManualPilot::ACMManualPilot(AirStrucPtr const ControlledAC2)
{
	if (ControlledAC2)
	{
		keyboardcontrol = FALSE;

// All of the next bit is done in the flight model
		//Calculating the fuel used will now be done within the flight model	CSB
		FlightModelFixedTime(ControlledAC2);
		ControlledAC2->SetManualFlightParams ();				//DAW 04Jul96
 	}
}

//------------------------------------------------------------------------------
//Author		R. Hyde 
//Date		Wed 30 Aug 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	ManualPilot::SubManualPilot(AirStrucPtr const ControlledAC)
{

//TempCode ARM 27Aug97 		ANGLES a,b;
//TempCode ARM 27Aug97 		ControlledAC->fly.pModel->HaveWeLanded (ControlledAC, a,b);

}
//------------------------------------------------------------------------------
//Procedure		InitManualPilot
//Author		Paul.   
//Date			Thu 2 Nov 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void ManualPilot::InitManualPilot(AirStrucPtr const ControlledAC)

{

	InitManualPilotSub(ControlledAC);


	//Do this whenever you switch auto/man/auto

	ControlledAC->ai.PilotSkills.ManStep = PHASE0;							//RDH 10Sep96
	ControlledAC->ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;				//JIM 05Aug96
	ControlledAC->manoeuvretime = 0;							//JIM 05Aug96
//	ControlledAC->ai.JinkTime = 0;							//JIM 05Aug96

}


//------------------------------------------------------------------------------
//Procedure		InitManualPilot
//Author		Paul.   
//Date			Thu 2 Nov 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void ManualPilot::InitManualPilotSub(AirStrucPtr const ControlledAC)

{

		InitSimpleAc(ControlledAC);
///		Save_Data.keysensitivity = 2;											//RDH 12Mar96
		trim =0;													//RDH 15Nov96
		lastmsgtime = 0;

//this does a big initialise									//rdh
		if (ControlledAC2 == ControlledAC)							//ARM 01May97
		{// Player's Aircraft
			ControlledAC->fly.pModel->Initialise (ControlledAC,TRUE);			//ARM 01May97
		}else														//ARM 01May97
		{// ACM Aircraft
			ControlledAC->fly.pModel->Initialise (ControlledAC,FALSE);			//ARM 01May97
		}
}

void ManualPilot::InitSimpleAc(AirStrucPtr const ControlledAC)
{
//	ControlledAC->fly.rpm_lastcycle = ControlledAC->fly.thrustpercent;
//	ControlledAC->fly.rpmtrimtime = 0;

//the following need to be set on moving to manual
	if (ControlledAC->vel > 0)
		ControlledAC->fly.cpitch = ControlledAC->pitch;			//RDH 10Apr96
	ControlledAC->fly.i_a_s = ControlledAC->vel;				//RDH 19Oct96
	ControlledAC->fly.vel_cms = 	ControlledAC->vel * 10;			//RDH 19Oct96
//	ControlledAC->fly.turnrate = ANGLES_0Deg;					//RDH 19Oct96
	ControlledAC->fly.redeffect = 0;							//RJS 13Nov96
	ControlledAC->ai.moraleinc = 0;
	ControlledAC->ai.deltapitchsum = 0;		//rdh
}
void ManualPilot::AutoToggleAircraft(AirStrucPtr const ControlledAC, CONTROLMODE mode)
{
	if (mode!=controlmode)
	{
		if (mode==MANUAL)	ControlledAC->fly.pModel->Controlled = TRUE;
		else				ControlledAC->fly.pModel->Controlled = FALSE;
		controlmode=mode;
		InitFlightModelPos(ControlledAC);
		ControlledAC->ai.PilotSkills.ManStep = PHASE0;							//RDH 10Sep96
		ControlledAC->ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;				//JIM 05Aug96
		ControlledAC->manoeuvretime = 0;							//JIM 05Aug96
		ControlledAC->FormationInfo.information=FALSE;							  //JIM 16/04/99
//		ControlledAC->ai.JinkTime = 0;							//JIM 05Aug96
	}
}

void ManualPilot::InitFlightModelPos(AirStrucPtr const ControlledAC)
{
	InitSimpleAc(ControlledAC);
//init aoa for first frame where it may be used before it is set

	ControlledAC->fly.elevator = 0;								//AMM 19Jan00
	ControlledAC->fly.aileron  = 0;								//AMM 19Jan00
	ControlledAC->fly.rudder   = 0;								//AMM 19Jan00
	ControlledAC->fly.pModel->Elevator = 0;						//AMM 19Jan00
	ControlledAC->fly.pModel->Aileron  = 0;						//AMM 19Jan00
	ControlledAC->fly.pModel->Rudder   = 0;						//AMM 19Jan00

	PMAINPLANE pMainPlane = ControlledAC->fly.pModel->MainPlaneList;
	while(pMainPlane)
	{
		pMainPlane->aoa[0][0] = 0;
		pMainPlane->aoa[0][1] = 0;
		pMainPlane->aoa[1][0] = 0;
		pMainPlane->aoa[1][1] = 0;
		pMainPlane = pMainPlane->List.NextItem();
	}

	ControlledAC->fly.pModel->InitialiseEngines(ControlledAC);
	ControlledAC->fly.pModel->UpdateModel (ControlledAC);
}

UByte	DUMMY;
UByte&	item::AnimByte(Animfieldref)
{
	return(DUMMY=0);
}


//------------------------------------------------------------------------------
//Procedure		PrintStatistics
//Author		R. Hyde 
//Date			Thu 16 Nov 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	ManualPilot::PrintStatistics(MigWindow* Whole_Screen)
{

//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(6500,1000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->hdg,5,OFORM_LZ));
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(13000,1000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->pitch,5,OFORM_LZ));	  


//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(0000,2000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv("VEL"));
//TempCode RDH 10Apr96 
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(6500,2000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->fly.chdg,5,OFORM_LZ));
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(13000,2000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->fly.cpitch,5,OFORM_LZ));
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(19500,2000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->fly.croll,5,OFORM_LZ));
//TempCode RDH 10Apr96 
//TempCode RDH 10Apr96 
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(0000,3000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv("VAS"));
//TempCode RDH 10Apr96 
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(6500,3000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->vel,5,OFORM_LZ));
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(13000,3000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->fly.aoa,5,OFORM_LZ));
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(19500,3000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->fly.slip,5,OFORM_SIGN));
//TempCode RDH 10Apr96 
//TempCode RDH 10Apr96 
//TempCode RDH 10Apr96 
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(0000,4000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv("Inputs"));
//TempCode RDH 10Apr96 
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(6500,4000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->fly.aileron,5,OFORM_LZ));
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(13000,4000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->fly.elevator,5,OFORM_LZ));
//TempCode RDH 10Apr96 	_Text.Print(Whole_Screen,ScreenXY(19500,4000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96 		_Text.Conv(ControlledAC2->fly.rudder,5,OFORM_LZ));

 
//TempCode RDH 10Apr96  	_Text.Print(Whole_Screen,ScreenXY(0000,5000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96  		_Text.Conv("Pos"));
//TempCode RDH 10Apr96  
//TempCode RDH 10Apr96  	_Text.Print(Whole_Screen,ScreenXY(6500,5000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96  		_Text.Conv(ControlledAC2->World.X,5,OFORM_LZ));
//TempCode RDH 10Apr96  	_Text.Print(Whole_Screen,ScreenXY(13000,5000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96  		_Text.Conv(ControlledAC2->World.Y,5,OFORM_LZ));
//TempCode RDH 10Apr96  	_Text.Print(Whole_Screen,ScreenXY(19500,5000),ScreenWH(0,0),PF_VAlignCenter,
//TempCode RDH 10Apr96  		_Text.Conv(ControlledAC2->World.Z,5,OFORM_LZ));
 
//TempCode JIM 19Jan98  	_Text.Print(Whole_Screen,ScreenXY(6000,5000),ScreenWH(7000,1200),PF_VAlignCenter,
//TempCode JIM 19Jan98  		_Text.Conv(ControlledAC2->fly.acroll,5,OFORM_LZ));
//TempCode JIM 19Jan98  	_Text.Print(Whole_Screen,ScreenXY(13000,5000),ScreenWH(6000,1200),PF_VAlignCenter,
//TempCode JIM 19Jan98  		_Text.Conv(ControlledAC2->fly.rollrate,5,OFORM_LZ));

//	_Text.Print(ScreenXY(0000,14000),ScreenWH(0,0),PF_VAlignCenter,
//		_Text.Conv("TLD"));
//
//	_Text.Print(ScreenXY(6500,14000),ScreenWH(0,0),PF_VAlignCenter,
//		_Text.Conv(thrust,5,OFORM_LZ));
//	_Text.Print(ScreenXY(13000,14000),ScreenWH(0,0),PF_VAlignCenter,
//		_Text.Conv(lift,5,OFORM_LZ));
//	_Text.Print(ScreenXY(19500,14000),ScreenWH(0,0),PF_VAlignCenter,
//		_Text.Conv(totaldrag,5,OFORM_LZ));
//





}

//
// ACM - moved and reclassed...
//
//


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

//------------------------------------------------------------------------------
//Filename       acmai.cpp
//System         
//Author         Andrew McRae
//Date           Mon 17 Mar 1997
//Description    Flight Model ACM
//------------------------------------------------------------------------------
//
//RANGES_Included
//

#include "_MODE.H" //RERUN

#define F_GRAFIX											//DAW 05Aug96
#define F_COMMON
#define F_BATTLE

#include <stdio.h>
#include <string.h>


#include "COLLIDED.H"
#include "DOSDEFS.H"
#include "WORLDINC.H"

#include "RANGES.H"
#include "MODVEC.H"
#include "MODEL.H"



#include "FLYMODEL.H"
#include "MYMATH.H"
#include "ENUMBITS.M"
#include "MYTIME.H"
#include "RANGES.H"
#include "MISSMAN2.H"
#include "MOVEMENT.H"
#include "TRANSITE.H"


#include "AI.H"
#include "MILES.H"
#include "LANDSCAP.H"
#include "VIEWSEL.H"
#include "3DCOM.H"
#include "SPEED.H"
#include "PERSONS2.H"

#include "MODEL.H"
#include "MONOTXT.H"

#include "MATRIX.H"
#include "WINMOVE.H"

//#define PRINT_AI_DATA	//CSB 16/03/99
//MATHABLE	(ANGLES)

// ROD don't use STALL_ANGLE
ANGLES const	STALL_ANGLE = ANGLES_15Deg;

ANGLES const	STALL_ANGLE_MINUSONE = STALL_ANGLE - ANGLES_1Deg;
ANGLES const	STALL_ANGLE_MINUSTWO = STALL_ANGLE - ANGLES_2Deg;
ANGLES const	STALL_ANGLE_MINUSTHREE = STALL_ANGLE - ANGLES_3Deg;
SLong	const	ELEVATOR_DELTA = 40;
SLong	const	ELEVATOR_DELTA_DOWN = ELEVATOR_DELTA * 2;
SLong	const	RUDDER_DELTA = 1000;
//SLong	const	MAXAUTOTHRUST = 80;	

ANGLES const	DSLIPFORMAXRUDDER = ANGLES_10Deg;
ANGLES const	DPITCHFORMAXELEVATOR = ANGLES_60Deg;
ANGLES const	DAOAFORMAXELEVATOR = ANGLES_15Deg;
//sensitivity for Diferential control
ANGLES const	DIFFPITCHFORMAXELEVATOR = ANGLES_10Deg;
ANGLES const	DHDGFORMAXAILERON = ANGLES_22Deg;
SLong const MAXELEVATORFORTIGHTTURN = -25000;
SLong const SLIGHTPULLELEVATOR = -4000;
SLong const MEDIUMPULLELEVATOR = -12000;
SLong const HEAVYPULLELEVATOR = -20352;							//RDH 21Aug96




extern RequiredBankDataElement 	CombatReqBankData[];
// =
//			{
//				{ANGLES_FifthDeg, ANGLES_0Deg},
//				{ANGLES_HalfDeg, ANGLES_3Deg},
//				{ANGLES_1Deg, ANGLES_4Deg},
//				{ANGLES_2Deg, ANGLES_5Deg},
//				{ANGLES_3Deg, ANGLES_10Deg},
//				{ANGLES_4Deg, ANGLES_15Deg},
//				{ANGLES_5Deg, ANGLES_22Deg},
//				{ANGLES_6Deg, ANGLES_30Deg},
//				{ANGLES_10Deg, ANGLES_45Deg},
////DeadCode RDH 17Jul96 				{ANGLES_22Deg, ANGLES_60Deg},
//				{ANGLES_30Deg, MAXCOMBATROLL},
//				{ANGLES_180BareDeg, MAXCOMBATROLL}
//			};
void	AirStruc::MoveToThrust (SWord thrust)
{
}

  //------------------------------------------------------------------------------
  //Procedure		MoveToHdg
  //Author		R. Hyde 
  //Date			Wed 13 Dec 1995
  //
  //Description	based on difference between desired and actual hdg, fix aileron
  //				to give a roll, morale and skill will fix maxroll				
  //
  //Inputs		
  //
  //Returns	
  //
  //------------------------------------------------------------------------------
 void	AirStruc::MoveToHdg (ANGLES desiredhdg, ANGLES desiredpitch)
 {
  	ACMMODEL& ACM = *fly.pAcmModel;

	SWord	reqdeltahdg,reqdeltahdg_sign;
	UWord	reqdeltahdg_pos;
	SWord	reqdeltapitch,reqdeltapitch_sign;
	UWord	reqdeltapitch_pos;

	reqdeltahdg = (SWord)hdg - (SWord)desiredhdg;
	Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);

	reqdeltapitch = (SWord)desiredpitch - (SWord)fly.cpitch;
	Math_Lib.Pos (reqdeltapitch,reqdeltapitch_pos,reqdeltapitch_sign);

	if((reqdeltahdg > ANGLES_10Deg) || (reqdeltahdg < -ANGLES_10Deg))
		TurnFightTargetNotOnNose(reqdeltahdg, reqdeltapitch, reqdeltapitch_sign, reqdeltapitch_pos, ANGLES_0Deg);
	else
	{
		if( ((SWord)roll > ANGLES_45Deg) || ((SWord)roll < -ANGLES_45Deg) )
			;
		else
			ACM.Pitch(desiredpitch);
		SWord reqroll = FindRequiredRoll (reqdeltahdg,CombatReqBankData);
		ACM.Roll(reqroll);
	}

	ACM.SetRudder(0);

  
  }
//------------------------------------------------------------------------------
//Procedure		MoveToDesiredHdg
//Author		R. Hyde 
//Date			Mon 18 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::MoveToDesiredHdg (ANGLES desiredhdg,ANGLES desiredpitch,ANGLES target)

{
	Bool	retval;

	MoveToHdg (desiredhdg, desiredpitch);
	retval = ChkAtDesiredHdg (desiredhdg,target);
	return (retval);
}


//------------------------------------------------------------------------------
//Procedure		GetRequiredRoll
//Author		R. Hyde 
//Date			Fri 13 Mar 1998
//
//Description	Find the roll to put trgitem on lift vector of srcitem
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ANGLES GetRequiredRoll(rotitem& srcItem,COORDS3D trg, Float& pitch, Float& hdg)

{
	//const Float _PI=22/7;
 	FPMATRIX fpmMatrix;
	_matrix.generate(-srcItem.hdg,-srcItem.pitch,-srcItem.roll,&fpmMatrix);
	IFShare	x,y,z;
	x.i=trg.X-srcItem.World.X;
	y.i=trg.Y-srcItem.World.Y;
	z.i=trg.Z-srcItem.World.Z;
	_matrix.transform(&fpmMatrix,x,y,z);
	if (z.f<0.) z.f=-z.f;
	x.f/=z.f;
	hdg = x.f;
	y.f/=z.f;
#if defined (__MSVC__)
	_asm {
		fld x.f;
		fld y.f;
		fpatan;
		fstp x.f;
	}
#else
	x.f = atan2(y.f, x.f);
#endif
	//x.f is required angle in radians
	x.f=Float(32768)*x.f/FPIE;
	pitch = y.f;
	return (Angles)(UWord)x.f;
}

//------------------------------------------------------------------------------
//Procedure		LookAround
//Author		R. Hyde 
//Date			Mon 19 Aug 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::LookAround ()
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;

			ACM.Thrust (ThrustfromFF());
			SetManoeuvreTime (500);
 			ai.desiredroll = -(ANGLES)ANGLES_60Deg +	//CSB 18/03/99	
					(Angles)(ANGLES_120Deg * Math_Lib.rnd(2));	//CSB 18/03/99	


			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			ACM.Roll (ai.desiredroll);
			ACM.Pitch (BestClimbAngle());
			ACM.MinHeight (FT_2000);

			if  (!ManoeuvreTimeLeft())
			{
 		    	ai.PilotSkills.ManStep = PHASE0;
 				if (formpos == 0)
 				{
 					if (AllButLdrNotOnCombat ())
 					{
 						ai.PilotSkills.manoeuvre=MANOEUVRE_TOPCOVER;
 					}else
 					{
 						ai.PilotSkills.manoeuvre=MANOEUVRE_LOOKROUND;
 					}
 
 				}else
 				{
 					ai.PilotSkills.manoeuvre=MANOEUVRE_TOPCOVER;
 				}

			}

		}
	}
}
//------------------------------------------------------------------------------
//Procedure		WeldedWingMan
//Author		R. Hyde 
//Date			Mon 2 Mar 1998
//
//Description	Formation flag not set so a/c can spot
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::WeldedWingMan()
{
	
//	stick to the leaders tail, range about 1000ft and weave
//	However for low morale bring wingman closer
//	For very low morale it should appear as if he is trying to stay in formation
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
			ai.PilotSkills.ManStep = PHASE1;
			ACM.maxdeltaforclosework = 0.0;
			SetManoeuvreTime (3000);


		}
		case PHASE1:
		{
			if (WeldedWingSixOK())
			{
				if (		(fly.leadflight)					
						&&	 (fly.leadflight->ai.PilotSkills.manoeuvre == MANOEUVRE_TOPCOVER)
					)//force manoeuvre to allow disengage
						ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
				
				if (!leader)
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
				else
				{
					double	desiredturn;

					COORDS3D	target;
					target = leader->World; 									//JIM 02Aug96
					target.Y +=	FT_500;
					InterceptandRange(&target);

					SWord	reqdeltahdg,reqdeltahdg_sign;
					UWord	reqdeltahdg_pos;
					reqdeltahdg = 	hdg - HdgIntercept;
					Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);

					SWord	reqdeltapitch,reqdeltapitch_sign;
					UWord	reqdeltapitch_pos;
					reqdeltapitch = (SWord)PitchIntercept - (SWord)fly.cpitch;
					Math_Lib.Pos (reqdeltapitch,reqdeltapitch_pos,reqdeltapitch_sign);

									
					if  (		(		(reqdeltahdg_pos < ANGLES_40Deg)
						&&	(ACM.maxdeltaforclosework == 0.0)
						)
						||
							(		(reqdeltahdg_pos < ANGLES_45Deg)
								&&	(ACM.maxdeltaforclosework != 0.0)
							)		
						)
					{
				 		SWord reqroll = RollforTurn(reqdeltapitch, reqdeltahdg, desiredturn, target);
						ACM.Roll ((Angles)reqroll);
		
					}else
					{
						ACM.maxdeltaforclosework = 0.0;
						TurnFightTargetNotOnNose(reqdeltahdg, reqdeltapitch, reqdeltapitch_sign, reqdeltapitch_pos, ANGLES_0Deg);
						ACM.SetRudder(0);
					}
					InterceptandRange (&leader->World);
					UWord DesRange = 256 * ai.morale;	// up to 650 metres
					if(DesRange < WEAPONSRANGE) DesRange = WEAPONSRANGE;

					if (Range < DesRange)
						ACM.Thrust (0);
					else
						ACM.Thrust (ThrustfromFF());
				
				}
			}
		}
	}


}

//------------------------------------------------------------------------------
//Procedure		BailOut
//Author		R. Hyde 
//Date			Tue 3 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::BailOut()
{
//pilot ejects or bails out depending on ac type
//ac in slow turning dive

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			ACM.Thrust(0);	 
			ACM.Roll(0);
			ACM.Pitch(ANGLES_0Deg);
			SetManoeuvreTime(100);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			ACM.Roll(0);
			ACM.Pitch(ANGLES_0Deg);
			if(!TimeLeft())
			{
				Trans_Obj.LaunchCanopyEject(this,*mobileitem::currworld);
				AirStrucPtr	unfriendly = *ai.unfriendly;
				unfriendly->ai.unfriendly = NULL;
				ai.PilotSkills.ManStep = PHASE2;
				SetManoeuvreTime(100);
			}
			break;
		}
		case PHASE2:
		{
			ACM.Roll(0);
			ACM.Pitch(ANGLES_0Deg);
			if(!TimeLeft())
			{
				Trans_Obj.LaunchParachute(this,*mobileitem::currworld);
				AutoMoveInfo.movecode = AUTO_DEATHSEQUENCE;
			}
		}
	}


}

//------------------------------------------------------------------------------
//Procedure		TopCover
//Author		R. Hyde 
//Date			Mon 19 Aug 1996
//
//Description	The followers are supposed to go near to home line or
//				not depending on skill
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::TopCover ()

{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	UByte	FF = FlyingFactor();

   if (formpos == 0)
   {//leader
 	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{
			MOD.Rudder = 0;

			ACM.Thrust (ThrustfromFF());

 			SetManoeuvreTime (500);			//rdh 25/6/99
 			ai.desiredroll = -(ANGLES)ANGLES_60Deg +
  						(Angles)(ANGLES_120Deg * Math_Lib.rnd(2)); 
 			ai.PilotSkills.ManStep = PHASE1;
 		}
 
 
 		case PHASE1:
 		{
			ACM.Roll (ai.desiredroll);
			ACM.Pitch (BestClimbAngle());

 			if (!ManoeuvreTimeLeft())
 			{
 				SetManoeuvreTime (1000);
 				ai.PilotSkills.ManStep = PHASE2;
  			}
 
 			break;
 		}
 		case PHASE2:
 		{
			ACM.Roll (ai.desiredroll);
			ACM.Pitch (ANGLES_0Deg);
 
 			if (!ManoeuvreTimeLeft())
 			{
 				ai.PilotSkills.ManStep = PHASE3;
  			}
 
 			break;
 		}
 		case PHASE3:
 		{
			ACM.Roll (ai.desiredroll);
			ACM.Pitch (ANGLES_0Deg);
 
 			if (AllOnTopCover ())
 			{
 				Art_Int.AllBreakOff(this);						//JIM 11Oct96
 
//##				if (ai.eventlog->stage==EventLog::TAKEOFF)		//JIM 06Dec96
//##					if (formpos!=0 || follower || fly.nextflight)
//##					 	Persons_2.AddMessage(UserMsg::GOHOME,TEXT_SQUADRONREGROUPED,NULL);
 			}
 			else
 			{
 		    	ai.PilotSkills.ManStep = PHASE1;
 				SetManoeuvreTime (1000);
 			}
 
 			break;
 		}
   	}
   }else
   {
	   	AirStruc* leader = FindGroupLeader();

	   if (leader == Persons2::PlayerGhostAC)
	   {
			Art_Int.BreakOff(this);
	   }else
	   {
 			switch (ai.PilotSkills.ManStep)
 			{
 				case PHASE0:
 				{
					MOD.Rudder = 0;

					ACM.Thrust (ThrustfromFF());
 					SetManoeuvreTime (500);
 					ai.desiredroll = -(ANGLES)ANGLES_60Deg +
  								(Angles)(ANGLES_120Deg * Math_Lib.rnd(2)); 
 					ai.PilotSkills.ManStep = PHASE1;
 				}
 
 
 				case PHASE1:
 				{
					ACM.Roll (ai.desiredroll);
					ACM.Pitch (BestClimbAngle());
 					if (!ManoeuvreTimeLeft())
 					{
 						SetManoeuvreTime (1000);
 						ai.PilotSkills.ManStep = PHASE2;
  					}
 
 					break;
 				}
 				case PHASE2:
 				{
 					if (fly.leadflight->World.Y > World.Y)
 					{
						ACM.Roll (ai.desiredroll);
						ACM.Pitch (ANGLES_0Deg);
 					}else
 					{
						ACM.Roll (ai.desiredroll);
						ACM.Pitch (ANGLES_5Deg);
 					}
 					break;
 				}
 
   			}
	   }
   }

}

//------------------------------------------------------------------------------
//Procedure		LineAbreast
//Author		R. Hyde 
//Date			Tue 17 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::LineAbreast()
{


	//only followers go line abreast. We don't link flights
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	if (!leader) 
	{//leader, do same as roundabout, get height, dive and then zoom
	 //at end of zoom, choose manoeuvre based on firsttactic
			ai.PilotSkills.ManStep = PHASE0;
			ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;			
	}else
	{//follower
		if (Range < DANGERRANGE)
		{
			FireABullet(30);
		}
		else
		{
			switch (ai.PilotSkills.ManStep)
			{
				case PHASE0:
				{
					MOD.Rudder = 0;
					ACM.Thrust (ThrustfromFF());
					ai.PilotSkills.ManStep = PHASE1;
					ACM.maxdeltaforclosework = 0.0;

				}
				case PHASE1:
				{

					
					if (!leader)
						ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;					//if no leader break manoeuvre
					else
					{

 						COORDS3D	target;
 						target = leader->World; 
 						InterceptandRange(&target);

						SWord c,s;
						SWord newhdg;
						SLong range;
						newhdg = leader->hdg + ANGLES_90Deg;
						Math_Lib.high_sin_cos((Angles)newhdg,s,c);
						range = METRES20;
						target.X = target.X + MULSHSIN(s, range, ANGLES_SHIFT);
						target.Y = target.Y;
						target.Z = target.Z + MULSHSIN(c, range, ANGLES_SHIFT);


						Float fpitch, fhdg;
						SWord reqroll = GetRequiredRoll(*(rotitem*)this, (COORDS3D)target, fpitch, fhdg );
						if  (		(	(fpitch < ACM.maxdeltaforclosework)
									&&	(fpitch > -ACM.maxdeltaforclosework)
									&&	(fhdg < ACM.maxdeltaforclosework)
									&&	(fhdg > -ACM.maxdeltaforclosework)
									)
							)
						{
							TurnFightPhase1(HdgIntercept, TRUE, ANGLES_0Deg, target);
						}else
						{
							ANGLES rroll;
							rroll = (Angles)(reqroll + roll);
							ACM.Roll (rroll);
							ACM.ZeroSpecificPower();
						}
					}

				}
			}
		}
	}
}


//
//use leader's unfriendly
//also use leader's phase
//Phase 0: stay out of way until separation say 100-200m
//
//phase 1: highest speed dive down to target + offset to avoid collision
//			AutoShoot() will do firing, all fire with  leader	
//phase 2:	at 200 yards, zoom climb
//				success depends on flyingskill
//				will break out of manoeuvre if attacked, damaged, winchester, bingo
//phase 3: establish circle around target, go to phase 0				



//

//------------------------------------------------------------------------------
//Procedure		Scatter
//Author		R. Hyde 
//Date			Mon 30 Ma 1998
//
//Description	this is for a group of aircraft
//				all aircraft go into break turns
//				then mush around, easy targets
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::Scatter()
{

}
//------------------------------------------------------------------------------
//Procedure		Pincer
//Author		R. Hyde 
//Date			Thu 5 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::Pincer()
{

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
			SetManoeuvreTime (2000);
			ai.PilotSkills.ManStep = PHASE1;

		}
		case PHASE1:
		{
			AirStrucPtr	unfriendly = *ai.unfriendly;
			SWord	sinbearing,cosbearing;
			SLong	range = METRES1200;
			ANGLES brg;

			brg = (Angles)((SWord)ai.desiredhdg + (SWord)unfriendly->hdg);
			Math_Lib.high_sin_cos(brg,sinbearing,cosbearing);

			despos = unfriendly->World; 			
			despos.X += MULDIVSIN (sinbearing,range,ANGLES_FRACT);
			despos.Z += MULDIVSIN (cosbearing,range,ANGLES_FRACT);
			despos.Y += FT_500;
			InterceptandRange (&despos);


			MoveToDesiredHdg (HdgIntercept,PitchIntercept, ANGLES_5Deg);
			if	(	(		(!ManoeuvreTimeLeft())
						&&	(Range < COMBATRANGE)
					)
					||
					(!SubjectFacingTarget(ANGLES_80Deg))
				 )
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;
		}
	}


//
//split force into 2: right and left
//find rightmost  ac in trg group
//find leftmost  ac in trg group
//set manoeuvretime to allow pass
//
//right unfriendly = rightmost in trg group
//fly to unfriendly beam(use desired hdg to set up which beam)

//exit when manoeuvretime up (something went wrong)
//or targeton tail
//reset unfriedly to one with lowest HdgIntercept 
}
//------------------------------------------------------------------------------
//Procedure		MultiWave
//Author		R. Hyde 
//Date			Wed 18 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::MultiWave()
{
 //The second wave gets set to this manoeuvre.
 //Aircraft are set to climb. This should provide separation and an advantage
 //Only exit when time is up and in engage range
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
			SetManoeuvreTime (3000);
			ai.PilotSkills.ManStep = PHASE1;
   		}
		case PHASE1:
		{
			MoveToDesiredHdg (HdgIntercept, ANGLES_10Deg, ANGLES_5Deg);
			if  (		(!ManoeuvreTimeLeft())
					&&	(Range < ENGAGERANGE)
				)
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
		}
	}



//	until range to unfriendly is < ENGAGERANGE
//		followleader using formpos to determine distance
//	then slect manoeuvre
}
//------------------------------------------------------------------------------
//Procedure		DiveAndZoom
//Author		R. Hyde 
//Date			Mon 18 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::DiveAndZoom ()

{
 	SWord	deltapitch_pos,deltapitch_sign;
 	SLong	temp_range;
 	COORDS3D	target;
 	SLong		t,actualrange;
 	AirStrucPtr	unfriendly = *ai.unfriendly;
  	SWord		deltaroll;										//RDH 14Mar96
 	SWord		easestickforward;
 	ANGLES		local_PI;
   	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	SWord	reqdeltahdg,reqdeltahdg_sign;
	UWord	reqdeltahdg_pos;
	FP desiredturn;
	SWord	reqdeltapitch,reqdeltapitch_sign;
	UWord	reqdeltapitch_pos;

 
 	local_PI  = PitchIntercept;
	reqdeltahdg = 	hdg - HdgIntercept;
 	Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);
	reqdeltapitch = PitchIntercept - fly.cpitch;//	fly.cpitch - PitchIntercept;

 
 	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{
			MOD.Rudder = 0;
  			MoveToThrust (100);
 			ai.PilotSkills.ManStep = PHASE1;
			SetManoeuvreTime (2000);

 		}
 		case PHASE1:
 		{
  			if  (SubjectOnTargetTail(ANGLES_120Deg))
  			{
  				ai.PilotSkills.ManStep = PHASE2;
  			}
			if (!ManoeuvreTimeLeft())
 			{
 				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
 			}
			target = ai.unfriendly->World;
			
			target.X = ai.unfriendly->World.X;
			target.Y = ai.unfriendly->World.Y + FT_3000;
			target.Z = ai.unfriendly->World.Z;
			InterceptandRange (&target);


			TurnFightPhase1(HdgIntercept, TRUE, ANGLES_0Deg, target);
//			SWord reqroll;
//			if (reqdeltahdg > 0)
//				reqroll = ANGLES_290Deg;
//			else
//				reqroll = ANGLES_70Deg;
//			ACM.Roll ((Angles)reqroll);
////DeadCode DAW 25Nov98 			ACM.LiftLimit();
//			ACM.ZeroSpecificPower();

 			break;
 		}

 		case PHASE2:
 		{
  			if  (		(ClosureLessThan(MPH50))
					&&	(SubjectOnTargetTail(ANGLES_45Deg))
					&&	((World.Y - ai.unfriendly->World.Y) < FT_1000)
				)
  			{
  				GainSpeed ();									//RDH 05Jan99
  			}else
  			{
  				LeadUnFriendly ();
  			}
  			if  ((Range - (World.Y - ai.unfriendly->World.Y)) < ENGAGERANGE)
 
  			{
 				SetManoeuvreTime (5000);
  				ai.PilotSkills.ManStep = PHASE3;
				ACM.maxdeltaforclosework = 0.3;
  			}
  			if (local_PI << ANGLES_90Deg)
  			{
  				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
  			}
 			break;
 		}
 		case PHASE3:
 		{
 			temp_range = Range; 
			SLong vx, vy, vz;

			CalcLead(*this, *ai.unfriendly,vx,vy,vz, TRUE);

			target.X = ai.unfriendly->World.X + vx;
			target.Y = ai.unfriendly->World.Y + vy;
			target.Z = ai.unfriendly->World.Z + vz;
			InterceptandRange (&target);

			Float fpitch, fhdg;
			SWord reqroll = GetRequiredRoll(*(rotitem*)this, (COORDS3D)target, fpitch, fhdg );
				if  (	(	(fpitch < ACM.maxdeltaforclosework)
					&&	(fpitch > -ACM.maxdeltaforclosework)
					&&	(fhdg < ACM.maxdeltaforclosework)
					&&	(fhdg > -ACM.maxdeltaforclosework)
				)
			)
			{
				TurnFightPhase1(HdgIntercept, FALSE, ANGLES_0Deg, target);
			}else
			{
				ANGLES rroll;
				rroll = (Angles)(reqroll + roll);
				ACM.Roll (rroll);
				ACM.ZeroSpecificPower();
			}

			if  (	(!ManoeuvreTimeLeft())
					 ||
					(		(TargetOnSubjectTail(ANGLES_90Deg))
					&&	((World.Y - ai.unfriendly->World.Y) < FT_1000)
					)
				)
			{
			 	ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre=MANOEUVRE_ZOOM;
			}
 			if  (	(ai.PilotSkills.combatskill < SKILL_VETERAN)
 			  	&&	(ChkAtDesiredHdg (HdgIntercept,ANGLES_5Deg))
 				)
 				FireABullet(30);
  			
 			break;
 		}
 
 	}

}
//------------------------------------------------------------------------------
//Procedure		PanicTurn
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	1.63
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::PanicTurn()
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	COORDS3D	target;
	AirStrucPtr	unfriendly = *ai.unfriendly;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
 			ai.desiredhdg = hdg + ANGLES_60Deg; 

			SetManoeuvreTime (1500);

			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
 		{
 			if  (		(!ManoeuvreTimeLeft())
				)
 			{
 				ai.PilotSkills.ManStep = PHASE3;
				SetManoeuvreTime (6000);
 			}
 			if (MoveToDesiredHdg (ai.desiredhdg, ANGLES_5Deg, ANGLES_5Deg))
 			{
				if (Math_Lib.rnd(6))
				{
					ai.PilotSkills.ManStep = PHASE3;
					SetManoeuvreTime (6000);
				}else
				{
 					ai.PilotSkills.ManStep = PHASE2;
 					ai.desiredhdg = hdg - ANGLES_120Deg; 
				}
 			}
 			break;
 		}
 		case PHASE2:
 		{
 			if (	(!ManoeuvreTimeLeft())
 					||	(Range > COMBATRANGE)
 					)
 			{
 				ai.PilotSkills.ManStep = PHASE3;
				SetManoeuvreTime (6000);
 			}
 			if (MoveToDesiredHdg (ai.desiredhdg,ANGLES_5Deg,ANGLES_5Deg))
 			{
				if (Math_Lib.rnd(6))
				{
					ai.PilotSkills.ManStep = PHASE3;
					SetManoeuvreTime (6000);
				}else
				{
	 				ai.PilotSkills.ManStep = PHASE1;
 					ai.desiredhdg = hdg + ANGLES_120Deg; 
				}
 			}
 			break;
 		}

		case PHASE3:
		{
			ACM.maxdeltaforclosework = 0.1;
			target.X = ai.unfriendly->World.X;
			target.Y = ai.unfriendly->World.Y;
			target.Z = ai.unfriendly->World.Z;
			InterceptandRange (&target);
			TurnFightPhase1(HdgIntercept, FALSE, ANGLES_0Deg, target);
 			if (!ManoeuvreTimeLeft())
 			{
  				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
 			}

		}
	}




}

//------------------------------------------------------------------------------
//Procedure		LeadPursuit
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	1.63
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::LeadPursuit()
{
//	ifcombatakill high
//		turn in parallel plane so that target still visible
//
//	calc ideal lead
//		could base calc on maintaining collision course(hdg-hdgintercept?)
//	actual lead depends also on skill
//
// end after x secs or within DANGERRANGE

//DONE IN TURNING FIGHT	
}
//------------------------------------------------------------------------------
//Procedure		LagPursuit
//Author		R. Hyde 
//Date			Tue 10 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::LagPursuit()
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	COORDS3D	target;
	SLong		time;
	AirStrucPtr	unfriendly = *ai.unfriendly;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());

			SetManoeuvreTime (2000);
			ai.PilotSkills.ManStep = PHASE1;
			ai.desiredroll = unfriendly->roll;
			if (unfriendly->roll << ANGLES_180Deg)
				ai.desiredhdg = ANGLES_270Deg;
			else
				ai.desiredhdg = ANGLES_90Deg;

		}
		case PHASE1:
		{
				SWord c,s;
				SWord newhdg;
				SLong range;
				newhdg = unfriendly->hdg + ai.desiredhdg;
				Math_Lib.high_sin_cos((Angles)newhdg,s,c);
				if (SubjectOnTargetTail(ANGLES_90Deg))
					range = METRES200;
				else
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

				target.X = unfriendly->World.X + MULSHSIN(s, range, ANGLES_SHIFT);
				target.Y = unfriendly->World.Y;
				target.Z = unfriendly->World.Z + MULSHSIN(c, range, ANGLES_SHIFT);
				InterceptandRange (&target);

			TurnFightPhase1(HdgIntercept, FALSE, ANGLES_0Deg, target);
			if  (		(!ManoeuvreTimeLeft())
					||	(		(ai.desiredroll << ANGLES_180Deg)
							&&	(unfriendly->roll >>ANGLES_180Deg)
						)
					||	(		(ai.desiredroll >> ANGLES_180Deg)
							&&	(unfriendly->roll << ANGLES_180Deg)
						)
				)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

		}
	}





}

//------------------------------------------------------------------------------
//Procedure		Circumvent
//Author		R. Hyde 
//Date			Mon 30 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::Circumvent()
{
	//will not be implemented
}
//------------------------------------------------------------------------------
//Procedure		DecoyActive
//Author		R. Hyde 
//Date			Mon 30 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
//rdhvoid	AirStruc::DecoyActive()
//{
//}
//------------------------------------------------------------------------------
//Procedure		DecoyPassive
//Author		R. Hyde 
//Date			Mon 30 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
//rdh void	AirStruc::DecoyPassive()
//{
//}
//------------------------------------------------------------------------------
//Procedure		RoundAbout
//Author		R. Hyde 
//Date			Mon 30 Mar 1998
//
//Description	
//															  
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::RoundAbout()
{

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;

			ACM.Thrust (ThrustfromFF());
			SetManoeuvreTime (1500);
 			ai.desiredroll = -(ANGLES)ANGLES_60Deg +
					(Angles)(ANGLES_120Deg * Math_Lib.rnd(2)); 


			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			SWord desroll = ai.desiredroll;
			UWord desroll_pos;
			SWord desroll_sign;
			Math_Lib.Pos (desroll,desroll_pos,desroll_sign);
			if	(SubjectFacingTarget(ANGLES_45Deg))
			//when facing unfriendly unroll to move towards it
				desroll_pos = desroll_pos/2;
			else
				desroll_pos = desroll_pos;
			if (desroll_sign==MathLib::NEGATIVE)
				desroll_pos = -desroll_pos;
			desroll = desroll_pos;
			ACM.Roll ((Angles)desroll);

			ACM.Pitch (BestClimbAngle());
 			if  (		((Range - (World.Y - ai.unfriendly->World.Y)) < ENGAGERANGE)
					&&	((World.Y - ai.unfriendly->World.Y) > FT_3000)
				)
			{
 		    	ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;
			}

		}
	}
//	if leader attain the desiredalt
//	else line astern
//	if unfriendly is set
//			fly towards for x secs
//			flyaway and repeat
//	else
//			circle

}
//------------------------------------------------------------------------------
//Procedure		SplitManoeuvre
//Author		R. Hyde 
//Date			Tue 17 Mar 1998
//
//Description	
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::ResetUnfriendly()
{
//	reset the unfriendlies, so that ac goes after a more appropriate one
//	good to do after a pass or a split manoeuvre
//
}
//------------------------------------------------------------------------------
//Procedure		SplitManoeuvre
//Author		R. Hyde 
//Date			Tue 17 Mar 1998
//
//Description	
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SplitManoeuvre()
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
			SetManoeuvreTime (1500);
			ai.PilotSkills.ManStep = PHASE1;

		}
		case PHASE1:
		{
			if  (		(MoveToDesiredHdg (ai.desiredhdg, ai.desiredpitch, ANGLES_40Deg))
					||	(!ManoeuvreTimeLeft())
				)
			{
				RndVal	rndnum = Math_Lib.rnd();
				if (		(rndnum > RND50PC)
						&&	(ai.morale < MORALE_MEDIUM)
					)
				{
					ai.PilotSkills.ManStep = PHASE0;
					ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;
				}else
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}

		}
	}






}
//------------------------------------------------------------------------------
//Procedure		HeadOn
//Author		R. Hyde 
//Date			Tue 3 Mar 1998
//
//Description	Theoretically no offset, 
//				However use a little to avoid collision
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::HeadOn ( )
{
	//just pass to the side
	//poor pilots may turn too early
	//good pilots may turn early
	//on passing pilot skill will dtermine action

		ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;

	SLong temp_range;
	temp_range = Range; 	
	COORDS3D	target;
	SWord c,s;
	SWord newhdg;
	UByte	FF = FlyingFactor();

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
			SetManoeuvreTime (2000);
			SWord dhdg = HdgIntercept - hdg;
			if ((dhdg) > 0)
				ai.desiredhdg = ANGLES_90Deg;
			else
				ai.desiredhdg = ANGLES_270Deg;
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			ANGLES bca = BestClimbAngle();
			if (PitchIntercept > bca )
				ACM.Pitch (bca);
			else
				ACM.Pitch (PitchIntercept);

			ANGLES desiredhdg = ai.desiredhdg;
			ANGLES unfHdgIntercept = HdgIntercept;
			SLong	unfRange = Range;
			newhdg = unfriendly->hdg + ai.desiredhdg;

			Math_Lib.high_sin_cos((Angles)newhdg,s,c);
			target.X = unfriendly->World.X + ((METRES30 * s) >> ANGLES_SHIFT);
			target.Y = unfriendly->World.Y;
			target.Z = unfriendly->World.Z + ((METRES30 * c) >> ANGLES_SHIFT);
			InterceptandRange (&target);
			desiredhdg = HdgIntercept;

			SWord	reqdeltahdg = hdg - desiredhdg;
		 	SWord reqroll = FindRequiredRoll (reqdeltahdg,CombatReqBankData);
			ACM.Roll ((Angles)reqroll);


			int decisionpoint = Math_Lib.rnd(2);

			if  (		(Range < (2 * BREAKTURNRADIUS ))
					&&	(FF < 200)
				)
			{//poor pilot might turn too early
				if (decisionpoint)
				{
					if (FF < 90)
					{
						ai.PilotSkills.manoeuvre = MANOEUVRE_PANICTURN;
						ai.PilotSkills.ManStep = PHASE0;
					}else  
					{
						ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKTURN;
						ai.PilotSkills.ManStep = PHASE0;
					}
				}else
				{
					ai.PilotSkills.ManStep = PHASE2;
				}

			}else if	(		(Range < BREAKTURNRADIUS)
							
						)
			{//good pilot might turn early to get advantage
				if (decisionpoint)
				{
					ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKTURN;
					ai.PilotSkills.ManStep = PHASE0;
				}else
				{
					ai.desiredhdg = ai.desiredhdg  + ANGLES_180Deg;
					ai.PilotSkills.ManStep = PHASE2;
				}
			}

			if  (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}
		case PHASE2:
		{
			ANGLES bca = BestClimbAngle();
			if (PitchIntercept > bca )
				ACM.Pitch (bca);
			else
				ACM.Pitch (PitchIntercept);

			ANGLES desiredhdg = ai.desiredhdg;
			ANGLES unfHdgIntercept = HdgIntercept;
			SLong	unfRange = Range;
			newhdg = unfriendly->hdg + ai.desiredhdg;

			Math_Lib.high_sin_cos((Angles)newhdg,s,c);
			target.X = unfriendly->World.X + ((METRES30 * s) >> ANGLES_SHIFT);
			target.Y = unfriendly->World.Y;
			target.Z = unfriendly->World.Z + ((METRES30 * c) >> ANGLES_SHIFT);
			InterceptandRange (&target);
			desiredhdg = HdgIntercept;

			SWord	reqdeltahdg = hdg - ai.desiredhdg;
		 	SWord reqroll = FindRequiredRoll (reqdeltahdg,CombatReqBankData);
			MODLIMIT(reqroll, ANGLES_45Deg);
			ACM.Roll ((Angles)reqroll);

			if  (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			if	(SubjectOnTargetTail(ANGLES_80Deg))
			{
				if	(		(ai.unfriendly == Manual_Pilot.ControlledAC2)
						&&	(((AirStruc*)ai.unfriendly)->Follower())
					)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_HEADON, MSG_HEADON, ((AirStruc*)ai.unfriendly)->Follower(), this, ((AirStruc*)ai.unfriendly)));

				int startpoint = Math_Lib.rnd(2);
				ai.PilotSkills.ManStep = PHASE0;
				if (FF < 90)
				{
					if (startpoint)
						ai.PilotSkills.manoeuvre = MANOEUVRE_JINK;
					else
						ai.PilotSkills.manoeuvre = MANOEUVRE_MILDSCISSORS;
				}else if (FF < 200)
				{
					if (startpoint)
							ai.PilotSkills.manoeuvre = MANOEUVRE_STRAIGHTANDLEVEL;
					else
							ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;
				}else 
				{
					if (startpoint)
							ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
					else
							ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKTURN;
				}
			}
		}
	}
	
}
//------------------------------------------------------------------------------
//Procedure		LineAstern
//Author		R. Hyde 
//Date			Tue 17 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::LineAstern()
{
//only followers go line astern. We don't link flights
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	if (!leader) 
	{//leader, do same as roundabout, get height, dive and then zoom
	 //at end of zoom, choose manoeuvre based on firsttactic
			ai.PilotSkills.ManStep = PHASE0;
			ai.PilotSkills.manoeuvre = MANOEUVRE_ROUNDABOUT;			
	}else
	{//follower
		if (Range < WEAPONSRANGE)
		{
			ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;
			ai.PilotSkills.ManStep = PHASE0;
		}
		else
		{
			switch (ai.PilotSkills.ManStep)
			{
				case PHASE0:
				{
					MOD.Rudder = 0;
					ACM.Thrust (ThrustfromFF());
					ai.PilotSkills.ManStep = PHASE1;
					ACM.maxdeltaforclosework = 0.0;

				}
				case PHASE1:
				{

					
					if (!leader)
						ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;					//if no leader break manoeuvre
					else
					{

 						COORDS3D	target;
 						target = leader->World; 
 						InterceptandRange(&target);
						Float fpitch, fhdg;
						SWord reqroll = GetRequiredRoll(*(rotitem*)this, (COORDS3D)target, fpitch, fhdg );
						if  (		(	(fpitch < ACM.maxdeltaforclosework)
									&&	(fpitch > -ACM.maxdeltaforclosework)
									&&	(fhdg < ACM.maxdeltaforclosework)
									&&	(fhdg > -ACM.maxdeltaforclosework)
									)
							)
						{
							TurnFightPhase1(HdgIntercept, TRUE, ANGLES_0Deg, target);
						}else
						{
							ANGLES rroll;
							rroll = (Angles)(reqroll + roll);
							ACM.Roll (rroll);
							ACM.ZeroSpecificPower();
						}
						if (Range < METRES200)
 							   ACM.SpeedBrakesOut();
 						else
 							ACM.SpeedBrakesIn();
 						if (Range < METRES400)
 							ACM.Thrust (0);
 						else
 							ACM.Thrust (ThrustfromFF());

					}

				}
			}
		}
	}
}


//
//use leader's unfriendly
//also use leader's phase
//Phase 0: stay out of way until separation exceeds say 400m
//
//phase 1: if leader 
//				highest speed dive down to target
//			else
//				follow using formation data
//phase 2: when leader in DANGERRANGE
//				do highest speed dive down to target
//			AutoShoot() will do firing	(avoid firing at buddy)
//phase 2:	at 200 yards, zoom climb
//				success depends on flyingskill
//				will break out of manoeuvre if attacked, damaged, winchester, bingo
//phase 3: establish circle around target, go to phase 0				
//

//------------------------------------------------------------------------------
//Procedure		BarrelRollAttack
//Author		Andrew McRae
//Date			Mon 2 Jun 1997
//
//Description	1.69
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::BarrelRollAttack ( )
{
//roll wings level
//pull up
//slow roll towads enemy to keep him in view
//when upside down and behind, pull down

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			ACM.Thrust (100);
			SetManoeuvreTime (1500);
 			ai.desiredroll = ANGLES_0Deg; 
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			ACM.Roll (ai.desiredroll);
			ACM.SetRelAoA(ANGLES_4Deg);

//			ACM.Pitch (ANGLES_30Deg);
//			ACM.MinHeight (FT_2000);

			if  ( !ManoeuvreTimeLeft())
  				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			if (TargetOnSubjectTail(ANGLES_40Deg))
				ai.PilotSkills.ManStep = PHASE2;
			break;

		}
		case PHASE2:
		{
			COORDS3D	target;
			Float fpitch,fhdg;
			AirStrucPtr	unfriendly = *ai.unfriendly;
			target = ai.unfriendly->World;

			SWord	reqdeltahdg,reqdeltahdg_sign;
			UWord	reqdeltahdg_pos;
			reqdeltahdg = 	hdg - HdgIntercept;
 			Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);

			ANGLES reqroll = roll + GetRequiredRoll(*(rotitem*)this, (COORDS3D)target, fpitch, fhdg );
			ACM.Roll (reqroll);
			ACM.TurnRate(fpitch);

			if  (		
						(reqdeltahdg_pos < ANGLES_50Deg)
					||	(World.Y < unfriendly->World.Y)
				)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
				ai.PilotSkills.ManStep = PHASE0;
			}
//			ACM.MinHeight (FT_2000);

		}
	}


}

//------------------------------------------------------------------------------
//Procedure		Scissors
//Author		R. Hyde 
//Date			Tue 12 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::Scissors ( )

{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

 	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{
			MOD.Rudder = 0;
 			ACM.Thrust (70);
			SetManoeuvreTime (1500);
 			ai.PilotSkills.ManStep = PHASE1;
 			ai.desiredhdg = hdg - ANGLES_60Deg; 
 		}
 		case PHASE1:
 		{
 			if (!ManoeuvreTimeLeft())
 			{
 				ai.PilotSkills.ManStep = PHASE3;
 			}
 			if (MoveToDesiredHdg (ai.desiredhdg,ANGLES_5Deg, ANGLES_5Deg))
 			{
 				ai.PilotSkills.ManStep = PHASE2;
 				ai.desiredhdg = hdg + ANGLES_120Deg; 
 			}
 			break;
 		}
 		case PHASE2:
 		{
 			if (	(!ManoeuvreTimeLeft())
 				||	 (!(TargetFacingSubject(ANGLES_40Deg)))
 					||	(Range > COMBATRANGE)
 					)
 			{
 				ai.PilotSkills.ManStep = PHASE3;
 			}
 			if (MoveToDesiredHdg (ai.desiredhdg,ANGLES_5Deg,ANGLES_5Deg))
 			{
 				ai.PilotSkills.ManStep = PHASE1;
 				ai.desiredhdg = hdg - ANGLES_120Deg; 
 			}
 			break;
 		}
 		case PHASE3:
 		{
 
 			ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
 			break;
 		}
 	}

}

//------------------------------------------------------------------------------
//Procedure		MildScissors
//Author		R. Hyde 
//Date			Wed 13 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::MildScissors ( )

{
	
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

 	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{
			MOD.Rudder = 0;
 			ACM.Thrust (70);
			SetManoeuvreTime (1500);
 			ai.PilotSkills.ManStep = PHASE1;
 			ai.desiredhdg = hdg - ANGLES_15Deg; 
 		}
 		case PHASE1:
 		{
 			if (!ManoeuvreTimeLeft())
 			{
 				ai.PilotSkills.ManStep = PHASE3;
 			}
 			if (MoveToDesiredHdg (ai.desiredhdg,ANGLES_5Deg,ANGLES_5Deg))
 			{
 				ai.PilotSkills.ManStep = PHASE2;
 				ai.desiredhdg = hdg + ANGLES_30Deg; 
 			}
 			break;
 		}
 		case PHASE2:
 		{
 			if (	(!ManoeuvreTimeLeft())
 				||	 (!(TargetFacingSubject(ANGLES_40Deg)))
 					||	(Range > COMBATRANGE)
 					)
 			{
 				ai.PilotSkills.ManStep = PHASE3;
 			}
 			if (MoveToDesiredHdg (ai.desiredhdg,ANGLES_5Deg,ANGLES_5Deg))
 			{
 				ai.PilotSkills.ManStep = PHASE1;
 				ai.desiredhdg = hdg - ANGLES_30Deg; 
 			}
 			break;
 		}
 		case PHASE3:
 		{
 
 			ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
 			break;
 		}
 	}

}
//------------------------------------------------------------------------------
//Procedure		TurningFight
//Author		R. Hyde 
//Date			Wed 13 Dec 1995
//
//Description	attempt to lead, find Hdgintercept and delta, set roll
//				roll should not give alt lose
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
//void AirStruc::TurningFight ()
//
//{
//	ACMMODEL& ACM = *fly.pAcmModel;
//	MODEL& MOD = *fly.pModel;
//
//	SLong temp_range;
//	temp_range = Range; 	
//
//	switch (ai.PilotSkills.ManStep)
//	{
//		case PHASE0:
//		{
//			ACM.Clear ();
//			if (ai.skill > SKILL_REGULAR)
//				ACM.Thrust (100);
//			else
//				ACM.Thrust (70);
//
//			SetManoeuvreTime (400);
//			ai.PilotSkills.ManStep = PHASE1;
//		}
//		case PHASE1:
//		{
//
//			ACM.MinHeight (FT_2000);
//
//		 	SWord	reqdeltahdg = 	Rads2Rowan(MOD.fHdg) - HdgIntercept;
//		 	SWord reqroll = FindRequiredRoll (reqdeltahdg,CombatReqBankData);
//			ACM.Roll ((Angles)reqroll);
////test
//		UWord y = 0;
//		if (ai.skill > SKILL_REGULAR)
//				y = 4;
//		char strg [32];
//		for (UWord n=0; n<32; n++) strg[n] = 0;
//		sprintf (strg, "DHdgIntercept  %d ",reqdeltahdg);
//		if (strg[31] == 0)
//			Mono_Text.PrintAt (40,y,(UByte*)strg);
//
//		for (n=0; n<32; n++) strg[n] = 0;
//		sprintf (strg, "Range,m  %d ",Range/100);
//		if (strg[31] == 0)
//			Mono_Text.PrintAt (40,y+1,(UByte*)strg);
//
//		for (n=0; n<32; n++) strg[n] = 0;
//		sprintf (strg, "ReqRoll  %d ",reqroll);
//		if (strg[31] == 0)
//			Mono_Text.PrintAt (40,y+2,(UByte*)strg);
///

//			ACM.Pitch (PitchIntercept);


			// test
//			ACM.DesiredRoll (ANGLES_0Deg);
//			ACM.KeepWingsLevel ();
//			ACM.FlyDesiredRoll ();

//			if  (!ManoeuvreTimeLeft())
//				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
//
//		}
//	}

//}

//------------------------------------------------------------------------------
//Procedure		SplitS
//Author		R. Hyde 
//Date			Mon 18 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SplitS ()

{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
 	SWord 	deltaroll;
 	UWord	deltaroll_pos;
 	SWord	deltaroll_sign;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	Float fpitch, fhdg;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			int groundlevel = Land_Scape.GetGroundLevel(this);
			if(World.Y - groundlevel < 150000)	//5000 ft
			{
				if(Math_Lib.rnd(1))
					ai.PilotSkills.manoeuvre = MANOEUVRE_SCISSORS;
				else
 					ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKTURN;
				SetManoeuvreTime(0);
			}
			else
			{
				MOD.Rudder = 0;
				ACM.Thrust (ThrustfromFF());
				COORDS3D	target;
				target = unfriendly->World;
				ANGLES reqroll = (roll + GetRequiredRoll(*(rotitem*)this, (COORDS3D)target, fpitch,fhdg ));
				if (	(reqroll >> ANGLES_120Deg)
					&&	(reqroll << ANGLES_240Deg)
					)
					ai.desiredroll = reqroll;
				else
					ai.desiredroll = ANGLES_180Deg;
				SetManoeuvreTime (1000);

				ai.PilotSkills.ManStep = PHASE1;
			}
		}
		case PHASE1:
		{

			ACM.Roll (ai.desiredroll);
			deltaroll = (SWord) (ai.desiredroll - roll);
			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
			if  (		( roll >> ANGLES_90Deg)
					&&	(roll << ANGLES_270Deg)
					&&	(deltaroll_pos <ANGLES_30Deg)
				)
				ACM.LiftLimit();
			else
				ACM.SetElevator(0);

			if (deltaroll_pos <ANGLES_20Deg)
			{
				ai.PilotSkills.ManStep = PHASE2;
				SetManoeuvreTime (1500);
			}
			break;
		}
 		case PHASE2:
		{
			MOD.Aileron = 0;
			ACM.LiftLimit();
			if  ( 		(!ManoeuvreTimeLeft())
					||  (		(DesiredPitchChk (ANGLES_315Deg,ANGLES_20Deg))
							&&	(		(roll << ANGLES_90Deg)
									||	(roll >> ANGLES_270Deg)
								)						
						)
				)
	 			{
 					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
	 			}
	 			break;
		}
	}

}

//------------------------------------------------------------------------------
//Procedure		ZoomandDrop
//Author		R. Hyde 
//Date			Mon 18 Dec 1995
//
//Description	
//Zoom for only a few seconds in hope of gettig other a/c to slip under
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::ZoomandDrop ()
{

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

//	ACM.Step = ai.PilotSkills.ManStep;

		
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SetManoeuvreTime (1000);
			MOD.Rudder = 0;
			ACM.Thrust (0);
			ACM.SpeedBrakesOut ();

			ai.PilotSkills.ManStep = PHASE1;
		}

		case PHASE1:
		{
			ACM.KeepWingsLevel ();
			ACM.Pitch (ANGLES_40Deg);
			if  (	(MOD.Inst.MachNo < 0.3)
				||	(!ManoeuvreTimeLeft ())
				)
			{
				ACM.Clear ();
				ACM.Thrust (ThrustfromFF());
				ACM.SpeedBrakesIn ();
				SWord	reqdeltahdg,reqdeltahdg_sign;
				UWord	reqdeltahdg_pos;
				reqdeltahdg = 	hdg - HdgIntercept;
	 			Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);
				if (reqdeltahdg_pos < ANGLES_30Deg)
				{
					ai.PilotSkills.ManStep = PHASE0;
					ai.PilotSkills.manoeuvre=MANOEUVRE_TURNINGFIGHT;
				}
				else
				{
					SetManoeuvreTime (700);
					ai.PilotSkills.ManStep = PHASE2;
				}
			}

			break;
		}


		case PHASE2:
		{
			ACM.Pitch (ANGLES_290Deg);
			ACM.KeepWingsLevel ();
			if (!ManoeuvreTimeLeft ())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}

	}


}

//------------------------------------------------------------------------------
//Procedure		StraightandLevel
//Author		Andrew McRae
//Date			Wed 14 May 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::StraightandLevel ()
{

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;


	UByte	FF = FlyingFactor();

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SetManoeuvreTime (2000);
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			//this manoeuvre is only done by poor pilots so we can reduce speed so that
			//other can catch up and attack
				FP	Mach = MOD.Inst.MachNo;
			if (Mach < 0.5)
				ACM.Thrust (ThrustfromFF());
			else
				ACM.Thrust (0);

			if (FF > 160)
			{
				if (Mach > 0.9)
					Mach = 0.9;
				if (Mach < 0.3)
					Mach = 0.3;

				ANGLES desiredpitch;
				SWord deltapitch;
				deltapitch = ANGLES_5Deg * ((Mach - 0.3)/0.6);
				desiredpitch = (Angles)((SWord)ANGLES_355Deg + deltapitch);
				ACM.Pitch (desiredpitch);

			}else
				ACM.Pitch (ANGLES_1Deg);
			ACM.KeepWingsLevel ();
			if (!ManoeuvreTimeLeft ())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}
	}
}


//------------------------------------------------------------------------------
//Procedure		SpinOut
//Author		R. Hyde 
//Date			Tue 12 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SpinOut ( )

{
	SWord	deltaroll;
	UWord	deltayaw_pos;
	SWord	deltayaw_sign;

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			ACM.Thrust (0);
			ACM.SetElevator(-MAXELEVATOR/2);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			if (MOD.RotVel.y > 0)
			{
				MOD.Rudder = MAXRUDDER;
			}else
			{
				MOD.Rudder = -MAXRUDDER;
			}
			ai.PilotSkills.ManStep = PHASE2;
			SetManoeuvreTime (2000);
			break;
		}
		case PHASE2:
		{
			ACM.SetElevator(-MAXELEVATOR);
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.ManStep = PHASE0;
				ACM.SetElevator(0);
				MOD.Rudder = 0;
				ai.PilotSkills.manoeuvre = MANOEUVRE_SPINRECOVERY;

			}
			break;
		}
	}

}

//------------------------------------------------------------------------------
//Procedure		DiveforHome
//Author		Andrew McRae
//Date			Tue 3 Jun 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::DiveforHome ()
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
 	SLong	temp_range;
 
 	temp_range = Range; 	
 	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{

			MOD.Rudder = 0;
  			MoveToThrust (100);
 			SetManoeuvreTime (2000);
 			ai.PilotSkills.ManStep = PHASE1;

			ACM.Thrust (ThrustfromFF());

			if (ai.homebase)
				InterceptandRange (&ai.homebase->World);
			else
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
 			ai.desiredhdg = HdgIntercept; 
 		}
 		case PHASE1:
 		{
//##			if (Range < MILES01)
//			{//within mile of home
//				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
//			}
//			Range = temp_range; 	
//			if (Range > BREAKOFFRANGE)
//			{//xx from action, reconsider
//				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
//			}
			if (Range < DANGERRANGE)
			{//xx from action, reconsider
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			if  (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;


			ANGLES	desiredpitch;
			if (MOD.Pos.y < FT_3000)
				desiredpitch = ANGLES_0Deg;
			else
				desiredpitch = ANGLES_340Deg;

			MoveToHdg (ai.desiredhdg, desiredpitch);
 		}
 	}


}
//------------------------------------------------------------------------------
//Procedure		GoHome
//Author		R. Hyde 
//Date			Tue 12 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::GoHome ( )

{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
 	SLong	temp_range;
 
 	temp_range = Range; 	
 	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{
			MOD.Rudder = 0;
  			MoveToThrust (100);
 			SetManoeuvreTime (2000);
 			ai.PilotSkills.ManStep = PHASE1;

			ACM.Thrust (ThrustfromFF());

			if (ai.homebase)
				InterceptandRange (&ai.homebase->World);
			else
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
 			ai.desiredhdg = HdgIntercept; 
			Range = temp_range;
 			if	(		TargetOnSubjectTail(ANGLES_10Deg)
 					&&	(Range < 2 * WEAPONSRANGE)
 					&&	(ai.PilotSkills.combatskill >= SKILL_REGULAR)
 				)
 				ai.desiredhdg = HdgIntercept + ANGLES_20Deg; 

 		}
 		case PHASE1:
 		{
//##			if (Range < MILES01)
//			{//within mile of home
//				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
//			}
			Range = temp_range; 	
//			if (Range > BREAKOFFRANGE)
//			{//xx from action, reconsider
//				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
//			}
			if (Range < DANGERRANGE)
			{//xx from action, reconsider
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			if  (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;


				FP	Mach = MOD.Inst.MachNo;
				if (Mach > 0.9)
					Mach = 0.9;
				if (Mach < 0.3)
					Mach = 0.3;

				ANGLES desiredpitch;
				SWord deltapitch;
				deltapitch = ANGLES_5Deg * ((Mach - 0.3)/0.6);
				desiredpitch = (Angles)((SWord)ANGLES_355Deg + deltapitch);
//				ACM.Pitch (desiredpitch);

 			MoveToHdg (ai.desiredhdg, desiredpitch);
 		}
 	}

}

//------------------------------------------------------------------------------
//Procedure		MakeForFriendly
//Author		R. Hyde 
//Date			Tue 12 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::MakeForFriendly ( )

{
	
}



//------------------------------------------------------------------------------
//Procedure		MoveAway
//Author		Andrew McRae
//Date			Mon 2 Jun 1997
//
//Description	
//	Not much different from extension. Ant need for both?
//
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::MoveAway ()
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
			SetManoeuvreTime (2000);
			ai.PilotSkills.ManStep = PHASE1;
			break;
		}
		case PHASE1:
		{
			ACM.MinHeight (FT_1000);
//			ACM.Pitch (ANGLES_5Deg);
			MoveToHdg (HdgIntercept + ANGLES_180Deg, ANGLES_5Deg);
 
			if (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}
	}

}	


//rdh}
//------------------------------------------------------------------------------
//Procedure		HeadOnOffset
//Author		R. Hyde 
//Date			Wed 13 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::HeadOnOffset ( )

{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;

	SLong temp_range;
	temp_range = Range; 	
	COORDS3D	target;
	SWord c,s;
	SWord newhdg;
	UByte	FF; 
	SWord	reqdeltahdg;


	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
			SetManoeuvreTime (2000);
			SWord dhdg = HdgIntercept - hdg;
			if ((dhdg) > 0)
				ai.desiredhdg = ANGLES_90Deg;
			else
				ai.desiredhdg = ANGLES_270Deg;
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{

			ANGLES bca = BestClimbAngle();
			if (PitchIntercept > bca )
				ACM.Pitch (bca);
			else
				ACM.Pitch (PitchIntercept);




			ANGLES desiredhdg = ai.desiredhdg;
			ANGLES unfHdgIntercept = HdgIntercept;
			SLong	unfRange = Range;
			newhdg = unfriendly->hdg + ai.desiredhdg;

			Math_Lib.high_sin_cos((Angles)newhdg,s,c);
			target.X = unfriendly->World.X + ((BREAKTURNRADIUS * s) >> ANGLES_SHIFT);
			target.Y = unfriendly->World.Y;
			target.Z = unfriendly->World.Z + ((BREAKTURNRADIUS * c) >> ANGLES_SHIFT);
			InterceptandRange (&target);
			desiredhdg = HdgIntercept;

			SWord	reqdeltahdg = hdg - desiredhdg;
		 	ANGLES reqroll = (Angles)FindRequiredRoll (reqdeltahdg,CombatReqBankData);
			if  (		(reqroll >> ANGLES_30Deg)
					&&	(reqroll << ANGLES_180Deg)
				)
				reqroll = ANGLES_30Deg;
			if  (		(reqroll << ANGLES_330Deg)
					&&	(reqroll >> ANGLES_180Deg)
				)	
				reqroll = ANGLES_330Deg;

			ACM.Roll (reqroll);

			if (unfRange < Range)
				Range = unfRange;
			if  (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			if	(		(SubjectOnTargetTail(ANGLES_85Deg))
					||	(TargetOnSubjectTail(ANGLES_85Deg))
				)
			{
		  		ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;						
			}else if (Range < (BREAKTURNRADIUS))
			{
				FF = Math_Lib.rnd(4);
				if (		(FF  < 2)
						&&	(		(unfriendly->roll >> ANGLES_30Deg)
								&&	(unfriendly->roll << ANGLES_330Deg)
							)
					)
				{//turn in
					reqdeltahdg = 	hdg - unfHdgIntercept;
					if (reqdeltahdg < 0)
					{
						ai.desiredroll = ANGLES_85Deg;
						ai.desiredhdg = ANGLES_90Deg + hdg;
					}else
					{
						ai.desiredhdg = ANGLES_270Deg + hdg;
						ai.desiredroll = ANGLES_275Deg;
					}
					ai.PilotSkills.ManStep = PHASE2;
				}else
				{//wait
					ai.PilotSkills.ManStep = PHASE3;
				}
			}
			break;
		}
		case PHASE2:
		{
			ACM.Roll (ai.desiredroll);
			ACM.ZeroSpecificPower();
			if (ChkAtDesiredHdg (ai.desiredhdg,ANGLES_5Deg))
			{
		  		ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;						
			}
			break;
		}
		case PHASE3:
		{
//			if (PitchIntercept >> pitch)
				ACM.Pitch (PitchIntercept);
//			else
//				ACM.Pitch (pitch + ANGLES_3Deg);
			ANGLES desiredhdg = ai.desiredhdg;

			newhdg = unfriendly->hdg + ai.desiredhdg;
			Math_Lib.high_sin_cos((Angles)newhdg,s,c);
			target.X = unfriendly->World.X + ((BREAKTURNRADIUS * s) >> ANGLES_SHIFT);
			target.Y = unfriendly->World.Y;
			target.Z = unfriendly->World.Z + ((BREAKTURNRADIUS * c) >> ANGLES_SHIFT);
			InterceptandRange (&target);
			desiredhdg = HdgIntercept;

			SWord	reqdeltahdg = hdg - desiredhdg;
		 	SWord reqroll = FindRequiredRoll (reqdeltahdg,CombatReqBankData);
			// RERUN: The following code makes no sense as bit shifts at first glance. 
			// Maybe it should be if ((reqroll > ANGLES_30Deg) && (reqroll < ANGLES_180Deg)), TBC.
			if  (		(reqroll >> ANGLES_30Deg) && (reqroll << ANGLES_180Deg)) // RERUN: this code
				reqroll = ANGLES_30Deg;
			if  (		(reqroll << ANGLES_330Deg) && (reqroll >> ANGLES_180Deg)) // RERUN same here.	
				reqroll = ANGLES_330Deg;

			ACM.Roll ((Angles)reqroll);

			if  (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			else if	(		(SubjectOnTargetTail(ANGLES_80Deg))
					||	(TargetOnSubjectTail(ANGLES_80Deg))
				)
			{
				if  (		(unfriendly->roll << ANGLES_30Deg)
						||	(unfriendly->roll >> ANGLES_330Deg)
					)
				{
			  		ai.PilotSkills.ManStep = PHASE0;
					ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;						

				}else 
				{
					if (		(roll >> ANGLES_30Deg)
							&&	(roll << ANGLES_180Deg)
						)
					{
						ai.desiredroll = ANGLES_85Deg;
						ai.desiredhdg = ANGLES_90Deg + hdg;
					}else if (	(roll << ANGLES_330Deg)
								&&	(roll >> ANGLES_180Deg)
							 )
					{
						ai.desiredhdg = ANGLES_270Deg + hdg;
						ai.desiredroll = ANGLES_275Deg;
					}else if ((hdg - HdgIntercept) < 0)
					{
						ai.desiredroll = ANGLES_85Deg;
						ai.desiredhdg = ANGLES_90Deg + hdg;
					}else
					{
						ai.desiredhdg = ANGLES_270Deg + hdg;
						ai.desiredroll = ANGLES_275Deg;
					}
					
//					if  (		(Math_Lib.rnd(3) == 0)
//						||	(Range > BREAKTURNRADIUS/2)
//					)
//					{
//						if  (		(unfriendly->roll << ANGLES_180Deg)
//								||	(roll >> ANGLES_20Deg)
//							)
//						{
//							ai.desiredroll = ANGLES_85Deg;
//							ai.desiredhdg = ANGLES_90Deg + hdg;
//						}else
//						{
//							ai.desiredhdg = ANGLES_270Deg + hdg;
//							ai.desiredroll = ANGLES_275Deg;
//						}
//					}else
//					{
//						if (	(unfriendly->roll >> ANGLES_180Deg)
//							||	(roll << ANGLES_340Deg)	
//							)
//						{
//							ai.desiredroll = ANGLES_85Deg;
//							ai.desiredhdg = ANGLES_90Deg + hdg;
//						}else
//						{
//							ai.desiredhdg = ANGLES_270Deg + hdg;
//							ai.desiredroll = ANGLES_275Deg;
//						}
//					}
			  		ai.PilotSkills.ManStep = PHASE4;
				}
			}		
			break;

		}
		case PHASE4:
		{
			ACM.Roll (ai.desiredroll);
			ACM.ZeroSpecificPower();
			if (ChkAtDesiredHdg (ai.desiredhdg,ANGLES_5Deg))
			{
		  		ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;						
			}

		}
	}
}


//------------------------------------------------------------------------------
//Procedure		StayWithPrey
//Author		R. Hyde 
//Date			Wed 13 Dec 1995
//
//Description	As target looses alt, stay until explosion or hits ground or alt <???
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::StayWithPrey ( )

{

}

//------------------------------------------------------------------------------
//Procedure		Immelmann
//Author		R. Hyde 
//Date			Wed 13 Dec 1995
//
//Description	not used and so not coded for Flying Corps
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::Immelmann ( )

{
	AirStrucPtr	unfriendly = *ai.unfriendly;

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

//	ACM.Step = ai.PilotSkills.ManStep;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (100);
			SetManoeuvreTime (2000);
			if (		(MOD.Inst.MachNo != 0)		//really just for test purposes, first time in mchno is not set
					&&	(MOD.Inst.MachNo < 0.55)
				)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
			}else
				ai.PilotSkills.ManStep = PHASE1;
			break;
		}
		case PHASE1:
		{
			ACM.Pitch (ANGLES_355Deg);
			if (!ManoeuvreTimeLeft ())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			if (MOD.Inst.MachNo > 0.6)
			{
				ai.desiredroll = ANGLES_0Deg;
				SetManoeuvreTime (1000);
				ai.PilotSkills.ManStep = PHASE2;
			}
			break;
		}
		case PHASE2:
		{
			SWord 	deltaroll;
			UWord	deltaroll_pos;
		 	SWord	deltaroll_sign;
			ACM.Roll (ai.desiredroll);
 			deltaroll = (SWord) (ai.desiredroll - roll);
 			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
 			if (deltaroll_pos <ANGLES_5Deg)
			{
				ai.PilotSkills.ManStep = PHASE3;
				SetManoeuvreTime (1000);
				ai.desiredhdg = hdg;
			}
			if (!ManoeuvreTimeLeft ())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}
		case PHASE3:
		{
			ACM.LiftLimit();
			ACM.Roll (ANGLES_0Deg);
 			if  (	(roll >> ANGLES_90Deg)
				&&	(roll << ANGLES_270Deg)
				)
			{
				ai.PilotSkills.ManStep = PHASE4;
				SetManoeuvreTime (1000);
			}
			if (!ManoeuvreTimeLeft ())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}
		case PHASE4:
		{
			SWord 	deltaroll;
			UWord	deltaroll_pos;
		 	SWord	deltaroll_sign;
			Float fpitch,fhdg;
			COORDS3D	target;
			target = ai.unfriendly->World;

			ANGLES reqroll = roll + GetRequiredRoll(*(rotitem*)this, (COORDS3D)target, fpitch, fhdg );
			ACM.Roll (reqroll);

			int ele = MOD.Elevator + MOD.MODEL_DT * 3000;
			if (ele >0)
				ele = 0;
			ACM.SetElevator(ele);
			
			deltaroll = (SWord) (reqroll - roll);
			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
			if (deltaroll_pos <ANGLES_5Deg)
			{
				ai.PilotSkills.ManStep = PHASE5;
				SetManoeuvreTime (1000);
			}
			if (!ManoeuvreTimeLeft ())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}
		case PHASE5:
		{
			ACM.KeepWingsLevel ();
			ACM.LiftLimit();
			if   (	(DesiredPitchChk (ANGLES_0Deg, ANGLES_10Deg))
				 ||	(!ManoeuvreTimeLeft ())
				 )
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

		}
	}



}
//------------------------------------------------------------------------------
//Procedure		ClimbforHome
//Author		R. Hyde 
//Date			Tue 12 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::ClimbforHome ( )

{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
 	SLong	temp_range;
 
 	temp_range = Range; 	
 	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{
			MOD.Rudder = 0;
  			MoveToThrust (100);
 			SetManoeuvreTime (2000);
 			ai.PilotSkills.ManStep = PHASE1;

			ACM.Thrust (ThrustfromFF());

 			InterceptandRange (&ai.homebase->World);
 			ai.desiredhdg = HdgIntercept; 
 		}
 		case PHASE1:
 		{
//##			if (Range < MILES01)
//			{//within mile of home
//				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
//			}
			Range = temp_range; 	
//			if (Range > BREAKOFFRANGE)
//			{//xx from action, reconsider
//				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
//			}
			if (Range < DANGERRANGE)
			{//xx from action, reconsider
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			if  (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

//			ACM.Pitch (BestClimbAngle());
 			MoveToHdg (ai.desiredhdg, BestClimbAngle());
 		}
 	}
}


//------------------------------------------------------------------------------
//Procedure		StraightDive
//Author		Andrew McRae
//Date			Tue 3 Jun 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::StraightDive ()
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			SetManoeuvreTime (2000);
			ACM.Thrust (ThrustfromFF());

			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			ACM.Roll (ANGLES_0Deg);
			ACM.Pitch (ANGLES_330Deg);
			if (MOD.Pos.y < FT_2000)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			if  (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}
	}


}


//------------------------------------------------------------------------------
//Procedure		StandOnTail
//Author		
//Date			Mon 8 Sep 1997
//
//Description	
//Zoom until low speed, then roll 180 and pull level
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::StandOnTail ()
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			SetManoeuvreTime (2000);	//800
			ACM.Thrust (100);

			ACM.ClimbRate (0);
			ACM.KeepWingsLevel ();
			ai.PilotSkills.ManStep = PHASE1;
			break;
		}

		case PHASE1:
		{
			if (MOD.Inst.IndicatedAirSpeed > 450)
			{

				ACM.Clear ();
				ACM.Thrust (100);
//				ACM.StayVertical ();

				ai.PilotSkills.ManStep = PHASE2;
			}
			break;
		}

		case PHASE2:
		{
 			if (!ManoeuvreTimeLeft())
 				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

// 			if (ACM.pModel->Vel.y <= 0)
//				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

			break;
		}
	}

}
//------------------------------------------------------------------------------
//Procedure		ShootToFrighten
//Author		R. Hyde 
//Date			Tue 12 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::ShootToFrighten ( )

{
	ACMMODEL& ACM = *fly.pAcmModel;
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			ACM.Clear ();
			SetManoeuvreTime (1000);
			ACM.maxdeltaforclosework = 0.0;
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			SWord	reqdeltahdg,reqdeltahdg_sign;
			UWord	reqdeltahdg_pos;
			COORDS3D	target;
			SLong		time;
			MODEL& MOD = *fly.pModel;
			AirStrucPtr	unfriendly = *ai.unfriendly;


			reqdeltahdg = 	Rads2Rowan(MOD.fHdg) - HdgIntercept;
			Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);

			if (		(Range < METRES400)
					&&	(reqdeltahdg_pos < (SWord)ANGLES_30Deg)
				)		
			{
				//this method just extrapolates position
				//to get turn:
				//get distance travelled and turn in time to work out offset
				time = Range / classtype->muzzlevel;		//MUZZELVEL;
				//calc lead position using this time
				target.X = ai.unfriendly->World.X + (unfriendly->velx * time );
				target.Y = ai.unfriendly->World.Y + (unfriendly->vely * time );
				target.Z = ai.unfriendly->World.Z + (unfriendly->velz * time );
				InterceptandRange (&target);
			}else
				target = ai.unfriendly->World;

			TurnFightPhase1(HdgIntercept, TRUE, ANGLES_0Deg, target);
			if (		(Range < ENGAGERANGE)
					&&	((unfriendly->vel - vel) > 0)
					&&	(		(unfriendly->roll << ANGLES_10Deg)
							||	(unfriendly->roll >> ANGLES_350Deg)
						)
				)	
			{
						FireABullet(120);
						if	(	(		(unfriendly->ai.morale < MORALE_GOOD)
									||	(unfriendly->ai.PilotSkills.combatskill < SKILL_VETERAN)
								)
								&&
								(ai.unfriendly != Persons2::PlayerGhostAC)
							)
						{
							unfriendly->ai.PilotSkills.manoeuvre = MANOEUVRE_PANICTURN;
							unfriendly->ai.PilotSkills.ManStep = PHASE0;
						}
			}


		}
	}



}
//------------------------------------------------------------------------------
//Procedure		ShootTooEarly
//Author		R. Hyde 
//Date			Mon 30 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::ShootTooEarly()
{
	ACMMODEL& ACM = *fly.pAcmModel;
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			ACM.Clear ();
			SetManoeuvreTime (1000);
			ACM.maxdeltaforclosework = 0.0;
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{

				SWord	reqdeltahdg,reqdeltahdg_sign;
			UWord	reqdeltahdg_pos;
			COORDS3D	target;
			SLong		time;
			MODEL& MOD = *fly.pModel;
			AirStrucPtr	unfriendly = *ai.unfriendly;

			target = ai.unfriendly->World;

			reqdeltahdg = 	Rads2Rowan(MOD.fHdg) - HdgIntercept;
			Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);

			if (		(Range < METRES400)
					&&	(reqdeltahdg_pos < (SWord)ANGLES_30Deg)
				)		
			{
				//this method just extrapolates position
				//to get turn:
				//get distance travelled and turn in time to work out offset
				time = Range / classtype->muzzlevel;		//MUZZELVEL;
				//calc lead position using this time
				target.X = ai.unfriendly->World.X + (unfriendly->velx * time );
				target.Y = ai.unfriendly->World.Y + (unfriendly->vely * time );
				target.Z = ai.unfriendly->World.Z + (unfriendly->velz * time );
				InterceptandRange (&target);
			}else
				target = ai.unfriendly->World;

			if (		(Range < ENGAGERANGE)
					&&	((unfriendly->vel - vel) < 0)
				)	
						FireABullet(10);

			TurnFightPhase1(HdgIntercept, TRUE, ANGLES_0Deg, target);
		}
	}

}

ANGLES AirStruc::BestClimbAngle()
{
//## needs rewrite
		return ( classtype->bestclimbpitch);
}

//------------------------------------------------------------------------------
//Procedure		GainHeight
//Author		R. Hyde 
//Date			Tue 12 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::GainHeight ( )

{
	
	// ##Select Max power
	// and maintain best climb speed
	// fly flat if < bestclimb speed
	

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

//	ACM.Step = ai.PilotSkills.ManStep;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SWord reqdeltahdg = 	hdg - HdgIntercept;
			ACM.Thrust (ThrustfromFF());
//			if (reqdeltahdg > 0)
//				ai.desiredroll = ANGLES_330Deg;
//			else
//				ai.desiredroll = ANGLES_30Deg;

			SetManoeuvreTime (1500);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{

//			Angles gcurve [] =
//			{//0-11g
//				ANGLES_0Deg,ANGLES_0Deg,ANGLES_60Deg,ANGLES_70_5Deg,ANGLES_75_5Deg,ANGLES_78_5Deg,ANGLES_80_4Deg,
//				ANGLES_81_8Deg,ANGLES_82_8Deg,ANGLES_83_6Deg,ANGLES_84_3Deg,ANGLES_84_8Deg,
//			};
//
 //			SWord reqroll;
//
//			int g, g100;
//			FP gfp;
//			gfp = MOD.Inst.NormalAcc;
//			FP modifier = 10;
//
//
//			gfp = gfp + ((modifier * (SWord)(fly.cpitch))/ ((SWord)ANGLES_70Deg));
//
//
//
//			SWord d1,d2;
//			g =	(int)gfp;
//			g100 = 100 * gfp;
//				
//			SWord rroll;
//			if (g <0)
//				g = 0;
//			if (g > 10)
//				g = 10;
//			d1 = gcurve[g];
 //			d2 = gcurve[g + 1];
//		 	rroll = d1+ (d2-d1) * (g100 - g * 100)/100;
//			if (rroll < ANGLES_50Deg)
//				rroll = ANGLES_50Deg;
//			ACM.Roll ((Angles)rroll);
//			ACM.LiftLimit();

//##should attempt to get to side
			ACM.Pitch (BestClimbAngle());
			SWord	reqdeltahdg = hdg - HdgIntercept;
		 	SWord reqroll = FindRequiredRoll (reqdeltahdg,CombatReqBankData);
			//##skill factor
			if (reqroll > ANGLES_30Deg)
				reqroll = ANGLES_30Deg;
			if (reqroll < ANGLES_330Deg)
				reqroll = ANGLES_330Deg;
			ACM.Roll ((Angles)reqroll);

			if  (		(!ManoeuvreTimeLeft())
					||	(		(Range < ENGAGERANGE)
							&&	SubjectFacingTarget(ANGLES_45Deg)
						)	
				)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			
			
			
			break;	
			
			
			
			
			
//			MOD.Rudder = 0;
//			SetManoeuvreTime (2000);
//			ACM.GearUp ();
//			UByte	FF = FlyingFactor();
//			SWord thrust = 85 + (FF * 15)/256;	
//			if (thrust > 95)
//				thrust = 100;
//			ACM.Thrust (thrust);
//			if (ai.PilotSkills.combatskill > SKILL_REGULAR)
//			{
//				SWord	reqdeltahdg;
//				reqdeltahdg = 	hdg - HdgIntercept;
//				if (reqdeltahdg > 0)
//					ai.desiredhdg = HdgIntercept + ANGLES_20Deg;
//				else
//					ai.desiredhdg = HdgIntercept - ANGLES_20Deg;
//
//			}
//			else
//				ai.desiredhdg = HdgIntercept;
//
////			ACM.ClimbRate (-1500);
//			ai.PilotSkills.ManStep = PHASE1;
		}
//		case PHASE1:
//		{
 //			MoveToHdg (ai.desiredhdg);
//			ACM.Pitch (BestClimbAngle());
////			if (MOD.Inst.MachNo < 0.4)
//			if (vel <= SustainedTurnSpeed())
//				ACM.Pitch (ANGLES_355Deg);
//			else
//				ACM.Pitch (ANGLES_5Deg);


//			if (!ManoeuvreTimeLeft())
//				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
//			break;
//		}
	}

}

//------------------------------------------------------------------------------
//Procedure		LagRoll
//Author		R. Hyde 
//Date			Tue 3 Mar 1998
//
//Description	Ref1.67..	
//				manoeuvre incorporates barrel roll attack(1.69..)
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::LagRoll()
{
//maintain throttle
//roll towards unfriendly for 10-20deg hdg (f(AOT))
//go upside down
//climb to loose speed to corner speed
//exit manoeuvre when unfriendly goes behind or time out

//treat the same as lagroll
	HighYoYo();

}

//------------------------------------------------------------------------------
//Procedure		ExtensionManoeuvre
//Author		R. Hyde 
//Date			Thu 5 Mar 1998
//
//Description	1.23
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::ExtensionManoeuvre()
{
//attempt to get to extend range to ENGAGERANGE safely
//this is to try to escape
//or get distance to try again
//if at same level
//	good mig pilots should go up
//	good us pilots should go down
//if higher than bogey
//	
//
//ref1.70
//## if turning good pilots will initially unload and
//retain roll to disguise manoeuvre 
//
//initially aircraft could be closing

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;


	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
			if ( (World.Y - ai.unfriendly->World.Y) > FT_1000)
			{//above him
				ai.desiredpitch = ANGLES_0Deg;
			}else if ( (ai.unfriendly->World.Y - World.Y) > FT_1000)
			{//below him
				if (classtype->phrasename == PHRASE_MIGS)
					ai.desiredpitch = ANGLES_0Deg;
				else
					ai.desiredpitch = ANGLES_350Deg;
			}else
			{
				if (classtype->phrasename == PHRASE_MIGS)
					ai.desiredpitch = ANGLES_10Deg;
				else
					ai.desiredpitch = ANGLES_350Deg;
			}

			SetManoeuvreTime (2000);
			ai.PilotSkills.ManStep = PHASE1;
			break;
		}
		case PHASE1:
		{
//			ACM.Pitch (ai.desiredpitch);
			ACM.MinHeight (FT_1000);
			MoveToHdg (HdgIntercept + ANGLES_180Deg,ai.desiredpitch);
 			if  (		(Range > ENGAGERANGE)
					||	(!ManoeuvreTimeLeft())
				)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}
	}


}
void	AirStruc::DivingSpin()
{
		ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
 	SWord 	deltaroll;
 	UWord	deltaroll_pos;
 	SWord	deltaroll_sign;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	Float fpitch, fhdg;
	UByte	FF = FlyingFactor();

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;

			
			ACM.Thrust (ThrustfromFF());
			COORDS3D	target;
			target = unfriendly->World;
			ANGLES reqroll = (roll + GetRequiredRoll(*(rotitem*)this, (COORDS3D)target, fpitch,fhdg ));
			if (	(reqroll >> ANGLES_120Deg)
				&&	(reqroll << ANGLES_240Deg)
				)
				ai.desiredroll = reqroll;
			else
				ai.desiredroll = ANGLES_180Deg;
			SetManoeuvreTime (1000);

			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{ //get upside down or therabouts

			ACM.FastRoll (ai.desiredroll);
			deltaroll = (SWord) (ai.desiredroll - roll);
			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
			if  (		( roll >> ANGLES_90Deg)
					&&	(roll << ANGLES_270Deg)
					&&	(deltaroll_pos <ANGLES_30Deg)
				)
				ACM.LiftLimit();
			else
				ACM.Pitch (ANGLES_0Deg);

			if (deltaroll_pos <ANGLES_20Deg)
			{
				if (deltaroll >= 0 )
					ai.desiredroll = (Angles)1;
				else
					ai.desiredroll = (Angles)-1;

				ai.PilotSkills.ManStep = PHASE2;
				SetManoeuvreTime (1000);
			}
			break;
		}
 		case PHASE2:
		{// pull and roll until pointing down
			ACM.LiftLimit();

			if ((SWord)ai.desiredroll >= 0)
				ACM.RollRate (400 * FF/256);		//deg/cs gets converted to rad/cs

			else
				ACM.RollRate (-400 * FF/256);

			if  ( 		(!ManoeuvreTimeLeft())
					||  (DesiredPitchChk (ANGLES_270Deg,ANGLES_45Deg))
				)
	 			{
					SetManoeuvreTime (1000);
					ai.PilotSkills.ManStep = PHASE3;
 	 			}
	 			break;
		}
 		case PHASE3:
		{//pull and roll until clear or time up
			ACM.LiftLimit();

			if ((SWord)ai.desiredroll >= 0)
				ACM.RollRate (400 * FF/256);		//rad/cs
			else
				ACM.RollRate (-400 * FF/256);
			if  ( 				(!ManoeuvreTimeLeft())
							||	(Range > ENGAGERANGE)							
					
				)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

 			break;
		}
	}

}
//------------------------------------------------------------------------------
//Procedure		ReverseTurn
//Author		R. Hyde 
//Date			Thu 5 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::ReverseTurn()
{
//	DesiredRoll = opposite of existing
//	Move to desired and hold for 3 secs
//	if (good)
//		out of plane
//	Close brakes if open at end of manoeuvre(used in selfasbait())
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());

			SetManoeuvreTime (600);
			ai.desiredroll = -roll;

			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			SWord	droll_sign, droll;
			UWord	droll_pos;

			ACM.Roll (ai.desiredroll);
			ACM.Pitch (PitchIntercept);
//			ACM.MinHeight (FT_2000);
			droll = ai.desiredroll - roll;
			Math_Lib.Pos ((SWord)droll, droll_pos, droll_sign);

			if  (		
						(!ManoeuvreTimeLeft())
					||	(droll_pos < ANGLES_5Deg)
				)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

		}
	}


}

//------------------------------------------------------------------------------
//Procedure		SelfAsBait
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	Ref19
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SelfAsBait()
{
//
//this could be where bandit is on your tail, wait for him to get close 
//before turning
//keep buddy in combat spread so that you can sandwich him

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	SWord	reqdeltahdg;
	reqdeltahdg = 	hdg - HdgIntercept;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (50);
			SetManoeuvreTime (1500);
			if (roll << ANGLES_90Deg)
				ai.desiredroll = ANGLES_30Deg;
			else
				ai.desiredroll = ANGLES_330Deg;

			ai.PilotSkills.ManStep = PHASE1;

		}
		case PHASE1:
		{
			ACM.Roll (ai.desiredroll);
			if (ChkAtDesiredRoll(ai.desiredroll, ANGLES_5Deg))
				ACM.Pitch (ANGLES_10Deg);
			ACM.MinHeight (FT_2000);

			if  (		(Range < WEAPONSRANGE)
					||	(!ManoeuvreTimeLeft())
				)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
				ai.PilotSkills.ManStep = PHASE0;
			}


		}
	}

}
//------------------------------------------------------------------------------
//Procedure		Jink
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::Jink()
{
//
//fast 90 deg roll
//hard push or pull on stick
//hold for 2-3 secs
//
//flyingability determines rollrate and stick push pull
//combatablity determines length of manoeuvre
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{
 			SetManoeuvreTime (500);
 			ai.desiredhdg = (Angles)(1 + Math_Lib.rnd(2));
 			ai.PilotSkills.ManStep = PHASE1;
 			if (MOD.Aileron >= 0)
			{
				ai.desiredroll = ANGLES_90Deg;
				ACM.SetAileron(32767);
			}
			else
			{
				ai.desiredroll = ANGLES_270Deg;
 				ACM.SetAileron(-32767);
			}
 		}
 		case PHASE1:
 		{
			SWord 	deltaroll;
			UWord	deltaroll_pos;
		 	SWord	deltaroll_sign;

			deltaroll = (SWord) (ai.desiredroll - roll);
			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
			if (deltaroll_pos <ANGLES_10Deg)
				ACM.LiftLimit();
			ACM.FastRoll(ai.desiredroll);	//CSB 21/03/99	
 			if (!ManoeuvreTimeLeft())
 			{
				ai.desiredhdg--;
				if (((SWord)ai.desiredhdg) <0)
 					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
				else
				{
					if (Math_Lib.rnd(2) == 0)
						ai.desiredroll = (Angles)(ANGLES_50Deg + Math_Lib.rnd((SWord)(ANGLES_45Deg)));
					else	
						ai.desiredroll = (Angles)(ANGLES_260Deg+ Math_Lib.rnd((SWord)(ANGLES_45Deg)));
		 			SetManoeuvreTime (200 + Math_Lib.rnd(300));
				}
 			}
 			break;
 		}
 	}


}
//------------------------------------------------------------------------------
//Procedure		BreakTurn
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	
//				Maximum performance turn towards unfriendly
//				Aim for corner speed (see summary at top of acmman.cpp)
//				When quoted as IAS, corner speed is practically a constant
//				cornerspeed should be part of the planetype
//				corner speed occurs at max g allowed for ac
//				max g should be part of the planetype 
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::BreakTurn()
{
//	roll = cos -1 (1/maxg)
//ability to reach roll = f(flyingskill)
//ability to hold corner vel = f(flyingskill)
//	option to nose down to keep speed up and nose up to reduce speed
//hold for 4-5 secs
//
//low morale gives a very lazy turn
//
	SWord	reqdeltahdg;
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	reqdeltahdg = 	hdg - HdgIntercept;


	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());

			SetManoeuvreTime (200);
			RndVal	rndnum = Math_Lib.rnd();
			if (FlyingFactor() > 120)
			{
				if (reqdeltahdg < 0)
					ai.desiredroll = ANGLES_85Deg;
				else
					ai.desiredroll = ANGLES_275Deg;
			}else
			{
				if (rndnum > RND50PC)
					ai.desiredroll = ANGLES_60Deg;
				else
					ai.desiredroll = ANGLES_300Deg;
			}
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{

			ACM.Roll (ai.desiredroll);

		 	SWord 	deltaroll;
		 	UWord	deltaroll_pos;
		 	SWord	deltaroll_sign;
 			deltaroll = (SWord) (ai.desiredroll - roll);
 			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
 			if (deltaroll_pos <ANGLES_5Deg)
				ACM.LiftLimit();
//			ACM.MinHeight (FT_2000);

			if  (		(!ManoeuvreTimeLeft())
				)
				if (	(classtype->phrasename == PHRASE_MIGS)
						&&	(fly.cpitch >> ANGLES_180Deg)
					)
				{
					ai.PilotSkills.ManStep = PHASE2;
					SetManoeuvreTime (600);

				}else
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;

		}
		case PHASE2:
		{
			COORDS3D	target = ai.unfriendly->World;
			TurnFightPhase1(HdgIntercept, TRUE, ANGLES_0Deg, target);
			if  (		(!ManoeuvreTimeLeft())
				)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

		}

	}

}
//------------------------------------------------------------------------------
//Procedure		LazyTurn
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::LazyTurn()
{
//
//	Choose a roll 10-40deg
//		= f(morale)
//	continue on turn for 20s
//	if (ai.morale > MORALE_GOOD)
//		if (Range < WEAPONS)
//			end manoeuvre
//
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (80);

			SetManoeuvreTime (2000);
   			RndVal	rndnum = Math_Lib.rnd();
			if (rndnum > RND50PC)
				ai.desiredroll = ANGLES_60Deg;
			else
				ai.desiredroll = ANGLES_300Deg;

			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			ACM.Roll (ai.desiredroll);
			SWord pitch = PitchIntercept;
			if (pitch > ANGLES_10Deg)
				pitch = ANGLES_10Deg;
			if (pitch < ANGLES_2Deg)
				pitch = ANGLES_2Deg;
			ACM.Pitch (pitch);
			ACM.MinHeight (FT_2000);
			if  (		(!ManoeuvreTimeLeft())
					||	(		(Range < WEAPONSRANGE)
							&&	(ai.morale > MORALE_GOOD)
						)
				)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

		}
	}

}
//------------------------------------------------------------------------------
//Procedure		Break180
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	1.26
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::Break180()
{
//roll 180
//pull hard until pitch 45 deg down
//slowly unroll for x secs
		ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
 	SWord 	deltaroll;
 	UWord	deltaroll_pos;
 	SWord	deltaroll_sign;
	AirStrucPtr	unfriendly = *ai.unfriendly;


	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			ACM.Thrust (ThrustfromFF());
			ai.desiredroll = ANGLES_180Deg;
			SetManoeuvreTime (1000);

			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{

			ACM.Roll (ai.desiredroll);
			deltaroll = (SWord) (ai.desiredroll - roll);
			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
			if (deltaroll_pos <ANGLES_10Deg)
			{
				ai.PilotSkills.ManStep = PHASE2;
				SetManoeuvreTime (1500);
			}
			break;
		}
 		case PHASE2:
		{
			MOD.Aileron = 0;
			ACM.LiftLimit();
			if  ( 		(!ManoeuvreTimeLeft())
					||  (		(DesiredPitchChk (ANGLES_270Deg,ANGLES_10Deg))
						)
				)
	 			{
 					ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
					ai.PilotSkills.ManStep = PHASE0;
	 			}
	 			break;
		}
	}

}
//------------------------------------------------------------------------------
//Procedure		BreakLow
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::BreakLow()
{
//decide which way to roll to go down
//roll 90
//pull hard until pitch 45 deg down
 	SWord 	deltaroll;
 	UWord	deltaroll_pos;
 	SWord	deltaroll_sign;
 	MODEL& MOD = *fly.pModel;
	ACMMODEL& ACM = *fly.pAcmModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;


 	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{
			MOD.Rudder = 0;
 			MoveToThrust (100);
			if ((hdg - HdgIntercept) > 0)
					ai.desiredroll = ANGLES_225Deg;
				else
					ai.desiredroll = ANGLES_135Deg;
			ai.PilotSkills.ManStep = PHASE1;
 			break;	
 		}
 		case PHASE1:
 		{	
			ACM.Roll (ai.desiredroll);
 			ACM.LiftLimit();
			UByte	FF = FlyingFactor();
			FP aoa0 = AngleSign (MOD.MainPlaneList->aoa[0][0]) + AngleSign (MOD.MainPlaneList->aoa[0][1]);
			FP aoa1 = AngleSign (MOD.MainPlaneList->aoa[1][0]) + AngleSign (MOD.MainPlaneList->aoa[1][1]);
			aoa0 *= 0.5;
			aoa1 *= 0.5;
			FP maxaoa = MOD.MainPlaneList->AoaMax - 	((256 - FF) * 0.174)/256;	//* 10 degs
			if (	(aoa0 > maxaoa)
				||	(aoa1 > maxaoa)
				)
 				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
 			deltaroll = (SWord) (ai.desiredroll - roll);
 			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
 			if (deltaroll_pos <ANGLES_5Deg)
			{
				ai.PilotSkills.ManStep = PHASE2;
				SetManoeuvreTime (1000);
			}
			break;
		}
		case PHASE2:
		{
			ACM.LiftLimit();
 			if  (	(	(roll >> ANGLES_90Deg)
					&&	(roll << ANGLES_270Deg)
					)
					||
					(MOD.Inst.MachNo < 0.35)
				)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			if (!ManoeuvreTimeLeft ())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

			break;
		}
 	}

}
//------------------------------------------------------------------------------
//Procedure		BreakHigh
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::BreakHigh()
{
//decide which way to roll to go up
//roll 90
//pull hard until pitch 45 deg up
 	SWord 	deltaroll;
 	UWord	deltaroll_pos;
 	SWord	deltaroll_sign;
 	MODEL& MOD = *fly.pModel;
	ACMMODEL& ACM = *fly.pAcmModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;


 	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{
			MOD.Rudder = 0;
 			MoveToThrust (100);
			if ((hdg - HdgIntercept) > 0)
					ai.desiredroll = ANGLES_315Deg;
				else
					ai.desiredroll = ANGLES_45Deg;
			ai.PilotSkills.ManStep = PHASE1;
 			break;	
 		}
 		case PHASE1:
 		{	
			ACM.Roll (ai.desiredroll);
 			ACM.LiftLimit();
			UByte	FF = FlyingFactor();
			FP aoa0 = AngleSign (MOD.MainPlaneList->aoa[0][0]) + AngleSign (MOD.MainPlaneList->aoa[0][1]);
			FP aoa1 = AngleSign (MOD.MainPlaneList->aoa[1][0]) + AngleSign (MOD.MainPlaneList->aoa[1][1]);
			aoa0 *= 0.5;
			aoa1 *= 0.5;
			FP maxaoa = MOD.MainPlaneList->AoaMax - 	((256 - FF) * 0.174)/256;	//* 10 degs
			if (	(aoa0 > maxaoa)
				||	(aoa1 > maxaoa)
				)
 				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
 			deltaroll = (SWord) (ai.desiredroll - roll);
 			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
 			if (deltaroll_pos <ANGLES_5Deg)
			{
				ai.PilotSkills.ManStep = PHASE2;
				SetManoeuvreTime (1000);
			}
			break;
		}
		case PHASE2:
		{
			ACM.LiftLimit();
 			if  (	(	(roll >> ANGLES_90Deg)
					&&	(roll << ANGLES_270Deg)
					)
					||
					(MOD.Inst.MachNo < 0.35)
				)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			if (!ManoeuvreTimeLeft ())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

			break;
		}
 	}

}

//------------------------------------------------------------------------------
//Procedure		Break90
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	1.27
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::Break90()
{
	if (classtype->phrasename == PHRASE_MIGS)
		BreakHigh();
	else if (	(ai.morale > MORALE_MEDIUM)
			&&	(ClosureLessThan(MPH50))
			)
		{
				ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKHIGH;
				BreakHigh();
		}else
		{
				ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKLOW;
				BreakLow();
		}
}

//------------------------------------------------------------------------------
//Procedure		HiGBarrelRoll
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	1.30	
//				Defensive manoeuvre, used when attacker has high overtake and
//				is close range
//				Method is to pull max g and roll rapidly
//				Supposed to inscribe circle around attacker's flight path
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::HiGBarrelRoll()
{
//max g, roll in one direction
//nose low or high
//unbalanced flight, power reduction, increased drag for better pilots
//
//
//treat the same as lagroll
	HighYoYo();

}
//------------------------------------------------------------------------------
//Procedure		UnBalancedFlight
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::UnBalancedFlight()
{
//skid and slip while making turns
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

 	switch (ai.PilotSkills.ManStep)
 	{
 		case PHASE0:
 		{
 			SetManoeuvreTime (200);
			if  (roll << ANGLES_20Deg)
				ai.desiredhdg = hdg + ANGLES_60Deg;
			else if  (roll >> ANGLES_340Deg)
				ai.desiredhdg = hdg - ANGLES_60Deg;
			else if ( roll == ANGLES_0Deg)
			{
				RndVal	rndnum = Math_Lib.rnd();
				if (rndnum > RND50PC)
					ai.desiredhdg = hdg + ANGLES_60Deg;
				else
					ai.desiredhdg = hdg - ANGLES_60Deg;
			}else
 				ai.desiredhdg = hdg;
			ai.desiredroll = (Angles)(2 + Math_Lib.rnd(2));
 			ai.PilotSkills.ManStep = PHASE1;
 			if (MOD.Aileron >= 0)
 			{
 				ACM.SetRudder(MAXRUDDER);
 			}else
 			{
 				ACM.SetRudder(-MAXRUDDER);
 			}
			RndVal	rndnum = Math_Lib.rnd();
			if (rndnum > RND50PC)
				ai.desiredpitch = ANGLES_5Deg;
			else
				ai.desiredpitch = ANGLES_355Deg;
 			
 		}
 		case PHASE1:
 		{
// 			ACM.Pitch(ai.desiredpitch);
 			SWord TempCtrlRudder = ACM.CtrlRudder;
			MoveToHdg (ai.desiredhdg, ai.desiredpitch);
			ACM.CtrlRudder = TempCtrlRudder;
 			if (!ManoeuvreTimeLeft())
 			{
				ai.desiredroll--;
				if (((SWord)ai.desiredroll) < 0)
				{
	 				MOD.Rudder = 0;
					ACM.CtrlRudder = 0;
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
 
				}else
				{
					if (MOD.Rudder > 0)
		 				ACM.SetRudder(-MAXRUDDER);
 					else
		 				ACM.SetRudder(MAXRUDDER);
					RndVal	rndnum = Math_Lib.rnd();
					if (rndnum > RND50PC)
						ai.desiredpitch = ANGLES_5Deg;
					else
						ai.desiredpitch = ANGLES_355Deg;
		 			SetManoeuvreTime (200);
				}
 			}
 			break;
 		}
 	}

}
//------------------------------------------------------------------------------
//Procedure		TrackingShot
//Author		R. Hyde 
//Date			Tue 10 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::LowAlt()
{
	SWord	reqdeltahdg = 	hdg - HdgIntercept;	
	SWord dpitch;
 	ACMMODEL& ACM = *fly.pAcmModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			ACM.Thrust (ThrustfromFF());

			SetManoeuvreTime (800);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			if (TargetFacingSubject(ANGLES_45Deg))
				dpitch = ANGLES_5Deg;
			else
				dpitch = ANGLES_10Deg;
			ACM.Pitch(dpitch);
			ACM.Roll(ANGLES_0Deg);

			if  (!ManoeuvreTimeLeft())
							ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

					

		}
	}

}
//------------------------------------------------------------------------------
//Procedure		SnapShot
//Author		R. Hyde 
//Date			Tue 10 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SnapShot()
{
//
//similar to turningfight 21/5/98
// but don't slow down
//
	SWord	reqdeltahdg,reqdeltahdg_sign;
	UWord	reqdeltahdg_pos;
	COORDS3D	target;
	SLong		time;
	MODEL& MOD = *fly.pModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;

	reqdeltahdg = 	Rads2Rowan(MOD.fHdg) - HdgIntercept;
	Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);

	if (		(Range < DANGERRANGE)
			&&	(reqdeltahdg_pos < (SWord)ANGLES_30Deg)
		)		
	{
		//this method just extrapolates position
		//to get turn:
		//get distance travelled and turn in time to work out offset
		time = Range / classtype->muzzlevel;		//MUZZELVEL;
		//calc lead position using this time
		target.X = ai.unfriendly->World.X + (unfriendly->velx * time );
		target.Y = ai.unfriendly->World.Y + (unfriendly->vely * time );
		target.Z = ai.unfriendly->World.Z + (unfriendly->velz * time );
		InterceptandRange (&target);
	}else
		target = ai.unfriendly->World;

	if (Range < THRUST_0_RANGE)
	{
		ai.PilotSkills.ManStep = PHASE0;
		ai.PilotSkills.manoeuvre=MANOEUVRE_ZOOM;
	}
	else
		TurnFightPhase1(HdgIntercept, FALSE, ANGLES_0Deg, target);

}
//------------------------------------------------------------------------------
//Procedure		StayOnTail
//Author		R. Hyde 
//Date			Fri 20 Mar 1998
//
//Description	This is not a good idea, very aggressive
//				Stay at corner speed or less
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::StayOnTail()
{
//	if necessary shed energy:
//		turn
//		brakes
//		throttle
//	to stay on tail
//
//same as turningfight 21/5/98
//
	SWord	reqdeltahdg,reqdeltahdg_sign;
	UWord	reqdeltahdg_pos;
	COORDS3D	target;
	MODEL& MOD = *fly.pModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	SLong vx = 0, vy = 0, vz = 0;

	reqdeltahdg = 	hdg - HdgIntercept;
	Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);

	if (	(reqdeltahdg_pos < (SWord)ANGLES_90Deg)
		)		
	{
		CalcLead(*this, *ai.unfriendly,vx,vy,vz, TRUE);

		target.X = ai.unfriendly->World.X + vx;
		target.Y = ai.unfriendly->World.Y + vy;
		target.Z = ai.unfriendly->World.Z + vz;
		InterceptandRange (&target);
	}else
		target = ai.unfriendly->World;

	TurnWithPitchChange(HdgIntercept, TRUE, target);

}
//------------------------------------------------------------------------------
//Procedure		TurnToHdgAndPitch
//Author		R. Hyde 
//Date			Tue 17 Mar 1998
//
//Description	move to within 5 deg of desiredpitch and desiredhdg 
//				break or sustained turn depending on circumstances
//				if break then nose down if <corner vel
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::TurnToHdgAndPitch()
{
	//return true on success
	return(TRUE);
}

void		AirStruc::CalcDesiredPitchRate(SWord	reqdeltapitch, SWord	reqdeltapitch_sign, UWord	reqdeltapitch_pos)
{
	MODEL& MOD = *fly.pModel;
	SWord dpitch;

	ai.desiredpitchrate = AngleSign (Rowan2Rads(PitchIntercept - (SWord)ai.oldoldoldpitchI));
	ai.desiredpitchrate = ai.desiredpitchrate /(3 * MOD.MODEL_DT);


	FP pitchleadmodifier = 0.0025;
	if (reqdeltapitch_sign >= 0 )
		pitchleadmodifier = 0.1;
	if (reqdeltapitch_pos > ANGLES_70Deg)
		dpitch = ANGLES_70Deg;
	else
		dpitch = reqdeltapitch_pos;
	if (reqdeltapitch_sign < 0)
		dpitch = - dpitch;
	ai.deltapitchsum += dpitch;
	SLong IntPitchForMaxDP = 5000000;
	if (ai.deltapitchsum > IntPitchForMaxDP)
		ai.deltapitchsum = IntPitchForMaxDP;
	else if (ai.deltapitchsum < -IntPitchForMaxDP)
		ai.deltapitchsum = - IntPitchForMaxDP;

	ai.desiredpitchrate = ai.desiredpitchrate + (pitchleadmodifier * dpitch/ANGLES_70Deg);
 	ai.desiredpitchrate = ai.desiredpitchrate + pitchleadmodifier * ai.deltapitchsum / (4*IntPitchForMaxDP); 
}



FP	AirStruc::FineRollAdjust(SWord reqdeltapitch,SWord deltahdg,FP rroll, FP fpitch, FP fhdg)
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	SWord	reqdeltapitch_sign;
	UWord	reqdeltapitch_pos;
	FP altmod;
	FP deltahdgmod = 0.0005;
	SWord	deltahdg_sign, dhdg, dpitch;
	UWord	deltahdg_pos;
	FP	desiredturn;
	
	ACM.maxdeltaforclosework = 0.2;

//Global axis
	{
		
		FP AveHeadgRate = AngleSign (Rowan2Rads(HdgIntercept   - (SWord)ai.oldoldoldhdgI  )) / (3 * MOD.MODEL_DT);
		FP AvePitchRate = AngleSign (Rowan2Rads(PitchIntercept - (SWord)ai.oldoldoldpitchI)) / (3 * MOD.MODEL_DT);
		rroll			= FPATan(MOD.Speed * AveHeadgRate, (MOD.Speed * AvePitchRate + GRAVITY));// * cospitch / 32768);
		FP DeltaRroll = 0;
		if(MOD.RotVel.x > 0)
			DeltaRroll = 0.01 * fhdg / MOD.RotVel.x;			
		MODLIMIT(DeltaRroll, 0.25);
			rroll += DeltaRroll;
	}
	rroll = AngleSign(rroll);
#ifdef PRINT_AI_DATA
	PrintVar(50, 18, "Rroll     %.1f ", (FP)(57.3 * rroll));	
	PrintVar(50, 19, "RPitch    %.1f ", (FP)(57.3 * fpitch));	
#endif
	{
#ifdef PRINT_AI_DATA
	PrintString(0, 20, "FINE ROLL ADJUST");
#endif

		ACM.TurnRate(fpitch);
	}

	return(rroll);
}
//}
FP	AirStruc::CoarseRollAdjust(FP rroll, FP fpitch)
{
#ifdef PRINT_AI_DATA
	PrintString(0, 19, "Coarse Roll Adjust");
#endif

	MODEL& MOD = *fly.pModel;
	ACMMODEL& ACM = *fly.pAcmModel;

	ACM.maxdeltaforclosework = 0.1;
	MOD.Rudder = 0;//MOD.Rudder/2;
	ACM.deltayawsum = 0;//ACM.deltayawsum/2;

	if((fpitch > 0) && (ACM.deltapitchsum < 0)) ACM.deltapitchsum = 0;
	if((fpitch < 0) && (ACM.deltapitchsum > 0)) ACM.deltapitchsum = 0;

	rroll = AngleSign(rroll);
	FP AbsRroll;
	if(rroll > 0)	AbsRroll =  rroll;
	else			AbsRroll = -rroll;

	if((AbsRroll < FPIE / 6) || (AbsRroll > (1 - 1/6) * FPIE))	ACM.TurnRate(fpitch);	//CSB 09/03/99
	else ACM.TurnRate(0);

	rroll += Rowan2Rads(roll);
	rroll = AngleSign(rroll);

	return(rroll);
}


Bool	AirStruc::UnfriendlyOnSameRoll()
{
	MODEL& MOD = *fly.pModel;

	AirStrucPtr	unfriendly = *ai.unfriendly;
	if (	(		(roll << ANGLES_180Deg)
				&&	(unfriendly->roll << ANGLES_180Deg)			
			)
			||
			(		(roll >> ANGLES_180Deg)
				&&	(unfriendly->roll >> ANGLES_180Deg)			
			)
		)
		return (TRUE);
	else
		return(FALSE);


}
SWord	AirStruc::RollforTurn(SWord reqdeltapitch,SWord deltahdg, Float& desiredturn,COORDS3D	target)
{


	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
		
		if(ACM.maxdeltaforclosework < 0.1)
		ACM.maxdeltaforclosework = 0.1;
#ifdef PRINT_AI_DATA
	PrintVar(15, 20, "MaxDelta %.1f ", ACM.maxdeltaforclosework);
#endif

//find roll which would put target on lift line
	FP rroll;
	Float fpitch,fhdg;

	FP gdpitch =  AngleSign( Rowan2Rads(reqdeltapitch) );
	FP gdhdg   = -AngleSign( Rowan2Rads(deltahdg) );
	SWord sinroll, cosroll;
	Math_Lib.high_sin_cos(roll, sinroll, cosroll);
	fpitch = gdpitch * cosroll / 32768 + gdhdg   * sinroll / 32768;
	fhdg   = gdhdg   * cosroll / 32768 - gdpitch * sinroll / 32768;
	fpitch = AngleSign(fpitch);
	fhdg   = AngleSign(fhdg);
	ANGLES reqroll = Angles(Rads2Rowan(FPATan(fhdg, fpitch) ) );
	
#ifdef PRINT_AI_DATA
	PrintVar(35, 10, "ReqDPitch %.2f ", (FP)(fpitch * 57.3) );
	PrintVar(35, 11, "ReqDRoll  %.2f ", (FP)(reqroll / 182.04) );
	PrintVar(35, 12, "ReqDHeadg %.2f ", (FP)(fhdg * 57.3) );
	PrintVar(35, 13, "Range     %.2f ", (FP)(Range * 0.01) );
#endif

		FP rr = Rowan2Rads(reqroll);
		rroll =  AngleSign(rr);

	if(//(UnfriendlyOnSameRoll())
		//&&	
			(fpitch < ACM.maxdeltaforclosework)
		&&	(fpitch > -ACM.maxdeltaforclosework)
		&&	(fhdg < ACM.maxdeltaforclosework)
		&&	(fhdg > -ACM.maxdeltaforclosework)	)
	{
		rroll = FineRollAdjust(reqdeltapitch, deltahdg, rroll, fpitch, fhdg);
	}
	else
		rroll = CoarseRollAdjust(rroll, fpitch);

	return(Rads2Rowan (rroll));


}

//------------------------------------------------------------------------------
SWord	AirStruc::ThrustfromFF()
{
//simple thrust modified in CalcSimpleAccel
//complex model: instead of modifying thrust we rely on drag effect via reduced lift
 	return(100);
}

void	AirStruc::SpeedUp(FP velc)
{
	//Enter with velc negaticve, ie need to speed up
		FP time;
		MODEL& MOD = *fly.pModel;
   	SWord thrust;
   	SWord maxthrust = ThrustfromFF();
	thrust = maxthrust;
		ACMMODEL& ACM = *fly.pAcmModel;

	velc = -velc;
	time = (Range - IDEALCOMBATRANGE)/velc;				//cs
	if (time > 0)
	{
		FP accell = velc/time;							//cm/cs/cs
			//Told - Dold  = m* accelold
		//Tnew -Dnew = m * accelnew + m* accelold
		//assume Dold and Dnew are same
		//Tn = To + m(an)

//CSB 29Jun99 VVV
		FP fMass = weap.left.int_fuel + weap.centre.int_fuel + weap.right.int_fuel;
		if(Save_Data.flightdifficulty [FD_STORESWEIGHTDRAG])
		{
			fMass += weap.left.ext_fuel + weap.centre.ext_fuel + weap.right.ext_fuel;
			fMass += weap.left.mass     + weap.centre.mass     + weap.right.mass;
		}
		fMass = fMass * 0.001 + classtype->weightempty;
//CSB 29Jun99 ^^^
		FP deltaforce = (accell) * fMass;//N	//CSB 29Jun99
		FP Tn = MOD.EngineList->Thrust + deltaforce;
		thrust = 100 * Tn/MOD.EngineList->MaxThrust;
		if (thrust < 0)
		{
			if (thrust < - 300)
			{
				thrust = maxthrust;
				ACM.SpeedBrakesIn();
				if (Range < METRES150)
				{
					ai.PilotSkills.ManStep = PHASE0;
					if (SubjectLeadingTarget())
						ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_LAGROLL, MANOEUVRE_TURNINGFIGHT);
					else
						ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_LAGPURSUIT, MANOEUVRE_TURNINGFIGHT);
				}

			}else
			{
				thrust = 0;
				ACM.SpeedBrakesOut();
			}
		}else
			ACM.SpeedBrakesIn();
		if (thrust > maxthrust)
			thrust = maxthrust;
	}
	else
	{	//stay as you are if inside min range
		thrust = 100 * MOD.EngineList->Thrust/MOD.EngineList->MaxThrust;
	}
	ACM.Thrust (thrust);
}

void	AirStruc::SlowDown(FP velc)
{
	//Enter with velc positive, need to slow down
		FP time;
			MODEL& MOD = *fly.pModel;
		SWord thrust;
  	SWord maxthrust = ThrustfromFF();
		ACMMODEL& ACM = *fly.pAcmModel;
	thrust = maxthrust;


	if (velc != 0)
		time = (Range - IDEALCOMBATRANGE)/velc;				//cs
	else
		time = 100;
	if (time < 0)
	{//inside min range and need to low down
				if (Range < METRES150)
				{
					thrust = maxthrust;
					ACM.SpeedBrakesIn();
					ai.PilotSkills.ManStep = PHASE0;
					if (SubjectLeadingTarget())
						ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_LAGROLL, MANOEUVRE_ZOOM);
					else
						ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_LAGPURSUIT, MANOEUVRE_ZOOM);
				}else
				{
					thrust = 0;
					ACM.SpeedBrakesOut();
				}
	}
	else
	{
		FP decell = -velc/time;							//cm/cs/cs
			//Told - Dold  = m* accelold
		//Tnew -Dnew = m * accelnew
		//assume Dold and Dnew are same
		//Tn = To + m(an-ao)

//CSB 29Jun99 VVV
		FP fMass = weap.left.int_fuel + weap.centre.int_fuel + weap.right.int_fuel;
		if(Save_Data.flightdifficulty [FD_STORESWEIGHTDRAG])
		{
			fMass += weap.left.ext_fuel + weap.centre.ext_fuel + weap.right.ext_fuel;
			fMass += weap.left.mass     + weap.centre.mass     + weap.right.mass;
		}
		fMass = fMass * 0.001 + classtype->weightempty;
//CSB 29Jun99 ^^^
		FP deltaforce = (decell) * fMass;//N	   //CSB 29Jun99
		FP Tn = MOD.EngineList->Thrust + deltaforce;
		thrust = 100 * Tn/MOD.EngineList->MaxThrust;
		if (thrust < 0)
		{
			if  (	(thrust < - 300)
				&&	(Range < METRES150)
				)
			{
				thrust = maxthrust;
				ACM.SpeedBrakesIn();
				ai.PilotSkills.ManStep = PHASE0;
				if (SubjectLeadingTarget())
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_LAGPURSUIT, MANOEUVRE_TURNINGFIGHT);
				else
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_LAGPURSUIT, MANOEUVRE_TURNINGFIGHT);
			}else
			{
				thrust = 0;
				ACM.SpeedBrakesOut();
			}
		}else
			ACM.SpeedBrakesIn();
		if (thrust > maxthrust)
			thrust = maxthrust;
	}
	ACM.Thrust (thrust);
}



void	AirStruc::SlowDownCheck(UWord reqdeltahdg_pos)
{
	SWord thrust = 100;
	MODEL& MOD = *fly.pModel;
	ACMMODEL& ACM = *fly.pAcmModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;

 	if((!SubjectFacingTarget(ANGLES_45Deg)) || (!SubjectOnTargetTail(ANGLES_45Deg)))
		thrust = ThrustfromFF();
	else
	{
		SLong ReqVel = 0;
		SWord MaxAcc = 0;
		Bool TooClose = FALSE;
		if(Range < IDEALCOMBATRANGE) TooClose = TRUE;

		if(TooClose)
			MaxAcc = -CalcAccel(32000);
		else
			MaxAcc = -CalcAccel(0);	//Ensure requesting decel not accel
 		MaxAcc = MaxAcc / 2;

		if(MaxAcc == 0)
		{
			if(TooClose)
				thrust = 0;
			else
				thrust = ThrustfromFF();
		}
		else
		{
			FP velfactor = ((FP)MaxAcc * ((FP)Range - (FP)IDEALCOMBATRANGE)) * (2.0 / 2560.0);
			if (velfactor > 0)
			{
				if(Range > IDEALCOMBATRANGE)
					ReqVel = unfriendly->vel + FSqrt(velfactor);
				else
					ReqVel = unfriendly->vel - FSqrt(velfactor);
			}
			else
				ReqVel = unfriendly->vel;

#ifdef PRINT_AI_DATA
PrintVar(40, 2, "Unf Vel %.1f ", (FP)(unfriendly->vel * 0.1));
PrintVar(40, 3, "Req Vel %.1f ", (FP)(ReqVel * 0.1));
PrintVar(40, 4, "Act Vel %.1f ", (FP)(vel * 0.1));
PrintVar(40, 5, "Throttl %.0f ", (FP)(fly.thrustpercent));
#endif

			if(ReqVel > vel)						//CSB 27/05/99	
			{
				thrust = ThrustfromFF();	//CSB 27/05/99	
				if(TooClose) 
		 		   ACM.SpeedBrakesIn();
			}
			else									//CSB 27/05/99	
			{
				thrust = 0;				//CSB 27/05/99	
				if(Range < IDEALCOMBATRANGE / 2) 
		 		   ACM.SpeedBrakesOut();
			}
		}
	}

	ACM.Thrust (thrust);
}

		
void	AirStruc::TurnFightTargetNotOnNose(SWord	reqdeltahdg, SWord	reqdeltapitch, SWord	reqdeltapitch_sign, UWord	reqdeltapitch_pos, ANGLES	deltapitch)
{


//roll = arccos (1/(g * cos pitch)
//arccos x = arctan (sqr(1-x^2)/x^2)

 	MODEL& MOD = *fly.pModel;
	ACMMODEL& ACM = *fly.pAcmModel;
	SWord delta;
	SWord dpitch;

//	if (reqdeltahdg > 0)
//		reqroll = -1;
//	else
//		reqroll = 1;
	if (reqdeltapitch_pos > ANGLES_10Deg)
		dpitch = ANGLES_10Deg;
	else
		dpitch = reqdeltapitch_pos;
	if (reqdeltapitch_sign < 0)
		dpitch = - dpitch;
	RollAndPull(reqdeltahdg,dpitch);

	
	
	
}
void	AirStruc::RollAndPull(SWord	reqdeltahdg, SWord dpitch)
{
	MODEL& MOD = *fly.pModel;
	ACMMODEL& ACM = *fly.pAcmModel;
	
	SWord reqroll;

	if (reqdeltahdg > 0)
		reqroll = -1;
	else
		reqroll = 1;

	SWord c,s;
	Math_Lib.high_sin_cos(fly.cpitch,s,c);
	FP x, r;
	FP g = MOD.Inst.NormalAcc;
	if (g < 1)
		g = 1;
	FP	gmodifier = dpitch;
		gmodifier = 1 + gmodifier/ANGLES_10Deg;
	x = gmodifier/(g * c / 32768);
	if (x > 1)
		x = 1;
	if (x < -1)
		x = -1;
	x = FSqrt((1-x*x)/(x*x));
	r = FPATan (x,1);
	if (r < 0.5)
		r = 0.5;

	if (reqroll > 0)
		reqroll = Rads2Rowan(r);
	else
		reqroll = -Rads2Rowan(r);

#ifdef PRINT_AI_DATA
	PrintVar(60, 11, "R Roll %.1f", (FP)(reqroll/182.04));
#endif

 	if (		(ai.PilotSkills.manoeuvre != MANOEUVRE_SPLITMANOEUVRE)
			&&	(BanditOnColdSide())
		)
	//smart fighter realises that bandit has gone cold side in a turning fight: Don't reverse
 		if (	(Range < DANGERRANGE)	&& (ai.PilotSkills.combatskill > SKILL_REGULAR))
 				reqroll = -reqroll;

	ACM.Roll ((Angles)reqroll);
	ACM.ZeroSpecificPower();
}
//------------------------------------------------------------------------------
//Procedure		TurnFightSub
//Author		R. Hyde 
//Date			Fri 13 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::TurnFightSub(ANGLES	HdgIntercept, Bool sitbehindtarget, ANGLES	deltapitch,COORDS3D	target)
{
	ACMMODEL& ACM = *fly.pAcmModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			ACM.Clear ();
			SetManoeuvreTime (1000);
			ACM.maxdeltaforclosework = 0.0;
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			TurnFightPhase1(HdgIntercept, sitbehindtarget, deltapitch, target);
		}
	}

}

void	AirStruc::TurnFightPhase1(ANGLES	HdgIntercept, Bool sitbehindtarget, ANGLES	deltapitch,COORDS3D	target)
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;

	SLong temp_range;
	temp_range = Range; 	

	SWord	reqdeltahdg,reqdeltahdg_sign;
	UWord	reqdeltahdg_pos;
	SWord	reqdeltapitch,reqdeltapitch_sign;
	UWord	reqdeltapitch_pos;
	Float desiredturn;
	SLong		time;

		reqdeltahdg = 	hdg - HdgIntercept;		//Are You Sure ????	//CSB

//			ACM.MinHeight (FT_2000);
 			Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);
			reqdeltapitch = PitchIntercept - pitch;
			Math_Lib.Pos (reqdeltapitch,reqdeltapitch_pos,reqdeltapitch_sign);

			ANGLES delta = unfriendly->hdg + ANGLES_180Deg + ANGLES_30Deg - hdg;

			if	(SubjectFacingTarget(ANGLES_45Deg))
			{
				if	(	(delta << ANGLES_60Deg)
						&&	(Range > DANGERRANGE)
						&&	(reqdeltapitch_pos << ANGLES_15Deg)
						&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_DIVEANDZOOM)
					)
				{
						if  (		(FlyingFactor() < 160)
								||	(classtype->aerobaticfactor < AEROBATIC_FIGHTER)
							)
							ai.PilotSkills.manoeuvre = MANOEUVRE_HEADON;
						else
							ai.PilotSkills.manoeuvre = MANOEUVRE_HEADONOFFSET;
						ai.PilotSkills.ManStep = PHASE0;
				}
				if	(		( Range > WEAPONSRANGE)
						&&	(ai.PilotSkills.manoeuvre !=  MANOEUVRE_LOWYOYO)
						&&  (ai.PilotSkills.combatskill >= SKILL_VETERAN)
						&&	(ClosureLessThan(MPH15))
						&&	(SubjectOnTargetTail(ANGLES_45Deg))
						&&	(AboveGround(FT_1000))
						&&	((World.Y - ai.unfriendly->World.Y) < FT_100)
						&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_DIVEANDZOOM)
					)
				{
					ai.PilotSkills.ManStep = PHASE0;
					ai.PilotSkills.manoeuvre = MANOEUVRE_LOWYOYO;
				}
			}
			if  (		(PitchIntercept >> ANGLES_180Deg)
					&&	(PitchIntercept << ANGLES_340Deg)
					&&	((World.Y - ai.unfriendly->World.Y) > FT_3000)
					&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_DIVEANDZOOM)
				)
			{
				ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;
			}

#ifdef PRINT_AI_DATA
PrintString(0, 18, "                    ");	//CSB 05/03/99
PrintString(0, 19, "                    ");	//CSB 05/03/99
PrintString(0, 20, "                    ");	//CSB 05/03/99
PrintString(0, 21, "                    ");	//CSB 05/03/99
#endif

			{
				if (sitbehindtarget)
					SlowDownCheck(reqdeltahdg_pos);
	
				if  (		(		(reqdeltahdg_pos < ANGLES_40Deg)
							&&	(ACM.maxdeltaforclosework == 0.0)
							)
						||
							(		(reqdeltahdg_pos < ANGLES_45Deg)
								&&	(ACM.maxdeltaforclosework != 0.0)
							)
					)
				{
#ifdef PRINT_AI_DATA
	PrintString(0, 18, "Lift Line Calc");
#endif

					SWord reqroll = RollforTurn(reqdeltapitch, reqdeltahdg, desiredturn, target);
					ai.PilotSkills.flatturn = FALSE;
					ACM.Roll ((Angles)reqroll);
				}else
				{
					ACM.maxdeltaforclosework = 0.0;
					SWord rollpos = roll;
					if (rollpos < 0)
						rollpos = -rollpos;
					
//					FP	factor = 0.2 + 0.0000916 * (ANGLES_60Deg - rollpos);	//0.75 / ANGLES_60Deg
					{
						if (!ai.PilotSkills.flatturn) 
							fly.pAcmModel->ZeroIntControl();
						ai.PilotSkills.flatturn = TRUE;
						TurnFightTargetNotOnNose(reqdeltahdg, reqdeltapitch, reqdeltapitch_sign, reqdeltapitch_pos, deltapitch);
						ACM.SetRudder(0);
					}

				}
				if  (		(!ManoeuvreTimeLeft())
					)
				{
					
					if (		(Range > ENGAGERANGE)
							&&	(		(ai.firsttactic == TACTIC_LINEABREAST)
									||	(ai.firsttactic == TACTIC_LINEASTERN)
								)
						)
					{
						ai.PilotSkills.ManStep = PHASE0;
						if (ai.firsttactic == TACTIC_LINEASTERN)
							ai.PilotSkills.manoeuvre = MANOEUVRE_LINEASTERN;
						else
							ai.PilotSkills.manoeuvre = MANOEUVRE_LINEABREAST;
					}else
					{
						if	(ACM.maxdeltaforclosework > 0.1)
							ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
						else 
						{
							if (		(fly.leadflight)					
									&&	 (fly.leadflight->ai.PilotSkills.manoeuvre == MANOEUVRE_TOPCOVER)
								)//force manoeuvre to allow disengage
								ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
							else
							{//on tail of target so keep in manoeuvre and up morale

								AirStruc* unf			= (AirStruc*)ai.unfriendly;
								AirStruc* unfbuddy	= unf->FindBuddy();
								if (		(unfbuddy)
										&&	(unf == Persons2::PlayerGhostAC)
										&&	(Range > DANGERRANGE)
										&&	(Range < ENGAGERANGE)
									)
								{
								 	RndVal	rndpc = (RndVal)Math_Lib.rnd(RndValMAX);	
									if	((rndpc > RND50PC)	&&	(Range < VISIBLERANGE))
											_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_BANDIT_RELPOS, MSG_MIGSCOMING_RPT, unfbuddy, this, unf));
								}
								FormationInfo.advantage = TRUE;
								FormationInfo.disadvantage = FALSE;
								MoraleMod();
								SetManoeuvreTime (1000);
							}
						}
					}
				}
			}

}


Float	AirStruc::Model_InstTurnRate(AirStrucPtr subject)
{//return tr in rads per cs, dhdg in rowans
	FP tr, tr2;


	if (!subject)
		return 0.0;

	tr = subject->fly.dhdg * 0.00009587	/ Timer_Code.FRAMETIME;					

//	if (subject == Persons2::PlayerGhostAC)
//		subject = Manual_Pilot.ControlledAC2;
 //	tr = ((((SWord)subject->hdg)* 0.00009587) - 
 //				AngleSign (subject->fly.LastfHdg)) / Timer_Code.FRAMETIME;
//	FP test;
//	test = tr - tr2;
//	if (abs(test) > 0.0001)
//		test = -test ;

	return(tr);
}


void	AirStruc::CalcLead(AirStrucPtr  subject, AirStrucPtr target, SLong& deltax,SLong& deltay,SLong& deltaz, Bool cutcorner)
{
//	MODEL& TARGETMOD = *(target->fly).pModel;	
	Float time, shoottime, closuretime;
	Float dhdg;
	SWord c,s;
	SWord newhdg;
	SLong	closure;
	SLong dx,dy,dz;

	deltax = 0;
	deltay = 0;
	deltaz = 0;



//al times in 1/10 s
SLong	const	PUREPURSUITRANGE = METRES700;	
SLong	const	DLEADRANGE = PUREPURSUITRANGE - WEAPONSRANGE;
SLong	const	LAGRANGE = METRES2500;
SLong	const	DLAGRANGE = LAGRANGE - PUREPURSUITRANGE;
	FP targetvelx = target->velx;//10 * ((FP)target->World.X - (FP)ai.oldoldoldufposx) / (3 * (FP)Timer_Code.FRAMETIME);
	FP targetvely = target->vely;//10 * ((FP)target->World.Y - (FP)ai.oldoldoldufposy) / (3 * (FP)Timer_Code.FRAMETIME);
	FP targetvelz = target->velz;//10 * ((FP)target->World.Z - (FP)ai.oldoldoldufposz) / (3 * (FP)Timer_Code.FRAMETIME);
 	FP VelComp;
		VelComp = (subject->velx * targetvelx + subject->vely * targetvely + subject->velz * targetvelz) / subject->vel;
	shoottime = (FP)Range / ((FP)vel - VelComp + (FP)subject->classtype->muzzlevel);	//CSB 05/03/99
//	shoottime = (Range  + (shoottime *  target->vel))/ (subject->classtype->muzzlevel);		//MUZZELVEL;

	ANGLES VelHdg  = Math_Lib.HighArcTan((SLong)targetvelx, (SLong)targetvelz);
	FP	   VelHori = FSqrt(targetvelx * targetvelx + targetvelz * targetvelz);
	FP tr = Model_InstTurnRate(target);	//rads/cs
	dhdg = (Float)(Rads2Rowan (tr * shoottime * 5) );		//10/2: 2 to get ave
	if (dhdg > ANGLES_30Deg)
		dhdg = ANGLES_30Deg;
	if (dhdg < -ANGLES_30Deg)
		dhdg = -ANGLES_30Deg;
	newhdg = (SWord)VelHdg + dhdg;

	Math_Lib.high_sin_cos((Angles)newhdg,s,c);
	dx = MULSHSIN(s, VelHori,ANGLES_SHIFT);
	dz = MULSHSIN(c, VelHori,ANGLES_SHIFT);

	if (Range < PUREPURSUITRANGE)
	{
		deltax = dx * shoottime;
		deltaz = dz * shoottime;
		deltay = targetvely * shoottime;
		//lift nose to account for bullet drop
		//delta = 0.5 * g * t^2
		//g = 10 cm/ts/ts
		deltay = deltay + 5 * shoottime * shoottime;

		Float dr = Range - WEAPONSRANGE;
		if (dr > 0)
		{
			deltax = deltax * (1-dr/DLEADRANGE);
			deltay = deltay * (1-dr/DLEADRANGE);
			deltaz = deltaz * (1-dr/DLEADRANGE);
		}
	}

	FP	range;

#ifdef PRINT_AI_DATA
	PrintString(0, 17, "          ");
#endif

	if (cutcorner)
	{
		closure = vel - target->vel- MPH10;
		if ((closure ) < 0)
		{
			if (Range > INSIDEWEAPONSRANGE)
			{
#ifdef PRINT_AI_DATA
	PrintString(0, 17, "Cut Corner");
#endif
			
				if (Range > LAGRANGE)
					Range = LAGRANGE;

				dhdg = ANGLES_30Deg * (Range - INSIDEWEAPONSRANGE)/(LAGRANGE - INSIDEWEAPONSRANGE);
				if (tr < 0 )
					dhdg = -dhdg;
				newhdg = dhdg + target->hdg;
	
				Math_Lib.high_sin_cos((Angles)newhdg,s,c);
				SLong	cornerdeltax = (20 * MULSHSIN(s, target->velhori,ANGLES_SHIFT));
				SLong	cornerdeltaz = ( 20 *MULSHSIN(c, target->velhori,ANGLES_SHIFT));
				SLong	cornerdeltay = target->vely * 20;

				if (closure < -MPH100)
					closure = -MPH100;
				cornerdeltax = cornerdeltax * -closure/MPH100;
				cornerdeltaz = cornerdeltaz * -closure/MPH100;
				cornerdeltay = cornerdeltay * -closure/MPH100;

				deltax += cornerdeltax;
				deltay += cornerdeltay;
				deltaz += cornerdeltaz;


			}
		}
	}
}


//------------------------------------------------------------------------------
//Procedure		TurnFight
//Author		R. Hyde 
//Date			Fri 13 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::TurningFight()
{
	SWord	reqdeltahdg,reqdeltahdg_sign;
	UWord	reqdeltahdg_pos;
	COORDS3D	target;
	MODEL& MOD = *fly.pModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	SLong vx = 0, vy = 0, vz = 0;

	reqdeltahdg = 	hdg - HdgIntercept;
	Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);

	

//	if (		(Range < (DANGERRANGE + METRES200))
//	if (		(Range < (COMBATRANGE+ METRES400))
//			&&	(reqdeltahdg_pos < (SWord)ANGLES_30Deg)
//			&&	(reqdeltahdg_pos < (SWord)ANGLES_60Deg)
//		)		

#ifdef PRINT_AI_DATA
	PrintString(0, 15, "Target Tracking");
	PrintString(0, 16, "               ");
#endif

	if (	(reqdeltahdg_pos < (SWord)ANGLES_90Deg)
		)		
	{
#ifdef PRINT_AI_DATA
	PrintString(0, 15, "               ");
	PrintString(0, 16, "Lead Tracking  ");
#endif

		target.X = ai.unfriendly->World.X;
		target.Y = ai.unfriendly->World.Y;
		target.Z = ai.unfriendly->World.Z;
		InterceptandRange (&target);

		CalcLead(*this, *ai.unfriendly,vx,vy,vz, TRUE);

		target.X += vx;
		target.Y += vy;
		target.Z += vz;
		InterceptandRange (&target);

	}else
		target = ai.unfriendly->World;
	if (		(classtype->phrasename == PHRASE_MIGS)
//			&&	(		(ai.PilotSkills.combatskill >= SKILL_ACE)
//					||	(ai.morale < MORALE_GOOD)
			&&	(		(ai.morale < MORALE_GOOD)
				)
		)
		TurnWithPitchChange(HdgIntercept, FALSE, target);
	else
		TurnWithPitchChange(HdgIntercept, TRUE, target);
}
//------------------------------------------------------------------------------
void	AirStruc::TurnWithPitchChange(ANGLES	HdgIntercept, Bool sitbehindtarget,COORDS3D	target)
{
	if (CombatFactor() > 200)
	{
		if (classtype->phrasename == PHRASE_MIGS)
//			TurnFightSub(HdgIntercept, sitbehindtarget, ANGLES_5Deg, target);	
			TurnFightSub(HdgIntercept, sitbehindtarget, ANGLES_0Deg, target);	
		else if (World.Y > FT_10000)
			TurnFightSub(HdgIntercept, sitbehindtarget, ANGLES_0Deg, target);	
		else
			TurnFightSub(HdgIntercept, sitbehindtarget, ANGLES_0Deg, target);	

	}else
		TurnFightSub(HdgIntercept, sitbehindtarget, ANGLES_0Deg, target);

}
Bool	AirStruc::BanditOnColdSide()
{
	SWord reqdeltahdg = hdg - HdgIntercept;

	if	(		(reqdeltahdg < ANGLES_90Deg)
			||	(reqdeltahdg > ANGLES_270Deg)
		)	
	{
		return (FALSE);
	}

	if	(	(		(reqdeltahdg > 0)
				&&	(roll << ANGLES_90Deg)
			)
			||
			(		(reqdeltahdg < 0)
				&&	(roll >> ANGLES_270Deg)
			)
		)
		return(TRUE);
	else
		return(FALSE);

}
Bool	AirStruc::BanditHasLead()
{
	return(TRUE);
}
//
//------------------------------------------------------------------------------
//Procedure		HighYoYo
//Author		R. Hyde 
//Date			Wed 11 Mar 1998
//
//Description	Ref1.70
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::HighYoYo()
{
//approach from 30-60 AOT, cospeed
//
//roll wings level
//pull up
//when in rear hemisphere (say near 6 o'clock), closure 0
//	roll towards target then lead or lag pursuit
//
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			ACM.Thrust (100);
			SetManoeuvreTime (1500);
 			ai.desiredroll = ANGLES_0Deg; 
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			ACM.Roll (ai.desiredroll);
			ACM.Pitch (ANGLES_20Deg);

			if  ( !ManoeuvreTimeLeft())
  				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

		 	SWord 	deltaroll;
		 	UWord	deltaroll_pos;
 			SWord	deltaroll_sign;

			deltaroll = (SWord) (ai.desiredroll - roll);
  			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
  			if (		(deltaroll_pos <ANGLES_5Deg)
					&&	((World.Y - ai.unfriendly->World.Y) > FT_100)
				)
 				ai.PilotSkills.ManStep = PHASE2;

			break;

		}
		case PHASE2:
		{
			COORDS3D	target;
			Float fpitch,fhdg;
			AirStrucPtr	unfriendly = *ai.unfriendly;
			target = ai.unfriendly->World;


			ANGLES reqroll = roll +  GetRequiredRoll(*(rotitem*)this, (COORDS3D)target, fpitch, fhdg );
			ACM.Roll (reqroll);
			ACM.Pitch (ANGLES_20Deg);

			if (TargetOnSubjectTail(ANGLES_85Deg))
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
				ai.PilotSkills.ManStep = PHASE0;
			}
		}
	}


}
//------------------------------------------------------------------------------
//Procedure		Zoom
//Author		R. Hyde 
//Date			Wed 11 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::Zoom()
{
//roll wings nearly level
//pull up then zoom climb for x secs or near stall
//roll 180 and pull level
//
//
		ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

//	ACM.Step = ai.PilotSkills.ManStep;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SetManoeuvreTime (2000);
			MOD.Rudder = 0;

			ACM.Thrust (100);			//rdh
//			ACM.ClimbRate (0);
//			ACM.KeepWingsLevel ();

			ai.PilotSkills.ManStep = PHASE1;
			break;
		}
		case PHASE1:
		{
				ACM.Roll (ANGLES_0Deg);
				ACM.Pitch (ANGLES_40Deg);
			

			if  (		(!ManoeuvreTimeLeft ())
					||
						(vel <= CornerSpeed())
				)
			{
 				if (ai.firsttactic == TACTIC_ROUNDABOUT)
 					ai.PilotSkills.manoeuvre = MANOEUVRE_ROUNDABOUT;
 				else if (ai.firsttactic == TACTIC_LINEABREAST)
 					ai.PilotSkills.manoeuvre = MANOEUVRE_LINEABREAST;
 				else if (ai.firsttactic == TACTIC_LINEASTERN)
 					ai.PilotSkills.manoeuvre = MANOEUVRE_LINEASTERN;
 				else if (ai.firsttactic == TACTIC_DIVEANDZOOM)
					ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;					
				else
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;
		}
	}
}
//------------------------------------------------------------------------------
//Procedure		LowYoYo
//Author		R. Hyde 
//Date			Tue 3 Mar 1998
//
//Description	Ref1.73	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::LowYoYo ( )
{
//used when closure is insufficient
//
//in turning fight, attacker increases horizontal component of turn rate by nose down
//lead target
//level wings and pull up
//
//Note may have to write so that the ac doesn't try to get advantage in one 
//manoeuvre (see btm 1.73)
//
//
//
//Note can also be used when in tailchase to pick up speed to get just inside gun range
//
//
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	SWord	reqdeltahdg,reqdeltahdg_sign;
	UWord	reqdeltahdg_pos;
	COORDS3D	target;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	SLong		time;
	SWord	reqdeltapitch,reqdeltapitch_sign;
	UWord	reqdeltapitch_pos;
	SLong dx;
	SLong dy;
	SLong dz;




	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;

			ACM.Thrust (ThrustfromFF());
			SetManoeuvreTime (1500);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			CalcLead(*this, *ai.unfriendly,dx,dy,dz, TRUE);
		
			target.X = ai.unfriendly->World.X + dx;
			target.Y = ai.unfriendly->World.Y + dy;
			target.Z = ai.unfriendly->World.Z + dz;

			SWord c,s;
			ANGLES	depression = ANGLES_340Deg;
			Math_Lib.high_sin_cos(depression,s,c);
			SLong deltay = MULSHSIN(s, Range,ANGLES_SHIFT);
			SLong range = Range - WEAPONSRANGE;
			if (range < 0)
				range = 0;
			if (range > ENGAGERANGE)
				range = ENGAGERANGE;
			deltay = deltay * range /ENGAGERANGE;
			target.Y += deltay;
//			depression = ANGLES_0Deg;

			InterceptandRange (&target);
			TurnFightPhase1(HdgIntercept, TRUE, ANGLES_0Deg, target);

//			ACM.MinHeight (FT_2000);

			if  (!ManoeuvreTimeLeft())
			{
				if (Range > INSIDEWEAPONSRANGE)
				{
					if (		(Math_Lib.rnd(2))
							&&	(vel > unfriendly->vel)
						)
						ai.PilotSkills.ManStep = PHASE0;
					else
						ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
				}else
				{
					 ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
				}

			}
		}
	}

}
//------------------------------------------------------------------------------
//Procedure		InterceptHigh
//Author		R. Hyde 
//Date			Wed 11 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::InterceptHigh()
{
//intercept but stay at alt
//
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	SWord	reqdeltahdg,reqdeltahdg_sign;
	UWord	reqdeltahdg_pos;
	COORDS3D	target;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	SLong		time;
	SWord	reqdeltapitch,reqdeltapitch_sign;
	UWord	reqdeltapitch_pos;
	SLong dx;
	SLong dy;
	SLong dz;




	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;

			ACM.Thrust (ThrustfromFF());
			SetManoeuvreTime (15000);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			CalcLead(*this, *ai.unfriendly,dx,dy,dz, TRUE);
		
			target.X = ai.unfriendly->World.X + dx;
			target.Y = World.Y;
			target.Z = ai.unfriendly->World.Z + dz;


			InterceptandRange (&target);

			SLong deltaalt = World.Y - ai.unfriendly->World.Y;
			if (deltaalt < 0)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			if  (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
	
			TurnFightPhase1(HdgIntercept, TRUE, ANGLES_0Deg, target);

			if  (		((Range - deltaalt ) < DANGERRANGE)
					&&	 (SubjectOnTargetTail(ANGLES_45Deg))
					&&	(SubjectFacingTarget(ANGLES_45Deg))
				)
			{
				 ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;
			}

		}
	}


}
//------------------------------------------------------------------------------
//Procedure		ClimbAtSustainedTurnSpeed
//Author		R. Hyde 
//Date			Fri 20 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::ClimbAtSustainedTurnSpeed()
{
//climb but don't go below sustained turn speed
//aim towards unfriendly plus offset = f(desiredrange,desiredhdg)


//very similar to gainheight at present
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;


	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			SetManoeuvreTime (2000);
			ACM.GearUp ();
			ACM.Thrust (ThrustfromFF());
			if (ai.PilotSkills.combatskill > SKILL_REGULAR)
			{
				SWord	reqdeltahdg;
				reqdeltahdg = 	hdg - HdgIntercept;
				if (reqdeltahdg > 0)
					ai.desiredhdg = HdgIntercept + ANGLES_20Deg;
				else
					ai.desiredhdg = HdgIntercept - ANGLES_20Deg;

			}
			else
				ai.desiredhdg = HdgIntercept;

//			ACM.ClimbRate (-1500);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
//			ACM.SpeedWithElevators(0.61);

			ACM.Roll (ANGLES_0Deg);
	
//			MoveToHdg (ai.desiredhdg);


			if (MOD.Inst.MachNo > 0.55)
				ACM.Pitch (BestClimbAngle());
			else
				ACM.Pitch (ANGLES_355Deg);
//			if (MOD.Inst.MachNo < 0.4)
//			if (vel <= SustainedTurnSpeed())
//				ACM.Pitch (ANGLES_355Deg);
//			else
//				ACM.Pitch (ANGLES_5Deg);


			if (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}
	}


}

//------------------------------------------------------------------------------
//Procedure		SpinRecovery
//Author		R. Hyde 
//Date			Wed 21 Aug 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SpinRecovery ()
{
	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;
	FP rotvelymod;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			ACM.Thrust (100);

			SetManoeuvreTime (200);

			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			MOD.Elevator = 0;
			MOD.Aileron = 0;
			if (MOD.RotVel.y > 0)
			{
				MOD.Rudder = +MAXRUDDER;
			}else
			{
				MOD.Rudder = -MAXRUDDER;
			}

			MOD.RotVel.y = 0.95 * MOD.RotVel.y;
			rotvelymod =  MOD.RotVel.y;
			if (rotvelymod < 0)
				rotvelymod = - rotvelymod;

			if  (rotvelymod < 0.0005)
				ai.PilotSkills.ManStep = PHASE2;
			break;
				
		}
		case PHASE2:
		{
			MOD.Rudder = 0;
			MOD.RotVel.y = MOD.RotVel.y/2;		//at this stage we want spin to definitely stop
			rotvelymod =  MOD.RotVel.y;
			if (rotvelymod < 0)
				rotvelymod = - rotvelymod;
			if  (rotvelymod > 0.0005)
				ai.PilotSkills.ManStep = PHASE1;
			ACM.Roll (ANGLES_0Deg);
			ACM.Pitch (ANGLES_5Deg + pitch);
			if (	(pitch >> ANGLES_340Deg)
					||
					(pitch << ANGLES_90Deg)
				)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
		}
	}

}

//------------------------------------------------------------------------------
//Procedure		GainSpeed()
//Author		R. Hyde 
//Date			Tue 10 Sep 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::GainSpeed()
{


	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

//	ACM.Step = ai.PilotSkills.ManStep;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			MOD.Rudder = 0;
			SWord reqdeltahdg = 	hdg - HdgIntercept;
			ACM.Thrust (ThrustfromFF());
			if (reqdeltahdg > 0)
				ai.desiredroll = ANGLES_330Deg;
			else
				ai.desiredroll = ANGLES_30Deg;

			SetManoeuvreTime (2000);
			ACM.GearUp ();
			ACM.Thrust (ThrustfromFF());
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			if (classtype->phrasename == PHRASE_MIGS)
				ACM.Pitch (ANGLES_3Deg);
			else
				ACM.Pitch (ANGLES_355Deg);
			ACM.Roll (ai.desiredroll);

			if (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			break;
		}
	}





}





//------------------------------------------------------------------------------
//Procedure		LeadUnFriendly
//Author		R. Hyde 
//Date			Fri 15 Mar 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::LeadUnFriendly ()
{
	ACMMODEL& ACM = *fly.pAcmModel;

 	COORDS3D	target;
 	SLong		t,actualrange;
 	AirStrucPtr	unfriendly = *ai.unfriendly;
 
 //			target.X = ai.unfriendly->World.X;
 //			target.Y = ai.unfriendly->World.Y;
 //			target.Z = ai.unfriendly->World.Z;
 //			InterceptandRange (&target);
 			actualrange = Range;
 
 			//clofg velocity
 			SLong deltax = velx - unfriendly->velx;
 			SLong deltay = vely - unfriendly->vely;
 			SLong deltaz = velz - unfriendly->velz;
 			Math_Lib.Intercept (deltax,deltay,deltaz,Range,HdgIntercept,PitchIntercept);
 			//Range is a velocity in 10cm/s
 			if (Range != 0)
 			{
 				t = actualrange / Range;
 			}else
 			{
 				t = 0;
 			}
 			//when a long way off don't attempt to lead
 			if (t >2000)
 			t = 0;
 
 			target.X = ai.unfriendly->World.X + (unfriendly->velx * t / 10);
 			target.Y = ai.unfriendly->World.Y + (unfriendly->vely * t / 10);
 			target.Z = ai.unfriendly->World.Z + (unfriendly->velz * t / 10);
 			InterceptandRange (&target);
			TurnFightPhase1(HdgIntercept, TRUE, ANGLES_0Deg, target);

// 			ACM.Pitch (PitchIntercept);
//			MoveToHdg (HdgIntercept);
			Range = actualrange;

}



//------------------------------------------------------------------------------
//Procedure		AllButLdrNotOnCombat
//Author		R. Hyde 
//Date			Mon 11 Nov 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool AirStruc::AllButLdrNotOnCombat ()

{
	AirStrucPtr		ac,leadac;	
	if (formpos == 0)
	{//leader
		ac = *this;
	}else
	{
		ac = fly.leadflight;
	}
	leadac =ac;
	do
	{
		do
		{
			if  (	(ac->AutoMoveInfo.movecode==AUTO_COMBAT)
				&&  (	(ac->ai.PilotSkills.manoeuvre != MANOEUVRE_TOPCOVER)
					&&	(ac->ai.PilotSkills.manoeuvre != MANOEUVRE_LOOKROUND)
					&&	(ac->ai.PilotSkills.manoeuvre != MANOEUVRE_DIVEFORHOME)	
					&&	(ac->ai.PilotSkills.manoeuvre != MANOEUVRE_CLIMBFORHOME)
					&&	(ac->ai.PilotSkills.manoeuvre != MANOEUVRE_STRAIGHTDIVE)
					&&	(ac->ai.PilotSkills.manoeuvre != MANOEUVRE_GOHOME )		
					&&	(ac->ai.PilotSkills.manoeuvre != MANOEUVRE_WELDEDWINGMAN )		//rdh 25/6/99
					)
				&&
					(formpos != 0)
				)
				return (FALSE);
			ac = *ac->Follower();
		}
		while (ac);
	leadac = *leadac->fly.nextflight;
	ac = leadac;
	}
	while (leadac);
	return (TRUE);

}
//------------------------------------------------------------------------------
//Procedure		AllOnTopCover
//Author		R. Hyde 
//Date			Mon 19 Aug 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool AirStruc::AllOnTopCover ()

{
	AirStrucPtr		ac,leadac;	
	if (formpos == 0)
	{//leader
		ac = *this;
	}else
	{
		ac = fly.leadflight;
	}
	leadac =ac;

	do
	{
		do
		{
			if (	ac->AutoMoveInfo.movecode==AUTO_COMBAT
				&&	ac->ai.PilotSkills.manoeuvre != MANOEUVRE_TOPCOVER
				&&  (InterceptandRange(ac),Range<METRES4000)
				)
				return (FALSE);
			ac = *ac->Follower();
		}
		while (ac);
	leadac = *leadac->fly.nextflight;
	ac = leadac;
	}
	while (leadac);
	return (TRUE);

}



//------------------------------------------------------------------------------
//Procedure		ClimbAtBestSpeed
//Author		R. Hyde 
//Date			Mon 18 Mar 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::ClimbAtBestSpeed ()
{

}


//------------------------------------------------------------------------------
//Procedure		AutoShoot
//Author		R. Hyde 
//Date			Fri 12 Jul 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::AutoShoot (Bool manual)
{
	const	int TARGETSIZE = 400;		//METRES02_50;		//wingspan = 40ft, we want less
	COORDS3D	target;
	Float		time;
	SWord	reqdeltahdg,reqdeltahdg_sign;
	UWord	reqdeltahdg_pos;
 	SWord		reqdeltapitch;
	UWord	reqdeltapitch_pos;
	SWord	reqdeltapitch_sign;
	AirStrucPtr	unfriendly = *ai.unfriendly;



	reqdeltahdg = 	hdg - HdgIntercept;
	Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);
	if  (		(Range < INSIDEWEAPONSRANGE)
//	if  (		(Range < METRES300)
			&&	(reqdeltahdg_pos < ANGLES_70Deg)
		)
	{
		target.X = ai.unfriendly->World.X;
		target.Y = ai.unfriendly->World.Y;
		target.Z = ai.unfriendly->World.Z;
		InterceptandRange (&target);

//		time = Range / classtype->muzzlevel;		//MUZZELVEL;
//		//calc lead position using this time
//		Float velx = unfriendly->velx * time;
//		Float vely = unfriendly->vely * time;
//		Float velz = unfriendly->velz * time;

		SLong vx;
		SLong vy;
		SLong vz;

//#ifndef PRINT_AI_DATA
//#define PRINT_AI_DATA
//#endif

#ifdef PRINT_AI_DATA
	PrintString(0, 3, "                    ");
	PrintString(0, 4, "                    ");
	PrintString(0, 5, "                    ");
#endif

		CalcLead(*this, *ai.unfriendly,vx,vy,vz, FALSE);

		target.X = ai.unfriendly->World.X + vx;
		target.Y = ai.unfriendly->World.Y + vy;
		target.Z = ai.unfriendly->World.Z + vz;
		InterceptandRange (&target);

		reqdeltahdg = 	hdg - HdgIntercept;

		Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);
		if (reqdeltahdg_pos < ANGLES_10Deg)
		{
			reqdeltapitch = PitchIntercept - pitch;
			Math_Lib.Pos (reqdeltapitch,reqdeltapitch_pos,reqdeltapitch_sign);
			if (reqdeltapitch_pos < ANGLES_10Deg)
			{
#ifdef PRINT_AI_DATA
	PrintVar(0,  3, "T Pitch %.1f ", (FP)( (PitchIntercept - pitch) / 182.04) );
	PrintVar(0,  4, "T Headg %.1f ", (FP)( (HdgIntercept - hdg)  / 182.04) );
	PrintVar(0,  5, "T Range %.1f ", (FP)( (Range)  / 100.0) );
#endif

				SWord AngError = reqdeltahdg_pos;	//CSB 04/06/99	
				if (reqdeltahdg_pos  < reqdeltapitch_pos)
					AngError = reqdeltapitch_pos;
				Float delta = (Range * AngError) / 10431;	//CSB 04/06/99	
#ifdef PRINT_AI_DATA
	PrintVar(0, 7, "T Delta %.2f ", (FP)(delta / 100));
#endif
				UByte	FF = FlyingFactor();
//				int		target = TARGETSIZE/4 + 3*TARGETSIZE*(256-FF)/1024;
				int		target = TARGETSIZE/2 + TARGETSIZE*(256-FF)/512;
				if((delta < 1000))// || ((reqdeltahdg_pos < ANGLES_5Deg) && (reqdeltapitch_pos < ANGLES_5Deg))	//CSB 14/04/99		//Fix This !!!!

				{
/* TEST CODE CSB 03/06/99 */
/* TEST CODE CSB 03/06/99 */					
/* TEST CODE CSB 03/06/99 */					
/* TEST CODE CSB 03/06/99 */
/* TEST CODE CSB 03/06/99 */
/* TEST CODE CSB 03/06/99 */

#ifdef PRINT_AI_DATA
PrintString(60, 19, " @@@@@@@@@@@@@@ ");
PrintString(60, 20, "@@@  FIRING  @@@");
PrintString(60, 21, " @@@@@@@@@@@@@@ ");
#endif
					ANGLES OldPitch = pitch;
					ANGLES OldHdg   = hdg;

					if((classtype->visible == MIG15) || (classtype->visible == CMIG15))
						if(ai.PilotSkills.simpleacm)
						{
							pitch += Math_Lib.rnd(ANGLES_10Deg) - ANGLES_5Deg;
							hdg   += Math_Lib.rnd(ANGLES_10Deg) - ANGLES_5Deg;
						}

					FireABullet(150);			
				
					pitch = OldPitch;
					hdg   = OldHdg;
					
					if (ai.unfriendly == Persons2::PlayerGhostAC)
						 _Miles.SequenceAudible(MOOD_DOOMED);
				}else
				{
					if (follower)
						_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_SHOOTHIM, MSG_SHOOTHIM, Follower(), NULL, this));

					if (ai.unfriendly == Persons2::PlayerGhostAC)
						 _Miles.SequenceAudible(MOOD_DESPERATE);
				}
//				if	(classtype->visible != MIG15)
//					FireABullet(480);			
				ai.lastdelta = delta;

			}else
			{
				ai.lastdelta = INSIDEWEAPONSRANGE;
				if (ai.unfriendly == Persons2::PlayerGhostAC)
					 _Miles.SequenceAudible(MOOD_TENSE);
			}
		}else
			ai.lastdelta = INSIDEWEAPONSRANGE;
	}else
		ai.lastdelta = INSIDEWEAPONSRANGE;


}

//------------------------------------------------------------------------------
//Procedure		PublicFireABullet
//Author		Craig Beeston 
//Date			Sat 3 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::PublicFireABullet(SWord delaybetweenbursts, Bool isArmed)
{
	FireABullet(delaybetweenbursts,isArmed);
}
//------------------------------------------------------------------------------
//Procedure		FireABullet
//Author		R. Hyde 
//Date			Sat 13 Jul 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::FireABullet(SWord delaybetweenbursts, Bool isArmed)
{
	//	SWord	timeleft;
	WeapAnimData*	weapon;										//RJS 29Aug96
	SLong			xpos, ypos, zpos;							//RJS 29Aug96
	UWord			index;										//RJS 03Sep96
	UWord			mvel,mdelay,mburst;							//RDH 31Jul98
	UWord			theShape;									//RDH 31Jul98

//	timeleft = weap.ShootDelay-Timer_Code.FRAMETIME;//DAW 31Jul96
//	if (timeleft<0)
	if (weap.ShootDelay <=0)
	{
//done by Robert		Art_Int.PersonalThreat(*ai.unfriendly,this);				//RDH 17Oct96
		WorldStuff*	worldptr = mobileitem::currworld;
		SLong			highestammo = 0;						//RJS 08Sep98
		UWord			indexlist[6];							//RJS 08Sep98
		UWord			indexcnt;								//RJS 08Sep98
		WeapAnimData*	useweapon = NULL;								//RJS 08Sep98
		SLong			usexpos, useypos, usezpos;				//RJS 08Sep98
		UWord			usemvel;								//RJS 08Sep98
		UWord			useindex;								//RJS 08Sep98
		UWord			usemdelay;								//RJS 08Sep98
		UWord			usemburst;								//RJS 08Sep98
		UWord			useShape;								//RJS 08Sep98

 		Trans_Obj.PlayGunSound((mobileitem* )this,*worldptr);//RDH 12Jul96
		theShape = BULLET;										//RJS 20Nov98

		indexcnt = 0;

		// digadigadiga (alternate fire)
		for (index = 0; index < 6; index++)						//RJS 16Jun98
		{
			weapon = SHAPE.GetWeaponLauncher(this,index,xpos,ypos,zpos,mvel,mdelay,mburst,LT_BULLET,&theShape);//RDH 31Jul98
			if (weapon)
			{
				indexlist[indexcnt++] = index;
				if (weapon->LoadedStores > highestammo)
				{
				 	highestammo = weapon->LoadedStores;
					useweapon = weapon;
					usexpos = xpos;
					useypos = ypos;
					usezpos = zpos;
					useindex = index;
					usemdelay = mdelay;
					usemburst = mburst;
					useShape = theShape;
					usemvel = mvel;
				}
//this was insered by maccas changes ?
//so ROD has been editing on macca machine then ??

					if (this==Persons2::PlayerSeenAC)
					{
						_DPlay.NewBullet(mburst,index);					//RDH 31Jul98
					}
			}
		}

		if (useweapon)
		{
			SLong	i;
			SLong	otherindex = useindex;
			for (i=0; i < indexcnt; i++)
			{
				if (indexlist[i] != useindex)
				{
					otherindex = indexlist[i];
					break;
				}
			}

			SLong	mydelaybetweenbursts = usemburst * usemdelay;	//RJS 07Jul99
			if (mydelaybetweenbursts > delaybetweenbursts)			//RJS 07Jul99
				delaybetweenbursts = mydelaybetweenbursts;			//RJS 07Jul99

			Trans_Obj.LaunchManyGunBullets(	(mobileitem* )this,
		 									usemvel,
		 									usemburst,				// no bullets//RDH 31Jul98
		 									usemdelay,		// delay between each bullet//DAW 29Jul98
		 									useindex,		// left  /  right
		 									otherindex,		// right /  left
		 									(ShapeNum)useShape,	//RJS 08Sep98
		 									usexpos,
		 									useypos,
		 									usezpos,
		 									*worldptr,
		 									ANGLES_0Deg,
											LT_BULLET,
											isArmed);	//RJS 27May99
		 
			useweapon->LoadedStores--;

		 	// store num bullets and index of launcher

			if (this==Persons2::PlayerSeenAC)					//AMM 17Nov98
			{
			 	_DPlay.NewBullet(usemburst,useindex);				//RJS 08Sep98
			}
		}

		weap.ShootDelay = delaybetweenbursts;			//RJS 07Jul99
	}
}

//------------------------------------------------------------------------------
//Procedure		FireARocket
//Author		Craig Beeston
//Date			Thu 25 Mar 1999
//
//Description	Just Like Fire a Bullet but for rockets
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::FireARocket(SWord delaybetweenbursts,Bool isArmed)	//RJS 27May99
{
	WeapAnimData*	weapon;										//RJS 29Aug96
	SLong			xpos, ypos, zpos;							//RJS 29Aug96
	UWord			index;										//RJS 03Sep96
	UWord			mvel,mdelay,mburst;							//RDH 31Jul98
	UWord			theShape;									//RDH 31Jul98

	Mono_Text.PrintHexAt(60,15,weap.ShootDelay,4);
	if (weap.ShootDelay <=0)
	{
		WorldStuff*	worldptr = mobileitem::currworld;
		SLong			highestammo = 0;						//RJS 08Sep98
		UWord			indexlist[6];							//RJS 08Sep98
		UWord			indexcnt;								//RJS 08Sep98
		WeapAnimData*	useweapon = 0;								//RJS 08Sep98
		SLong			usexpos, useypos, usezpos;				//RJS 08Sep98
		UWord			usemvel;								//RJS 08Sep98
		UWord			useindex;								//RJS 08Sep98
		UWord			usemdelay;								//RJS 08Sep98
		UWord			usemburst;								//RJS 08Sep98
		UWord			useShape;								//RJS 08Sep98
		theShape = ROCKET;										//RJS 20Nov98

		indexcnt = 0;

		// digadigadiga (alternate fire)
		for (index = 0; index < 6; index++)						//RJS 16Jun98
		{
			weapon = SHAPE.GetWeaponLauncher(this,index,xpos,ypos,zpos,mvel,mdelay,mburst,LT_ROCKET,&theShape);//RDH 31Jul98	//CSB 25/03/99	
			if (weapon)
			{
				indexlist[indexcnt++] = index;
				if (weapon->LoadedStores > highestammo)
				{
				 	highestammo = weapon->LoadedStores;
					useweapon = weapon;
					usexpos = xpos;
					useypos = ypos;
					usezpos = zpos;
					useindex = index;
					usemdelay = mdelay;
					usemburst = mburst;
					useShape = theShape;
					usemvel = mvel;
				}
//this was insered by maccas changes ?
//so ROD has been editing on macca machine then ??

					if (this==Persons2::PlayerSeenAC)
					{
						_DPlay.NewBullet(mburst,index);					//RDH 31Jul98
					}
			}
		}

		if (useweapon)
		{
			SLong	i;
			SLong	otherindex = useindex;
			for (i=0; i < indexcnt; i++)
			{
				if (indexlist[i] != useindex)
				{
					otherindex = indexlist[i];
					break;
				}
			}
#ifdef PRINT_AI_DATA
	PrintVar(20, 14, "Muz Vel %.1f ", (FP)(usemvel / 10) );
#endif

			WorldStuff*	worldptr = mobileitem::currworld;
			Trans_Obj.LaunchRocket( (mobileitem*)this,
									(ShapeNum)useShape,
									usexpos, 
									useypos,
									usezpos,			  
									*worldptr);
		 
			useweapon->LoadedStores--;
			weap.ShootDelay = usemdelay;
			

			if (this==Persons2::PlayerSeenAC)					//AMM 17Nov98
			{
			 	_DPlay.NewBullet(usemburst,useindex);				//RJS 08Sep98
			}
		}
	}
//	weap.ShootDelay = timeleft;

}




//------------------------------------------------------------------------------
//Procedure		ManoeuvreTimeLeft
//Author		R. Hyde 
//Date			Tue 19 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
SWord AirStruc::ManoeuvreTimeLeft ()

{
	return(TimeLeft());

}

//------------------------------------------------------------------------------
 //------------------------------------------------------------------------------
//Procedure		BreakCallNecessary
//Author		R. Hyde 
//Date			Tue 19 Dec 1995
//
//Description	
//					Horrible code to retain the globals
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool AirStruc::WeldedWingSixOK()
{
	bool sixok = true;
	int range = ENGAGERANGE;
	if  ( !ManoeuvreTimeLeft())
	{
		if (ai.attacker)
		{
			if (BreakCallNecessary(this, ai.attacker, range))
			{
				sixok = false;
				MANOEUVRE manoeuvre;
				RndVal	rndnum = Math_Lib.rnd();
				if	(rndnum < RND50PC)
					manoeuvre = MANOEUVRE_BREAKTURN;
				else
					manoeuvre = MANOEUVRE_SPLITS;						  //RDH 15/06/99 //RDH 20/06/99

				Art_Int.SetEngage(this,ai.attacker,manoeuvre,ANGLES_0Deg,ANGLES_0Deg, FALSE);
				if (leader)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_NOTCLEAR, MSG_CLEAR_RPT, this, NULL, ((AirStrucPtr)leader)));
			}

		}
		if (	(leader)	&&	(((AirStrucPtr)leader)->ai.attacker))
		{
			if (BreakCallNecessary(((AirStrucPtr)leader), ((AirStrucPtr)leader)->ai.attacker, range))
			{
				sixok = false;
			   BreakCallandReaction(this, ((AirStrucPtr)leader), ((AirStrucPtr)leader)->ai.attacker); //DAW 05/07/99


			}
		}
		if(sixok)
		{
			int drange = range - DANGERRANGE;
			if (drange < 0)
				drange = 0;
			int time = 200 + drange/100;
			SetManoeuvreTime (time);
		}else
			SetManoeuvreTime (200);
	}
	return(sixok);

}

//------------------------------------------------------------------------------
//Procedure		BreakCallNecessary
//Author		R. Hyde 
//Date			Tue 19 Dec 1995
//
//Description	
//					Horrible code to retain the globals
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool AirStruc::BreakCallNecessary(AirStrucPtr currac,AirStrucPtr unf, int range)
{
	bool Break = false;
	if (		(currac->ai.unfriendly != unf)
			||	(currac->AcIsPlayer())
		)
	{
  		ANGLES	oldhdgint, oldpitchint;
		SLong	oldrange;
		ANGLES	hdgint;
		oldhdgint = HdgIntercept;
		oldpitchint = PitchIntercept;
		oldrange  = Range;

		Art_Int.InterceptRangeFromTo(currac, unf);
		range = Range;
		if (Range < ENGAGERANGE)		//rdh 13/7(DANGERRANGE+ METRES200)) 
		{
			ANGLES delta = HdgIntercept + ANGLES_180Deg + ANGLES_45Deg - currac->hdg;
			if ((delta) << (ANGLES_90Deg))
				Break = true;
		}
		PitchIntercept = oldpitchint;
		HdgIntercept = oldhdgint;
		Range = oldrange;
	}

	return(Break);
}

//------------------------------------------------------------------------------
//Procedure		SetFollowerTactic
//Author		Craig Beeston 
//Date			Mon 5 Jul 1999
//
//Description	Sets the ai.FirstTactic for the whole group
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SetFollowersTactic(int TACTIC)
{
	AirStrucPtr Lead, Foll;
	for(Lead = FindGroupLeader(); Lead; Lead = Lead->fly.nextflight)
		for(Foll = Lead; Foll; Foll = Foll->Follower())
			Foll->ai.firsttactic = (tFirstEncounterTactics)TACTIC;

}

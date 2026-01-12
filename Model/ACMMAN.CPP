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
//Filename       acmman.cpp
//System         
//Author         R. Hyde 
//Date           Fri 1 Mar 1996
//Description    ACM manoeuvres - moved from flymodel
//------------------------------------------------------------------------------

//									SCRIPT_NOTCLEAR
//FlightSep, nose to nose: use Shaw to check
//
//
//: fill in
//check out:
//Bool AirStruc::SubjectClosureGreaterThan(SLong dvel)
//
//
//1		Need to try to decide between wanting max turnrate or min turn radius
//2		Ref1.78: not done nosetonose nosetotail differences
//3		check lead or lag pursuit defense, 1.63
//4		Use Immelmann where appropriate
//5		 Need a chance of individual breaking off and trying to take on opposition without help
//6		Need to check that angles v energy fight is covered:
//			if thrust/wt ratio is higher or got more energy choose energy
//			angles:
//				go for corner vel with dive and zoom to stay there
//			energy:
//				stay at sustained turn and wait for other fighter to loose energy
//				then zoom to get advantage
//7		Using circumvent?

//Summary
//
//
//Throughout manoeuvre choice we should be aware of the best thing to do and
//be able to choose worse manoeuvres for poorer pilots
//
//The manoeuvres are aimed to either get an angles or an energy advantage.
//Most of the time, it is advantageous to use introduce a vertical component
//to the manoeuvre
//
//An important decision is to decide whether to aim for the corner speed or the
//maximum sustained turnrate. The corner speed gives the max instantaneous
//turnrate but at the expense of loss of energy and deterioration of performance.
//Sometimes minimum turn radius is the aim (at or less than corner vel).
//
//
//
//The process is as follows
//
//1		if ac wants to and if it is appropriate
//			attempt to disengage
//
//2		if necessary goto GunDefence()
//			may be a special manoeuvre that is appropriate
//				else BreakTurn which goes for corner vel
//
//3		if appropriate go to special manoeuvre
//		else choose lead,pure,lag pursuit at either sustained turn or corner vel
//				BREAKTURN is corner vel manoeuvre
//				SUSTAINEDTURN is sustained turn manoeuvre with either lead or pure pursuit
//4		AutoShoot()
//			Attempt either a tracking or snapshot
//
//
//morale		determines the aggressivenes of the manoeuvre chosen
//					high morale means a highly aggressive manoeuvre wil be chosen
//combat.PilotSkills.combatskill	determines the type of manoeuvre chosen
//flying.PilotSkills.combatskill	determines how successful the implementation of the manoeuvre is
//
//
//
//LP Jobs
//1		Muzzle vel as f(aircraft or weapon)
//2		Formations in formatn.cpp, revised figures and introduce variation = f(.PilotSkills.combatskill)
//			Formation distance will depend on flyingability: lots of stragglers
//3		Attack bombers: multiple directions/waves
//4		evade by going into cloud
//5		Drag sabre down aaa invested vallley
//
//
//References
//
//1		Shaw
//2		Walker
//
//
//Notes
//1		When attacker field is set, it just means that we have set an attacker to attack ac
//			ac does not necessarily know. Ac knows if he unfriendly field is set
//
//2		Morale is dropped in combat by being hit and pulling g
//3		Some of the conditions to choose need fuzzier logic
//4		<= MORALE_MEDIUM for dive and zoom
//5		Range should be considered a 3d distance
//6		gmeter measures lift in air and 1 on ground
//			radial accel = lift/mass - g.cos pitch
//		so unloaded when lift = 0, gmeter = 0
//7		Task overload 1.236
//8		Flat Scissors should not coded as a manoeuvre
//			it should come as a result of beam to beam turn in
//			this will have to be tuned
//			scissors will be used by the inexperienced just to weave about
//
//Russion tactic: attack ,cover, reserve
//
//
//rollbanditplane	this is the roll to put the bandit on the lift vector plane
//gbanditplane		this is g required to keep a/c in the horizontal plane
//					when rolled to put lift vector on bandit
//
//gsustained		for a particular set of conditions, this is the max g that can
//					be sustained 
//
//rollsustained		roll for gsustained
//
//
//
//
//
//

#include "_MODE.H" //RERUN

#define F_GRAFIX												//DAW 05Aug96
#define F_COMMON
#define F_BATTLE
#include <stdio.h>
#include <string.h>

#include	"COLLIDED.H"
#include	"DOSDEFS.H"
#include	"WORLDINC.H"
#include	"FLYMODEL.H"
#include	"MYMATH.H"
#include	"ENUMBITS.M"
#include	"time.h"											//RDH 25Apr96
#include	"RANGES.H"
#include	"MISSMAN2.H"
#include	"MOVEMENT.H"
#include	"TRANSITE.H"
#include	"AI.H"
#include	"MILES.H"
#include	"LANDSCAP.H"
#include	"VIEWSEL.H"
#include	"3DCOM.H"
#include	"SPEED.H"
#include	"PERSONS2.H"
#include "MONOTXT.H"
#include "MODVEC.H"
#include "MODEL.H"
#include "ACMMODEL.H"
#include	"REPLAY.H"
#include "OVERLAY.H"

//MATHABLE	(ANGLES)

// ROD don't use STALL_ANGLE
//ANGLES const	STALL_ANGLE = ANGLES_15Deg;

struct RequiredElevatorElement
	{
		ANGLESInst pitch; SWord elevator;
	};




static RequiredElevatorElement 	RequiredElevatorData[] =
			{
				{ANGLES_FifthDeg, 1},
				{ANGLES_HalfDeg, 4},
				{ANGLES_1Deg, 6},
				{ANGLES_2Deg, 8},
				{ANGLES_3Deg, 10},
				{ANGLES_4Deg, 12},
				{ANGLES_5Deg, 14},
				{ANGLES_6Deg, 16},
				{ANGLES_10Deg, 20},
				{ANGLES_22Deg, MAXELEVATOR},
				{ANGLES_180Deg, MAXELEVATOR}
			};



struct RequiredAileronElement
	{
		ANGLESInst roll; SWord aileron;
	};



static RequiredAileronElement 	RequiredAileronData[] =
			{
				{ANGLES_FifthDeg, 2},
				{ANGLES_HalfDeg, 3},
				{ANGLES_1Deg, 4},
				{ANGLES_2Deg, 6},
				{ANGLES_3Deg, 10},
				{ANGLES_4Deg, 12},
				{ANGLES_5Deg, 16},
				{ANGLES_6Deg, 20},
				{ANGLES_10Deg, 24},
				{ANGLES_22Deg, 28},
				{ANGLES_90Deg, 60},
				{ANGLES_180Deg, 60}
			};
//TempCode RDH 17Jul96 			{
//TempCode RDH 17Jul96 				{ANGLES_FifthDeg, ANGLES_0Deg},
//TempCode RDH 17Jul96 				{ANGLES_HalfDeg, ANGLES_3Deg},
//TempCode RDH 17Jul96 				{ANGLES_1Deg, ANGLES_4Deg},
//TempCode RDH 17Jul96 				{ANGLES_2Deg, ANGLES_5Deg},
//TempCode RDH 17Jul96 				{ANGLES_3Deg, ANGLES_7Deg},
//TempCode RDH 17Jul96 				{ANGLES_4Deg, ANGLES_9Deg},
//TempCode RDH 17Jul96 				{ANGLES_5Deg, ANGLES_11Deg},
//TempCode RDH 17Jul96 				{ANGLES_6Deg, ANGLES_16Deg},
//TempCode RDH 17Jul96 				{ANGLES_10Deg, ANGLES_20Deg},
//TempCode RDH 17Jul96 				{ANGLES_22Deg, ANGLES_45Deg},
//TempCode RDH 17Jul96 				{ANGLES_45Deg, MAXCOMBATROLL},
//TempCode RDH 17Jul96 				{ANGLES_180BareDeg, MAXCOMBATROLL}
//TempCode RDH 17Jul96 			};
//TempCode RDH 17Jul96 
//------------------------------------------------------------------------------
//Procedure		TargetOnSubjectTail
//Author		R. Hyde 
//Date			Thu 14 Dec 1995
//
//Description	target is on subject tail
//
//Inputs		HdgIntercept,PitchIntercept and Range setup
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::TargetOnSubjectTail(ANGLES angle)

{
	Bool retval=FALSE;												  //DAW 27/09/00
	AirStrucPtr	unfriendly = *ai. unfriendly;
	
	ANGLES delta = HdgIntercept + ANGLES_180Deg + angle - hdg;
	ANGLES anglerange = angle + angle;

	if ((delta) << (anglerange))
	{
//		ANGLES delta = PitchIntercept + ANGLES_180Deg + angle - pitch;
		delta = - PitchIntercept + angle - pitch;
		if ((delta) << (anglerange))
			retval=TRUE;
	}
	return(retval);

}

//------------------------------------------------------------------------------
//Procedure		SubjectOnTargetTail
//Author		R. Hyde 
//Date			Thu 14 Dec 1995
//
//Description	subject is on target tail
//
//Inputs		HdgIntercept,PitchIntercept and Range setup
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::SubjectOnTargetTail(ANGLES angle)

{
//	AirStrucPtr	unfriendly = *ai.unfriendly;
//
//	ANGLES	temp;
//	
//	temp = HdgIntercept + angle - unfriendly->hdg; 
////DeadCode RDH 30Mar98 	if (temp < (2 * angle))
//	if (temp << (angle + angle))
//	{
//		temp = PitchIntercept + angle - unfriendly->pitch;
//		if (temp << (angle + angle))
//		{
//	 		return (TRUE);
//		}else
//		{
//			return (FALSE);
//		}
//	}else
//	{
//		return (FALSE);
//	}

	Bool retval=FALSE;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	SWord temp;
	SWord range = angle;
	
	if (unfriendly)
	{
		temp = HdgIntercept  - unfriendly->hdg; 
		if  (		(temp < range)
				&&	(temp > -range)
			)
		{
			temp = PitchIntercept - unfriendly->pitch;
			if  (		(temp < range)
					&&	(temp > -range)
				)
				retval=TRUE;
		}
	}
	return(retval);
}
//------------------------------------------------------------------------------
//Procedure		NoseToTail
//Author		R. Hyde 
//Date			Wed 11 Mar 1998
//
//Description	when beam to beam, the ac are on a pass
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::NoseToTail (ANGLES angle)
{
	Bool retval=FALSE;
	AirStrucPtr	unfriendly = *ai.unfriendly;

	if (unfriendly)
		if ((hdg + ANGLES_180Deg + angle - unfriendly->hdg) << (angle + angle))//RDH 19Sep97
			retval=TRUE;
	return(retval);
}	

//------------------------------------------------------------------------------
//Procedure		TargetFacingSubject
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
Bool	AirStruc::TargetFacingSubject(ANGLES angle)
{
	Bool retval=FALSE;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	if (unfriendly)
		if ((HdgIntercept + ANGLES_180Deg + angle - unfriendly->hdg) << (angle + angle))//RDH 19Sep97
			retval=TRUE;
	return(retval);
}

//------------------------------------------------------------------------------
//Procedure		SubjectFacingTarget
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
Bool	AirStruc::SubjectFacingTarget(ANGLES angle)
{
	Bool retval=FALSE;

	if ((HdgIntercept + angle - hdg) << (angle + angle))	//RDH 19Sep97
		retval=TRUE;
	return(retval);

}

//------------------------------------------------------------------------------
//Procedure		AircraftDamaged
//Author		R. Hyde 
//Date			Tue 20 Aug 1996
//
//Description	
//
//Inputs		
//
//Returns	if above critical damage return true
//
//------------------------------------------------------------------------------
Bool	AirStruc::AircraftDamaged()
{

		return(FALSE);


}


//return a value between 0 and 255 to indicate ability to make the right decisions
UByte	AirStruc::CombatFactor()
{
	UWord retval;
//##temp

//	if	(classtype->visible == MIG15)
//	{
//		ai.PilotSkills.combatskill = SKILL_HERO;
//		ai.morale =   MORALE_TOPMORALE;
//		ai.PilotSkills.combatskill = SKILL_REGULAR;
//		ai.morale =   MORALE_MEDIUM;
//		ai.PilotSkills.combatskill = SKILL_NOVICE;
//		ai.morale =   MORALE_ROCKBOTTOM;
//	}else
//	{
//		ai.PilotSkills.combatskill = SKILL_HERO;
//		ai.morale =   MORALE_TOPMORALE;
//		ai.PilotSkills.combatskill = SKILL_REGULAR;
//		ai.morale =   MORALE_MEDIUM;
//		ai.PilotSkills.combatskill = SKILL_NOVICE;
//		ai.morale =   MORALE_ROCKBOTTOM;
//	}


	retval = (64*ai.morale)/MORALE_TOPMORALE + (192*ai.PilotSkills.combatskill)/SKILL_MAX;
	if (retval > 255)
		retval = 255;
	

	return((UByte)retval);
}
//return a value between 0 and 255 to indicate ability to fly chosen manoeuvre
UByte	AirStruc::FlyingFactor()
{
	UWord retval;
	
//	if	(classtype->visible == MIG15)
//	{
//		ai.PilotSkills.flyingskill = SKILL_HERO;
//		ai.morale =   MORALE_TOPMORALE;
//		ai.PilotSkills.flyingskill = SKILL_REGULAR;
//		ai.morale =   MORALE_MEDIUM;
//		ai.PilotSkills.flyingskill = SKILL_NOVICE;
//		ai.morale =   MORALE_ROCKBOTTOM;
//	}else
//	{
//		ai.PilotSkills.flyingskill = SKILL_HERO;
//		ai.morale =   MORALE_TOPMORALE;
//		ai.PilotSkills.flyingskill = SKILL_REGULAR;
//		ai.morale =   MORALE_MEDIUM;
//		ai.PilotSkills.flyingskill = SKILL_NOVICE;
//		ai.morale =   MORALE_ROCKBOTTOM;
//	}


	retval = (64*ai.morale)/MORALE_TOPMORALE + (192*ai.PilotSkills.flyingskill)/SKILL_MAX;
	if (retval > 255)
		retval = 255;
	
	return((UByte)retval);
}
//------------------------------------------------------------------------------
//Procedure		SelectNextDisengageManoeuvre
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
void	AirStruc::SelectNextDisengageManoeuvre ()

{
//## if k(combat.PilotSkills.combatskill + morale) < combattime
//	try to Disengage

	AirStrucPtr	unfriendly = *ai.unfriendly;

	if (	(unfriendly->classtype->aerobaticfactor > AEROBATIC_MEDIUM)
		&&	classtype->aerobaticfactor == AEROBATIC_MEDIUM)	
	{//prop fighter v jet
		if	(Range > ENGAGERANGE)
		{
			ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;
		}else if (AboveGround(FT_250))
			ai.PilotSkills.manoeuvre = MANOEUVRE_DIVINGSPIN;
		else
			ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;

	}else if	(		(Range > ENGAGERANGE)
					&&	(AircraftDamaged())
				)
	{
		if (ai.morale 	< 	MORALE_GOOD)
			ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEFORHOME;
		else if	(ai.PilotSkills.combatskill < SKILL_REGULAR)
			ai.PilotSkills.manoeuvre = MANOEUVRE_GOHOME;
		else
			ai.PilotSkills.manoeuvre = MANOEUVRE_CLIMBFORHOME;

		AirStrucPtr	u=*ai.unfriendly;
		if (u)
		{
			ai.unfriendly=NULL;
			u->ai.attacker=NULL;
		}

	}else  if	(				(Range > (METRES1000 + BREAKOFFRANGE))
							&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_GAINHEIGHT)							
				)
					
	{
			ai.PilotSkills.manoeuvre = MANOEUVRE_LOOKROUND;
			unfriendly = *ai.unfriendly;
			unfriendly->ai.attacker = NULL;
			ai.unfriendly = NULL;

	}else
	{
		if (AircraftDamaged())
				if (SubjectFacingTarget(ANGLES_60Deg))
					ai.PilotSkills.manoeuvre = MANOEUVRE_MOVEAWAY;
				else
					ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;

		if	(ai.morale 	< 	MORALE_POOR)
		{
			if (ai.PilotSkills.combatskill	<=	SKILL_POOR)
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
			else if (ai.PilotSkills.combatskill	<	SKILL_REGULAR)
				ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEFORHOME;
			else
				ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;

		}else
		{
			if (!(TargetFacingSubject(ANGLES_60Deg)))			//RDH 06Nov96
			{
				if (SubjectFacingTarget(ANGLES_60Deg))
					ai.PilotSkills.manoeuvre = MANOEUVRE_MOVEAWAY;
				else
					ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;
			}else
			{
				SelectTailToNoseManoeuvre();
			}
		}
	}
}
//------------------------------------------------------------------------------
//Procedure		PanicManoeuvres
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
void	AirStruc::PanicManoeuvres()
{
  			int startpoint = Math_Lib.rnd(6);
  			switch (startpoint)
  			{
  				case	0:
  					{
						if (!(M_SpinOut()))
							ai.PilotSkills.manoeuvre=MANOEUVRE_MILDSCISSORS;	  //RDH 12/06/99
  						break;
  					}
  				case	1:
  					{
						if (		(ai.PilotSkills.combatskill < SKILL_REGULAR)
								&&	(classtype->phrasename == PHRASE_MIGS)
							)
							ai.PilotSkills.manoeuvre = MANOEUVRE_BAILOUT;
						else if (World.Y > FT_3000)
	  						ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEFORHOME;
						else
							ai.PilotSkills.manoeuvre= MANOEUVRE_GOHOME;
  						break;
  					}
  				case	2:
  					{
						if (World.Y > FT_3000)
  							ai.PilotSkills.manoeuvre = MANOEUVRE_STRAIGHTDIVE;
						else
  							ai.PilotSkills.manoeuvre = MANOEUVRE_MILDSCISSORS;
						break;
  					}
  				case	3:
  					{
  						ai.PilotSkills.manoeuvre = MANOEUVRE_MILDSCISSORS;
  						break;
  					}
  				case	4:
  					{
						ai.PilotSkills.manoeuvre = MANOEUVRE_PANICTURN;
  						break;
  					}
  				case	5:
  					{
						if (classtype->phrasename == PHRASE_MIGS)
						ai.PilotSkills.manoeuvre = MANOEUVRE_BAILOUT;
						else
							ai.PilotSkills.manoeuvre = MANOEUVRE_PANICTURN;

  						break;
  					}

  			}

			FormationInfo.advantage = FALSE;
			FormationInfo.disadvantage = FALSE;
}
//------------------------------------------------------------------------------
//Procedure		BanditHighManoeuvre
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
void	AirStruc::BanditHighManoeuvre()
{
	if	(TargetFacingSubject(ANGLES_45Deg))
	{
		if	(ai.morale > MORALE_MEDIUM)
		{
			ai.PilotSkills.manoeuvre = MANOEUVRE_GAINHEIGHT;
		}else
			PanicManoeuvres();

	}else
	{
		if	(ai.morale 	> 	MORALE_MEDIUM)
		{
			if (Math_Lib.rnd(2))
			{
				if (classtype->phrasename == PHRASE_MIGS)
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_GAINHEIGHT,MANOEUVRE_ZOOM);
				else
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_BREAKTURN);
			}else
			ai.PilotSkills.manoeuvre = MANOEUVRE_GAINHEIGHT;
		}else
			ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;

		
	}
}

void	AirStruc::LowAltManoeuvre()
{
}
Bool	AirStruc::TooLow()
{
	Bool retval = FALSE;
	if  (	(fly.cpitch >> ANGLES_180Deg)
		&&	(World.Y < HIGHESTGROUND)
		)
	{

		FP fvely = vely;
		FP TimeToLevelOut = -((FP)pitch - 65536.0) * 0.055;	// cs	@ 10deg/s
		FP AltLoss        = -fvely * TimeToLevelOut * 0.055;	//mm/cs * cs * 0.5 * 1.1 -> cm
		AltLoss -= 25.6 * fvely;

		int groundlevel = Land_Scape.GetGroundLevel(this);
		if(ai.PilotSkills.simpleacm)
			groundlevel += 250;
		else
			groundlevel += 25;

		if(groundlevel > World.Y - AltLoss)
			retval = TRUE;
	}

	return (retval);
}
//------------------------------------------------------------------------------
//Procedure		BanditLowManoeuvre
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
void	AirStruc::BanditLowManoeuvre()
{
	AirStrucPtr	unfriendly = *ai. unfriendly;

	if	(unfriendly->classtype->aerobaticfactor == AEROBATIC_LOW)
		ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;
	else if	(SubjectFacingTarget(ANGLES_45Deg))
	{
		if	(		(ai.morale > MORALE_MEDIUM)
				&& 	(ai.PilotSkills.combatskill < SKILL_REGULAR)
			)
			ai.PilotSkills.manoeuvre = MANOEUVRE_STAYONTAIL;
		else if (	(		(ai.morale > MORALE_MEDIUM)
						||	(Math_Lib.rnd(20))
					)
				)
		{
			if ((unfriendly->vel - vel) > MPH50)
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
			else
				ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;
		}
		else
			ai.PilotSkills.manoeuvre = MANOEUVRE_LAZYTURN;
		

	}else
	{
		MANOEUVRE tempman;
		if  (		(SubjectFacingTarget(ANGLES_90Deg))
				||	(
							(classtype->phrasename == PHRASE_MIGS)
						&&	(ai.PilotSkills.combatskill > SKILL_REGULAR)
					)
			)
			tempman = MANOEUVRE_TURNINGFIGHT;
		else
			tempman = MANOEUVRE_SPLITS;
		if	(		(ai.morale > MORALE_MEDIUM)
				|| (!Math_Lib.rnd(20))
			)
		{
			if (ai.morale < MORALE_GOOD)	
				ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,tempman);
			else
				ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,tempman,MANOEUVRE_TURNINGFIGHT);
		}
		else
			ai.PilotSkills.manoeuvre = MANOEUVRE_LAZYTURN;
	}
}
//------------------------------------------------------------------------------
//Procedure		ChooseRadioMessage
//Author		R. Hyde 
//Date			Tue 12 Mar 1996
//
//Description	
//
//Inputs
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::ChooseRadioMessage()
{
	ANGLES trgtlead = ActualLead(*ai.unfriendly, *this);

	if  (		(Range	< INSIDEWEAPONSRANGE)
			&&	(TargetHasCorrectLead(trgtlead, ANGLES_3Deg))
		)
	{
		AirStrucPtr	unfriendly = *ai. unfriendly;
		AirStrucPtr	unfbuddy = unfriendly->FindBuddy();
		if (unfbuddy && !unfbuddy->AcIsPlayer())
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_SHOOTHIM, MSG_SHOOTHIM, unfbuddy, this, unfriendly));
	}

 	if	 (		(Range	< DANGERRANGE)
			&&	(Math_Lib.rnd(RndValMAX) < RND10PC)
			&&	(TargetOnSubjectTail(ANGLES_45Deg))
		 )
	{
		SWord	droll = roll - ((AirStruc*)ai.unfriendly)->roll;
		if (droll < 0)
			droll = -droll;
		
		if (		(roll >> ANGLES_60Deg)
				&& 	(roll << ANGLES_300Deg)
				&&	(droll < 6536)
					
		   )
		{
			SWord	dhdg = fly.dhdg;
			if (dhdg < 0)
				dhdg = -dhdg;
			SWord	trgdhdg = ((AirStruc*)ai.unfriendly)->fly.dhdg;
			if (trgdhdg < 0)
				trgdhdg = -trgdhdg;
			SWord delta = trgdhdg - dhdg;
			if (		(delta > 0)
					&&	((dhdg * 100/Timer_Code.FRAMETIME) > ((SWord)ANGLES_5Deg))
					&&	((trgdhdg * 100/Timer_Code.FRAMETIME) > ((SWord)ANGLES_6Deg))
			   )
			{
				AirStruc*	callee = FindBuddy();
				if (callee)
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_OUTTURN, MSG_OUTTURN, this, NULL, callee));
			}
		}
	}



}
void	AirStruc::SetACMType()
{
	if	(	(		(ai.unfriendly == Persons2::PlayerGhostAC)
				||	(ai.attacker == Persons2::PlayerGhostAC)
				||	(this == Manual_Pilot.ControlledAC2)
			   	||	(Save_Data.gamedifficulty [GD_AIPILOTSALLTIME]
				|| (ai.unfriendly && ai.unfriendly->uniqueID.bitfieldUid.commsmove)//AMM 07May99
				|| (ai.attacker && ai.attacker->uniqueID.bitfieldUid.commsmove))//AMM 07May99
			)
		)
	{
		if(ai.PilotSkills.simpleacm)
		{
			Manual_Pilot.InitFlightModelPos(this);
		}
		ai.PilotSkills.simpleacm = FALSE;
	}
	else
		ai.PilotSkills.simpleacm = TRUE;
}

//------------------------------------------------------------------------------
//Procedure		SelectNextEngageManoeuvre
//Author		R. Hyde 
//Date			Tue 12 Mar 1996
//
//Description	
//
//Inputs		PitchIntercept, HdgIntercept and Range set 
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SelectNextEngageManoeuvre ()
{


  	SWord	startpoint;
	Bool	oldadvantage,olddisadvantage;
	MANOEUVRE	oldmanoeuvre = ai.PilotSkills.manoeuvre;

	SetACMType();


	oldadvantage = FormationInfo.advantage;
	olddisadvantage = FormationInfo.disadvantage;


	if (	ai.unfriendly==NULL
		||	ai.unfriendly->MigStatus.LaunchFlags.deaded						//AMM 30Nov98
		)
	{//break link with dead unfriendly
		ai.PilotSkills.ManStep = PHASE0;
		ai.PilotSkills.manoeuvre=MANOEUVRE_LOOKROUND;						//JIM 18Oct96
		if (	(ai.unfriendly)
			&&	(((AirStrucPtr)*ai.unfriendly)->ai.attacker==this)
			)
			((AirStrucPtr)*ai.unfriendly)->ai.attacker=NULL;
		ai.unfriendly=NULL;
		return;
	}


  	ai.PilotSkills.ManStep = PHASE0;
  

	if	(TimeToChangeToDisengage ())
	{
		if (AcIsGroupLeader())
		{
			AirStruc*	callee = FindABuddyWithPlayerGivenPriority();
			if	(Math_Lib.rnd(RndValMAX) < RND25PC)
	   			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_DISENGAGE_GOTPROBLEM, MSG_STATUSREPORT, this, NULL, callee));
			if	(Math_Lib.rnd(RndValMAX) < RND25PC)
	   			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_GOHOME_GOTPROBLEM, MSG_STATUSREPORT, this, NULL, callee));
		}
		FormationInfo.advantage = FALSE;
		FormationInfo.disadvantage = FALSE;
		SelectNextDisengageManoeuvre ();
		return;
	}
	ChooseRadioMessage();

	SLong d_alt = World.Y - ai.unfriendly->World.Y;
	if	(d_alt > FT_3000)
	{
 		BanditLowManoeuvre();
	}
	elseif	(d_alt < -FT_3000)
	{
 		BanditHighManoeuvre();
	}
  	elseif (			(ai.PilotSkills.combatskill  <= SKILL_POOR)
				   &&	(classtype->phrasename == PHRASE_MIGS)
			)
  	{
		FormationInfo.advantage = FALSE;
		FormationInfo.disadvantage = FALSE;
		RndVal	rndnum = Math_Lib.rnd();

		if (		(classtype->phrasename == PHRASE_MIGS)
				&&	(rndnum > RND99PC)
				&&	(Range	< INSIDEWEAPONSRANGE)
				&&	TargetFacingSubject(ANGLES_45Deg)
			)
			ai.PilotSkills.manoeuvre = MANOEUVRE_BAILOUT;
		else 
		{
			if (ai.PilotSkills.combatskill  < SKILL_POOR)
	  			ai.PilotSkills.manoeuvre = MANOEUVRE_STRAIGHTANDLEVEL;
			else
	  	 		ai.PilotSkills.manoeuvre = MANOEUVRE_LAZYTURN;
		}
 	}else if (		(ai.morale <= MORALE_POOR)
			   &&	(classtype->phrasename == PHRASE_MIGS)
			)
	{
 		PanicManoeuvres();
	}elseif	(SubjectFacingTarget(ANGLES_45Deg))
	{
		if (LooseDeuceSituation())
		   TakeLooseDeuceOption();
		else if (TargetFacingSubject(ANGLES_45Deg))
			SelectNoseToNoseManoeuvre();
		else if (SubjectOnTargetTail(ANGLES_45Deg))
			SelectNoseToTailManoeuvre();
		else
			SelectNoseToBeamManoeuvre();

		FormationInfo.advantage = TRUE;
		FormationInfo.disadvantage = FALSE;
  		
	}elseif (TargetOnSubjectTail(ANGLES_45Deg))
	{
		if (TargetFacingSubject(ANGLES_45Deg))
		{
			RndVal	rndpc = (RndVal)Math_Lib.rnd(RndValMAX);
			if (rndpc > RND75PC)
			{
				AirStruc*	callee = FindABuddyWithPlayerGivenPriority();
				if (rndpc > RND90PC)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_OUTTURN, MSG_OUTTURN, this, NULL, callee));
				else
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_TURNINSIDE, MSG_OUTTURN, this, NULL, callee));

			}
			
			SelectTailToNoseManoeuvre();
			FormationInfo.advantage = FALSE;
			FormationInfo.disadvantage = TRUE;

		}else if (SubjectOnTargetTail(ANGLES_45Deg))
		{
			SelectTailToTailManoeuvre();
			FormationInfo.advantage = FALSE;
			FormationInfo.disadvantage = FALSE;
		}else	
		{// target on subject tail and subject on target beam
			SelectTailToBeamManoeuvre();
			FormationInfo.advantage = FALSE;
			FormationInfo.disadvantage = FALSE;
		}
	}else
	{	if (LooseDeuceSituation())
			TakeLooseDeuceOption();
		else if (TargetFacingSubject(ANGLES_45Deg))
		{
			SelectBeamToNoseManoeuvre();
			FormationInfo.advantage = FALSE;
			FormationInfo.disadvantage = TRUE;

		}else if (SubjectOnTargetTail(ANGLES_45Deg))
		{
			SelectBeamToTailManoeuvre();
			FormationInfo.advantage = FALSE;
			FormationInfo.disadvantage = FALSE;
		}else
		{
			SelectBeamToBeamManoeuvre();
			FormationInfo.advantage = FALSE;
			FormationInfo.disadvantage = FALSE;
		}
	}
	if (oldmanoeuvre != ai.PilotSkills.manoeuvre)
	{

		if (_DPlay.Implemented)
		{
			if (this->CommsOwner==Persons2::PlayerSeenAC->uniqueID.bitfieldUid.count)	//AMM 11Jan99
			{												//AMM 11Jan99
				_DPlay.AddAIACToPositionList(this);			//AMM 11Jan99
			}												//AMM 11Jan99
		}
	}
//	if (	((oldadvantage != advantage)
//		||	(olddisadvantage != disadvantage) )
//		)
	{
		MoraleMod();
//		if (ai.unfriendly == Manual_Pilot.ControlledAC2)
//		{//change music
//##rdh temp			if (advantage == TRUE)
//			{
//				_Miles.SequenceAudible(FIL_MUSIC_DISADVANTAGE);
//			}elseif (disadvantage == TRUE)
//			{
//				_Miles.SequenceAudible(FIL_MUSIC_ADVANTAGE);
//			}else
//			{
//				_Miles.SequenceAudible(FIL_MUSIC_NEUTRAL);
//			}
//		}
	}

}


//------------------------------------------------------------------------------
void	AirStruc::MoraleMod()

{
	return;	//CSB 07/07/99	
}

//------------------------------------------------------------------------------
//Procedure		LooseDeuceSituation
//Author		R. Hyde 
//Date			Fri 20 Mar 1998
//
//Description	if not leader and on loose deuce and on same unfriendly as
//				leader take the option
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::LooseDeuceSituation()
{
	AirStrucPtr	unfriendly = *ai. unfriendly;

	if (	(leader)
		&&	(ai.elementtactics == LOOSEDEUCE)
		&&	(unfriendly == ((AirStrucPtr)leader)->ai.unfriendly)
		)
	{
		return (TRUE);
	}else
		return(FALSE);
}
//------------------------------------------------------------------------------
//Procedure		TakeLooseDeuceOption
//Author		R. Hyde 
//Date			Fri 20 Mar 1998
//
//Description	either take the shot or gets some distance for a better shot
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::TakeLooseDeuceOption()
{
	if	(		(SubjectFacingTarget(ANGLES_45Deg))
			&&	(Range < DANGERRANGE)
		)
	{
  	 	ai.PilotSkills.manoeuvre = 	MANOEUVRE_LEADPURSUIT;
	}else if(		((ai.PilotSkills.combatskill < SKILL_REGULAR)	&& (vel < CornerSpeed()))
				||	((ai.PilotSkills.combatskill > SKILL_REGULAR)	&& (vel < SustainedTurnSpeed()))
			)
	{
		if (ai.PilotSkills.combatskill < SKILL_REGULAR)	
	  	 	ai.PilotSkills.manoeuvre = 	MANOEUVRE_GAINHEIGHT;
		else
	  	 	ai.PilotSkills.manoeuvre = 	MANOEUVRE_CLIMBATSUSTAINEDTURNSPEED;
	}
	else	
		ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_IMMELMANN,MANOEUVRE_CLIMBATSUSTAINEDTURNSPEED);
}


//------------------------------------------------------------------------------
//Procedure		TimeToChangeToDisengage
//Author		R. Hyde 
//Date			Thu 14 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::TimeToChangeToDisengage ()

{
	WeapAnimData*	weapon;										//RJS 29Aug96
	SLong			xpos, ypos, zpos;							//RJS 29Aug96
	UWord			index;										//RJS 03Sep96
	AirStrucPtr	unfriendly = *ai. unfriendly;
	UWord			mvel,mdelay,mburst;							//RDH 31Jul98

	SWord combatfactor = CombatFactor() * 50;
//##	if (combatfactor < ai.combattime)
//		return (TRUE);

	if (	(unfriendly->classtype->aerobaticfactor > AEROBATIC_MEDIUM)
		&&	(classtype->aerobaticfactor == AEROBATIC_MEDIUM)	
		)
		{//prop fighter v jet
			return (TRUE);											//RDH 20Mar98
		}
	if	(Range > VISIBLERANGE)									//JIM 21Oct96
		return (TRUE);											//JIM 21Oct96

	//maintain the globals

	SLong		oldrange;											  //DAW 01/04/99
	ANGLES		oldHI, oldPI;
	oldrange = Range;
	oldHI = HdgIntercept;
	oldPI = PitchIntercept;
	Bool bingofuel = FuelShort (SECS100*60);
	Range			= oldrange;
	HdgIntercept	= oldHI;
	PitchIntercept	= oldPI;										  //DAW 01/04/99


	if (bingofuel)
		return (TRUE);												  //RDH 24/03/99

	
	if (AircraftDamaged())
		return (TRUE);

	for (index = 0; index < 6; index++)							//RDH 31Jul98
	{
		weapon = SHAPE.GetWeaponLauncher(this,index,xpos,ypos,zpos,mvel,mdelay,mburst,LT_BULLET);//RDH 31Jul98
		if (   (weapon)
			&& (weapon->LoadedStores <= 0)
			)
		{
			return (TRUE);
		}
	}



	if (fly.leadflight)											//ARM 18Sep96
		if (fly.leadflight->ai.PilotSkills.manoeuvre == MANOEUVRE_TOPCOVER)
			return (TRUE);

	if	(		(ai.morale < MORALE_MEDIUM)
			&&	(Range > (2 * WEAPONSRANGE))
			&&	(TargetOnSubjectTail(ANGLES_30Deg))				//JIM 07Oct96
		)
	{
		return (TRUE);
	}
	switch (ai.aggressionlevel)
	{
		case	AL_TRAINING:
		case	AL_DEFENSIVE:
			return(TRUE);
		case	AL_ATTACKBOMBERSONLY:
			if (unfriendly->classtype->aerobaticfactor == AEROBATIC_HIGHEST)
		 		return(DisengageAppropriate());
		case	AL_SPLITFORCE:
		case	AL_ATTACKWEAKNESSES:
		{
			if (unfriendly->classtype->aerobaticfactor < AEROBATIC_HIGHEST)
			{
		 		return (FALSE);
			}else if (WeHaveNumbersAdvantage ())
		 	{
		 		return (FALSE);
		 	}else
		 		return(DisengageAppropriate());
		}
		case	AL_ATTACKANYTHING:
				return (FALSE);

 	}
	return (FALSE);

}
  
//------------------------------------------------------------------------------
//Procedure		WeHaveNumbersAdvantage
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
Bool	AirStruc::WeHaveNumbersAdvantage ()
{
	SWord	unfriendlycount = 0;
	SWord	friendlycount = 1;

	if (ai.unfriendly !=NULL)
	{
		unfriendlycount = Art_Int.CountFormationSize (*ai.unfriendly);
		friendlycount = Art_Int.CountFormationSize (this);
		if (unfriendlycount > friendlycount)
		{
			return (FALSE);
		}else
		{
			return (TRUE);
		}
	}else
	{
			return (TRUE);

	}
}
//------------------------------------------------------------------------------
//Procedure		DisengageAppropriate
//Author		R. Hyde 
//Date			Thu 14 Dec 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::DisengageAppropriate ()

{
					if	(	(		(SubjectOnTargetTail(ANGLES_60Deg))
							&&	(TargetOnSubjectTail(ANGLES_60Deg))
							&&	(!SubjectFacingTarget(ANGLES_60Deg))
							&&	(!TargetFacingSubject(ANGLES_60Deg))
							&& (Range > (ENGAGERANGE))
							)
							||
							 (Range > COMBATRANGE)
						)
					{
						 return (TRUE);
					}else
					{
						return (FALSE);
					}
}
void	AirStruc::FrightenBogie()
{
	if (		(ai. unfriendly)
			&&	(ai. unfriendly->MigStatus.LaunchFlags.size==AIRSTRUCSIZE)
		)	
	{
    		AirStrucPtr	unfriendly = *ai. unfriendly;
		InterceptandRange (&ai.unfriendly->World);
		RndVal	rndpc = (RndVal)Math_Lib.rnd(RndValMAX);	

		if (		(rndpc < RND25PC)
				&&	(SubjectOnTargetTail(ANGLES_45Deg))
				&&	(SubjectFacingTarget(ANGLES_45Deg))
		   )
		{
			if (unfriendly->ai.morale <= MORALE_MEDIUM)
			{
				if	(		(Range > WEAPONSRANGE)
						&&	(Range < ENGAGERANGE)
					)
				{
		  			unfriendly->ai.PilotSkills.ManStep = PHASE0;
					unfriendly->ai.PilotSkills.manoeuvre = MANOEUVRE_MILDSCISSORS;
				}
			}else if (unfriendly->ai.morale <= MORALE_VERYGOOD)
			{
				if	(		(Range > WEAPONSRANGE)
						&&	(Range < DANGERRANGE)
					)
				{
		  			unfriendly->ai.PilotSkills.ManStep = PHASE0;
					unfriendly->ai.PilotSkills.manoeuvre = MANOEUVRE_SCISSORS;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		InformOfDanger
//Author		R. Hyde 
//Date			RDH 04/03/99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::InformOfDanger(AirStruc* caller, AirStruc* callee)
{
	//NOTE: the caller is the unfriendly of this
	//		HdgIntercept etc is f(caller wrt this)
	//		Standard rel hdg routines can be used

	RndVal	rndpc = (RndVal)Math_Lib.rnd(RndValMAX);	
	
	//comingin msg are for MiGs only
	if  (		(SubjectOnTargetTail(ANGLES_20Deg))
			&&	(rndpc > RND50PC)
			&&	(classtype->phrasename == PHRASE_MIGS)
			&&	SubjectFacingTarget(ANGLES_50Deg)
		)
	{
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_COMINGINBEHIND, MSG_INITIALCOMBAT, caller, this, callee));
	}else if (		(SubjectFacingTarget(ANGLES_30Deg))
				&&	(TargetFacingSubject(ANGLES_60Deg))
				&&	(rndpc > RND50PC)
				&&	(classtype->phrasename == PHRASE_MIGS)
			 )
	{
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_COMINGINAHEAD, MSG_INITIALCOMBAT, caller, this, callee));
	}else if (SubjectFacingTarget(ANGLES_40Deg))
	{	
		if (caller->formpos == 0) 
		{
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_INITALCOMBATLDR, MSG_INITIALCOMBAT, caller, this, callee));
		}else
		{
			if (		(classtype->phrasename == PHRASE_MIGS)
					||	(classtype->phrasename == PHRASE_YAK9S)
			   )
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_INITALCOMBATWING, MSG_INITIALCOMBAT, caller, this, callee));
			else
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_INITALCOMBATWING_MIGS, MSG_INITIALCOMBAT, caller, this, callee));
		}
	}else if (		(rndpc > RND50PC)
				&&	(classtype->phrasename == PHRASE_MIGS)
				&&	SubjectFacingTarget(ANGLES_90Deg)
			 )	
	{
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_COMINGINABEAM, MSG_INITIALCOMBAT, caller, this, callee));
	}
}  
//------------------------------------------------------------------------------
//Procedure		FirstTimeInCombatMsg
//Author		R. Hyde 
//Date			RDH 04/03/99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	AirStruc::OutNumbered(AirStruc* trg)
{

	if (		(trg->AutoMoveInfo.nationality == Persons2::PlayerGhostAC->AutoMoveInfo.nationality)
			&&	(!trg->PlayerInGroup())
	   )
	{
		//see if the target is outnumbered
		AirStruc* caller = FindGroupLeaderInFormation();
		int count = 0;
		while (caller->fly.nextflight)
		{
			AirStruc* agg = caller;
			while (agg)
			{
				if (agg->ai.unfriendly == trg)
					count++;
				agg = (AirStruc*)agg->follower;
			}
			caller = caller->fly.nextflight;		
		}
		if (count > 2)
		{
			ScriptTables script = SCRIPT_OUTNUMBERED;
			if (classtype->phrasename == PHRASE_MIGS)
				script = SCRIPT_OUTNUMBERED_MIGS;
			_Radio.TriggerMsg(MESSAGE_STRUC(script, MSG_INITIALCOMBAT, trg, NULL, Manual_Pilot.ControlledAC2));
			_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_OUTNUMBEREDINFO, MSG_INITIALCOMBATINFO_DELAY, VOICE_DENTIST , trg, Manual_Pilot.ControlledAC2)); //RDH 05/07/99
			return true;
		}else
			return false;
	}else
		return false;
}

//------------------------------------------------------------------------------
//Procedure		FirstTimeInCombatMsg
//Author		R. Hyde 
//Date			RDH 04/03/99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::FirstTimeInCombatMsg()
{
	AirStruc*	trg = (AirStruc*)ai.unfriendly;
	if (trg)
	{
		if (AutoMoveInfo.nationality != Persons2::PlayerGhostAC->AutoMoveInfo.nationality)
		{//player's enemy entering combat
			if (	(trg)	&&	(trg->ai.unfriendly))	// and trg knows of this //RDH 27/06/99
			{
				if (!formpos)				//only the group leader should provide a message
				{
					if (!OutNumbered(trg))
					{
						AirStruc* buddy = trg;
						buddy = trg->FindABuddyWithPlayerGivenPriority();

						if (TargetFacingSubject(ANGLES_80Deg))
						{
							if ( (buddy) &&	(!trg->AcIsPlayer()))
							if	(		((World.Y - trg->World.Y) > FT_3000)
									&&	(Math_Lib.rnd(RndValMAX) < RND50PC)
									&&	(classtype->phrasename == PHRASE_MIGS)
								)
							{
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_MIGSCOMING, MSG_MIGSCOMING, trg, this, buddy));
							}else
							{
								if (trg->follower)
								{
									if	(		(ai.morale 	> 	MORALE_GOOD)
											&& 	(ai.PilotSkills.combatskill < SKILL_REGULAR)
											&&	(Math_Lib.rnd(RndValMAX) < RND10PC)
											&&	(!trg->AcIsPlayer())
										)	
									{
										_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_FEELING, MSG_INITIALCOMBAT, buddy, NULL, trg));
										_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_FEELINGREPLY, MSG_INITIALCOMBATREPLY, trg, NULL, buddy));
									}
								}
								InformOfDanger(trg, buddy);
							}
						}else
						{//standard bandit call
							if ((buddy)	&&	(Range < VISIBLERANGE))		//RJS 09May99
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_BANDIT_RELPOS, MSG_MIGSCOMING, trg, this, buddy));
						}
					}
				}
			}
		}else
		{
			AirStruc* ldr = FindGroupLeader();

			
			if  (		(ldr->AcIsPlayer())
					&&	 (Art_Int.AircraftAreClose(this,ldr))
					&&	(!weap.AircraftStatus.FuelDumped)
					
				)													  //RDH 10/05/99
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_JETTISONREQUEST, MSG_STATUSREPORT, this, NULL, Manual_Pilot.ControlledAC2));

			if  (		(trg->ai.unfriendly) 
					&& (trg->ai.unfriendly->MigStatus.LaunchFlags.size == AIRSTRUCSIZE)
					&&	(trg->ai.unfriendly != this)				  //RDH 18/06/99
				)
			{
					if  (		(!leader)
							||	(		(follower)
									&&	(ai.elementtactics != WELDEDWING)
								)
						)
						
					{
							InterceptandRange (&trg->World, &trg->ai.unfriendly->World);
							if (Art_Int.CallerOnTargetTail(this, trg, ANGLES_45Deg))
							{
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ENGAGETAIL, MSG_STATUSREPORT, this, NULL, trg->ai.Unfriendly()));
									if	(	(Math_Lib.rnd(RndValMAX) < RND25PC)	&& (Range < DANGERRANGE))	
										_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ENGAGETAIL_REPLY, MSG_STATUSREPORT_REPLY_DELAY, trg->ai.Unfriendly(), NULL, this));
							}else
								_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ENGAGEBANDIT, MSG_STATUSREPORT, this, NULL, trg->ai.Unfriendly()));
							InterceptandRange (&ai.unfriendly->World);
					}
			}
			if  (		(!leader)
					&&	(follower)
					&&	(ai.elementtactics == WELDEDWING)
				)		
			{
				_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_COVERME_RAND, MSG_COVERME, this, trg, Follower()));
				if (Math_Lib.rnd(RndValMAX) < RND50PC)	
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_ROGER, (MsgType)(MSG_COVERMEREPLYDELAY+MSG_BLOCKCHATTER), Follower(),NULL,this));//RJS 06Jul99
			}
		}
	}
}
//------------------------------------------------------------------------------
//Procedure		ACM
//Author		R. Hyde 
//Date			Fri 8 Mar 1996				
//
//Description	ACM movement routine
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::AirCombat()
{

	ACMMODEL& ACM = *fly.pAcmModel;
	MODEL& MOD = *fly.pModel;

/* TEST CODE CSB 01/06/99 */

	UWord EveryXcs;	
	if(ai.PilotSkills.simpleacm)
		EveryXcs = EVERY256cs;
	else
		EveryXcs = EVERY64cs;
	if(!(MOD.FrameCount & EveryXcs))	//This will do the following commands for each aircraft
	{								//once every n operations.
		if  (		(TooLow())
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_LOWALT)
			)
		{
			ai.PilotSkills.ManStep = PHASE0;
			ai.PilotSkills.manoeuvre = MANOEUVRE_LOWALT;
		}
	}
	EveryXcs = EVERY512cs;

	if  (			!(MOD.FrameCount & EveryXcs)
//				&&	(ai.unfriendly != Persons2::PlayerGhostAC)
				&&	(ai.PilotSkills.combatskill >= SKILL_REGULAR)
				&&	(AutoMoveInfo.nationality != Persons2::PlayerGhostAC->AutoMoveInfo.nationality)
		)
	{
		InterceptandRange (&Persons2::PlayerGhostAC->World);
		int spotrange = METRES400 + (ai.PilotSkills.combatskill - SKILL_REGULAR) * METRES50;
		if (		(TargetOnSubjectTail(ANGLES_45Deg))
				&&	(Range < spotrange)
				&&	((HdgIntercept + ANGLES_180Deg + ANGLES_15Deg - Persons2::PlayerGhostAC->hdg) << (ANGLES_30Deg))
			)
		{
			AirStruc* buddy	= FindBuddy();
			if  (buddy)
			{//unfriendly is attacked by aircraft 
			   BreakCallandReaction(buddy, this,Persons2::PlayerGhostAC);
			}

		}
	}

//#pragma warnmsg("Paul: need test for accel")
	if (currworld->vp && currworld->vp->Accel())				//AMM 13Apr99
	{//if in accel, break combat if manoeuvre does not involve unfriendly
		if (ai.PilotSkills.manoeuvre<=MANOEUVRE_TOPCOVER)
		{
			MOD.Rudder = 0;
			Art_Int.BreakOff(this);
		}
	}else
	{
		if		( 	(!ai.PilotSkills.simpleacm)				//don't need to attempt to recover from simple model
				&&	(MOD.Spinning)
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_SPINOUT)
 				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_ACCIDENTALSPINOUT)
 				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_SPINRECOVERY)
 				)
		{
		  		ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre=MANOEUVRE_SPINRECOVERY;
		}else if (ai.unfriendly==NULL)
		{
			if (	(	(ai.PilotSkills.manoeuvre != MANOEUVRE_TOPCOVER)			//JIM 21Aug96
					&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_LOOKROUND)			//JIM 21Aug96
					&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_DIVEFORHOME)			//JIM 15Oct96
					&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_CLIMBFORHOME)		//JIM 15Oct96
					&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_STRAIGHTDIVE)		//JIM 15Oct96
					&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_GOHOME )				//JIM 15Oct96
					&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_SPINRECOVERY )				//JIM 15Oct96
					&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_STRAIGHTANDLEVEL)		//rdh 5/1/99
					&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_WELDEDWINGMAN)	  //RDH 21/02/99
					)
				)
			{
				MOD.Rudder = 0;
		  		ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre=MANOEUVRE_LOOKROUND;
			}
		}else
			 InterceptandRange (&ai.unfriendly->World);

		if  (		(ai.PilotSkills.ManStep == PHASERESET)
				||	(ai.PilotSkills.manoeuvre == MANOEUVRE_SELECT)
			)
		{//the first time in either ai.PilotSkills.ManStep must be equal to PHASERESET
		 //or ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT 
			FirstTimeInCombatMsg();
			Manual_Pilot.InitFlightModelPos(this);
			//drop bombs
			WorldStuff*	worldptr = mobileitem::currworld;
			
			AirStruc* leader = FindGroupLeader();
			if (leader != Persons2::PlayerGhostAC)	//only drop stores if leader is not player
				Trans_Obj.DropAllStores((mobileitem* )this,*worldptr);	//RJS 27May98
			ai.PilotSkills.ManStep = PHASE0;
			SetACMType();
			
//##test
//			ai.PilotSkills.ManStep = PHASE0;
//			ai.PilotSkills.manoeuvre= MANOEUVRE_INTERCEPTHIGH;
			//ai.PilotSkills.manoeuvre= MANOEUVRE_HEADON;
			

		}
		if (ai.PilotSkills.manoeuvre == MANOEUVRE_SELECT)
		{
			SelectNextEngageManoeuvre ();

		}
		if (ai.PilotSkills.manoeuvre == MANOEUVRE_SELECT)
		{
			//##
//			SelectNextEngageManoeuvre ();
			Art_Int.BreakOff(this);									//JIM 16Jul96
			MoveItem(this,*currworld);								//JIM 16Jul96
		}else
		{

			if (ai.PilotSkills.simpleacm)
				SimpleACM ();
			else
				ManualACM ();
		}
	}

}

//------------------------------------------------------------------------------
//Procedure		SimpleACM
//Author		R. Hyde 
//Date			Fri 11 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SimpleACM ()
{
	
	switch (ai.PilotSkills.manoeuvre)
		{
			case MANOEUVRE_LOOKROUND:
			{						  
 				SimpleLookAround ();		  
				break;				  
			}			
			case MANOEUVRE_WELDEDWINGMAN:										//RDH 03Mar98
			{
				SimpleWeldedWingMan();
				break;
			}
			case MANOEUVRE_BAILOUT:										//RDH 03Mar98
			{
				SimpleBailOut();
				break;
			}
			case MANOEUVRE_TOPCOVER:
			{						  
				SimpleTopCover ();		  
				break;				  
			}			
			case MANOEUVRE_LINEABREAST:										//RDH 27Mar98
			{
 				SimpleLineAbreast();
				break;
			}
			case MANOEUVRE_SCATTER:											//RDH 27Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_PINCER:											//RDH 27Mar98
			{
 				SimplePincer();
				break;
			}
			case MANOEUVRE_MULTIWAVE:										//RDH 27Mar98
			{
 				SimpleMultiWave();
				break;
			}
			case MANOEUVRE_DIVEANDZOOM:
			{
 				SimpleDiveAndZoom();
				break;
			}
			case MANOEUVRE_LEADPURSUIT:
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_LAGPURSUIT:
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_CIRCUMVENT:										//RDH 27Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_ROUNDABOUT:										//RDH 27Mar98
			{
				SimpleRoundAbout();
				break;
			}
			case MANOEUVRE_SPLITMANOEUVRE:									//RDH 27Mar98
			{
				SimpleSplitManoeuvre();
				break;
			}
			case MANOEUVRE_HEADON:
			{
				SimpleHeadOn();
				break;
			}
			case MANOEUVRE_LINEASTERN:										//RDH 27Mar98
			{
				SimpleLineAstern();
				break;
			}
			case MANOEUVRE_BARRELROLLATTACK:							
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_SCISSORS:							
			{													
				SimpleScissors ();									
				break;											
			}													
			case MANOEUVRE_MILDSCISSORS:						
			{													
				SimpleMildScissors ();								
				break;											
			}													
			case MANOEUVRE_SPLITS:								
			{													
				SimpleSplitS ();										
				break;											
			}													
			case MANOEUVRE_ZOOMANDDROP:							
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_STRAIGHTANDLEVEL :					
			{													
				SimpleStraightandLevel ( );							
				break;											
			}													
			case MANOEUVRE_SPINOUT :							
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_DIVEFORHOME :						
			{													
				SimpleDiveforHome ( );								
				break;											
			}													
			case MANOEUVRE_GOHOME :								
			{													
				SimpleGoHome ( );										
				break;											
			}													
			case MANOEUVRE_MAKEFORFRIENDLY :							
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_MOVEAWAY :							
			{													
				SimpleMoveAway ( );									
				break;											
			}													
			case MANOEUVRE_ACCIDENTALSPINOUT :					
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_VERTICALLOOP :						
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_IMMELMANNTURN :						
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_HEADONOFFSET :						
			{													
				SimpleHeadOnOffset ( );								
				break;											
			}													
			case MANOEUVRE_IMMELMANN :						
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_STAYWITHPREY :						
			{													
//will not implement 				SimpleStayWithPrey ( );								
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_CLIMBFORHOME :						
			{													
				SimpleClimbforHome ( );								
				break;											
			}													
			case MANOEUVRE_STRAIGHTDIVE:						
			{													
				SimpleStraightDive ( );								
				break;											
			}													
			case MANOEUVRE_STANDONTAIL:							
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_SHOOTTOFRIGHTEN:						
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_SHOOTTOOEARLY:						
			{													
				SimpleTurningFight();
				break;											
			}													
			case MANOEUVRE_GAINHEIGHT:							
			{													
	 			SimpleGainHeight ( );									
				break;											
			}													
			case MANOEUVRE_LAGROLL:											//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_EXTENSION:								//RDH 30Mar98
			{
				SimpleMoveAway ( );									  //RDH 16/05/99

				break;
			}
			case MANOEUVRE_DIVINGSPIN:
			{
				SimpleSplitS();
				break;
			}
			case MANOEUVRE_REVERSETURN:										//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_SELFASBAIT:										//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_JINK:												//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_BREAKTURN:										//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_LAZYTURN:											//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_BREAK180:											//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_BREAKLOW:											//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_BREAKHIGH:										//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_BREAK90:											//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_HIGBARRELROLL:									//RDH 30Mar98
			{
				SimpleHiGBarrelRoll();
				break;
			}
			case MANOEUVRE_PANICTURN:									//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_UNBALANCEDFLIGHT:									//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_LOWALT:									//RDH 30Mar98
			{
				SimpleLowAlt();
				break;
			}
			case MANOEUVRE_SNAPSHOT:											//RDH 30Mar98
			{
//will not be used			SimpleSnapShot();
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_STAYONTAIL:										//RDH 30Mar98
			{
//will not be used 				SimpleStayOnTail();
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_TURNTOHDGANDPITCH:								//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_HOTSIDELAGPURSUITFIGHT:							//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_SANDWICHMANOEUVRES:								//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_TURNINGFIGHT:										//RDH 30Mar98
			case MANOEUVRE_SUSTAINEDTURN:									//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_HIGHYOYO:											//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_ZOOM:											//RDH 30Mar98
			{
 				SimpleZoom();
				break;
			}
			case MANOEUVRE_LOWYOYO:											//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_INTERCEPTHIGH:									//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_CLIMBATSUSTAINEDTURNSPEED:						//RDH 30Mar98
			{
				SimpleTurningFight();
				break;
			}
			case MANOEUVRE_GAINSPEED:						
			{													
				SimpleGainSpeed ( );								
				break;											
			}													
			case MANOEUVRE_SPINRECOVERY:						
			{													
				SimpleTurningFight();
				break;											
			}													
		}

	hdg += (Angles) CalcHdg ();

	SimpleCalcVel (); 
	if( (2 * vel < 3 * classtype->minvel) && (ai.PilotSkills.manoeuvre != MANOEUVRE_SPINRECOVERY) )
	{
		if(ai.PilotSkills.manoeuvre != MANOEUVRE_GAINSPEED)
			ai.PilotSkills.ManStep = 0;
		ai.PilotSkills.manoeuvre = MANOEUVRE_GAINSPEED;
	}
	CalcXYZVel();
	if((ai.unfriendly!=NULL) && (ai.PilotSkills.manoeuvre != MANOEUVRE_WELDEDWINGMAN))
		AutoShoot(TRUE);
	NewPosition ();
	SetFlightParams ();

	weap.ShootDelay = weap.ShootDelay-Timer_Code.FRAMETIME;
	if (weap.ShootDelay < 0)
		weap.ShootDelay = 0;

	if (ai.PilotSkills.manoeuvre == MANOEUVRE_SELECT)
	{
		if (ai.unfriendly==NULL)
		{
			if (	(ai.PilotSkills.manoeuvre != MANOEUVRE_TOPCOVER)			//JIM 21Aug96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_LOOKROUND)			//JIM 21Aug96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_DIVEFORHOME)			//JIM 15Oct96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_CLIMBFORHOME)		//JIM 15Oct96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_STRAIGHTDIVE)		//JIM 15Oct96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_GOHOME )				//JIM 15Oct96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_SPINRECOVERY )				//JIM 15Oct96
				)
			{
		  		ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre=MANOEUVRE_LOOKROUND;
			}
		}else
		{
			InterceptandRange (&ai.unfriendly->World);
			SelectNextEngageManoeuvre ();
		  		ai.PilotSkills.ManStep = PHASE0;
		}
	}
	if (ai.PilotSkills.manoeuvre == MANOEUVRE_SELECT)
	{
		AutoMoveInfo.movecode=AUTO_FOLLOWWP;
	}

//temp	if (ai.unfriendly!=NULL)
//	{
//		AutoShoot(FALSE);

//	}
#ifdef ACM_PILOT_DATA

	PrintVar(0,  10, "T Pitch %.1f ", (FP)( (PitchIntercept - pitch) / 182.04) );
	PrintVar(0,  11, "T Headg %.1f ", (FP)( (HdgIntercept - hdg)  / 182.04) );
	PrintVar(0,  12, "T Range %.0f ", (FP)(Range / 100) );
	PrintVar(0,  13, "D Headg %.1f ", (FP)(ai.desiredhdg / 182.04) );

	PrintVar(20,   9, "C Vel   %.1f ", (FP)( (vel) / 10) );
	PrintVar(20,  10, "C Pitch %.1f ", (FP)( (pitch) / 182.04) );
	PrintVar(20,  11, "C Roll  %.1f ", (FP)( (roll) / 182.04) );
	PrintVar(20,  12, "C Headg %.1f ", (FP)( (hdg) / 182.04) );
	PrintVar(20,  13, "C Alt   %.0f ", (FP)( (World.Y) / 100.0) );
	PrintVar(20,  14, "C Thrst %.0f ", (FP)( (fly.thrustpercent) ) );


	
	switch (ai.PilotSkills.manoeuvre)
	{
		case MANOEUVRE_SELECT:						{ PrintString(0,  24, "MANOEUVRE_SELECT						");    break; }
		case MANOEUVRE_LOOKROUND:					{ PrintString(0,  24, "MANOEUVRE_LOOKROUND					");    break; }
		case MANOEUVRE_WELDEDWINGMAN:				{ PrintString(0,  24, "MANOEUVRE_WELDEDWINGMAN				");    break; }
		case MANOEUVRE_BAILOUT:						{ PrintString(0,  24, "MANOEUVRE_BAILOUT					");    break; }
		case MANOEUVRE_TOPCOVER:					{ PrintString(0,  24, "MANOEUVRE_TOPCOVER					");    break; }
		case MANOEUVRE_LINEABREAST:					{ PrintString(0,  24, "MANOEUVRE_LINEABREAST				");    break; }
		case MANOEUVRE_SCATTER:						{ PrintString(0,  24, "MANOEUVRE_SCATTER					");    break; }
		case MANOEUVRE_PINCER:						{ PrintString(0,  24, "MANOEUVRE_PINCER						");    break; }
		case MANOEUVRE_MULTIWAVE:					{ PrintString(0,  24, "MANOEUVRE_MULTIWAVE					");    break; }
		case MANOEUVRE_DIVEANDZOOM:					{ PrintString(0,  24, "MANOEUVRE_DIVEANDZOOM				");    break; }
		case MANOEUVRE_LEADPURSUIT:					{ PrintString(0,  24, "MANOEUVRE_LEADPURSUIT				");    break; }
		case MANOEUVRE_LAGPURSUIT:					{ PrintString(0,  24, "MANOEUVRE_LAGPURSUIT					");    break; }
		case MANOEUVRE_CIRCUMVENT:					{ PrintString(0,  24, "MANOEUVRE_CIRCUMVENT					");    break; }
		case MANOEUVRE_ROUNDABOUT:					{ PrintString(0,  24, "MANOEUVRE_ROUNDABOUT					");    break; }
		case MANOEUVRE_SPLITMANOEUVRE:				{ PrintString(0,  24, "MANOEUVRE_SPLITMANOEUVRE				");    break; }
		case MANOEUVRE_HEADON:						{ PrintString(0,  24, "MANOEUVRE_HEADON						");    break; }
		case MANOEUVRE_LINEASTERN:					{ PrintString(0,  24, "MANOEUVRE_LINEASTERN					");    break; }
		case MANOEUVRE_BARRELROLLATTACK:			{ PrintString(0,  24, "MANOEUVRE_BARRELROLLATTACK			");    break; }
		case MANOEUVRE_SCISSORS:					{ PrintString(0,  24, "MANOEUVRE_SCISSORS					");    break; }
		case MANOEUVRE_MILDSCISSORS:				{ PrintString(0,  24, "MANOEUVRE_MILDSCISSORS				");    break; }
		case MANOEUVRE_TURNINGFIGHT:				{ PrintString(0,  24, "MANOEUVRE_TURNINGFIGHT				");    break; }
		case MANOEUVRE_SPLITS:						{ PrintString(0,  24, "MANOEUVRE_SPLITS						");    break; }
		case MANOEUVRE_ZOOMANDDROP:					{ PrintString(0,  24, "MANOEUVRE_ZOOMANDDROP				");    break; }
		case MANOEUVRE_STRAIGHTANDLEVEL:			{ PrintString(0,  24, "MANOEUVRE_STRAIGHTANDLEVEL			");    break; }
		case MANOEUVRE_SPINOUT:						{ PrintString(0,  24, "MANOEUVRE_SPINOUT					");    break; }
		case MANOEUVRE_DIVEFORHOME:					{ PrintString(0,  24, "MANOEUVRE_DIVEFORHOME				");    break; }
		case MANOEUVRE_GOHOME:						{ PrintString(0,  24, "MANOEUVRE_GOHOME						");    break; }
		case MANOEUVRE_MAKEFORFRIENDLY:				{ PrintString(0,  24, "MANOEUVRE_MAKEFORFRIENDLY			");    break; }
		case MANOEUVRE_MOVEAWAY:					{ PrintString(0,  24, "MANOEUVRE_MOVEAWAY					");    break; }
		case MANOEUVRE_ACCIDENTALSPINOUT:			{ PrintString(0,  24, "MANOEUVRE_ACCIDENTALSPINOUT			");    break; }
		case MANOEUVRE_VERTICALLOOP:				{ PrintString(0,  24, "MANOEUVRE_VERTICALLOOP				");    break; }
		case MANOEUVRE_IMMELMANNTURN:				{ PrintString(0,  24, "MANOEUVRE_IMMELMANNTURN				");    break; }
		case MANOEUVRE_HEADONOFFSET:				{ PrintString(0,  24, "MANOEUVRE_HEADONOFFSET				");    break; }
		case MANOEUVRE_IMMELMANN:					{ PrintString(0,  24, "MANOEUVRE_IMMELMANN					");    break; }
		case MANOEUVRE_STAYWITHPREY:				{ PrintString(0,  24, "MANOEUVRE_STAYWITHPREY				");    break; }
		case MANOEUVRE_CLIMBFORHOME:				{ PrintString(0,  24, "MANOEUVRE_CLIMBFORHOME				");    break; }
		case MANOEUVRE_STRAIGHTDIVE:				{ PrintString(0,  24, "MANOEUVRE_STRAIGHTDIVE				");    break; }
		case MANOEUVRE_STANDONTAIL:					{ PrintString(0,  24, "MANOEUVRE_STANDONTAIL				");    break; }
		case MANOEUVRE_SHOOTTOFRIGHTEN:				{ PrintString(0,  24, "MANOEUVRE_SHOOTTOFRIGHTEN			");    break; }
		case MANOEUVRE_SHOOTTOOEARLY:				{ PrintString(0,  24, "MANOEUVRE_SHOOTTOOEARLY				");    break; }
		case MANOEUVRE_GAINHEIGHT:					{ PrintString(0,  24, "MANOEUVRE_GAINHEIGHT					");    break; }
		case MANOEUVRE_LAGROLL:						{ PrintString(0,  24, "MANOEUVRE_LAGROLL					");    break; }
		case MANOEUVRE_EXTENSION:					{ PrintString(0,  24, "MANOEUVRE_EXTENSION					");    break; }
		case MANOEUVRE_DIVINGSPIN:					{ PrintString(0,  24, "MANOEUVRE_DIVINGSPIN					");    break; }
		case MANOEUVRE_REVERSETURN:					{ PrintString(0,  24, "MANOEUVRE_REVERSETURN				");    break; }
		case MANOEUVRE_SELFASBAIT:					{ PrintString(0,  24, "MANOEUVRE_SELFASBAIT					");    break; }
		case MANOEUVRE_JINK:						{ PrintString(0,  24, "MANOEUVRE_JINK						");    break; }
		case MANOEUVRE_BREAKTURN:					{ PrintString(0,  24, "MANOEUVRE_BREAKTURN					");    break; }
		case MANOEUVRE_LAZYTURN:					{ PrintString(0,  24, "MANOEUVRE_LAZYTURN					");    break; }
		case MANOEUVRE_BREAK180:					{ PrintString(0,  24, "MANOEUVRE_BREAK180					");    break; }
		case MANOEUVRE_BREAKLOW:					{ PrintString(0,  24, "MANOEUVRE_BREAKLOW					");    break; }
		case MANOEUVRE_BREAKHIGH:					{ PrintString(0,  24, "MANOEUVRE_BREAKHIGH					");    break; }
		case MANOEUVRE_BREAK90:						{ PrintString(0,  24, "MANOEUVRE_BREAK90					");    break; }
		case MANOEUVRE_HIGBARRELROLL:				{ PrintString(0,  24, "MANOEUVRE_HIGBARRELROLL				");    break; }
		case MANOEUVRE_PANICTURN:					{ PrintString(0,  24, "MANOEUVRE_PANICTURN					");    break; }
		case MANOEUVRE_UNBALANCEDFLIGHT:			{ PrintString(0,  24, "MANOEUVRE_UNBALANCEDFLIGHT			");    break; }
		case MANOEUVRE_LOWALT:						{ PrintString(0,  24, "MANOEUVRE_LOWALT						");    break; }
		case MANOEUVRE_SNAPSHOT:					{ PrintString(0,  24, "MANOEUVRE_SNAPSHOT					");    break; }
		case MANOEUVRE_STAYONTAIL:					{ PrintString(0,  24, "MANOEUVRE_STAYONTAIL					");    break; }
		case MANOEUVRE_TURNTOHDGANDPITCH:			{ PrintString(0,  24, "MANOEUVRE_TURNTOHDGANDPITCH			");    break; }
		case MANOEUVRE_HOTSIDELAGPURSUITFIGHT:		{ PrintString(0,  24, "MANOEUVRE_HOTSIDELAGPURSUITFIGHT		");    break; }
		case MANOEUVRE_SANDWICHMANOEUVRES:			{ PrintString(0,  24, "MANOEUVRE_SANDWICHMANOEUVRES			");    break; }
		case MANOEUVRE_SUSTAINEDTURN:				{ PrintString(0,  24, "MANOEUVRE_SUSTAINEDTURN				");    break; }
		case MANOEUVRE_HIGHYOYO:					{ PrintString(0,  24, "MANOEUVRE_HIGHYOYO					");    break; }
		case MANOEUVRE_ZOOM:						{ PrintString(0,  24, "MANOEUVRE_ZOOM						");    break; }
		case MANOEUVRE_LOWYOYO:						{ PrintString(0,  24, "MANOEUVRE_LOWYOYO					");    break; }
		case MANOEUVRE_INTERCEPTHIGH:				{ PrintString(0,  24, "MANOEUVRE_INTERCEPTHIGH				");    break; }
		case MANOEUVRE_CLIMBATSUSTAINEDTURNSPEED:	{ PrintString(0,  24, "MANOEUVRE_CLIMBATSUSTAINEDTURNSPEED	");    break; }
		case MANOEUVRE_GAINSPEED:					{ PrintString(0,  24, "MANOEUVRE_GAINSPEED					");    break; }
		case MANOEUVRE_SPINRECOVERY:				{ PrintString(0,  24, "MANOEUVRE_SPINRECOVERY				");    break; }
	}

	PrintVar(30, 24, "Phase %.0f ", (FP)ai.PilotSkills.ManStep);
	PrintVar(40, 24, "Time %.0f ", (FP)manoeuvretime);
#endif
}

//------------------------------------------------------------------------------
//Procedure		NewDeadPosition
//Author		Robert Slater
//Date			Mon 23 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::NewDeadPosition()
{
	SLong	deltax,deltay,deltaz,groundlevel=0;					//RJS 14Apr99
	SLong	timedelta = (Timer_Code.FRAMETIME << 16) / 10;		//RJS 14Apr99
																//RJS 14Apr99
	deltax =  (velx * timedelta) >> 16;							//RJS 14Apr99
	deltay =  (vely * timedelta) >> 16;							//RJS 14Apr99
	deltaz =  (velz * timedelta) >> 16;							//RJS 14Apr99
			  													//RJS 14Apr99
	CalcNewPos (deltax,deltay,deltaz);							//RJS 14Apr99
}

//------------------------------------------------------------------------------
//Procedure		SubjectClosureGreaterThan
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description  
// 			
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
SLong AirStruc::SubjectClosure()
{
	ANGLES	brg;
	SWord	sinbearing,cosbearing;
	AirStrucPtr	unfriendly = *ai. unfriendly;

	brg = HdgIntercept - hdg;
	Math_Lib.high_sin_cos(brg,sinbearing,cosbearing);
	if (cosbearing < 0)
		return(FALSE);
	//cal the subject velocity along the hdg intercept (ie horizontal plane)
	SLong vel_s_hi_hori = velhori * cosbearing/ANGLES_FRACT;

//	brg = PitchIntercept - fly.cpitch;
	Math_Lib.high_sin_cos(PitchIntercept,sinbearing,cosbearing);
	//cal the subject hori velocity along the pitch intercept
	SLong vel_s_pi_hori = vel_s_hi_hori * cosbearing/ANGLES_FRACT;
	
	
	SLong vel_s_pi_vert = vely * sinbearing/ANGLES_FRACT;




	brg = HdgIntercept - unfriendly->hdg;
	Math_Lib.high_sin_cos(brg,sinbearing,cosbearing);
	SLong vel_t_hi_hori = unfriendly->velhori * cosbearing/ANGLES_FRACT;

//	brg = PitchIntercept - unfriendly->fly.cpitch;
	Math_Lib.high_sin_cos(PitchIntercept,sinbearing,cosbearing);
	//cal the subject hori velocity along the pitch intercept
	SLong vel_t_pi_hori = vel_t_hi_hori * cosbearing/ANGLES_FRACT;
	

	SLong vel_t_pi_vert = unfriendly->vely * sinbearing/ANGLES_FRACT;
	

	SLong	closure = (vel_s_pi_hori + vel_s_pi_vert) - (vel_t_pi_hori + vel_t_pi_vert); 

	return (closure);
}
//------------------------------------------------------------------------------
//Procedure		SubjectClosureGreaterThan
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description  
// 			
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool AirStruc::SubjectClosureGreaterThan(SLong dvel)
{
	SLong	closure;
	

	closure = SubjectClosure();
	if (closure > dvel)
		return(TRUE);
	else
		return(FALSE);
}
//------------------------------------------------------------------------------
//Procedure		SubjectLeadingTarget
//Author		R. Hyde 
//Date			Tue 10 Mar 1998
//
//Description	This is a simple method. A better one may be needed
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::SubjectLeadingTarget()
{
	Bool retval=FALSE;
	AirStrucPtr	unfriendly = *ai. unfriendly;

	if (unfriendly)
	{
		SWord beta = unfriendly->hdg - HdgIntercept;
		SWord alpha = hdg - HdgIntercept;
		if (beta > 0)
		{
			if (alpha > 0)
				retval=TRUE;
		}else{
			if (alpha < 0)
				retval=TRUE;
		}
	}
	return(retval);
}
//------------------------------------------------------------------------------
//Procedure		TargetClosureGreaterThan
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description  
// 			
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool AirStruc::TargetClosureGreaterThan(SLong dvel)
{
	SLong	closure;
	ANGLES	brg;
	SWord	sinbearing,cosbearing;
	AirStrucPtr	unfriendly = *ai. unfriendly;


	brg = ANGLES_180Deg + HdgIntercept - hdg;
	Math_Lib.high_sin_cos(brg,sinbearing,cosbearing);
	if (cosbearing < 0)
		return(FALSE);
	//cal the subject velocity along the hdg intercept (ie horizontal plane)
	SLong vel_s_hi_hori = velhori * cosbearing/ANGLES_FRACT;

	brg = (- PitchIntercept) - fly.cpitch;
	Math_Lib.high_sin_cos(brg,sinbearing,cosbearing);
	//cal the subject hori velocity along the pitch intercept
	SLong vel_s_pi_hori = vel_s_hi_hori * cosbearing/ANGLES_FRACT;
	

	SLong vel_s_pi_vert = vely * sinbearing/ANGLES_FRACT;




	brg = ANGLES_180Deg + HdgIntercept - unfriendly->hdg;
	Math_Lib.high_sin_cos(brg,sinbearing,cosbearing);
	SLong vel_t_hi_hori = unfriendly->velhori * cosbearing/ANGLES_FRACT;

	brg = ( - PitchIntercept) - unfriendly->fly.cpitch;
	Math_Lib.high_sin_cos(brg,sinbearing,cosbearing);
	//cal the subject hori velocity along the pitch intercept
	SLong vel_t_pi_hori = vel_t_hi_hori * cosbearing/ANGLES_FRACT;
	

	SLong vel_t_pi_vert = unfriendly->vely * sinbearing/ANGLES_FRACT;
	

	closure = (vel_t_pi_hori + vel_t_pi_vert) - (vel_s_pi_hori + vel_s_pi_vert); 


	if (closure > dvel)
		return(TRUE);
	else
		return(FALSE);
}

//------------------------------------------------------------------------------
//Procedure		ClosureLessThan
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
Bool AirStruc::ClosureLessThan(SLong dvel)
{
	SLong	closure;

	closure = SubjectClosure();

	if (closure < dvel)
		return(TRUE);
	else
		return(FALSE);
}

//------------------------------------------------------------------------------
//Procedure		M_LagRoll
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
Bool AirStruc::M_LagRoll()
{
			if (SubjectClosureGreaterThan(MPH50))
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_LAGROLL;
				return(TRUE);
			}else
				return(FALSE);
}
//------------------------------------------------------------------------------
//Procedure		M_FlightSeparation
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
Bool 	AirStruc::M_FlightSeparation()
{
	AirStrucPtr	unfriendly = *ai. unfriendly;

	SWord combatfactor = CombatFactor();
	if  (		(PitchIntercept >> ANGLES_180Deg)
			&&	(PitchIntercept << ANGLES_350Deg)
		)
		combatfactor += 10;
	if	( velhori > unfriendly->velhori)
		combatfactor += (velhori - unfriendly->velhori)/10;
	if  (	(combatfactor < 160)
		||	( classtype->aerobaticfactor < AEROBATIC_FIGHTER)
		)
		ai.PilotSkills.manoeuvre = MANOEUVRE_HEADON;
	else
		ai.PilotSkills.manoeuvre = MANOEUVRE_HEADONOFFSET;
	return(TRUE);
}
//------------------------------------------------------------------------------
//Procedure		AboveGround
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
bool AirStruc::AboveGround(SLong height)
{
	SLong lowestsafealt = Land_Scape.GroundHuggingAltitude(this);

	if (World.Y > (height + lowestsafealt))
		return(TRUE);
	else
		return(FALSE);

}
//------------------------------------------------------------------------------
//Procedure		
//Author		R. Hyde 
//Date			Tue 10 Mar 1998
//
//Description	nosetotail or nosetobeam range > danger
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::ManoeuvreBasedOnClosure()
{
	AirStrucPtr	unfriendly = *ai. unfriendly;

//##check allaspects for this
		if (SubjectClosureGreaterThan(MPH100))
		{
			if (	(ai.morale 	> 	MORALE_MEDIUM)
				)
			{
				if (SubjectLeadingTarget())
				{//turning fight changes to to lag roll if appropriate
						ai.PilotSkills.manoeuvre= MANOEUVRE_TURNINGFIGHT;
				}
//					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_LAGROLL, MANOEUVRE_TURNINGFIGHT);
					//ref1.68
				else
				{	
					if ((vel - unfriendly->vel) > MPH50)
						ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_LAGPURSUIT, MANOEUVRE_TURNINGFIGHT);
					else
						ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
				}
			}else if (ai.morale 	== 	MORALE_MEDIUM)
				ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_ZOOM, MANOEUVRE_EXTENSION);
			else
				ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;

		}else if(		(ai.PilotSkills.combatskill >= SKILL_VETERAN)
					&&  (AboveGround(FT_1000))
//rdh					&&	SubjectOnTargetTail(ANGLES_20Deg)
					&&	( Range > WEAPONSRANGE)
					&&	(ClosureLessThan(MPH15))
				)
			ai.PilotSkills.manoeuvre = MANOEUVRE_LOWYOYO;
		else if (SubjectLeadingTarget())
		{
			if (SubjectClosureGreaterThan(MPH50))
			{
				if (SubjectOnTargetTail(ANGLES_30Deg))
					ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
				else	
					ai.PilotSkills.manoeuvre = MANOEUVRE_HIGHYOYO;
			}
			else if (Range > WEAPONSRANGE)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_LEADPURSUIT;
			}else
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
		}
		else if (SubjectOnTargetTail(ANGLES_30Deg))
			ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
		else	
			ai.PilotSkills.manoeuvre = MANOEUVRE_HIGHYOYO;
		
}	
//------------------------------------------------------------------------------
//Procedure		M_BarrelRoll
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
Bool 	AirStruc::M_BarrelRoll()
{

	if  (	(!AircraftDamaged())
		&&	(	(fly.cpitch << ANGLES_50Deg)	||	(fly.cpitch >> ANGLES_310Deg))
		&&  (AboveGround(FT_500))
		)
	{
		ai.PilotSkills.manoeuvre = MANOEUVRE_BARRELROLLATTACK;
		return (TRUE);
	}else
		return(FALSE);

}
//------------------------------------------------------------------------------
//Procedure		M_ZoomAndDrop
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
Bool 	AirStruc::M_ZoomAndDrop()
{
	if  (	(fly.cpitch < ANGLES_10Deg)
		&&	(	(roll << ANGLES_20Deg)
			|| 	(roll >> ANGLES_340Deg)
			)
		&&	(ai.morale 	> 	MORALE_GOOD)
		&& 	(ai.PilotSkills.combatskill < SKILL_REGULAR)

		)
	{
		ai.PilotSkills.manoeuvre = MANOEUVRE_ZOOMANDDROP;
		return (TRUE);
	}else
		return(FALSE);

}
//------------------------------------------------------------------------------
//Procedure		M_SpinOut
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
Bool 	AirStruc::M_SpinOut()
{
	RndVal	rndnum = Math_Lib.rnd();
	if	(	(
					 (AircraftDamaged())
				&&  (AboveGround(FT_20000))
				&&	(rndnum > RND75PC)	
			)
		||	(	(	(ai.PilotSkills.combatskill < SKILL_REGULAR)
				&&  (AboveGround(FT_10000))
				&&	(rndnum > RND50PC)	
				)
			)
		)
	{
		ai.PilotSkills.manoeuvre = MANOEUVRE_SPINOUT;
		return (TRUE);
	}else
		return(FALSE);

}
//------------------------------------------------------------------------------
//Procedure		M_SelfAsBait
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	1.19
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool 	AirStruc::M_SelfAsBait()
{
	if (	(		(classtype->phrasename != PHRASE_MIGS)
				&&	(ai.PilotSkills.combatskill > SKILL_VETERAN)
				&&	(ai.morale 	> 	MORALE_MEDIUM)
				&&  (Range > DANGERRANGE)
			)
			||
			(		(classtype->phrasename == PHRASE_MIGS)
				&&	(ai.aggressionlevel == AL_ATTACKANYTHING)
				&&  (Range > DANGERRANGE)
			)
		)
	{
			ai.PilotSkills.manoeuvre = MANOEUVRE_SELFASBAIT;
			return (TRUE);
	}
	else
			return(FALSE);

}
//------------------------------------------------------------------------------
//Procedure		TargetHasLead
//Author		R. Hyde 
//Date			Thu 12 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::TargetHasCorrectLead(ANGLES lead, ANGLES maxlead)
{
	ANGLES delta = lead + maxlead;
	ANGLES anglerange = maxlead + maxlead;
	if ((delta) << (anglerange))
	{
	 	return (TRUE);
	}else
	{
		return (FALSE);
	}
}
//------------------------------------------------------------------------------
//Procedure		ActualLead
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	degrees from perfect lead, positive is excess lead
//
//------------------------------------------------------------------------------
ANGLES AirStruc::ActualLead(AirStrucPtr  subjt, AirStrucPtr trgt)
{
	COORDS3D	target;
	COORDS3D	subject;
	SLong		t,actualrange, oldrange;
	ANGLES		HItoTarget, oldHI, oldPI;


	oldrange = Range;
	oldHI = HdgIntercept;
	oldPI = PitchIntercept;

	subject.X = subjt->World.X;
	subject.Y = subjt->World.Y;
	subject.Z = subjt->World.Z;

	target.X = trgt->World.X;
	target.Y = trgt->World.Y;
	target.Z = trgt->World.Z;
	InterceptandRange (&subject, &target);
	HItoTarget = HdgIntercept;

	SLong	vx,vy,vz;

	CalcLead(subjt,trgt, vx,vy,vz, FALSE);
	target.X = trgt->World.X + vx;
	target.Y = trgt->World.Y + vy;
	target.Z = trgt->World.Z + vz;

	InterceptandRange (&subject, &target);
	ANGLES Lead;
	Lead = HdgIntercept - subjt->hdg;
	if (HdgIntercept >> HItoTarget)
		Lead = -Lead;

	Range			= oldrange;
	HdgIntercept	= oldHI;
	PitchIntercept	= oldPI;


	return (Lead);

}
//------------------------------------------------------------------------------
//Procedure		ManoeuvreBasedOn.PilotSkills.combatskill
//Author		Rod Hyde
//Date			Thu 12 Mar 1998
//
//Description	it is best to come out true
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
MANOEUVRE	AirStruc::ManoeuvreBasedOnSkill(SkillType combatskill, MANOEUVRE goodman, MANOEUVRE badman)
{
	if (ai.PilotSkills.combatskill < combatskill)
	{
		return (badman);
	}else
	{
		int	rndnum = Math_Lib.rnd((int) ai.PilotSkills.combatskill);
		if (rndnum < (int)combatskill)
		{
			return (badman);
		}
		else
		{
			return (goodman);
		}
	}
}
//------------------------------------------------------------------------------
//Procedure		SubjectHasEnergyAdvantage
//Author		Rod Hyde
//Date			Thu 12 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	AirStruc::SubjectHasEnergyAdvantage()
{
// PE or speed advantage
// or if not much in it, which has greater T/W ratio
	AirStrucPtr	unfriendly = *ai. unfriendly;

	if (		(DeltaAltitude() > FT_2000)
			||	((vel - unfriendly->vel) > MPH100)
			||	(		(DeltaAltitude() > -FT_1000)
					&&	((vel - unfriendly->vel) > -MPH50)
					&&	(classtype->phrasename == PHRASE_MIGS)
				)
		)
		return (TRUE);
	else
		return(FALSE);


}
//------------------------------------------------------------------------------
//Procedure		M_GunDefence
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	1.23..
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool 	AirStruc::M_GunDefence(ANGLES lead)
{
	MODEL& MOD = *fly.pModel;
	
	if (!TargetHasCorrectLead(lead, ANGLES_3Deg))
	{
		return(FALSE);
	}else
	{

		if (	(ai.PilotSkills.combatskill < SKILL_REGULAR)
			&&	(ai.morale > MORALE_GOOD)
			)
		{	//close quarter, self as bait
			if (Range < WEAPONSRANGE)
				ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKTURN;
			else
				ai.PilotSkills.manoeuvre = MANOEUVRE_LAZYTURN;
		}else
		{
			SWord combatfactor = CombatFactor();
			int rnd = Math_Lib.rnd((int)ai.morale);
			RndVal	rndnum = Math_Lib.rnd();
			if ( rnd <= (int)MORALE_POOR)
				 ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_SCISSORS,MANOEUVRE_MILDSCISSORS);
			else if	(		(rndnum < RND25PC)
						&&	(MOD.Inst.MachNo > 0.6)
					)
				ai.PilotSkills.manoeuvre= MANOEUVRE_ZOOM;
			else if (!M_SplitS())
			{
				if (	(TargetFacingSubject(ANGLES_5Deg))
					&&	(TargetOnSubjectTail(ANGLES_5Deg))
					&&	(TargetClosureGreaterThan(MPH50))
					)
				{
						ai.PilotSkills.manoeuvre = MANOEUVRE_HIGBARRELROLL;

				}else if(		(roll << ANGLES_20Deg)
							|| 	(roll >> ANGLES_340Deg)
						)
				{
					RndVal	rndnum = Math_Lib.rnd();
					if	(rndnum < RND25PC)
						ai.PilotSkills.manoeuvre = MANOEUVRE_JINK;
					else if	(rndnum < RND50PC)
						ai.PilotSkills.manoeuvre = MANOEUVRE_UNBALANCEDFLIGHT;
					else
						ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKTURN;
				}else
				{
					if	(rndnum < RND25PC)
						ai.PilotSkills.manoeuvre = MANOEUVRE_JINK;
					else if	(rndnum < RND50PC)
					{
						if (!AboveGround(FT_1000))
							ai.PilotSkills.manoeuvre = MANOEUVRE_JINK;
						else
							ai.PilotSkills.manoeuvre = MANOEUVRE_BREAK90;
					}
					else
					{
						if (!AboveGround(FT_3000))
							ai.PilotSkills.manoeuvre = MANOEUVRE_JINK;
						else
							ai.PilotSkills.manoeuvre = MANOEUVRE_BREAK180;
					}

				}
			}
	//rdh 6/1/99			if (rndnum < RND90PC)
//rdh 6/1/99			{
//rdh 6/1/99				if (combatfactor < 100)
//rdh 6/1/99					ai.PilotSkills.manoeuvre = MANOEUVRE_STRAIGHTANDLEVEL;
//rdh 6/1/99				else if (combatfactor < 150)
//rdh 6/1/99					ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;						
//rdh 6/1/99			}
		}
		return(TRUE);
	}
}

//------------------------------------------------------------------------------
//Procedure		M_UnSighted
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
Bool 	AirStruc::M_UnSighted()
{
//if unsighted:
//	good move is to slip out of plane
//	bad move is to reverse
//
	if  (	(roll <<  ANGLES_45Deg)
		||	(roll >>  ANGLES_315Deg)
		||	(	(PitchIntercept >> ANGLES_30Deg)
			&&	(PitchIntercept << ANGLES_330Deg)
			)
		)
	{
		return (FALSE);
	}else
	{
		SWord	brg = HdgIntercept - hdg;
		if	(	(		((roll >>  ANGLES_45Deg)	&&	(brg < 0))
					||	((roll >>  ANGLES_45Deg)	&&	(brg < 0))
				)
			&&
				(	(ai.morale < MORALE_MEDIUM)
				||	(ai.PilotSkills.combatskill < SKILL_REGULAR)
				)
			)
			{//ref1.66 poor manoeuvre when unsighted
				ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKTURN;
				return (TRUE);

			}else
			{
				return (FALSE);
			}
	}			

}
//------------------------------------------------------------------------------
//Procedure		M_DiveForHome
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
Bool 	AirStruc::M_DiveForHome()
{

	if	(	(		(AircraftDamaged())
			&&	(ai.PilotSkills.combatskill < SKILL_REGULAR)
			)
		||	(
					(ai.PilotSkills.combatskill < SKILL_REGULAR)
				&&	(ai.PilotSkills.combatskill > SKILL_POOR)
				&&	(ai.morale 	< 	MORALE_MEDIUM)
			)

		)
		{
			ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEFORHOME;
			return (TRUE);
		}else
			return(FALSE);
}
//------------------------------------------------------------------------------
//Procedure		M_SplitS
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
Bool 	AirStruc::M_SplitS()
{
	RndVal	rndnum = Math_Lib.rnd();
	if	(	(	(roll >> ANGLES_10Deg)
			|| 	(roll << (ANGLES_350Deg))
			)
			&&  (AboveGround(FT_10000))
			&&	(!AircraftDamaged())
		)
		{
			if	(rndnum < RND25PC)
				ai.PilotSkills.manoeuvre = MANOEUVRE_BREAK180;
			else if	(rndnum < RND50PC)
			{
				if(classtype->visible == F86)				//CSB 08/07/99	
					ai.PilotSkills.manoeuvre = MANOEUVRE_DIVINGSPIN;	//CSB 08/07/99	
				else										//CSB 08/07/99	
					ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKLOW;
			}
			else
				ai.PilotSkills.manoeuvre = MANOEUVRE_SPLITS;
			return (TRUE);
		}else if	(	(	(roll >> ANGLES_10Deg)
			|| 	(roll << (ANGLES_350Deg))
			)
			&&  (AboveGround(FT_5000))
		)
		{
			if	(rndnum < RND25PC)
				ai.PilotSkills.manoeuvre = MANOEUVRE_BREAK180;
			else if	(rndnum < RND50PC)
				ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKLOW;
			else
				ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKHIGH;
			return (TRUE);
		}else
			return(FALSE);

}


//------------------------------------------------------------------------------
//Procedure		M_Scissors
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
Bool 	AirStruc::M_Scissors(ANGLES lead)
{
	if  (	(	(lead << ANGLES_90Deg)
				||
				(lead >> ANGLES_358Deg)
			)
			&&	(	(roll >> ANGLES_30Deg)
				&& 	(roll << ANGLES_330Deg)
				)
		)
	{//ie the lead is too much or just enough for target to get in a shot
		if	(	(	(ai.PilotSkills.combatskill >= SKILL_VETERAN))
				&&	(lead << ANGLES_90Deg)
				)
		{
			ai.PilotSkills.manoeuvre = MANOEUVRE_REVERSETURN;
			return (TRUE);
		}else
		{
			RndVal	rndnum = Math_Lib.rnd();
			if	(rndnum < RND50PC)
			{
			 	if (ai.PilotSkills.combatskill >= SKILL_REGULAR)
					ai.PilotSkills.manoeuvre = MANOEUVRE_SCISSORS;
				else
					ai.PilotSkills.manoeuvre = MANOEUVRE_MILDSCISSORS;
				return (TRUE);
			}else
				return(FALSE);	
		}
	}else
		return(FALSE);	
}

//------------------------------------------------------------------------------
//Procedure		M_ShootTooEarly
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
Bool	AirStruc::M_ShootTooEarly()
{

	if	(		SubjectFacingTarget(ANGLES_10Deg)
			&&	(SubjectOnTargetTail(ANGLES_20Deg))
			&&	(ai.PilotSkills.combatskill < SKILL_REGULAR)
//rdh 6/1/99			&&	(ai.morale < MORALE_GOOD)
		)
		{
			ai.PilotSkills.manoeuvre = MANOEUVRE_SHOOTTOOEARLY;
	 		return (TRUE);
 	}else
 		return(FALSE);
}
//------------------------------------------------------------------------------
//Procedure		DefenceManoeuvre
//Author		R. Hyde 
//Date			Thu 12 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::DefenceManoeuvre()
{

	if  (	(TargetClosureGreaterThan(MPH50))				//ref1.64
		)
	{
		if 	(ai.morale > MORALE_POOR)
		{
			if (SubjectHasEnergyAdvantage())
				 ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT, MANOEUVRE_BREAKTURN);
			else
				 ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_BREAKTURN, MANOEUVRE_TURNINGFIGHT);

		}else
			ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;
	}
	else if (ai.morale > MORALE_POOR)
	{
		if (SubjectHasEnergyAdvantage())
			 ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_HIGBARRELROLL);
		else
			 ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_HIGBARRELROLL,MANOEUVRE_TURNINGFIGHT);	////lag roll defense ref1.71
	}
	else
		ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
		
}
//------------------------------------------------------------------------------
//Procedure		M_ShootToFrighten
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
Bool	AirStruc::M_ShootToFrighten()
{
	if	(		SubjectFacingTarget(ANGLES_10Deg)
			&&	(SubjectOnTargetTail(ANGLES_20Deg))
			&&	(ClosureLessThan(MPH00))
			&&	(ai.PilotSkills.combatskill > SKILL_VETERAN)
		)
	{
		ai.PilotSkills.manoeuvre = MANOEUVRE_SHOOTTOFRIGHTEN;
		return (TRUE);
 	}else
 		return(FALSE);

}
//------------------------------------------------------------------------------
//Procedure		DeltaAltitude
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
SLong		AirStruc::DeltaAltitude()
{
	return (World.Y - ai.unfriendly->World.Y);
}
//------------------------------------------------------------------------------
//Procedure		AggressivePassmanoeuvre
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
void	AirStruc::AggressivePassManoeuvre()
{
	RndVal	rndnum = Math_Lib.rnd();

	if (	(	(classtype->phrasename == PHRASE_MIGS)
			&&	(rndnum < RND25PC)
			)
			||
			(	(classtype->visible == F86)
			&&	(rndnum > RND25PC)
			)
		)
		ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKTURN;
	else
		ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
}
//------------------------------------------------------------------------------
//Procedure		DefensivePassManoeuvre
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
void	AirStruc::DefensivePassManoeuvre()
{
	RndVal	rndnum = Math_Lib.rnd();
	 if (rndnum < RND25PC)
		ai.PilotSkills.manoeuvre=MANOEUVRE_TURNINGFIGHT;
	 else if (rndnum < RND50PC)
		ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;
	else
		ai.PilotSkills.manoeuvre = MANOEUVRE_ZOOM;


}
//------------------------------------------------------------------------------
//Procedure		SelectNoseToNoseManoeuvre
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
void	AirStruc::SelectNoseToNoseManoeuvre()
{
	if (Range > ENGAGERANGE)
	{
		if (DeltaAltitude() > FT_2000)
			ai.PilotSkills.manoeuvre = MANOEUVRE_INTERCEPTHIGH;
		else if (DeltaAltitude() < FT_2000)
			ai.PilotSkills.manoeuvre = MANOEUVRE_GAINHEIGHT;
		else
			M_FlightSeparation();
	}else
	{
		if  (DeltaAltitude() > FT_2000)
			ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;
		else 
			M_FlightSeparation();
	}


//	if (Range > ENGAGERANGE)
//	{
//		if (DeltaAltitude() > FT_2000)
//			ai.PilotSkills.manoeuvre = MANOEUVRE_INTERCEPTHIGH;
//		else if (DeltaAltitude() < FT_2000)
//			ai.PilotSkills.manoeuvre = MANOEUVRE_GAINHEIGHT;
//		else
//			M_FlightSeparation();
//	}else if (Range > 3 * BREAKTURNRADIUS)
//	{
//		M_FlightSeparation();
//	}else
//	{
//		RndVal	rndnum = Math_Lib.rnd();
//		if (DeltaAltitude() > FT_2000)
//		{
//			if (ai.morale >= MORALE_VERYGOOD)
//				AggressivePassManoeuvre();
//			else
//				DefensivePassManoeuvre();
//		}else if  (Range < DANGERRANGE)
//		{
//			if (ai.morale >= MORALE_VERYGOOD)
//				AggressivePassManoeuvre();
//
//			else if	(TargetFacingSubject(ANGLES_5Deg))
//			{
//				if (ai.morale < MORALE_MEDIUM)
//					ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKTURN;		//panic
//				else if (ai.morale < MORALE_VERYGOOD)
//				{
//					if (ai.PilotSkills.combatskill > SKILL_REGULAR)
//						ai.PilotSkills.manoeuvre = MANOEUVRE_JINK;
//					else
//						ai.PilotSkills.manoeuvre = MANOEUVRE_STRAIGHTANDLEVEL;
//				}
//			}else
//				ai.PilotSkills.manoeuvre = MANOEUVRE_STRAIGHTANDLEVEL;
//				
//		}else
//		{
//			if (ai.morale >= MORALE_VERYGOOD)
//			{
//				if ((	(ai.PilotSkills.combatskill < SKILL_REGULAR)
//					)
//					||
//					(	(ai.PilotSkills.combatskill > SKILL_VETERAN)
//					 		&&	(Range < BREAKTURNRADIUS)
//					)
//				   )
//						ai.PilotSkills.manoeuvre = MANOEUVRE_BREAKTURN;
//					else
//						M_FlightSeparation();
//			}else
//				M_FlightSeparation();
//				
//		}
//	}

//	if alt advantage
//		dive and zoom	
//	
//	ai.PilotSkills.manoeuvre = MANOEUVRE_HEADONOFFSET;
//	
//	
//	
//	
//##	lead turn 77
//	nose to nose 78
//	nose to tail 79
//	in horizontal and vertical planes
//if (morale high and aggressionlevel == attack)
//	try Headonattack
//if (under headonattack and morale == low)
//	panic and scatter
//
}

//------------------------------------------------------------------------------
//Procedure		SelectNoseToTailManoeuvre
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
void	AirStruc::SelectNoseToTailManoeuvre()
{ 
	if (Range > DANGERRANGE)
	{
		if(		(ai.PilotSkills.combatskill >= SKILL_VETERAN)
					&&  (AboveGround(FT_1000))
					&&	( Range > WEAPONSRANGE)
					&&	(ClosureLessThan(MPH15))
				)
			ai.PilotSkills.manoeuvre = MANOEUVRE_LOWYOYO;
		else
			ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
//	}else if (Range > DANGERRANGE)
//		ManoeuvreBasedOnClosure();
	}
	else
	{
		if (follower)
		{
			{
				if	(Math_Lib.rnd(RndValMAX) < RND50PC)
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_COVERME, MSG_STATUSREPORT, this, NULL, Follower()));
				else
					_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_STAYCLOSE, MSG_STATUSREPORT , this, NULL,Follower()));
			}
		}	
		AirStruc* unf			= (AirStruc*)ai.unfriendly;
		AirStruc* unfbuddy	= unf->FindBuddy();

		
		if  (unfbuddy)
		{//unfriendly is attacked by aircraft 
		   BreakCallandReaction(unfbuddy, unf, this);
		}
		if (Range > WEAPONSRANGE)
		{
			if (ai.PilotSkills.combatskill > SKILL_REGULAR)
			{
				if (!M_ShootToFrighten())
					ManoeuvreBasedOnClosure();
			}else
			{
				if (!M_ShootTooEarly())
					ManoeuvreBasedOnClosure();
			}
		}
		else
			ManoeuvreBasedOnClosure();
	}

}
//------------------------------------------------------------------------------
//Procedure		SelectNoseToBeamManoeuvre
//Author		R. Hyde 
//Date			Wed 4 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SelectNoseToBeamManoeuvre()
{
	if (Range > ENGAGERANGE)
		ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
	else if (Range > DANGERRANGE)
		ManoeuvreBasedOnClosure();
	else
	{
		if (Range > WEAPONSRANGE)
		{
			if (ai.PilotSkills.combatskill > SKILL_REGULAR)
			{
				if (!M_ShootToFrighten())
					ManoeuvreBasedOnClosure();
			}else
			{
				if (!M_ShootTooEarly())
					ManoeuvreBasedOnClosure();
			}
		}
		else
			ManoeuvreBasedOnClosure();
	}
}
//------------------------------------------------------------------------------
//Procedure		SelectBeamToNoseManoeuvre
//Author		R. Hyde 
//Date			Wed 4 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SelectBeamToNoseManoeuvre()
{

	if (Range > COMBATRANGE)
	{
		SelectNextDisengageManoeuvre ();
	}else if (Range > ENGAGERANGE)
	{
		if	(ai.morale 	> 	MORALE_MEDIUM)
		{
			if (SubjectHasEnergyAdvantage())
			{
				if (classtype->phrasename == PHRASE_MIGS)
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_GAINHEIGHT,MANOEUVRE_BREAKTURN);
				else
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
			}else
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
		}
	}else if (Range > DANGERRANGE)									  //rdh 31/03/99
	{
		if (!M_UnSighted())
			ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
	}else 
	{
		ANGLES trgtlead = ActualLead(*ai.unfriendly, *this);
		if (!M_GunDefence(trgtlead))
			if	(!M_Scissors(trgtlead))
				DefenceManoeuvre();
	}

}
//------------------------------------------------------------------------------
//Procedure		SelectBeamToTailManoeuvre
//Author		R. Hyde 
//Date			Wed 4 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SelectBeamToTailManoeuvre()
{
	if	(ai.morale 	> 	MORALE_GOOD)
		ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
	else if (Range > COMBATRANGE)
	{
		if	(ai.morale 	< 	MORALE_MEDIUM)
			SelectNextDisengageManoeuvre ();
		else
		{
			if (SubjectHasEnergyAdvantage())
			{
				if (classtype->phrasename == PHRASE_MIGS)
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_GAINHEIGHT,MANOEUVRE_BREAKTURN);
				else
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
			}else
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
		}
	}else if (Range > ENGAGERANGE)
	{
		if	(ai.morale 	> 	MORALE_MEDIUM)
		{
			if (SubjectHasEnergyAdvantage())
			{
				if (classtype->phrasename == PHRASE_MIGS)
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_GAINHEIGHT,MANOEUVRE_BREAKTURN);
				else
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
			}else
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
		}else
			ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
	}else
	{
		if (ai.morale >= MORALE_MEDIUM)
		//##	if (ai.PilotSkills.combatskill > SKILL_REGULAR)
				//##speedbrakes
			ai.PilotSkills.manoeuvre=MANOEUVRE_TURNINGFIGHT;
		else
			ai.PilotSkills.manoeuvre = MANOEUVRE_EXTENSION;

	}

}
//------------------------------------------------------------------------------
//Procedure		SelectBeamToBeamManoeuvre
//Author		R. Hyde 
//Date			Wed 4 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SelectBeamToBeamManoeuvre()
{
	if	(ai.morale 	> 	MORALE_GOOD)
		ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
	else if (Range > COMBATRANGE)
	{
		if	(ai.morale 	< 	MORALE_MEDIUM)
			SelectNextDisengageManoeuvre ();
		else
		{
			if (SubjectHasEnergyAdvantage())
			{
				if (classtype->phrasename == PHRASE_MIGS)
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_GAINHEIGHT,MANOEUVRE_BREAKTURN);
				else
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
			}else
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
		}
	}else if (Range > 2 * BREAKTURNRADIUS)
	{
		if	(ai.morale 	> 	MORALE_MEDIUM)
		{
			if (SubjectHasEnergyAdvantage())
			{
				MANOEUVRE tempman;
				if (classtype->phrasename == PHRASE_MIGS)
				{
					if (DeltaAltitude() > FT_2000)
						tempman = MANOEUVRE_TURNINGFIGHT;
					else
						tempman = MANOEUVRE_GAINHEIGHT; 
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_GAINHEIGHT,MANOEUVRE_BREAKTURN);
				}else
				{
					if (DeltaAltitude() > FT_2000)
						tempman = MANOEUVRE_BREAKTURN;
					else
						tempman = MANOEUVRE_ZOOM; 
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
				}
			}else
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
		}else
			SelectNextDisengageManoeuvre ();

	}else
	{
		if (NoseToTail(ANGLES_45Deg))
		{
			if (ai.morale >= MORALE_VERYGOOD)
				AggressivePassManoeuvre();
			else
				DefensivePassManoeuvre();
		}else
		{
			//if (ai.PilotSkills.combatskill > SKILL_REGULAR)
				//##speedbrakes
			ai.PilotSkills.manoeuvre=MANOEUVRE_TURNINGFIGHT;
		}
	}

}

//------------------------------------------------------------------------------
//Procedure		SelectTailToTailManoeuvre
//Author		R. Hyde 
//Date			Wed 4 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SelectTailToTailManoeuvre()
{

	if (Range > COMBATRANGE)
	{
		if	(ai.morale 	< 	MORALE_MEDIUM)
			SelectNextDisengageManoeuvre ();
		else
		{
			SLong d_alt = World.Y - ai.unfriendly->World.Y;
			if (d_alt > FT_3000)
				ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_INTERCEPTHIGH,MANOEUVRE_SPLITS);
			else if (d_alt < -FT_3000)
				ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_GAINHEIGHT,MANOEUVRE_IMMELMANN);
			else if (SubjectHasEnergyAdvantage())
			{
				if (classtype->phrasename == PHRASE_MIGS)
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_GAINHEIGHT,MANOEUVRE_ZOOM);
				else
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_BREAKTURN);
			}else
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
		}
	}else 
	{
		if	(ai.morale 	> 	MORALE_MEDIUM)
		{
			if (classtype->phrasename == PHRASE_MIGS)
				ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_GAINHEIGHT);
			else
				ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_BREAKTURN);
		}else
//			ai.PilotSkills.manoeuvre = MANOEUVRE_LAZYTURN;
			ai.PilotSkills.manoeuvre = MANOEUVRE_STRAIGHTANDLEVEL;
	}


}
//------------------------------------------------------------------------------
//Procedure		SelectTailToBeamManoeuvre
//Author		R. Hyde 
//Date			Wed 4 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SelectTailToBeamManoeuvre()
{
	if	(ai.morale 	> 	MORALE_GOOD)
		ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
	else if (Range > COMBATRANGE)
	{
		if	(ai.morale 	< 	MORALE_MEDIUM)
			SelectNextDisengageManoeuvre ();
		else
		{
			if (SubjectHasEnergyAdvantage())
			{
					if (classtype->phrasename == PHRASE_MIGS)
						ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_GAINHEIGHT,MANOEUVRE_BREAKTURN);
					else
						ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
			}else
				 ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
		}
	}else if (Range > ENGAGERANGE)
	{
		if	(ai.morale 	> 	MORALE_MEDIUM)
		{
			if (SubjectHasEnergyAdvantage())

			{
				if (classtype->phrasename == PHRASE_MIGS)
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_GAINHEIGHT,MANOEUVRE_BREAKTURN);
				else
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
			}else
				ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
		} else
			 ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
	}else
	{
		if (ai.PilotSkills.combatskill < SKILL_REGULAR)
		{
			ai.PilotSkills.manoeuvre = MANOEUVRE_REVERSETURN;
		}else
		{
			if (SubjectHasEnergyAdvantage())
				 ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_ZOOM,MANOEUVRE_TURNINGFIGHT);
			else
				 ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR,MANOEUVRE_TURNINGFIGHT,MANOEUVRE_ZOOM);
		}
	}
}


//------------------------------------------------------------------------------
//Procedure		SelectTailToNoseManoeuvre
//Author		R. Hyde 
//Date			Mon 18 Mar 1996
//
//Description	ChooseRearDefenseManoeuvre
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SelectTailToNoseManoeuvre()
{
	if (!M_SelfAsBait())
	{
		if (!M_DiveForHome())
		{
			if (Range > (METRES600 + COMBATRANGE))
			{
				ai.PilotSkills.manoeuvre= MANOEUVRE_TURNINGFIGHT;
			}else if (Range > ENGAGERANGE)
			{
				if (vel > MINIMUMCOMBATSPEED)
					ai.PilotSkills.manoeuvre = MANOEUVRE_TURNINGFIGHT;
				else			
					ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_GAINHEIGHT, MANOEUVRE_GAINSPEED);

			}else if (Range > DANGERRANGE)
			{
				if  (!M_UnSighted())
				{
					if (vel < MINIMUMCOMBATSPEED)
						ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_GAINSPEED, MANOEUVRE_TURNINGFIGHT);
					else			
						ai.PilotSkills.manoeuvre=ManoeuvreBasedOnSkill(SKILL_REGULAR, MANOEUVRE_GAINHEIGHT, MANOEUVRE_TURNINGFIGHT);
				}

			}else
			{
				ANGLES trgtlead = ActualLead(*ai.unfriendly, *this);
				if (!M_GunDefence(trgtlead))
					if (!M_UnSighted())
						if	(!M_Scissors(trgtlead))
							if (!M_ZoomAndDrop())
								DefenceManoeuvre();
			}
		}	
	}
}

//------------------------------------------------------------------------------
//Procedure		CornerSpeed
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
SWord AirStruc::CornerSpeed()
{
	SWord retval = CORNERSPEED;
	return(retval);
}

//------------------------------------------------------------------------------
//Procedure		SustainedTurn
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
SWord AirStruc::SustainedTurnSpeed()
{

		SWord retval = SUSTAINEDTURNSPEED;
		return(retval);
}
//------------------------------------------------------------------------------
//Procedure		SetDesiredVel()
//Author		R. Hyde 
//Date			9/12/98
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SetDesiredVel()
{
	//the range will be from 0 to OUTERCOMBATRANGE
#define IDEALCOMBATRANGE2 30000
	Float rangefactor;
	Float velfactor = 0.05;
	AirStrucPtr	unfriendly = *ai. unfriendly;

	Float v;
	rangefactor = Range - IDEALCOMBATRANGE2;
	if (rangefactor < -METRES100)
		rangefactor = -METRES100;
	rangefactor = rangefactor * velfactor / IDEALCOMBATRANGE2;
	v = unfriendly->vel * (1 - rangefactor);
	fly.pModel->SetSpeed(this,v);

	if (Range  < COLLISIONRANGE)
		_Miles.SequenceAudible(MOOD_HERO);	  //RDH 01/03/99
	else if (Range  < DANGERRANGE)
		_Miles.SequenceAudible(MOOD_CONFIDENT);	  //RDH 01/03/99
	else
		_Miles.SequenceAudible(MOOD_HOPEFUL);	  //RDH 01/03/99

}
//------------------------------------------------------------------------------
//Procedure		ManualACM
//Author		R. Hyde 
//Date			Fri 11 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::ManualACM ()
{

		switch (ai.PilotSkills.manoeuvre)
		{
			case MANOEUVRE_LOOKROUND:
			{						  
 				LookAround ();		  
				break;				  
			}			
			case MANOEUVRE_WELDEDWINGMAN:										//RDH 03Mar98
			{
				WeldedWingMan();
				break;
			}
			case MANOEUVRE_BAILOUT:										//RDH 03Mar98
			{
 				BailOut();
				break;
			}
			case MANOEUVRE_TOPCOVER:
			{						  
				TopCover ();		  
				break;				  
			}			
			case MANOEUVRE_LINEABREAST:										//RDH 27Mar98
			{
 				LineAbreast();
				break;
			}
			case MANOEUVRE_SCATTER:											//RDH 27Mar98
			{
				TurningFight();
				break;
			}
			case MANOEUVRE_PINCER:											//RDH 27Mar98
			{
 				Pincer();
				break;
			}
			case MANOEUVRE_MULTIWAVE:										//RDH 27Mar98
			{
 				MultiWave();
				break;
			}
			case MANOEUVRE_DIVEANDZOOM:
			{
 				DiveAndZoom();
				break;
			}
			case MANOEUVRE_LEADPURSUIT:
			{
//done in TurningFight 				LeadPursuit();
				TurningFight();
				break;
			}
			case MANOEUVRE_LAGPURSUIT:
			{
 				LagPursuit();
				break;
			}
			case MANOEUVRE_CIRCUMVENT:										//RDH 27Mar98
			{
//will not be implemented				Circumvent();
				TurningFight();
				break;
			}
			case MANOEUVRE_ROUNDABOUT:										//RDH 27Mar98
			{
 				RoundAbout();
				break;
			}
			case MANOEUVRE_SPLITMANOEUVRE:									//RDH 27Mar98
			{
 				SplitManoeuvre();									  //RDH 21/02/99
				break;
			}
			case MANOEUVRE_HEADON:
			{
  				HeadOn();
				break;
			}
			case MANOEUVRE_LINEASTERN:										//RDH 27Mar98
			{
				LineAstern();
				break;
			}
			case MANOEUVRE_BARRELROLLATTACK:							
			{													
				BarrelRollAttack ();									
				break;											
			}													
			case MANOEUVRE_SCISSORS:							
			{													
 				Scissors ();									
				break;											
			}													
			case MANOEUVRE_MILDSCISSORS:						
			{													
 				MildScissors ();								
				break;											
			}													
			case MANOEUVRE_SPLITS:								
			{													
 				SplitS ();										
				break;											
			}													
			case MANOEUVRE_ZOOMANDDROP:							
			{													
 				ZoomandDrop ();									
				break;											
			}													
			case MANOEUVRE_STRAIGHTANDLEVEL :					
			{													
 				StraightandLevel ( );							
				break;											
			}													
			case MANOEUVRE_SPINOUT :							
			{													
 				SpinOut ( );									
				break;											
			}													
			case MANOEUVRE_DIVEFORHOME :						
			{													
				DiveforHome ( );								
				break;											
			}													
			case MANOEUVRE_GOHOME :								
			{													
 				GoHome ( );										
				break;											
			}													
			case MANOEUVRE_MAKEFORFRIENDLY :							
			{													
				TurningFight();
				break;											
			}													
			case MANOEUVRE_MOVEAWAY :							
			{													
 				MoveAway ( );									
				break;											
			}													
			case MANOEUVRE_ACCIDENTALSPINOUT :					
			{//no difference from standard spin													
				SpinOut ( );							
				break;											
			}													
			case MANOEUVRE_VERTICALLOOP :						
			{	
//will not be used				VerticalLoop ( );								
				TurningFight();
				break;											
			}													
			case MANOEUVRE_IMMELMANNTURN :						
			{													
//will not be used 				ImmelmannTurn ( );								
				TurningFight();
				break;											
			}													
			case MANOEUVRE_HEADONOFFSET :						
			{													
  				HeadOnOffset ( );								
				break;											
			}													
			case MANOEUVRE_IMMELMANN :						
			{													
 				Immelmann ( );								
				break;											
			}													
			case MANOEUVRE_STAYWITHPREY :						
			{													
//will not implement 				StayWithPrey ( );								
				TurningFight();
				break;											
			}													
			case MANOEUVRE_CLIMBFORHOME :						
			{													
 				ClimbforHome ( );								
				break;											
			}													
			case MANOEUVRE_STRAIGHTDIVE:						
			{													
				StraightDive ( );								
				break;											
			}													
			case MANOEUVRE_STANDONTAIL:							
			{													
//will not be used
				TurningFight();
				break;											
			}													
			case MANOEUVRE_SHOOTTOFRIGHTEN:						
			{													
 				ShootToFrighten ( );							
				break;											
			}													
			case MANOEUVRE_SHOOTTOOEARLY:						
			{													
 				ShootTooEarly ( );							
				break;											
			}													
			case MANOEUVRE_GAINHEIGHT:							
			{													
				GainHeight ( );									
				break;											
			}													
			case MANOEUVRE_LAGROLL:											//RDH 30Mar98
			{
				LagRoll();
				break;
			}
			case MANOEUVRE_EXTENSION:								//RDH 30Mar98
			{
 				ExtensionManoeuvre();
				break;
			}
			case MANOEUVRE_DIVINGSPIN:
			{
 				DivingSpin();
				break;
			}
			case MANOEUVRE_REVERSETURN:										//RDH 30Mar98
			{
 				ReverseTurn();
				break;
			}
			case MANOEUVRE_SELFASBAIT:										//RDH 30Mar98
			{
 				SelfAsBait();
				break;
			}
			case MANOEUVRE_JINK:												//RDH 30Mar98
			{
 				Jink();
				break;
			}
			case MANOEUVRE_BREAKTURN:										//RDH 30Mar98
			{
				BreakTurn();
				break;
			}
			case MANOEUVRE_LAZYTURN:											//RDH 30Mar98
			{
				LazyTurn();
				break;
			}
			case MANOEUVRE_BREAK180:											//RDH 30Mar98
			{
 				Break180();
				break;
			}
			case MANOEUVRE_BREAKLOW:											//RDH 30Mar98
			{
 				BreakLow();
				break;
			}
			case MANOEUVRE_BREAKHIGH:										//RDH 30Mar98
			{
 				BreakHigh();
				break;
			}
			case MANOEUVRE_BREAK90:											//RDH 30Mar98
			{
 				Break90();
				break;
			}
			case MANOEUVRE_HIGBARRELROLL:									//RDH 30Mar98
			{
 				HiGBarrelRoll();
				break;
			}
			case MANOEUVRE_PANICTURN:									//RDH 30Mar98
			{
				PanicTurn();
				break;
			}
			case MANOEUVRE_UNBALANCEDFLIGHT:									//RDH 30Mar98
			{
 				UnBalancedFlight();
				break;
			}
			case MANOEUVRE_LOWALT:
			{
 				LowAlt();
				break;
			}
			case MANOEUVRE_SNAPSHOT:											//RDH 30Mar98
			{
//will not be used	SnapShot();
				TurningFight();
 				
				break;
			}
			case MANOEUVRE_STAYONTAIL:										//RDH 30Mar98
			{
//will not be used 				StayOnTail();
				TurningFight();
				break;
			}
			case MANOEUVRE_TURNTOHDGANDPITCH:								//RDH 30Mar98
			{
				TurningFight();
				break;
			}
			case MANOEUVRE_HOTSIDELAGPURSUITFIGHT:							//RDH 30Mar98
			{
//will not be used  				HotSideLagPursuitFight();
				TurningFight();
				break;
			}
			case MANOEUVRE_SANDWICHMANOEUVRES:								//RDH 30Mar98
			{
				TurningFight();
				break;
			}
			case MANOEUVRE_TURNINGFIGHT:									//RDH 30Mar98
			{
				TurningFight();
				break;
			}
			case MANOEUVRE_SUSTAINEDTURN:									//RDH 30Mar98
			{
//will not use, covered in turningfight = f(skill)				SustainedTurn();
 				TurningFight();
				break;
			}
			case MANOEUVRE_HIGHYOYO:											//RDH 30Mar98
			{
 				HighYoYo();
				break;
			}
			case MANOEUVRE_ZOOM:											//RDH 30Mar98
			{
 				Zoom();
				break;
			}
			case MANOEUVRE_LOWYOYO:											//RDH 30Mar98
			{
 				LowYoYo();
				break;
			}
			case MANOEUVRE_INTERCEPTHIGH:									//RDH 30Mar98
			{
				InterceptHigh();
				break;
			}
			case MANOEUVRE_CLIMBATSUSTAINEDTURNSPEED:						//RDH 30Mar98
			{
				ClimbAtSustainedTurnSpeed();
				break;
			}
			case MANOEUVRE_GAINSPEED:						
			{													
				if (ai.unfriendly)
					TurningFight();
				else
					GainSpeed();
				break;											
			}													
			case MANOEUVRE_SPINRECOVERY:						
			{													
 				SpinRecovery ( );								
				break;											
			}
		}

		weap.ShootDelay = weap.ShootDelay-Timer_Code.FRAMETIME;
		if (weap.ShootDelay < 0)
			weap.ShootDelay = 0;
		
		if (ai.unfriendly!=NULL)
			AutoShoot(TRUE);

		
		ai.oldoldoldhdgI = ai.oldoldhdgI;
		ai.oldoldhdgI = ai.oldhdgI;
		ai.oldhdgI = HdgIntercept;

		ai.oldoldoldpitchI = ai.oldoldpitchI;
		ai.oldoldpitchI = ai.oldpitchI;
		ai.oldpitchI = PitchIntercept;
//Old_Code DAW 18May99 	if (	(Save_Data.flightdifficulty [FD_AUTOTHROTTLE])
//Old_Code DAW 18May99 		&&	(	ai.unfriendly == Persons2::PlayerGhostAC 
//Old_Code DAW 18May99 				|| (		(ai.unfriendly)
//Old_Code DAW 18May99 						&&	(ai.unfriendly->uniqueID.UniqueID.commsmove)				//AMM 07May99
//Old_Code DAW 18May99 					)
//Old_Code DAW 18May99 			)
//Old_Code DAW 18May99 		&&	(Range  < OUTERCOMBATRANGE)
//Old_Code DAW 18May99 		&&	(TargetFacingSubject(ANGLES_45Deg))
//Old_Code DAW 18May99 		&&	(TargetOnSubjectTail(ANGLES_45Deg))
//Old_Code DAW 18May99 		)
	if (	(ai.unfriendly )
		&&	(Save_Data.flightdifficulty [FD_AUTOTHROTTLE])
		&&	((ai.unfriendly == Persons2::PlayerGhostAC)
			|| (ai.unfriendly->uniqueID.bitfieldUid.commsmove))
		&&	(Range  < OUTERCOMBATRANGE)
		&&	(TargetFacingSubject(ANGLES_45Deg))
		&&	(TargetOnSubjectTail(ANGLES_45Deg))
		)
		SetDesiredVel();
	
	Manual_Pilot.ACMManualPilot(this);	

	if (ai.PilotSkills.manoeuvre == MANOEUVRE_SELECT)
	{
		if (ai.unfriendly==NULL)
		{
			if (	(ai.PilotSkills.manoeuvre != MANOEUVRE_TOPCOVER)			//JIM 21Aug96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_LOOKROUND)			//JIM 21Aug96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_DIVEFORHOME)			//JIM 15Oct96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_CLIMBFORHOME)		//JIM 15Oct96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_STRAIGHTDIVE)		//JIM 15Oct96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_GOHOME )				//JIM 15Oct96
				&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_SPINRECOVERY )				//JIM 15Oct96
				)
			{
		  		ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre=MANOEUVRE_LOOKROUND;
			}

		}else
		{
			InterceptandRange (&ai.unfriendly->World);
			SelectNextEngageManoeuvre ();
	  		ai.PilotSkills.ManStep = PHASE0;

		}
	}
	if (ai.PilotSkills.manoeuvre == MANOEUVRE_SELECT)
	{
		AutoMoveInfo.movecode=AUTO_FOLLOWWP;
	}
}

//------------------------------------------------------------------------------
//Procedure		BreakCallandReaction
//Author		R. Hyde 
//Date			 //RDH 13/06/99
//
//Description	
//					HdgIntercept assumed
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::BreakCallandReaction(AirStrucPtr buddy,AirStrucPtr currac,AirStrucPtr unf)
{
   if(currac->classtype->aerobaticfactor != AEROBATIC_LOW)
   {
 		SWord dhdg = unf->hdg - HdgIntercept;
		bool onright = false;
		if (dhdg > 0)
			onright = true;
		_Radio.TriggerMsg(MESSAGE_STRUC(Art_Int.BreakCall(buddy, onright),
					MSG_TOLDTOBREAK, buddy, unf, currac));
		BreakCallReaction(currac, unf);
   }
}
			
//------------------------------------------------------------------------------
//Procedure		BreakCallReaction
//Author		R. Hyde 
//Date			 //RDH 13/06/99
//
//Description	
//					HdgIntercept assumed
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::BreakCallReaction(AirStrucPtr currac,AirStrucPtr unf)
{
		MANOEUVRE manoeuvre = MANOEUVRE_SELECT;
		MANOEUVRE goodmanoeuvre = MANOEUVRE_SELECT;
		if 	(currac->ai.PilotSkills.combatskill >= SKILL_VETERAN)
			 goodmanoeuvre = MANOEUVRE_SPLITS;

		if 	(currac->ai.PilotSkills.combatskill >= SKILL_REGULAR)
		{
			if((currac->classtype->visible == F86) && (!Math_Lib.rnd(4)))	//CSB 08/07/99	
				manoeuvre = MANOEUVRE_DIVINGSPIN;							//CSB 08/07/99	
			else															//CSB 08/07/99	
			{
				RndVal	rndnum = Math_Lib.rnd();
				if	(rndnum < RND33PC)	//CSB 29/06/99	
					manoeuvre = MANOEUVRE_SCISSORS;
				else if	(rndnum < RND67PC)	//CSB 29/06/99	
					manoeuvre = MANOEUVRE_BREAKTURN;
				else
					manoeuvre = goodmanoeuvre;						  //RDH 15/06/99 //RDH 20/06/99
			}
		}
		Art_Int.SetEngage(currac,unf,manoeuvre,ANGLES_0Deg,ANGLES_0Deg, FALSE);

}
			

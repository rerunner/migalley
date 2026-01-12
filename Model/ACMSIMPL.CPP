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

#include "_MODE.H" //RERUN

#define F_GRAFIX											//DAW 05Aug96
#define F_COMMON
#define F_BATTLE



#include "COLLIDED.H"
#include "DOSDEFS.H"
#include "WORLDINC.H"


#include "MODVEC.H"
#include "MODEL.H"
#include "ACMMODEL.H"



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




SLong	const	MAXAUTOTHRUST = 100;	
//static 
RequiredBankDataElement 	CombatReqBankData[] =
			{
				{ANGLES_FifthDeg, ANGLES_10Deg},
				{ANGLES_2Deg, ANGLES_20Deg},
				{ANGLES_10Deg, ANGLES_45Deg},
				{ANGLES_45Deg, MAXCOMBATROLL},
				{ANGLES_180BareDeg, MAXCOMBATROLL}
			};

Float	AirStruc::CalcSimpleAccel ()
{
	if(classtype->aerobaticfactor == AEROBATIC_LOW)
	{
		INT3;
		return(0);
	}
	
	MODEL& MOD = *fly.pModel;
	ACMMODEL& ACM = *fly.pAcmModel;
	//RERUN extern enum ACM_COMMAND;
	Float Thrust, Drag, Lift;
	Float mass;		//kg		
	Float accel;
	Float fvel = vel * 0.1;	//m/s
	SWord sinroll, cosroll;
	Float N;

//***************************************************************************************
	//CSB 24/09/98
	mass = classtype->weightempty * 0.01 + (weap.left.int_fuel + weap.centre.int_fuel + weap.right.int_fuel) * 0.00001;	// -> kg
	PMAINPLANE pMainPlane = MOD.MainPlaneList;

	FP qS = 0.5 * MOD.AmbDensity * /* 10000 * */ fvel * fvel * pMainPlane->Area; //* 0.0001;

	Math_Lib.high_sin_cos(roll, sinroll, cosroll);
	if(cosroll == 0) 
		Lift = mass * 9 * 9.8065;
	else
		Lift = mass * 32767 / cosroll * 9.8065;
	if(Lift < 0) Lift *= -1;

	if(Lift > 1.25 * qS) Lift = 1.25 * qS;

	Drag = pMainPlane->Cdo * qS + pMainPlane->k * Lift * Lift / qS;

	PENGINE pEngine = MOD.EngineList;

	UByte	FF = FlyingFactor();
	AircraftAnimData*	adptr = (AircraftAnimData *)Anim;

	Float ThrustPercent =  fly.thrustpercent;
	ThrustPercent       *= (255.0 - Float(adptr->ENGINELEFTIN)) / 255.0;

	if(pEngine->Type == ET_JET)
		Thrust = pEngine->T100 * ThrustPercent * MOD.AmbDensity / 0.0001225 * 0.01;
	else
		Thrust = pEngine->T100 * ThrustPercent * MOD.AmbDensity / 0.0001225 * 0.1 / vel;

	Drag *= (2048 - adptr->LEFTWINGOUT - adptr->LEFTWINGIN - adptr->RIGHTWINGIN - adptr->RIGHTWINGOUT);
	Drag /= 2048;

	Drag *= (512.0 - FP(FF)) / 256.0;
	
	accel = (Thrust - Drag)/mass;
	if(fvel != 0)
		accel -= 9.8065 * 0.1 * vely/fvel ; 

	return(accel);


//***************************************************************************************
}


//------------------------------------------------------------------------------
//Procedure		CalcMaxPitchRate
//Author		Craig Beeston 
//Date			Tue 1 June 1999
//
//Description	Calculates the maximum pitch rate for a simple AutoMoveInfo.movecode
//				Limited by G and stall limit
//
//Inputs		
//
//Returns		Max Pitch rate in Rowans per second
//
//------------------------------------------------------------------------------
UWord AirStruc::CalcMaxPitchRate()
{
	if(classtype->aerobaticfactor == AEROBATIC_LOW)
		return(classtype->maxpitchrate);

	SWord FF = FlyingFactor();
	SWord maxG = 1 + (10 * FF) / 256;
	ULong pitchrateMG = ANGLES_90Deg;
	if(vel > 150) pitchrateMG = (1043100 * maxG) / vel;	//rowans/sec for 9 g
	FP maxpitchrate = (ANGLES_50Deg * FlyingFactor()) / 256.0;
	maxpitchrate *= vel * fly.pModel->AmbDensity / (0.0001225 * classtype->maxvel);
		//Pretty good figure for all MiG Alley Aircraft		//CSB 01/06/99	

	if(maxpitchrate > pitchrateMG) 
		maxpitchrate = pitchrateMG;
	
/* TEST CODE CSB 01/06/99 */	
/* TEST CODE CSB 01/06/99 */		
/* TEST CODE CSB 01/06/99 */	
/*	MODEL& MOD = *fly.pModel;
	int flightno = formpos & 0x03;
	int flightpo = (formpos & 0x30) / 16;
	int grouppos = 1 + flightno * 4 + flightpo;
	int line = grouppos + 1;
	if((MOD.Type == AT_MIG15) || (MOD.Type == AT_MIG15BIS))
		line += 9;

	PrintVar(60, line, "F %.0f ", FP(FF));

	FP MPR = maxpitchrate / 182.04;
	if(maxpitchrate == pitchrateMG)
	{
		PrintVar(65, line, "%.0f x G ", FP(maxG));
		PrintVar(70, line, " %.2f ", FP(MPR));
	}
	else
		PrintVar(65, line, "STALL %.2f ", FP(MPR));

	FP APR = Model_InstTurnRate(this) * 5730.0;
	if(APR < 0) APR *= -1;
	if(APR > MPR)
		PrintVar(76, line, "%.1f", FP(APR));
*/
/* TEST CODE CSB 01/06/99 */
/* TEST CODE CSB 01/06/99 */
/* TEST CODE CSB 01/06/99 */
	return((UWord)maxpitchrate);
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
void	AirStruc::SimpleTurningFight()
{
	COORDS3D	target;
	MODEL& MOD = *fly.pModel;
	ACMMODEL& ACM = *fly.pAcmModel;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	SLong vx = 0, vy = 0, vz = 0;
	SWord	reqdeltahdg = 0, reqdeltahdg_sign = 0;
	UWord	reqdeltahdg_pos = 0;
	SWord	reqdeltapitch = 0, reqdeltapitch_sign = 0;
	UWord	reqdeltapitch_pos = 0;
	
	if (!ai.unfriendly)		
		return;			

	target.X = ai.unfriendly->World.X;
	target.Y = ai.unfriendly->World.Y;
	target.Z = ai.unfriendly->World.Z;
	InterceptandRange (&target);

	reqdeltahdg   =	HdgIntercept - hdg;
	Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);
	SimpleMoveToThrust (ThrustfromFF());							  //RDH 06/05/99
	if((reqdeltahdg_pos < ANGLES_45Deg) && (Range < WEAPONSRANGE))
	{
		if (	(classtype->phrasename == PHRASE_MIGS)
			&& ((ai.PilotSkills.combatskill >= SKILL_ACE) || (ai.morale < MORALE_GOOD)) )
			if(reqdeltahdg_pos < ANGLES_90Deg)
			{
				if	(		(ai.PilotSkills.manoeuvre != MANOEUVRE_DIVEANDZOOM)			  //RDH 04/07/99
						&&	(unfriendly->classtype->aerobaticfactor != AEROBATIC_LOW)						
					)
				{
					if(Range < WEAPONSRANGE)
						SimpleMoveToThrust(0);
					else
						SimpleMoveToThrust (ThrustfromFF());
					if(Range < 0.5 * WEAPONSRANGE)
						ACM.SpeedBrakesOut();
					else
						ACM.SpeedBrakesIn();
				}
			}

		CalcLead(*this, *ai.unfriendly,vx,vy,vz, FALSE);
		target.X = ai.unfriendly->World.X + vx;
		target.Y = ai.unfriendly->World.Y + vy;
		target.Z = ai.unfriendly->World.Z + vz;
		InterceptandRange (&target);
		reqdeltahdg   =	HdgIntercept - hdg;
		reqdeltapitch =	PitchIntercept - pitch;
		Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);
		Math_Lib.Pos (reqdeltapitch,reqdeltapitch_pos,reqdeltapitch_sign);
		
		if(reqdeltapitch != 0)
		{
			SWord DesRoll = Math_Lib.arctan(reqdeltahdg, reqdeltapitch);
			SimpleMoveToRoll(DesRoll);
		}

		SWord sin_roll, cos_roll;
		Math_Lib.high_sin_cos(roll, sin_roll, cos_roll);
		FP MaxDeltaPitch = (FP)CalcMaxPitchRate() * (FP)Timer_Code.FRAMETIME / (100.0 * 32768.0);
		FP MaxDeltaYaw   = MaxDeltaPitch * sin_roll;
		MaxDeltaPitch	 = MaxDeltaPitch * cos_roll;

		SWord	maxdeltapitch_sign, maxdeltayaw_sign;
		UWord	maxdeltapitch_pos,  maxdeltayaw_pos;
		Math_Lib.Pos((SWord)MaxDeltaPitch, maxdeltapitch_pos, maxdeltapitch_sign);
		Math_Lib.Pos((SWord)MaxDeltaYaw  , maxdeltayaw_pos,   maxdeltayaw_sign);
		FP factor1 = 1, factor2 = 1;
		if(maxdeltapitch_pos > reqdeltapitch_pos) factor1 = reqdeltapitch_pos / maxdeltapitch_pos;
		if(maxdeltayaw_pos   > reqdeltahdg_pos)   factor2 = reqdeltahdg_pos / maxdeltayaw_pos;
		if(factor1 > factor2) factor1 = factor2;
		pitch += SWord((FP)MaxDeltaPitch * (FP)factor1);
		hdg   += SWord((FP)MaxDeltaYaw   * (FP)factor1);
		fly.cpitch = pitch;

		hdg -= (Angles) CalcHdg ();	//This means that all the heading change is done inside this routine
	}
	else
	{
		if((roll > ANGLES_90Deg) || (roll < ANGLES_270Deg))
			SimpleMoveToRoll(0);
		else
			SimpleMoveToHdg (HdgIntercept);	
		AutoCalcPitch();
	}
	
	if	(ai.PilotSkills.manoeuvre != MANOEUVRE_DIVEANDZOOM)			  //RDH 04/07/99
	{
		if((reqdeltahdg_pos < ANGLES_10Deg) && (reqdeltapitch_pos < ANGLES_10Deg))
		{
			FormationInfo.advantage = TRUE;
			FormationInfo.disadvantage = FALSE;
			MoraleMod();
			SetManoeuvreTime (1000);
		}

		if (		(fly.leadflight)					
				&&	 (fly.leadflight->ai.PilotSkills.manoeuvre == MANOEUVRE_TOPCOVER)
			)//force manoeuvre to allow disengage
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

		if(!TimeLeft())
		{
			if(    (World.Y < FT_1000)
				|| (Range > ENGAGERANGE)
				|| (pitch > ANGLES_30Deg)
				|| (pitch < ANGLES_330Deg) )
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
		}
		if(Math_Lib.rnd(4 * 65536) < (256 - FlyingFactor()))	//Poor skill -> every 20 secs
			ai.PilotSkills.manoeuvre = MANOEUVRE_SCISSORS;
	}
}				

//------------------------------------------------------------------------------
void	AirStruc::SimpleTurnWithPitchChange(ANGLES	HdgIntercept, Bool sitbehindtarget,COORDS3D	target)
{
	if (CombatFactor() > 200)
	{
		if (classtype->phrasename == PHRASE_MIGS)
			SimpleTurnFightSub(HdgIntercept, sitbehindtarget, ANGLES_5Deg, target);	
		else if (World.Y > FT_10000)
			SimpleTurnFightSub(HdgIntercept, sitbehindtarget, ANGLES_0Deg, target);	
		else
			SimpleTurnFightSub(HdgIntercept, sitbehindtarget, ANGLES_0Deg, target);	

	}else
		SimpleTurnFightSub(HdgIntercept, sitbehindtarget, ANGLES_0Deg, target);

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
void	AirStruc::SimpleTurnFightSub(ANGLES	HdgIntercept, Bool sitbehindtarget, ANGLES	deltapitch,COORDS3D	target)
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
	FP desiredturn;
	SLong		time;

	reqdeltahdg = 	hdg - HdgIntercept;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SimpleMoveToThrust (ThrustfromFF());
			SetManoeuvreTime (1000);
			ACM.maxdeltaforclosework = 0.0;
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			ACM.MinHeight (FT_2000);
 			Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);
			reqdeltapitch = PitchIntercept - fly.cpitch;//	fly.cpitch - PitchIntercept;		//CHECK THIS !!!!
			Math_Lib.Pos (reqdeltapitch,reqdeltapitch_pos,reqdeltapitch_sign);

			ANGLES delta = unfriendly->hdg + ANGLES_180Deg + ANGLES_30Deg - hdg;

			{
				if (sitbehindtarget)
					SlowDown(reqdeltahdg_pos);
	
				SimpleTurnFightTargetNotOnNose(reqdeltahdg, reqdeltapitch, reqdeltapitch_sign, reqdeltapitch_pos, deltapitch);

				if  (		(!ManoeuvreTimeLeft())
					)
				{
					if	(ACM.maxdeltaforclosework <= 0.1)
						ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
					else
					{
						if (		(fly.leadflight)					
								&&	 (fly.leadflight->ai.PilotSkills.manoeuvre == MANOEUVRE_TOPCOVER)
							)//force manoeuvre to allow disengage
						{
								ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
						}else
						{//on tail of target so keep in manoeuvre and up morale
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

}
void	AirStruc::SimpleTurnFightTargetNotOnNose(SWord	reqdeltahdg, SWord	reqdeltapitch, SWord	reqdeltapitch_sign, UWord	reqdeltapitch_pos, ANGLES	deltapitch)
{

	SWord delta;
	SWord reqroll;
	SWord dpitch;

	SWord PI = PitchIntercept;
	if  (		(PI < 0)
			&&	(Range > WEAPONSRANGE)
		)
		PI = 0;
	delta = PI + deltapitch;
	if  (		(delta < ANGLES_0Deg)
			&&	(ai.PilotSkills.manoeuvre != MANOEUVRE_DIVEANDZOOM)
			&&	(deltapitch << ANGLES_180Deg)				//if he really wants to go down then let him
			&&	(		(Range > WEAPONSRANGE)
					||	(classtype->phrasename == PHRASE_MIGS)
						
				)
		)	
		delta = ANGLES_0Deg;
	reqdeltapitch = 	delta - fly.cpitch;
	Math_Lib.Pos (reqdeltapitch,reqdeltapitch_pos,reqdeltapitch_sign);
	if (reqdeltapitch_pos > ANGLES_40Deg)
		dpitch = ANGLES_40Deg;
	else
		dpitch = reqdeltapitch_pos;
	if (reqdeltapitch_sign < 0)
		dpitch = - dpitch;

	
	SimpleMoveToHdg (HdgIntercept);	
	PitchIntercept = (Angles) dpitch;
	AutoCalcPitch();
}

//------------------------------------------------------------------------------
//Procedure		ChkAtDesiredHdg
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
Bool	AirStruc::ChkAtDesiredHdg (ANGLES desiredhdg,ANGLES target)

{
	SWord deltahdg;
	UWord	deltahdg_pos;
	SWord	deltahdg_sign;

	deltahdg = desiredhdg -	hdg;
	Math_Lib.Pos (((SWord)deltahdg),deltahdg_pos,deltahdg_sign);
	if (deltahdg_pos < target)
	{
		return (TRUE);
	}else
	{
		return (FALSE);
	}
}
//------------------------------------------------------------------------------
//Procedure		ChkAtDesiredHdg
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
Bool	AirStruc::ChkAtDesiredRoll (ANGLES desiredroll,ANGLES target)

{
	SWord deltaroll;
	UWord	deltaroll_pos;
	SWord	deltaroll_sign;

	deltaroll = desiredroll -	roll;
	Math_Lib.Pos (((SWord)deltaroll),deltaroll_pos,deltaroll_sign);
	if (deltaroll_pos < target)
	{
		return (TRUE);
	}else
	{
		return (FALSE);
	}
}
 
//------------------------------------------------------------------------------
//Procedure		DesiredPitchChk
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
SWord	AirStruc::DesiredPitchChk (ANGLES desiredpitch,ANGLES target)

{
	SWord deltapitch;
	UWord	deltapitch_pos;
	SWord	deltapitch_sign;

	deltapitch = desiredpitch -	fly.cpitch;
	Math_Lib.Pos (((SWord)deltapitch),deltapitch_pos,deltapitch_sign);
	if (deltapitch_pos < target)
	{
		return (TRUE);
	}else
	{
		return (FALSE);
	}


}
//------------------------------------------------------------------------------
//Procedure		DesiredAcPitchChk
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
SWord	AirStruc::DesiredAcPitchChk (ANGLES desiredpitch,ANGLES target)

{
	SWord deltapitch;
	UWord	deltapitch_pos;
	SWord	deltapitch_sign;

	deltapitch = desiredpitch -	pitch;
	Math_Lib.Pos (((SWord)deltapitch),deltapitch_pos,deltapitch_sign);
	if (deltapitch_pos < target)
	{
		return (TRUE);
	}else
	{
		return (FALSE);
	}


}

//------------------------------------------------------------------------------
//Procedure		ChkAtDesiredAcHdg
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
Bool	AirStruc::ChkAtDesiredAcHdg (ANGLES desiredhdg,ANGLES target)

{
	SWord deltahdg;
	UWord	deltahdg_pos;
	SWord	deltahdg_sign;

	deltahdg = desiredhdg -	hdg;
	Math_Lib.Pos (((SWord)deltahdg),deltahdg_pos,deltahdg_sign);
	if (deltahdg_pos < target)
	{
		return (TRUE);
	}else
	{
		return (FALSE);
	}
}

void  AirStruc::SimpleMakeForFriendly()
{
}
void  AirStruc::SimpleImmelmann()
{
}

void  AirStruc::SimpleGainSpeed()
{
	SWord 	deltaroll;
	UWord	deltaroll_pos;
	SWord	deltaroll_sign;
	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SetManoeuvreTime (2000);
 			SimpleMoveToThrust (100);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			SimpleMoveToRoll (ANGLES_0Deg);
			if (World.Y < FT_1000)
				PitchIntercept =  ANGLES_0Deg;
			else
				PitchIntercept =  ANGLES_350Deg;
			AutoCalcPitch();

			if  (!ManoeuvreTimeLeft())
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
void	AirStruc::SimpleLeadUnFriendly ()
{

	COORDS3D	target;
	SLong		t,actualrange;
	AirStrucPtr	unfriendly = *ai.unfriendly;

			target.X = ai.unfriendly->World.X;
			target.Y = ai.unfriendly->World.Y;
			target.Z = ai.unfriendly->World.Z;
			InterceptandRange (&target);
			actualrange = Range;

			//closing velocity
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
			AutoCalcPitch();
			SimpleMoveToHdg (HdgIntercept);

}

//------------------------------------------------------------------------------
//Procedure		GatherSpeed
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
void	AirStruc::SimpleGatherSpeed ()

{
	COORDS3D	target;

		target.X = ai.unfriendly->World.X;
		target.Y = ai.unfriendly->World.Y;
		target.Z = ai.unfriendly->World.Z;
		InterceptandRange (&target);
		PitchIntercept = ANGLES_330Deg;
		AutoCalcPitch();
 		SimpleMoveToHdg (HdgIntercept);
}



//Simple acm routines											//RDH 11Oct96

//------------------------------------------------------------------------------
//Procedure		SimpleTopCover
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	The followers are supposed to go near to home line or
//				not depending on skill
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleTopCover ()

{
  if (formpos == 0)
  {//leader
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			SetManoeuvreTime (500);
			ai.desiredroll = - (ANGLES)ANGLES_30Deg +
 						(Angles)(ANGLES_60Deg * Math_Lib.rnd(2)); 
			ai.PilotSkills.ManStep = PHASE1;
		}


		case PHASE1:
		{
			SimpleTightClimbingTurn (ANGLES_5Deg);
		if (!ManoeuvreTimeLeft())
			{
				SetManoeuvreTime (500);
				ai.PilotSkills.ManStep = PHASE2;
 			}

			break;
		}
		case PHASE2:
		{
			SimpleTightClimbingTurn (ANGLES_0Deg);

			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.ManStep = PHASE3;
 			}

			break;
		}
		case PHASE3:
		{
			SimpleTightClimbingTurn (ANGLES_0Deg);

			if (AllOnTopCover ())								//JIM 25Nov96
			{													//JIM 25Nov96
				Art_Int.AllBreakOff(this);	//fly.leadflight);	//JIM 25Nov96
//TEMP			 	Persons_2.AddMessage(UserMsg::GOHOME,TEXT_SQUADRONREGROUPED,NULL);
			}													//JIM 25Nov96
			else												//JIM 25Nov96
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
 					SimpleMoveToThrust (100);
					SetManoeuvreTime (500);
					ai.desiredroll = -(ANGLES)ANGLES_30Deg +
 								(Angles)(ANGLES_60Deg * Math_Lib.rnd(2)); 
					ai.PilotSkills.ManStep = PHASE1;
				}


				case PHASE1:
				{
					SimpleTightClimbingTurn (ANGLES_5Deg);				//RDH 13Dec96
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
					SimpleTightClimbingTurn (ANGLES_0Deg);
					}else
					{
					SimpleTightClimbingTurn (ANGLES_5Deg);
					}
					break;
				}

  			}
	   }
  }

}

//------------------------------------------------------------------------------
//Procedure		SimpleTightClimbingTurn
//Author		R. Hyde 
//Date			Sat 30 Mar 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SimpleTightClimbingTurn (ANGLES desiredpitch)
{
 
 	SWord	deltapitch;
 
 	deltapitch = desiredpitch - fly.cpitch;
 	PitchIntercept = desiredpitch; 
 	SimpleTightTurnSubSub (deltapitch);
 
}


//------------------------------------------------------------------------------
//Procedure		SimpleLookAround
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleLookAround ()
{

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			SetManoeuvreTime (500);								//RDH 25/6/99
			ai.desiredroll = - (ANGLES)ANGLES_45Deg +
 						(Angles)(ANGLES_90Deg * Math_Lib.rnd(2)); 
			ai.PilotSkills.ManStep = PHASE1;
		}


		case PHASE1:
		{
			if (World.Y > classtype->fightceiling)
				SimpleTightClimbingTurn (ANGLES_0Deg);
			else
				SimpleTightClimbingTurn (ANGLES_5Deg);			//RDH 14Nov96
			if (!ManoeuvreTimeLeft())
			{
		    	ai.PilotSkills.ManStep = PHASE0;
				if (formpos == 0)
				{
					if (AllButLdrNotOnCombat ())				//RDH 26Nov96
					{											//RDH 26Nov96
						ai.PilotSkills.manoeuvre=MANOEUVRE_TOPCOVER;		//RDH 26Nov96
					}else										//RDH 26Nov96
					{											//RDH 26Nov96
						ai.PilotSkills.manoeuvre=MANOEUVRE_LOOKROUND;		//RDH 26Nov96
					}											//RDH 26Nov96
				}else
				{
					ai.PilotSkills.manoeuvre=MANOEUVRE_TOPCOVER;
				}
 			}

			break;
		}
	}

}
//------------------------------------------------------------------------------
//Procedure		BarrelRoll
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleBarrelRollAttack ( )

{
	SimpleScissors ( );
}

//------------------------------------------------------------------------------
//Procedure		Scissors
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleScissors ( )

{
	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			SetManoeuvreTime (1500);
			ai.PilotSkills.ManStep = PHASE1;
			ai.desiredhdg = hdg + ANGLES_120Deg; 
		}
		case PHASE1:
		{
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.ManStep = PHASE3;
			}
			PitchIntercept = ANGLES_2Deg;
			AutoCalcPitch();
			if (SimpleMoveToDesiredHdg (ai.desiredhdg,ANGLES_100Deg))
			{
				ai.PilotSkills.ManStep = PHASE2;
				ai.desiredhdg = hdg - ANGLES_240Deg; 
			}
			break;
		}
		case PHASE2:
		{
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.ManStep = PHASE3;
			}
			PitchIntercept = ANGLES_2Deg;
			AutoCalcPitch();
			if (SimpleMoveToDesiredHdg (ai.desiredhdg,ANGLES_100Deg))
			{
				ai.PilotSkills.ManStep = PHASE1;
				ai.desiredhdg = hdg + ANGLES_240Deg; 
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
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleMildScissors ( )

{
	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
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
			PitchIntercept = ANGLES_0Deg;
			AutoCalcPitch();
			if (SimpleMoveToDesiredHdg (ai.desiredhdg,ANGLES_5Deg))
			{
				ai.PilotSkills.ManStep = PHASE2;
				ai.desiredhdg = hdg + ANGLES_30Deg; 
			}
			break;
		}
		case PHASE2:
		{
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.ManStep = PHASE3;
			}
			PitchIntercept = ANGLES_0Deg;
			AutoCalcPitch();
			if (SimpleMoveToDesiredHdg (ai.desiredhdg,ANGLES_5Deg))
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
Bool	AirStruc::SimpleMoveToDesiredHdg (ANGLES desiredhdg,ANGLES target)

{
	Bool	retval;

	SimpleMoveToHdg (desiredhdg);
	retval = ChkAtDesiredHdg (desiredhdg,target);
	return (retval);
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
void	AirStruc::SimpleMoveToHdg (ANGLES desiredhdg)
 
{
 	SWord	reqdeltahdg, reqdeltahdg_sign;
	UWord	reqdeltahdg_pos;
 	SWord	reqroll,deltaroll;
 
 	reqdeltahdg = 	hdg - desiredhdg;
 	Math_Lib.Pos ((SWord) reqdeltahdg, reqdeltahdg_pos,reqdeltahdg_sign);
 	reqroll = FindRequiredRoll (reqdeltahdg,CombatReqBankData);
	ai.desiredroll = (Angles) reqroll;
	SimpleMoveToRoll (reqroll);
}


//------------------------------------------------------------------------------
//Procedure		MoveToRoll
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
void	AirStruc::SimpleMoveToRoll (SWord desiredroll)

{	//RequiredDRollSub: parameter is desired roll, return val is delta roll

	roll += (Angles) (RequiredDRollSub (desiredroll));
}



//------------------------------------------------------------------------------
//Procedure		MoveToDesiredPitch
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
SWord	AirStruc::SimpleMoveToDesiredPitch (ANGLES desiredpitch,ANGLES target)

{
	SWord	retval;


	PitchIntercept = desiredpitch;
	AutoCalcPitch();
	retval = DesiredPitchChk (desiredpitch,target);
	return (retval);


}

//------------------------------------------------------------------------------
//Procedure		StraightandLevel
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleStraightandLevel ( )

{
	SWord 	deltaroll;
	UWord	deltaroll_pos;
	SWord	deltaroll_sign;
	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SetManoeuvreTime (2000);
			ai.PilotSkills.ManStep = PHASE1;
	 		ai.desiredroll = ANGLES_0Deg;
		}
		case PHASE1:
		{
			//this manoeuvre is only done by poor pilots so we can reduce speed so that
			//other can catch up and attack							  //RDH 20/06/99
			if (vel < MPH250)
				SimpleMoveToThrust (80);
			else
				SimpleMoveToThrust (0);

			if (World.Y < FT_1000)
				PitchIntercept = ANGLES_4Deg;
			else
				PitchIntercept = ANGLES_0Deg;
			AutoCalcPitch();

			deltaroll = (SWord) (ai.desiredroll - roll);
			SimpleMoveToRoll (ai.desiredroll);
			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
			if (deltaroll_pos <ANGLES_2Deg)
			{
				ai.PilotSkills.ManStep = PHASE2;
			}

			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;

		}
		case PHASE2:
		{
			//this manoeuvre is only done by poor pilots so we can reduce speed so that
			//other can catch up and attack							  //RDH 20/06/99
			if (vel < MPH250)
				SimpleMoveToThrust (80);
			else
				SimpleMoveToThrust (0);

			if (World.Y < FT_1000)
				PitchIntercept = ANGLES_10Deg;
			else
				PitchIntercept = ANGLES_0Deg;
			AutoCalcPitch();
			SimpleMoveToRoll (ai.desiredroll);
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;

		}

	}

}

//------------------------------------------------------------------------------
//Procedure		AcmSideSlip
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleAcmSideSlip ( )

{
	
	SimpleSustainedTurn ();
}

//------------------------------------------------------------------------------
//Procedure		SpinRecovery
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleSpinRecovery ()
{
	SimpleSustainedTurn ();

}
//------------------------------------------------------------------------------
//Procedure		SpinOut
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleSpinOut ( )

{
	SWord	deltaroll;
	UWord	deltayaw_pos;
	SWord	deltayaw_sign;


	
	SLong groundlevel = _Collide.LowestSafeAlt(this,			//RDH 16Dec96
											this->pitch,		//RDH 16Dec96
											this->roll);		//RDH 16Dec96


	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (1);
			ai.PilotSkills.ManStep = PHASE1;
			SetManoeuvreTime (600);
			ai.desiredroll = ANGLES_45Deg;
		}
		case PHASE1:
		{
			if ((World.Y - groundlevel) < FT_1000)				//RDH 16Dec96
				PitchIntercept =  ANGLES_8Deg;					//RDH 16Dec96
			else												//RDH 16Dec96
				PitchIntercept =  ANGLES_305Deg;

			AutoCalcPitch();
			SimpleMoveToRoll (ai.desiredroll);
			if (!ManoeuvreTimeLeft())
			{
				ai.desiredroll = ANGLES_0Deg;
				ai.PilotSkills.ManStep = PHASE2;
				SetManoeuvreTime (400);
			}

			break;
		}
		case PHASE2:
		{
			if ((World.Y - groundlevel) < FT_1000)				//RDH 16Dec96
				PitchIntercept =  ANGLES_8Deg;					//RDH 16Dec96
			else												//RDH 16Dec96
			PitchIntercept =  ANGLES_340Deg;
			AutoCalcPitch();
			SimpleMoveToRoll (ai.desiredroll);
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
	 			SimpleMoveToThrust (100);

			}else
			{
				
			}
			break;
		}

	}
		SetControlSurfaces ();

}
//------------------------------------------------------------------------------
//Procedure		DiveforHome
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleDiveforHome ( )

{
	SLong	temp_range;

	temp_range = Range; 	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			ai.PilotSkills.ManStep = PHASE1;
			InterceptandRange (&ai.homebase->World);
			ai.desiredhdg = HdgIntercept; 
			SetManoeuvreTime(0);
		}
		case PHASE1:
		{
			if(!TimeLeft())
			{
				Range = temp_range; 	
				if (Range < DANGERRANGE)
				{
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
				}

				if (World.Y < FT_1000)
					PitchIntercept = ANGLES_0Deg;
				else
					PitchIntercept = ANGLES_10Deg;
				Bool level = SimpleMoveToDesiredHdg (ai.desiredhdg, ANGLES_1Deg);
				if((AutoCalcPitch()) && (level))
					SetManoeuvreTime (1000);
				Range = temp_range; 	
				if (Range > ENGAGERANGE)
				{
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		ClimbforHome
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleClimbforHome ( )

{
	SLong	temp_range;

	temp_range = Range; 	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			ai.PilotSkills.ManStep = PHASE1;
			InterceptandRange (&ai.homebase->World);
			ai.desiredhdg = HdgIntercept; 
			SetManoeuvreTime(0);
		}
		case PHASE1:
		{
			if(!TimeLeft())
			{
				Range = temp_range; 	
				if (Range < DANGERRANGE)
				{
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
				}
				PitchIntercept =  ANGLES_10Deg;
				Bool level = SimpleMoveToDesiredHdg (ai.desiredhdg, ANGLES_1Deg);
				if((AutoCalcPitch()) && (level))
					SetManoeuvreTime (1000);
				Range = temp_range; 	
				if (Range > ENGAGERANGE)
				{
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
				}
			}
		}
	}
}


//------------------------------------------------------------------------------
//Procedure		StraightDive
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleStraightDive ( )

{
	SWord 	deltaroll;
	UWord	deltaroll_pos;
	SWord	deltaroll_sign;
	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SetManoeuvreTime (2000);
 			SimpleMoveToThrust (90);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			SimpleMoveToRoll (ANGLES_0Deg);
			PitchIntercept =  ANGLES_340Deg;
			AutoCalcPitch();
			if (World.Y < FT_1000)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			if  (!ManoeuvreTimeLeft())
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

			break;
		}
	}

}


//------------------------------------------------------------------------------
//Procedure		StandOnTail
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleStandOnTail ( )

{
	COORDS3D	target;
	SLong		time;
	AirStrucPtr	unfriendly = *ai.unfriendly;
	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			SetManoeuvreTime (800);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			target.X = ai.unfriendly->World.X;
			target.Y = ai.unfriendly->World.Y;
			target.Z = ai.unfriendly->World.Z;
			InterceptandRange (&target);

			//time for bullets to arrive at current unfriendly position
			time = Range / MUZZELVEL;
			//calc lead position using this time
			target.X = ai.unfriendly->World.X + (unfriendly->velx * time );
			target.Y = ai.unfriendly->World.Y + (unfriendly->vely * time );
			target.Z = ai.unfriendly->World.Z + (unfriendly->velz * time );
			InterceptandRange (&target);


			SimpleMoveToHdg (HdgIntercept);
			if (vel < MPH55)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}else
			{
				if ((SWord) fly.cpitch < (SWord) PitchIntercept)
				{
					AutoCalcPitch();
				}else
				{
					AutoCalcPitch();
					FireABullet(10);
				}
			}
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;
		}

	}
}
//------------------------------------------------------------------------------
//Procedure		ShootToFrighten
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleShootToFrighten ( )

{
	AirStrucPtr	unfriendly = *ai.unfriendly;
	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			SetManoeuvreTime (200);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			if (!ManoeuvreTimeLeft())
			{
			   	ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre=MANOEUVRE_SUSTAINEDTURN;
			}
			SimpleTightTurn (FT_1000);
			FireABullet(10);
			break;
		}
	}
}





//------------------------------------------------------------------------------
//Procedure		GoHome
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleGoHome ( )

{
	SLong	temp_range;

	temp_range = Range; 	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			SetManoeuvreTime (1000);
			ai.PilotSkills.ManStep = PHASE1;
			InterceptandRange (&ai.homebase->World);
			ai.desiredhdg = HdgIntercept; 
			Range = temp_range;
//jink a bit possibly
			if	(		TargetOnSubjectTail(ANGLES_10Deg)
					&&	(Range < 4 * WEAPONSRANGE)
					&&	(ai.PilotSkills.combatskill >= SKILL_REGULAR)
				)
				ai.desiredhdg = HdgIntercept + ANGLES_20Deg; 
		}
		case PHASE1:
		{
			if (!ManoeuvreTimeLeft())
			{
				SetManoeuvreTime (500);
				ai.PilotSkills.ManStep = PHASE2;
				InterceptandRange (&ai.homebase->World);
				ai.desiredhdg = HdgIntercept; 
				Range = temp_range;
				if	(		TargetOnSubjectTail(ANGLES_10Deg)
						&&	(Range < 4 * WEAPONSRANGE)
						&&	(ai.PilotSkills.combatskill >= SKILL_REGULAR)
					)
					ai.desiredhdg = HdgIntercept + ANGLES_340Deg; 
			}
			if (Range < WEAPONSRANGE)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			PitchIntercept =  ANGLES_0Deg;
			AutoCalcPitch();
			SimpleMoveToHdg (ai.desiredhdg);
			if (Range > COMBATRANGE +METRES1400)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;
		}
		case PHASE2:
		{
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.ManStep = PHASE1;
				SetManoeuvreTime (500);
				InterceptandRange (&ai.homebase->World);


				ai.desiredhdg = HdgIntercept; 
				Range = temp_range;
				if	(		TargetOnSubjectTail(ANGLES_10Deg)
						&&	(Range < 4 * WEAPONSRANGE)
						&&	(ai.PilotSkills.combatskill >= SKILL_REGULAR)
					)
					ai.desiredhdg = HdgIntercept + ANGLES_20Deg; 
			}
			if (Range < WEAPONSRANGE)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			PitchIntercept =  ANGLES_0Deg;
			AutoCalcPitch();
			SimpleMoveToHdg (ai.desiredhdg);
			if (Range > COMBATRANGE+METRES1400)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;
		}
	}
}
//------------------------------------------------------------------------------
//Procedure		MoveAway
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleMoveAway ( )

{
	UWord	roll_pos;
	SWord	roll_sign;
	SWord	desiredaoa;
	SWord deltahdg;
	UWord	deltahdg_pos;
	SWord	deltahdg_sign;
	UWord	elevator_pos;
	SWord	elevator_sign;
	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			SetManoeuvreTime (2000);
			ai.PilotSkills.ManStep = PHASE1;
			break;
		}
		case PHASE1:
		{
			if  		(!ManoeuvreTimeLeft())
				
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}

			PitchIntercept =  ANGLES_0Deg;
			AutoCalcPitch();
			SimpleMoveToHdg (HdgIntercept+ANGLES_180Deg);	//get aidesiredroll

			deltahdg = hdg - (HdgIntercept+ANGLES_180Deg);
			Math_Lib.Pos (((SWord)deltahdg),deltahdg_pos,deltahdg_sign);

			if (deltahdg_pos <ANGLES_5Deg)
			{
				ai.PilotSkills.ManStep = PHASE2;
				SetManoeuvreTime (1500);
			}
			break;
		}

		case PHASE2:
		{
			SimpleMoveToHdg (HdgIntercept+ANGLES_180Deg);	//get aidesiredroll
			PitchIntercept =  ANGLES_4Deg;
			AutoCalcPitch();
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;
		}
	}

}	

//------------------------------------------------------------------------------
//Procedure		AccidentalSpinOut
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	similar to SpinOut, but longer recovery
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleAccidentalSpinOut ( )

{
	SimpleSpinOut ( );
}
//------------------------------------------------------------------------------
//Procedure		TurningFight
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	attempt to lead, find Hdgintercept and delta, set roll
//				roll should not give alt lose
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
//void AirStruc::SimpleTurningFight ()
//
//{
//	SWord	deltapitch;
//	COORDS3D	target;
//	SLong		time;
//	AirStrucPtr	unfriendly = *ai.unfriendly;
//	UWord	acroll_pos,aoa_pos;
//	SWord	acroll_sign,reqroll,aoa_sign;
//	
//	switch (ai.PilotSkills.ManStep)
//	{
//		case PHASE0:
//		{
//			SimpleMoveToThrust (100);
//			SetManoeuvreTime (1500);
//			ai.PilotSkills.ManStep = PHASE1;
//			break;
//		}
//		case PHASE1:
//		{
//			if (!ManoeuvreTimeLeft())
//			{
//				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
//			}
//			if (	(TargetFacingSubject(ANGLES_10Deg))
//				&&	SubjectFacingTarget(ANGLES_10Deg)
//				&&	(Range < METRES150)
//				)
//				{
//					HdgIntercept = HdgIntercept + ANGLES_90Deg;
//					PitchIntercept = PitchIntercept + ANGLES_10Deg;
//				}
//			SimpleTightTurn (FT_1000);
//			break;
//
//		}
//	}
//
//}		
	
//------------------------------------------------------------------------------
//Procedure		LowGYoYo
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleLowGYoYo ( )

{
	UWord	dhdg_pos;
	SWord	dhdg_sign;
	SWord dhdg;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SetManoeuvreTime (1000);
 			SimpleMoveToThrust (100);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			dhdg = (SWord) (hdg - HdgIntercept);
			Math_Lib.Pos (dhdg,dhdg_pos,dhdg_sign);
			if (dhdg_pos < ANGLES_45Deg)
			{
				dhdg_pos = 5*dhdg_pos/4;  
			}
			dhdg = Math_Lib.UseSign (dhdg_pos,dhdg_sign);

		 	ai.desiredroll = (Angles) FindRequiredRoll (dhdg,CombatReqBankData);
			SimpleMoveToRoll (ai.desiredroll);
			if (	(SimpleMoveToDesiredPitch (ANGLES_340Deg,ANGLES_15Deg))
				||	(!ManoeuvreTimeLeft())
				)
			{
				ai.PilotSkills.ManStep = PHASE2;
				SetManoeuvreTime (500);

			}
			break;
		}
		case PHASE2:
		{
			dhdg = (SWord) (hdg - HdgIntercept);
			Math_Lib.Pos (dhdg,dhdg_pos,dhdg_sign);
			if (dhdg_pos < ANGLES_45Deg)
			{
				dhdg_pos = 5*dhdg_pos/4;  
			}
			dhdg = Math_Lib.UseSign (dhdg_pos,dhdg_sign);

		 	ai.desiredroll = (Angles) FindRequiredRoll (dhdg,CombatReqBankData);
			SimpleMoveToRoll (ai.desiredroll);
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;
		}
	}


}
//------------------------------------------------------------------------------
//Procedure		VerticalLoop
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleVerticalLoop ( )

{

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SetManoeuvreTime (1000);
 			SimpleMoveToThrust (100);
			ai.desiredroll = - (ANGLES)ANGLES_45Deg +
 						(Angles)(ANGLES_90Deg * Math_Lib.rnd(2)); 
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			SimpleTightClimbingTurn (ANGLES_7Deg);				//RDH 13Dec96
			if  (!ManoeuvreTimeLeft())
			{
 				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

			}
			break;
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		ImmelmannTurn
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
void AirStruc::SimpleImmelmannTurn ( )

{
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			SimpleTightClimbingTurn (ANGLES_7Deg);
			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		HeadOnOffset
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleHeadOnOffset ( )

{
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
			InterceptandRange(ai.unfriendly);
			SimpleMoveToThrust (ThrustfromFF());
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
			InterceptandRange(ai.unfriendly);
			ANGLES bca = BestClimbAngle();
			if (PitchIntercept > bca) PitchIntercept = bca;
			AutoCalcPitch();

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

			int decisionpoint = Math_Lib.rnd(2);

			if  (		(Range < (2 * BREAKTURNRADIUS ))
					&&	(FF < 200)
				)
			{
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
			{
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
			InterceptandRange(ai.unfriendly);
			ANGLES bca = BestClimbAngle();
			if (PitchIntercept > bca ) PitchIntercept = bca;
			AutoCalcPitch();

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
			SimpleMoveToRoll((Angles)reqroll);

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
//Procedure		StayWithPrey
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	As target looses alt, stay until explosion or hits ground or alt <???
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void AirStruc::SimpleStayWithPrey ( )

{
	
	SWord	acroll_sign,reqroll;
	SWord	deltapitch;
	UWord	acroll_pos;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			SetManoeuvreTime (1500);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			if  (		(!ManoeuvreTimeLeft())
					||	(World.Y < FT_700)
				)
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			SimpleTightTurn (FT_500);


		}
	}


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
void AirStruc::SimpleGainHeight ( )

{
	
	AirStrucPtr		unfriendly;	
	SWord 	deltaroll;
	UWord	deltaroll_pos;
	SWord	deltaroll_sign,desiredaoa;
	
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			unfriendly = *ai.unfriendly;
			if (	(unfriendly->ai.attacker == this)
				||	(World.Y > classtype->fightceiling)
				)
			{
				ai.PilotSkills.manoeuvre=MANOEUVRE_SUSTAINEDTURN;
				SimpleSustainedTurn ();
				break;
			}
			SetManoeuvreTime (2000);
 			SimpleMoveToThrust (100);
			ai.PilotSkills.ManStep = PHASE1;
//##			if ((SWord) fly.turnrate > 0)
//			{
//		 		ai.desiredroll = ANGLES_20Deg;
//			}else
//			{
//		 		ai.desiredroll = ANGLES_340Deg;
//			}
		}
		case PHASE1:
		{
			PitchIntercept = ANGLES_10Deg;
			AutoCalcPitch();
			if(roll > 0)
				SimpleMoveToRoll(ANGLES_30Deg);
			else
				SimpleMoveToRoll(ANGLES_330Deg);

			if (!ManoeuvreTimeLeft())
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;

		}

	}

}



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
void	AirStruc::SimpleSplitS ()

{
	SWord 	deltaroll;
	UWord	deltaroll_pos;
	SWord	deltaroll_sign;

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
				ai.desiredroll = ANGLES_180Deg;
 				SimpleMoveToThrust (100);
				ai.PilotSkills.ManStep = PHASE1;
				if((this != Persons2::PlayerSeenAC) && (this != Persons2::PlayerGhostAC))
					fly.elevator = 0;
			}
		}
		case PHASE1:
		{
			deltaroll = (SWord) (ai.desiredroll - roll);
			SimpleMoveToRoll (ai.desiredroll);
			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
			if (deltaroll_pos <ANGLES_10Deg)
			{
				ai.PilotSkills.ManStep = PHASE2;
	 			SimpleMoveToThrust (0);
//rdh				ai.lastdeltapitch = ANGLES_0Deg;
				SetManoeuvreTime (1500);

			}
			break;
		}

		case PHASE2:
		{

			PitchIntercept = ANGLES_280Deg;
			AutoCalcPitch();
			if ( 		(!ManoeuvreTimeLeft())
					|| (    DesiredPitchChk (ANGLES_270Deg,ANGLES_20Deg)
						)										//RDH 24Jul96
				)
			{
	 			SimpleMoveToThrust (100);
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
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SimpleZoomandDrop ()

{
	SWord phase,deltaroll = 0;
	UWord	deltaroll_pos;
	SWord	deltaroll_sign;

	phase = (SWord) ai.PilotSkills.ManStep;
	switch (phase)
	{
		case PHASE0:
		{
			ai.desiredroll = ANGLES_0Deg;
 			SimpleMoveToThrust (0);
			SetManoeuvreTime (500);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			SimpleMoveToRoll (ai.desiredroll);
			Math_Lib.Pos (deltaroll,deltaroll_pos,deltaroll_sign);
			if (deltaroll_pos <ANGLES_20Deg)
			{
				SetManoeuvreTime (500);
				ai.PilotSkills.ManStep = PHASE2;
			}
			break;
		}
		case PHASE2:
		{
			PitchIntercept = ANGLES_70Deg;
			AutoCalcPitch();
			if (	(SimpleMoveToDesiredPitch (ANGLES_70Deg,ANGLES_10Deg))
				||	(!ManoeuvreTimeLeft())
				)
			{
				ai.PilotSkills.ManStep = PHASE3;
 				SimpleMoveToThrust (100);
			}
		break;
		}

		case PHASE3:
		{
			if (SimpleMoveToDesiredPitch (ANGLES_315Deg,ANGLES_10Deg))
			{
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
			break;
		}


	}

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
void	AirStruc::SimpleDiveAndZoom ()

{
	SWord	deltapitch_pos,deltapitch_sign;
	SLong	temp_range;
	COORDS3D	target;
	SLong		t,actualrange;
	AirStrucPtr	unfriendly = *ai.unfriendly;
 	SWord		deltaroll;										//RDH 14Mar96
	SWord		easestickforward;
	ANGLES		local_PI;


	local_PI  = PitchIntercept;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
 			SimpleMoveToThrust (100);
			ai.PilotSkills.ManStep = PHASE1;
			SetManoeuvreTime (12000);
		}
		case PHASE1:
		{
 			if  (	(		(!ManoeuvreTimeLeft())
						||	((Range - (World.Y - ai.unfriendly->World.Y)) < ENGAGERANGE)
					)
					&&
					(SubjectFacingTarget(ANGLES_45Deg))
				 )
  			{
  				ai.PilotSkills.ManStep = PHASE2;
				SetManoeuvreTime (5000);
  			}
   			if (local_PI << ANGLES_90Deg)
  			{
  				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
  			}
 

			PitchIntercept = ANGLES_5Deg;
			AutoCalcPitch();
			SimpleMoveToHdg (HdgIntercept);
			break;
		}
		case PHASE2:
		{
			SimpleTurningFight();
			if  (			(!ManoeuvreTimeLeft())
					 ||
							(TargetOnSubjectTail(ANGLES_90Deg))
					 ||	
							(Range < METRES200)
				)
			{
			 	ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre=MANOEUVRE_ZOOM;
			}
 	
			break;
		}

	}

}
void AirStruc::SimpleTightTurn (SLong recovery_alt)
{
}
void AirStruc::SimpleTightTurnSubSub (SWord deltapitch)
{
	SLong	desiredroll; 
	UWord	acroll_pos,aoa_pos,dhdg_pos,desiredroll_pos;
	SWord	acroll_sign,reqroll,aoa_sign,dhdg_sign,desiredroll_sign;
		//now do pitch
		PitchIntercept = (Angles)deltapitch + fly.cpitch;
		{
			if (	(deltapitch > 0)
				&&	((SWord) fly.cpitch < ANGLES_15Deg)	//sustainable max climbrate
				)
			{
				AutoCalcPitch();
		
			}else
			{
				if (	(deltapitch <= 0)
					)
				{//can push down
					AutoCalcPitch();
				}else
				{
					PitchIntercept = ANGLES_0Deg;
					AutoCalcPitch();
				}
			}
			Math_Lib.Pos ((SWord)ai.desiredroll,desiredroll_pos,desiredroll_sign);
			
			if (	(fly.cpitch > ANGLES_5Deg)
				&& (desiredroll_pos > ANGLES_20Deg)
				&& (deltapitch > 0)
					)
			
			{
				if (fly.cpitch > ANGLES_15Deg)
				{
						desiredroll_pos = ANGLES_20Deg;
				}else
				{
					desiredroll = ANGLES_20Deg +  (desiredroll_pos - ANGLES_20Deg) *
								(ANGLES_15Deg - fly.cpitch) / ANGLES_15Deg;
					desiredroll_pos = (Angles) desiredroll;
				}
				ai.desiredroll = (Angles) Math_Lib.UseSign (desiredroll_pos,desiredroll_sign);
			}
			SimpleMoveToRoll (ai.desiredroll);
		}
}

//------------------------------------------------------------------------------
//Procedure		SimpleCalcVel
//Author		R. Hyde 
//Date			Fri 25 Oct 1996
//
//Description	rewritten to use the accel calculated via Ps 3/9/98
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SimpleCalcVel ()
{

   		Float	accel, fvel;
		bool positive = true;
		SWord		tvel;
		accel = CalcSimpleAccel ();
		fvel	=  accel * Timer_Code.FRAMETIME * 0.1;
		if (fvel < 0)
		{
			positive = false;
			fvel = -fvel;
		}
		tvel = fvel;
		fvel = 65536 * (fvel - tvel);
		int rnd = Math_Lib.rnd();
		if (fvel > rnd)
			tvel++;
		if (!positive)
			tvel = -tvel;
		vel = vel + tvel;



//	SWord thrust;
//	SLong reqvel;
//
//	thrust = fly.thrustpercent;
//	if (thrust > 10)
//	{
//		reqvel = classtype->maxvel*thrust*thrust/10000;
//	}else
//	{
//		reqvel = MPH30;
//	}
//	if (fly.cpitch >> ANGLES_90Deg)
//	{
//		reqvel = classtype->bestclimbvel + (classtype->maxvel - classtype->bestclimbvel) *
//									(SWord) (classtype->bestclimbpitch - pitch)/(ANGLES)classtype->bestclimbpitch;
//	}
//	CalcVelSub (reqvel);
//	//if we want to go slow then have a double decell
//	if (	(reqvel == MPH30)
//		&&	(Range <METRES70)
//		)
//		CalcVelSub (reqvel);
}

//------------------------------------------------------------------------------
//Procedure		SimpleMoveToThrust
//Author		R. Hyde 
//Date			//RDH 11Oct96
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SimpleMoveToThrust (SWord thrust)

{
	fly.thrustpercent =  thrust * thrust / 100;;

//FC code:
//	SLong	reqvel;
//	SLong	accel;
//	SWord	deltavel,deltavel_sign;	
//	UWord	deltavel_pos;
//	SLong	climb_vel;
//
//	if  (thrust == -100)
//	{
//		if (fly.thrustpercent ==0)
//			fly.thrustpercent =  MAXAUTOTHRUST;
//	}
//	elseif  (thrust == -1)
//	{
//		if (fly.thrustpercent ==MAXAUTOTHRUST)
//			fly.thrustpercent =  0;
//	}
//	else
//		fly.thrustpercent =  thrust;
//	if (fly.thrustpercent > MAXAUTOTHRUST)
//		fly.thrustpercent = MAXAUTOTHRUST;	

}




//------------------------------------------------------------------------------
//Procedure		WeldedWingMan
//Author		R. Hyde							Craig Beeston
//Date			Mon 2 Mar 1998					Tue 27 Apr 1999
//
//Description	Formation flag not set so a/c can spot
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SimpleWeldedWingMan()
{
	
//	stick to the leaders tail, range about 1000ft and weave
//	However for low morale bring wingman closer
//	For very low morale it should appear as if he is trying to stay in formation

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			fly.thrustpercent = ThrustfromFF();
			ai.PilotSkills.ManStep = PHASE1;
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
				else if (!leader)
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
				else
				{
					Bool PullHard = FALSE;
					COORDS3D	target;
					SWord	reqdeltahdg,reqdeltahdg_sign;
					UWord	reqdeltahdg_pos;
					SWord	reqdeltapitch,reqdeltapitch_sign;
					UWord	reqdeltapitch_pos;

					target = leader->World; 									//JIM 02Aug96
					target.Y +=	60 * ai.morale;//FT_500;		Top Moral gives 500 ft

					InterceptandRange(&target);


					reqdeltahdg   =	HdgIntercept - hdg;
					reqdeltapitch =	PitchIntercept - pitch;
					Math_Lib.Pos (reqdeltahdg,reqdeltahdg_pos,reqdeltahdg_sign);
					Math_Lib.Pos (reqdeltapitch,reqdeltapitch_pos,reqdeltapitch_sign);
					
					SWord DesRoll	= 0;
					SWord IdealRoll = 0;
					if(reqdeltapitch != 0)
						IdealRoll = Math_Lib.arctan(reqdeltahdg, reqdeltapitch);
					else
					{
						if(reqdeltahdg > 0) IdealRoll += ANGLES_90Deg;
						if(reqdeltahdg < 0) IdealRoll -= ANGLES_90Deg;
					}

					if((reqdeltahdg_pos > ANGLES_10Deg) || (reqdeltapitch_pos > ANGLES_10Deg))
					{
						DesRoll = IdealRoll;
						PullHard = TRUE;
					}
					else
					{
						if((leader->roll > MAXCOMBATROLL) || (-leader->roll > MAXCOMBATROLL))
						{
							DesRoll = IdealRoll;
							PullHard = TRUE;
						}
						else
						{
							DesRoll = leader->roll;
							DesRoll += 4 * reqdeltahdg;
							MODLIMIT(DesRoll, ANGLES_90Deg);
							PullHard = FALSE;
						}
					}
					SimpleMoveToRoll(DesRoll);

					if(PullHard)
					{
						SWord RollError = (SWord)roll - IdealRoll;
						if((RollError < ANGLES_90Deg) && (-RollError < ANGLES_90Deg))
						{
							SWord sin_roll, cos_roll;
							Math_Lib.high_sin_cos(roll, sin_roll, cos_roll);
							FP MaxDeltaPitch = (FP)CalcMaxPitchRate() * (FP)Timer_Code.FRAMETIME / (100.0 * 32768.0);
							FP MaxDeltaYaw   = MaxDeltaPitch * sin_roll;
							MaxDeltaPitch	 = MaxDeltaPitch * cos_roll;

							SWord	maxdeltapitch_sign, maxdeltayaw_sign;
							UWord	maxdeltapitch_pos,  maxdeltayaw_pos;
							Math_Lib.Pos((SWord)MaxDeltaPitch, maxdeltapitch_pos, maxdeltapitch_sign);
							Math_Lib.Pos((SWord)MaxDeltaYaw  , maxdeltayaw_pos,   maxdeltayaw_sign);
							FP factor1 = 1, factor2 = 1;
							if(maxdeltapitch_pos > reqdeltapitch_pos) factor1 = (FP)reqdeltapitch_pos / (FP)maxdeltapitch_pos;
							if(maxdeltayaw_pos   > reqdeltahdg_pos)   factor2 = (FP)reqdeltahdg_pos / (FP)maxdeltayaw_pos;
							if(factor1 > factor2) factor1 = factor2;
							pitch += SWord((FP)MaxDeltaPitch * (FP)factor1);
							hdg   += SWord((FP)MaxDeltaYaw   * (FP)factor1);
						}
						fly.cpitch = pitch;

						hdg -= (Angles) CalcHdg ();	//This means that all the heading change is done inside this routine
					}
					else
						AutoCalcPitch();

					UWord DesRange = 256 * ai.morale;	// up to 650 metres
					if(DesRange < WEAPONSRANGE) DesRange = WEAPONSRANGE;

					FP DeltaVel = (Range - DesRange) * 0.025;
					MODLIMIT(DeltaVel, 200);
					UWord DesVel = leader->vel + DeltaVel;
					if(vel > DesVel)
					{
						fly.thrustpercent = 0;
						if(vel > leader->vel + 200) vel = leader->vel + 200;
					}
					else
					{
						fly.thrustpercent = 200;
						if(vel < leader->vel - 200) vel = leader->vel - 200;
					}
					
	//#define PRINT_WWM_DATA
	#ifdef PRINT_WWM_DATA
	PrintVar(0, 1, "D_Range %.1f ", (FP)(DesRange / 100.0));
	PrintVar(0, 2, "Range   %.1f ", (FP)(Range / 100.0));
	PrintVar(0, 3, "Pitch   %.1f ", (FP)((SWord)pitch / 182.04));
	PrintVar(0, 4, "Pitch_I %.1f ", (FP)((SWord)PitchIntercept / 182.04));
	PrintVar(0, 5, "Headg   %.1f ", (FP)((SWord)hdg / 182.04));
	PrintVar(0, 6, "Headg_I %.1f ", (FP)((SWord)HdgIntercept / 182.04));
	PrintVar(0, 8, "D_Pitch %.1f ", (FP)(reqdeltapitch / 182.04));
	PrintVar(0, 9, "D_Headg %.1f ", (FP)(reqdeltahdg / 182.04));
	PrintVar(0,10, "DesRoll %.1f ", (FP)(DesRoll / 182.04));

	if(PullHard) PrintString(0, 12, "PULL HARD");
	else		 PrintString(0, 12, "         ");

	PrintVar(0,14, "L_Vel   %.1f ", (FP)(leader->vel / 10));
	PrintVar(0,15, "DesVel  %.1f ", (FP)(DesVel / 10));
	PrintVar(0,16, "Vel     %.1f ", (FP)(vel / 10));

	#endif
				}
			}
		}
	}
	
}				
/*	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			fly.thrustpercent = ThrustfromFF();
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			if (!leader)
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			else
			{
				COORDS3D target;
				target = leader->World; 									//JIM 02Aug96
				target.Y +=	FT_500;
				InterceptandRange(&target);

				SWord reqdeltahdg, reqdeltahdg_sign;
				UWord reqdeltahdg_pos;
				reqdeltahdg = HdgIntercept - hdg;
				Math_Lib.Pos(reqdeltahdg, reqdeltahdg_pos, reqdeltahdg_sign);

				SWord reqdeltapitch, reqdeltapitch_sign;
				UWord reqdeltapitch_pos;
				reqdeltapitch = (SWord)PitchIntercept - (SWord)pitch;
				Math_Lib.Pos (reqdeltapitch, reqdeltapitch_pos, reqdeltapitch_sign);

				
				hdg -= (Angles) CalcHdg ();

				if((reqdeltahdg_pos < ANGLES_1Deg) && (reqdeltapitch_pos < ANGLES_1Deg))
					SimpleMoveToRoll(ANGLES_0Deg);
				else
				{
					if(reqdeltahdg_pos < ANGLES_180Deg)
					{
						SWord DesRoll = 0;
						if(reqdeltapitch != 0)
							DesRoll = Math_Lib.arctan(reqdeltahdg, reqdeltapitch);
						else
						{
							if(reqdeltahdg > 0) DesRoll += ANGLES_90Deg;
							if(reqdeltahdg < 0) DesRoll -= ANGLES_90Deg;
						}
						SimpleMoveToRoll(DesRoll);

#define PRINT_WWM_DATA
#ifdef PRINT_WWM_DATA
PrintVar(0, 2, "Range   %.0f ", (FP)(Range / 100.0));
PrintVar(0, 3, "Pitch   %.0f ", (FP)((SWord)pitch / 182.04));
PrintVar(0, 4, "Pitch_I %.0f ", (FP)((SWord)PitchIntercept / 182.04));
PrintVar(0, 5, "Headg   %.0f ", (FP)((SWord)hdg / 182.04));
PrintVar(0, 6, "Headg_I %.0f ", (FP)((SWord)HdgIntercept / 182.04));
PrintVar(0, 8, "D_Pitch %.0f ", (FP)(reqdeltapitch / 182.04));
PrintVar(0, 9, "D_Headg %.0f ", (FP)(reqdeltahdg / 182.04));
PrintVar(0,10, "DesRoll %.0f ", (FP)(DesRoll / 182.04));
#endif
						SWord rollerror = DesRoll - (SWord)roll;
						if((rollerror < ANGLES_45Deg) && (-rollerror < ANGLES_45Deg))
						{
							SWord sin_roll, cos_roll;
							Math_Lib.high_sin_cos(roll, sin_roll, cos_roll);
							FP MaxDeltaPitch = (FP)classtype->maxpitchrate * (FP)Timer_Code.FRAMETIME / 100.0 / 32768.0;
							FP MaxDeltaYaw   = MaxDeltaPitch * sin_roll;
							MaxDeltaPitch	 = MaxDeltaPitch * cos_roll;
							FP factor1 = 1, factor2 = 1;
							pitch += SWord((FP)MaxDeltaPitch * (FP)factor1);
							hdg   += SWord((FP)MaxDeltaYaw   * (FP)factor1);
							fly.cpitch = pitch;
						}
					}
					else 
					{
						AutoCalcPitch();				
						SimpleMoveToHdg(HdgIntercept);
					}
				}				
				vel = leader->vel;
				if (Range < DANGERRANGE)
					vel -= 100;//CalcVelAlt(0);
				else
					vel += 100;//CalcVelAlt(32000);
			}

		}
	}


}
*/
//------------------------------------------------------------------------------
//Procedure		BailOut				
//Author		R. Hyde 			CSB
//Date			Tue 3 Mar 1998		Sun 28 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SimpleBailOut()
{
//pilot ejects or bails out depending on ac type
//ac in slow turning dive

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SimpleMoveToThrust (0);	 
			SimpleMoveToRoll (0);
			SimpleMoveToDesiredPitch (ANGLES_0Deg, ANGLES_0Deg);
			SetManoeuvreTime(100);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			SimpleMoveToThrust (0);	 
			SimpleMoveToRoll (0);
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
			SimpleMoveToRoll (0);
			SimpleMoveToDesiredPitch (ANGLES_0Deg, ANGLES_0Deg);
			if(!TimeLeft())
			{
				Trans_Obj.LaunchParachute(this,*mobileitem::currworld);
				AutoMoveInfo.movecode = AUTO_DEATHSEQUENCE;
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
void	AirStruc::SimpleLineAbreast()
{
	//only followers go line abreast. We don't link flights

	if (!leader) 
	{//leader, do same as roundabout, get height, dive and then zoom
	 //at end of zoom, choose manoeuvre based on firsttactic
			ai.PilotSkills.ManStep = PHASE0;
			ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;			
	}else
	{//follower
		if (		(Range < WEAPONSRANGE)
				&&	(FormationInfo.information)
			)
		{
			FireABullet(30);
		}
		{
			switch (ai.PilotSkills.ManStep)
			{
				case PHASE0:
				{
		 			SimpleMoveToThrust (100);
					ai.PilotSkills.ManStep = PHASE1;
				}
				case PHASE1:
				{

					
					if (!leader)
						ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;					//if no leader break manoeuvre
					else
					{
						AutoFollowWpWing();

					}

				}
			}
		}
	}


}
//------------------------------------------------------------------------------
//Procedure		Scatter
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
void	AirStruc::SimpleScatter()
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
void	AirStruc::SimplePincer()
{
  	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SimpleMoveToThrust (ThrustfromFF());
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

			PitchIntercept = PitchIntercept;
			AutoCalcPitch();
			SimpleMoveToHdg (HdgIntercept);

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
void	AirStruc::SimpleMultiWave()
{
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
			PitchIntercept = ANGLES_10Deg;
			AutoCalcPitch();
			SimpleMoveToHdg (HdgIntercept);

			if  (		(!ManoeuvreTimeLeft())
					&&	(Range < ENGAGERANGE)
				)
					ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
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
void	AirStruc::SimpleLeadPursuit()
{
//	ifcombatakill high
//		turn in parallel plane so that target still visible
//
//	calc ideal lead
//		could base calc on maintaining collision course(hdg-hdgintercept?)
//	actual lead depends also on skill
//
// end after x secs or within DANGERRANGE
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
void	AirStruc::SimpleLagPursuit()
{
//
//aim for a point off target's tail
//
//
//	acutal lag depends also on skill
//
//
//
// end after x secs or when go behind traget's tail
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
void	AirStruc::SimpleCircumvent()
{
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
//rdhvoid	AirStruc::SimpleDecoyActive()
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
//rdhvoid	AirStruc::SimpleDecoyPassive()
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
void	AirStruc::SimpleRoundAbout()
{
	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SimpleMoveToThrust(ThrustfromFF());
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
			SimpleMoveToRoll ((Angles)desroll);

			PitchIntercept = BestClimbAngle();
			AutoCalcPitch ();
 			if  (		((Range - (World.Y - ai.unfriendly->World.Y)) < ENGAGERANGE)
					&&	((World.Y - ai.unfriendly->World.Y) > FT_3000)
				)
			{
 		    	ai.PilotSkills.ManStep = PHASE0;
				ai.PilotSkills.manoeuvre = MANOEUVRE_DIVEANDZOOM;
			}

		}
	}

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
void	AirStruc::SimpleSplitManoeuvre()
{


	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SimpleMoveToThrust (ThrustfromFF());
			SetManoeuvreTime (1500);
			ai.PilotSkills.ManStep = PHASE1;

		}
		case PHASE1:
		{
			PitchIntercept = ai.desiredpitch;
			AutoCalcPitch();
			SimpleMoveToHdg (ai.desiredhdg);
			if  (		(ChkAtDesiredHdg (ai.desiredhdg,ANGLES_40Deg))
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
void	AirStruc::SimpleHeadOn ( )
{
//use desiredpitch and desiredhdg to determine offset
// distance = 2 WINGSPANS
	//just pass to the side
	//poor pilots may turn too early
	//good pilots may turn early
	//on passing pilot skill will dtermine action

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
			InterceptandRange(ai.unfriendly);
			SimpleMoveToThrust (ThrustfromFF());
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
			InterceptandRange(ai.unfriendly);
			ANGLES bca = BestClimbAngle();
			if (PitchIntercept > bca) PitchIntercept = bca;
			AutoCalcPitch();

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
		 	ANGLES reqroll = (Angles)FindRequiredRoll (reqdeltahdg,CombatReqBankData);

			int decisionpoint = Math_Lib.rnd(2);

			if  (		(Range < (2 * BREAKTURNRADIUS ))
					&&	(FF < 200)
				)
			{
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
			{
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
			InterceptandRange(ai.unfriendly);
			ANGLES bca = BestClimbAngle();
			if (PitchIntercept > bca ) PitchIntercept = bca;
			AutoCalcPitch();

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
			SimpleMoveToRoll((Angles)reqroll);

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
void	AirStruc::SimpleLineAstern()
{
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
	if (!leader) 
	{//leader, do same as roundabout, get height, dive and then zoom
	 //at end of zoom, choose manoeuvre based on firsttactic
			ai.PilotSkills.ManStep = PHASE0;
			ai.PilotSkills.manoeuvre = MANOEUVRE_ROUNDABOUT;			
	}else
	{//follower
		if (Range < 2 * WEAPONSRANGE)
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
					SimpleMoveToThrust (ThrustfromFF());
					ai.PilotSkills.ManStep = PHASE1;
				}
				case PHASE1:
				{
					if (!leader)
						ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;					//if no leader break manoeuvre
					else
					{
 						COORDS3D	target;
						AirStrucPtr MyLeader = Leader();
						for(;;)
						{
							AirStrucPtr Foll = MyLeader->Follower();
							if(Foll == this)
								break;			 
							MyLeader = Foll;
						}
 						target = MyLeader->World; 
						target.X += 10 * MyLeader->velx;
						target.Y += 10 * MyLeader->vely;
						target.Z += 10 * MyLeader->velz;
 						InterceptandRange(&target);

						AutoCalcPitch();
						SWord DeltaHdg = HdgIntercept - hdg;
						if((DeltaHdg > ANGLES_0Deg) && (DeltaHdg < ANGLES_HalfDeg))
							HdgIntercept = hdg + ANGLES_HalfDeg;
						if((-DeltaHdg > ANGLES_0Deg) && (-DeltaHdg < ANGLES_HalfDeg))
							HdgIntercept = hdg - ANGLES_HalfDeg;

						CalcHdgRoll();
						SLong DesVel = MyLeader->vel;
						Range -= MyLeader->vel * 10;
						if (Range < METRES200)
							DesVel -= 100;
 						else
						{
							SLong DeltaVel = (Range - METRES400) / 10;
							MODLIMIT(DeltaVel, 100);
							DesVel += DeltaVel;
						}
						if(DesVel > vel) SimpleMoveToThrust(200);
						else			 SimpleMoveToThrust(0);	

					}
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
void AirStruc::SimpleShootTooEarly()
{
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
void	AirStruc::SimpleLagRoll()
{
//maintain throttle
//roll towards unfriendly for 10-20deg hdg (f(AOT))
//go upside down
//climb to loose speed to corner speed
//exit manoeuvre when unfriendly goes behind or time out

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
void	AirStruc::SimpleExtensionManoeuvre()
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
//if turning good pilots will initially unload and
//retain roll to disguise manoeuvre 
//
//initially aircraft could be closing

}
void	AirStruc::SimpleDivingSpin()
{
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
void	AirStruc::SimpleReverseTurn()
{
//	DesiredRoll = opposite of existing
//	Move to desired and hold for 3 secs
//	if (good)
//		out of plane
//	Close brakes if open at end of manoeuvre(used in selfasbait())
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
void	AirStruc::SimpleSelfAsBait()
{
//
//let bandit out turn you
//just as he get lead, pop brakes
//goto RevereTurn
//
//this could be where bandit is on your tail, wait for him to get close 
//before turning
//keep buddy in combat spread so that you can sandwich him
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
void	AirStruc::SimpleJink()
{
//
//fast 90 deg roll
//hard push or pull on stick
//hold for 2-3 secs
//
//flyingability determines rollrate and stick push pull
//combatablity determines length of manoeuvre
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
void	AirStruc::SimpleBreakTurn()
{
//	roll = cos -1 (1/maxg)
//ability to reach roll = f(flyingskill)
//ability to hold corner vel = f(flyingskill)
//	option to nose down to keep speed up and nose up to reduce speed
//hold for 4-5 secs
//
//low morale gives a very lazy turn
//
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
void	AirStruc::SimpleLazyTurn()
{
//
//	Choose a roll 10-40deg
//		= f(morale)
//	continue on turn for 10s
//	if (ai.morale > MORALE_GOOD)
//		if (Range < WEAPONS)
//			end manoeuvre
//
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
void	AirStruc::SimpleBreak180()
{
//roll 180
//pull hard until pitch 45 deg down
//slowly unroll for x secs
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
void	AirStruc::SimpleBreakLow()
{
//decide which way to roll to go down
//roll 90
//pull hard until pitch 45 deg down
//slowly unroll for x secs
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
void	AirStruc::SimpleBreakHigh()
{
//decide which way to roll to go up
//roll 90
//pull hard until pitch 45 deg up
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
void	AirStruc::SimpleBreak90()
{
//	if (classtype->aerobaticfactor == AEROBATIC_MIG)
//		BreakHigh();
//	else if (	(ai.morale > MORALE_MEDIUM)
//			&&	(ClosureLessThan(MPH50))
//			)
//				BreakHigh();
//			else
//				BreakLow();
}

//------------------------------------------------------------------------------
//Procedure		HiGBarrelRoll
//Author		R. Hyde						Craig Beeston
//Date			Mon 9 Mar 1998				Sun 13 Jun 1999
//
//Description	1.30	
//				Defensive manoeuvre
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SimpleHiGBarrelRoll()
{
	switch(ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SimpleMoveToThrust(0);
			ai.PilotSkills.ManStep = PHASE1;
		}
		case PHASE1:
		{
			PitchIntercept = ANGLES_0Deg;
			AutoCalcPitch();
			SWord DesRoll = ANGLES_45Deg;	DesRoll = -DesRoll;
			SimpleMoveToRoll(DesRoll);

			if(((SWord)pitch == (SWord)PitchIntercept) && ((SWord)roll == DesRoll))
				ai.PilotSkills.ManStep = PHASE2;
		
			break;
		}
		case PHASE2:
		{
			PitchIntercept = ANGLES_45Deg;
			AutoCalcPitch();
			SWord DesRoll = 2 * SWord(pitch);
			SimpleMoveToRoll(DesRoll);

			if(((SWord)pitch == (SWord)PitchIntercept) && ((SWord)roll == DesRoll))
				ai.PilotSkills.ManStep = PHASE3;
		
			break;
		}
		case PHASE3:
		{
			PitchIntercept = ANGLES_0Deg;
			AutoCalcPitch();
			SWord DesRoll = ANGLES_180Deg - 2 * SWord(pitch);
			SimpleMoveToRoll(DesRoll);

			SWord dHdg = CalcHdg();
			hdg -= 2 * dHdg;

			if(((SWord)pitch == (SWord)PitchIntercept) && ((SWord)roll == DesRoll))
				ai.PilotSkills.ManStep = PHASE4;
		
			break;
		}
		case PHASE4:
		{
			PitchIntercept = ANGLES_45Deg;	PitchIntercept = -PitchIntercept;
			AutoCalcPitch();
			SWord DesRoll =  -2 * SWord(pitch) - ANGLES_180Deg;
			SimpleMoveToRoll(DesRoll);

			SWord dHdg = CalcHdg();
			hdg -= 2 * dHdg;

			if(((SWord)pitch == (SWord)PitchIntercept) && ((SWord)roll == DesRoll))
				ai.PilotSkills.ManStep = PHASE5;
		
			break;
		}
		case PHASE5:
		{
			PitchIntercept = ANGLES_0Deg;
			AutoCalcPitch();
			SWord DesRoll =  2 * SWord(pitch);
			SimpleMoveToRoll(DesRoll);

			if(((SWord)pitch == (SWord)PitchIntercept) && ((SWord)roll == DesRoll))
			{
				SimpleMoveToThrust(ThrustfromFF());
				ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;
			}
		
			break;
		}
	}
	
//max g, roll in one direction
//nose low or high
//unbalanced flight, power reduction, increased drag for better pilots
//
//
}
//------------------------------------------------------------------------------
//Procedure		PanicTurn
//Author		R. Hyde 
//Date			Mon 9 Mar 1998
//
//Description	1.30
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SimplePanicTurn()
{
//
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
void	AirStruc::SimpleUnBalancedFlight()
{
//skid and slip while making turns
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
void	AirStruc::SimpleTrackingShot()
{
//aim for gun solution
//actually modify pitch and heading within limits on top of changes produced by manoeuvre
//just enough lead
//modify speed(rpm and airbrakes) to stay on tail
//
//
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
void	AirStruc::SimpleSnapShot()
{
//
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
void	AirStruc::SimpleStayOnTail()
{
//	if necessayr shed energy:
//		turn
//		brakes
//		throttle
//	to stay on tail

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
Bool	AirStruc::SimpleTurnToHdgAndPitch()
{
	//return true on success
	return(TRUE);
}

//------------------------------------------------------------------------------
//Procedure		LagFight
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
void	AirStruc::SimpleHotSideLagPursuitFight()
{
//1	trgItem is behind target
//	TurningFightSub()
}

//------------------------------------------------------------------------------
Bool	AirStruc::SimpleSandwichManoeuvres()
{
	return(TRUE);
}
//void	AirStruc::SimpleTurningFight()
//{
//}
//void	AirStruc::SimpleTightTurnSubtSub(ANGLES	HdgIntercept)
//{
//}
//
//------------------------------------------------------------------------------
//Procedure		SustainedTurn
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
void	AirStruc::SimpleSustainedTurn()
{
//1	GetRequiredRoll(srcItem,trgItem)
//2	pull on elevators so that Drag = Thrust 
//			this is the max sustainable turn, in-plane
//turn to minimise the difference between hdgintercept and hdg, go shortest way
//
//
//	roll = f(morale, flyingskill)
//	drag = f(combatskill)
//
//
//	if (skill high)
//			check lead/lag to avoid turning into target
//
//	turn at best sustained turnrate  (how near = f(skill), maintain altitude if possible
}
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
void	AirStruc::SimpleHighYoYo()
{
//approach from 30-60 AOT, cospeed
//
//roll wings level
//pull up
//when in rear hemisphere (say near 6 o'clock)
//	roll towards target then lead or lag pursuit
//
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
void	AirStruc::SimpleZoom()
{
//roll wings nearly level
//pull up then zoom climb for x secs or near stall
//roll 180 and pull level
//
//
//	ACM.Step = ai.PilotSkills.ManStep;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SetManoeuvreTime (2000);

			SimpleMoveToThrust (100);			//rdh
//			ACM.ClimbRate (0);
//			ACM.KeepWingsLevel ();

			ai.PilotSkills.ManStep = PHASE1;
			break;
		}
		case PHASE1:
		{
				SimpleMoveToRoll (ANGLES_0Deg);
				PitchIntercept = ANGLES_40Deg;
				AutoCalcPitch ();
			

			if  (		(!ManoeuvreTimeLeft ())
					||
						(vel <= CornerSpeed())
					||
						((World.Y - ai.unfriendly->World.Y)	>	FT_4000)
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
void	AirStruc::SimpleLowYoYo ( )
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
void	AirStruc::SimpleInterceptHigh()
{
//intercept but stay at alt
//
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
void	AirStruc::SimpleClimbAtSustainedTurnSpeed()
{
//climb but don't go below sustained turn speed
//aim towards unfriendly plus offset = f(desiredrange,desiredhdg)

}

//------------------------------------------------------------------------------
//Procedure		
//Author		R. Hyde 
//Date			 //RDH 25/05/99
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	AirStruc::SimpleLowAlt()
{

	ANGLES dpitch;

	switch (ai.PilotSkills.ManStep)
	{
		case PHASE0:
		{
			SimpleMoveToThrust (ThrustfromFF());
			SetManoeuvreTime (800);
			if (TargetFacingSubject(ANGLES_45Deg))
				ai.desiredpitch = ANGLES_5Deg;
			else
				ai.desiredpitch = ANGLES_10Deg;
			ai.PilotSkills.ManStep = PHASE1;

		}
		case PHASE1:
		{

			PitchIntercept = ai.desiredpitch;
			AutoCalcPitch();
			SimpleMoveToHdg (HdgIntercept);

			if  (!ManoeuvreTimeLeft())
							ai.PilotSkills.manoeuvre = MANOEUVRE_SELECT;

		}
	}


}

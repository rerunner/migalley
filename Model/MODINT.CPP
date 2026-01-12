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
//Filename       modint.cpp
//System         
//Author         Andrew McRae
//Date           Mon 24 Feb 1997
//Description    Flight model interface functions
//------------------------------------------------------------------------------

#include "_MODE.H" //RERUN

#define F_GRAFIX												//RJS 09Dec97
#define F_BATTLE												//RJS 09Dec97

#include "DOSDEFS.H"
#include "WORLDINC.H"
#include "MODVEC.H"
#include "MODEL.H"
#include "RANGES.H"
#include "MYANGLES.H"
#include "PLANETYP.H"
#include "COLLIDED.H"
#include "MYTIME.H"
#include "MYERROR.H"
#include "MONOTXT.H"
//#include "files.g"
#include "FILES.H"
#include "MILES.H"
#include "REPLAY.H"
#include "PERSONS2.H"
#include "FLYMODEL.H"
#include "3DCOM.H"
#include "MISSMAN2.H" // RERUN

#define MODLIMIT(val,lim) 	if (val>lim) val=lim; \
							elseif (val<-lim) val=-lim

//------------------------------------------------------------------------------
//Procedure		ResetDeltaPos
//Author		Andrew McRae
//Date			Wed 5 Feb 1997
//
//Description	Change Aircraft Position (in cms)
//
//Inputs		AirStrucPtr & a change in position
//
//Returns	
//
//------------------------------------------------------------------------------
void Model::ResetDeltaPos (AirStrucPtr const ControlledAC,SLong x, SLong y, SLong z)
{

	Pos.x += x;
	Pos.y += y;
	Pos.z += z;

	SetAirStrucPosition (ControlledAC);

}

//------------------------------------------------------------------------------
//Procedure		ResetPosition
//Author		Andrew McRae
//Date			Wed 26 Feb 1997
//
//Description	Change position to ...
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Model::ResetPosition (AirStrucPtr const ControlledAC,SLong x, SLong y, SLong z)
{

	Pos.x = x;
	Pos.y = y;
	Pos.z = z;

	SetAirStrucPosition (ControlledAC);

}


//------------------------------------------------------------------------------
//Procedure		ResetVelocity
//Author		Andrew McRae
//Date			Wed 26 Feb 1997
//
// Reset aircraft velocity (cms/csec)
// Pass TRUE to reset aircraft orientation to align with new velocity // vector
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Model::ResetVelocity (AirStrucPtr const ControlledAC,SLong x, SLong y, SLong z, Bool Align)
{

	Vel.x = x;
	Vel.y = y;
	Vel.z = z;

	SetAirStrucVelocity (ControlledAC);

	if (Align)
	{
		Ori.z.x = Vel.x;
		Ori.z.y = Vel.y;
		Ori.z.z = Vel.z;

		NrmVec (Ori.z, Ori.z);

		Ori.y.x = 0;
		Ori.y.y = 1;
		Ori.y.z = 0;

		CPrdX (Ori);
		CPrdY (Ori);

		SetAirStrucOrientation (ControlledAC);
	}
}


//------------------------------------------------------------------------------
//Procedure		ResetAngles
//Author		Andrew McRae
//Date			Wed 26 Feb 1997
//
// 		Reset aircraft orientation (RowanAngles)
// 		order of rotation is bearing, pitch then roll
// 		positive bearing is clockwise looking down onto the ground
// 		positive pitch is nose up
// 		positive roll is right wing down
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Model::ResetAngles (AirStrucPtr const ControlledAC,UWord hdg, UWord pitch, UWord roll)
{

	Ori.x.x = 1;
	Ori.x.y = 0;
	Ori.x.z = 0;
	Ori.y.x = 0;
	Ori.y.y = 1;
	Ori.y.z = 0;
	Ori.z.x = 0;
	Ori.z.y = 0;
	Ori.z.z = 1;

	SetOri (Ori, Rowan2Rads(pitch), Rowan2Rads(hdg), Rowan2Rads(roll));

	SetAirStrucOrientation (ControlledAC);

}


//TempCode ARM 27Aug97 //------------------------------------------------------------------------------
//TempCode ARM 27Aug97 //Procedure		HaveWeLanded
//TempCode ARM 27Aug97 //Author		Andrew McRae
//TempCode ARM 27Aug97 //Date			Mon 24 Feb 1997
//TempCode ARM 27Aug97 //
//TempCode ARM 27Aug97 //Description	
//TempCode ARM 27Aug97 //
//TempCode ARM 27Aug97 //Inputs		
//TempCode ARM 27Aug97 //
//TempCode ARM 27Aug97 //Returns	
//TempCode ARM 27Aug97 //
//TempCode ARM 27Aug97 //------------------------------------------------------------------------------
//TempCode ARM 27Aug97 Collide::Pos Model::HaveWeLanded (AirStruc*ac, ANGLES& pitch, ANGLES& roll)
//TempCode ARM 27Aug97 {
//TempCode ARM 27Aug97 	return _Collide.LANDED_OK;
//TempCode ARM 27Aug97 }


//------------------------------------------------------------------------------
//Procedure		UpdateAirStruc
//Author		Andrew McRae
//Date			Tue 7 Jan 1997
//
//Description	Updates AirStruc structure with new data
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Model::UpdateAirStruc (AirStrucPtr const ControlledAC)
{

//	if (_Replay.Record || _Replay.Playback)
//		_Replay.Record=_Replay.Record;

	// Velocity
	SetAirStrucVelocity (ControlledAC);
	// Orientation
	SetAirStrucOrientation (ControlledAC);
	// Position
	SetAirStrucPosition (ControlledAC);
}

//------------------------------------------------------------------------------
//Procedure		SetAirStrucPosition
//Author		Andrew McRae
//Date			Tue 7 Jan 1997
//
//Description	Cleanly changes AirStuc position in the world
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Model::SetAirStrucPosition (AirStrucPtr const ControlledAC)
{

// 	WorldStuff* World = ControlledAC->GetCurrWorld ();
// 	World->RemoveFromWorld (ControlledAC);
	FCRD CoGOffset;
	TnsPnt(ShapeCoG, CoGOffset, Ori);

/* TEST CODE CSB 18/06/99 */	COORDS3D OldWorld = ControlledAC->World;
	
	ControlledAC->World.X = (SLong)(Pos.x + 0.5 - CoGOffset.x);
	ControlledAC->World.Y = (SLong)(Pos.y + 0.5 - CoGOffset.y);
	ControlledAC->World.Z = (SLong)(Pos.z + 0.5 - CoGOffset.z);


	ControlledAC->uniqueID.bitfieldUid.changed = TRUE;

// 	World->AddToWorld (ControlledAC);

}

//------------------------------------------------------------------------------
//Procedure		SetAirStrucOrientation
//Author		Andrew McRae
//Date			Tue 7 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Model::SetAirStrucOrientation (AirStrucPtr const ControlledAC)
{

	// sheading
	if ((Ori.z.x == 0) && (Ori.z.z == 0))
		fHdg = CalcAngle (Ori.y.z, Ori.y.x);
	else
		fHdg = CalcAngle (Ori.z.z, Ori.z.x);

	// pitch
	fPitch = CalcAngle (VecLen2D (Ori.z.x, Ori.z.z), Ori.z.y);

	// roll
	FCRD vecx;
	FCRD vecz;
	FP sin, cos;

	sin = Ori.z.x;
	cos = Ori.z.z;
	NrmVec2D (sin, cos);
	RotVecYSC (Ori.x, vecx, -sin, cos);
	RotVecYSC (Ori.z, vecz, -sin, cos);

	RotVecXSC (vecx, vecx, vecz.y, vecz.z);
	fRoll = CalcAngle (vecx.x, -vecx.y);

	// These are Global Angles
	ControlledAC->hdg	= (Angles)(Rads2Rowan(fHdg));
//	ControlledAC->fly.chdg	= ControlledAC->hdg;
	ControlledAC->pitch	= (Angles)(Rads2Rowan(fPitch));
	ControlledAC->roll	= (Angles)(Rads2Rowan(fRoll));

	FP	velpitch = CalcAngle (VecLen2D(Vel.x, Vel.z), Vel.y);
	velpitch = AngleSign (velpitch);
	ControlledAC->fly.cpitch = (Angles)(Rads2Rowan(velpitch));

}


//------------------------------------------------------------------------------
//Procedure		SetAirStrucVelocity
//Author		Andrew McRae
//Date			Tue 7 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Model::SetAirStrucVelocity (AirStrucPtr const ControlledAC)
{
	SLong wx, wy, wz;	
	MMC.Sky.GetWind(Pos.y, wx, wy, wz);

	ControlledAC->velx = (SWord)(10 * Vel.x - wx);
	ControlledAC->vely = (SWord)(10 * Vel.y - wy);
	ControlledAC->velz = (SWord)(10 * Vel.z - wz);

	ControlledAC->velhori = (SWord)(VecLen2D(ControlledAC->velx, ControlledAC->velz) );	//CSB 10/06/99	
	ControlledAC->vel = (SWord)(VecLen2D(ControlledAC->velhori, ControlledAC->vely) );	//CSB 10/06/99	
}

//------------------------------------------------------------------------------
//Procedure		UpdateModel
//Author		Andrew McRae
//Date			Fri 2 May 1997
//
//Description	Sets up Model data from AirStruc
//
//Inputs		none
//
//Returns		none
//
//------------------------------------------------------------------------------
void Model::UpdateModel (AirStrucPtr const ControlledAC)
{
	// Orientation
	Ori.x.x = 1;
	Ori.x.y = 0;
	Ori.x.z = 0;
	Ori.y.x = 0;
	Ori.y.y = 1;
	Ori.y.z = 0;
	Ori.z.x = 0;
	Ori.z.y = 0;
	Ori.z.z = 1;
	if (ControlledAC->pitch==ANGLES_90Deg)
		ControlledAC->pitch=ANGLES_90BareDeg;
	else
	if (ControlledAC->pitch==ANGLES_270Deg)
		ControlledAC->pitch=ANGLES_270BareDeg;
	RotOriYVec (Ori, -Rowan2Rads(ControlledAC->hdg));
	RotOriXVec (Ori, Rowan2Rads(ControlledAC->pitch));
	RotOriZVec (Ori, Rowan2Rads(ControlledAC->roll));

	CPrdX (Ori);
	CPrdY (Ori);

	NrmVec (Ori.x, Ori.x);
	NrmVec (Ori.y, Ori.y);
	NrmVec (Ori.z, Ori.z);

	// Velocity
	SLong wx, wy, wz;
	MMC.Sky.GetWind(ControlledAC->World.Y, wx, wy, wz);
	Vel.x = (Ori.z.x * ControlledAC->vel + wx) / 10;
	Vel.y = (Ori.z.y * ControlledAC->vel + wy) / 10;
	Vel.z = (Ori.z.z * ControlledAC->vel + wz) / 10;

	NullVec (RotVel);

	// Position
	Coords3D w,c1,c2;
	FCRD CoGOffset;

	if (ShapeCoG.z==0.0 && ShapeCoG.y==0.0 && ShapeCoG.x==0.0)//AMM 30Jul99
	{
		SHAPE.GetContactPoint(ControlledAC, CT_GRAVITY, w, c1, c2);
		ShapeCoG.x = c1.X;
		ShapeCoG.y = c1.Y;
		ShapeCoG.z = c1.Z;
	}

	TnsPnt(ShapeCoG, CoGOffset, Ori);

	Pos.x = ControlledAC->World.X + CoGOffset.x;
	Pos.y = ControlledAC->World.Y + CoGOffset.y;
	Pos.z = ControlledAC->World.Z + CoGOffset.z;
}




// SURFACE ANIMATIONS


//------------------------------------------------------------------------------
//Procedure		ModelAnim
//Author		Andrew McRae
//Date			Wed 2 Apr 1997
//
//Description	MA_SUSPENSION
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ModelAnim::ModelAnim (AirStrucPtr const ControlledAC,MODELANIMTYPE type, PGEAR gear, void* pbyte, ULong min, ULong max)
{

	Type = type;

	pModel = gear->pModel;

	pGear = gear;

	pOffset = ControlledAC->Anim.Offset(pbyte);// (ULong)pbyte - (ULong)(ControlledAC->Anim);
	pAnim = NULL;

	Min = min;
	Max = max;

	List.Attach (&gear->pModel->AnimList, this);

}


//------------------------------------------------------------------------------
//Procedure		ModelAnim
//Author		Andrew McRae
//Date			Wed 2 Apr 1997
//
//Description	MA_SURFACE & MA_GENERICFP
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ModelAnim::ModelAnim (	AirStrucPtr		ControlledAC,
						MODELANIMTYPE	type,
						PMODEL			pmodel,
						void*			psrce,
						void*			pbyte,
						SLong			minval,
						SLong			maxval,
						SLong			scale,
						Bool			AnimAC)
{

	Type = type;

	pModel = pmodel;

	pGear = NULL;

	pOffset = ControlledAC->Anim.Offset(pbyte);//(ULong)pbyte - (ULong)(ControlledAC->Anim);
	if (AnimAC)
	{
		pSrcOffset = (UByteP)psrce - (UByteP)ControlledAC;
		pSrce = NULL;
	}
	else
	{
		pSrcOffset = -1;
		pSrce = psrce;
	}

	pAnim = NULL;

	Min = minval;														//RJS 14May98
	Max = maxval;														//RJS 14May98

	Scale = Max - Min;													//RJS 14May98

	List.Attach (&pModel->AnimList, this);

}

//------------------------------------------------------------------------------
//Procedure		ModelAnim
//Author		Andrew McRae
//Date			Fri 21 Mar 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ModelAnim::~ModelAnim ()
{
	List.Detach (this);
}


//------------------------------------------------------------------------------
//Procedure		ProcAnimUnity
//Author		Andrew McRae
//Date			Wed 2 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void ModelAnim::ProcAnimSuspension ()
{

  	if (!pGear->Touched)
  	{
  		*(UByte*)pAnim = Min;
  		return;
  	}
  
	FP fVal = pGear->LegPos * 255;
  	SLong val = (int)fVal;//(pGear->IntL) * 255;
  
  	if (val > (SLong)Max) val = Max;
  	if (val < (SLong)Min) val = Min;
  
  	*(UByte*)pAnim = (UByte)val;

}

//static	int	poop;

//------------------------------------------------------------------------------
//Procedure		ProcAnimSurface
//Author		Andrew McRae
//Date			Wed 2 Apr 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void ModelAnim::ProcAnimSurface ()
{
	SLong num = (SLong)*(SWord*)pSrce;

	num *= 255;
	num /= Scale;

	*(UByte*)pAnim = num;

//	num = (num * Scale) >> 13;
//	if (num == -1) num = 0;
//	UByte val = (UByte)(num >> 1);
//
//	val += 128;
//	if (val == 0) val = 1;
//	if (val == 128) val = 0;
//
//	*(UByte*)pAnim = (UByte)val;
}

//------------------------------------------------------------------------------
//Procedure		ProcAnimGenericFP
//Author		Andrew McRae
//Date			Mon 29 Sep 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void ModelAnim::ProcAnimGenericFP ()
{
//	ULong	val = (ULong) (*(PFP)pSrce);
	ULong	val = (*(SWordP)pSrce);

	*(UWord*)pAnim = (val * Scale)>>16;
}


//------------------------------------------------------------------------------
//Procedure		Animations
//Author		Andrew McRae
//Date			Mon 13 Jan 1997
//
//Description	animation functions
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void Model::Animations (AirStrucPtr const ControlledAC)
{

	PMODELANIM pModelAnim = AnimList;

	if (ControlledAC)														//RJS 27May98
	{
		while (pModelAnim != NULL)
		{
			pModelAnim->pAnim = (void*)&ControlledAC->Anim[(int)pModelAnim->pOffset];//(pModelAnim->pOffset + (ULong)(ControlledAC->Anim));
			switch (pModelAnim->Type)
			{

				case MA_SURFACE:
					pModelAnim->pSrce = (void*)(pModelAnim->pSrcOffset + (ULong)ControlledAC);//RJS 20Jul98
				case MA_AEROSURFACE:											//RJS 20Jul98
					pModelAnim->ProcAnimSurface ();
					break;

				case MA_SUSPENSION:
					pModelAnim->ProcAnimSuspension ();
					break;

				case MA_GENERICFP:
					pModelAnim->pSrce = (void*)(pModelAnim->pSrcOffset + (ULong)ControlledAC);//RDH 10Sep98
					pModelAnim->ProcAnimGenericFP ();
					break;

			}

			pModelAnim = pModelAnim->List.NextItem ();
		}
	}

	// Gear Up/Down

	if (Gear)
		SetGearAnimPos(ControlledAC,ANIM_GEAR_AUTO);							//RJS 08Apr98

}

//------------------------------------------------------------------------------
//Procedure		SetGearAnimPos
//Author		Robert Slater
//Date			Wed 8 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Model::SetGearAnimPos(AirStrucPtr const ControlledAC,SWord	val,Bool	killnow)
{
	AircraftAnimData* adptr = (AircraftAnimData* )ControlledAC->Anim;
	SLong	geardamage = 0;										//RJS 07Sep98

	if (killnow==TRUE)											//RJS 07Sep98
		adptr->animtoggle = 1;									//RJS 07Sep98
	else													
		geardamage = (adptr->LEFTWHEEL + adptr->RIGHTWHEEL + adptr->FRONTWHEEL + adptr->BACKWHEEL)/4;//RJS 07Sep98
														
	if (geardamage < BS_DAMLV2)									//RJS 07Sep98
	{
//	_DPlay.NewCollision(NULL,NULL,PIDC_ACTION,ACTION_GEAR);

		if (_Replay.Record || _DPlay.Implemented)
		{
			if (ControlledAC==Persons2::PlayerSeenAC)				  //AMM 23/06/99
				_DPlay.NewGearThingy(val);
		}

		if (val < ANIM_GEAR_DOWN)
		{
			if(ControlledAC == Manual_Pilot.ControlledAC2)
			{
				if (EngineList->Type == ET_PISTON)
					_Miles.PlayOnce(FIL_SFX_GEAR_P51,ControlledAC);		
				else
					_Miles.PlayOnce(FIL_SFX_GEAR_JET,ControlledAC);		
			}

			if (adptr->animtoggle)									
				adptr->animtoggle = -adptr->animtoggle;				
			else													
				adptr->animtoggle = 1;								
		}
		else
		{
			if (val == ANIM_GEAR_DOWN)
				adptr->animtoggle = -1;
			else
				adptr->animtoggle = 1;

			adptr->acleglowerf = val;
			adptr->acleglowerl = val;
			adptr->acleglowerr = val;
			adptr->acleglowerb = val;
		}
	}



}

//------------------------------------------------------------------------------
//Procedure		SetInstruments
//Author		Andrew McRae
//Date			Wed 27 Aug 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void INSTRUMENTS::SetInstruments (AirStrucPtr const ControlledAC,PMODEL pModel)
{
	SLong				scaled, scaled2, TotalEngineDamage;
	SLong				animstep;
	ActualSpeed = pModel->Speed;		// m/s	//CSB 23/06/99	

	TrueAirSpeed = -1.9426 * pModel->InstAirVel.z;	// knotts
	if (TrueAirSpeed < 0) TrueAirSpeed = 0;
	IndicatedAirSpeed = FSqrt (pModel->AmbDensity / 0.0001225) * TrueAirSpeed; //knotts//RJS 16Feb99
	ControlledAC->fly.i_a_s = (SWord)(IndicatedAirSpeed + 0.5);

	SLong TempGroundAlt = 0;
	if(ControlledAC->ai.homebase)
		TempGroundAlt = ControlledAC->ai.homebase->World.Y;

	Altitude = 0.032808 * (pModel->Pos.y - TempGroundAlt);	// feet

	RateOfClimb = 196.85 * pModel->Vel.y;	// feet per minute
	MODLIMIT(RateOfClimb, 6000);

	BankAngle = Rads2Rowan(pModel->fRoll) * 0.00549199;						//RJS 21Apr98
	PitchAngle = Rads2Degs (pModel->fPitch);
	HdgAngle = Rads2Degs (pModel->fHdg);

	NormalAcc = pModel->Acc.y / GRAVITY + pModel->Ori.y.y;

	Voltage = 28500;
	if(pModel->EngineList)
	{
		if (pModel->EngineList->Type == ET_PISTON)
		{
			Rpm1 = pModel->EngineList->Speed * 954.93;
			Power = 0;
		}

		if (pModel->EngineList->Type == ET_JET)
		{
			Rpm1 = pModel->EngineList->SpeedRpm;
			Power = (1000 * pModel->EngineList->Thrust) / pModel->EngineList->MaxThrust;

			FP EGT = ControlledAC->fly.thrustpercent -			//CSB 01/04/99
				(100 * pModel->EngineList->Thrust) / pModel->EngineList->MaxThrust;
			EGT = 190 + (65 * EGT) / 75;
			if(EGT < 0)   EGT = 0;
			if(EGT > 255) EGT = 255;
			ExhaustTemp = EGT;
			if(pModel->EngineList->EngineOut)
				ExhaustTemp = 0;

			FP Rpm0 = pModel->EngineList->Rpm100;
			FP RpmPercent = (100 * Rpm1) / Rpm0;
			if(RpmPercent < 23) Voltage = 24000;
			else if(RpmPercent > 45) Voltage = 28500;
			else 
			{
				FP fVoltage = 24000 + (28500 - 24000) * (RpmPercent - 23) / (45 - 23);
				Voltage = fVoltage;
			}
		}
	}

	// Calc slip,turn, ambient temp here...

	TempFreeAir = pModel->AmbTemp - 273.15;

//	FuelContentInt = ControlledAC->fly.mainfuel / 3967;
	SLong	fueltot = 0;					//Maybe only do this every N frames	//CSB 15/09/98
//	if (ControlledAC->weap.UseIntFuel)
		fueltot += ControlledAC->weap.left.int_fuel;
		fueltot += ControlledAC->weap.centre.int_fuel;
		fueltot += ControlledAC->weap.right.int_fuel;
//	else

	//cg * 2.205 / 100000 -> lbs
	FuelContentInt = fueltot / 45360;

	fueltot += ControlledAC->weap.left.ext_fuel;
	fueltot += ControlledAC->weap.centre.ext_fuel;
	fueltot += ControlledAC->weap.right.ext_fuel;

	FuelContentTot = fueltot / 45360;
	Turn = -pModel->RotVel.y * 1043038;	//Rowans / sec
	Turn = (LastHdgAngle - HdgAngle) * 100 / pModel->MODEL_DT;
	MODLIMIT(Turn, 3641);//0.349 * 1043038);


	if(pModel->AirVel.z != 0) Slip = -pModel->AirVel.x / pModel->AirVel.z * 10430;
	else Slip = 0.5 * 10430;
	MODLIMIT(Slip, 0.5 * 10430);
	FP cos = FCos(pModel->fRoll);
	FP sin = FSin(pModel->fRoll);
	pitchrate = -pModel->RotVel.y * sin + pModel->RotVel.x * cos;	//rdh
	Turn2 = (HdgAngle - LastHdgAngle) * 100 / pModel->MODEL_DT;
	LastHdgAngle = HdgAngle;

	ULong	tp = ControlledAC->fly.thrustpercent;
	tp *= ControlledAC->fly.thrustpercent;
	tp *= ControlledAC->fly.thrustpercent;		  //max 1 million
	tp /= 4500;

	ManifoldPressure = tp;		//0 - 61   (0-255)

	ControlledAC->fly.rpm = SWord(Rpm1);

	CarburettorAirTemp = (60*255)/100;							//RJS 30Mar99
	SuctionGuage = (40*255)/100;								//RJS 30Mar99

	// -10 to 1 degrees
	HorizStab = ControlledAC->fly.elevator;						//RJS 31Mar99
	if (HorizStab > 0)	HorizStab /= 10;					//RJS 31Mar99
	HorizStab += 32767;											//RJS 31Mar99



}

//------------------------------------------------------------------------------
//Procedure		EngineDamage
//Author		Robert Slater
//Date			Fri 13 Feb 1998
//
//Description	
//
//Inputs		Animation Data ptr,  number of engines
//
//Returns		Total engine damage 0 - 255 (DEAD)
//
//------------------------------------------------------------------------------
SLong	INSTRUMENTS::EngineDamage(UByteP	animptr, int  noengines)
{
	PolyPitAnimData*	adptr = (PolyPitAnimData*) animptr;
	SLong				total;

	total = adptr->ENGINELEFTOUT;
	total += adptr->ENGINELEFTIN;
	total += adptr->ENGINERIGHTOUT;
	total += adptr->ENGINERIGHTIN;

	total /= noengines;

	return(total);
}

//------------------------------------------------------------------------------
//Procedure		SetTimedAnim
//Author		Robert Slater
//Date			Fri 13 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
SLong INSTRUMENTS::SetTimedAnim (SLong	&SrcVal, UWord  &TimerCount, SLong Desired, UWord timeup, UWord timedown)
{
	SLong	scaled;
	SLong	gap;

	gap = Desired - SrcVal;
	if (gap)
	{
		if (gap > 0)
		{
			scaled = (TimerCount * gap) / timeup;
			if (scaled > gap)
			{
				scaled = 0;
				SrcVal = Desired;
			}
		}
		else
		{
			scaled = (TimerCount * gap) / timedown;
			if (scaled < gap)
			{
				scaled = 0;
				SrcVal = Desired;
			}
		}
	}
	else
	{
		TimerCount = 0;
		scaled = 0;
		SrcVal = Desired;
	}

	return(scaled);
}

//------------------------------------------------------------------------------
//Procedure		SetCockpitAnims
//Author		Robert Slater
//Date			Mon 16 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::SetCockpitAnims(ItemPtr	itm, UByteP	animptr)
{
	PolyPitAnimData*	adptr = (PolyPitAnimData*) animptr;
	SLong				scaled, scaled2, TotalEngineDamage, animstep;
	SLong				index;

	for (index = 0; index < DialCnt; index++)
		DialList[index].SetDialValue(itm, animptr,(UByteP*)DialSrcList);


	// Faked dials....

	TotalEngineDamage = EngineDamage((UByteP)adptr,1);   // 1 is number of engines

	if (adptr->acvoltage < 64)									//RJS 01May98
	{
		adptr->acpitch = 255;
		adptr->achydraulicpress = 0;
		adptr->acfuelpress = 0;
		adptr->accabinpress = 0;
		adptr->acoxygenpress = 0;
		adptr->acoilpress = 0;
		adptr->actempgas = 0;
	}
	else
	{
		scaled = adptr->LEFTWINGIN;

		if (adptr->LEFTWINGOUT > scaled)
			scaled = adptr->LEFTWINGOUT;

		if (adptr->RIGHTWINGIN > scaled)
			scaled = adptr->RIGHTWINGIN;

		if (adptr->RIGHTWINGOUT > scaled)
			scaled = adptr->RIGHTWINGOUT;

		if (adptr->FIN > scaled)
			scaled = adptr->FIN;

		if (adptr->TAIL > scaled)
			scaled = adptr->TAIL;

		if (adptr->OTHER > scaled)								//RJS 07Sep98
			scaled = adptr->OTHER;								//RJS 07Sep98

		if (adptr->OTHER == BS_DEAD)							//RJS 07Sep98
			adptr->achydraulicpress = 0;						//RJS 07Sep98
		else
		{
			scaled2 = (	adptr->LEFTWINGIN + adptr->LEFTWINGOUT +
						adptr->RIGHTWINGIN + adptr->RIGHTWINGOUT +
						adptr->FIN + adptr->TAIL + adptr->OTHER	) / 7;

			scaled = (scaled * scaled2)/BS_DEAD;
			adptr->achydraulicpress = BS_DEAD - scaled;
		}														//RJS 07Sep98

		scaled2 = 200 - ((Altitude * 5)/5000);
		scaled = (scaled2 * ((256 - adptr->CANOPY1)>>8))<<8;
		adptr->accabinpress = scaled / 400;

		adptr->acoxygenpress = adptr->accabinpress;				//RJS 01MaY98

		adptr->acoilpress = (75*(255-adptr->ENGINELEFTIN))/100;	//RJS 30Mar99
		adptr->acfuelpress = (55*(255-adptr->ENGINELEFTIN))/100;	//RJS 30Mar99
		adptr->actempgas = ((60*255)/100)+((40*adptr->ENGINELEFTIN)/100);//RJS 30Mar99
	}


	// Manifold pressure is time-based...
	//
	//	ManifoldPressure								
	//	SuctionGauge									



	// Now scale to timer...
	for (index = 0; index < TimedDialCnt; index++)				//RJS 29Jun98
		TimedDialList[index].SetTimedAnim(animptr);				//RJS 29Jun98
}

//------------------------------------------------------------------------------
//Procedure		UpdateCockpitTimers
//Author		Robert Slater
//Date			Mon 16 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::UpdateCockpitTimers(SLong	theframetime)
{
	int	index;
	for (index=0; index < TimedDialCnt; index++)
		TimedDialList[index].theTimer += theframetime;
}

//------------------------------------------------------------------------------
//Procedure		SetDialValue
//Author		Robert Slater
//Date			Mon 16 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DialSpec::SetDialValue(ItemPtr	itm, UByteP	adptr,UByteP	*SrcListPtr)
{
	UByteP	SrcValPtr = NULL;
	SLong	theValue;

	switch (btype)
	{
	case SBASE_DEFAULT:
		SrcValPtr = SrcListPtr[SrcValStructIndex];
		SrcValPtr += SrcValOffset;
		 break;
	case SBASE_AC:
		SrcValPtr = (UByteP)itm;
		SrcValPtr += SrcValOffset;
		break;
	case SBASE_ANIM:
		SrcValPtr = (UByteP)&itm->Anim[0];
		SrcValPtr += SrcValOffset;
		break;
	}

	adptr += DestValOffset;

	if (useint)
	{
		switch (shiftsrc)
		{
		case 8:
			theValue = *SrcValPtr;
			break;
		case 15:
			theValue = *((SWord*)SrcValPtr);
			break;
		case 31:
			theValue = *((SLong*)SrcValPtr);
			break;
		}

		if (theValue < minIntval)
			theValue = minIntval;

		theValue -= minIntval;

		if (theValue >= maxIntval)
			theValue = maxIntval-1;

		theValue <<= shiftdest;
		theValue /= Intvalscale;
	}
	else
	{
		FP theValue2 = *((FP*)SrcValPtr);

		if (theValue2 < minval)
			theValue2 = minval;

		theValue2 -= minval;

		if (theValue2 > maxval)
			theValue2 = maxval;

		theValue2 *= ValScale;
		theValue = theValue2;
	}

	if (shiftdest == 8)
		*adptr = theValue;
	else
		*((SWord*)adptr) = theValue;
}

//------------------------------------------------------------------------------
//Procedure		NewDialValue
//Author		Robert Slater
//Date			Mon 16 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::NewDialValue(	void*	animptr,
									UByteP	animdestptr,
									void*	vstrucptr,
									UByteP	srcptr,
									int		minval,
									int		maxval,
									int		scaler,
									UByte	shiftsrc,
									UByte	shiftdest,
									UWord	restrot		)
{
	if (DialCnt < MaxAutoDial)									//RJS 29Jun98
	{
		int	i;
		DialSpec	*DPtr = &DialList[DialCnt];
		UByteP		adptr = (UByteP) animptr;
		UByteP		strucptr = (UByteP) vstrucptr;

		DPtr->useint = 1;
		DPtr->DestValOffset = animdestptr - adptr;
		DPtr->btype = SBASE_DEFAULT;

		if (vstrucptr != (void*)this)
		{
			if (vstrucptr == (void*)Manual_Pilot.ControlledAC2)
				DPtr->btype = SBASE_AC;
			else
			{
				if (vstrucptr == (void*)&Manual_Pilot.ControlledAC2->Anim[0])
					DPtr->btype = SBASE_ANIM;
			}
		}

		if (DPtr->btype == SBASE_DEFAULT)
		{
			for (i=0; i < DialSrcCnt; i++)
			{
				if (DialSrcList[i] == strucptr)
					break;
			}

			if (i == DialSrcCnt)
			{
				if (DialSrcCnt < MaxDialSrc)
					DialSrcList[DialSrcCnt++] = strucptr;
			}

			DPtr->SrcValStructIndex = i;
		}
		else
			DPtr->SrcValStructIndex = 0;

		DPtr->SrcValOffset = srcptr - strucptr;
		DPtr->minIntval = minval;
		DPtr->maxIntval = maxval - minval;
		if (scaler == 0)										//RJS 24Apr98
			DPtr->Intvalscale = DPtr->maxIntval;				//RJS 24Apr98
		else													//RJS 24Apr98
			DPtr->Intvalscale = scaler;							//RJS 24Apr98

		DPtr->shiftsrc = shiftsrc;
		DPtr->shiftdest = shiftdest;
		DPtr->torestval = restrot;

		if (	(DPtr->DestValOffset < 0)
			||	(DPtr->SrcValOffset < 0)	)
		{
			_Error.EmitSysErr(__FILE__":Dial Offset Wrong!");
		}

		DialCnt++;
	}
}

//------------------------------------------------------------------------------
//Procedure		NewDialValue
//Author		Robert Slater
//Date			Mon 16 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::NewDialValue(	void*	animptr,
									UByteP	animdestptr,
									void*	vstrucptr,
									UByteP	srcptr,
									FP		minval,
									FP		maxval,
									FP		scaler,
									UByte	shiftdest,
									UWord	restrot		)
{
	if (DialCnt < MaxAutoDial)									//RJS 29Jun98
	{
		int	i;
		DialSpec	*DPtr = &DialList[DialCnt];
		UByteP		adptr = (UByteP) animptr;
		UByteP		strucptr = (UByteP) vstrucptr;

		DPtr->useint = 0;
		DPtr->DestValOffset = animdestptr - adptr;
		DPtr->btype = SBASE_DEFAULT;

		if (vstrucptr != (void*)this)
		{
			if (vstrucptr == (void*)Manual_Pilot.ControlledAC2)
				DPtr->btype = SBASE_AC;
			else
			{
				if (vstrucptr == (void*)&Manual_Pilot.ControlledAC2->Anim[0])
					DPtr->btype = SBASE_ANIM;
			}
		}

		if (DPtr->btype == SBASE_DEFAULT)
		{
			for (i=0; i < DialSrcCnt; i++)
			{
				if (DialSrcList[i] == strucptr)
					break;
			}

			if (i == DialSrcCnt)
			{
				if (DialSrcCnt < MaxDialSrc)
					DialSrcList[DialSrcCnt++] = strucptr;
			}

			DPtr->SrcValStructIndex = i;
		}
		else
			DPtr->SrcValStructIndex = 0;

		DPtr->SrcValOffset = srcptr - strucptr;
		DPtr->minval = minval;
		DPtr->maxval = maxval - minval;
		if (scaler == 0)										//RJS 24Apr98
		{
			switch (shiftdest)									//RJS 24Apr98
			{													//RJS 24Apr98
			case 8:												//RJS 24Apr98
				DPtr->ValScale = 255.0 / DPtr->maxval;			//RJS 24Apr98
				break;											//RJS 24Apr98
			case 15:											//RJS 24Apr98
				DPtr->ValScale = 65535.0 / DPtr->maxval;		//RJS 24Apr98
				break;											//RJS 24Apr98
			}
		}														//RJS 24Apr98
		else													//RJS 24Apr98
			DPtr->ValScale = scaler;							//RJS 24Apr98

		DPtr->shiftdest = shiftdest;
		DPtr->torestval = restrot;

		if (	(DPtr->DestValOffset < 0)
			||	(DPtr->SrcValOffset < 0)	)
		{
			_Error.EmitSysErr(__FILE__":Dial Offset Wrong!");
		}

		DialCnt++;
	}
}

//------------------------------------------------------------------------------
//Procedure		AddAutoDial
//Author		Robert Slater
//Date			Wed 18 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::AddAutoDial(	void*	animptr,
									UByteP	animdestptr,
									void*	strucptr,
									FP*		srcptr,
									FP		minval,
									FP		maxval,
									FP		scaler	)
{
	NewDialValue(animptr,animdestptr,strucptr,(UByteP)srcptr,minval,maxval,scaler,8);
}

//------------------------------------------------------------------------------
//Procedure		AddAutoDial
//Author		Robert Slater
//Date			Wed 18 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::AddAutoDial(	void*	animptr,
									SWordP	animdestptr,
									void*	strucptr,
									FP*		srcptr,
									FP		minval,
									FP		maxval,
									FP		scaler	)
{
	NewDialValue(animptr,(UByteP)animdestptr,strucptr,(UByteP)srcptr,minval,maxval,scaler,15);
}

//------------------------------------------------------------------------------
//Procedure		AddAutoDial
//Author		Robert Slater
//Date			Wed 18 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::AddAutoDial(	void*	animptr,
									UByteP	animdestptr,
									void*	strucptr,
									SWordP	srcptr,
									int		minval,
									int		maxval,
									int		scaler	)
{
	NewDialValue(animptr,animdestptr,strucptr,(UByteP)srcptr,minval,maxval,scaler,15,8);
}

//------------------------------------------------------------------------------
//Procedure		AddAutoDial
//Author		Robert Slater
//Date			Wed 18 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::AddAutoDial(	void*	animptr,
									SWordP	animdestptr,
									void*	strucptr,
									SWordP	srcptr,
									int		minval,
									int		maxval,
									int		scaler	)
{
	NewDialValue(animptr,(UByteP)animdestptr,strucptr,(UByteP)srcptr,minval,maxval,scaler,15,15);
}

//------------------------------------------------------------------------------
//Procedure		AddAutoDial
//Author		Robert Slater
//Date			Wed 18 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::AddAutoDial(	void*	animptr,
									UByteP	animdestptr,
									void*	strucptr,
									ULongP	srcptr,
									int		minval,
									int		maxval,
									int		scaler	)
{
	NewDialValue(animptr,animdestptr,strucptr,(UByteP)srcptr,minval,maxval,scaler,31,8);
}

//------------------------------------------------------------------------------
//Procedure		AddAutoDial
//Author		Robert Slater
//Date			Wed 18 Feb 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::AddAutoDial(	void*	animptr,
									SWordP	animdestptr,
									void*	strucptr,
									ULongP	srcptr,
									int		minval,
									int		maxval,
									int		scaler	)
{
	NewDialValue(animptr,(UByteP)animdestptr,strucptr,(UByteP)srcptr,minval,maxval,scaler,31,15);
}

//------------------------------------------------------------------------------
//Procedure		AddAutoDial
//Author		Robert Slater
//Date			Mon 30 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::AddAutoDial(	void*	animptr,
									UByteP	animdestptr,
									void*	strucptr,
									SLongP	srcptr,
									int		minval,
									int		maxval,
									int		scaler	)
{
	NewDialValue(animptr,(UByteP)animdestptr,strucptr,(UByteP)srcptr,minval,maxval,scaler,31,8);
}

//------------------------------------------------------------------------------
//Procedure		AddAutoDial
//Author		Robert Slater
//Date			Mon 30 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::AddAutoDial(	void*	animptr,
									SWordP	animdestptr,
									void*	strucptr,
									SLongP	srcptr,
									int		minval,
									int		maxval,
									int		scaler	)
{
	NewDialValue(animptr,(UByteP)animdestptr,strucptr,(UByteP)srcptr,minval,maxval,scaler,31,15);
}

//------------------------------------------------------------------------------
//Procedure		AddTimedDial
//Author		Robert Slater
//Date			Mon 29 Jun 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::AddTimedDial(	void*	adptr,
									UByteP	animdp,
									int		timeUp,
									int		timeDown,
									Bool	signedSrc	)
{
	int	doffset = ((UByteP)animdp) - ((UByteP)adptr);

	NewTimedDial(doffset,timeUp,timeDown,signedSrc,FALSE,8);
}

void	INSTRUMENTS::AddTimedDial(	void*	adptr,
									SByteP	animdp,
									int		timeUp,
									int		timeDown,
									Bool	signedSrc	)
{
	int	doffset = ((UByteP)animdp) - ((UByteP)adptr);

	NewTimedDial(doffset,timeUp,timeDown,signedSrc,TRUE,8);
}

void	INSTRUMENTS::AddTimedDial(	void*	adptr,
									UWordP	animdp,
									int		timeUp,
									int		timeDown,
									Bool	signedSrc	)
{
	int	doffset = ((UByteP)animdp) - ((UByteP)adptr);

	NewTimedDial(doffset,timeUp,timeDown,signedSrc,FALSE,16);
}

void	INSTRUMENTS::AddTimedDial(	void*	adptr,
									SWordP	animdp,
									int		timeUp,
									int		timeDown,
									Bool	signedSrc	)
{
	int	doffset = ((UByteP)animdp) - ((UByteP)adptr);

	NewTimedDial(doffset,timeUp,timeDown,signedSrc,TRUE,16);
}

void	INSTRUMENTS::AddTimedDial(	void*	adptr,
									ULongP	animdp,
									int		timeUp,
									int		timeDown,
									Bool	signedSrc	)
{
	int	doffset = ((UByteP)animdp) - ((UByteP)adptr);

	NewTimedDial(doffset,timeUp,timeDown,signedSrc,FALSE,32);
}

void	INSTRUMENTS::AddTimedDial(	void*	adptr,
									SLongP	animdp,
									int		timeUp,
									int		timeDown,
									Bool	signedSrc	)
{
	int	doffset = ((UByteP)animdp) - ((UByteP)adptr);

	NewTimedDial(doffset,timeUp,timeDown,signedSrc,TRUE,32);
}

//------------------------------------------------------------------------------
//Procedure		NewTimedDial
//Author		Robert Slater
//Date			Mon 29 Jun 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	INSTRUMENTS::NewTimedDial(	int		doffset,
									int		tup,
									int		tdown,
									Bool	signedSrc,
									Bool	signedDest,
									UByte	nobits	)
{
	if (TimedDialCnt < MaxAutoDial)
	{
		TimedDialList[TimedDialCnt].OldVal = 0;
		TimedDialList[TimedDialCnt].OldGap = 0;
		TimedDialList[TimedDialCnt].theTimer = 0;
		TimedDialList[TimedDialCnt].SrcVal = 0;
		TimedDialList[TimedDialCnt].destOffset = doffset;
		TimedDialList[TimedDialCnt].timeUp = tup;
		TimedDialList[TimedDialCnt].timeDown = tdown;
//		TimedDialList[TimedDialCnt].signedSrc = signedSrc;
		TimedDialList[TimedDialCnt].signedDest = signedDest;
		TimedDialList[TimedDialCnt++].nobitsDest = nobits;
	}
}

//------------------------------------------------------------------------------
//Procedure		SetTimedAnim
//Author		Robert Slater
//Date			Mon 29 Jun 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	TimedDialSpec::SetTimedAnim(void*	adptr)
{
	SLong	scaled=0;
	SLong	gap;
	SLong	Desired = 0;
	UByteP	animp = ((UByteP)adptr) + destOffset;
	SLong	dist = 255;
	SLong	ROld;

	if (!signedDest)
	{
		switch (nobitsDest)
		{
		case 8:
			Desired = *animp;
			break;
		case 16:
			Desired = *((UWordP)animp);
			dist = 65535;
			break;
		}
	}
	else
	{
		switch (nobitsDest)
		{
		case 8:
			Desired = *((SByteP)animp);
			break;
		case 16:
			Desired = *((SWordP)animp);
			dist = 65535;
			break;
		}
	}

	ROld = Desired - OldVal;
	gap = Desired - SrcVal;

	if (gap)
	{
		if (gap > 0)
		{
			if (ROld <= 0)
			{
				theTimer = 0;
				SrcVal = OldVal;
			}
			else
			{
				scaled = (theTimer * dist) / timeUp;
				if (scaled > gap)
				{
					scaled = 0;
					SrcVal = Desired;
				}
			}
		}
		else
		{
			if (ROld >= 0)
			{
				theTimer = 0;
				SrcVal = OldVal;
			}
			else
			{
				scaled = (theTimer * -dist) / timeDown;
				if (scaled < gap)
				{
					scaled = 0;
					SrcVal = Desired;
				}
			}
		}

		scaled += SrcVal;
		switch (nobitsDest)
		{
		case 8:
			*((UByteP)animp) = scaled;
			break;
		case 16:
			*((UWordP)animp) = scaled;
			break;
		case 32:
			*((ULongP)animp) = scaled;
			break;
		}

		OldVal = scaled;
		OldGap = gap;
	}
	else
	{
		theTimer = 0;
		if (ROld)
		{
			SrcVal = OldVal;
			OldGap = -OldGap;
		}
		else
		{
			SrcVal = Desired;
			OldVal = SrcVal;
			OldGap = 0;
		}
	}
}



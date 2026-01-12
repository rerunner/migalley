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

#include "_COMM.H"

//------------------------------------------------------------------------------
//Filename       replay.cpp
//System         
//Author         Andy McMaster
//Date           Wed 29 Apr 1998
//Description    
//------------------------------------------------------------------------------

#include	"DOSDEFS.H"
#define F_GRAFIX												//DAW 05Aug96
#define F_BATTLE
#define F_COMMON
#undef  F_SOUNDS
//RERUN #include	"files.g"
#include	"FILES.H"
//RERUN #include	<stdlib.h>
#include	"REPLAY.H"
#include	"MYERROR.H"
#include	"MISSMAN2.H"
#include	"PLANETYP.H"
#include	"VIEWSEL.H"
#include	"PERSONS2.H"
#include	"WINMOVE.H"
#include	"FLYMODEL.H"
#include	"AI.H"
//RERUN #include	"windows.h"
#include	"WIN32_COMPAT.H"


#include	"WORLDINC.H"
#include	"MYANGLES.H"
#include	"3DCOM.H"
#include	"MILES.H"
#include	"WORLD.H"
#include	"TRANSITE.H"
#include	"MODVEC.H"
#include	"MODEL.H"
#include	"SHAPES.H"
#include	"3DCODE.H"
#include	"STUB3D.H"
#include	"TRANSITE.H"
//#include	"direct.h"
#include <unistd.h>
#include <filesystem> //C++17 and up
#include	"AREATYPE.H"
#include	"mfc/RESOURCE.H" // RERUN
#include	"SAVEGAME.H"
#include	"AAA.H"

//#define VELCHECK
//#define REPLAYFILE
//#define HEADERLENGTH 4376
//#define MAXHEADERSIZE 10000;
//#define	FRAMESINBLOCK 1000
//#define	FRAMESINBLOCK 1000
// for fuel in setflightparam purposes need multiple of 256
#define	FRAMESINBLOCK 1024
extern int BAD_RV; // RERUN value located in GLOBREFS.CPP. 

Replay	_Replay; // RERUN added

//------------------------------------------------------------------------------
//Procedure		OpenRecordLog
//Author		Andy McMaster
//Date			Wed 13 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::OpenRecordLog()
{
	DWORD err;

	Record=TRUE;	
	GetReplayFileName();
	Record=FALSE;

	recordstorage=CreateFile(
		&rfilename[0],
			GENERIC_WRITE|GENERIC_READ,
			FILE_SHARE_WRITE|FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,	// add to any file that is there
			FILE_ATTRIBUTE_TEMPORARY,
			NULL);

// instead of deleting file, just truncate to zero 

	if (ResetFileFlag)
	{
		ResetFileFlag=false;
		SetEndOfFile(recordstorage);
	}

	if (recordstorage!=INVALID_HANDLE_VALUE)
	{
		SetFilePointer(recordstorage, 0, NULL, FILE_END); // get to end of file

		Record=TRUE;
		return TRUE;
	}

	err=GetLastError();

	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		CloseRecordLog
//Author		Andy McMaster
//Date			Wed 13 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::CloseRecordLog()
{
	DWORD err;

	if (recordstorage)
	{
		if (!CloseHandle(recordstorage))
			err=GetLastError();
	}

	recordstorage=NULL;

	Record=FALSE;
}

//------------------------------------------------------------------------------
//Procedure		OpenPlaybackLog
//Author		Andy McMaster
//Date			Wed 29 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::OpenPlaybackLog()
{
#if 0 //RERUN
	DWORD err;

	PlaybackPaused=TRUE;

	if (!RestorePosition)
	{
		Playback=TRUE;
		GetReplayFileName();
		Playback=FALSE;
	}

	playbackstorage=CreateFile(
		&pfilename[0],
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_TEMPORARY,
		NULL);

	if (playbackstorage!=INVALID_HANDLE_VALUE)
	{
		mappedplaybackfile=CreateFileMapping(
			playbackstorage,
			NULL,
			PAGE_READONLY,
			0,
			0,
			NULL);

		if (mappedplaybackfile)
		{

			playbackfilestart=MapViewOfFile(
				mappedplaybackfile,
				FILE_MAP_READ,
				0,
				0,
				0);

			if (playbackfilestart)
			{
				playbackfilepos=(UByteP)playbackfilestart;
				playbackfileend=playbackfilepos+GetFileSize(playbackstorage,NULL);

				if (!RestorePosition)
				{
					endmark.memaddress=(UByteP)playbackfileend;
				}
				Playback=TRUE;
			}
		}
	}

	return Playback;
#else
	return BOOL_TRUE;
#endif
}

//------------------------------------------------------------------------------
//Procedure		CloseReplayLog
//Author		Andy McMaster
//Date			Wed 29 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::ClosePlaybackLog()
{
#if 0 //RERUN
	DWORD err;

	if (playbackfilestart)										//DAW 29Sep99
		if (!UnmapViewOfFile(playbackfilestart))				//DAW 29Sep99
			err=GetLastError();									//DAW 29Sep99

	if (mappedplaybackfile)										//DAW 29Sep99
		if (!CloseHandle(mappedplaybackfile))					//DAW 29Sep99
			err=GetLastError();									//DAW 29Sep99

	if (playbackstorage)
		if (!CloseHandle(playbackstorage))
			err=GetLastError();

	playbackfilestart=NULL;										//DAW 29Sep99
	playbackfilepos=NULL;										//DAW 29Sep99
	playbackfileend=NULL;										//DAW 29Sep99

	playbackstorage=NULL;
	mappedplaybackfile=NULL;

	Playback=FALSE;
	PlaybackPaused=FALSE;
#endif
}

//------------------------------------------------------------------------------
//Procedure		StoreDeltas
//Author		Andy McMaster
//Date			Wed 29 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreDeltas()
{
	SLong	rval;
	UByte	shiftfactor;
	AirStrucPtr gac=Persons2::PlayerGhostAC;

	REPLAYPACKET output;

	_DPlay.MakeDeltaPacket(&output);

// apply deltas 

	shiftfactor=output.Shift;

	rval=_DPlay.GetVal(output.X,shiftfactor);
	_DPlay.Deltas.lastpos.X-=rval;
	gac->World.X+=rval;

	rval=_DPlay.GetVal(output.Y,shiftfactor);
	_DPlay.Deltas.lastpos.Y-=rval;
	gac->World.Y+=rval;

	rval=_DPlay.GetVal(output.Z,shiftfactor);
	_DPlay.Deltas.lastpos.Z-=rval;
	gac->World.Z+=rval;

	rval=_DPlay.GetVal(output.Heading,shiftfactor);
	_DPlay.Deltas.Heading.a-=rval;
	gac->hdg.a+=rval;

	gac->fly.dhdg=rval; ///_DPlay.RateDivider;					//RDH 30Nov98

	rval=_DPlay.GetVal(output.Pitch,shiftfactor);
	_DPlay.Deltas.Pitch.a-=rval;
	gac->pitch.a+=rval;
	gac->fly.cpitch=gac->pitch;									//AMM 29Jun99
//	gac->fly.cpitch=gac->pitch;

	rval=_DPlay.GetVal(output.Roll,shiftfactor);
	_DPlay.Deltas.Roll.a-=rval;
	gac->roll.a+=rval;

	if (output.Velocity&8)
	{
		rval=-((output.Velocity&7)<<shiftfactor);
	}
	else
	{
		rval=((output.Velocity&7)<<shiftfactor);
	}
	_DPlay.Deltas.Velocity-=rval;
	gac->vel+=rval;

	_Replay.UpdatePosBuffer();									//AMM 19Mar99
	_DPlay.CalcVels();											//AMM 19Mar99

// check for collision etc

	_DPlay.MakeExtraPacket(&output,TRUE);

	if (!ReplayWrite((UByte*)&output,sizeof(REPLAYPACKET)))
		return FALSE;

#ifdef DEBUGMOVE2
	FILE*fp=fopen("record.txt","at");
	fprintf(fp,"%d: %d\n",replayframecount,output.IDCode);
	fclose(fp);
#endif

	replayframecount++;

	if (replayframecount==FRAMESINBLOCK)
	{
		if (!StoreRealFrameCounts(replayframecount,0,FRAMESINBLOCK-1))
			return FALSE;

//		if (!StoreNewHeader())
		StoreHeader=true;										//AMM 29Jun99
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadDeltas
//Author		Andy McMaster
//Date			Fri 15 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadDeltas()
{
	REPLAYPACKET input;
	SLong	val;
	AirStrucPtr sac=Persons2::PlayerSeenAC;
	AirStrucPtr gac=Persons2::PlayerGhostAC;
	UByte	RecCode;

	if (emptyblock)
	{
		UByteP temp=playbackfilepos;

		DoSmokeTrailStuff=FALSE;
		if (!LoadBlockHeader())
		{
			DoSmokeTrailStuff=TRUE;
			playbackfilepos=temp;
			return FALSE;
		}
		DoSmokeTrailStuff=TRUE;
	}

	if (replayframecount>thisblockendframe)
	{
// dont want to load any frames that are after end marker in end block

		PlaybackPaused=TRUE;
		return FALSE;
	}

	if (!ReplayRead((UByte*)&input,sizeof(REPLAYPACKET)))
		return FALSE;

	RecCode=input.IDCode>>6;

	val=_DPlay.GetVal(input.X,input.Shift);
	gac->World.X+=val;

	val=_DPlay.GetVal(input.Y,input.Shift);
	gac->World.Y+=val;

	val=_DPlay.GetVal(input.Z,input.Shift);
	gac->World.Z+=val;

	val=_DPlay.GetVal(input.Heading,input.Shift);
	gac->hdg.a+=val;

	gac->fly.dhdg=val; ///_DPlay.RateDivider;					//RDH 30Nov98

	val=_DPlay.GetVal(input.Pitch,input.Shift);
	gac->pitch.a+=val;

	val=_DPlay.GetVal(input.Roll,input.Shift);
	gac->roll.a+=val;

	gac->fly.cpitch=gac->pitch;									//AMM 29Jun99
//	ac->fly.cpitch=ac->pitch;

	if(input.Velocity&8)
	{
		val=-((input.Velocity&7)<<input.Shift);
	}
	else
	{
		val=+((input.Velocity&7)<<input.Shift);
	}
	gac->vel+=val;

	//Set to allow contrails to appear...
	if (sac->fly.pModel)				//RJS 26Mar99
	{
		//This is so CalcLauncherVel will work...
		sac->fly.pModel->Pos.y=gac->World.Y;			
		sac->fly.pModel->replay_pitch = gac->pitch.a;	
		sac->fly.pModel->replay_hdg = gac->hdg.a;		
		sac->fly.pModel->replay_roll = gac->roll.a;		
		if (gac->World.Y < HIGHESTGROUND)						//RJS 29Jun99
			sac->fly.pModel->GroundHeight = Land_Scape.GetGroundLevel(gac);
		else
			sac->fly.pModel->GroundHeight = 0;
	}

	_Replay.UpdatePosBuffer();									//AMM 19Mar99
	_DPlay.CalcVels();											//AMM 19Mar99

// update seenac vels from ghostac vels for rocket launches etc

	sac->velx=gac->velx;//*10;
	sac->vely=gac->vely;//*10;
	sac->velz=gac->velz;//*10;

	Persons2::PlayerSeenAC->uniqueID.bitfieldUid.changed = FALSE;					

	switch (RecCode)
	{
//		case PT_EFFECT:
//			_DPlay.ProcessKillLauncher((LPBASICPACKET)&input);
		case PT_UIDS:
			_DPlay.ProcessUIDSPacket((LPBASICPACKET)&input,0);
			break;

		case PT_BULLET:
			_DPlay.ProcessBulletPacket((LPBASICPACKET)&input,sac);//Persons2::PlayerGhostAC->uniqueID.bitfieldUid.count);
			break;

		case PT_SPECIAL:
			_DPlay.ProcessSpecialPacket((LPBASICPACKET)&input,Persons2::PlayerSeenAC,0);
			break;

		case PT_COLLISION:
			_DPlay.ProcessCollisionPacket((LPBASICPACKET)&input,0);
			break;

		default:
			_Error.EmitSysErr("Error: Illegal IDCode");
			break;
	}

	switch (ReplayDeathMode)									//RJS 28Oct98
	{
		case RDEATH_SKID:
			DummyCrashSkid();
			break;
		case RDEATH_ROLL:
			DummyCrashRoll();
			break;
		case RDEATH_DIVE:
			DummyCrashDive();
			break;
	}															//RJS 28Oct98

#ifdef DEBUGMOVE2
	FILE*fp=fopen("playback.txt","at");
	fprintf(fp,"%d: %d\n",replayframecount,input.IDCode);
	fclose(fp);
#endif

	replayframecount++;
	if (replayframecount>numframes[currblock-1]-1)
	{
		LoadHeader=true;
	}
	else
	{
		if (replayframecount>thisblockendframe)
		{
// only one block will have dud frames at end and that is end block, so just pause!

			PlaybackPaused=TRUE;
			return FALSE;
//			ReplayFlag=RF_SKIPFRAMES;			
//			framestoskip=(numframes[currblock-1]-thisblockendframe);
		}
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		DeleteReplayLog
//Author		Andy McMaster
//Date			Tue 5 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::DeleteReplayLog()
{
// seem to have problem deleting file, so set a flag saying truncate me to zero length instead

//	int ok;
//	DWORD err;

//	ok=DeleteFile(&filename[0]);
	ResetFileFlag=true;

//	if (!ok)
//	{
//		err=GetLastError();
//	}
}

//------------------------------------------------------------------------------
//Procedure		StoreRandomList
//Author		Andy McMaster
//Date			Mon 18 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreRandomList()
{
	UWord RndPacket [RNDPACKETSIZE],n;
	
	for (n=0; n<(RNDPACKETSIZE - 2); n++)
		RndPacket[n] = Math_Lib.GetRndLookUp(n);

	RndPacket[(RNDPACKETSIZE - 2)] = Math_Lib.Getbval();
	RndPacket[(RNDPACKETSIZE - 1)] = Math_Lib.Getcval();

	if (!ReplayWrite((UByte*)&RndPacket,RNDPACKETSIZE*sizeof(UWord)))
		return FALSE;

	Math_Lib.ResetRndCount();

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadRandomList
//Author		Andy McMaster
//Date			Mon 18 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadRandomList()
{
	UWord RndPacket [RNDPACKETSIZE],n;
	
	if (!ReplayRead((UByte*)&RndPacket,RNDPACKETSIZE*sizeof(UWord)))
		return FALSE;

	for (n=0; n<(RNDPACKETSIZE - 2); n++)
		Math_Lib.SetRndLookUp(n,RndPacket[n]);

	Math_Lib.Setbval(RndPacket[(RNDPACKETSIZE - 2)]);
	Math_Lib.Setcval(RndPacket[(RNDPACKETSIZE - 1)]);

	Math_Lib.ResetRndCount();

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadGlobalRefs
//Author		Andy McMaster
//Date			Tue 19 May 1998
//
//Description	Read in header bit for next 8 GR's in BFieldGlobalTable. If bit
//				in header is set then values following header will be for those
//				values, otherwise 0.
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadInitialGlobRefs()
{
// load Pack_PlayerSquad and Pack_PlayerPos

	if (!ReplayRead((UByte*)&PlayerSquad,sizeof(int)))
		return FALSE;

	if (!ReplayRead((UByte*)&PlayerPos,sizeof(int)))
		return FALSE;

	DeleteGRList(initgrlist);

	if (!LoadGlobRefList(initgrlist))
		return FALSE;

	SetGlobRefs(initgrlist);

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadGlobRefList
//Author		Andy McMaster
//Date			Tue 15 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadGlobRefList(LPREPGRENTRY& list)
{
	ULong val,m;
	UByte set;
	Bool	finished=FALSE;

	while (!finished)
	{
		if (!ReplayRead((UByte*)&set,sizeof(UByte)))
			return FALSE;

		for (m=0;m<8;m++)
		{
			LPREPGRENTRY temp=new REPGRENTRY;
			temp->next=NULL;

			if (set&0x1<<m)
			{
				if (!ReplayRead((UByte*)&val,sizeof(ULong)))
					return FALSE;

				temp->val=val;

				if (val==BAD_RV)
				{
					finished=TRUE;
				}
			}
			else
			{
				temp->val=0;
			}

			if (list)
			{
				LPREPGRENTRY temp2=list;

				while (temp2->next)
					temp2=temp2->next;

				temp2->next=temp;
			}
			else
			{
				list=temp;
			}
			temp=NULL;

			if (finished)
				break;
		}
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		BeginReplayRecord
//Author		Andy McMaster
//Date			Tue 26 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Procedure		StoreDeadItems
//Author		Andy McMaster
//Date			Thu 28 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreDeadItems()
{
	DeadStream::DeadBlock* block=NULL;

	WorldStuff &world = mobileitem::GetWorld();

//	Dead_Stream.ClrWorldDead();
// 	Dead_Stream.SetWorldDead(&world);
// 	Dead_Stream.GetWorldDead(&world);

	Replay_Dead_Stream.ClrWorldDead();

// put back in when dead stuff sorted out

	Replay_Dead_Stream.SetWorldDead(&world); // yippee it works!!

	block=Replay_Dead_Stream.firstblock;

	if (!ReplayWrite((UByte*)&Replay_Dead_Stream.olddaytime,sizeof(int)))
		return FALSE;

	while (block)
	{
// last entry in deadstream list has uniqueID=UID_Null

// valid dead item
		if (!ReplayWrite((UByte*)&(block->SGT),sizeof(UWord)))
			return FALSE;
		if (!ReplayWrite((UByte*)&(block->dataused),sizeof(UWord)))
			return FALSE;

//		if (!ReplayWrite((UByte*)&block->data,1024))
		if (!ReplayWrite((UByte*)&(block->data),block->dataused))
			return FALSE;

		block=block->nextblock;
	}

// use IllegalSepID to indicate end of list
	UWord enduid=IllegalSepID;
	if (!ReplayWrite((UByte*)&enduid,sizeof(UWord)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadDeadItems
//Author		Andy McMaster
//Date			Thu 28 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadDeadItems()
{
// construct a deadstream struc 

	WorldStuff &world = mobileitem::GetWorld();
	Bool	end=FALSE;
	UniqueID	uid;
	UWord		fake;

	Dead_Stream.ClrWorldDead();

	if (!ReplayRead((UByte*)&Dead_Stream.olddaytime,sizeof(int)))
		return FALSE;

	while (!end)
	{
		if (!ReplayRead((UByte*)&fake,sizeof(UWord)))
			return FALSE;

		uid=(UniqueID)fake;

		if (uid==IllegalSepID)
			break;

		DeadStream::DeadBlock* block=new DeadStream::DeadBlock;

		block->SGT=uid;

		if (!ReplayRead((UByte*)&(block->dataused),sizeof(UWord)))
			return FALSE;

//		if (!ReplayRead((UByte*)&block->data,1024))
		if (!ReplayRead((UByte*)&(block->data),block->dataused))
			return FALSE;

		block->nextblock=NULL;

// add block to list

		if (Dead_Stream.firstblock)
		{
			DeadStream::DeadBlock* tblock=Dead_Stream.firstblock;

			while (tblock->nextblock)
			{
				tblock=tblock->nextblock;
			}
			tblock->nextblock=block;
			block=NULL;
			tblock=NULL;
		}
		else
		{
			Dead_Stream.firstblock=block;
			block=NULL;
		}
	}

	if (!prescan)
		Dead_Stream.GetWorldDead(&world);

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		UpdateGhostAC
//Author		Andy McMaster
//Date			Thu 28 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::UpdateGhostAC()
{
	UByte	shiftfactor=_DPlay.MakeShiftValue();
	UByte	val;
	SLong	rval;
	AirStrucPtr gac=Persons2::PlayerGhostAC;

	REPLAYPACKET output;

	output.Shift=shiftfactor;
	output.IDCode=PIDC_EMPTY;

//rdh on macca advice	WorldStuff &world = mobileitem::GetWorld();
//	UWord	oldsector=	world.GetSector(gac);

#define MAKEDELTAS(src,dest,shift)	  \
	{								   \
		val=(abs(src))>>shift;			\
		if (val!=0 && src<0)			 \
		{								  \
			val|=128;					   \
		}									\
		dest=val;							 \
	}

	MAKEDELTAS(_DPlay.Deltas.lastpos.X,output.X,shiftfactor);
	MAKEDELTAS(_DPlay.Deltas.lastpos.Y,output.Y,shiftfactor);
	MAKEDELTAS(_DPlay.Deltas.lastpos.Z,output.Z,shiftfactor);
	MAKEDELTAS(_DPlay.Deltas.Heading.a,output.Heading,shiftfactor);
	MAKEDELTAS(_DPlay.Deltas.Pitch.a,output.Pitch,shiftfactor);
	MAKEDELTAS(_DPlay.Deltas.Roll.a,output.Roll,shiftfactor);

	val=(abs(_DPlay.Deltas.Velocity))>>shiftfactor;
	if(val!=0 && _DPlay.Deltas.Velocity<0)
	{
		val|=8;
	}
	output.Velocity=val;

	rval=_DPlay.GetVal(output.X,shiftfactor);
	_DPlay.Deltas.lastpos.X-=rval;
	gac->World.X+=rval;

	rval=_DPlay.GetVal(output.Y,shiftfactor);
	_DPlay.Deltas.lastpos.Y-=rval;
	gac->World.Y+=rval;

	rval=_DPlay.GetVal(output.Z,shiftfactor);
	_DPlay.Deltas.lastpos.Z-=rval;
	gac->World.Z+=rval;

	rval=_DPlay.GetVal(output.Heading,shiftfactor);
  	_DPlay.Deltas.Heading.a-=rval;
	gac->hdg+=rval;

	gac->fly.dhdg=rval;											//RDH 30Nov98

	rval=_DPlay.GetVal(output.Pitch,shiftfactor);
	_DPlay.Deltas.Pitch.a-=rval;
	gac->pitch+=rval;
	gac->fly.cpitch=gac->pitch;									//AMM 29Jun99
//	gac->fly.cpitch=gac->pitch;

	rval=_DPlay.GetVal(output.Roll,shiftfactor);
	_DPlay.Deltas.Roll.a-=rval;
	gac->roll+=rval;

	if (output.Velocity&8)
	{
		rval=-((output.Velocity&7)<<shiftfactor);
	}
	else
	{
		rval=((output.Velocity&7)<<shiftfactor);
	}
	_DPlay.Deltas.Velocity-=rval;
	gac->vel+=rval;

	_Replay.UpdatePosBuffer();									//AMM 19Mar99
	_DPlay.CalcVels();											//AMM 19Mar99

#ifdef VELCHECK
	AirStrucPtr sac=Persons2::PlayerSeenAC;
	FILE* fp=fopen("vels.txt","at");
	fprintf(fp,"vx=%d vy=%d vz=%d\n",gac->velx,gac->vely,gac->velz);
	fprintf(fp,"fmvx=%f fmvy=%f fmvz=%f\n\n",sac->fly.pModel->Vel.x,
		sac->fly.pModel->Vel.y,sac->fly.pModel->Vel.z);
	fclose(fp);
#endif
}	

//------------------------------------------------------------------------------
//Procedure		LoadItemAnims
//Author		Andy McMaster
//Date			Tue 9 Jun 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadItemAnims()
{
	MobileItemPtr	item;
	AirStrucPtr		ac,tempac;
	ULong	num,temp;
	UWord	offset;
	UByte	newval;
	ULong	n=0;
	ItemPtr i;
	UWord id=0;
	do
	{
		if (!ReplayRead((UByte*)&id,sizeof(UWord)))
			return FALSE;

		ac=*Persons2::ConvertPtrUID((UniqueID)id);

		if (ac)
		{
			if (!prescan) // && DoSmokeTrailStuff)
			{
				if (ac->Anim == NULL)			   //RJS 23Apr99
				{
// I dont think this should ever happen, maybe in mig super flight?

					SHAPE.SetAnimData(ac,ac->weap.AircraftStatus.Weapons);
				}
				SHAPE.ResetAnimData_NewShape(ac,ac->shape,ac->weap.AircraftStatus.Weapons);	//AMM 08Apr99
			}

			if (!ReplayRead((UByte*)&num,sizeof(ULong)))
				return FALSE;

			temp=num;
			n = 0;	

			while (num--)
			{
				if (!ReplayRead((UByte*)&offset,sizeof(UWord)))
					return FALSE;

				if (!ReplayRead((UByte*)&newval,sizeof(UByte)))
					return FALSE;

				if (!prescan) // && DoSmokeTrailStuff)
				{
					SHAPE.AnimDeltaList[n].deltaoffset=offset;
					SHAPE.AnimDeltaList[n++].newbyte=newval;
				}
			}

			if (!prescan) // && DoSmokeTrailStuff)
				SHAPE.PatchAnimData((ItemPtr)ac,n);
		}
	}
	while (id!=0);
	if (!prescan)
	{
		//Stop muzzleflash for player...
		AircraftAnimData*	adptr = (AircraftAnimData*) Persons2::PlayerSeenAC->Anim;
		adptr->shooting = 0;
	}

	do 
	{
		if (!ReplayRead((UByte*)&id,sizeof(UWord)))
			return FALSE;

		item=(MobileItemPtr)Persons2::ConvertPtrUID((UniqueID)id);

		if (item)
		{
			if (!prescan) // && DoSmokeTrailStuff)
			{
				if (item->Anim == NULL)				//RJS 23Apr99
				{
					SHAPE.SetAnimData(item,0);
				}
				SHAPE.ResetAnimData_NewShape(item,item->shape);
			}

			if (!ReplayRead((UByte*)&num,sizeof(ULong)))
				return FALSE;

			temp=num;
			n = 0;    

			while (num--)
			{
				if (!ReplayRead((UByte*)&offset,sizeof(UWord)))
					return FALSE;

				if (!ReplayRead((UByte*)&newval,sizeof(UByte)))
					return FALSE;

				if (!prescan) // && DoSmokeTrailStuff)
				{
					SHAPE.AnimDeltaList[n].deltaoffset=offset;
					SHAPE.AnimDeltaList[n++].newbyte=newval;
				}
			}

			if (!prescan) // && DoSmokeTrailStuff)
				SHAPE.PatchAnimData(item,n);
		}
	}
	while (id!=0);

// items done by dead stream
// all dead or damaged are in dead stream

	return TRUE;
}
//------------------------------------------------------------------------------
//Procedure		StoreItemAnims
//Author		Andy McMaster
//Date			Tue 9 Jun 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreItemAnims()
{
	MobileItemPtr	item;
	AirStrucPtr		ac,tempac;
	ULong	num;
	ULong	n = 0,m;
	ItemPtr i;
	UWord uid;

	ac=*AirStruc::ACList;

	while (ac)
	{
		num=SHAPE.ScanAnimData((ItemPtr)ac);

		if (num)
		{
			uid=ac->uniqueID.bitfieldUid.count;

			if (!ReplayWrite((UByte*)&uid,sizeof(UWord)))
				return FALSE;

			if (!ReplayWrite((UByte*)&num,sizeof(ULong)))
				return FALSE;

			for (n=0; n < num; n++)
			{
				if (!ReplayWrite((UByte*)&SHAPE.AnimDeltaList[n].deltaoffset,sizeof(UWord)))
					return FALSE;

				if (!ReplayWrite((UByte*)&SHAPE.AnimDeltaList[n].newbyte,sizeof(UByte)))
					return FALSE;
			}
		}

		ac=*ac->nextmobile;
	}

	uid=0;

	if (!ReplayWrite((UByte*)&uid,sizeof(UWord)))
		return FALSE;

	item=mobileitem::MobileList;

	while (item)
	{
		num=SHAPE.ScanAnimData(item);

		if (num)
		{
			uid=item->uniqueID.bitfieldUid.count;
			if (!ReplayWrite((UByte*)&uid,sizeof(UWord)))
				return FALSE;

			if (!ReplayWrite((UByte*)&num,sizeof(ULong)))
				return FALSE;

			for (n=0; n < num; n++)
			{
				if (!ReplayWrite((UByte*)&SHAPE.AnimDeltaList[n].deltaoffset,sizeof(UWord)))
					return FALSE;

				if (!ReplayWrite((UByte*)&SHAPE.AnimDeltaList[n].newbyte,sizeof(UByte)))
					return FALSE;
			}
		}

		item=item->nextmobile;
	}

// UID_NULL indicates end of data
	uid=0;
	if (!ReplayWrite((UByte*)&uid,sizeof(UWord)))
		return FALSE;

// is this necessary?
// NO - items done by deadstream
// dead stream has all dead or damaged items

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadBinaryBattlefield
//Author		Andy McMaster
//Date			Wed 1 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Procedure		StoreBinaryBattlefield
//Author		Andy McMaster
//Date			Wed 1 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Procedure		BlockForward
//Author		Andy McMaster
//Date			Wed 1 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::BlockForward()
{
	SLong n=sizeof(REPLAYPACKET);
	SLong m=numframes[currblock-1];
	UByteP temp=playbackfilepos;

	ResetFlags();

	m-=replayframecount;

	SLong skip=n*m;
	playbackfilepos+=skip;
	
//	RemoveAllTransients();
	if (!LoadBlockHeader())
	{
		playbackfilepos=temp;

		return FALSE;
	}
//	RemoveAllTransients();
	ResetReplayView();
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		BlockRewind
//Author		Andy McMaster
//Date			Wed 1 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::BlockRewind(Bool framerewind)
{
	SLong n=sizeof(REPLAYPACKET);
	SLong m=replayframecount;
	SLong skip=0;
	UByteP temp=playbackfilepos;
	ULong pos=playbackfilepos-(UByteP)playbackfilestart;
	UByte	bskip=0;

	ResetFlags();

// if we are 1 sec or less into block then goto to start of previous block
// rather than start of this one
//actually 2 secs with 100 frames

	if ((replayframecount<100 && !Three_Dee.IsPaused() && !framerewind)
	|| replayframecount==0)
	{
		if (currblock>1)
		{
			skip+=m*n;
			skip+=headlength[currblock-1];

			currblock--;
			bskip=1;
		}
	}

	if (bskip)
	{
		skip+=numframes[currblock-1]*n;
	}
	else
	{
		skip+=(n*m);
	}

	skip+=headlength[currblock-1];

	if (skip>pos)
	{
		skip=pos;
	}

	playbackfilepos-=skip;

	currblock--;
	bskip++;

//	RemoveAllTransients();
	if (!LoadBlockHeader())
	{
		playbackfilepos=temp;
		currblock+=bskip;
		return FALSE;
	}
//	RemoveAllTransients();
	ResetReplayView();
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadPilots
//Author		Andy McMaster
//Date			Wed 1 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Procedure		StorePilots
//Author		Andy McMaster
//Date			Wed 1 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Procedure		StoreHeaderLength
//Author		Andy McMaster
//Date			Thu 2 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreHeaderLength(ULong length)
{
	if (!length)
	{
		FlushFileBuffers(recordstorage);
		length=GetFileSize(recordstorage,NULL);
	}

	ULong reallen=length+sizeof(ULong);

	if (!ReplayWrite((UByte*)&reallen,sizeof(ULong)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadHeaderLength
//Author		Andy McMaster
//Date			Thu 2 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ULong	Replay::LoadHeaderLength()
{
	ULong len;

	if (!ReplayRead((UByte*)&len,sizeof(ULong)))
		return FALSE;

	return len;
}


//------------------------------------------------------------------------------
//Procedure		DeleteGRList
//Author		Andy McMaster
//Date			Wed 8 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::DeleteGRList(LPREPGRENTRY& list)
{
	LPREPGRENTRY temp1,temp2;

	temp1=list;

	while (temp1)
	{
		temp2=temp1->next;
		delete temp1;
		temp1=NULL;
		temp1=temp2;
	}
	list=NULL;
}

//------------------------------------------------------------------------------
//Procedure		ReplayRead
//Author		Andy McMaster
//Date			Sun 5 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
//Bool	Replay::ReplayRead(UByte* dest, ULong size)
Bool	Replay::ReplayRead(UByte* dest, ULong size)
{
	LPASPRIMARYVALUES lpas;

	if (playbackfilepos+size>playbackfileend)
	{
		PlaybackPaused=TRUE;
		return FALSE;
	}
//	else
//	{
//		PlaybackPaused=FALSE;
//	}


	memcpy(dest,playbackfilepos,size);
	playbackfilepos+=size;

	lpas=(LPASPRIMARYVALUES)dest;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		ReplayWrite
//Author		Andy McMaster
//Date			Sun 5 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
//Bool	Replay::ReplayWrite(UByte* src, ULong size)
Bool	Replay::ReplayWrite(UByte* src, ULong size)
{
	DWORD num;

 	if (WriteFile(recordstorage,src,size,&num,NULL))
 		return TRUE;

 	return FALSE;
		
//	memcpy(recordfilepos,src,size);
//	recordfilepos+=size;
//	return TRUE;
}
//------------------------------------------------------------------------------
//Procedure		GetInitalGlobRefs
//Author		Andy McMaster
//Date			Wed 8 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::GetInitialGlobRefs()
{
	Bool	finished=FALSE;
	ULong	val,n=0;
	LPREPGRENTRY	temp1,temp2;

	DeleteGRList(initgrlist);

	GetGlobRefs(initgrlist);
}
//------------------------------------------------------------------------------
//Procedure		StoreInitalGlobRefs
//Author		Andy McMaster
//Date			Wed 8 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreInitialGlobRefs()
{
// store Pack_PlayerSquad and Pack_PlayerPos because these are no longer glob refs

	if (!ReplayWrite((UByte*)&PlayerSquad,sizeof(int)))
		return FALSE;

	if (!ReplayWrite((UByte*)&PlayerPos,sizeof(int)))
		return FALSE;

	if (!StoreGlobRefList(initgrlist))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreGlobRefList
//Author		Andy McMaster
//Date			Tue 15 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreGlobRefList(LPREPGRENTRY list)
{
	ULong val;
	ULong n=0,m;
	ULong t[8];
	UByte set=0;
	Bool finished=FALSE;
	LPREPGRENTRY temp=list;

	while (!finished && temp)
	{
		for (m=0;m<8;m++)
		{
			val=temp->val;
			temp=temp->next;
			t[m]=val;

			if (t[m]!=0)
				set|=0x1<<m;

			if (t[m]==BAD_RV || !temp)
			{
				finished=TRUE;
				break;
			}
			n++;	
		}

// have got 8 values, store header and altered values

		if (!ReplayWrite((UByte*)&set,sizeof(UByte)))
			return FALSE;

		for (m=0;m<8;m++)
		{
			if (set&0x1<<m)
			{
				if (!ReplayWrite((UByte*)&t[m],sizeof(int)))
					return FALSE;
			}
		}
		set=0;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreHeaderID
//Author		Andy McMaster
//Date			Thu 9 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreHeaderID()
{
	ULong id=REPLAYHEADBEGINID;

	if (!ReplayWrite((UByte*)&id,sizeof(ULong)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadHeaderID
//Author		Andy McMaster
//Date			Thu 9 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadHeaderID()
{
	ULong id=0;

	if (!ReplayRead((UByte*)&id,sizeof(ULong)))
		return FALSE;

	if (id!=REPLAYHEADBEGINID)
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadReplayData
//Author		Andy McMaster
//Date			Fri 10 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadReplayData(char* name)
{
	strcpy(pfilename,name);
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		SaveReplayData
//Author		Andy McMaster
//Date			Fri 10 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::SaveReplayData(char* name)
{
#if 0 //RERUN
	HANDLE file;
	char	buffer[150];
	char	buffer2[150];
	
	if (RestorePosition)
	{
		strcpy(buffer,"tblock.dat");
	}
	else
	{
		strcpy(buffer,"replay.dat");
	}

	strcpy(buffer2,name);

	GetReplayFilename(buffer);
	GetReplayFilename(buffer2);

	if (!RestorePosition)
		strcpy(pfilename,buffer);

	file=CreateFile(
		buffer,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_TEMPORARY,
		NULL);

	if (file==INVALID_HANDLE_VALUE)
	{
		CloseHandle(file);
		_DPlay.CommsMessage(IDS_NOREPLAY);
		return TRUE;
	}

	CloseHandle(file);

	if (!CopyFile(buffer,buffer2,FALSE))
		_DPlay.CommsMessage(IDS_NOTSAVE);

#endif
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadBFieldNum
//Author		Andy McMaster
//Date			Thu 30 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
FileNum	Replay::LoadBFieldNum()
{
	FileNum file=FIL_NULL;

	ReplayRead((UByte*)&file,sizeof(FileNum));

	return file;
}

//------------------------------------------------------------------------------
//Procedure		StoreBFieldNum
//Author		Andy McMaster
//Date			Thu 30 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreBFieldNum(FileNum bf)
{
	if (!ReplayWrite((UByte*)&bf,sizeof(FileNum)))	
		return FALSE;

	return TRUE;
}

ULong	Replay::LoadBFCtrl()
{
	ULong a;

	ReplayRead((UByte*)&a,sizeof(ULong));

	return a;
}

Bool	Replay::StoreBFCtrl(ULong ctrl)
{
	if (!ReplayWrite((UByte*)&ctrl,sizeof(ULong)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreItemData
//Author		Andy McMaster
//Date			Thu 27 Aug 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreItemData()
{
	MobileItemPtr	item;
	AirStrucPtr		ac;

	ac=*AirStruc::ACList;

	while (ac)
	{
// airstruc stuff

		if (!StorePrimaryASData(ac)) //,&aspv))
			return FALSE;

		if (!StorePrimaryMIData((MobileItemPtr)ac)) //,&mipv))
			return FALSE;

		ac->fly.cpitch=ac->pitch;								//AMM 30Jun99
		ac->CalcXYZVel();											//AMM 30Jun99

		if (ac==Persons2::PlayerSeenAC)							//AMM 30Jun99
		{														//AMM 30Jun99
			ac->fly.cpitch=ac->pitch;								//AMM 30Jun99
			ac->CalcXYZVel();											//AMM 30Jun99
		}														//AMM 30Jun99

// need to store this for anim purposes for all ac

		if (!StoreAeroDevData(ac))
			return FALSE;

// flight model stuff

		if (!ac->ai.PilotSkills.simpleacm && ac->AutoMoveInfo.movecode==AUTO_COMBAT && !ac->MigStatus.LaunchFlags.deaded)
		{
			if (ac!=Persons2::PlayerSeenAC)
			{
				if (!StorePrimaryFMData(ac))
					return FALSE;

	 			if (!StorePrimaryEngineData(ac))
					return FALSE;

				if (!StorePrimaryElmtData(ac))
					return FALSE;

				if (!StoreSuspensionData(ac))
					return FALSE;
			}
		}
		else
		{
			if (!StoreFMMass(ac))
				return FALSE;

			if (ac!=Persons2::PlayerSeenAC)
				ac->fly.pModel->FirstCycle=TRUE;
		}

		ResetSecondaryFMValues(ac);

		ac=*ac->nextmobile;
	}

	item=mobileitem::MobileList;

	while (item)
	{
		if (!StorePrimaryMIData(item)) //,&mipv))
			return FALSE;

		item->CalcXYZVel();										//AMM 28Apr99

		item=item->nextmobile;
	}
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadItemData
//Author		Andy McMaster
//Date			Thu 27 Aug 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadItemData()
{
	MobileItemPtr	item;
	AirStrucPtr		ac;
	UWord	oldsector,newsector;;
	WorldStuff &world = mobileitem::GetWorld();
	ASPRIMARYVALUES aspv;
	MIPRIMARYVALUES mipv;
	AERODEVVALUES	adv;

	ac=*AirStruc::ACList;

	while (ac)
	{
		oldsector=world.GetSector((MobileItemPtr)ac);

		if (!ReplayRead((UByte*)&aspv,sizeof(ASPRIMARYVALUES)))
			return FALSE;

		if (!ReplayRead((UByte*)&mipv,sizeof(MIPRIMARYVALUES)))
			return FALSE;

		RestorePrimaryASValues( ac,&aspv);
		RestorePrimaryMIValues((MobileItemPtr)ac,&mipv);

		ac->fly.cpitch=ac->pitch;								//AMM 30Jun99
		ac->CalcXYZVel();											//AMM 30Jun99

		if (ac==Persons2::PlayerSeenAC)							//AMM 30Jun99
		{														//AMM 30Jun99
			ac->fly.cpitch=ac->pitch;							//AMM 30Jun99
			ac->CalcXYZVel();									//AMM 30Jun99
		}														//AMM 30Jun99

		PAERODEVICE pAeroDevice=ac->fly.pModel->DeviceList;

		while (pAeroDevice)
		{
			if (!ReplayRead((UByte*)&adv,sizeof(AERODEVVALUES)))
				return FALSE;

			if (!prescan)
				RestoreAeroDevData(&adv,pAeroDevice);									//AMM 26May99

			pAeroDevice=pAeroDevice->List.NextItem();
		}

		newsector=world.GetSector((MobileItemPtr)ac);

		if (newsector!=oldsector)
		{
			world.RemoveFromSector((MobileItemPtr)ac,oldsector);
			world.AddToWorld((MobileItemPtr)ac);
		}

		if (!ac->ai.PilotSkills.simpleacm && ac->AutoMoveInfo.movecode==AUTO_COMBAT && !ac->MigStatus.LaunchFlags.deaded) 
		{
			if (ac!=Persons2::PlayerSeenAC)
			{
				if (!LoadFMData(ac))
					return FALSE;
			}
		}
		else
		{
			if (!LoadFMMass(ac))
				return FALSE;

			if (ac!=Persons2::PlayerSeenAC)
				ac->fly.pModel->FirstCycle=TRUE;
		}

		if (!prescan)
			ResetSecondaryFMValues(ac);

		ac=*ac->nextmobile;
	}	

 	item=mobileitem::MobileList;

 	while (item)
 	{
		oldsector=world.GetSector(item);

		if (!ReplayRead((UByte*)&mipv,sizeof(MIPRIMARYVALUES)))
			return FALSE;

		if (!prescan)
		{
			RestorePrimaryMIValues(item,&mipv);

			newsector=world.GetSector(item);

			if (newsector!=oldsector)
			{
				world.RemoveFromSector(item,oldsector);
				world.AddToWorld(item);
			}
		}

		item->CalcXYZVel();										//AMM 28Apr99

 		item=item->nextmobile;
 	}
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StorePrimaryFlightModelData
//Author		Andy McMaster
//Date			Thu 27 Aug 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StorePrimaryFMData(AirStrucPtr ac)
{
	FMPRIMARYVALUES	fmpv;

	GetPrimaryFMData(ac,&fmpv);

	if (!ReplayWrite((UByte*)&fmpv,sizeof(FMPRIMARYVALUES)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetPrimaryFMData
//Author		Andy McMaster
//Date			Tue 10 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::GetPrimaryFMData(AirStrucPtr ac, LPFMPRIMARYVALUES fmpv)
{

//model.h

	fmpv->FirstCycle=ac->fly.pModel->FirstCycle;				//AMM 29Jun99
	fmpv->RotVel=ac->fly.pModel->RotVel;
	fmpv->Elevator=ac->fly.pModel->Elevator;
	fmpv->Aileron=ac->fly.pModel->Aileron;
	fmpv->Rudder=ac->fly.pModel->Rudder;
	fmpv->CrashTime=ac->fly.pModel->CrashTime;

// acmmodel.h
	
	fmpv->lastdeltapitch=ac->fly.pAcmModel->lastdeltapitch;
	fmpv->deltapitchsum=ac->fly.pAcmModel->deltapitchsum;
	fmpv->deltarollsum=ac->fly.pAcmModel->deltarollsum;
	fmpv->deltayawsum=ac->fly.pAcmModel->deltayawsum;
	fmpv->maxdeltaforclosework=ac->fly.pAcmModel->maxdeltaforclosework;
	fmpv->OldCtrlRoll=ac->fly.pAcmModel->OldCtrlRoll;
	fmpv->OldCtrlTurnRate=ac->fly.pAcmModel->OldCtrlTurnRate;
	fmpv->OldCtrlYawRate=ac->fly.pAcmModel->OldCtrlYawRate;
	fmpv->desiredRoll=ac->fly.pAcmModel->desiredRoll;

}

//------------------------------------------------------------------------------
//Procedure		StorePrimaryEngineData
//Author		Andy McMaster
//Date			Tue 29 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StorePrimaryEngineData(AirStrucPtr ac)
{
	PRIMARYENGINEVALUES enpv;

// engine.h

	PENGINE pEngine = ac->fly.pModel->EngineList;

	while (pEngine)
	{
		GetPrimaryEngineData(&enpv,pEngine);

		if (!ReplayWrite((UByte*)&enpv,sizeof(PRIMARYENGINEVALUES)))
			return FALSE;

		pEngine=pEngine->List.NextItem();
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetPrimaryEngineData
//Author		Andy McMaster
//Date			Tue 10 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::GetPrimaryEngineData(LPPRIMARYENGINEVALUES enpv, PENGINE pEngine)
{
	enpv->Speed=pEngine->Speed;
	enpv->SpeedRpm=pEngine->SpeedRpm;
	enpv->MaxThrust=pEngine->MaxThrust;
	enpv->PropInc=pEngine->PropInc;
	enpv->PropVel=pEngine->PropVel;
	enpv->SlipVel=pEngine->SlipVel;
	enpv->SlipRot=pEngine->SlipRot;
	enpv->BlowerHigh=pEngine->BlowerHigh;
}

//------------------------------------------------------------------------------
//Procedure		StorePrimaryElmtData
//Author		Andy McMaster
//Date			Tue 29 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StorePrimaryElmtData(AirStrucPtr ac)
{
	PRIMARYELMTVALUES elpv;

// elmt.h

	PMAINPLANE pMainPlane = ac->fly.pModel->MainPlaneList;

	while (pMainPlane)
	{
		GetPrimaryElmtData(&elpv,pMainPlane);

		if (!ReplayWrite((UByte*)&elpv,sizeof(PRIMARYELMTVALUES)))
			return FALSE;

		pMainPlane=pMainPlane->List.NextItem();
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetPrimaryElmtData
//Author		Andy McMaster
//Date			Tue 10 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::GetPrimaryElmtData(LPPRIMARYELMTVALUES elpv,PMAINPLANE pMainPlane)
{
		for(SWord wing = 0; wing < 2; wing++)
			for(SWord section = 0; section < 2; section++)
			{
				elpv->aoa[wing][section]=pMainPlane->aoa[wing][section];
			}
}

//------------------------------------------------------------------------------
//Procedure		RestorePrimaryFMData
//Author		Andy McMaster
//Date			Tue 10 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RestorePrimaryFMData(AirStrucPtr ac,LPFMPRIMARYVALUES fmpv)
{
//model.h

	ac->fly.pModel->FirstCycle=fmpv->FirstCycle;				//AMM 29Jun99
	ac->fly.pModel->RotVel=fmpv->RotVel;
	ac->fly.pModel->Elevator=fmpv->Elevator;
	ac->fly.pModel->Aileron=fmpv->Aileron;
	ac->fly.pModel->Rudder=fmpv->Rudder;
	ac->fly.pModel->CrashTime=fmpv->CrashTime;

//acmmodel.h

	ac->fly.pAcmModel->lastdeltapitch=fmpv->lastdeltapitch;
	ac->fly.pAcmModel->deltapitchsum=fmpv->deltapitchsum;
	ac->fly.pAcmModel->deltarollsum=fmpv->deltarollsum;
	ac->fly.pAcmModel->deltayawsum=fmpv->deltayawsum;
	ac->fly.pAcmModel->maxdeltaforclosework=fmpv->maxdeltaforclosework;
	ac->fly.pAcmModel->OldCtrlRoll=fmpv->OldCtrlRoll;
	ac->fly.pAcmModel->OldCtrlTurnRate=fmpv->OldCtrlTurnRate;
	ac->fly.pAcmModel->OldCtrlYawRate=fmpv->OldCtrlYawRate;
	ac->fly.pAcmModel->desiredRoll=fmpv->desiredRoll;

}

//------------------------------------------------------------------------------
//Procedure		RestorePrimaryEngineData
//Author		Andy McMaster
//Date			Tue 10 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RestorePrimaryEngineData(LPPRIMARYENGINEVALUES e, PENGINE pEngine)
{
	pEngine->Speed=e->Speed;
	pEngine->SpeedRpm=e->SpeedRpm;
	pEngine->MaxThrust=e->MaxThrust;
	pEngine->PropInc=e->PropInc;
	pEngine->PropVel=e->PropVel;
	pEngine->SlipVel=e->SlipVel;
	pEngine->SlipRot=e->SlipRot;
	pEngine->BlowerHigh=e->BlowerHigh;
}

//------------------------------------------------------------------------------
//Procedure		RestorePrimaryElmtData
//Author		Andy McMaster
//Date			Tue 10 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RestorePrimaryElmtData(LPPRIMARYELMTVALUES elpv,PMAINPLANE pMainPlane)
{
		for(SWord wing = 0; wing < 2; wing++)
			for(SWord section = 0; section < 2; section++)
			{
				pMainPlane->aoa[wing][section]=elpv->aoa[wing][section];
			}

}

//------------------------------------------------------------------------------
//Procedure		LoadPrimaryFlightModelData
//Author		Andy McMaster
//Date			Thu 27 Aug 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadFMData(AirStrucPtr ac)
{
	FMPRIMARYVALUES fmpv;

	if (!ReplayRead((UByte*)&fmpv,sizeof(FMPRIMARYVALUES)))
		return FALSE;

	if (!prescan)
		RestorePrimaryFMData(ac,&fmpv);

// engine.h

	PENGINE pEngine = ac->fly.pModel->EngineList;
	PRIMARYENGINEVALUES e;

	while (pEngine)
	{
		if (!ReplayRead((UByte*)&e,sizeof(PRIMARYENGINEVALUES)))
			return FALSE;

		if (!prescan)
			RestorePrimaryEngineData(&e,pEngine);

		pEngine=pEngine->List.NextItem();
	}

// elmt.h

	PMAINPLANE pMainPlane = ac->fly.pModel->MainPlaneList;
	PRIMARYELMTVALUES elpv;

	while (pMainPlane)
	{
		if (!ReplayRead((UByte*)&elpv,sizeof(PRIMARYELMTVALUES)))
			return FALSE;

		if (!prescan)
			RestorePrimaryElmtData(&elpv,pMainPlane);

		pMainPlane=pMainPlane->List.NextItem();
	}

	FP legpos;
	PGEAR pGear=ac->fly.pModel->GearList;

	while (pGear)
	{
		if (!ReplayRead((UByte*)&legpos,sizeof(FP)))
			return FALSE;

		if (!prescan)
			RestoreSuspensionData(legpos,pGear);

	 	pGear = pGear->List.NextItem();
	}
	
	return TRUE;
}


//------------------------------------------------------------------------------
//Procedure		ResetSecondaryFlightModelData
//Author		Andy McMaster
//Date			Thu 27 Aug 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::ResetSecondaryFMValues(AirStrucPtr ac)
{
	FCRD	backupvel;

	PMODEL p=ac->fly.pModel;
	PACMMODEL a=ac->fly.pAcmModel;
	FCRD	temprotvel=p->RotVel;

	if (ac==Persons2::PlayerSeenAC)
		backupvel=p->Vel;

// model.h

	p->FrameCount=ac->uniqueID.bitfieldUid.count;

	p->Gust.Time=0;

	p->Acc.x=0;
	p->Acc.y=0;
	p->Acc.z=0;

	p->CheatAcc.x=32767;
	p->CheatAcc.y=32767;
	p->CheatAcc.z=32767;

// acmmodel.h

	ac->fly.pAcmModel->CtrlThrust=0;
	ac->fly.pAcmModel->CtrlRollRate=0.0;
	ac->fly.pAcmModel->CtrlPitchRate=0.0;
	ac->fly.pAcmModel->CtrlPitch=0.0;
	ac->fly.pAcmModel->CtrlRoll=0.0;
	ac->fly.pAcmModel->CtrlSpeed=0.0;
	ac->fly.pAcmModel->CtrlMach=0.0;
	ac->fly.pAcmModel->CtrlClimbRate=0.0;
	ac->fly.pAcmModel->CtrlTurnRate=0.0;
	ac->fly.pAcmModel->CtrlYawRate=0.0;
	ac->fly.pAcmModel->CtrlHeight=0.0;
	ac->fly.pAcmModel->CtrlMinHeight=0.0;
	ac->fly.pAcmModel->CtrlHeading=0.0;
	ac->fly.pAcmModel->CtrlVector.x=0.0;
	ac->fly.pAcmModel->CtrlVector.y=0.0;
	ac->fly.pAcmModel->CtrlVector.z=0.0;
	ac->fly.pAcmModel->CtrlMaxBank=0.0;
	ac->fly.pAcmModel->CtrlRudder=0;
	ac->fly.pAcmModel->CtrlElevator=0;
	ac->fly.pAcmModel->CtrlAileron=0;
	ac->fly.pAcmModel->CtrlAoA=0.0;
	ac->fly.pAcmModel->OldAoa=0.0;								//AMM 08Jul99

// engine.h
	
	// none

// elmt.h

	PMAINPLANE pMainPlane = ac->fly.pModel->MainPlaneList;

	while (pMainPlane)
	{
		pMainPlane->fCd0Mach = 1;
		pMainPlane->fKMach   = 1;
		pMainPlane=pMainPlane->List.NextItem();
	}

//call updatemodel
//call calcairvel
//call gearcalcs
//call accurategroundaltitude
//call SetAirStrucOrientation - WHY?????
// calc cpitch 
// calc i_a_s - INSTRUMENTS NOT NEEDED
// calc vel_cms

	ac->fly.pModel->UpdateModel(ac);
	ac->fly.pModel->UpdateAirStruc(ac);							//AMM 30Jun99
	ac->fly.pModel->UpdateMassInertias(ac);						//AMM 29Jun99
	if (ac!=Persons2::PlayerSeenAC)									  //AMM 08/07/99
		ac->fly.pModel->Instruments(ac);								  //AMM 08/07/99

// update model resets rotvel, so update it again!

	p->RotVel=temprotvel;
// 	ac->fly.pModel->ProcessGears(ac, 0); // this calls AccurateGroundAltitude	//CSB 01/02/99
	ac->fly.pModel->CalcAirVel(ac);

	MMC.Sky.Ambient(p->Pos.y,p->AmbDensity,p->AmbTemp,p->AmbPres);

// set up thrust

	if (ac!=Persons2::PlayerSeenAC)						//AMM 28/07/99
	{													//AMM 28/07/99
		PENGINE pEngine = ac->fly.pModel->EngineList;	//AMM 28/07/99
														//AMM 28/07/99
		while (pEngine)									//AMM 28/07/99
		{												//AMM 28/07/99
			pEngine->Thrust = 0;						//AMM 28/07/99
			pEngine=pEngine->List.NextItem();			//AMM 28/07/99
		}												//AMM 28/07/99
	}													//AMM 28/07/99

// calc groundlevel

	_Collide.LowestSafeAlt(ac,ac->pitch,ac->roll);

	Bool	realplayback=Playback;
	Playback=TRUE;
	ac->SetFlightParams	(false); // sets vel_cms

	if (ac==Persons2::PlayerSeenAC)
		p->Vel=backupvel;

	Playback=realplayback;

	ac->weap.currentbullet=NULL;
}

//------------------------------------------------------------------------------
//Procedure		ResetReplayData
//Author		Andy McMaster
//Date			Tue 15 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::ResetReplayData()
{
	replayframecount=0;
	currblock=0;
	numblocks=0;

	ResetFlags();

	ReplayDeathMode = RDEATH_NULL;
}

//------------------------------------------------------------------------------
//Procedure		AddFileAndGLobRefsToList
//Author		Andy McMaster
//Date			Tue 15 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::AddFileAndGlobRefsToList(FileNum file,ULong bfctrl)
{
 	LPGRLIST temp=bfgrlist;

	LPGRLIST temp2=new GRLIST;

	temp2->file=file;
	temp2->bfctrl=bfctrl;
	
	temp2->list=NULL;
	GetGlobRefs(temp2->list);
	temp2->next=NULL;

	if (temp)
	{
		while (temp->next)
		{
			temp=temp->next;
		}
		temp->next=temp2;
	}
	else
	{
		bfgrlist=temp2;
	}

	temp2=NULL;

}

//------------------------------------------------------------------------------
//Procedure		GetGlobRefs
//Author		Andy McMaster
//Date			Tue 15 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::GetGlobRefs(LPREPGRENTRY& list)
{
	Bool	finished=FALSE;
	ULong	n=0;
	int val;
	LPREPGRENTRY	temp1,temp2;

	while (!finished)
	{
		val=Persons2::GetLoc(n++);

		if (val==BAD_RV)
			finished=TRUE;

		temp1=new REPGRENTRY;
		temp1->val=val;
		temp1->next=NULL;


		if (list)
		{
			temp2=list;

			while (temp2->next)
				temp2=temp2->next;

			temp2->next=temp1;
			temp2=NULL;
		}
		else
		{
			list=temp1;
		}
		temp1=NULL;
	}
}

//------------------------------------------------------------------------------
//Procedure		DeleteFileAndGlobRefList
//Author		Andy McMaster
//Date			Tue 15 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::DeleteFileAndGlobRefList(LPGRLIST& list)
{
	LPGRLIST	temp2,temp=list;
	LPREPGRENTRY	t1,t2;

	while (temp)
	{
		temp2=temp->next;
		
		t1=temp->list;

		DeleteGRList(t1);
		t1=NULL;

		delete temp;
		temp=NULL;
		temp=temp2;
	}

	list=NULL;
}

//------------------------------------------------------------------------------
//Procedure		StoreFileAndGlobRefList
//Author		Andy McMaster
//Date			Tue 15 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreFileAndGlobRefList()
{
	LPGRLIST	temp=bfgrlist;

	while (temp)
	{
		if (!StoreBFieldNum(temp->file))
			return FALSE;

		if (!StoreBFCtrl(temp->bfctrl))
			return FALSE;

		if (!StoreGlobRefList(temp->list))
			return FALSE;

		temp=temp->next;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadFileAndGlobRefList
//Author		Andy McMaster
//Date			Tue 15 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadFileAndGlobRefList()
{
	FileNum file;
	ULong ctrl;

	DeleteFileAndGlobRefList(bfgrlist);

	do
	{
		LPGRLIST temp=new GRLIST;

		temp->next=NULL;
		temp->file=LoadBFieldNum();
		temp->bfctrl=LoadBFCtrl();
		temp->list=NULL;
		file=temp->file;
		ctrl=temp->bfctrl;
		if (!LoadGlobRefList(temp->list))
			return FALSE;

		if (bfgrlist)
		{
			LPGRLIST temp2=bfgrlist;

			while (temp2->next)
				temp2=temp2->next;

			temp2->next=temp;
		}
		else
		{
			bfgrlist=temp;
		}
		temp=NULL;
	}
	while (ctrl!=Persons2::FINISHBF);

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		SetGlobRefs
//Author		Andy McMaster
//Date			Tue 15 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::SetGlobRefs(LPREPGRENTRY list)
{
	LPREPGRENTRY temp=list;
	ULong n=0;

	while (temp && temp->val!=BAD_RV)
	{
		Persons2::SetLoc(n,temp->val);
		n++;
		temp=temp->next;
	}
}

//------------------------------------------------------------------------------
//Procedure		SkipToBeginning
//Author		Andy McMaster
//Date			Wed 16 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::SkipToBeginning()
{
	ResetFlags();

	playbackfilepos=(UByteP)playbackfilestart+SuperHeaderSize;
	currblock=0;

//	RemoveAllTransients();
	if (!LoadBlockHeader())
	{
		_Error.SayAndQuit("Error: cannot read playback file");
//		playbackfilepos=temp;
//		currblock=tempb;
	}
//	RemoveAllTransients();
	ResetReplayView();
}

//------------------------------------------------------------------------------
//Procedure		SkipToEnd
//Author		Andy McMaster
//Date			Wed 16 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::SkipToEnd()
{
	SLong n=sizeof(REPLAYPACKET);
	SLong m=replayframecount;
	UByteP temp=playbackfilepos;
	SLong skip;

//	Timer_Code.paused=TRUE;
	PlaybackPaused=TRUE;
	ResetFlags();

// rewind to beginning of this block

	skip=n*m;
	skip+=headlength[currblock-1];
	playbackfilepos-=skip;
	currblock--;

// keep jumping forward till no new headers

//	RemoveAllTransients();
	while (LoadBlockHeader())
	{
//		ReplayFlag=RF_SKIPFRAMES;			
		m=numframes[currblock-1];
//		framestoskip=m;
		skip=n*m;
		
		temp=playbackfilepos;
		playbackfilepos+=skip;
	}

// jump to end of last good header, frames will be skipped next

	framestoskip=thisblockendframe;
	_Replay.ReplayFlag=RF_SKIPFRAMES;
	playbackfilepos=temp;
	ResetReplayView();
}

//------------------------------------------------------------------------------
//Procedure		FrameForward
//Author		Andy McMaster
//Date			Wed 16 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::FrameForward()
{
//	ResetFlags();

//	FrameForwardFlag=TRUE;

// dont need to do anything !!!!!!!!!!!!!!!!
}

//------------------------------------------------------------------------------
//Procedure		FrameRewind
//Author		Andy McMaster
//Date			Wed 16 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ULong	Replay::FrameRewind()
{
	ULong temp=replayframecount,skip=0;

	ResetFlags();
//	FrameRewindFlag=TRUE;

	WorldStuff& world=mobileitem::GetWorld();

	if (!(temp==0 && currblock==1))
	{
		if (temp==0)
		{
			skip=numframes[currblock-2]-1;
		}
		else
		{
			skip=temp-1;
		}

		if (!BlockRewind(TRUE))
		{
// dont call skip if there are no frames to skip

			return 0;
		}
	}
	else
	{
// at beginning of file, dont do anything
		
	}
	return skip;
}

//------------------------------------------------------------------------------
//Procedure		StoreInitFrameCounts
//Author		Andy McMaster
//Date			Thu 17 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreInitFrameCounts()
{
	UWord	temp=0;

	if (!ReplayWrite((UByte*)&temp,sizeof(UWord)))
		return FALSE;

	if (!ReplayWrite((UByte*)&temp,sizeof(UWord)))
		return FALSE;

	if (!ReplayWrite((UByte*)&temp,sizeof(UWord)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreRealFrameCounts
//Author		Andy McMaster
//Date			Thu 17 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreRealFrameCounts(UWord num, UWord start, UWord end)
{
// rewind to correct place and store num of frames etc

	SLong jump=num*sizeof(REPLAYPACKET);

//	jump+=(sizeof(ULong)*2);	// sizeof of header len
//	jump+=(sizeof(UWord)*2);	//sizeof frame counts
	jump+=sizeof(ULong);
	jump+=sizeof(UWord)*3;
	jump=-jump;

	SetFilePointer(recordstorage,jump,NULL,FILE_CURRENT);

	if (!ReplayWrite((UByte*)&num,sizeof(UWord)))
		return FALSE;

	if (!ReplayWrite((UByte*)&start,sizeof(UWord)))
		return FALSE;

	if (!ReplayWrite((UByte*)&end,sizeof(UWord)))
		return FALSE;

	SetFilePointer(recordstorage,0,NULL,FILE_END);

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadFrameCounts
//Author		Andy McMaster
//Date			Thu 17 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadFrameCounts()
{
	if (!ReplayRead((UByte*)&thisblocknumframes,sizeof(UWord)))
		return FALSE;

	if (!ReplayRead((UByte*)&thisblockstartframe,sizeof(UWord)))
		return FALSE;

	if (!ReplayRead((UByte*)&thisblockendframe,sizeof(UWord)))
		return FALSE;

	if (thisblocknumframes==0)
		emptyblock=true;
	else
		emptyblock=false;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StorePrimaryASData
//Author		Andy McMaster
//Date			Mon 28 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StorePrimaryASData(AirStrucPtr ac) //, LPASPRIMARYVALUES aspv)
{
	ASPRIMARYVALUES aspv;

	GetPrimaryASData(ac,&aspv);

	if (!ReplayWrite((UByte*)&aspv,sizeof(ASPRIMARYVALUES)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetPrimaryASData
//Author		Andy McMaster
//Date			Tue 10 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::GetPrimaryASData(AirStrucPtr ac, LPASPRIMARYVALUES aspv)
{
	UniqueID ID;
	AirStrucPtr	tempac;

//worldinc.h

// for ai stuff take the ghosts data

	if (ac==Persons2::PlayerSeenAC)
	{
		tempac=Persons2::PlayerSeenAC;
		ac=Persons2::PlayerGhostAC;
	}
	else
	{
		tempac=ac;
	}

	aspv->playermoving=AirStruc::playermoving;					//DAW 22Jun99

// backup seen AI info into ghost info later

	if (ac->ai.homebase)
		ID=ac->ai.homebase->uniqueID.bitfieldUid.count;
	else
		ID=UID_Null;

	aspv->homebase=ID;

	if (ac->ai.unfriendly)
		ID=ac->ai.unfriendly->uniqueID.bitfieldUid.count;
	else
		ID=UID_Null;

	aspv->unfriendly=ID;

	if (ac->ai.attacker)
		ID=ac->ai.attacker->uniqueID.bitfieldUid.count;
	else
		ID=UID_Null;

	aspv->attacker=ID;

	aspv->manoeuvre=ac->ai.PilotSkills.manoeuvre;
	aspv->ManStep=ac->ai.PilotSkills.ManStep;
	aspv->SimpleACMFlags.simpleacm=ac->ai.PilotSkills.simpleacm;
	aspv->SimpleACMFlags.flatturn=ac->ai.PilotSkills.flatturn;
	aspv->morale=ac->ai.morale;
	aspv->moraleinc=ac->ai.moraleinc;
	aspv->lastdelta=ac->ai.lastdelta;
	aspv->desiredroll=ac->ai.desiredroll;
	aspv->desiredpitch=ac->ai.desiredpitch;
	aspv->desiredaoa=ac->ai.desiredaoa;
	aspv->desiredhdg=ac->ai.desiredhdg;
	aspv->desiredturn=ac->ai.desiredturn;
	aspv->desiredpitchrate=ac->ai.desiredpitchrate;
	aspv->desiredrange=ac->ai.desiredrange;
	aspv->desiredalt=ac->ai.desiredalt;
	aspv->deltapitchsum=ac->ai.deltapitchsum;

	ac=tempac;

	UpdateSeenAIData();

// this contains fuel levels

	aspv->rreloadmass=(ac->weap.right.reloadmass&0xffff00)>>0x8;
	aspv->creloadmass=(ac->weap.centre.reloadmass&0xffff00)>>0x8;
	aspv->lreloadmass=(ac->weap.left.reloadmass&0xffff00)>>0x8;

	aspv->rreloaddrag=(ac->weap.right.reloaddrag&0xffff00)>>0x8;
	aspv->creloaddrag=(ac->weap.centre.reloaddrag&0xffff00)>>0x8;
	aspv->lreloaddrag=(ac->weap.left.reloaddrag&0xffff00)>>0x8;

	aspv->rmass=(ac->weap.right.mass&0xffff00)>>0x8;
	aspv->cmass=(ac->weap.centre.mass&0xffff00)>>0x8;
	aspv->lmass=(ac->weap.left.mass&0xffff00)>>0x8;

	aspv->rdrag=(ac->weap.right.drag&0xffff00)>>0x8;
	aspv->cdrag=(ac->weap.centre.drag&0xffff00)>>0x8;
	aspv->ldrag=(ac->weap.left.drag&0xffff00)>>0x8;

	aspv->rint_fuel=ac->weap.right.int_fuel;					//AMM 13Jul99
	aspv->cint_fuel=ac->weap.centre.int_fuel;					//AMM 13Jul99
	aspv->lint_fuel=ac->weap.left.int_fuel;						//AMM 13Jul99

	aspv->rext_fuel=ac->weap.right.ext_fuel;
	aspv->cext_fuel=ac->weap.centre.ext_fuel;
	aspv->lext_fuel=ac->weap.left.ext_fuel;

	aspv->rint_fuel_leakage=ac->weap.right.int_fuel_leakage;	//AMM 13Jul99
	aspv->cint_fuel_leakage=ac->weap.centre.int_fuel_leakage;	//AMM 13Jul99
	aspv->lint_fuel_leakage=ac->weap.left.int_fuel_leakage;		//AMM 13Jul99

	aspv->rext_fuel_leakage=ac->weap.right.ext_fuel_leakage;
	aspv->cext_fuel_leakage=ac->weap.centre.ext_fuel_leakage;
	aspv->lext_fuel_leakage=ac->weap.left.ext_fuel_leakage;

	aspv->shpweapindex=ac->weap.shpweapindex;
	aspv->ShootDelay=ac->weap.ShootDelay;
	aspv->SimpleACMFlags.UseIntFuel=ac->weap.AircraftStatus.UseIntFuel;
	aspv->SimpleACMFlags.FuelDumped=ac->weap.AircraftStatus.FuelDumped;
	aspv->weapforce=ac->weap.weapforce;

	aspv->EmergencyFlags.StoresDumped=ac->weap.AircraftStatus.StoresDumped;
	aspv->EmergencyFlags.Ejected=ac->weap.AircraftStatus.Ejected;

	aspv->originalformpos=ac->fly.originalformpos;

	if (ac->fly.nextflight)
		ID=ac->fly.nextflight->uniqueID.bitfieldUid.count;
	else
		ID=UID_Null;

	aspv->nextflight=ID;

	if (ac->fly.leadflight)
		ID=ac->fly.leadflight->uniqueID.bitfieldUid.count;
	else
		ID=UID_Null;

	aspv->leadflight=ID;
	aspv->rpm=ac->fly.rpm;
	aspv->thrustpercent=ac->fly.thrustpercent;

	if (ac->fly.backupWP)
		ID=ac->fly.backupWP->uniqueID.bitfieldUid.count;
	else
		ID=UID_Null;

	aspv->backupWP=ID;
	aspv->gforce=ac->fly.gforce;
	aspv->aileron=ac->fly.aileron;
	aspv->elevator=ac->fly.elevator;
	aspv->rudder=ac->fly.rudder;
	aspv->dhdg=ac->fly.dhdg;
	aspv->lasthitter=ac->lasthitter;

	aspv->Weapons=ac->weap.AircraftStatus.Weapons;

	aspv->formpos=ac->formpos;
	aspv->formation=ac->FormationInfo.formation;
	aspv->SimpleACMFlags.information=ac->FormationInfo.information;
	aspv->SimpleACMFlags.slowdownleader=ac->FormationInfo.slowdownleader;
	aspv->duty=ac->FormationInfo.duty;
	aspv->SimpleACMFlags.advantage=ac->FormationInfo.advantage;
	aspv->SimpleACMFlags.disadvantage=ac->FormationInfo.disadvantage;
	aspv->manoeuvretime=ac->manoeuvretime;

	SetVelsAndOldVals(ac);
}

//------------------------------------------------------------------------------
//Procedure		UpdateSeenAIData
//Author		Andy McMaster
//Date			Wed 6 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::UpdateSeenAIData()
{
	memcpy(&Persons2::PlayerSeenAC->ai,&Persons2::PlayerGhostAC->ai,sizeof(ai_info));
}

//------------------------------------------------------------------------------
//Procedure		StorePrimaryMIData
//Author		Andy McMaster
//Date			Tue 29 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StorePrimaryMIData(MobileItemPtr ac) //, LPMIPRIMARYVALUES mipv)
{
	MIPRIMARYVALUES mipv;
	
	GetPrimaryMIData(ac, &mipv);

	if (!ReplayWrite((UByte*)&mipv,sizeof(MIPRIMARYVALUES)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetPrimaryMIData
//Author		Andy McMaster
//Date			Tue 10 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::GetPrimaryMIData(MobileItemPtr ac, LPMIPRIMARYVALUES mipv)
{
	UniqueID ID;
	MobileItemPtr seenac;
	MobileItemPtr ghostac;

	if (ac==Persons2::PlayerSeenAC)
	{
		ghostac=(MobileItemPtr)Persons2::PlayerGhostAC;
		seenac=(MobileItemPtr)Persons2::PlayerSeenAC;
	}
	else
	{
		seenac=ac;
		ghostac=ac;
	}
	mipv->pos=ghostac->World;

	mipv->MigStatus=seenac->MigStatus;

	mipv->hdg.a=ghostac->hdg.a;
	mipv->pitch.a=ghostac->pitch.a;
	mipv->roll.a=ghostac->roll.a;

	mipv->vel=ghostac->vel;

	if (seenac->waypoint)
		ID=seenac->waypoint->uniqueID.bitfieldUid.count;
	else
		ID=UID_Null;

	mipv->waypoint=ID;
	mipv->movecode=seenac->AutoMoveInfo.movecode;

	mipv->shape=seenac->shape;

	if (seenac->MigStatus.LaunchFlags.size>=FORMATIONSIZE)						//AMM 05Jul99
	{															//AMM 05Jul99
		if (((FormationItemPtr)(seenac))->leader)				//AMM 05Jul99
			ID=((FormationItemPtr)(seenac))->leader->uniqueID.bitfieldUid.count;//AMM 05Jul99
		else													//AMM 05Jul99
			ID=UID_Null;										//AMM 05Jul99
	}															//AMM 05Jul99

	mipv->leader=ID;											//AMM 05Jul99

	if (seenac->MigStatus.LaunchFlags.size>=FORMATIONSIZE)						//AMM 05Jul99
	{															//AMM 05Jul99
		if (((FormationItemPtr)(seenac))->follower)				//AMM 05Jul99
			ID=((FormationItemPtr)(seenac))->follower->uniqueID.bitfieldUid.count;//AMM 05Jul99
		else													//AMM 05Jul99
			ID=UID_Null;										//AMM 05Jul99
	}															//AMM 05Jul99

	mipv->follower=ID;											//AMM 05Jul99
}

//------------------------------------------------------------------------------
//Procedure		RestorePrimaryASValues
//Author		Andy McMaster
//Date			Mon 28 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RestorePrimaryASValues(AirStrucPtr ac, LPASPRIMARYVALUES aspv)
{
	AirStrucPtr tempac;

	if (ac==Persons2::PlayerSeenAC)
	{
		tempac=Persons2::PlayerSeenAC;
		ac=Persons2::PlayerGhostAC;
	}
	else
	{
		tempac=ac;
	}

	if (aspv->playermoving)										//DAW 22Jun99
		AirStruc::playermoving=true;							//DAW 22Jun99

 	ac->ai.homebase=(itemptr)Persons2::ConvertPtrUID(aspv->homebase);

 	ac->ai.unfriendly=(itemptr)Persons2::ConvertPtrUID(aspv->unfriendly);

	if (ac->ai.unfriendly==Persons2::PlayerSeenAC)
		ac->ai.unfriendly=Persons2::PlayerGhostAC;

	ac->ai.attacker=(AirStrucPtr)Persons2::ConvertPtrUID(aspv->attacker);

	if (ac->ai.attacker==Persons2::PlayerSeenAC)
		ac->ai.attacker=Persons2::PlayerGhostAC;

	ac->ai.PilotSkills.manoeuvre=aspv->manoeuvre;
	ac->ai.PilotSkills.ManStep=aspv->ManStep;
	ac->ai.PilotSkills.simpleacm=aspv->SimpleACMFlags.simpleacm;
	ac->ai.PilotSkills.flatturn=aspv->SimpleACMFlags.flatturn;
	ac->ai.morale=aspv->morale;
	ac->ai.moraleinc=aspv->moraleinc;
	ac->ai.lastdelta=aspv->lastdelta;
	ac->ai.desiredroll=aspv->desiredroll;
	ac->ai.desiredpitch=aspv->desiredpitch;
	ac->ai.desiredaoa=aspv->desiredaoa;
	ac->ai.desiredhdg=aspv->desiredhdg;
	ac->ai.desiredturn=aspv->desiredturn;
	ac->ai.desiredpitchrate=aspv->desiredpitchrate;
	ac->ai.desiredrange=aspv->desiredrange;
	ac->ai.desiredalt=aspv->desiredalt;
	ac->ai.deltapitchsum=aspv->deltapitchsum;

	UpdateSeenAIData();

	ac=tempac;

	ac->weap.right.reloadmass=(aspv->rreloadmass<<0x8);
	ac->weap.centre.reloadmass=(aspv->creloadmass<<0x8);
	ac->weap.left.reloadmass=(aspv->lreloadmass<<0x8);

	ac->weap.right.reloaddrag=(aspv->rreloaddrag<<0x8);
	ac->weap.centre.reloaddrag=(aspv->creloaddrag<<0x8);
	ac->weap.left.reloaddrag=(aspv->lreloaddrag<<0x8);

	ac->weap.right.mass=(aspv->rmass<<0x8);
	ac->weap.centre.mass=(aspv->cmass<<0x8);
	ac->weap.left.mass=(aspv->lmass<<0x8);

	ac->weap.right.drag=(aspv->rdrag<<0x8);
	ac->weap.centre.drag=(aspv->cdrag<<0x8);
	ac->weap.left.drag=aspv->ldrag<<0x8;

	ac->weap.right.int_fuel=aspv->rint_fuel;						//AMM 13Jul99
	ac->weap.centre.int_fuel=aspv->cint_fuel;					//AMM 13Jul99
	ac->weap.left.int_fuel=aspv->lint_fuel;						//AMM 13Jul99

	ac->weap.right.ext_fuel=aspv->rext_fuel;
	ac->weap.centre.ext_fuel=aspv->cext_fuel;
	ac->weap.left.ext_fuel=aspv->lext_fuel;

	ac->weap.right.int_fuel_leakage=aspv->rint_fuel_leakage;	//AMM 13Jul99
	ac->weap.centre.int_fuel_leakage=aspv->cint_fuel_leakage;	//AMM 13Jul99
	ac->weap.left.int_fuel_leakage=aspv->lint_fuel_leakage;		//AMM 13Jul99

	ac->weap.right.ext_fuel_leakage=aspv->rext_fuel_leakage;
	ac->weap.centre.ext_fuel_leakage=aspv->cext_fuel_leakage;
	ac->weap.left.ext_fuel_leakage=aspv->lext_fuel_leakage;

	ac->weap.shpweapindex=aspv->shpweapindex;
	ac->weap.ShootDelay=aspv->ShootDelay;
	ac->weap.AircraftStatus.UseIntFuel=aspv->SimpleACMFlags.UseIntFuel;
	ac->weap.AircraftStatus.FuelDumped=aspv->SimpleACMFlags.FuelDumped;
	ac->weap.weapforce=aspv->weapforce;

	ac->weap.AircraftStatus.StoresDumped=aspv->EmergencyFlags.StoresDumped;
	ac->weap.AircraftStatus.Ejected=aspv->EmergencyFlags.Ejected;

	ac->fly.originalformpos=aspv->originalformpos;

	ac->fly.nextflight=(AirStrucPtr)Persons2::ConvertPtrUID(aspv->nextflight);

	if (ac->fly.nextflight==Persons2::PlayerSeenAC)
		ac->fly.nextflight=Persons2::PlayerGhostAC;

	ac->fly.leadflight=(AirStrucPtr)Persons2::ConvertPtrUID(aspv->leadflight);

	if (ac->fly.leadflight==Persons2::PlayerSeenAC)
		ac->fly.leadflight=Persons2::PlayerGhostAC;

	ac->fly.rpm=aspv->rpm;
	ac->fly.thrustpercent=aspv->thrustpercent;
	ac->fly.backupWP=(WayPointPtr)Persons2::ConvertPtrUID(aspv->backupWP);
	ac->fly.gforce=aspv->gforce;
	ac->fly.aileron=aspv->aileron;
	ac->fly.elevator=aspv->elevator;
	ac->fly.rudder=aspv->rudder;
	ac->fly.dhdg=aspv->dhdg;
	ac->lasthitter=aspv->lasthitter;
	ac->weap.AircraftStatus.Weapons=aspv->Weapons;
	ac->formpos=aspv->formpos;

	ac->FormationInfo.formation=aspv->formation;
	ac->FormationInfo.information=aspv->SimpleACMFlags.information;
	ac->FormationInfo.slowdownleader=aspv->SimpleACMFlags.slowdownleader;
	ac->FormationInfo.duty=aspv->duty;
	ac->FormationInfo.advantage=aspv->SimpleACMFlags.advantage;
	ac->FormationInfo.disadvantage=aspv->SimpleACMFlags.disadvantage;
	ac->manoeuvretime=aspv->manoeuvretime;

	SetVelsAndOldVals(ac);
}


//------------------------------------------------------------------------------
//Procedure		RestorePrimaryASValues
//Author		Andy McMaster
//Date			Tue 29 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RestorePrimaryMIValues(MobileItemPtr ac, LPMIPRIMARYVALUES mipv)
{

	ac->World=mipv->pos;

	ac->MigStatus=mipv->MigStatus;

	ac->hdg.a=mipv->hdg.a;
	ac->pitch.a=mipv->pitch.a;
	ac->roll.a=mipv->roll.a;
	ac->vel=mipv->vel;

	ac->waypoint=(WayPointPtr)Persons2::ConvertPtrUID(mipv->waypoint);

	ac->AutoMoveInfo.movecode=mipv->movecode;

	ac->shape=mipv->shape;

	if (ac->MigStatus.LaunchFlags.size>=FORMATIONSIZE)							//AMM 05Jul99
	{															//AMM 05Jul99
		FormationItemPtr fip=(FormationItemPtr)ac;				//AMM 05Jul99

		fip->leader=(FormationItemPtr)Persons2::ConvertPtrUID(mipv->leader);//AMM 05Jul99

		if (fip->leader==Persons2::PlayerSeenAC)				//AMM 05Jul99
			fip->leader=Persons2::PlayerGhostAC;				//AMM 05Jul99
	}															//AMM 05Jul99

	if (ac->MigStatus.LaunchFlags.size>=FORMATIONSIZE)							//AMM 05Jul99
	{
		FormationItemPtr fip=(FormationItemPtr)ac;				//AMM 05Jul99

		fip->follower=(FormationItemPtr)Persons2::ConvertPtrUID(mipv->follower);//AMM 05Jul99

		if (fip->follower==Persons2::PlayerSeenAC)				//AMM 05Jul99
			fip->follower=Persons2::PlayerGhostAC;				//AMM 05Jul99
	}															//AMM 05Jul99

	if (ac==Persons2::PlayerSeenAC)
	{
		AirStrucPtr gac=Persons2::PlayerGhostAC;

		gac->MigStatus=ac->MigStatus; //AMM 21/07/99

		gac->World.X=ac->World.X;
		gac->World.Y=ac->World.Y;
		gac->World.Z=ac->World.Z;

		gac->hdg.a=ac->hdg.a;
		gac->pitch.a=ac->pitch.a;
		gac->roll.a=ac->roll.a;

		gac->vel=ac->vel;

		gac->AutoMoveInfo.movecode=mipv->movecode;
		gac->waypoint=(WayPointPtr)Persons2::ConvertPtrUID(mipv->waypoint);
	}
}

//------------------------------------------------------------------------------
//Procedure		LoadGamePrefs
//Author		Andy McMaster
//Date			Thu 1 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadGamePrefs()	
{
	//THIS SHOULD BE AS BELOW, BUT I NEED TO MAINTAIN BACKWARD COMPATIBILITY. THANKS MACCA!:
	//if (!ReplayRead((UByte*)&Save_Data.flightdifficulty,sizeof(FlightDifficultyField)))
	//	return FALSE;
	//if (!ReplayRead((UByte*)&Save_Data.gamedifficulty,sizeof(GameDifficultyField)))
	//	return FALSE;

	char	sparebuffer[sizeof(int)*GD_MAX];
	if (!ReplayRead((UByte*)&sparebuffer,sizeof(int)*FD_MAX))
		return FALSE;
	for (int i=0;i<FlightDifficultyField::BYTES;i++)
		Save_Data.flightdifficulty.FuckingDirtyAccess()[i]=sparebuffer[i];
	if (!ReplayRead((UByte*)&sparebuffer,sizeof(int)*GD_MAX))
		return FALSE;
	for (int j=0;j<GameDifficultyField::BYTES;j++)
		Save_Data.gamedifficulty.FuckingDirtyAccess()[j]=sparebuffer[j];

	if (!ReplayRead((UByte*)&Save_Data.autopilotskillUN,sizeof(SWord)))
		return FALSE;

	if (!ReplayRead((UByte*)&Save_Data.autopilotskillRED,sizeof(SWord)))
		return FALSE;

	if (!ReplayRead((UByte*)&Save_Data.targetsize,sizeof(TargetSize)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreGamePrefs
//Author		Andy McMaster
//Date			Thu 1 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreGamePrefs()
{
	//THIS SHOULD BE AS BELOW, BUT I NEED TO MAINTAIN BACKWARD COMPATIBILITY. THANKS MACCA!:
	//if (!ReplayRead((UByte*)&Save_Data.flightdifficulty,sizeof(FlightDifficultyField)))
	//	return FALSE;
	//if (!ReplayRead((UByte*)&Save_Data.gamedifficulty,sizeof(GameDifficultyField)))
	//	return FALSE;

	if (!ReplayWrite((UByte*)&Save_Data.flightdifficulty,sizeof(int)*FD_MAX))
		return FALSE;

	if (!ReplayWrite((UByte*)&Save_Data.gamedifficulty,sizeof(int)*GD_MAX))
		return FALSE;

	if (!ReplayWrite((UByte*)&Save_Data.autopilotskillUN,sizeof(SWord)))
		return FALSE;

	if (!ReplayWrite((UByte*)&Save_Data.autopilotskillRED,sizeof(SWord)))
		return FALSE;

	if (!ReplayWrite((UByte*)&Save_Data.targetsize,sizeof(TargetSize)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreFinalRecordData
//Author		Andy McMaster
//Date			Thu 1 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::StoreFinalRecordData()
{
	Record=FALSE;

	StoreSuperHeaderEnd();
		
	if (StoreBlockHeader())
		Record=TRUE;

	if (Record)
		badrecord=false;
}

//------------------------------------------------------------------------------
//Procedure		LoadFinalPlaybackData
//Author		Andy McMaster
//Date			Thu 1 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::LoadFinalPlaybackData()
{
	Playback=FALSE;

	LoadSuperHeaderEnd();

	BackupPrefs();

	PreScanReplayFile();										//AMM 18Feb99

	if (LoadBlockHeader())
		Playback=TRUE;

	if (!Playback)
		_Error.EmitSysErr("Error reading playback log");

	Manual_Pilot.controlmode=ManualPilot::AUTO;
}

//------------------------------------------------------------------------------
//Procedure		ResetAICount
//Author		Andy McMaster
//Date			Mon 5 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::ResetAICount()
{
	Art_Int.ACArrayInd=0;
	Art_Int.gunstimer=0;										//DAW 28Jun99
}

//------------------------------------------------------------------------------
//Procedure		RemoveAllTransients
//Author		Andy McMaster
//Date			Tue 6 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RemoveAllTransients(Bool	totalremove)
{
	TransientItemPtr	temp=TransientItem::TransientList;

	while (temp)
	{
		temp->isOwned = 0;										//DAW 21Jun99
		temp->isArmed = 0;										//DAW 21Jun99
		Trans_Obj.AddTransientItemToDeadList(temp,true);		//DAW 21Jun99

		temp=temp->nexttrans;
	}

	if (totalremove)											//RJS 21Jun99
	{
		SHAPE.GenerateProbeTrails();								
		SHAPE.KillVapourStreamDeadList();							

		Trans_Obj.RemoveDeadListFromWorld();
	}
}

//------------------------------------------------------------------------------
//Procedure		StopRecord
//Author		Andy McMaster
//Date			Wed 7 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::StopRecord()
{
	if (Record)
	{
		ClearReplayBuffers();
		StoreRealFrameCounts(replayframecount,0,replayframecount-1);
		Record=FALSE;
		CloseRecordLog();
	}
}

//------------------------------------------------------------------------------
//Procedure		GetReplayFileName
//Author		Andy McMaster
//Date			Wed 7 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::GetReplayFileName()
{
	if (Record)
	{
		strcpy(rfilename,"replay.dat");

		GetReplayFilename(rfilename);
	}
	else
		GetReplayFilename(pfilename);
}

//------------------------------------------------------------------------------
//Procedure		StoreSmkData
//Author		Andy McMaster
//Date			Wed 4 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreSmkData(LPREPLAYSMOKE smokey,UByteP pointlist)
{
// store each smoke trail in turn
	// Down sample smoke trail...and save...
	ReplaySmkPoint	SmokeList[12];
	Coords3DList*	apoint = (Coords3DList*) pointlist;
	Coords3DList*	npoint;
	SLong			index = 0;
	SLong			divvy = smokey->nopoints / 5;
	if (divvy)
	{
		SLong	howmany = divvy-1;
//		SLong	howmany = (smokey->nopoints/divvy)-1;
		SLong	counter = 0;

		while (apoint)
		{
			npoint = apoint;
			apoint = apoint->next;

			if (!apoint || !counter)
			{
				// Save this one...
				counter = howmany;

				if (!npoint->KillMe)
				{
					SmokeList[index].x = npoint->trailpoint.gx;
					SmokeList[index].y = npoint->trailpoint.gy;
					SmokeList[index].z = npoint->trailpoint.gz;
					SmokeList[index].lifeleft = npoint->lifeleft;
					SmokeList[index].radius = npoint->radius;
					SmokeList[index].LifeTime = npoint->LifeTime;
					SmokeList[index].FadeTime = npoint->FadeTime;
					SmokeList[index].KillMe = npoint->KillMe;
				}
				index++;
			}
			else
				counter--;
		}
	}
	else
	{
		while (apoint)
		{
			//Save...
			SmokeList[index].x = apoint->trailpoint.gx;
			SmokeList[index].y = apoint->trailpoint.gy;
			SmokeList[index].z = apoint->trailpoint.gz;
			SmokeList[index].lifeleft = apoint->lifeleft;
			SmokeList[index].radius = apoint->radius;
			SmokeList[index].LifeTime = apoint->LifeTime;
			SmokeList[index].FadeTime = apoint->FadeTime;
			SmokeList[index].KillMe = apoint->KillMe;
			index++;

			apoint = apoint->next;
		}
	}

	smokey->nopoints = index;
	if (!ReplayWrite((UByte*)smokey,sizeof(REPLAYSMOKE)))
		return FALSE;

	// Write it out backwards....so I can read back in the right order...
	while (index > 0)
	{
		index--;

		if (!ReplayWrite((UByteP)&SmokeList[index],sizeof(ReplaySmkPoint)))
			return FALSE;
	}

	return  TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadSmkData
//Author		Andy McMaster
//Date			Wed 4 Nov 1998
//
//Description	//Need to cater for dummy smoke trails too!!
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadSmkData(LPREPLAYSMOKE smokey, Bool dummy, UByteP	&TrailListPtr)
{
	if (!SmokeRead(smokey,sizeof(REPLAYSMOKE)))
		return FALSE;

	Coords3DList*	apoint;
	Coords3DList*	baseptr = NULL;
	ReplaySmkPoint	smkpoint;
	int	i;
	for (i = 0; i < smokey->nopoints; i++)
	{
		if (!SmokeRead((UByte*)&smkpoint,sizeof(ReplaySmkPoint)))
			return FALSE;

		if (!dummy)
		{
			apoint = new Coords3DList;
			if (apoint)
			{
				apoint->trailpoint.gx = smkpoint.x;
				apoint->trailpoint.gy = smkpoint.y;
				apoint->trailpoint.gz = smkpoint.z;
				apoint->lifeleft = smkpoint.lifeleft;
				apoint->radius = smkpoint.radius;
				apoint->LifeTime = smkpoint.LifeTime;
				apoint->FadeTime = smkpoint.FadeTime;
				apoint->KillMe = smkpoint.KillMe;
				apoint->next = baseptr;

				baseptr = apoint;
			}
		}
	}

	TrailListPtr = (UByteP)baseptr;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreSmkIndexData
//Author		Andy McMaster
//Date			Wed 4 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreSmkIndexData(ULong index)
{
	if (!ReplayWrite((UByte*)&index,sizeof(ULong)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadSmkIndexData
//Author		Andy McMaster
//Date			Wed 4 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadSmkIndexData(ULong& index)
{
	if (!SmokeRead(&index,sizeof(ULong)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		MarkBlockBeginning
//Author		Andy McMaster
//Date			Fri 6 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::MarkBlockBeginning()
{
	ULong tempb=currblock-1;
	UWord tempf=replayframecount;

	if (tempb==beginmark.blocknum && tempf==beginmark.framenum)
	{
// if this is current marked position

		if (beginmark.blocknum!=0 || (beginmark.blocknum==0 && beginmark.framenum!=0))
		{
// and its not start of replay then remove mark

			RemoveBeginningMark();
		}
	}
	else
	{
// cant put begin mark at end of replay, that would be silly

		if (!(currblock-1==EndBlockNum && replayframecount==EndFrameNum))
		{
// if begin mark is after end mark, end mark becomes eof

			if (currblock-1>endmark.blocknum 
			|| (currblock-1==endmark.blocknum && replayframecount>endmark.framenum))
			{
				RemoveEndMark();
			}

			beginmark.blocknum=currblock-1;
			beginmark.framenum=replayframecount;
			beginmark.memaddress=headmems[0][currblock-1];
		}
	}

	CalcBeginMarkPos();
	CalcEndMarkPos();
}

//------------------------------------------------------------------------------
//Procedure		MarkBlockEnd
//Author		Andy McMaster
//Date			Fri 6 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::MarkBlockEnd()
{
	ULong tempb=currblock-1;
	UWord tempf=replayframecount;

	if (tempb==endmark.blocknum && tempf==endmark.framenum)
	{
// if this is current marked position

		if (endmark.blocknum!=EndBlockNum || (endmark.blocknum==EndBlockNum && endmark.framenum!=EndFrameNum))
		{
// and its not end of replay then remove mark

			RemoveEndMark();
		}
	}
	else
	{
// OY, I said NO, you cannot put an end mark at the beginning of replay!

		if (!(currblock-1==0 && replayframecount==BeginBlockFrame))
		{
// if end mark is before begin mark, remove begin mark

			if (currblock-1<beginmark.blocknum
			|| (currblock-1==beginmark.blocknum && replayframecount<beginmark.framenum))
			{
				RemoveBeginningMark();
			}

			endmark.blocknum=currblock-1;
			endmark.framenum=replayframecount;
			endmark.memaddress=headmems[1][currblock-1];
		}
	}
	CalcBeginMarkPos();
	CalcEndMarkPos();
}

//------------------------------------------------------------------------------
//Procedure		RemoveMarks
//Author		Andy McMaster
//Date			Wed 11 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RemoveMarks()
{
	RemoveBeginningMark();
	RemoveEndMark();

	CalcBeginMarkPos();
	CalcEndMarkPos();
}

//------------------------------------------------------------------------------
//Procedure		SaveBlock
//Author		Andy McMaster
//Date			Fri 6 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::SaveBlock()
{
	HANDLE handy;
	char* newfile=new char[256];
	DWORD numtowrite,numwritten;
	DWORD totaltowrite=0,totalwritten=0;
	Bool	OK=FALSE;

	strcpy (newfile,"tblock.dat");
	GetReplayFilename(newfile);

// save block

	handy=CreateFile(
		newfile,
		GENERIC_WRITE|GENERIC_READ,
		FILE_SHARE_WRITE|FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,	// overwrite any existing file 
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (handy!=INVALID_HANDLE_VALUE)
	{
// need to save superheader! this is from beginning of recordfile to start
// of first block, which we have memory address for - phew!

		UByteP superstart,superend;
		ULong supersize;

		superstart=(UByteP)playbackfilestart;
		superend=headmems[0][0];

		supersize=superend-superstart;

		WriteFile(
			handy,
			superstart,
			supersize,
			&numwritten,
			NULL);

		totaltowrite=supersize;
		totalwritten=numwritten;

// need to alter start and end frames in block headers for this section and then restore
// after saving
// cant alter file, so copy file up to address to alter, write out new value, copy up till
// next alterable address, write value, then copy rest of file
// 5 sections
// 1st: copy file up to address that needs altering
// 2nd: write out new begin frame value
// 3rd: copy file from address after begin frame to address of new last frame value
// 4th: write out new last frame value
// 5th: copy file up to end address.

// get addresses that need altering

		UByteP here,start,end,src;

// end of block - headlength(ULong) - endframenum (UWord) - beginframenum (UWord)

		here=beginmark.memaddress+(headlength[beginmark.blocknum]-(sizeof(ULong)+(2*sizeof(UWord))));
		start=here;

// end of block - headlength(ULong) - endframenum (UWord)

		here=endmark.memaddress-((sizeof(ULong)+sizeof(UWord))+(numframes[endmark.blocknum]*sizeof(REPLAYPACKET)));
		end=here;

// 1st section

		src=beginmark.memaddress;
		numtowrite=start-beginmark.memaddress;

		WriteFile(
			handy,
			src,
			numtowrite,
			&numwritten,
			NULL);

		totaltowrite+=numtowrite;
		totalwritten+=numwritten;

// 2nd section

		src=(UByteP)&beginmark.framenum;
		numtowrite=sizeof(UWord);

		WriteFile(
			handy,
			src,
			numtowrite,
			&numwritten,
			NULL);

		totaltowrite+=numtowrite;
		totalwritten+=numwritten;

// 3rd section

		src=start+sizeof(UWord);
		numtowrite=end-src;

		WriteFile(
			handy,
			src,
			numtowrite,
			&numwritten,
			NULL);

		totaltowrite+=numtowrite;
		totalwritten+=numwritten;

// 4th section

		src=(UByteP)&endmark.framenum;
		numtowrite=sizeof(UWord);

		WriteFile(
			handy,
			src,
			numtowrite,
			&numwritten,
			NULL);

		totaltowrite+=numtowrite;
		totalwritten+=numwritten;

// 5th section

		src=end+sizeof(UWord);;
		numtowrite=endmark.memaddress-src;

		WriteFile(
			handy,
			src,
			numtowrite,
			&numwritten,
			NULL);

		CloseHandle(handy);

		totaltowrite+=numtowrite;
		totalwritten+=numwritten;

		if (totalwritten!=totaltowrite)
		{
			DeleteFile(newfile);
			_DPlay.CommsMessage(IDS_NOTWRITE);
		}
		else
		{
			OK=TRUE;
		}
	}

	delete [] newfile;

	if (OK)
		return TRUE;
	else
		return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		ResetFlags
//Author		Andy McMaster
//Date			Wed 11 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::ResetFlags()
{
	ReplayFlag=RF_NONE;
	framestoskip=0;
}

//------------------------------------------------------------------------------
//Procedure		DummyCrashSkid
//Author		Robert Slater
//Date			Wed 28 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::DummyCrashSkid()
{
	AirStrucPtr			sac=Persons2::PlayerSeenAC;
	Bool				onWater = FALSE;
	AreaType			theArea;
	UByte				slowshift = 0;
	AircraftAnimData	*adptr = (AircraftAnimData*) sac->Anim;
	SLong				groundlevel = Land_Scape.GetGroundLevel(sac);
	WorldStuff &currworld = mobileitem::GetWorld();

	theArea = (AreaType)Three_Dee.GetLandType();
	if ((theArea == AT_sea) || (theArea == AT_river) || (theArea == AT_washShore))
	{
		onWater = TRUE;
		slowshift = 2;
	}

	if (sac->vel <= 0)
	{
		_Miles.StopEngine();						

		SHAPE.DetatchAllVapourStreams(sac,0);
		adptr->hasdust = FALSE;

		if (onWater)
		{
			ShapeDescPtr	sdptr = SHAPESTUFF.GetShapePtr(sac->shape);
			SLong			size = sdptr->Size << 4;
			SLong			dist = sac->World.Y - groundlevel;

			//Sink...
			if (dist > -size)
			{
				if (dist < 0)
				{
					dist = size + dist;
				}
				else
					dist = size;

				Trans_Obj.LaunchBubbles(sac,dist,currworld);
			}
			else
				ReplayDeathMode = RDEATH_NULL;
		}
		else
		{
			// The end...
			ReplayDeathMode = RDEATH_NULL;
		}
	}
	else
	{
		if (onWater)
		{
			// Give it 4 seconds to knacker totally...
			UWord	damage = (BS_DEAD * sac->fly.pModel->CrashTime)/400;

			if (damage > BS_DEAD) damage = BS_DEAD;

			if (Math_Lib.rnd() > 32767)
			{
				if ((adptr->LEFTWINGIN < BS_DAMLV1) && (damage >= BS_DAMLV1))
					SHAPE.ForceDamage(sac,sac,&adptr->LEFTWINGIN,BS_DAMLV1);
				else
				{
					if ((adptr->LEFTWINGIN < BS_DAMLV2) && (damage >= BS_DAMLV2))
						SHAPE.ForceDamage(sac,sac,&adptr->LEFTWINGIN,BS_DAMLV2);
					else
					{
						if ((adptr->LEFTWINGIN < BS_DEAD) && (damage >= BS_DEAD))
							SHAPE.ForceDamage(sac,sac,&adptr->LEFTWINGIN,BS_DEAD);
					}
				}
			}
			else
			{
				if ((adptr->RIGHTWINGIN < BS_DAMLV1) && (damage >= BS_DAMLV1))
					SHAPE.ForceDamage(sac,sac,&adptr->RIGHTWINGIN,BS_DAMLV1);
				else
				{
					if ((adptr->RIGHTWINGIN < BS_DAMLV2) && (damage >= BS_DAMLV2))
						SHAPE.ForceDamage(sac,sac,&adptr->RIGHTWINGIN,BS_DAMLV2);
					else
					{
						if ((adptr->RIGHTWINGIN < BS_DEAD) && (damage >= BS_DEAD))
							SHAPE.ForceDamage(sac,sac,&adptr->RIGHTWINGIN,BS_DEAD);
					}
				}
			}

//			_Miles.PlayLooped(FIL_SFX_WATER_LOOP,sac);
		}

//		_Miles.PlayLanded(FIL_SFX_LANDING_GRASS_LOOP,NULL,sac->vel);
//			_Miles.PlayEngine(classtype,fly.rpm,vel);	
//		}
	}

	sac->fly.pModel->CrashTime += Timer_Code.FRAMETIME;
}

//------------------------------------------------------------------------------
//Procedure		DummyCrashRoll
//Author		Robert Slater
//Date			Wed 28 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::DummyCrashRoll()
{
	AirStrucPtr			sac=Persons2::PlayerSeenAC;
	Bool				onWater = FALSE;
	AreaType			theArea;
	UByte				slowshift;
	AircraftAnimData	*adptr = (AircraftAnimData*) sac->Anim;
	Bool				rolldone, pitchdone;
	SLong				groundlevel = Land_Scape.GetGroundLevel(sac);
	WorldStuff &currworld = mobileitem::GetWorld();


	slowshift = 4;
	theArea = (AreaType)Three_Dee.GetLandType();
	if ((theArea == AT_sea) || (theArea == AT_river) || (theArea == AT_washShore))
	{
		onWater = TRUE;
		slowshift += 6;
	}

	SWord	tmproll, tmppitch;
	SLong	crashtime = sac->fly.pModel->CrashTime;

	if (sac->vel > 0)
	{
		if (adptr->RIGHTWINGIN == BS_DEAD)						
		{
			tmproll = sac->roll;
			if (tmproll < -ANGLES_90Deg)
				SHAPE.ForceDamage(sac,sac,&adptr->LEFTWINGOUT,BS_DEAD);
		}
		else
		{
			if (adptr->LEFTWINGIN == BS_DEAD)						
			{
				tmproll = sac->roll;
				if (tmproll > ANGLES_90Deg)
					SHAPE.ForceDamage(sac,sac,&adptr->RIGHTWINGOUT,BS_DEAD);
			}
		}
	}
	else
	{
		SWord	maxangle = ANGLES_170Deg;
		bool	tipleft = false;

		_Miles.StopEngine();				

		SHAPE.DetatchAllVapourStreams(sac,0);			
		adptr->hasdust = FALSE;

		tmproll = sac->roll;
		tmppitch = sac->pitch;

		//Feeble gravity effect...
		//The bigger the angle, the slower it tips...

		if ((adptr->RIGHTWINGIN == BS_DEAD) && (tmproll < ANGLES_45Deg))
		{
			maxangle = ANGLES_0Deg;
			tipleft = true;
		}
		else
		{
			if ((adptr->LEFTWINGIN == BS_DEAD) && (tmproll > -ANGLES_45Deg))
				maxangle = ANGLES_0Deg;
			else
				tipleft = true;
		}

		if (tipleft == false)
		{
			if (tmproll >= maxangle)
				rolldone = TRUE;
			else
				rolldone = FALSE;
		}
		else
		{
			if (tmproll <= -maxangle)
				rolldone = TRUE;
			else
				rolldone = FALSE;
		}

		if (tmppitch < ANGLES_170Deg)
			pitchdone = FALSE;
		else
			pitchdone = TRUE;

		if (rolldone && pitchdone)
		{
			Trans_Obj.LaunchDirtMedium(sac,currworld);

			ReplayDeathMode = RDEATH_NULL;
		}
		else
			sac->fly.pModel->CrashTime += Timer_Code.FRAMETIME;
	}
}

//------------------------------------------------------------------------------
//Procedure		DummyCrashDive
//Author		Robert Slater
//Date			Wed 28 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::DummyCrashDive()
{
	AirStrucPtr		ac=Persons2::PlayerSeenAC;

//	_Miles.PlayLooped(FIL_SFX_FIREBURN_LOOP, ac);
}

//------------------------------------------------------------------------------
//Procedure		StoreDeathMoveState
//Author		Robert Slater
//Date			Thu 29 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreDeathMoveState()
{
	UByte		effect = RDEATH_NULL;
	AirStrucPtr sac=Persons2::PlayerSeenAC;

	switch (sac->AutoMoveInfo.movecode)
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

	// a byte......

	if (!ReplayWrite((UByte*)&effect,sizeof(UByte)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadDeathMoveState
//Author		Robert Slater
//Date			Thu 29 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadDeathMoveState()
{
	UByte		effect = RDEATH_NULL;

	if (!ReplayRead((UByte*)&effect,sizeof(UByte)))
		return FALSE;

	ReplayDeathMode = (RDEATH)effect;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		ResetRecord
//Author		Andy McMaster
//Date			Wed 18 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::ResetRecord()
{
	if (!_Replay.Playback && !_DPlay.Implemented)
	{
		badrecord=true;
		SuperHeaderStored=false;
		DeleteReplayLog();
		if (Record)
		{
			ClearReplayBuffers();
			Record=FALSE;
			CloseRecordLog();
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		ToggleRecord
//Author		Andy McMaster
//Date			Wed 18 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::ToggleRecord()
{
	if (Record)
	{
// done in stoprecord
		StopRecord();
	}
	else
		StartRecordFlag=TRUE;
}

bool	Replay::ValidReplayFile()
{
	HANDLE f;
	bool ok=false;
	char	temp[256];

	strcpy(temp,pfilename);
	GetReplayFilename(temp);

	f=CreateFile(
		temp,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_TEMPORARY,
		NULL);

	if (f!=INVALID_HANDLE_VALUE)
		ok=true;

	if (!CloseHandle(f))
		ok=false;

	return ok;
}

//------------------------------------------------------------------------------
//Procedure		StoreMissionConditions
//Author		Andy McMaster
//Date			Thu 26 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Procedure		LoadMissionConditions
//Author		Andy McMaster
//Date			Thu 26 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//Procedure		BeginNewBlock
//Author		Andy McMaster
//Date			Thu 7 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::BeginNewBlock()
{
	StoreRealFrameCounts(replayframecount,0,replayframecount-1);
	StopRecord();												//AMM 01Jul99

	StartRecordFlag=TRUE;										//AMM 01Jul99
}

//------------------------------------------------------------------------------
//Procedure		StoreAeroDevData
//Author		Andy McMaster
//Date			Thu 7 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreAeroDevData(AirStrucPtr ac)
{
	AERODEVVALUES a;

	PAERODEVICE pAeroDevice=ac->fly.pModel->DeviceList;

	while (pAeroDevice)
	{
		GetAeroDevData(&a,pAeroDevice);

		if (!ReplayWrite((UByte*)&a,sizeof(AERODEVVALUES)))
			return FALSE;

		pAeroDevice=pAeroDevice->List.NextItem();
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetAeroDevData
//Author		Andy McMaster
//Date			Thu 7 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::GetAeroDevData(LPAERODEVVALUES adv,PAERODEVICE pad)
{
	adv->Val=pad->Val;
	adv->Attractor=pad->Attractor;
	adv->OldAttractor=pad->OldAttractor;
}

//------------------------------------------------------------------------------
//Procedure		RestoreAeroDevData
//Author		Andy McMaster
//Date			Thu 7 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RestoreAeroDevData(LPAERODEVVALUES adv,PAERODEVICE pad)
{
	pad->Val=adv->Val;
	pad->Attractor=adv->Attractor;
	pad->OldAttractor=adv->OldAttractor;
}

//------------------------------------------------------------------------------
//Procedure		StoreSuspensionData
//Author		Andy McMaster
//Date			Thu 7 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreSuspensionData(AirStrucPtr ac)
{
	FP legpos;

	PGEAR pGear=ac->fly.pModel->GearList;

	while (pGear)
	{
		legpos=GetSuspensionData(pGear);

		if (!ReplayWrite((UByte*)&legpos,sizeof(FP)))
			return FALSE;

	 	pGear = pGear->List.NextItem();
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetSuspensionData
//Author		Andy McMaster
//Date			Thu 7 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
FP	Replay::GetSuspensionData(PGEAR pGear)
{
	return pGear->LegPos;
}

//------------------------------------------------------------------------------
//Procedure		RestoreSuspensionData
//Author		Andy McMaster
//Date			Thu 7 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RestoreSuspensionData(FP legpos,PGEAR pGear)
{
	pGear->LegPos=legpos;
}

//------------------------------------------------------------------------------
//Procedure		LoadSuperHeader
//Author		Andy McMaster
//Date			Tue 12 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadSuperHeader()
{
	if (!LoadSuperHeaderBeginning())
		return FALSE;

	if (!LoadSuperHeaderEnd())
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadSuperHeaderBeginning
//Author		Andy McMaster
//Date			Tue 12 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadSuperHeaderBeginning()
{
	if (!LoadSuperHeaderID())
		return FALSE;

	if (!LoadGRRandomList())
		return FALSE;

	if (!LoadInitialGlobRefs())
		return FALSE;

	if (!LoadSaveGame())
		return FALSE;

	if (!LoadFileAndGlobRefList())
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadSuperHeaderEnd
//Author		Andy McMaster
//Date			Tue 12 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadSuperHeaderEnd()
{

	if (!LoadSuperHeaderEndID())
		return FALSE;

	SuperHeaderSize=playbackfilepos-(UByteP)playbackfilestart;

	if (!RestorePosition)
	{
		CurrPos=0;
		BeginMarkPos=0;
		EndMarkPos=255;
	}

	return TRUE;
}

Bool	Replay::StoreSuperHeader()
{
	if (!StoreSuperHeaderBeginning())
		return FALSE;

	if (!StoreSuperHeaderEnd())
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreSuperHeaderBeginning
//Author		Andy McMaster
//Date			Tue 12 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreSuperHeaderBeginning()
{
	if (SuperHeaderStored)
		return TRUE;

	if (!StoreSuperHeaderID())
		return FALSE;

	if (!StoreGRRandomList())
		return FALSE;

	if (!StoreInitialGlobRefs())
		return FALSE;

	if (!StoreSaveGame())
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreSuperHeaderEnd
//Author		Andy McMaster
//Date			Tue 12 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreSuperHeaderEnd()
{
	if (SuperHeaderStored)
		return TRUE;

 	if (!StoreFileAndGlobRefList())
		return FALSE;

	if (!StoreSuperHeaderEndID())
		return FALSE;

	SuperHeaderStored=true;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadSuperHeaderID
//Author		Andy McMaster
//Date			Tue 12 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadSuperHeaderID()
{
	ULong id=0;

	if (!ReplayRead((UByte*)&id,sizeof(ULong)))
		return FALSE;

	if (id!=REPLAYSUPERHEADBEGINID)
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreSuperHeaderID
//Author		Andy McMaster
//Date			Tue 12 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreSuperHeaderID()
{
	ULong id=REPLAYSUPERHEADBEGINID;

	if (!ReplayWrite((UByte*)&id,sizeof(ULong)))
		return FALSE;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreBlockHeader
//Author		Andy McMaster
//Date			Tue 12 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreBlockHeader()
{
	ULong size1=GetFileSize(recordstorage,NULL),size2,len;
	ULong size3,size4;

	ResetAICount();
	_DPlay.ResetDeltas();											  //AMM 08/07/99

//TEMPCODE DAW 28/03/99 	Timer_Code.msgcounttime=0; // reset rounding stuff

	if (!StoreHeaderID())
		return FALSE;

	if (!StoreTimeOfDay())
		return FALSE;

	if (!StoreRandomList())
		return FALSE;

	if (!StoreGamePrefs())
		return FALSE;

	if (!StoreDeadItems())
		return FALSE;

	if (!StoreAAAList())
		return FALSE;												  //JIM 14/06/99

	if (!StoreItemData())
		return FALSE;

	if (!StoreItemAnims())
		return FALSE;

	if (!StoreArmedTransients())								//AMM 24Jun99
		return FALSE;											//AMM 24Jun99

	if (!StoreDeathMoveState())
		return FALSE;

	size3=0;
	if (!ReplayWrite((UByte*)&size3,sizeof(ULong)))
		return FALSE;

	size3=GetFileSize(recordstorage,NULL);

	if (!SHAPE.SampleTrailList())
		return FALSE;

	size4=GetFileSize(recordstorage,NULL);
	len=size4-size3;

	if (!StoreSmokeSize(len))
		return FALSE;

	if (!StorePrevPosBuffer())
		return FALSE;

	if (!StoreHeaderEndID())
		return FALSE;

// NOTE:: these must be stored last because when block ends file pointer is rewound to position
// and frame counts filled in, if there is data after them then I dont know how far to rewind

	if (!StoreInitFrameCounts())
		return FALSE;

	size2=GetFileSize(recordstorage,NULL);

	len=size2-size1;

	if (!StoreHeaderLength(len))
		return FALSE;

	replayframecount=0;

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		LoadBlockHeader
//Author		Andy McMaster
//Date			Tue 12 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadBlockHeader()
{
	ULong n;

	_Miles.delayedsounds.isSet = FALSE;							//DAW 18Aug99

	AirStruc::playermoving=false;								//DAW 22Jun99
	ResetAICount();
	_DPlay.ResetDeltas();											  //AMM 08/07/99
//TEMPCODE DAW 28/03/99 	Timer_Code.msgcounttime=0; // reset rounding stuff

	headmems[0][currblock]=playbackfilepos;

	if (!LoadHeaderID())
		return FALSE;

	if (DoSmokeTrailStuff)		 //AMM 30Jul99
		UpdateLandscape=true;

	if (!LoadTimeOfDay())
		return FALSE;

// OK, in alot of places when skipping about the transients need to be removed from world
// before new smoke trails etc can be launched. The only time this should not happen is
// during normal playback in which case smoke trail stuff is not done

	if (DoSmokeTrailStuff)			//RJS 01Apr99	(WAS !)
		RemoveAllTransients();

	if (!LoadRandomList())
		return FALSE;
			   
	if (!LoadGamePrefs())
		return FALSE;

	if (DoSmokeTrailStuff)											  //AMM 23/06/99
		ResetItemAnim();											  //AMM 23/06/99

	if (!LoadDeadItems())
		return FALSE;

	if (!LoadAAAList())
		return FALSE;

	if (!LoadItemData())
		return FALSE;

	if (!LoadItemAnims())
		return FALSE;

	if (!LoadArmedTransients())									//AMM 24Jun99
		return FALSE;											//AMM 24Jun99

	if (!LoadDeathMoveState())
		return FALSE;

	if (!BackupSmokeInfo())
		return FALSE;

	if (DoSmokeTrailStuff)
	{
		processsmokes=true;
	}
	else
	{
		if (!SHAPE.GetSmokeTrailList(TRUE))
			return FALSE;
	}

	if (!LoadPrevPosBuffer())
		return FALSE;

	if (!LoadHeaderEndID())
		return FALSE;

	if (!LoadFrameCounts())
		return FALSE;

//	if (thisblockstartframe!=0)
//	{
//		ReplayFlag=RF_SKIPFRAMES;			
//		framestoskip=thisblockstartframe;
//	}

	n=LoadHeaderLength();

	if (n==0)
		return FALSE;

	currblock++;

	if (currblock>numblocks)
	{

		if (emptyblock)
		{
			numframes[numblocks]=0;
			headmems[1][currblock-1]=playbackfilepos;
			markers[0][currblock-1]=0;
			markers[1][currblock-1]=0;
		}
		else
		{
			markers[0][currblock-1]=thisblockstartframe;
			markers[1][currblock-1]=thisblockendframe;
			numframes[numblocks]=thisblocknumframes;
			headmems[1][currblock-1]=(playbackfilepos+(thisblocknumframes*sizeof(REPLAYPACKET)));
		}
		headlength[numblocks++]=n;
	}

	replayframecount=0;

	if (thisblockstartframe!=0)
	{
		ReplayFlag=RF_SKIPFRAMES;			
		framestoskip=thisblockstartframe;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		BackupPlaybackPosition
//Author		Andy McMaster
//Date			Mon 18 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::BackupPlaybackPosition()
{
	backupblock=currblock-1;
	backupframe=replayframecount;
	RestorePosition=true;
	backupstate=GetState();
}

//------------------------------------------------------------------------------
//Procedure		RestorePlaybackPosition
//Author		Andy McMaster
//Date			Mon 18 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RestorePlaybackPosition()
{
	ULong n;

	RestorePosition=false;

	for (n=0;n<backupblock;n++)
	{
		BlockForward();
	}

	framestoskip=backupframe;
	ReplayFlag=RF_SKIPFRAMES;

	SetState(backupstate);
}

//------------------------------------------------------------------------------
//Procedure		LoadSaveGame
//Author		Andy McMaster
//Date			Mon 18 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::LoadSaveGame()
{
	ULong size;
	DWORD temp;
	char	dummyname[150];
	HANDLE	dummyfile;
	bool retval;
	ULong	tempstate;

// backup current savegame

//	if (!BackupSaveGame())
//		return FALSE;

	if (!ReplayRead((UByte*)&tempstate,sizeof(ULong)))
		return FALSE;

	PlaybackGameState=GetState();

	if (!IsCampState(tempstate))
	{
		if (!LoadCampaignStruc())
			return FALSE;
		return TRUE;
	}

	if (!ReplayRead((UByte*)&size,sizeof(ULong)))
		return FALSE;

	UByteP	savedata=new UByte[size];

	if (ReplayRead(savedata,size))
	{
		strcpy(dummyname,"dreplay.dat");

		_DPlay.GetCommsSavename(dummyname);

		dummyfile=CreateFile(
			dummyname,
			GENERIC_WRITE|GENERIC_READ,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			CREATE_ALWAYS,
			0,
			NULL);

		if (dummyfile!=INVALID_HANDLE_VALUE)
		{
			WriteFile(
				dummyfile,
				savedata,
				size,
				&temp,
				NULL);

			CloseHandle(dummyfile);

// load game here

			Persons4::ShutDownMapWorld();
			retval=_DPlay.LoadDummySavegame("dreplay.dat");
			Persons4::ShutDownMapWorld();
		}
		else
			retval=FALSE;

	}
	else 
		retval=FALSE;

	delete savedata;

	return retval;
}

//------------------------------------------------------------------------------
//Procedure		StoreSaveGame
//Author		Andy McMaster
//Date			Mon 18 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreSaveGame()
{
	ULong	savesize;
	DWORD numread;
	char savedgamename[150];
	HANDLE	savegame;
	Bool	retval=FALSE;
	ULong	tempstate;

	tempstate=GetState();

	if (!ReplayWrite((UByte*)&tempstate,sizeof(ULong)))
		return FALSE;

	if (!IsCampState(tempstate))
	{
		if (!StoreCampaignStruc())
			return FALSE;
		return TRUE;
	}

// need to find best place for this

	strcpy(savedgamename,"dreplay.dat");
	_DPlay.GetCommsSavename(savedgamename);

	savegame=CreateFile(
		savedgamename,
		GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (savegame!=INVALID_HANDLE_VALUE)
	{
		savesize=GetFileSize(savegame,NULL);

		if (ReplayWrite((UByte*)&savesize,sizeof(ULong)))
		{
			UByteP save=new UByte[savesize];

			ReadFile(
				savegame,
				save,
				savesize,
				&numread,
				NULL);

			if (ReplayWrite(save,savesize))
			{
				retval=TRUE;
			}
			
			delete [] save;
		}
		CloseHandle(savegame);									//MACCA 21Oct99
	}

	return retval;
}

//------------------------------------------------------------------------------
//Procedure		BackupSaveGame
//Author		Andy McMaster
//Date			Fri 22 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::BackupSaveGame()
{
	return (SaveTempSaveGame("rbackup.dat",true));
}

//------------------------------------------------------------------------------
//Procedure		RestoreSaveGame
//Author		Andy McMaster
//Date			Fri 22 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::RestoreSaveGame()
{
	ULong	tempstate;

	tempstate=GetState();

	if (IsCampState(tempstate))
	{
		return (_DPlay.LoadDummySavegame("rbackup.dat"));
	}

	return true;
}

void	Replay::ResetReplayView()
{
	Trans_Obj.View_Object->SetToPiloted((itemptr)Persons2::PlayerSeenAC,true);
}

//------------------------------------------------------------------------------
//Procedure		SetVelsAndOldVals
//Author		Andy McMaster
//Date			Mon 8 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::SetVelsAndOldVals(AirStrucPtr ac)
{

	ac->ai.oldpitchI=ANGLES_0Deg;
	ac->ai.oldoldpitchI=ANGLES_0Deg;
	ac->ai.oldoldoldpitchI=ANGLES_0Deg;
	ac->ai.oldhdgI=ANGLES_0Deg;
	ac->ai.oldoldhdgI=ANGLES_0Deg;
	ac->ai.oldoldoldhdgI=ANGLES_0Deg;
}

//------------------------------------------------------------------------------
//Procedure		SkipToBeginningMark
//Author		Andy McMaster
//Date			Wed 17 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::SkipToBeginningMark()
{
	ULong n;

	currblock=0;
	playbackfilepos=(UByteP)playbackfilestart+SuperHeaderSize;
	LoadBlockHeader();

//	RemoveAllTransients();
	for (n=0;n<beginmark.blocknum;n++)
	{
		BlockForward();
	}

	framestoskip=beginmark.framenum;
	_Replay.ReplayFlag=RF_SKIPFRAMES;
	ResetReplayView();
}

//------------------------------------------------------------------------------
//Procedure		SkipToEndMark
//Author		Andy McMaster
//Date			Wed 17 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::SkipToEndMark()
{
	ULong n;

	currblock=0;
	playbackfilepos=(UByteP)playbackfilestart+SuperHeaderSize;
	LoadBlockHeader();

//	RemoveAllTransients();
	for (n=0;n<endmark.blocknum;n++)
	{
		BlockForward();
	}

	framestoskip=endmark.framenum;
	_Replay.ReplayFlag=RF_SKIPFRAMES;
	ResetReplayView();
}

//------------------------------------------------------------------------------
//Procedure		RemoveBeginningMark
//Author		Andy McMaster
//Date			Wed 17 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RemoveBeginningMark()
{
// 1st block beginning becomes start of marked block

	beginmark.memaddress=(UByteP)playbackfilestart+SuperHeaderSize;
	beginmark.blocknum=0;
	beginmark.framenum=BeginBlockFrame;
}

//------------------------------------------------------------------------------
//Procedure		RemoveEndMark
//Author		Andy McMaster
//Date			Wed 17 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RemoveEndMark()
{
// end block end becomes end of marked block

	endmark.memaddress=(UByteP)playbackfileend;
	endmark.blocknum=EndBlockNum;
	endmark.framenum=EndFrameNum;
}

//------------------------------------------------------------------------------
//Procedure		PreScanReplayFile
//Author		Andy McMaster
//Date			Wed 17 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::PreScanReplayFile()
{
/* Info needed:

	1: Total number of frames in replay file
	2: Total number of blocks
	3: Number of frames in last block
*/

	ULong n=0,skip;
	UByteP here=playbackfilepos;

	prescan=true;

	TotalFrames=0;

	if (!RestorePosition)
	{
		beginmark.memaddress=here;
		beginmark.blocknum=0;
	}

	DoSmokeTrailStuff=FALSE;

	while (LoadBlockHeader())
	{
		if (!n)
		{
			BeginBlockFrame=thisblockstartframe;

			if (!RestorePosition)
			{
				beginmark.framenum=thisblockstartframe;
			}
		}

		EndBlockNum=n;
		EndFrameNum=thisblockendframe;
		if (!emptyblock)
		{
			TotalFrames+=(numframes[n]-thisblockstartframe)-(numframes[n]-thisblockendframe);
			skip=sizeof(REPLAYPACKET)*numframes[n];

// skip to next block

			playbackfilepos+=skip;
		}
		n++;
	}
	
	prescan=false;
	
// should never happen, but best to be sure

	if (!TotalFrames)
		TotalFrames=1; 

	if (!RestorePosition)
	{
		endmark.memaddress=(UByteP)playbackfileend;
		endmark.blocknum=EndBlockNum;
		endmark.framenum=EndFrameNum;
	}

	playbackfilepos=here;
	currblock=0;

	DoSmokeTrailStuff=TRUE;
}

//------------------------------------------------------------------------------
//Procedure		SkipToPreviousMark
//Author		Andy McMaster
//Date			Thu 18 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::SkipToPreviousMark()
{
/*
1: if position is at end of file and end marker is at end skip to begin mark
2: if position is at end of file and end marker is not at end skip to end mark
3: if position is between end mark and end of file skip to end mark
4: if position is at end mark skip to begin mark
5: if position is between begin mark and end mark skip to begin mark
6: if position is at begin mark skip to beginning of file
7: if position is between beginning of file and begin mark skip to beginning of file
8: if position is at beginning of file ignore

9: if position is up to 1 sec after end mark skip to beginning mark
10:if position is up to 1 sec after begin mark skip to beginning
*/

	if ((currblock-1)<beginmark.blocknum 
	|| ((currblock-1)==beginmark.blocknum && replayframecount<=beginmark.framenum+1))
	{
		SkipToBeginning();
	}
	else if ((currblock-1)<endmark.blocknum
	|| ((currblock-1)==endmark.blocknum && replayframecount<=endmark.framenum+1))
	{
// after begin mark and before end mark
// if up to 1 sec after begin mark then skip to begining else to begin mark

		ULong framediff=0;
		if (currblock-1==beginmark.blocknum)
		{
			framediff=replayframecount-beginmark.framenum+1;
		}
		else
		{
			ULong block=beginmark.blocknum;

			framediff=markers[1][beginmark.blocknum]-beginmark.framenum+1;
			block++;

			while (block!=currblock-1)
			{
// intermediate blocks between beginmark and currentposition

				framediff+=numframes[currblock-1]
					-(markers[1][block]-markers[0][block]);
				block++;
			}

			framediff+=replayframecount-markers[0][block];
		}

		if (framediff<50) // less than 1 sec after begin mark
			SkipToBeginning();
		else
			SkipToBeginningMark();
	}
	else if ((currblock-1)>endmark.blocknum
	|| (currblock-1==endmark.blocknum && replayframecount>endmark.framenum+1))
	{
// after end mark
// if up to 1 sec after end mark skip to begin mark else end mark

		ULong framediff=0;
		if (currblock-1==endmark.blocknum)
		{
			framediff=replayframecount-endmark.framenum+1;
		}
		else
		{
			ULong block=endmark.blocknum;

			framediff=markers[1][endmark.blocknum]-endmark.framenum+1;
			block++;

			while (block!=currblock-1)
			{
// intermediate blocks between endmark and currentposition

				framediff+=numframes[currblock-1]
					-(markers[1][block]-markers[0][block]);
				block++;
			}

			framediff+=replayframecount-markers[0][block];
		}

		if (framediff<50) // less than 1 sec after end mark
			SkipToBeginningMark();
		else
			SkipToEndMark();
	}
}

//------------------------------------------------------------------------------
//Procedure		SkipToEndMark
//Author		Andy McMaster
//Date			Thu 18 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::SkipToNextMark()
{
// reverse of above

	if ((currblock-1)>endmark.blocknum
	|| ((currblock-1)==endmark.blocknum && replayframecount>=endmark.framenum))
	{
		SkipToEnd();
	}
	else if ((currblock-1)>beginmark.blocknum
	|| ((currblock-1)==beginmark.blocknum && replayframecount>=beginmark.framenum))
	{
		SkipToEndMark();
	}
	else
	{
		SkipToBeginningMark();
	}
}

//------------------------------------------------------------------------------
//Procedure		CalcBeginMarkPos
//Author		Andy McMaster
//Date			Fri 19 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::CalcBeginMarkPos()
{
	ULong n,total=0;
	SLong temp;

	for (n=0;n<beginmark.blocknum;n++)
	{
		if (numframes[n])
			total+=(numframes[n]-((markers[1][n]+1)-numframes[n])-markers[0][n]);
	}

//	total+=beginmark.framenum-markers[0][currblock-1];
	total+=beginmark.framenum-markers[0][n];

	temp=(total*255)/TotalFrames;

	if (temp<0)
		temp=0;
	else if (temp>255)
		temp=255;

	BeginMarkPos=temp;
}

//------------------------------------------------------------------------------
//Procedure		CalcEndMarkPos
//Author		Andy McMaster
//Date			Fri 19 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::CalcEndMarkPos()
{
	ULong n,total=0;
	SLong temp;

	for (n=0;n<endmark.blocknum;n++)
	{
		if (numframes[n])
			total+=(numframes[n]-((markers[1][n]+1)-numframes[n])-markers[0][n]);
	}

//	total+=endmark.framenum-markers[0][currblock-1];
	total+=endmark.framenum-markers[0][n];

	temp=(total*255)/TotalFrames;

	if (temp<0)
		temp=0;
	else if (temp>255)
		temp=255;

	EndMarkPos=temp;
}

//------------------------------------------------------------------------------
//Procedure		CalcCurrPos
//Author		Andy McMaster
//Date			Fri 19 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::CalcCurrPos()
{
	ULong n,total=0;
	SLong	temp;

	for (n=0;n<currblock-1;n++)
	{
		if (numframes[n])
			total+=(numframes[n]-((markers[1][n]+1)-numframes[n])-markers[0][n]);
	}

	total+=replayframecount-markers[0][currblock-1];

	temp=(total*255)/TotalFrames;

	if (temp<0)
		temp=0;
	else if (temp>255)
		temp=255;

	CurrPos=temp;
}

//------------------------------------------------------------------------------
//Procedure		StoreSmlSmkData
//Author		Robert Slater
//Date			Thu 18 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::StoreSmlSmkData(ReplaySmlSmkStruc *smokey,UByteP pointlist)
{
	ReplaySmkPoint	*SmokeList = new ReplaySmkPoint[smokey->nopoints];
	Coords3DList*	apoint = (Coords3DList*) pointlist;
	SLong			index = 0;
	Bool			retval = TRUE;

	while (apoint)
	{
		SmokeList[index].x = apoint->trailpoint.gx;
		SmokeList[index].y = apoint->trailpoint.gy;
		SmokeList[index].z = apoint->trailpoint.gz;
		SmokeList[index].lifeleft = apoint->lifeleft;
		SmokeList[index].radius = apoint->radius;
		SmokeList[index].LifeTime = apoint->LifeTime;
		SmokeList[index].FadeTime = apoint->FadeTime;
		SmokeList[index].KillMe = apoint->KillMe;
		index++;

		apoint = apoint->next;
	}

	if (!ReplayWrite((UByte*)smokey,sizeof(ReplaySmlSmkStruc)))
		retval = FALSE;
	else
	{
		// Write it out backwards....so I can read back in the right order...
		while (index > 0)
		{
			index--;
			if (!ReplayWrite((UByteP)&SmokeList[index],sizeof(ReplaySmkPoint)))
			{
				retval = FALSE;
				break;
			}
		}
	}

	delete[]SmokeList;

	return (retval);
}

//------------------------------------------------------------------------------
//Procedure		LoadSmlSmkData
//Author		Robert Slater
//Date			Thu 18 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	Replay::LoadSmlSmkData(ReplaySmlSmkStruc *smokey, Bool dummy, UByteP	&TrailListPtr, Coords3D& cpos)
{
	if (!SmokeRead((UByte*)smokey,sizeof(ReplaySmlSmkStruc)))
		return FALSE;

	Coords3DList*	apoint;
	Coords3DList*	baseptr = NULL;
	ReplaySmkPoint	smkpoint;
	int	i;

	cpos.X = cpos.Y = cpos.Z = 0;

	for (i = 0; i < smokey->nopoints; i++)
	{
		if (!SmokeRead((UByte*)&smkpoint,sizeof(ReplaySmkPoint)))
			return FALSE;

		if (!dummy)
		{
			apoint = new Coords3DList;
			if (apoint)
			{
				cpos.X += smkpoint.x;
				cpos.Y += smkpoint.y;
				cpos.Z += smkpoint.z;

				apoint->trailpoint.gx = smkpoint.x;
				apoint->trailpoint.gy = smkpoint.y;
				apoint->trailpoint.gz = smkpoint.z;
				apoint->lifeleft = smkpoint.lifeleft;
				apoint->radius = smkpoint.radius;
				apoint->LifeTime = smkpoint.LifeTime;
				apoint->FadeTime = smkpoint.FadeTime;
				apoint->KillMe = smkpoint.KillMe;
				apoint->next = baseptr;

				baseptr = apoint;
			}
		}
	}

	TrailListPtr = (UByteP)baseptr;
	if (smokey->nopoints)
	{
		cpos.X /= smokey->nopoints;
		cpos.Y /= smokey->nopoints;
		cpos.Z /= smokey->nopoints;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		UpdateSmokes
//Author		Andy McMaster
//Date			Thu 25 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::UpdateSmokes()
{
	UWord sector=MobileItem::currworld->GetSector(Persons2::PlayerSeenAC);

	ItemPtr next=MobileItem::currworld->getfirstitem(sector);

	while (next)
	{
		SHAPE.replay_animate_shape(next);
		next=next->Next;
	}
}

//------------------------------------------------------------------------------
//Procedure		LoadFMMass
//Author		Andy McMaster
//Date			Fri 5 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::LoadFMMass(AirStrucPtr ac)
{
	FP m;

	if (!ReplayRead((UByteP)&m,sizeof(FP)))
		return false;

	if (!prescan)
		ac->fly.pModel->Mass=m;

	return true; 
}

//------------------------------------------------------------------------------
//Procedure		StoreFMMass
//Author		Andy McMaster
//Date			Fri 5 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::StoreFMMass(AirStrucPtr ac)
{
	FP m=ac->fly.pModel->Mass;

	if (!ReplayWrite((UByteP)&m,sizeof(FP)))
		return false;

	return true;
}

//------------------------------------------------------------------------------
//Procedure		ClearReplayBuffers
//Author		Andy McMaster
//Date			Wed 17 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::ClearReplayBuffers()
{
	BASICPACKET packet;
	COMMSCOLLISION CommsCollision;
	SPECPACKET SpecPack;

	while (_DPlay.TestForCollision (&CommsCollision))
	{
		_DPlay.MakeCollPacket (&CommsCollision, &packet);
		packet.IDCode|=(PT_COLLISION<<6);
	 	_DPlay.ProcessCollisionPacket(&packet,0);
	}

	while (_DPlay.TestForUIDSPacket(&SpecPack))
	{
		_DPlay.MakeUIDSPacket(&SpecPack,&packet);
		packet.IDCode|=PT_UIDS<<6;
		_DPlay.ProcessUIDSPacket(&packet,0);
	}

	while (_DPlay.TestForSpecial(&SpecPack))
	{
		_DPlay.MakeSpecPacket(&SpecPack,&packet);
  		packet.IDCode|=(PT_SPECIAL<<6);
		_DPlay.ProcessSpecialPacket(&packet,Persons2::PlayerSeenAC,0);
	}
}

//------------------------------------------------------------------------------
//Procedure		InitPosBuffer
//Author		Andy McMaster
//Date			Fri 19 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::InitPosBuffer()
{
// initial last position are starting position

	PosBuffer.InitBuffer();
	PosBuffer.SetTempCurr();

	SWord num=POSBSIZE-1;
	SWord vx=Persons2::PlayerGhostAC->velx;
	SWord vy=Persons2::PlayerGhostAC->vely;
	SWord vz=Persons2::PlayerGhostAC->velz;

	SLong wx, wy, wz;												//CSB 11/06/99	
	MMC.Sky.GetWind(Persons2::PlayerGhostAC->World.Y, wx, wy, wz);	//CSB 11/06/99	
	vx += wx;														//CSB 11/06/99	
	vy += wy;														//CSB 11/06/99	
	vz += wz;														//CSB 11/06/99	

	COORDS3D	pos;
	COORDS3D	acpos=Persons2::PlayerGhostAC->World;

	do
	{
		pos.X=acpos.X-((num*vx)/5);
		pos.Y=acpos.Y-((num*vy)/5);
		pos.Z=acpos.Z-((num*vz)/5);
		num--;

		*PosBuffer.GetTemp()=pos;

		PosBuffer.UpdateTemp();
	}
	while (PosBuffer.GetTemp()!=PosBuffer.GetCurr());

// get to last pos in buffer

	for (int n=0;n<POSBSIZE-1;n++)
		PosBuffer.UpdateNext();
}

//------------------------------------------------------------------------------
//Procedure		StorePrevPosBuffer
//Author		Andy McMaster
//Date			Fri 19 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::StorePrevPosBuffer()
{
	PosBuffer.SetTempCurr();

	do
	{
		if (!ReplayWrite((UByte*)&(*PosBuffer.GetTemp()),sizeof(COORDS3D)))
			return false;

		PosBuffer.UpdateTemp();
	}
	while (PosBuffer.GetTemp()!=PosBuffer.GetCurr());

	return true;
}

//------------------------------------------------------------------------------
//Procedure		LoadPrevPosBuffer
//Author		Andy McMaster
//Date			Fri 19 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::LoadPrevPosBuffer()
{
	PosBuffer.InitBuffer();										//AMM 28Apr99

	for (int n=0;n<POSBSIZE-1;n++)								//AMM 28Apr99
		PosBuffer.UpdateNext();									//AMM 28Apr99

	PosBuffer.SetTempCurr();

	do
	{
		if (!ReplayRead((UByte*)&(*PosBuffer.GetTemp()),sizeof(COORDS3D)))
			return false;

		PosBuffer.UpdateTemp();
	}
	while (PosBuffer.GetTemp()!=PosBuffer.GetCurr());

	return true;
}

//------------------------------------------------------------------------------
//Procedure		UpdatePosBuffer
//Author		Andy McMaster
//Date			Fri 19 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::UpdatePosBuffer()
{
	*PosBuffer.GetNext()=Persons2::PlayerGhostAC->World;
}

//------------------------------------------------------------------------------
//Procedure		StoreHeaderEndID
//Author		Andy McMaster
//Date			Wed 31 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::StoreHeaderEndID()
{
	ULong id=REPLAYHEADENDID;

	if (!ReplayWrite((UByte*)&id,sizeof(ULong)))
		return false;

	return true;
}

//------------------------------------------------------------------------------
//Procedure		StoreSuperHeaderEndID
//Author		Andy McMaster
//Date			Wed 31 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::StoreSuperHeaderEndID()
{
	ULong id=REPLAYSUPERHEADENDID;

	if (!ReplayWrite((UByte*)&id,sizeof(ULong)))
		return false;

	return true;
}

//------------------------------------------------------------------------------
//Procedure		LoadHeaderEndID
//Author		Andy McMaster
//Date			Wed 31 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::LoadHeaderEndID()
{
	ULong id=0;

	if (!ReplayRead((UByte*)&id,sizeof(ULong)))
		return false;

	if (id!=REPLAYHEADENDID)
		return false;

	return true;
}

//------------------------------------------------------------------------------
//Procedure		LoadSuperHeaderEndID
//Author		Andy McMaster
//Date			Wed 31 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::LoadSuperHeaderEndID()
{
	ULong id=0;

	if (!ReplayRead((UByte*)&id,sizeof(ULong)))
		return false;

	if (id!=REPLAYSUPERHEADENDID)
		return false;

	return true;
}

//------------------------------------------------------------------------------
//Procedure		BackupSmokeInfo
//Author		Andy McMaster
//Date			Tue 20 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::BackupSmokeInfo()
{
	ULong smokesize;

	if (!ReplayRead((UByte*)&smokesize,sizeof(ULong)))
		return false;

	delete [] backupsmoke;
	backupsmoke=NULL;

	backupsmoke=new UByte[smokesize];
	tempbackupsmoke=backupsmoke;

	if (!backupsmoke)
		return false;

	if (!ReplayRead(backupsmoke,smokesize))
		return false;

	return true;
}

//------------------------------------------------------------------------------
//Procedure		UpdateSmokeyInfo
//Author		Andy McMaster
//Date			Tue 20 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::UpdateSmokeInfo()
{
	ULong n;

	SHAPE.DeleteAllSmokeTrails();

	SHAPE.GetSmokeTrailList(FALSE);

	processsmokes=false;

	delete [] backupsmoke;
	backupsmoke=NULL;
}

//------------------------------------------------------------------------------
//Procedure		SmokeRead
//Author		Andy McMaster
//Date			Tue 20 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::SmokeRead(void* dest, ULong size)
{
	memcpy(dest,tempbackupsmoke,size);
	tempbackupsmoke+=size;

	return true;
}

//------------------------------------------------------------------------------
//Procedure		StoreSmokeSize
//Author		Andy McMaster
//Date			Tue 20 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::StoreSmokeSize(ULong size)
{
	SLong jump=size+sizeof(ULong);

	jump=-jump;

	SetFilePointer(recordstorage,jump,NULL,FILE_CURRENT);

	if (!ReplayWrite((UByte*)&size,sizeof(ULong)))
		return false;

	SetFilePointer(recordstorage,0,NULL,FILE_END);

	return true;
}

//------------------------------------------------------------------------------
//Procedure		BackupPrefs
//Author		Andy McMaster
//Date			Fri 7 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::BackupPrefs()
{
	ReplayPrefs.flightdifficulty=Save_Data.flightdifficulty;
	ReplayPrefs.gamedifficulty=Save_Data.gamedifficulty;
	ReplayPrefs.autopilotskillUN=Save_Data.autopilotskillUN;
	ReplayPrefs.autopilotskillRED=Save_Data.autopilotskillRED;
	ReplayPrefs.targetsize=Save_Data.targetsize;
}

//------------------------------------------------------------------------------
//Procedure		RestorePrefs
//Author		Andy McMaster
//Date			Fri 7 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::RestorePrefs()
{
	Save_Data.flightdifficulty=ReplayPrefs.flightdifficulty;
	Save_Data.gamedifficulty=ReplayPrefs.gamedifficulty;
	Save_Data.autopilotskillUN=ReplayPrefs.autopilotskillUN;
	Save_Data.autopilotskillRED=ReplayPrefs.autopilotskillRED;
	Save_Data.targetsize=ReplayPrefs.targetsize;
}

//------------------------------------------------------------------------------
//Procedure		StoreTimeOfDay
//Author		Andy McMaster
//Date			Tue 11 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::StoreTimeOfDay()
{
	int	time=inst->timeofday;

	if (!ReplayWrite((UByte*)&time,sizeof(int)))
		return false;

	return true;
}

//------------------------------------------------------------------------------
//Procedure		LoadTimeOfDat
//Author		Andy McMaster
//Date			Tue 11 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::LoadTimeOfDay()
{
	int time,time2;

	if (!ReplayRead((UByte*)&time,sizeof(int)))
		return false;

	time2=inst->timeofday;

	if (abs(time-time2)>100)
		UpdateLandscape=true;

	inst->timeofday=time;

	return true;
}

void	Replay::SetTimeOfDay(int time)							//AMM 30Jun99
{																//AMM 30Jun99
	inst->timeofday=time;										//AMM 30Jun99
}																//AMM 30Jun99

//------------------------------------------------------------------------------
//Procedure		StoreCampaignStruc
//Author		Andy McMaster
//Date			Wed 12 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::StoreCampaignStruc()
{
	if (!ReplayWrite((UByte*)&MMC,sizeof(Campaign)))
		return false;

	return true;
}

//------------------------------------------------------------------------------
//Procedure		LoadCampaignStruc
//Author		Andy McMaster
//Date			Wed 12 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::LoadCampaignStruc()
{
	if (!ReplayRead((UByte*)&MMC,sizeof(Campaign)))
		return false;

	return true;
}

void	Replay::TruncateFile(char* name)
{
	HANDLE handle;

	handle=CreateFile(name,
		GENERIC_WRITE|GENERIC_READ,
		FILE_SHARE_WRITE|FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_TEMPORARY,
		NULL);

	if (handle!=INVALID_HANDLE_VALUE)
	{
		SetEndOfFile(handle);
		CloseHandle(handle);
	}
}

//------------------------------------------------------------------------------
//Procedure		BackupGRRandomList
//Author		Andy McMaster
//Date			Wed 26 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::BackupGRRandomList()
{
	for (int n=0; n<(RNDPACKETSIZE - 2); n++)
		BackupRndPacket[n] = Math_Lib.GetRndLookUp(n);

	BackupRndPacket[(RNDPACKETSIZE - 2)] = Math_Lib.Getbval();
	BackupRndPacket[(RNDPACKETSIZE - 1)] = Math_Lib.Getcval();
}

//------------------------------------------------------------------------------
//Procedure		LoadGRRandomList
//Author		Andy McMaster
//Date			Wed 26 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::LoadGRRandomList()
{
	UWord RndPacket [RNDPACKETSIZE],n;
	
	if (!ReplayRead((UByte*)&RndPacket,RNDPACKETSIZE*sizeof(UWord)))
		return FALSE;

	for (n=0; n<(RNDPACKETSIZE - 2); n++)
		Math_Lib.SetRndLookUp(n,RndPacket[n]);

	Math_Lib.Setbval(RndPacket[(RNDPACKETSIZE - 2)]);
	Math_Lib.Setcval(RndPacket[(RNDPACKETSIZE - 1)]);

	Math_Lib.ResetRndCount();

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreGRRandomList
//Author		Andy McMaster
//Date			Wed 26 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::StoreGRRandomList()
{
	if (!ReplayWrite((UByte*)&BackupRndPacket,RNDPACKETSIZE*sizeof(UWord)))
		return FALSE;

	Math_Lib.ResetRndCount();

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		StoreAAAList
//Author		Andy McMaster
//Date			Mon 14 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::StoreAAAList()
{
	ULong size=0;
	UWord array[40]; //32 may not be enough if every element in group is gun...

	UWord	num=0,num2,count=0;;
	SWord	skipsize=-1;

	while (skipsize<0)		//returns negative until end of list
	{
		skipsize=Three_Dee.livelist->ArchiveLiveList(array,40,num);
		num-=skipsize;
	}
	num+=skipsize+skipsize;	//total size is positive

	UWordP aaa=new UWord[num];

	if (aaa)
	{
		skipsize=Three_Dee.livelist->ArchiveLiveList(aaa,num,0);
		num2=skipsize;

		assert(num==num2);

		if (!ReplayWrite((UByteP)&num,sizeof(UWord)))
		{
			delete [] aaa;
			return false;
		}

		while (num2--)
		{
			if (!ReplayWrite((UByteP)&aaa[num2],sizeof(UWord)))
			{
				delete [] aaa;
				return false;
			}
		}

 		Three_Dee.livelist->RestoreSleepList();
		Three_Dee.livelist->ApplyLiveList(aaa,num);

		delete [] aaa;

		return true;
	}
	else
		return false;
}

//------------------------------------------------------------------------------
//Procedure		LoadAAAList
//Author		Andy McMaster
//Date			Mon 14 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::LoadAAAList()
{
	UWord num,num2;

	if (!ReplayRead((UByteP)&num,sizeof(UWord)))
		return false;

	UWordP aaa=new UWord[num];

	num2=num;

	while (num2--)
	{
		if (!ReplayRead((UByteP)&aaa[num2],sizeof(UWord)))
		{
			delete [] aaa;
			return false;
		}
	}

	if (!prescan)
	{
		Three_Dee.livelist->RestoreSleepList();
		Three_Dee.livelist->ApplyLiveList(aaa,num);
	}

	delete [] aaa;
	return true;
}

//------------------------------------------------------------------------------
//Procedure		ResetItemAnim
//Author		Andy McMaster
//Date			Wed 23 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	Replay::ResetItemAnim()
{
	ItemPtr i;

	if (prescan)
		return;

	for (int n=0;n<WorldStuff::PITEMTABLESIZE;n++)
	{
		if (Persons2::pItem[n])
		{
			i=(ItemPtr)Persons2::pItem[n];

			if (i->MigStatus.LaunchFlags.size==ItemSize) // all other type should be covered by AC or Mob
			{
				SHAPE.ResetAnimData_NewShape(i,i->shape);
			}
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		StoreArmedTransients
//Author		Andy McMaster
//Date			Wed 23 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::StoreArmedTransients()
{
	TransientItemPtr	temp=TransientItem::TransientList;
	ArmedTransient at;
	SLong		hitstrength;

	while (temp)
	{
		if (temp->isArmed)
		{
			at.Pos.X=temp->World.X;
			at.Pos.Y=temp->World.Y;
			at.Pos.Z=temp->World.Z;

			at.hdg=temp->hdg;									//AMM 24Jun99
			at.pitch=temp->pitch;								//AMM 24Jun99

			at.velx=temp->velx;
			at.vely=temp->vely;
			at.velz=temp->velz;
			at.velhori=temp->velhori;

			at.lifetime=(UWord)(temp->LaunchTime);
			at.movecode=(UByte)(temp->AutoMoveInfo.movecode);
			at.launcher=(UWord)(temp->Launcher->uniqueID.bitfieldUid.count);

			if (temp->MigStatus.LaunchFlags.deadtime)	at.deadtime = 1;		//AMM 24Jun99
			else at.deadtime=0;
			at.transrandom = temp->TransRandom;					//AMM 24Jun99
			at.trailindex = 0;									//AMM 24Jun99

			hitstrength = 0;
			ShapeDescPtr	sdptr = SHAPESTUFF.GetShapePtr(temp->shape);
			if (sdptr->AnimDataSize == MISSILEANIM)
			{
				MissileAnimData*	adptr = (MissileAnimData*)temp->Anim;
				hitstrength = adptr->hitstrength;
			}
			else												//AMM 24Jun99
			{													//AMM 24Jun99
				if (sdptr->AnimDataSize == MOVEGUNANIM)			//AMM 24Jun99
				{												//AMM 24Jun99
					MoveGunAnimData*	adptr = (MoveGunAnimData*)temp->Anim;//AMM 24Jun99
					at.trailindex = adptr->weaponlaunchers[0].hdg;//AMM 24Jun99
				}												//AMM 24Jun99
			}													//AMM 24Jun99
			
			at.shape = temp->shape;
			at.strength=hitstrength;

			if (!ReplayWrite((UByte*)&at,sizeof(ArmedTransient)))
				return FALSE;
		}
		temp=temp->nexttrans;
	}

	//End....
	at.shape = 0;

	if (!ReplayWrite((UByte*)&at,sizeof(ArmedTransient)))
		return FALSE;

	return true;
}

//------------------------------------------------------------------------------
//Procedure		LoadArmedTransients
//Author		Andy McMaster
//Date			Wed 23 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	Replay::LoadArmedTransients()
{
// load armed transients

	bool end=false;
	ArmedTransient at;

	while (!end)
	{
		if (!ReplayRead((UByte*)&at,sizeof(ArmedTransient)))
			return FALSE;

		if (at.shape==0)
			break;

		if (DoSmokeTrailStuff)
		{
// only process if block jump
			Trans_Obj.LaunchReplayWeapon((UByte*)&at); 
		}
	}
	return true;
}

//------------------------------------------------------------------------------
//Procedure		Replay
//Author		Andy McMaster
//Date			Tue 5 May 1998
// 
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
CON	Replay::Replay()
{
//	FILE* fp=fopen("sizes.txt","at");
//	fprintf(fp,"MobItem=%d\n",sizeof(MIPRIMARYVALUES));
//	fprintf(fp,"AirStruc=%d\n",sizeof(ASPRIMARYVALUES));
//	fprintf(fp,"FlightModel=%d\n",sizeof(FMPRIMARYVALUES));
//	fprintf(fp,"Engine=%d\n",sizeof(PRIMARYENGINEVALUES));
//	fprintf(fp,"Elmt=%d\n",sizeof(PRIMARYELMTVALUES));
//	fprintf(fp,"AeroDev=%d\n",sizeof(AERODEVVALUES));
//	fclose(fp);


	ULong n;

	strcpy(rfilename,"replay.dat");
	strcpy(pfilename,"replay.dat");
	strcpy(RealFilename,"");;
	Record=FALSE;
	Playback=FALSE;

	playbackfilestart=NULL;
	playbackfilepos=NULL;

	recordfilestart=NULL;
	recordfilepos=NULL;

	ResetReplayData();

	for (n=0;n<256;n++)
	{
		headlength[n]=0;
		numframes[n]=0;
	}

	initgrlist=NULL;
	bfgrlist=NULL;
 	currentry=NULL;

	recordstorage=NULL;
	playbackstorage=NULL;

	recordfilemaxsize=0;
	recordfilecurrsize=0;

	ReplayDeathMode = RDEATH_NULL;
	DoSmokeTrailStuff=TRUE;

	framestoskip=0;
	numrecordedblocks=0;
	ResetFileFlag=false;
	guncameradelay=0;
	SuperHeaderStored=false;
	RestorePosition=false;
	MenuViewPlayback=false;
	ReplayFlag=RF_NONE;
 	MapPlayback=false;
	badrecord=true;
	prefscheck=false;
	skipOK=false;
	ReadBlock=false;
	prescan=false;
	stopforaccel=false;
	setpaused=0;
	emptyblock=false;
	processsmokes=false;
	backupsmoke=NULL;
	tempbackupsmoke=NULL;
}

CON	Replay::~Replay()
{
	Playback=FALSE;
	Record=FALSE;
	CloseRecordLog();
	ClosePlaybackLog();
	DeleteGRList(initgrlist);
	DeleteFileAndGlobRefList(bfgrlist);
//	TruncateTempFiles();
}



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
#include "CSTRING.H" //RERUN
#include "timeapi.h"

//------------------------------------------------------------------------------
//Filename       winmove.cpp
//System         
//Author         Andrew McRae
//Date           Mon 5 Aug 1996
//Description    Move all things.
//------------------------------------------------------------------------------

#define IDIRECTPLAY2_OR_GREATER
#define	F_COMMON
///#include	"ui.h"
#include	"DOSDEFS.H"
#include	"WORLDINC.H"
#include	"UNIQUEID.H"
#include	"FLYMODEL.H"
#include	"AI.H"
#include	"dplay_stub.h"
#include	"SAVEGAME.H"
#include	"MISSSUB.H"
#include	"COMMS.H"
#include	"WINMOVE.H"
#include	"PERSONS2.H"
#include	"KEYTEST.H"
#include	"HOTKEYS.H"
#include	"3DCODE.H"
#include	"MYTIME.H"
#include	"VIEWSEL.H"
#include	"math.h"
#include	"MYMATH.H"
#include	"MYANGLES.H"
#include	"TRANSITE.H"
#include	"3DCOM.H"
#include	"3DCODE.H"
#include	"LSTREAM.H"
#include	"AAA.H"
#include	"COLLIDED.H"
#include	"RANGES.H"
#include	"SPEED.H"
#include	"math.h"
#include	"MONOTXT.H"
#include	"KEYTEST.H"
#include	"TEXT.H"
#include	"MILES.H"
#include	"WINMAIN.H"
#include	"AGGRGTOR.H"
#include	"REPLAY.H"
#include	"MYERROR.H"

#include	"MONOTXT.H"
#include "MODVEC.H"
#include "MODEL.H"
#include	"GLOBREFS.H"
#include	"CTIMEOUT.H"
#include	"BOXCOL.H"
#include	"OVERLAY.H"
#include		"RCHATTER.H"
#include "MISSMAN2.H" // RERUN
#include	"mfc/RESOURCE.H" // RERUN added, comes from MFC

#define		SPECIAL_KEYMAPS	FALSE

#ifdef BUFFERLENGTH
#undef BUFFERLENGTH
#endif

#define BUFFERLENGTH 128

AirStruc* Persons2::PlayerGhostAC=NULL;
AirStruc* Persons2::PlayerSeenAC=NULL;

Aggregator	_Agg ; 

extern Replay	_Replay; // RERUN, see replay.h & cpp

#ifdef TIMERTIME
static ULong Qfreq;
static ULong Qtimertime;
static ULong Qlasttimertime;

static ULong timertime;
static ULong lasttimertime;
FILE* timer;
#endif

//------------------------------------------------------------------------------
//Procedure		CommsGameSync
//Author		Andy McMaster
//Date			Fri 16 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	DPlay::CommsGameSync()
{
#ifdef WINTIME	
 	static ULong lasttime=_Miles.GetSysTime();
 	ULong ttime=_Miles.GetSysTime();

 	FILE* fp=fopen("wtime.txt","at");
 	fprintf(fp,"CCALL %d\n",(ttime-lasttime)/10);
 	fclose(fp);
 	lasttime=ttime;
#endif
	if (Implemented)
	{
		if (realfirsttimein)
		{
			realfirsttimein=FALSE;

// Set Active Player Bit field

			ActivePlayers = 0;
			for (int n=0; n<MAXPLAYERS; n++)
			{
				if (DPlay::H2H_Player[n].status==CPS_3D)
					ActivePlayers |= (1 << n);
			}

			if (Joining)
			{
				Joining=FALSE;
				Initiator=TRUE;
				BeginSyncPhase();
				JustJoined=TRUE;
				return false;
			}

			if (Host && !ResyncPhase)
			{
				_Agg.StopResetAggregator();
				_Agg.Running=TRUE;
			}
		}

		if (ResyncPhase)
		{
			AllowJoins=FALSE;	// dont allow people to join during a resync as it will get too messy
			if (!ReceiveSyncPackets())
				return false;

			Initiator=FALSE;
			InitBuffers();
			ResyncPhase=FALSE;

			if (GameType>TEAMPLAY)
			{
				WorldSyncPhase=TRUE;
				InitSyncData(false);
			}
			else
				DisplayResyncOKMessage();

			if (Host && !WorldSyncPhase)
			{
				_Agg.StopResetAggregator();
				_Agg.Running=TRUE;
			}
		}

		if (WorldSyncPhase)
		{
			if (!SyncCommsWorld())
				return false;

			DisplayResyncOKMessage();

			if (Host)
			{
				_Agg.StopResetAggregator();
				_Agg.Running=TRUE;
			}
		}

		if (firsttimein)
		{
			InitGameVars();
#ifdef INITCHECK
			FILE* fp=fopen("sync.txt","at");
			fprintf(fp,"initgamevars OK\n");
			fclose(fp);
#endif
		}

		if (!synched)
		{
			if (!InitSyncPhase())
				return false; 
#ifdef INITCHECK
			FILE* fp=fopen("sync.txt","at");
			fprintf(fp,"initsyncphase OK\n");
			fclose(fp);
#endif
		}

		if (!csync && synched)
		{
			if (!SecondSyncPhase())
				return false;
#ifdef INITCHECK
			FILE* fp=fopen("sync.txt","at");
			fprintf(fp,"secondsyncphase OK\n");
			fclose(fp);
#endif

			EnableJoins();
		}
	}
	resyncbar=false;											//AMM 07Jul99
	return true;
}

//------------------------------------------------------------------------------
//Procedure		WinMove
//Author		Andrew McRae
//Date			Thu 5 Sep 1996
//
//Description	Comms talking stuff
//
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void mobileitem::WinMove (int timeofday,WorldStuff* worldref)
{
	static ULong repeatcount=1000/_DPlay.RateDivider; // every 10 secs
 	DWORD n;

#ifdef WINTIME	
 	static ULong lasttime=_Miles.GetSysTime();
 	ULong ttime=_Miles.GetSysTime();

 	FILE* fp=fopen("wtime.txt","at");
 	fprintf(fp,"CALL %d\n",(ttime-lasttime)/10);
 	fclose(fp);
 	lasttime=ttime;
#endif
	if (_DPlay.Implemented)
	{
#ifdef TIMERTIME
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		ULong smlcount=count.LowPart;
		ULong Rtime=((smlcount-Qlasttimertime)*1000)/Qfreq;
		fprintf(timer,"%d %d %d Q %d Q %d Qt%d\n",timeGetTime()-timertime,timeGetTime()-lasttimertime,_DPlay.FrameCount,smlcount-Qtimertime,smlcount-Qlasttimertime,Rtime);
		Qlasttimertime=smlcount;
		lasttimertime=timeGetTime();
#endif

#ifdef	LOGHIST
		FILE* hist=fopen("histlog.txt","at");
		ULong g;

		_DPlay.HistPosBuffer[0].SetTempCurr();
		_DPlay.HistPosBuffer[1].SetTempCurr();

		fprintf(hist,"LOG\n");
		for (g=0;g<BUFFERLENGTH;g++)
		{
			fprintf(hist,
				"%d %d %d\n%d %d %d\n\n",
				_DPlay.HistPosBuffer[0].GetTemp()->Pos.X,
				_DPlay.HistPosBuffer[0].GetTemp()->Pos.Y,
				_DPlay.HistPosBuffer[0].GetTemp()->Pos.Z,
				_DPlay.HistPosBuffer[1].GetTemp()->Pos.X,
				_DPlay.HistPosBuffer[1].GetTemp()->Pos.Y,
				_DPlay.HistPosBuffer[1].GetTemp()->Pos.Z);

			_DPlay.HistPosBuffer[0].UpdateTemp();
			_DPlay.HistPosBuffer[1].UpdateTemp();
		}
		fprintf(hist,"END\n");
		fclose(hist);
#endif

// Set random list position
		_DPlay.SetRandomListPos();

// do this after random numbers have been synched!!!
		{
			_DPlay.ProcessInfoPackets();
			_DPlay.ApplyPartialDeltas();						//DAW 01Dec99

			if (_DPlay.ResyncPhase)
				return;
		}

		if (_DPlay.beginsyncflag)
		{
			_DPlay.beginsyncflag=false;
			_DPlay.Initiator=TRUE;
			_DPlay.BeginSyncPhase();
		}

// players have pressed accel but this player has not yet - repeat message every
// 10 secs until player complies or others get bored and deselect accel

		if (_DPlay.accelselected && _DPlay.accelselected!=(1<<_DPlay.mySlot) && !!Trans_Obj.View_Object->Accel())
		{
			if (repeatcount)
			{
				repeatcount--;
			}
			else
			{
				for (int loop=0;loop<MAXPLAYERS;loop++)
				{
					if (_DPlay.accelselected&(1<<loop))
					{
						ItemPtr	i=(ItemPtr)Persons2::ConvertPtrUID((UniqueID)_DPlay.AllocPacket[loop]);
						_DPlay.DisplayPlayerMessage(i,i,DPlay::ST_PLAYERSELACCEL);
					}
				}
				repeatcount=1000/_DPlay.RateDivider;
			}
		}

// need to sync random numbers again as launching weapons will use random numbers 
// differently!

		_DPlay.SetRandomListPos();

// move function 

		DosMove (timeofday,worldref);

		timeofday+=2;											//AMM 30Jun99
		_Replay.SetTimeOfDay(timeofday);						//AMM 30Jun99

// make packet (if its time to send one)

// if in accel dont bother sending packets

		if (!Trans_Obj.View_Object->Accel())
		{
 			if (++_DPlay.FrameInc==_DPlay.RateDivider)
 			{
	#ifdef LOGHIST
			COORDS3D *pos1,*pos2;
			FILE* hp=fopen("histlog.txt","at");
			pos1=_DPlay.GetGoodACPosition((UniqueID)_DPlay.AllocPacket[0]);
			if (pos1)
			{
				fprintf(hp,"%d: X=%d Y=%d Z=%d\n",
					_DPlay.FrameCount,
					pos1->X,
					pos1->Y,
					pos1->Z);
			}
			pos2=_DPlay.GetGoodACPosition((UniqueID)_DPlay.AllocPacket[1]);
			if (pos1)
			{
				fprintf(hp,"%d: X=%d Y=%d Z=%d\n",
					_DPlay.FrameCount,
					pos2->X,
					pos2->Y,
					pos2->Z);
			}
			fclose(hp);
	#endif
		
 				_DPlay.FrameInc=0;
 
 				_DPlay.MakeAndSendPacket();
			}
 		}
	}
	else
	{
// sync random numbers after 3d as well now before collision packets etc get processed

		_DPlay.SetRandomListPos();

		if (_Replay.prefscheck)
		{
			_Replay.prefscheck=false;
			if (_Replay.Record)
			{
				if (!Save_Data.gamedifficulty[GD_GUNCAMERAATSTART])
				{
// been to prefs, turned record off - stop
					_Replay.StopRecord();
				}
				else
				{
//been to prefs, still recording - begin new block in case prefs changed
					_Replay.BeginNewBlock();
				}
			}
			else if (!_Replay.Playback)
			{
// been to prefs, turned record on
				if (Save_Data.gamedifficulty[GD_GUNCAMERAATSTART])
					_Replay.StartRecordFlag=TRUE;
			}
		}

		if (_Replay.Record)
		{
			if (!_Replay.StoreDeltas())
			{
				_Replay.Record=FALSE;
				_Replay.CloseRecordLog();
			}
		}
		else if (_Replay.Playback)
		{
			_Replay.ResetFlags();

 			if (!_Replay.LoadDeltas())
			{
				{
					_Replay.PlaybackPaused=TRUE;				//AMM 30Jun99
					OverLay.pCurScr=OverLay.pNewScr=&OverLay.replayScr;
					return; // dont do dosmove if paused......
				}
			}
		}
		else
		{
			_Replay.UpdateGhostAC();
		}

// sync random numbers after packets have been processed as things like launching bullets 
// will cause extra random numbers to be used before AI etc

		_DPlay.SetRandomListPos();

		DosMove (timeofday,worldref);

		timeofday+=2;											//AMM 30Jun99
		_Replay.SetTimeOfDay(timeofday);						//AMM 30Jun99
																	  //DAW 30/06/99
		if (_Replay.Record)										//AMM 29Jun99
		{
			if (_Replay.StoreHeader)
			{
				_Replay.StoreHeader=false;

				if (!_Replay.StoreBlockHeader())
				{
					_Replay.Record=FALSE;
					_Replay.CloseRecordLog();
				}
			}
		}
		else if (_Replay.Playback)
		{
			if (_Replay.LoadHeader)
			{
				_Replay.LoadHeader=false;

				UByteP temp=_Replay.playbackfilepos;

				_Replay.DoSmokeTrailStuff=FALSE;
				if (!_Replay.LoadBlockHeader())
				{
					_Replay.DoSmokeTrailStuff=TRUE;
					_Replay.playbackfilepos=temp;
					_Replay.PlaybackPaused=TRUE;
					OverLay.pCurScr=OverLay.pNewScr=&OverLay.replayScr;//AMM 30Jun99
				}
				_Replay.DoSmokeTrailStuff=TRUE;
			}
		}
		else if (_Replay.StartRecordFlag)
		{
 			_Replay.StartRecordFlag=FALSE;

			if (_Replay.OpenRecordLog())
			{
				if (!_Replay.SuperHeaderStored)
					_Replay.StoreSuperHeader();

				if (_Replay.StoreBlockHeader())
				{
					_Replay.Record=TRUE;
					_Replay.badrecord=false;

					_DPlay.ResetDeltas();
					_DPlay.InitBuffers();

// this will ensure weapon packet is in first packet

					if (_Replay.DelayedWeapon>=0)
					{
						_DPlay.NewBullet(1,_Replay.DelayedWeapon);
						_Replay.DelayedWeapon=-1;
					}
					else if (_Replay.DelayedGearUp)
					{
						_Replay.DelayedGearUp=false;
						_DPlay.SendAccelGearUpMessage();
					}
				}
			}
// clean up if something wrong with record

			if (_Replay.DelayedWeapon>=0)
			{
				_DPlay.NewBullet(1,_Replay.DelayedWeapon);
				_Replay.DelayedWeapon=-1;
			}
			if (_Replay.DelayedGearUp)
			{
				_Replay.DelayedGearUp=false;
				_DPlay.SendAccelGearUpMessage();
			}
		}	
	}
#ifdef WINTIME	
 	ttime=_Miles.GetSysTime();

 	fp=fopen("wtime.txt","at");
 	fprintf(fp,"TIME %d\n",(ttime-lasttime)/10);
 	fclose(fp);
#endif
}

//------------------------------------------------------------------------------
//Procedure		StorePacket
//
//Description	
//				Store packet in buffer
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::StorePacket (LPAGGSENDPACKET AggPacket)				//AMM 22Sep99
{
	SWord	n,diff,m;
	SWord	thispacketcount;
	SWord	targetcount;
	SLong	sub;

	AGGSENDPACKET packet;										//AMM 22Sep99

// expand sent packet into full packet

	ExpandAggPacket(&packet, AggPacket);						//AMM 22Sep99

#ifdef FILELOGS
			FILE* fp=fopen("aggpack.txt","at");
#endif
#ifdef FILELOGS
			fprintf(fp,"PacketFrame %d CurrentFrame %d\n%d %d %d %d %d\n%d %d %d %d %d\n",
				packet.Count,
				FrameCount,
				packet.player[0].Count,
				packet.player[0].X,
				packet.player[0].Y,
				packet.player[0].Z,
				packet.player[0].IDCode,
				packet.player[1].Count,
				packet.player[1].X,
				packet.player[1].Y,
				packet.player[1].Z,
				packet.player[1].IDCode);
#endif

// took out to get 8 player network going
// update all ack info, even if I have already received this packet, and send
// acks in case they have got lost

#ifdef FILELOGS
			if (packet.player[0].Ack1)
				fprintf(fp,"got ACK for %d from p0\n",packet.player[0].Ack1);
			if (packet.player[0].Ack2)
				fprintf(fp,"got ACK for %d from p0\n",packet.player[0].Ack2);
			if (packet.player[1].Ack1)
				fprintf(fp,"got ACK for %d from p1\n",packet.player[1].Ack1);
			if (packet.player[1].Ack2)
				fprintf(fp,"got ACK for %d from p1\n",packet.player[1].Ack2);
#endif
	UpdateAckBuffer(&packet);
#ifdef FILELOGS
				fprintf(fp,"add %d for ACK\n",packet.Count);
#endif
	RecPackBuffer.GetNext()->packet=packet.Count;
	RecPackBuffer.AddEntryAndUpdateNext();

	for (n=0;n<MAXPLAYERS;n++)
	{
		if (packet.player[n].IDCode!=PIDC_PACKETERROR)
		{
// got a real packet from this player
			PacketBuffer.SetTempCurr();

			thispacketcount=packet.player[n].Count;
			targetcount=PacketBuffer.GetCurr()->Count;

			sub=thispacketcount-targetcount;

			if (((thispacketcount>=targetcount && sub<128)
			|| (thispacketcount<targetcount && abs(sub)>128)))
			{
// packet is within range I want

				if (thispacketcount>=targetcount)
					diff=sub;
				else
					diff=(sub)+256;

#ifdef FILELOGS
				fprintf(fp,"add to packetbuff.curr=%d updating by %d \n",PacketBuffer.GetCurr()->Count,diff);
				if (diff>=BUFFERLENGTH)
				{
// buffer about to wrap - not good

					_Error.SayAndQuit("buffer wrap");
				}
#endif

				while (diff--)
					PacketBuffer.UpdateTemp();

				if (!PacketBuffer.GetTemp()->Received[n])
				{
// have not already received this packet

					if (H2H_Player[n].dpid==myDPlayID)
					{
// can sent packet buffer be updated?

						if (packet.player[n].IDCode!=PIDC_INITPACK)
						{
// a real delta packet
							SendPackBuffer.SetTempCurr();

							targetcount=SendPackBuffer.GetCurr()->packet.Count;

							sub=thispacketcount-targetcount;

							if (((thispacketcount>=targetcount && sub<128)
							|| (thispacketcount<targetcount && abs(sub)>128)))
							{
// if within range from current up to half buffer

								if (thispacketcount>=targetcount)
									diff=sub;
								else
									diff=(sub)+256;

#ifdef FILELOGS
								fprintf(fp,"sendpackbuff.curr=%d updating by %d \n",SendPackBuffer.GetCurr()->packet.Count,diff);
#endif
								while (diff--)
									SendPackBuffer.UpdateTemp();

// not sure if these checks are needed. I know I want this packet, if counts are not same
// then something has gone badly wrong, and I have already checked about receiving this packet
// so done should always be false.....????

								if (packet.player[n].Count==SendPackBuffer.GetTemp()->packet.Count
								&& (!SendPackBuffer.GetTemp()->Done))
								{
// amnyway is this is one I want register as done and see if buffer can be updated

									SendPackBuffer.GetTemp()->Done=TRUE;

									while (SendPackBuffer.GetCurr()->Done)
									{
#ifdef FILELOGS
									fprintf(fp,"update currspb %d\n",SendPackBuffer.GetCurr()->packet.Count);
#endif
										SendPackBuffer.GetCurr()->Done=FALSE;
										SendPackBuffer.UpdateCurr();
									}
								}
							}
						}
					}

// register as received and copy into buffer

					PacketBuffer.GetTemp()->Received[n]=TRUE;
					CopyBuffer((char*)&PacketBuffer.GetTemp()->PlayerData[n],(char*)&packet.player[n],sizeof(ACPACKET));

#ifdef FILELOGS
					fprintf(fp,"processing %d\n",packet.player[n].Count);
					fflush(fp);
#endif

// process move packet here, but not anything else. Must wait for all players
// to have all packets from this frame so that AI is on sync when collsions,
// near misses etc occur.

					if (ActivePlayers&(1<<n))
					{
						if (packet.player[n].IDCode<PIDC_AIACPOSITION1)
						{
							ProcessDeltaPacket(&(PacketBuffer.GetTemp()->PlayerData[n]),n);

							UpdateHistBuffer(&PacketBuffer.GetTemp()->PlayerData[n],n);

 							CalcVels(n,PacketBuffer.GetTemp()->PlayerData[n].Count);

#ifdef CVELCHECK
							fp=fopen("vels.txt","at");
							if (n==mySlot)
							{
								AirStrucPtr sac=Persons2::PlayerSeenAC;
								AirStrucPtr gac=Persons2::PlayerGhostAC;
								fprintf(fp,"my vx=%d vy=%d vz=%d\n",gac->velx,gac->vely,gac->velz);
								fprintf(fp,"fmvx=%f fmvy=%f fmvz=%f\n\n",sac->fly.pModel->Vel.x,
									sac->fly.pModel->Vel.y,sac->fly.pModel->Vel.z);
							}
							else
							{
								AirStrucPtr ac=*Persons2::ConvertPtrUID((UniqueID)AllocPacket[n]);
								fprintf(fp,"other vx=%d vy=%d vz=%d\n\n",ac->velx,ac->vely,ac->velz);

							}
							fclose(fp);
#endif

						}
						else
						{
// dont process but still add to hist buffer

							AddPacketToHistBuffer(&PacketBuffer.GetTemp()->PlayerData[n],n);
						}
					}
				}
				else
				{
#ifdef FILELOGS
					fprintf(fp,"packet %d already received temp=%d\n",packet.player[n].Count,PacketBuffer.GetTemp()->Count);
#endif
				}
			}
			else
			{
#ifdef FILELOGS
			 	fprintf(fp,"packet %d not in range\n",packet.player[n].Count);
#endif
			}
		}
	}

	Bool update=TRUE;

	while (update)
	{
		m=0;
		gotfullpacket=FALSE;

		for (n=0;n<MAXPLAYERS;n++)
		{
			if (PacketBuffer.GetCurr()->Received[n])
			{
				m++;
			}
		}

		if (m==CurrPlayers)
		{
			gotfullpacket=TRUE;
		}
		else
		{
			update=FALSE;
		}

		if (gotfullpacket)
		{
// this is the last frame in which everybody has received the aggregated packet

			gotfullpacket=FALSE;

#ifdef LOGHIST
			FILE* hist=fopen("histlog.txt","at");
			fprintf(hist,"update hist log\n");
			fprintf(hist,"[0]=%d %d %d %d %d\n[1]=%d %d %d %d %d\n\n",
				PacketBuffer.GetCurr()->PlayerData[0].Count,
				PacketBuffer.GetCurr()->PlayerData[0].Shift,
				PacketBuffer.GetCurr()->PlayerData[0].X,
				PacketBuffer.GetCurr()->PlayerData[0].Y,
				PacketBuffer.GetCurr()->PlayerData[0].Z,
				PacketBuffer.GetCurr()->PlayerData[1].Count,
				PacketBuffer.GetCurr()->PlayerData[1].Shift,
				PacketBuffer.GetCurr()->PlayerData[1].X,
				PacketBuffer.GetCurr()->PlayerData[1].Y,
				PacketBuffer.GetCurr()->PlayerData[1].Z);

			fclose(hist);
#endif

// can update histposbuffer

			for (n=0;n<MAXPLAYERS;n++)
				HistPosBuffer[n].UpdateCurr();

// number of entries so far updated in buffer for vels purposes

			if (velcounter<POSBSIZE)
				velcounter++;

// now that everybody has got all packets from this frame we can process
// collisions, near misses etc.
#ifdef FILELOGS
			 	fflush(fp);
#endif
			for (n=0;n<MAXPLAYERS;n++)
			{
				if (ActivePlayers&(1<<n))
				{
					ProcessExtraPacket(&(PacketBuffer.GetCurr()->PlayerData[n]),n);
				}
// indicate no packet for this slot

				PacketBuffer.GetCurr()->PlayerData[n].IDCode=PIDC_PACKETERROR;
				PacketBuffer.GetCurr()->Received[n]=FALSE;
			}

// update current packet to process pointer

// this is used to add 1 entry before updating curr, because the way that packets are
// put into the buffer the count of actual packets in it is impossible to keep.

			PacketBuffer.AddEntry(); 

#ifdef FILELOGS
			fprintf(fp,"update packetbuffer.curr %d\n",PacketBuffer.GetCurr()->Count);
#endif

			UByte x;

			x=PacketBuffer.GetCurr()->Count;
			PacketBuffer.UpdateCurr();
			PacketBuffer.GetCurr()->Count=x+1;

		}
	}
#ifdef FILELOGS
	fprintf(fp,"\n");
	fclose(fp);
#endif
}

//------------------------------------------------------------------------------
//Procedure		ProcessDeltaPacket
//Author		Andrew McMaster
//
//Description	Update the predicted ac with new deltas
//
//Inputs		slot = slot of sender of packet to be processed
//				lpAcPacket is packet to be processed
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::ProcessDeltaPacket (LPACPACKET lpAcPacket,ULong slot)
{
	UByte RecCode=(lpAcPacket->IDCode)>>6;	// type of packet
	UByte Mess=lpAcPacket->IDCode;
	ItemBasePtr lpOpAc;
	AirStrucPtr Me;

	lpOpAc=Persons2::ConvertPtrUID((UniqueID)AllocPacket[slot]);

	if (lpOpAc)
	{
// Get AirStrucPtr for the Packet's Aircraft

		if (lpOpAc->MigStatus.LaunchFlags.size == AirStrucSize)
		{
			Me = (AirStrucPtr)lpOpAc;

			if (RecCode==PT_SPECIAL)
			{
				switch (Mess)
				{
				case PIDC_PACKETERROR:
				case PIDC_INITPACK:
// packets which dont have valid deltas, ignore 

					break;
				default:
// all the rest do have valid deltas
				AddDeltasToApply(lpAcPacket,slot);				//DAW 01Dec99
				}
			}
			else
			{
				if (Me==Persons2::PlayerSeenAC)
				{
					AddDeltasToApply(lpAcPacket,slot);				//DAW 01Dec99
				}
				else
				{
					AddDeltasToApply(lpAcPacket,slot);				//DAW 01Dec99
// can launch bullets now as they dont have any actual effect 

					if (RecCode==PT_BULLET)
					{
						ProcessBulletPacket((LPBASICPACKET)lpAcPacket,Me);
					}
				}
			}
 		}
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessExtraPacket
//Author		Andy McMaster
//Date			Tue 25 Aug 1998
//
//Description	Process extra packet info
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::ProcessExtraPacket (LPACPACKET lpAcPacket,ULong player_slot)
{
	UByte RecCode=(lpAcPacket->IDCode)>>6;	// type of packet
	ItemBasePtr lpOpAc;
	AirStrucPtr Me;

#ifdef FILELOGS1
	FILE* fp=fopen("aggpack.txt","at");
	fprintf(fp,"%d: ",lpAcPacket->Count);
#endif

	lpOpAc=Persons2::ConvertPtrUID((UniqueID)AllocPacket[player_slot]);

	if (lpOpAc)
	{
// Get AirStrucPtr for the Packet's Aircraft

		if (lpOpAc->MigStatus.LaunchFlags.size == AirStrucSize)
		{
			Me = (AirStrucPtr)lpOpAc;

			switch (RecCode)
			{
			case PT_UIDS:
				ProcessUIDSPacket((LPBASICPACKET)lpAcPacket,player_slot);
				break;

			case PT_SPECIAL:
				ProcessSpecialPacket((LPBASICPACKET)lpAcPacket,Me,player_slot);
				break;

			case PT_COLLISION:
				ProcessCollisionPacket((LPBASICPACKET)lpAcPacket,player_slot);
				break;

			case PT_BULLET:
// bullets now launched when received, keeps look up to date and as they
// dont have any actual effect on anything doesnt matter that they are 
// not correct frame

				break;
			}

 		}
	}
#ifdef FILELOGS1
	fclose(fp);
#endif

}

//------------------------------------------------------------------------------
//Procedure		ResurrectMe
//Author		Andrew McRae
//Date			Sat 14 Dec 1996
//
//				Its function is to resurrect an ac and send packet to start resurection
//				for same plane on other machines
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::ResurrectMe (AirStrucPtr ac,bool suicide)
{
	static ULong time=0;

	if (Implemented)
	{
// set pilot to be dead

		if (GameType>TEAMPLAY)
		{
			int deaderpilot=-1;

			if (ac->ai.PilotSkills.pilotnum<ai_info::PROPER_PILOT_MAX)
			{
				deaderpilot=ac->ai.PilotSkills.pilotnum;
			}
			if (deaderpilot>=0)
			{
				if (MMC.Active_Pilots[deaderpilot].status!=KIA)
				{
					MMC.Active_Pilots[deaderpilot].status=KIA;
					SendPilotsStatus(true,deaderpilot);
				}
			}
		}

		if (viewtime)
		{
			viewtime=false;
			time=_Miles.GetSysTime()+5000;
		}
		else
		{
			if (_Miles.GetSysTime()>time || suicide)
			{

// only resurrect if in DEATHMATCH or TEAMPLAY
// otherwise player must exit to ready room and select new ac and rejoin

				if (GameType<COMMSQUICKMISSION)
				{
// remove any bullet packets now that I am dead

					BulletBuffer.InitBuffer();

					if (ac->AutoMoveInfo.movecode != AUTO_RESURRECT)
					{
						ResurrectAircraft (ac, TRUE);

// always send
						NewSpecial(PIDC_RESURRECT,0,0,0);
					}
				}
			}
		}
	}	
}

//------------------------------------------------------------------------------
//Procedure		ResurrectAircarft
//Author		Andrew McRae
//Date			Fri 22 Nov 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::ResurrectAircraft (AirStrucPtr ControlledAC, Bool Forced)
{
	if (Implemented)
	{
		InitResurrect (ControlledAC);
		RepairAircraft (ControlledAC);
	}
}

//------------------------------------------------------------------------------
//Procedure		RepairAircraft
//Author		Andrew McRae
//Date			Sat 7 Dec 1996
//
//Description	Repair an Aircraft ready for flight etc,
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::RepairAircraft (AirStrucPtr ac)
{
	if (ac->MigStatus.LaunchFlags.size!=AirStrucSize)							
		_Error.SayAndQuit("Repair ac not aircraftsize");		

	ULong n;

	ac->ai.PilotSkills.radiosilent=FALSE;

	SHAPE.DetatchAllVapourStreams(ac);
	for (n=0; n<MAXPLAYERS; n++)
	{
		if (AllocPacket [n] == ac->uniqueID.bitfieldUid.count)
		{
			if (!SHAPE.ResetAnimData_NewShape(ac,ac->classtype->visible,ac->weap.AircraftStatus.Weapons))
				_Error.SayAndQuit("Wrong anim data");
		
			break;
		}
	}

	ac->fly.redeffect = 0;	
	Land_Scape.blackingout = 0;
	Land_Scape.whitingout = 0;

	ac->MigStatus.LaunchFlags.deadtime = 0;

// thrust
	if (ac == Persons2::PlayerSeenAC)
	{
		ac->fly.thrustpercent = 100;
		ac->weap.AircraftStatus.FuelDumped = FALSE; // allow rip stores again
	}

	MinAnimData* mad = (MinAnimData* )ac->Anim;					//DAW 22Jun99
	mad->itemstate = ALIVE;
	mad->IsInvisible=0;
}

//------------------------------------------------------------------------------
//Procedure		InitResurrect
//Author		Andrew McRae
//Date			Sat 7 Dec 1996
//
//Description	Set up Aircraft to use AUTO_RESURRECT
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::InitResurrect (AirStrucPtr ac)
{
	if (Implemented)
	{
		ac->AutoMoveInfo.movecode = AUTO_RESURRECT;	

		if (ac == Persons2::PlayerSeenAC)
		{
			Persons2::PlayerGhostAC->AutoMoveInfo.movecode=AUTO_RESURRECT;
			Manual_Pilot.controlmode=ManualPilot::PILOTDEAD;
			Trans_Obj.View_Object->SetToPiloted((itemptr)ac);
			ac->fly.pModel->Initialise(ac,TRUE);
		}

 		Key_Tests.KeyPress3d(RESURRECTKEY);
	}
}

#ifndef NDEBUG
//------------------------------------------------------------------------------
//Procedure		DosMove
//Author		Andrew McRae
//Date			Wed 9 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
static const char* GetMoveCode(AutoMoveCodeTypeSelect a)
{
	switch (a)
	{
	case AUTO_FOLLOWWP:         return "AUTO_FOLLOWWP=0, ";
	case AUTO_NOPPILOT:         return "AUTO_NOPPILOT, ";
	case AUTO_WAIT4TIME:        return "AUTO_WAIT4TIME, ";
	case AUTO_TAXI:             return "AUTO_TAXI, ";
	case AUTO_TAKEOFF:          return "AUTO_TAKEOFF, ";
	case GROUND_TAXI:		    return "GROUND_TAXI, ";
	case AUTO_TELLLEADER:		return "AUTO_TELLLEADER,	 ";
		                                              
	case AUTO_SPIRAL2GROUND:	return "AUTO_SPIRAL2GROUND,	 ";
	case AUTO_RESURRECT:		return "AUTO_RESURRECT,		 ";
	case AUTO_DEATHSEQUENCE:	return "AUTO_DEATHSEQUENCE,	 ";
	case AUTO_TRAININGPREMERGE: return "AUTO_TRAININGPREMERGE, ";
	case AUTO_TRAININGACM:	    return "AUTO_TRAININGACM,	 ";
	case AUTO_CRASHSKID:		return "AUTO_CRASHSKID,		 ";
	case AUTO_CRASHROLL:		return "AUTO_CRASHROLL,		 ";
		                                              
	case AUTO_BOMB:             return "AUTO_BOMB, ";
	case AUTO_COVER:            return "AUTO_COVER, ";
	case AUTO_STRAFFE:          return "AUTO_STRAFFE, ";
	case AUTO_BALLOON:          return "AUTO_BALLOON, ";
	case AUTO_RECONN:           return "AUTO_RECONN, ";
	case AUTO_ARTSPOT:          return "AUTO_ARTSPOT, ";
	case AUTO_HITWATER:		    return "AUTO_HITWATER,		 ";
		                                              
	case AUTO_TRACKINGBOGIE:	return "AUTO_TRACKINGBOGIE,	 ";
	case AUTO_TRACKINGBANDIT:   return "AUTO_TRACKINGBANDIT, ";
	case AUTO_PRECOMBAT:		return "AUTO_PRECOMBAT,		 ";
	case AUTO_LANDING:          return "AUTO_LANDING, ";
	case AUTO_FAC:              return "AUTO_FAC, ";
	case AUTO_CAS:              return "AUTO_CAS, ";
	case AUTO_ACCELERATE:       return "AUTO_ACCELERATE, ";
	case AUTO_MISSIONDICETHROW: return "AUTO_MISSIONDICETHROW, ";
	case AUTO_COMMSACCEL:	    return "AUTO_COMMSACCEL,	 ";
	case GROUND_TOGLOBREF:      return "GROUND_TOGLOBREF, ";
	case	AUTO_COMBAT:		return "AUTO_COMBAT ";
	}
	return"*** NO VALID MOVECODE ***";
}
#endif
void mobileitem::DosMove (int timeofday,WorldStuff* worldref)
{
	timerseed=UByte(timeofday);
	mobileitem::timeofday=timeofday;
	WorldStuff &world = *worldref;

	Art_Int.VisibleCheck();

#ifndef EMIT_HM_DATA
	mobileitem::MoveAll(world);	
#endif

	if (Three_Dee.livelist)
		Three_Dee.livelist->Event();

	TransientItem::MoveAllTrans(world);

	Persons2::PlayerGhostAC->CheckManualCourse();					  //RDH 13/07/99
	//other code may change these in ghost							  //JIM 06/05/99
	Persons2::PlayerSeenAC->waypoint=Persons2::PlayerGhostAC->waypoint;
	Persons2::PlayerSeenAC->leader=Persons2::PlayerGhostAC->leader;
	Persons2::PlayerSeenAC->follower=Persons2::PlayerGhostAC->follower;
	Persons2::PlayerSeenAC->fly.nextflight=Persons2::PlayerGhostAC->fly.nextflight;
	Persons2::PlayerSeenAC->fly.leadflight=Persons2::PlayerGhostAC->fly.leadflight;
	Persons2::PlayerSeenAC->ai.unfriendly=Persons2::PlayerGhostAC->ai.unfriendly;
	Persons2::PlayerSeenAC->FormationInfo.formation = Persons2::PlayerGhostAC->FormationInfo.formation;	//CSB 08/07/99
#ifndef NDEBUG
	static int monoline=0;
	if (Persons2::PlayerSeenAC->AutoMoveInfo.movecode!=Persons2::PlayerGhostAC->AutoMoveInfo.movecode)
	{
		Mono_Text.PrintAt(0,monoline,(UByte*)GetMoveCode(Persons2::PlayerSeenAC->AutoMoveInfo.movecode));
		Mono_Text.PrintAt(25,monoline,(UByte*)"<--");
		Mono_Text.PrintAt(30,monoline,(UByte*)GetMoveCode(Persons2::PlayerGhostAC->AutoMoveInfo.movecode));
		monoline++;
		if (monoline>=25) monoline-=25;
		Mono_Text.PrintAt(0,monoline,(UByte*)"................................................");
	}
#endif
	Persons2::PlayerSeenAC->AutoMoveInfo.movecode=Persons2::PlayerGhostAC->AutoMoveInfo.movecode;


	if (!_Replay.Playback)
	{
		#ifndef EMIT_HM_DATA
		Manual_Pilot.MainManualPilot();
		#endif
	}
	else
 	{
		UWord	oldsector=	world.GetSector(Persons2::PlayerSeenAC);

		Persons2::PlayerSeenAC->World.X=Persons2::PlayerGhostAC->World.X;
		Persons2::PlayerSeenAC->World.Y=Persons2::PlayerGhostAC->World.Y;
		Persons2::PlayerSeenAC->World.Z=Persons2::PlayerGhostAC->World.Z;
		Persons2::PlayerSeenAC->hdg.a=Persons2::PlayerGhostAC->hdg.a;
		Persons2::PlayerSeenAC->pitch.a=Persons2::PlayerGhostAC->pitch.a;
		Persons2::PlayerSeenAC->roll.a=Persons2::PlayerGhostAC->roll.a;
		Persons2::PlayerSeenAC->vel=Persons2::PlayerGhostAC->vel;

// set up animations for flaps,brakes

		PAERODEVICE pAeroDevice = Persons2::PlayerSeenAC->fly.pModel->DeviceList;
		while (pAeroDevice != NULL)
		{
			pAeroDevice->Action (Persons2::PlayerSeenAC, AeroDevice::CT_AUTO,FALSE);
			pAeroDevice = pAeroDevice->List.NextItem ();
		}

// do animations for flaps, brakes
		Persons2::PlayerSeenAC->fly.pModel->Animations(Persons2::PlayerSeenAC);

		UWord	newsector=	world.GetSector(Persons2::PlayerSeenAC);
		if (newsector!=oldsector)
		{
			world.RemoveFromSector(Persons2::PlayerSeenAC,oldsector);
			world.AddToWorld(Persons2::PlayerSeenAC);
		}
		Persons2::PlayerSeenAC->uniqueID.bitfieldUid.changed = FALSE;		
	}

// collisions against non-transient items

	if (!_Replay.Playback)										//AMM 08Jan99
	{
		Trans_Obj.PilotedACHit();					//make this last - 4 comms
	}

	#ifdef EMIT_HM_DATA
		Persons2::PlayerSeenAC->fly.pModel->EmitHMData(Manual_Pilot.ControlledAC2);
	#endif

	if (!_Replay.Playback)												//RJS 05May99
	{
		if (Manual_Pilot.controlmode==ManualPilot::MANUAL)				//RJS 05May99
			Trans_Obj.ControlKeybWeap(Persons2::PlayerSeenAC,world);	//RJS 05May99
		else
			_Miles.StopShooting();								//DAW 09Jun99

		//We do still want to be able to eject when we want to...
		if (Key_Tests.KeyPress3d(EJECTPILOT))						//RJS 05May99			
 		{
			if (!Persons2::PlayerSeenAC->fly.pModel->GearTouched)
			{
 				if (_DPlay.Implemented)
				{
					if (_DPlay.GameType>TEAMPLAY)
 						_DPlay.NewSpecial(PIDC_EJECT,0,0,0);			
				}
				else if (_Replay.Record)
				{
 					_DPlay.NewSpecial(PIDC_EJECT,0,0,0);			
				}
 				else
				{
	 				Trans_Obj.LaunchCanopyEject(Persons2::PlayerSeenAC,world);					
				}
			}
		}
	}
	timerseed=0; 
	//other code (weapons) may change these in seen							  //JIM 06/05/99

	Persons2::PlayerGhostAC->waypoint=		Persons2::PlayerSeenAC->waypoint;
	Persons2::PlayerGhostAC->leader=		Persons2::PlayerSeenAC->leader;
	Persons2::PlayerGhostAC->follower=		Persons2::PlayerSeenAC->follower;
	Persons2::PlayerGhostAC->fly.nextflight=Persons2::PlayerSeenAC->fly.nextflight;
	Persons2::PlayerGhostAC->fly.leadflight=Persons2::PlayerSeenAC->fly.leadflight;
	Persons2::PlayerGhostAC->ai.unfriendly=Persons2::PlayerSeenAC->ai.unfriendly;
	Persons2::PlayerGhostAC->FormationInfo.formation = Persons2::PlayerSeenAC->FormationInfo.formation;	//CSB 08/07/99
#ifndef NDEBUG
	if (Persons2::PlayerSeenAC->AutoMoveInfo.movecode!=Persons2::PlayerGhostAC->AutoMoveInfo.movecode)
	{
		Mono_Text.PrintAt(0,monoline,(UByte*)GetMoveCode(Persons2::PlayerSeenAC->AutoMoveInfo.movecode));
		Mono_Text.PrintAt(25,monoline,(UByte*)"==>");
		Mono_Text.PrintAt(30,monoline,(UByte*)GetMoveCode(Persons2::PlayerGhostAC->AutoMoveInfo.movecode));
		monoline++;
		if (monoline>=25) monoline-=25;
		Mono_Text.PrintAt(0,monoline,(UByte*)"...................................");
	}
#endif
	Persons2::PlayerGhostAC->AutoMoveInfo.movecode=Persons2::PlayerSeenAC->AutoMoveInfo.movecode;
}

//------------------------------------------------------------------------------
//Procedure		SendPacketToAggregator
//Author		Andy McMaster
//Date			Thu 20 Mar 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool DPlay::SendPacketToAggregator(LPACPACKET lpAcPacket)
{
	myDPID To=aggID,From=myDPlayID;
	HRESULT res; 

	res=lpDP4->SendEx(From,To,DPSEND_ASYNC|DPSEND_NOSENDCOMPLETEMSG,lpAcPacket,sizeof(ACPACKET),0,0,NULL,NULL);

#ifdef FILELOGS
	if (res!=DP_OK && res!=DPERR_PENDING)
	{
		FILE* fp=fopen("sendpack.txt","at");
		fprintf(fp,"FAILED TO SEND %d error=%d\n",lpAcPacket->Count,res);
		fclose(fp);
	}
#endif

	if (res==DP_OK || res==DPERR_PENDING)
		return TRUE;
	else
		return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		SendInitPacket
//Author		Andrew McRae
//Date			Thu 5 Sep 1996
//
//Description	Send First Initialisation Packet
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void SendInitPacket ()
{
	_Replay.PlaybackGameState=0;	 //clear flag that we may be playing back on exit
	if (_DPlay.Implemented)
	{
		UWord RndPacket [RNDPACKETSIZE];
		ULong n,From,To;
		HRESULT res;
		DWORD	packetlen = RNDPACKETSIZE * sizeof(UWord);
		char* Buffer;
		ULong BufLen;
		Bool gotone,sentOK=FALSE;

		if (_DPlay.Joining)
		{
			COMMONDATA p;

			p.PacketID=PID_RANDREQUEST;
			_DPlay.SendMessageToPlayers((char*)&p,sizeof(COMMONDATA));
				
// get initial random numbers

			ULong to=_DPlay.myDPlayID;
			while (_DPlay.ReceiveNextMessage(Buffer,BufLen,From,to,DPRECEIVE_TOPLAYER))
			{
				if (From==0)
				{
					_DPlay.ProcessSystemMessage(Buffer,CPS_GAMESTARTUP);
				}
				else if (From!=_DPlay.aggID)
				{
					if (*(ULong*)Buffer == PID_RANDOMLIST)		//AMM 12Jan00
					{
						UWord* ptr;
						ptr = (UWord*)Buffer;
						ptr+=sizeof(ULong);// skip PID
						for (n=0; n<RNDPACKETSIZE; n++)
							RndPacket[n] = *ptr++;

						for (n=0; n<(RNDPACKETSIZE - 2); n++)
							Math_Lib.SetRndLookUp(n,RndPacket[n]);

						Math_Lib.Setbval(RndPacket[(RNDPACKETSIZE - 2)]);
						Math_Lib.Setcval(RndPacket[(RNDPACKETSIZE - 1)]);

						break;
					}
					else
					{
						_DPlay.ProcessPlayerMessage(Buffer,BufLen,From,CPS_GAMESTARTUP);
					}
				}
			}

			for (int sq=0;sq<=SQ_R_NONFLYABLE;sq++)				
				GR_Pack_Sq_Used[sq]=0;

			return;
		}

		if (_DPlay.Host)
		{
// Send Random List
			
			for (n=0; n<(RNDPACKETSIZE - 2); n++)
				RndPacket[n] = Math_Lib.GetRndLookUp(n);

			RndPacket[(RNDPACKETSIZE - 2)] = Math_Lib.Getbval();
			RndPacket[(RNDPACKETSIZE - 1)] = Math_Lib.Getcval();

			sentOK=_DPlay.SendMessageToPlayers((char*)&RndPacket, ( 57 * sizeof (UWord)) );
			_Replay.BackupGRRandomList();
		}
		else
		{
// Receive Random List

			HRESULT res;

			gotone=FALSE;

// NEEDTIMEOUT
			ULong now=_Miles.GetSysTime();

			CommsTimeout time(now,_DPlay.CommsTimeoutLength); 

			while (!gotone)
			{
				now=_Miles.GetSysTime();

				if (time.TimedOut(now))
					_Error.SayAndQuit("Timed out (SIP)");

				ULong to=_DPlay.myDPlayID;
				while (_DPlay.ReceiveNextMessage(Buffer,BufLen,From,to,DPRECEIVE_TOPLAYER))
				{
					if (From==0)
					{
						_DPlay.ProcessSystemMessage(Buffer,CPS_GAMESTARTUP);
					}
					else if (From!=_DPlay.aggID)
					{
						if (BufLen == (RNDPACKETSIZE * sizeof(UWord)))
						{
							UWord* ptr;
							ptr = (UWord*)Buffer;
							for (n=0; n<RNDPACKETSIZE; n++)
								RndPacket[n] = *ptr++;

							for (n=0; n<(RNDPACKETSIZE - 2); n++)
								Math_Lib.SetRndLookUp(n,RndPacket[n]);

							Math_Lib.Setbval(RndPacket[(RNDPACKETSIZE - 2)]);
							Math_Lib.Setcval(RndPacket[(RNDPACKETSIZE - 1)]);

							gotone=TRUE;

							break;
						}
						else
						{
							_DPlay.ProcessPlayerMessage(Buffer,BufLen,From,CPS_GAMESTARTUP);
						}
					}
				}
			}
		}

// need to clear squads used here. It is done by host when determining new BField,
// but as guests only process BFields it will not get done, so do it here for all.
		for (int sq=0;sq<=SQ_R_NONFLYABLE;sq++)				//AMM 02Jun99
			GR_Pack_Sq_Used[sq]=0;
	}
	else
	{
		_Replay.BackupGRRandomList();
		_Replay.GetInitialGlobRefs();
		_Replay.ResetReplayData();
		_DPlay.firstdeathcall=true;
		_Replay.DelayedWeapon=-1;
		_Replay.DelayedGearUp=false;
		_Replay.StartRecordFlag=FALSE;
		_Replay.StoreHeader=false;
		_Replay.LoadHeader=false;

// set up replay stuff

// no need, divider not looked at in replay
//		_DPlay.RateDivider=1;

// need to set these up even if we are not in record in case record is started during game

		_Replay.PlayerSquad=MMC.playersquadron;
		_Replay.PlayerPos=MMC.playeracnum;

		if (_Replay.Playback)
		{
			_Replay.CloseRecordLog();

			_Replay.Playback=FALSE;

			if (_Replay.OpenPlaybackLog())
			{
				if (_Replay.LoadSuperHeaderBeginning())
				{
					_Replay.currentry=_Replay.bfgrlist;
					_Replay.Playback=TRUE;
				}
			}

			if (!_Replay.Playback)
				_Error.EmitSysErr("Error reading playback log");
		}
		else
		if (_Replay.Record)
		{
			_Replay.Record=FALSE;
			if (_Replay.OpenRecordLog())
			{
				if (_Replay.StoreSuperHeaderBeginning())
					_Replay.Record=TRUE;
			}
		}
	}

	_DPlay.SimulateExitKey=FALSE;

// set random list count

	Math_Lib.ResetRndCount();
}

extern int	BAD_RV;

FileNum Persons3::FindCommsNextBf (BattlefieldType& bfctrl)
{
	ClearGlobrefPackVars();

	FileNum filelist;

	char* BFieldPacket;
	char* ptr;

	ULong n,from;
	int val;

	char* RecPacket;
	ULong RecPacketLen=0;

	if (!_DPlay.Implemented)
	{
		if (_Replay.Playback)
		{
			filelist=_Replay.currentry->file;
			bfctrl=(BattlefieldType)_Replay.currentry->bfctrl;
			_Replay.SetGlobRefs(_Replay.currentry->list);
			_Replay.currentry=_Replay.currentry->next;
		}
		else
		{
			filelist=FindNextBf(bfctrl);

			_Replay.AddFileAndGlobRefsToList(filelist,bfctrl);
		}
	}
	else
	{
		if (_DPlay.Host)
		{
			filelist=FindNextBf(bfctrl);

			if (!(filelist==FIL_NULL && (int)bfctrl!=(int)FINISHBF))
			{
				_Replay.AddFileAndGlobRefsToList(filelist,bfctrl);
				_DPlay.SetRandomListPos();
			}
		}
		else
		{
			if (_DPlay.Joining && !SENT_REQ)
			{
// request bfield packet

				COMMONDATA packet;

				packet.PacketID=PID_BFIELDREQUEST;
				packet.PlayerID=_DPlay.myDPlayID;

				_DPlay.SendMessageToAll((char*)&packet,sizeof(COMMONDATA));

				SENT_REQ=TRUE;
			}

			Bool gotone=FALSE;
			
// NEEDTIMEOUT
			ULong end=_Miles.GetSysTime()+(_DPlay.CommsTimeoutLength*6);

			if (_DPlay.ProcessRequiredBFieldPacket((ULong&)bfctrl,filelist))
			{
				_DPlay.BFieldToProcess++;
			}
			else
			{
				while (!gotone)
				{
					if (_Miles.GetSysTime()>end)
						_Error.SayAndQuit("Timed out (FCNB)");

					ULong to=_DPlay.myDPlayID;
					if (_DPlay.ReceiveNextMessage (RecPacket, RecPacketLen,from,to,DPRECEIVE_TOPLAYER))
					{
						if (from==0)
						{
							_DPlay.ProcessSystemMessage(RecPacket,CPS_GAMESTARTUP);
						}
						else if (from!=_DPlay.aggID)
						{
							if(*(ULong*)RecPacket == PID_BFIELDPACKETIDENT)
							{
								_DPlay.StoreReceivedBField(RecPacket,RecPacketLen);

								if (_DPlay.ProcessRequiredBFieldPacket((ULong&)bfctrl,filelist))
								{
									_DPlay.BFieldToProcess++;
									gotone=TRUE;
								}
							}
							else
							{
								_DPlay.ProcessPlayerMessage(RecPacket,RecPacketLen,from,CPS_GAMESTARTUP);
							}
						}
					}
				}
			}

// once we have got a bfield packet, get the info we need, set up
// glob refs and then add to list so that we are same as host. Lists need
// to be same as host so that later on they can be compared during a 
// resync to see if re-processing is required 

			_Replay.AddFileAndGlobRefsToList(filelist,(ULong)bfctrl);
			_DPlay.SetRandomListPos();
		}
	}
	if (_DPlay.Joining && _DPlay.GameType<COMMSQUICKMISSION)
	{
// need to nobble global reference dealing with my ac so that I start with
// one selected, not one originally put in game as dummy!

		GR_Scram_Squad[_DPlay.mySlot]=DPlay::H2H_Player[_DPlay.mySlot].squadron;
	}
	ResetGlobrefPackVars();	 //Waypoints stuff!
	if (!_DPlay.Implemented)
	{
		_DPlay.SetRandomListPos();										  //JIM 17/05/99
	}
	return filelist;
}


//------------------------------------------------------------------------------
//Procedure		SendInit2Packet
//Author		Andrew McRae
//Date			Thu 5 Sep 1996
//
//Description	Send Second Initialisation Packet
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void SendInit2Packet ()
{
	_DPlay.ResetDeltas();
	_DPlay.InitBuffers();
	_DPlay.DeleteMyAIACPositionToSendList();

// set up AI CommsOwners (in replay/single player all AI owners are me)
	_DPlay.InitialiseOwners();

	if (_DPlay.Implemented)
	{
		if (_DPlay.Joining)
		{
			_DPlay.SendEnteringGameMessage();
		}

		UNIQUE_ID Me_uniqueID;
		AirStrucPtr Me;
		DPID	ServerID,From;
		DWORD	packetlen = sizeof(ULong) * MAXPLAYERS * 2;
		DWORD	recpacketlen = packetlen;
		myDPID sender = 0;
		DWORD n=0;
		
// if I am aggregator then receive any packets that have been sent to me as
// a result of sends to ID 0 that I have not removed from message Q

		myDPID from=0;
		myDPID to=_DPlay.aggID;
		char*	Buffer;
		ULONG   BufferLen;
		ULong receiveFlags = DPRECEIVE_TOPLAYER;

		if(_DPlay.Host)
		{
			while (_DPlay.ReceiveNextMessage(Buffer,BufferLen,from,to,receiveFlags))
			{
			}
		}

		_DPlay.DeleteAIACPositionsList();
		_DPlay.SetUpPlayersAC();
	}
}

//------------------------------------------------------------------------------
//Procedure		CopyPacket
//
//Description	Add contents of an aircraft packet onto AirStruc 
//
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::CopyPacket (ItemBasePtr lpItemBase, LPACPACKET lpAcPacket)
{
	AirStrucPtr lpAirStruc = *lpItemBase;
	UWord	oldsector= MobileItem::currworld->GetSector(*lpItemBase);
	SLong	val;
	UByte	shift=lpAcPacket->Shift;

// add deltas to ac position

#define ADDDELTAS(src,dest,shift)	\
	{								 \
		if (src>127)					\
		{								 \
			val=-((src-128)<<shift);	  \
		}								   \
		else								\
		{									 \
			val=src<<shift;					  \
		}									   \
		dest+=val;								\
	}

	ADDDELTAS(lpAcPacket->X,lpItemBase->World.X,shift)
	ADDDELTAS(lpAcPacket->Y,lpItemBase->World.Y,shift)
	ADDDELTAS(lpAcPacket->Z,lpItemBase->World.Z,shift)

	if (oldsector!=MobileItem::currworld->GetSector(*lpItemBase))
	{
		MobileItem::currworld->RemoveFromSector(*lpItemBase,oldsector);
		MobileItem::currworld->AddToWorld(*lpItemBase);
	}

// MigStatus ??

 	ADDDELTAS(lpAcPacket->Heading,lpAirStruc->hdg.a,shift)
	ADDDELTAS(lpAcPacket->Pitch,lpAirStruc->pitch.a,shift)
	ADDDELTAS(lpAcPacket->Roll,lpAirStruc->roll.a,shift)

	if (lpAcPacket->Heading>127)
	{								 
		val=-((lpAcPacket->Heading-128)<<shift);	  
	}								   
	else								
	{									 
		val=lpAcPacket->Heading<<shift;					  
	}									   

	lpAirStruc->fly.dhdg=val/RateDivider;
	lpAirStruc->fly.cpitch=lpAirStruc->pitch;

// velocity is 4 bits

	if(lpAcPacket->Velocity&8)
	{
		val=-((lpAcPacket->Velocity&7)<<lpAcPacket->Shift);
	}
	else
	{
		val=+((lpAcPacket->Velocity&7)<<lpAcPacket->Shift);
	}

	lpAirStruc->vel+=val;
}

//------------------------------------------------------------------------------
//Procedure		MakePacket
//Author		Andrew McRae
//Date			Thu 5 Sep 1996
//
//Description	construct an Aircraft packet from an AirStruc
//					   
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::MakeDeltaPacket (LPBASICPACKET packet)
{
	UByte	shiftfactor;
	UByte	val;

// want to send deltas from previous position

	shiftfactor=MakeShiftValue();
	packet->Shift=shiftfactor;

#define MAKEDELTAS(src,dest,shift)	  \
	{								   \
		val=(abs(src))>>shift;			\
		if (val!=0 && src<0)			 \
		{								  \
			val|=128;					   \
		}									\
		dest=val;							 \
	}

	MAKEDELTAS(Deltas.lastpos.X,packet->X,shiftfactor);
	MAKEDELTAS(Deltas.lastpos.Y,packet->Y,shiftfactor);
	MAKEDELTAS(Deltas.lastpos.Z,packet->Z,shiftfactor);
	MAKEDELTAS(Deltas.Heading.a,packet->Heading,shiftfactor);
	MAKEDELTAS(Deltas.Pitch.a,packet->Pitch,shiftfactor);
	MAKEDELTAS(Deltas.Roll.a,packet->Roll,shiftfactor);

// velocity is 4 bits

	val=(abs(Deltas.Velocity))>>shiftfactor;
	if(val!=0 && Deltas.Velocity<0)
	{
		val|=8;
	}
	packet->Velocity=val;
}

//------------------------------------------------------------------------------
//Procedure		UpdateDeltas
//Author		Andy McMaster
//Date			Wed 26 Aug 1998
//
//Description	When packet is sent remove sent values from delta record to leave residue
//				of values. These will accumulate to next frame
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::UpdateDeltas(LPACPACKET lpAcPacket)
{
	UByte	shiftfactor=lpAcPacket->Shift;
	SLong	val;

	val=GetVal(lpAcPacket->X,shiftfactor);
	Deltas.lastpos.X-=val;

	val=GetVal(lpAcPacket->Y,shiftfactor);
	Deltas.lastpos.Y-=val;

	val=GetVal(lpAcPacket->Z,shiftfactor);
	Deltas.lastpos.Z-=val;

	val=GetVal(lpAcPacket->Heading,shiftfactor);
	Deltas.Heading.a-=val;

	val=GetVal(lpAcPacket->Pitch,shiftfactor);
	Deltas.Pitch.a-=val;

	val=GetVal(lpAcPacket->Roll,shiftfactor);
	Deltas.Roll.a-=val;

	if(lpAcPacket->Velocity&8)
	{
		val=-((lpAcPacket->Velocity&7)<<shiftfactor);
	}
	else
	{
		val=+((lpAcPacket->Velocity&7)<<shiftfactor);
	}

	Deltas.Velocity-=val;
}

//------------------------------------------------------------------------------
//Procedure		UpdatePredictedAC
//Author		Andy McMaster
//Date			Wed 26 Aug 1998
//
//Description	When packet is received update predicted ac position
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::UpdatePredictedAC(LPACPACKET lpAcPacket)
{
	UByte	shiftfactor=lpAcPacket->Shift;
	SLong	val;
	
	val=GetVal(lpAcPacket->X,shiftfactor);
	Persons2::PlayerGhostAC->World.X+=val;

	val=GetVal(lpAcPacket->Y,shiftfactor);
	Persons2::PlayerGhostAC->World.Y+=val;

	val=GetVal(lpAcPacket->Z,shiftfactor);
	Persons2::PlayerGhostAC->World.Z+=val;

	val=GetVal(lpAcPacket->Heading,shiftfactor);
	Persons2::PlayerGhostAC->hdg.a+=val;

	Persons2::PlayerGhostAC->fly.dhdg=val/RateDivider;							//AMM 31Jul98

	val=GetVal(lpAcPacket->Pitch,shiftfactor);
	Persons2::PlayerGhostAC->pitch.a+=val;

	val=GetVal(lpAcPacket->Roll,shiftfactor);
	Persons2::PlayerGhostAC->roll.a+=val;

	if(lpAcPacket->Velocity&8)
	{
		val=-((lpAcPacket->Velocity&7)<<shiftfactor);
	}
	else
	{
		val=+((lpAcPacket->Velocity&7)<<shiftfactor);
	}

	Persons2::PlayerGhostAC->vel+=val;
	Persons2::PlayerGhostAC->fly.cpitch=Persons2::PlayerGhostAC->pitch;
}


//------------------------------------------------------------------------------
//Procedure		MakeSpecPacket
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::MakeSpecPacket (LPSPECPACKET spec, LPBASICPACKET packet)
{
// info already compacted into extra bytes

	packet->IDCode=spec->ID;
	packet->byte1=spec->spec1;
	packet->byte2=spec->spec2;
	packet->byte3=spec->spec3;
}

//------------------------------------------------------------------------------
//Procedure		MakeCollPacket
//Author		Andy McMaster
//Date			Mon 20 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::MakeCollPacket (LPCOMMSCOLLISION lpCommsCollision, LPBASICPACKET lpAcPacket)
{
// UniqueID is 14 bits
// offset is 7 bits 
// type is 2 bits 
// strength is 6 bits 

// strength is put in last 6 bits of IDCode
// Unique ID and type are put in bytes 1 and 2 
// offset put in 3rd byte, WOW a spare bit......
// offset is now offset/grp element, spare bit used to say which...

	UWord uidc;
	
	uidc=lpCommsCollision->id;

#ifndef NDEBUG
	if (lpCommsCollision->str>63)
		INT3;
#endif
	lpAcPacket->IDCode=lpCommsCollision->str;

	uidc<<=2;
	uidc+=lpCommsCollision->type;

	lpAcPacket->byte1=uidc>>8;
	lpAcPacket->byte2=(uidc&0xff);
	lpAcPacket->byte3=lpCommsCollision->offset;

  	AddHitterScorer(((LPACPACKET)lpAcPacket)->Count,lpCommsCollision);
}

//------------------------------------------------------------------------------
//Procedure		NewBullet
//Author		Andrew McRae
//Date			Thu 5 Sep 1996
//
//Description	Adds a new bullet to a table of bullets for transmission
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewBullet (ULong num, ULong index)
{
   	if (Implemented || _Replay.Record)
	{
// instead of causing an error if table is full, overwrite oldest entry ?????

 		if(BulletBuffer.NumEntries()>=BUFFERLENGTH)				//AMM 02Jul99
		{	
			BulletBuffer.UpdateCurr();
		}
		 
 		BulletBuffer.GetNext()->num=num;
 		BulletBuffer.GetNext()->index=index;

		if (Implemented)
			BulletBuffer.GetNext()->framelaunched=FrameCount;
		else
			BulletBuffer.GetNext()->framelaunched=_Replay.replayframecount;

		BulletBuffer.AddEntryAndUpdateNext();
	}
}

//------------------------------------------------------------------------------
//Procedure		TestForBullet
//Author		Andrew McRae
//Date			Fri 6 Sep 1996
//
//Description	Test if a Bullet has been fired and removes it from table
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool DPlay::TestForBullet (LPBULENTRY bul)
{
	while (BulletBuffer.NumEntries())
	{
		bul->num=BulletBuffer.GetCurr()->num;
		bul->index=BulletBuffer.GetCurr()->index;
		bul->framelaunched=BulletBuffer.GetCurr()->framelaunched;
		BulletBuffer.UpdateCurr();

// if bullet/missile is older than 1 sec dont send

		if (_DPlay.Implemented)
		{
			ULong fps=50/RateDivider;
			SLong age=FrameCount-bul->framelaunched;

			if (!(age>fps || (FrameCount<bul->framelaunched && (abs(age)<(256-fps)))))
				return TRUE;
		}
		else
		{
			if (!(_Replay.replayframecount-bul->framelaunched>50))
				return TRUE;
		}
	}

	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		TestForCollision
//Author		Andrew McRae
//Date			Tue 8 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool DPlay::TestForCollision (LPCOMMSCOLLISION lpCommsCollision)
{
	if(CollBuffer.NumEntries()==0)
		return FALSE;
	
	lpCommsCollision->id=CollBuffer.GetCurr()->id;
	lpCommsCollision->offset=CollBuffer.GetCurr()->offset;
	lpCommsCollision->type=CollBuffer.GetCurr()->type;
	lpCommsCollision->str=CollBuffer.GetCurr()->str;
	lpCommsCollision->hitterscorer=CollBuffer.GetCurr()->hitterscorer;

	CollBuffer.UpdateCurr();

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		TestForSpecial
//Author		Andy McMaster
//Date			Mon 20 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::TestForSpecial(LPSPECPACKET pack)
{
	if(SpecBuffer.NumEntries()==0)
		return FALSE;
	
	pack->ID=SpecBuffer.GetCurr()->ID;
	pack->spec1=SpecBuffer.GetCurr()->spec1;
	pack->spec2=SpecBuffer.GetCurr()->spec2;
	pack->spec3=SpecBuffer.GetCurr()->spec3;

	SpecBuffer.UpdateCurr();

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		Synchronise
//Author		Andrew McRae
//Date			Fri 8 Nov 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool DPlay::Synchronise ()
{
	if (Implemented)
	{
		ULong n,from;
		char* Buffer;
		ULong BufLen;
		COMMONDATA pack;
		Bool	sentOK=FALSE;
		ULong AllBits=0;
		static Bool sent=FALSE;

// init AllBits

		for (n=0; n<MAXPLAYERS; n++)
		{
			if (H2H_Player[n].status==CPS_3D)
			{
				AllBits |= (1 << n);
			}
		}
		AllBits ^= (1 << (mySlot));

// Send PID_SYNC

		if (!sent)
		{
			pack.PacketID=PID_SYNC;

			SendGMessageToPlayersTimeout ((char*)&pack, sizeof(COMMONDATA));
				
			sent=TRUE;
		}

// have we already received pid_syncs

		for (n=0;n<MAXPLAYERS;n++)
		{
			if (SyncPacks[n] && !(SyncBits & 1<<n))
			{
				SyncPacks[n]--;
				SyncBits|=1<<n;
			}
		}

// receive messages until we have got all sync messages

// NEEDTIMEOUT
		ULong now=_Miles.GetSysTime();

		CommsTimeout time(now,60000); // 1 min in all cases

		while (SyncBits!=AllBits)
		{
			now=_Miles.GetSysTime();

// instead of timing out, continue and see if we can get in, at
// least players can exit if they get to 3d rather than having a sys error
// exit soon as in 3d?

			if (time.TimedOut(now))
			{
				SimulateExitKey=TRUE;
				SyncBits=AllBits;
			}

			ULong to=myDPlayID;
			if (ReceiveNextMessage ( Buffer, BufLen, from,to,DPRECEIVE_TOPLAYER))
			{
				if (from==0)
				{
					ProcessSystemMessage(Buffer,CPS_GAMESTARTUP);
				}
				else if (from!=aggID)
				{
					ProcessPlayerMessage(Buffer,BufLen,from,CPS_GAMESTARTUP);
				}
			}
		}
		SyncBits = 0;
		sent=FALSE;
	}
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		PredictMove
//Author		Andy McMaster
//Date			Mon 24 Feb 1997
//
//Description	predict move for ac
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::PredictMove(AirStruc* ac)
{
	SWord bvx,bvy,bvz;
	bool	store=false;

// backup current vels as the ones that of calculated here are used only for
// prediction purposes, not for anything else.

	if (ac==Persons2::PlayerGhostAC || ac->uniqueID.bitfieldUid.commsmove)
	{
		store=true;
		bvx=ac->velx;
		bvy=ac->vely;
		bvz=ac->velz;
	}

	ac->CalcXYZVel();
	ac->mobileitem::NewPosition();

	if (store)
	{
		ac->velx=bvx;
		ac->vely=bvy;
		ac->velz=bvz;
	}
}

//------------------------------------------------------------------------------
//Procedure		PredictGoodMovement
//Author		Andy McMaster
//Date			Mon 21 Dec 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::PredictGoodMovement(UWord slot)
{
	AirStruc tempac;

// OK, what is this dudes predicted movement?
// set up tempac with good position and do a dummy predict

	tempac.uniqueID.bitfieldUid.count=UID_NULL;
	tempac.uniqueID.bitfieldUid.commsmove=TRUE;
	tempac.World.X=HistPosBuffer[slot].GetNext()->Pos.X;
	tempac.World.Y=HistPosBuffer[slot].GetNext()->Pos.Y;
	tempac.World.Z=HistPosBuffer[slot].GetNext()->Pos.Z;
	tempac.hdg.a=HistPosBuffer[slot].GetNext()->Heading.a;
	tempac.pitch.a=HistPosBuffer[slot].GetNext()->Pitch.a;
	tempac.roll.a=HistPosBuffer[slot].GetNext()->Roll.a;
	tempac.vel=HistPosBuffer[slot].GetNext()->Velocity;
	tempac.fly.cpitch.a=tempac.pitch.a;

	for (int n=0;n<RateDivider;n++)
		PredictMove(&tempac);

	HistPosBuffer[slot].GetNext()->Pos.X=tempac.World.X;
	HistPosBuffer[slot].GetNext()->Pos.Y=tempac.World.Y;
	HistPosBuffer[slot].GetNext()->Pos.Z=tempac.World.Z;
	HistPosBuffer[slot].GetNext()->Heading.a=tempac.hdg.a;
	HistPosBuffer[slot].GetNext()->Pitch.a=tempac.pitch.a;
	HistPosBuffer[slot].GetNext()->Roll.a=tempac.roll.a;
	HistPosBuffer[slot].GetNext()->Velocity=tempac.vel;
}

//------------------------------------------------------------------------------
//Procedure		AddDeltas
//Author		Andy McMaster
//Date			Mon 24 Feb 1997
//
//Description	add (sign=TRUE) or subtract (sign=FALSE) current position
//				from players delta record
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::AddDeltas(AirStruc* AC, Bool sign)//, myDPID playerid)
{
	if(sign)
	{
		Deltas.lastpos.X+=AC->World.X;
		Deltas.lastpos.Y+=AC->World.Y;
		Deltas.lastpos.Z+=AC->World.Z;

		Deltas.Heading.a+=AC->hdg.a;

		Deltas.Pitch.a+=AC->pitch.a;

		Deltas.Roll.a+=AC->roll.a;

		Deltas.Velocity+=AC->vel;
		if(Deltas.Velocity>32767)
		{
			Deltas.Velocity-=65536;
		}
	}
	else
	{
		Deltas.lastpos.X-=AC->World.X;
		Deltas.lastpos.Y-=AC->World.Y;
		Deltas.lastpos.Z-=AC->World.Z;

		Deltas.Heading.a-=AC->hdg.a;

		Deltas.Pitch.a-=AC->pitch.a;

		Deltas.Roll.a-=AC->roll.a;

		Deltas.Velocity-=AC->vel;;
		if(Deltas.Velocity<-32767)
		{
			Deltas.Velocity+=65536;
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		MakeShiftValue
//Author		Andy McMaster
//Date			Wed 26 Feb 1997
//
//Description	determines shift byte to apply to deltas
//				this is done by looking at the highest value and detemining
//				how much of a shift is required to bring the most sig. bit 
//				into specified number of bits
//
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
UByte DPlay::MakeShiftValue()
{
	ULong	maxnum;
	ULong	highestbit;
	ULong	n;

// get biggest absolute number

	maxnum=maxstub(abs(Deltas.Velocity*16),abs(Deltas.Roll.a));
	maxnum=maxstub(maxnum,abs(Deltas.Heading.a));
	maxnum=maxstub(maxnum,abs(Deltas.Pitch.a));
	maxnum=maxstub(maxnum,abs(Deltas.lastpos.Z));
	maxnum=maxstub(maxnum,abs(Deltas.lastpos.X));
	maxnum=maxstub(maxnum,abs(Deltas.lastpos.Y));

// determine highestbit

// if highest number can be represented in 7 bits then no shift required

	if(maxnum<128) return 0;

// shift is represented by 4 bits => max shift=15

	for(n=1;n<16;n++)
	{
		if((maxnum>>n)<128)
		{
			return n;
		}
	}

// in time accel deltas may be very big, so allow return value of 15. May require several
// frames to transmit a large delta, but will be OK in the end.
	return 15;
}


//------------------------------------------------------------------------------
//Procedure		SendGonePacket
//Author		Andy McMaster
//Date			Tue 25 Mar 1997
//
//Description	Sends a packet containing IDCode IDC_PLAYERGONE to aggregator
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::SendGonePacket()
{
	myDPID from;
	ULong	n;

	if(!Host)
	{
// could take too long for this packet to be processed send info packet

		COMMONDATA p;

		p.PacketID=PID_GUESTLEAVING;

		SendMessageToPlayers(&p,sizeof(COMMONDATA));
	}
	else
	{
// to do this via aggregator would require simulating everybodys packet so that
// it gets processed. This cannot be worked out so use a normal packet saying,
// OK, everybody out

		COMMONDATA p;
		p.PacketID=PID_HOSTLEAVING;
		SendMessageToPlayers(&p,sizeof(COMMONDATA));
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessPlayerGone
//Author		Andy McMaster
//Date			Mon 12 May 1997
//
//Description	
//
//Inputs		dplayID of player who has left, whether player was in 3d
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::ProcessPlayerGone(ULong playerDPID,Bool in3d) //,ULong& update)
{
	ULong n,slot=0;

	if (in3d)
	{
// if I'm not in 3D dont do this 

		if ((H2H_Player+mySlot)->status!=CPS_3D)
			return;												

		for (n=0;n<MAXPLAYERS;n++)
		{
			if (H2H_Player[n].dpid==playerDPID)
			{
				slot=n;
				break;
			}
		}

		if (n==MAXPLAYERS)
			return;

// this is necesary												//AMM 14Jul99
		if (H2H_Player[slot].status!=CPS_3D)					//AMM 14Jul99
			return;												//AMM 14Jul99

// update packet buffer to say this players packets are no longer important

		PacketBuffer.SetTempCurr();
		PacketBuffer.UpdateTemp();

		do
		{
			PacketBuffer.GetTemp()->Received[n]=TRUE;
			PacketBuffer.UpdateTemp();
		}
		while (PacketBuffer.GetTemp()!=PacketBuffer.GetCurr());

// ensure that multiple player gone messages dont cause problem

		ActivePlayers&=(ULong)(~(1<<slot));

		if(CurrPlayers)
			CurrPlayers--;

		(H2H_Player+slot)->status=CPS_READYROOM;						

		UniqueID thisid=UID_Null;

		for (n=0; n<MAXPLAYERS; n++)
		{
			if (n == slot)
			{
				thisid = (UniqueID)AllocPacket [n];
				break;
			}
		}

		if (thisid!=UID_Null)
		{
			AirStrucPtr thisac=NULL;
			thisac = (AirStrucPtr)Persons2::ConvertPtrUID (thisid);

			DisplayPlayerMessage(thisac,thisac,ST_PLAYERLEFTGAME,0);

			if (thisac)
			{
				if (GameType<COMMSQUICKMISSION)
				{
// for players who have gone, put aircraft at 20000 feet

					thisac->World.Y = FT_20000;

 					WorldStuff &world = mobileitem::GetWorld();
					MinAnimData* mad = (MinAnimData* )thisac->Anim;
					mad->itemstate = DEAD;
					mad->IsInvisible=1;							//DAW 22Jun99

					if (thisac == Persons2::PlayerSeenAC)
						Manual_Pilot.controlmode=ManualPilot::PILOTDEAD;

					thisac->MigStatus.LaunchFlags.deaded=TRUE;
					thisac->MigStatus.LaunchFlags.deadtime=1;
				}
				else
				{
// place player AC under AI control

					SwitchPlayerACToAIAC(thisac);
				}
			}
		}
	}
	else
	{
//check to make sure that this player exists in my game before removing

		for (n=0;n<MAXPLAYERS;n++)
		{
			if (H2H_Player[n].dpid==playerDPID)
			{
				slot=n;
				break;
			}
		}

		if (n==MAXPLAYERS)
			return;

		if(Host)		
		{
			lpDP4->DestroyPlayer(playerDPID);
		}

// currplayers is for 3d only, if leaving UI then should have left 3d already
		H2H_Player[slot].status=CPS_NONE;
		H2H_Player[slot].MyTeam=TS_UNASSIGNED;
		H2H_Player[slot].position=-1;
		H2H_Player[slot].squadron=-1;
	}
}

//------------------------------------------------------------------------------
//Procedure		CommitSuicide
//Author		Andy McMaster
//Date			Thu 19 Jun 1997
//
//Description	If player has been killed, then allow an immediate 
//				resurrect, to stop tedious decent to ground
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::CommitSuicide()
{
	if (Implemented)
	{
		if (Persons2::PlayerSeenAC->MigStatus.LaunchFlags.deaded && AllowSuicide && _Miles.GetSysTime()>SuicideTime)
		{
			SuicideTime=0;
			AllowSuicide=FALSE;
			viewtime=false;
			ResurrectMe(Persons2::PlayerSeenAC,true);
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		SendNeedResyncMessage
//Author		Andy McMaster
//Date			Wed 19 Nov 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendNeedResyncMessage()
{
	COMMONDATA packet;

	packet.PacketID=PID_NEEDRESYNC;

	SendMessageToAll((char*)&packet,sizeof(COMMONDATA));
//	SendGMessageToPlayers((char*)&packet,sizeof(COMMONDATA));
}

//------------------------------------------------------------------------------
//Procedure		ResetState
//Author		Andy McMaster
//Date			Wed 19 Nov 1997
//
//Description	set frame num to 0
//				clear buffers - packet,damage,collision,bullet
//				reset array latencies
//				
//	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ResetState()
{
	FrameCount=0;
	firsttimein=TRUE;
	synched=FALSE;
	csync=false;
	ResetDeltas();
	ResetSync=true;
}

//------------------------------------------------------------------------------
//Procedure		SendResyncMessage
//Author		Andy McMaster
//Date			Wed 19 Nov 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendResyncMessage()
{
	ACSTATEPACKET packet;
	ULong num,datasize,n;
	UByteP datapack,temp;
	UWord deltaoffset;
	UByte newbyte;

	AirStrucPtr gac=Persons2::PlayerGhostAC;
	AirStrucPtr sac=Persons2::PlayerSeenAC;

	gac->World.X=sac->World.X;
	gac->World.Y=sac->World.Y;
	gac->World.Z=sac->World.Z;
 
	gac->hdg.a=sac->hdg.a;
	gac->pitch.a=sac->pitch.a;
	gac->roll.a=sac->roll.a;
	gac->vel=sac->vel;

	gac->fly.cpitch=sac->pitch;

	packet.PacketID=PID_RESYNC;

	packet.Pos.X=gac->World.X;
	packet.Pos.Y=gac->World.Y;
	packet.Pos.Z=gac->World.Z;

	packet.Heading.a=gac->hdg.a;
	packet.Pitch.a=gac->pitch.a;
	packet.Roll.a=gac->roll.a;
	packet.Velocity=gac->vel;

	packet.MyKills=(H2H_Player+mySlot)->kills;
	packet.MyDeaths=(H2H_Player+mySlot)->deaths;

	if (!sac->MigStatus.LaunchFlags.deaded)
		packet.alive=TRUE;
	else
		packet.alive=FALSE;

	if (resurrectend)
	{
		packet.resurrect=true;
		Persons2::PlayerSeenAC->MigStatus.LaunchFlags.deaded=FALSE;
	}
	else
		packet.resurrect=false;

	if (JustJoined)
	{
		joiningplayerslot=mySlot;
		packet.joining=true;
	}
	else
		packet.joining=false;

// add anim data to packet

	num=SHAPE.ScanAnimData((ItemPtr)sac);

	datasize=sizeof(ACSTATEPACKET)+sizeof(ULong)+(num*(sizeof(UWord)+sizeof(UByte)));
	datasize+=sizeof(int); //timeofday

	datasize+=sizeof(UWord); // shape

	datapack=new UByte[datasize];
	temp=datapack;

	memcpy(temp,&packet,sizeof(ACSTATEPACKET));
	temp+=sizeof(ACSTATEPACKET);

	int time=_Replay.inst->timeofday;
	*(int*)temp=time;
	temp+=sizeof(int);

	MakeAnimPack(num,temp,sac->shape);

	SendGMessageToPlayersTimeout((char*)datapack,datasize);
	delete [] datapack;
}

//------------------------------------------------------------------------------
//Procedure		BeginSyncPhase
//Author		Andy McMaster
//Date			Wed 19 Nov 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::BeginSyncPhase()
{
// if in map back to 3d

	resyncbar=true;
#ifdef SYNCDATA
	FILE* fp=fopen("sync.txt","at");
	fprintf(fp,"begin sync phase\n");
	if (Initiator)
		fprintf(fp,"Initiator\n");
	else
		fprintf(fp,"NOT Initiator\n");
//	fclose(fp);
#endif

// only if we are in accel

	if (accelselected&(1<<mySlot)) //AMM 07/06/99
		ProcessQuitAccelMessage(mySlot);

#ifdef SYNCDATA
	fprintf(fp,"accel des\n");
#endif
	DisplayResyncingMessage();
#ifdef SYNCDATA
	fprintf(fp,"display mess\n");
#endif

	ResyncPhase=TRUE;
	ResetState(); // sets csync to false to begin commssync
	deadplayers=0;

	if (Initiator)
	{
#ifdef SYNCDATA
	fprintf(fp,"about to send need resync\n");
#endif

		SendNeedResyncMessage();
#ifdef SYNCDATA
	fprintf(fp,"send need resync\n");
#endif

	}
	if (Host)
	{
#ifdef SYNCDATA
	fprintf(fp,"about to stop agg\n");
#endif

		_Agg.Running=FALSE;
		_Agg.StopResetAggregator();
#ifdef SYNCDATA
	fprintf(fp,"stopped agg\n");
#endif

	}

// want to reset world sync at beginning of normal sync

	InitSyncData(true);
	resetreceivesync=true;
#ifdef SYNCDATA
	fprintf(fp,"exit begin sync phase\n");
	fclose(fp);
#endif
}

//------------------------------------------------------------------------------
//Procedure		ReceiveSyncPackets
//Author		Andy McMaster
//Date			Wed 19 Nov 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::ReceiveSyncPackets()
{
	static ULong needed=0;
	static ULong now=0;

	char* Buffer;
	ULong	BufLen;
	ULong	from,n;
	LPACSTATEPACKET packet;

#ifdef SYNCDATA
	FILE* fp=fopen("sync.txt","at");
	fprintf(fp,"in receivesyncpackets\n");
#endif
	if (resetreceivesync)
	{
		resetreceivesync=false;
		resyncsgot=0;
#ifdef SYNCDATA
	fprintf(fp,"reset\n");
#endif

	needed=ActivePlayers^(1<<mySlot);

		{
#ifdef SYNCDATA
	fprintf(fp,"about to sendresyncmessage\n");
#endif
			SendResyncMessage();
#ifdef SYNCDATA
	fprintf(fp,"sent resyncmessage\n");
#endif

		}

		now=_Miles.GetSysTime();
#ifdef SYNCDATA
//	FILE* fp=fopen("sync.txt","at");
	fprintf(fp,"recsyncpacks need=%d\n",needed);
//	fclose(fp);
#endif
	}
	
	ULong to=myDPlayID;
	while (ReceiveNextMessage(Buffer,BufLen,from,to,DPRECEIVE_TOPLAYER))
	{
		if (from==0)
		{
			ProcessSystemMessage(Buffer,CPS_GAMESTARTUP);
		}
		else if (from!=aggID)
		{
			packet=(LPACSTATEPACKET)Buffer;

			if (packet->PacketID==PID_RESYNC) // && BufLen>sizeof(ACPACKET))
			{
				n=DPID2Slot(from);

				if (n!=MAXPLAYERS)
				{
					ProcessResyncPacket(packet,n);
#ifdef SYNCDATA
//	FILE* fp=fopen("sync.txt","at");
	fprintf(fp,"got=%d\n",resyncsgot);
//	fclose(fp);
#endif
				}
			}
			else
			{
				ProcessPlayerMessage(Buffer,BufLen,from,CPS_RESYNCING);
			}
		}
	}

	if (resyncsgot==needed)
	{
// need to get battle position b4 sending position!!!!!!!!!!!!!!!!
		if (joiningplayerslot>=0 && GameType<COMMSQUICKMISSION)
		{
			GetBattlePosition(joiningplayerslot);
		}

		resyncsgot=0;
		needed=0;
#ifdef SYNCDATA
//	FILE* fp=fopen("sync.txt","at");
	fprintf(fp,"SYNCOK\n",resyncsgot);
	fclose(fp);
#endif
		return TRUE;
	}

	if (_Miles.GetSysTime()>now+10000)
	{
// if we have had problems resyncing then quit, and if host start another resync - still need
// to process all messages in Q. If sync packets arrive late it should not matter as they will
// still be the same info.

#ifdef SYNCDATA
//	FILE* fp=fopen("sync.txt","at");
	fprintf(fp,"TIMEOUT\n");
	fclose(fp);
#endif
		if (Host)
		{
			Initiator=TRUE;
			BeginSyncPhase();
		}
		return FALSE;
	}

#ifdef SYNCDATA
	fprintf(fp,"return false\n");
	fclose(fp);
#endif
	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		InitH2HPArray
//Author		Andy McMaster
//Date			Wed 10 Dec 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::InitH2HPArray()
{
	H2HPlayerInfo* p=H2H_Player;

	for (ULong n=0;n<MAXPLAYERS;n++)
	{
		p->name[0]=0;
		p->status=CPS_NONE;
		p->dialogue[0]=0;
		p->kills=0;
		p->deaths=0;
		p->MyTeam=TS_UNASSIGNED;

// initially players are unassgined
		p->position=-1;

		if (GameType>TEAMPLAY)
			p->squadron=-1;
		else
			p->squadron=0;

		p++;
	}
}

//------------------------------------------------------------------------------
//Procedure		InitSyncPhase
//Author		Andy McMaster
//Date			Mon 15 Dec 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::InitSyncPhase()
{
	ACPACKET InitPacket;
	AGGSENDPACKET	packet;
	ULong	n,num,m;
	static ULong 	IgnoreFrameIncs=6; //RateDivider*3;
	ULong	BufferLen;
	char* Buffer;
	DPID	From=0;
	static bool first=true; // RERUN added bool
	ULong to=myDPlayID;

	if (ResetSync)
	{
		ResetSync=false;
		first=true;
		IgnoreFrameIncs=6;

		to=myDPlayID;
		From=aggID;

		while (ReceiveNextMessage(Buffer,BufferLen,From,to,DPRECEIVE_FROMPLAYER))
		{
		}
	}

	bool got=false;

	From=aggID;
	to=myDPlayID;
	while (ReceiveNextMessage(Buffer,BufferLen,From,to,DPRECEIVE_TOPLAYER))
	{
		if (From==0)
			ProcessSystemMessage(Buffer,CPS_3D);
		else if (From==aggID)
		{
// keep receiving agg packets till we get the last one
				ExpandAggPacket(&packet,(LPAGGSENDPACKET)Buffer);//AMM 22Sep99
				got=true;
				break;
		}
		else
		{
			if (!ProcessPlayerMessage(Buffer,BufferLen,From,CPS_3D))
				break;
		}
	}

	if (!got)
		return FALSE;

 	InitPacket.Count=FrameCount;
 	InitPacket.X=myDPlayID;
	InitPacket.Ack1=0;
	InitPacket.Ack2=0;
 	InitPacket.IDCode=PIDC_DUMMYPACKET;
 	SendPacketToAggregator(&InitPacket);

#ifdef FILELOGS
 	FILE* fp=fopen("sendpack.txt","at");
 	fprintf(fp,"%d %d %d %d %d %d\n",InitPacket.Count,InitPacket.X,InitPacket.IDCode,_Miles.GetSysTime(),(_Miles.GetSysTime()-lasttime)/10,myDPlayID);
 	fclose(fp);
	lasttime=_Miles.GetSysTime();
#endif

// check to see if all players have sent a dummy packet. If we have received
// a packet then we can send another one, this synchronises with the aggregator
// for now...
#ifdef FILELOGS
	FILE* sp=fopen("aggpack.txt","at");
	fprintf(sp,"Agg frame%d My Frame%d\n%d %d %d %d\n%d %d %d %d\n",
	packet.Count,FrameCount,
	packet.player[0].Count,packet.player[0].X,packet.player[0].IDCode,packet.player[0].Y,
	packet.player[1].Count,packet.player[1].X,packet.player[1].IDCode,packet.player[1].Y);
	fclose(sp);
#endif

	num=0;
	for (n=0;n<MAXPLAYERS;n++)
	{
		if (packet.player[n].IDCode==PIDC_DUMMYPACKET)	
		{
			num++;
		}
	}

	{
		if (num!=CurrPlayers)
		{
// determine if I am in aggregated packet and if not, work out
// new framecount to get me in

			for (n=0;n<MAXPLAYERS;n++)
			{
				if (H2H_Player[n].dpid==myDPlayID)
					break;
			}

			if (packet.player[n].IDCode!=PIDC_DUMMYPACKET)
			{
				if (packet.Count<=FrameCount)
				{
// if we have got too far ahead
// I mean behind!!!! If packet received has a higher frame number than current frame 
// we have no chance of getting in!!

					if ((abs)(FrameCount-packet.Count)>10)
					{
						FrameCount=packet.Count;
						IgnoreFrameIncs=6; //RateDivider*3;
					}
					else
					{
						if (IgnoreFrameIncs==0)
						{
							FrameCount++;
							IgnoreFrameIncs=6; //FrameCount-packet.Count;
						}
						else
						{
							IgnoreFrameIncs--;
						}
					}
				}
				else
				{
// initial big catchup
					if (first)									//AMM 16May99
					{											//AMM 16May99
						first=false;							//AMM 16May99
						FrameCount=packet.Count+1;				//DAW 22Jun99
					}											//AMM 16May99
					else										//AMM 16May99
					if (IgnoreFrameIncs==0)
					{
						FrameCount=packet.Count;
						IgnoreFrameIncs=6; //RateDivider*3;
					}
					else
					{
						IgnoreFrameIncs--;
					}
				}
			}
			FrameCount++;
		}
		else
		{
#ifdef FILELOGS
 	FILE* fp=fopen("sendpack.txt","at");
 	fprintf(fp,"INITSYNCPHASE SUCCESSFUL\n");
 	fclose(fp);
#endif
			IgnoreFrameIncs=6; //RateDivider*3; //=FrameCount;
			FrameCount++;
			synched=TRUE;
			return TRUE;
		}
	}

	return FALSE;
}


//------------------------------------------------------------------------------
//Procedure		SecondSyncPhase
//Author		Andy McMaster
//Date			Mon 15 Dec 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::SecondSyncPhase()
{
// start transmitting null packets until all players dummy packets have
// been through

	ACPACKET InitPacket;
	AGGSENDPACKET	packet;
	ULong n,num;
	ULong	BufferLen;
	char* Buffer;
	DPID	From=0;

	bool got=false;

	ULong to=myDPlayID;
	From=aggID;
	while (ReceiveNextMessage(Buffer,BufferLen,From,to,DPRECEIVE_TOPLAYER))
	{
		if (From==0)
			ProcessSystemMessage(Buffer,CPS_3D);
		else if (From==aggID)
		{
			{
				ExpandAggPacket(&packet,(LPAGGSENDPACKET)Buffer);//AMM 22Sep99
				got=true;
				break;
			}
		}
		else
		{
			if (!ProcessPlayerMessage(Buffer,BufferLen,From,CPS_3D))
				break;
		}
	}

	if (!got)
		return FALSE;

	InitPacket.Count=FrameCount;
	InitPacket.IDCode=PIDC_INITPACK;
	InitPacket.Ack1=0;
	InitPacket.Ack2=0;
	InitPacket.X=0;
	InitPacket.Y=0;
	InitPacket.Z=0;
	InitPacket.Shift=0;
	InitPacket.Velocity=0;
	InitPacket.Heading=0;
	InitPacket.Pitch=0;
	InitPacket.Roll=0;

	SendPacketToAggregator(&InitPacket);

#ifdef FILELOGS
	FILE* fp=fopen("sendpack.txt","at");
	fprintf(fp,"%d %d %d %d %d\n",	InitPacket.Count,InitPacket.X,InitPacket.IDCode,_Miles.GetSysTime(),(_Miles.GetSysTime()-lasttime)/10);
	fclose(fp);
	lasttime=_Miles.GetSysTime();
#endif

#ifdef FILELOGS
	FILE* dp=fopen("aggpack.txt","at");
	fprintf(dp,"Agg frame%d My Frame%d\n%d %d %d %d\n%d %d %d %d\n",
	packet.Count,FrameCount,
	packet.player[0].Count,packet.player[0].X,packet.player[0].IDCode,packet.player[0].Y,
	packet.player[1].Count,packet.player[1].X,packet.player[1].IDCode,packet.player[1].Y);
	fclose(dp);
#endif

	num=0;
	for (n=0;n<MAXPLAYERS;n++)
	{
		if (packet.player[n].IDCode==PIDC_INITPACK)		
		{
			num++;
		}
	}

	if (num==CurrPlayers)
 	{
// everybody is ready, start gameplay

		gotfullpacket=FALSE;
		FrameCount++;

		PacketBuffer.GetCurr()->Count=packet.Count+1;	
		RecAckBuffer.GetCurr()->Frame=packet.Count;

		for (n=0;n<MAXPLAYERS;n++)
		{
			PacketBuffer.GetCurr()->PlayerData[n].Count=packet.Count;
			HistPosBuffer[n].SetTempNext();
			HistPosBuffer[n].TempPrev();
			HistPosBuffer[n].GetTemp()->Frame=packet.Count;
		}

// allow other players to join ready room

		AllowJoins=TRUE;

#ifdef TIMERTIME
		timertime=timeGetTime();
		LARGE_INTEGER count;
		QueryPerformanceCounter(&count);
		Qtimertime=count.LowPart;;
		Qlasttimertime=0;
		lasttimertime=0;
		timer=fopen("timertim.txt","at");
		fprintf(timer,"BEGIN\n");

		BOOL res=QueryPerformanceFrequency((LARGE_INTEGER*)(&Qfreq));

		fprintf(timer,"FREQ: %d\n",Qfreq);
#endif

		csync=true;
		return TRUE;
	}
	FrameCount++;

	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		InitGameVars
//Author		Andy McMaster
//Date			Mon 15 Dec 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::InitGameVars()
{
	ULong n;

	velcounter=0;
	ResendStatus=0;
	ResendPacket=0;
	firstdeathcall=true;
	accelselected=0;

	JustJoined=FALSE;
	ResyncPhase=FALSE;
	Initiator=FALSE;

	FrameInc=0;

	Math_Lib.ResetRndCount();

	FrameCount = 0;

	GR_Quit3DNow = 0;

	firsttimein=FALSE;

// start of game or on a resync delete, world will have been transmitted

	DeleteAIACPositionsList();

}

//------------------------------------------------------------------------------
//Procedure		MakeAndSendPacket
//Author		Andy McMaster
//Date			Thu 18 Dec 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::MakeAndSendPacket()
{
// if lots of AI AC require their position transmitting then it could be a while before
// a player delta is sent, resulting in warping. Use this flag to indicate that after every
// all packets for a single AI AC have been sent, send a player packet

#ifdef FILELOGS
	static ULong tally=0;
	static ULong total=0;
#endif
	static bool GuaranteedDeltaSend=true; 
	ACPACKET	SendPacket;

	memset (&SendPacket,0,sizeof(ACPACKET));

	SendPacket.Count=FrameCount++;

	if (MyAIACPositionsToSend && !GuaranteedDeltaSend)
	{
		switch (MyAIACPositionsToSend->extrainfo)
		{
		case 0:
			Make1stAIACPositionPacket(&SendPacket);
			break;
		case 1:
			Make2ndAIACPositionPacket(&SendPacket);
			GuaranteedDeltaSend=true;
			break;
		default :
			_Error.SayAndQuit("Invalid value for AI AC");
			break;
		}
	}
	else
	{
		GuaranteedDeltaSend=false;

		MakeDeltaPacket ( &SendPacket);

		MakeExtraPacket(&SendPacket,FALSE);

// update deltas with sent values

		UpdateDeltas(&SendPacket);									//AMM 26Aug98
	}
// send acks

	SendPacket.Ack1=0;
	SendPacket.Ack2=0;

#ifdef FILELOGS
	ULong a1=0,a2=0;
#endif

	if (RecPackBuffer.NumEntries()>0)
	{
 		SendPacket.Ack1=RecPackBuffer.GetCurr()->packet;		//RDH 11Jul99

#ifdef FILELOGS
		a1=RecPackBuffer.GetCurr()->packet;
#endif

		RecPackBuffer.UpdateCurr();

		if (RecPackBuffer.NumEntries()>0)
		{
 			if (RecPackBuffer.GetCurr()->packet!=SendPacket.Ack1)
 			{
 				SendPacket.Ack2=RecPackBuffer.GetCurr()->packet;
 
 #ifdef FILELOGS
 			a2=RecPackBuffer.GetCurr()->packet;
#endif
 
 				RecPackBuffer.UpdateCurr();
 			}
		}
	}

// send packet off to aggregator

	SendPacketToAggregator (&SendPacket);

// copy sent packet into buffer

	SendPackBuffer.GetNext()->Done=FALSE;
	SendPackBuffer.AddEntry();

// if send buffer is full then need to initiate a resynch

	if (SendPackBuffer.NumEntries()>BUFFERLENGTH)
	{
#ifdef FILELOGS
		tally=0;
		total=0;
#endif

		Initiator=TRUE;
		BeginSyncPhase();
		return;
	}

	CopyBuffer((char*)&SendPackBuffer.GetNext()->packet,(char*)&SendPacket,sizeof(ACPACKET));

	SendPackBuffer.UpdateNext();

// is resend of sent packets necessary? YES
// how many frames before we decide a resend is necessary? for now 25 
// resend on 1/4 and 1/2 length of buffer.....

	ULong num=SendPackBuffer.NumEntries();						//RDH 11Jul99

	SWord curr=SendPackBuffer.GetCurr()->packet.Count;
	SendPackBuffer.SetTempNext();
	SendPackBuffer.TempPrev();
	SWord next=SendPackBuffer.GetTemp()->packet.Count;

	if (curr!=ResendPacket)
	{
		ResendPacket=curr;
		ResendStatus=0;
	}

	if ((num>BUFFERLENGTH/4 && ResendStatus==0)					//RDH 11Jul99
	|| (num>BUFFERLENGTH/2 && ResendStatus<2))					//RDH 11Jul99
	{
		SendPacketToAggregator (&(SendPackBuffer.GetCurr()->packet));
		ResendStatus++;

#ifdef FILELOGS
	 	FILE* fpa=fopen("sendpack.txt","at");
		fprintf(fpa,"resend packet %d ",SendPackBuffer.GetCurr()->packet.Count);
		fclose(fpa);
#endif

	}

#ifdef FILELOGS
	ULong	nowtime=(_Miles.GetSysTime()-lasttime)/10;
	ULong ttime=_Miles.GetSysTime();

			tally+=nowtime;
			total++;

			FILE* fp=fopen("sendpack.txt","at");

			if (OverLay.keyFlags&KF_ACCELON)
			{
				fprintf(fp,"ACCEL ON\n");
			}

			fprintf(fp," buferlen=%d\n",num);
			fprintf(fp,"Frame %d Time %d diff=%d av=%d\n%d %d %d %d %d %d %d\n",
				SendPacket.Count,
				ttime,
				nowtime,
				tally/total,
				Deltas.lastpos.X,
				Deltas.lastpos.Y,
				Deltas.lastpos.Z,
				SendPacket.X,
				SendPacket.Y,
				SendPacket.Z,
				SendPacket.IDCode);

			lasttime=ttime;


		fprintf(fp,"ack1=%d (frame %d)\n",SendPacket.Ack1,a1);
		fprintf(fp,"ack2=%d (frame %d)\n",SendPacket.Ack2,a2);

		if ((SendPacket.IDCode>>6)==PT_SPECIAL)
			fprintf(fp,"Spec\n\n");
		else if ((SendPacket.IDCode>>6)==PT_BULLET)
			fprintf(fp,"bul\n\n");
		else 
			fprintf(fp,"coll\n\n");

		fclose(fp);
#endif

}

//------------------------------------------------------------------------------
//Procedure		CalcNewAbsPos
//Author		Andy McMaster
//Date			Thu 15 Jan 1998
//
//Description Update players absolute positions from delta packets	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::CalcNewAbsPos()
{
}

//------------------------------------------------------------------------------
//Procedure		ProcessInfoPackets
//Author		Andy McMaster
//Date			Tue 31 Mar 1998
//
//Description	Get any non aggregated packets and process them 
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessInfoPackets()
{
	ULong	BufferLen;
	char* Buffer;
	DPID	From=0;

	ULong to=myDPlayID;
	while (ReceiveNextMessage(Buffer,BufferLen,From,to,DPRECEIVE_TOPLAYER))
	{
		if (From==0)
			ProcessSystemMessage(Buffer,CPS_3D);
		else if (From==aggID)
		{
			{
				ProcessAggregatedPacket(Buffer);

				if (ResyncPhase)
					return;
			}
		}
		else
		{
// need to break out if resync message or something else has arrived

			if (!ProcessPlayerMessage(Buffer,BufferLen,From,CPS_3D))
				break;
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		UpdateAckBuffer
//Author		Andy McMaster
//Date			Mon 6 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::UpdateAckBuffer(LPAGGSENDPACKET packet)
{
	UByte	n,m;
	SWord	offset1,offset2;
	LPACKBUFF	temp;
	Bool	update=TRUE;

	for (n=0;n<MAXPLAYERS;n++)
	{
		if (packet->player[n].IDCode!=PIDC_PACKETERROR)
		{
			if (packet->player[n].Ack1)
			{
				RecAckBuffer.SetTempCurr();

				do
				{
					if (packet->player[n].Ack1==RecAckBuffer.GetTemp()->Frame)
						RecAckBuffer.GetTemp()->Acks[n]=TRUE;

					RecAckBuffer.UpdateTemp();
				}
				while (RecAckBuffer.GetTemp()!=RecAckBuffer.GetCurr());
			}

			if (packet->player[n].Ack2)
			{
				RecAckBuffer.SetTempCurr();

				do
				{
					if (packet->player[n].Ack2==RecAckBuffer.GetTemp()->Frame)
						RecAckBuffer.GetTemp()->Acks[n]=TRUE;

					RecAckBuffer.UpdateTemp();
				}
				while (RecAckBuffer.GetTemp()!=RecAckBuffer.GetCurr());
			}
		}
	}

	while (update)
	{
		m=0;

		for (n=0;n<MAXPLAYERS;n++)
		{
			if (RecAckBuffer.GetCurr()->Acks[n]==TRUE)
				m++;
		}

		if (m==CurrPlayers)
		{
			for (n=0;n<MAXPLAYERS;n++)
				RecAckBuffer.GetCurr()->Acks[n]=FALSE;

			RecAckBuffer.AddEntry();

			m=RecAckBuffer.GetCurr()->Frame;
			RecAckBuffer.UpdateCurr();
			RecAckBuffer.GetCurr()->Frame=m+1;

		}
		else
			update=FALSE;
	}
}

//------------------------------------------------------------------------------
//Procedure		SetUpPlayersAC
//Author		Andy McMaster
//Date			Wed 15 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SetUpPlayersAC()
{
	ULong n;
	UniqueID thisid=UID_Null;

	for (n=0;n<MAXPLAYERS;n++)
	{
		if (H2H_Player[n].status!=CPS_3D)
		{
			thisid = (UniqueID)AllocPacket [n];

			if (thisid!=UID_Null)
			{
				AirStrucPtr thisac=NULL;
				thisac = (AirStrucPtr)Persons2::ConvertPtrUID (thisid);

				if (thisac)
				{
 					WorldStuff &world = mobileitem::GetWorld();
					MinAnimData* mad = (MinAnimData* )thisac->Anim;
					mad->itemstate = DEAD;

					mad->IsInvisible=1;
					thisac->MigStatus.LaunchFlags.deaded=TRUE;

					thisac->MigStatus.LaunchFlags.deadtime=1;

				}
			}
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		JoinGame
//Author		Andy McMaster
//Date			Wed 15 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::JoinGame()
{
	COMMONDATA packet;
	Bool	got=FALSE;
	ULong from,t,count,n;
	ULong RecPacketLen = 0;
	char* RecPacket;
	PUILOOKUPTABLE	pUILookUpTable;
	PilotType* Active_Pilot=&MMC.Active_Pilots[0];

	if (!PlayerCreated)
		return FALSE;

	CountPlayers();

	ResetAllocPacket();

	if ((H2H_Player[mySlot].position==-1 || H2H_Player[mySlot].squadron==-1) && GameType>TEAMPLAY)//DAW 22Jun99
	{
		CommsMessage(IDS_NOTSLOTS);
		return FALSE;
	}

	SimulateExitKey=FALSE;

// get messages

	packet.PacketID=PID_MESSAGESREQUEST;

	SendMessageToPlayers((char*)&packet,sizeof(COMMONDATA));

	if (!UpdateMessages())
		return FALSE;

// get prefs

	packet.PacketID=PID_PREFSREQUEST;

	SendMessageToPlayers((char*)&packet,sizeof(COMMONDATA));

	got=FALSE;

// NEEDTIMEOUT
	ULong now=_Miles.GetSysTime();

	CommsTimeout time(now,CommsTimeoutLength);

	while (!got)
	{
		now=_Miles.GetSysTime();

		if (time.TimedOut(now))
			return FALSE;

		ULong to=myDPlayID;
		if (ReceiveNextMessage (RecPacket, RecPacketLen,from,to,DPRECEIVE_TOPLAYER))
		{
			if (from==0)
			{
				ProcessSystemMessage(RecPacket,CPS_GAMESTARTUP);
			}
			else if (from!=aggID)
			{
				PUIPLAYERDATA pUIPlayerData = (PUIPLAYERDATA)RecPacket;

				if (pUIPlayerData->PacketID == PID_PREFSDATA)
			   	{
					ProcessPrefsPacket(RecPacket);

					got=TRUE;
		   		}
				else
				{
					ProcessPlayerMessage(RecPacket,RecPacketLen,from,CPS_GAMESTARTUP);
				}
			}
		}
	}

	packet.PacketID=PID_PLAYERINFOREQUEST;

	SendMessageToPlayers((char*)&packet,sizeof(COMMONDATA));

	count=0;

// NEEDTIMEOUT
	time.Init(now,CommsTimeoutLength);

	for (;;)
	{
		if (count == (CurrPlayers - 1))
			break;

		now=1;

		if (time.TimedOut(now))
			return FALSE;

		ULong to=myDPlayID;
		if (ReceiveNextMessage (RecPacket, RecPacketLen,from,to,DPRECEIVE_TOPLAYER))
		{
			if (from==0)
			{
				ProcessSystemMessage(RecPacket,CPS_GAMESTARTUP);
			}
			else if (from!=aggID)
			{
				PUIPLAYERDATA pUIPlayerData = (PUIPLAYERDATA)RecPacket;

				if (pUIPlayerData->PacketID == PID_PLAYERUPDATE)
				{
					count++;

// Put data in H2HPlayer

					for (n=0;n<MAXPLAYERS;n++)
					{
						if (H2H_Player[n].dpid==from)
						{
							CopyBuffer((char*)&H2H_Player[n],(char*)&pUIPlayerData->playerinfo,sizeof(H2HPlayerInfo));
						}
					}
				}
				else
				{
					ProcessPlayerMessage(RecPacket,RecPacketLen,from,CPS_GAMESTARTUP);
				}
			}
		}
	}

	packet.PacketID=PID_MISSCONDSREQUEST;

	SendMessageToPlayers((char*)&packet,sizeof(COMMONDATA));

	got=FALSE;

	while (!got)
	{
		now=1;

		if (time.TimedOut(now))
			return FALSE;

		ULong to=myDPlayID;
		if (ReceiveNextMessage (RecPacket, RecPacketLen,from,to,DPRECEIVE_TOPLAYER))
		{
			if (from==0)
			{
				ProcessSystemMessage(RecPacket,CPS_GAMESTARTUP);
			}
			else if (from!=aggID)
			{
				PUIPLAYERDATA pUIPlayerData = (PUIPLAYERDATA)RecPacket;

				if (pUIPlayerData->PacketID == PID_MISSIONCONDITIONS)
			   	{
					ProcessConditionsPacket(RecPacket);

					got=TRUE;
		   		}
				else
				{
					ProcessPlayerMessage(RecPacket,RecPacketLen,from,CPS_GAMESTARTUP);
				}
			}
		}
	}

	ULong	squads[SQ_R_NONFLYABLE+1];	 //include Yak

	if (GameType<COMMSQUICKMISSION)
	{
		FillCSQuickStruc();

// position 

		for (n=0;n<=SQ_R_NONFLYABLE;n++)
		{
			squads[n]=0;
		}

		for (n=0;n<MAXPLAYERS;n++)
		{
			H2H_Player[n].position=squads[H2H_Player[n].squadron];
			squads[H2H_Player[n].squadron]++;
		}
	}

	if (!SendMyPlayerInfo(false))								//AMM 01Jul99
		return FALSE;											//AMM 01Jul99
					 
// no need to receive save game, the one initially should still be valid
	MMC.playersquadron=H2H_Player[mySlot].squadron;
	MMC.playeracnum=H2H_Player[mySlot].position;

	H2H_Player[mySlot].status=CPS_3D;

	GR_amberstrength = MAXPLAYERS;

	Joining=TRUE;

	InitialFlagReset();

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		AddPlayerToGame
//Author		Andy McMaster
//Date			Wed 15 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::AddPlayerToGame(ULong slot,ULong actype,ULong id)
{
	AirStrucPtr thisac=(AirStrucPtr)Persons2::ConvertPtrUID((UniqueID)id);

	if (!thisac)
		return;

	AllocPacket[slot]=id;
	CurrPlayers++;

	if (GameType<COMMSQUICKMISSION)
	{
// deathmatch/team game

// if actype of new player is different to type already set up then need to get new
// shape/anims - uniqueIDs will be out of sync as well!
// ids will be in sync for deathmatch/teamplay even if different ac types
// are created because they are all created from same id band

// OR set commsmove flag if lots of ac created
		ULong type=Persons3::Squad2Type(H2H_Player[slot].squadron);

		thisac->classtype=Plane_Type_Translate[type][0];

		MinAnimData* mad = (MinAnimData* )thisac->Anim;
		mad->IsInvisible=0;

		thisac->MigStatus.LaunchFlags.deaded=FALSE;
		thisac->MigStatus.LaunchFlags.deadtime=0;

		SHAPE.ResetAnimData_NewShape(thisac,thisac->shape,thisac->weap.AircraftStatus.Weapons);//AMM 08Jun99

		thisac->fly.pModel->Initialise(thisac,FALSE);

	}
	else
	{
// campaign/quickmission

		PlayerSync=true;
		SwitchAIACToPlayerAC(thisac,slot);
	}

// Set Active Player Bit field

	ActivePlayers|=(1<<slot);
}

//------------------------------------------------------------------------------
//Procedure		IsGameAvailable
//Author		Andy McMaster
//Date			Wed 15 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::IsGameAvailable()
{
	GAMEAVAILABLEPACKET data;

	data.PacketID=PID_ISTHEREAGAME;
	data.PlayerID=myDPlayID;
	
	SendMessageToPlayers((char*)&data,sizeof(GAMEAVAILABLEPACKET));
}

//------------------------------------------------------------------------------
//Procedure		SendGameAvailableMessage
//Author		Andy McMaster
//Date			Wed 15 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendGameAvailableMessage(Bool available)
{
	GAMEAVAILABLEPACKET data;

	data.available=available;
	data.PlayerID=myDPlayID;
	data.PacketID=PID_GAMEAVAILABLE;

	SendMessageToPlayers((char*)&data, sizeof(GAMEAVAILABLEPACKET));
}

//------------------------------------------------------------------------------
//Procedure		SendEnteringGameMesssage
//Author		Andy McMaster
//Date			Tue 21 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendEnteringGameMessage()
{
// send a PID_IAMIN message to other players

	ALLOCREQUEST data;

// want to send my uniqueID so other players can set up my AC

	data.id=Persons2::PlayerSeenAC->uniqueID.bitfieldUid.count;

	data.PacketID=PID_IAMIN;

	data.PlayerID=H2H_Player[mySlot].squadron;
	data.Slot=mySlot;

	SendMessageToPlayers((char*)&data,sizeof(ALLOCREQUEST));

	PlayerSync=true;
}

//------------------------------------------------------------------------------
//Procedure		BeginAggregation
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
void	DPlay::BeginAggregation()
{
	if (_Agg.CreateAggregator())
	{
		_Agg.RunAggregator();
	}
}


//------------------------------------------------------------------------------
//Procedure		InitBuffers
//Author		Andy McMaster
//Date			Tue 28 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::InitBuffers()
{
	ULong n,m;

	PacketBuffer.InitBuffer();
	PacketBuffer.SetTempCurr();
	do
	{
		for (n=0;n<MAXPLAYERS;n++)
		{
		 	PacketBuffer.GetTemp()->Received[n]=FALSE;
			PacketBuffer.GetTemp()->PlayerData[n].IDCode=PIDC_PACKETERROR;
		}

		PacketBuffer.UpdateTemp();
	}
	while (PacketBuffer.GetTemp()!=PacketBuffer.GetCurr());

	SendPackBuffer.InitBuffer();
	SendPackBuffer.SetTempCurr();
	do
	{
		SendPackBuffer.GetTemp()->Done=FALSE;
		SendPackBuffer.UpdateTemp();
	}
	while (SendPackBuffer.GetTemp()!=SendPackBuffer.GetCurr());

	RecPackBuffer.InitBuffer();
	RecAckBuffer.InitBuffer();
	CollBuffer.InitBuffer();
	SpecBuffer.InitBuffer();
	BulletBuffer.InitBuffer();
	UIDSBuffer.InitBuffer();

// fill historical buffer with inital position
	AirStrucPtr ac;

	for (n=0;n<MAXPLAYERS;n++)
	{
		if (AllocPacket[n])
		{
			ac=(AirStrucPtr)Persons2::ConvertPtrUID((UniqueID)AllocPacket[n]);

			if (ac)
			{
				HistPosBuffer[n].InitBuffer();
				HistPosBuffer[n].SetTempCurr();
				do 
				{
					HistPosBuffer[n].GetTemp()->Pos.X=ac->World.X;
					HistPosBuffer[n].GetTemp()->Pos.Y=ac->World.Y;
					HistPosBuffer[n].GetTemp()->Pos.Z=ac->World.Z;

					HistPosBuffer[n].GetTemp()->Heading=ac->hdg;
					HistPosBuffer[n].GetTemp()->Pitch=ac->pitch;
					HistPosBuffer[n].GetTemp()->Roll=ac->roll;
					HistPosBuffer[n].GetTemp()->Velocity=ac->vel;

					HistPosBuffer[n].GetTemp()->OK=false;

					HistPosBuffer[n].UpdateTemp();
				} 
				while (HistPosBuffer[n].GetTemp()!=HistPosBuffer[n].GetCurr());
			}

			m=BUFFERLENGTH-1;

			while (m--)
				HistPosBuffer[n].UpdateNext();
		}
	}

	for (n=0;n<MAXPLAYERS;n++)
	{
		for (m=0;m<3;m++)
		{
			ZeroMemory(&DeltasToApply[n][m],sizeof(DELTARECORD));
		}
	}
	DeltaIndex=0;

	DeleteClaimBuffer();									//AMM 02/07/99
}

//------------------------------------------------------------------------------
//Procedure		SendAllocPacket
//Author		Andy McMaster
//Date			Thu 30 Apr 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::SendAllocPacket()
{
	ULong From,n;
	Bool	sentok=FALSE;
	UILOOKUPTABLE ltable;

	if (Host)
	{
		ltable.PacketID=PID_ALLOCPACKET;
		ltable.PlayerID=myDPlayID;

		for (n=0;n<MAXPLAYERS;n++)
		{
			ltable.Table[n]=AllocPacket[n];
		}

		sentok=SendMessageToPlayers((char*)&ltable,sizeof(UILOOKUPTABLE));
	}
	return sentok;
}

//------------------------------------------------------------------------------
//Procedure		ResetDeltas
//Author		Andy McMaster
//Date			Fri 1 May 1998
//
//Description	
// clear deltas, i.e. assume that they were sent perfectly. This will
// be sorted out when packet is processed and deltas actually sent are
// compared to actual deltas at time
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ResetDeltas()
{
	Deltas.lastpos.X=0;
	Deltas.lastpos.Y=0;
	Deltas.lastpos.Z=0;
	Deltas.Roll=ANGLES_0Deg;
	Deltas.Pitch=ANGLES_0Deg;
	Deltas.Heading=ANGLES_0Deg;
	Deltas.Velocity=0;
	Deltas.SpecCode=PIDC_EMPTY;
}

//------------------------------------------------------------------------------
//Procedure		ProcessCollisionPacket
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
void	DPlay::ProcessCollisionPacket(LPBASICPACKET packet,ULong fromplayerslot)
{
	UWord uid;
	UByte offset;
	UByte type;
	UByte str;
	UWord temp;
	UByte pnum=((LPACPACKET)packet)->Count;
	bool	myscorer=false;

 		str=packet->IDCode&63;

		temp=packet->byte1<<8;
		temp+=packet->byte2;

		uid=(UniqueID)(temp>>2);																
		type=temp&3;

		offset=packet->byte3;

		ItemPtr src=NULL;

		CommsKiller=255;

		if (_DPlay.Implemented)
		{
			if (GameType<COMMSQUICKMISSION)
			{
				src=*Persons2::ConvertPtrUID((UniqueID)AllocPacket[fromplayerslot]);

// cant actually kill yourself, so last hitter must have caused the damage that has
// led to a death
				ULong damagedplayer=UID2Slot(uid);

				if (damagedplayer!=fromplayerslot)
					CommsKiller=fromplayerslot;
			}
		}

		if (_Replay.Record || (_DPlay.Implemented && fromplayerslot==mySlot))//AMM 08Apr99
			myscorer=true;

		BoxCol::Col_Shooter=src;
		BoxCol::Col_Hitter=NULL; // dont know what hitter is

// in comms game an ac may be killed on one machine and be hit but not killed on
// another. This may result in ac being killed then having damage applied,
// so need to stop any further damage packets if ac is dead. In replay this will not
// happen because packet order is known to be OK.
// have fix for replay just in case!

		{
			if ((Persons2::ConvertPtrUID((UniqueID)uid))->MigStatus.LaunchFlags.size==AIRSTRUCSIZE)
			{
				AirStrucPtr deadac=(AirStrucPtr)Persons2::ConvertPtrUID((UniqueID)uid);
				if (deadac->shape!=PDEATH && deadac->AutoMoveInfo.movecode!=AUTO_RESURRECT)//AMM 12Jul99
				{
		  			SHAPE.AddToCollisionList(uid,offset,type,str,pnum,myscorer);
				}
			}
			else 												//AMM 05Jul99
			{													//AMM 05Jul99
				SHAPE.AddToCollisionList(uid,offset,type,str,pnum,myscorer);//AMM 05Jul99
			}													//AMM 05Jul99
		}
		CommsKiller=255;

		if (_DPlay.Implemented && Persons2::PlayerSeenAC->MigStatus.LaunchFlags.deaded)//AMM 05Jul99
		{
// if I have been killed

			if (!SuicideTime)
			{
				SuicideTime=_Miles.GetSysTime()+5000; // 5 secs from now
			}
		}
}

//------------------------------------------------------------------------------
//Procedure		ProcessSpecialPacket
//Author		Andy McMaster
//Date			Mon 20 Jul 1998
//
//Description	Owner determines whether I should process an effect or not.
//				e.g. to want to process my near misses but not my SFX
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessSpecialPacket(LPBASICPACKET packet, AirStrucPtr AC, ULong slot)
{
	SLong effect=0;
	ULong ID=packet->IDCode;

	switch (ID)
	{
	case PIDC_PACKETERROR:
	case PIDC_DUMMYPACKET:
	case PIDC_INITPACK:
	case PIDC_EMPTY:
		break;

	case PIDC_NEARMISS:
		ProcessNearMiss(packet,AC);
		break;

	case PIDC_RESURRECT:
		if (Implemented)
		{
			if (AC!=Persons2::PlayerSeenAC)
			{
 // stop collisions occurring aginst AC whilst in resurrect

				AC->AutoMoveInfo.movecode=AUTO_RESURRECT;

				RepairAircraft(AC);
			}
		}
		break;

	case PIDC_RESURRECTEND:
		if (Implemented)
		{
			AC->ai.PilotSkills.radiosilent=FALSE;//AMM 4Jul99 (Doomsday!)

			if (AC!=Persons2::PlayerSeenAC)
			{
// allow collisions 

				AC->AutoMoveInfo.movecode=AUTO_FOLLOWWP;

				RepairAircraft (AC);
			}

// I am now officially not dead (undeaded????)

			AC->MigStatus.LaunchFlags.deaded=FALSE;

			ResetDeadPlayers(AC);

			if (AC==Persons2::PlayerSeenAC)
			{
				_Miles.ResetSoundFlags();
				ResetCommsSurfaces();
				resurrectend=false;
			}

			if (AC != Persons2::PlayerSeenAC)					//AMM 01Jul99
			{
				SHAPE.NavigationLightsActive(AC,FALSE);

				AircraftAnimData*	adptr;
				adptr = (AircraftAnimData*) AC->Anim;
				adptr->acleglowerl = 255;
				adptr->acleglowerr = 255;
				adptr->acleglowerf = 255;
				adptr->acleglowerb = 255;
				adptr->animtoggle = 1;

				AC->fly.pModel->ResetGearPos(false);			
			}
			else
				AC->fly.pModel->ResetGearPos(true);				
		}
		break;

	case PIDC_LANDEDEFFECT:
		if (!_Replay.Record)
		{
			effect+=(packet->byte1)<<8;
			effect+=packet->byte2;

			if (AC==Persons2::PlayerSeenAC)
			{
// if in comms dont playback my crashes

				if (!Implemented) // i.e. playback
				{
// this can cause ac position to change so need to keep track of deltas - not in playback

					Trans_Obj.DummyLandedEffect(AC,effect,packet->byte3);		
				}
				else
				{
					if (GameType<COMMSQUICKMISSION)
					{
						Persons2::PlayerGhostAC->AutoMoveInfo.movecode=AUTO_NOPPILOT;//AMM 14Jul99
						AC->AutoMoveInfo.movecode=AUTO_NOPPILOT; // start resurrection in comms
						Manual_Pilot.controlmode=ManualPilot::PILOTDEAD;
					}
				}
			}
			else
			{
				{
// other ac that is not dead, do an effect?

					Trans_Obj.KillLauncherComms(AC);
				}
			}

			if (_DPlay.Implemented)
			{
				SetScore(AC,NULL);
			}
		}
		break;

	case PIDC_AERODEVICE:
		if (_Replay.Playback || AC!=Persons2::PlayerSeenAC)
		{
			ProcessAeroDevAction(packet,AC);
		}
		break;

	case PIDC_GEAR:
		if (_Replay.Playback || AC!=Persons2::PlayerSeenAC)
		{
			ProcessGearThingy(packet,AC);
		}
		break;

	case PIDC_KILLTRANS:
		KillPlayersTransients(AC);
		break;

	case PIDC_STARTACCEL:
		ProcessStartAccelMessage(slot);
		break;

	case PIDC_PLAYERGONE:
		if (Implemented)
		{
// if host has left in a deathmatch game then its game over time 

			if (slot==0 && GameType<COMMSQUICKMISSION)
			{
				SimulateExitKey=TRUE;
			}
			else if (slot!=0)
			{
				ProcessPlayerGone(H2H_Player[slot].dpid,TRUE);
			}
			else
			{
// host is leaving a coop game, what shall we do?
			}
		}
		break;

	case PIDC_AIACPOSITION1:
		Process1stAIACPositionPacket(packet,slot);
		break;

	case PIDC_AIACPOSITION2:
		Process2ndAIACPositionPacket(packet,slot);
		break;

	case PIDC_DEATHMOVEEFFECT:									//RJS 29Oct98
		if (_Replay.Playback)									//RJS 29Oct98
		{														//RJS 29Oct98
			effect = packet->byte1;								//RJS 29Oct98
			_Replay.ReplayDeathMode = (RDEATH)effect;					//RJS 29Oct98
		}														//RJS 29Oct98
		break;													//RJS 29Oct98

	 case PIDC_OWNERCHANGE:
		 ProcessOwnerChange(packet);
		 break;

	case PIDC_KILLLAUNCHER:
		if (!_Replay.Record)
			ProcessKillLauncher(packet);

		break;

	case PIDC_PLAYERMESSAGE:
		ProcessPlayerMessage(packet,slot);
		break;

	case PIDC_EXPANDMIGS:
		ProcessExpandMigsMessage(packet);
		break;

	case PIDC_EJECT:
		ProcessEjectMessage(AC);
		break;

	case PIDC_SHAPECHANGE:
		ProcessNewShapePacket(packet);
		break;
		
	case PIDC_ACCELGEARUP:
		ProcessAccelGearUpMessage();
		break;

  	case PIDC_DESELECTACCEL:
		ProcessDeselectAccelMessage(slot);
		break;

	case PIDC_PLAYERMOVING:										//DAW 22Jun99
		ProcessPlayerMoving();									//DAW 22Jun99
		break;													//DAW 22Jun99

	case PIDC_PLAYERNOTMOVINGHELDAC:							//DAW 22Jun99
		ProcessNewPlayerNotMovingHeldAC(packet);				//DAW 22Jun99
		break;													//DAW 22Jun99

	case PIDC_NEWWP:											//AMM 24Jun99
		ProcessNewWPMessage(packet);							//AMM 24Jun99
		break;													//AMM 24Jun99

#ifndef NDEBUG
	 default:
		_Error.SayAndQuit("Packet ID not used yet %d",ID);
#endif
	}
}

//------------------------------------------------------------------------------
//Procedure		GetVal
//Author		Andy McMaster
//Date			Mon 11 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
SLong	DPlay::GetVal(UByte src,UByte shift)
{
	SLong retval;

	if (src>127)
		retval=-((src-128)<<shift);
	else
		retval=src<<shift;

	return retval;
}

//------------------------------------------------------------------------------
//Procedure		SetRandomListPos
//Author		Andy McMaster
//Date			Tue 19 May 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
#define MAXRNDINCS 64
void	DPlay::SetRandomListPos()
{
	static int x=0; // RERUN added int
	UWord rndincs;//RJS 04Dec98
	rndincs = Math_Lib.GetRndCount ();							//RJS 04Dec98
	if (rndincs > MAXRNDINCS)									//RJS 04Dec98
		rndincs = 0;											//RJS 04Dec98
	else														//RJS 04Dec98
		rndincs = MAXRNDINCS - rndincs;							//RJS 04Dec98

	for (;rndincs > 0; rndincs--)
		Math_Lib.rnd();

	Math_Lib.ResetRndCount();
}

//------------------------------------------------------------------------------
//Procedure		PossTransmitColl
//Author		Andy McMaster
//Date			Fri 3 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::PossTransmitColl(ItemPtr target, ItemPtr hitter,SLong &send)//AMM 08Jan99
{
	send = 0;													//AMM 08Jan99

	if (_DPlay.Implemented || _Replay.Record || _Replay.Playback)
	{
// bullets only at the moment
						
 		if (hitter->MigStatus.LaunchFlags.size==TransientSize)
		{
			TransientItemPtr t=(TransientItemPtr)(hitter);

			if (t->CollTestEnabled)
			{
// only check if active bullet

				if (_Replay.Playback)
				{
// never send packets in playback

// if I am involved in collision then return TRUE so that collision is not processed, 
//but because send is 0 nothing will be done and packet will arrive to deal with me,
// otherwise, do collision as normal

					if (target==Persons2::PlayerSeenAC || t->Launcher==Persons2::PlayerSeenAC)
					{
						return TRUE;
					}
					else
					{
						return FALSE;
					}
				}
				else if (_Replay.Record)
				{
// send packet if bullet is against me or I launched it

					if ((AirStrucPtr)(t->Launcher)==Persons2::PlayerSeenAC
					|| 	(AirStrucPtr)target==Persons2::PlayerSeenAC)
					{
						send=1;
						return TRUE;
					}
					else
					{
						return FALSE;
					}
				}
				else
				{
// comms game
// send packet if bullet is against me or I launched it
// unless bullet against me was launched by an AI AC with a comms owner 
// other than me, in which case that player will send packet

					if ((AirStrucPtr)(t->Launcher)==Persons2::PlayerSeenAC
					|| 	(AirStrucPtr)target==Persons2::PlayerSeenAC)
					{
						if (!t->Launcher->uniqueID.bitfieldUid.commsmove
						&&	((AirStrucPtr)(t->Launcher))->CommsOwner!=Persons2::PlayerSeenAC->uniqueID.bitfieldUid.count)
						{
// bullet against me launched by an AI AC controlled by other player. Dont want
// to do collision or send packet, add to log but set send to 0.

							send=0;
							return TRUE;
						}
						send=1;
						return TRUE;
					}
					else
					{
						return FALSE;
					}
				}
			}
			else
			{
// if not active bullet then dont want to send packet, should not have had
// a collision anyway so should never get here

#ifndef NDEBUG
				INT3;
#endif
				return FALSE;
			}
		}
		else
		{
// collision with something else

			// log this collision too...						//AMM 08Jan99
			if ((AirStrucPtr)target==Persons2::PlayerSeenAC		//AMM 08Jan99
				|| (AirStrucPtr)hitter==Persons2::PlayerSeenAC)	//AMM 08Jan99
			{													//AMM 08Jan99
				if (!_Replay.Playback)							//AMM 08Jan99
				{												//AMM 08Jan99
					send = 2;									//AMM 08Jan99
				}												//AMM 08Jan99
				return TRUE;									//AMM 08Jan99
			}													//AMM 08Jan99
		}														//AMM 08Jan99
	}

	return FALSE;												//AMM 08Jan99
}

//------------------------------------------------------------------------------
//Procedure		NewCollision2
//Author		Andy McMaster			
//Date			Sun 5 Jul 1998
//
//Description	Store new collision in collision table to be transmitted
//				as soon as possible
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewCollision2 (UWord id, UByte offset, UByte type, UByte str, UniqueID hitterscorer)
{
	if (id)
	{
		if (Implemented || _Replay.Record)
		{
// instead of causing an error if table is full, overwrite oldest entry ?????

			if(CollBuffer.NumEntries()>=BUFFERLENGTH)			//AMM 02Jul99
			{	
				CollBuffer.UpdateCurr();
			}
			
			type&=0x03;
			 
			CollBuffer.GetNext()->id=id;
			CollBuffer.GetNext()->offset=offset;
			CollBuffer.GetNext()->type=type;
			CollBuffer.GetNext()->str=str;
			CollBuffer.GetNext()->hitterscorer=hitterscorer;

			CollBuffer.AddEntryAndUpdateNext();
		}
	}
}


//------------------------------------------------------------------------------
//Procedure		NewNearMiss
//Author		Andy McMaster
//Date			Thu 23 Jul 1998
//
//Description	add nearmiss entry to special buffer
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewNearMiss(AirStrucPtr trg, AirStrucPtr hitter)
{
	UByte b1=0,b2=0,b3=0;

// store uniqueID of whichever ac is not player in byte1 and byte2
// byte3 stores whether that ac is trg or hitter

	ULong	ID=trg->uniqueID.bitfieldUid.count;

// only store near misses caused by player against AI AC

	b1=ID&0xff;
	b2=ID>>8;

	NewSpecial(PIDC_NEARMISS,b1,b2,b3);
}

//------------------------------------------------------------------------------
//Procedure		ProcessNearMiss
//Author		Andy McMaster
//Date			Thu 23 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessNearMiss(LPBASICPACKET packet,AirStrucPtr shooter)
{
// get uid of ac and whether it is the trg or hitter

	ULong ID;
	AirStrucPtr ac;

	ID=packet->byte2<<8;
	ID+=packet->byte1;

	ac=(AirStrucPtr)Persons2::ConvertPtrUID((UniqueID)ID);

// now knoe that all near misses thru hear are caused by player on AI ac
	if (ac && shooter)
	{
		if (shooter==Persons2::PlayerSeenAC)
			shooter=Persons2::PlayerGhostAC;

		Art_Int.PersonalThreat(ac,shooter);//AMM30Jul99
	}
}

//------------------------------------------------------------------------------
//Procedure		MakeExtraPacket
//Author		Andy McMaster
//Date			Thu 23 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::MakeExtraPacket(LPBASICPACKET packet,Bool replay)
{
// bullet, collision, other data to be tagged onto end of ac packet.
//
// One Packet can be appended to ACPACKET.
//		Priority is:-
//					collision packet
//					damage packet
//					bullet packet

	COMMSCOLLISION CommsCollision;
	SPECPACKET SpecPack;
	BULENTRY BulPack;

	packet->IDCode=0;

	if(TestForCollision (&CommsCollision))
	{
		MakeCollPacket (&CommsCollision, packet);
		packet->IDCode|=(PT_COLLISION<<6);

		if (replay)
		{
			ProcessCollisionPacket(packet,0);
		}
	}
	else if (TestForUIDSPacket(&SpecPack))
	{
		MakeUIDSPacket(&SpecPack,packet);
		packet->IDCode|=PT_UIDS<<6;

		if (replay)
		{
			ProcessUIDSPacket(packet,0);
		}
	}
	else if (TestForSpecial(&SpecPack))
	{
		MakeSpecPacket(&SpecPack,packet);
  		packet->IDCode|=(PT_SPECIAL<<6);

		if (replay)
		{
			ProcessSpecialPacket(packet,Persons2::PlayerSeenAC,0);
		}
	}
	else if(TestForBullet(&BulPack))
	{
		MakeBulletPacket(&BulPack,packet);
		packet->IDCode|=(PT_BULLET<<6);
	}
	else
	{
		packet->IDCode=PIDC_EMPTY;
	}
}

//------------------------------------------------------------------------------
//Procedure		NewSpecial
//Author		Andy McMaster
//Date			Fri 24 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewSpecial(ULong id,UByte byte1, UByte byte2, UByte byte3)
{
	if (Implemented || _Replay.Record)
	{
// instead of causing an error if table is full, overwrite oldest entry ?????

		if(SpecBuffer.NumEntries()>=BUFFERLENGTH)				//AMM 02Jul99
		{
			SpecBuffer.UpdateCurr();
		}
		 
		SpecBuffer.GetNext()->ID=id;
		SpecBuffer.GetNext()->spec1=byte1;
		SpecBuffer.GetNext()->spec2=byte2;
		SpecBuffer.GetNext()->spec3=byte3;

		SpecBuffer.AddEntryAndUpdateNext();
	}
}

//------------------------------------------------------------------------------
//Procedure		MakeBulletPacket
//Author		Andy McMaster
//Date			Fri 24 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::MakeBulletPacket(LPBULENTRY bul, LPBASICPACKET pack)
{
// byte1=num of bullets
// byte2=index of launcher
// byte3=frame launched..for ID purposes

	pack->byte1=bul->num;
	pack->byte2=bul->index;
	pack->byte3=FrameCount;
}

//------------------------------------------------------------------------------
//Procedure		ProcessBulletPacket
//Author		Andy McMaster
//Date			Fri 24 Jul 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessBulletPacket(LPBASICPACKET pack, AirStrucPtr AC)
{
	ULong num,index;
	WeapAnimData* weap;
	ItemPtr item;
	UWord	shape,MuzVel,MuzDel;
	SLong	xpos,ypos,zpos;
	UByte	ltype;
	TransientItem* item2;
	WorldStuff &world = mobileitem::GetWorld();

	num=pack->byte1;
	index=pack->byte2;

	item=(ItemPtr)AC; 

	weap=SHAPE.DirectWeaponLauncher(item,index,shape,xpos,ypos,zpos,MuzVel,MuzDel,ltype);
	if (weap)
	{
		if (num > 1)
			item2 = Trans_Obj.LaunchManyGunBullets((mobileitem*)item,MuzVel,num,MuzDel,index,index,(ShapeNum)shape,xpos,ypos,zpos,world,0,ltype);//RJS 26Mar99
		else
		{
			if (item->MigStatus.LaunchFlags.size == AIRSTRUCSIZE)							//RJS 03Jun99
				((AirStrucPtr)item)->weap.currentbullet = NULL;

			Trans_Obj.LaunchWeapon(true,(AirStrucPtr)item,weap,xpos,ypos,zpos,world,MuzVel,MuzDel);//AMM 09Apr99
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessAggregatedPacket
//Author		Andy McMaster
//Date			Fri 7 Aug 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessAggregatedPacket(char* packet)
{
// only process aggregated packets if we have synched

	if (csync)
 		StorePacket((LPAGGSENDPACKET)packet);
}


//------------------------------------------------------------------------------
//Procedure		GetRealACPosition
//Author		Andy McMaster
//Date			Tue 1 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
COORDS3D* DPlay::GetGoodACPosition(ItemBasePtr ac)
{
	ULong n;

	if (ac)
	{
		if (ac->uniqueID.bitfieldUid.commsmove)
		{
			for (n=0;n<MAXPLAYERS;n++)
			{
				if (AllocPacket[n]==ac->uniqueID.bitfieldUid.count)
				{
					return &HistPosBuffer[n].GetCurr()->Pos;
				}
			}
			//RERUN HACK START
			return & ac->World; // Don't know who this is, return current pos
			//RERUN HACK END
		}
		else
		{
			if (ac==Persons2::PlayerSeenAC)
				return &Persons2::PlayerGhostAC->World;
			else
				return & ac->World;
		}
	}

	return NULL;
}


//------------------------------------------------------------------------------
//Procedure		SetScore
//Author		Andy McMaster
//Date			Tue 1 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SetScore(AirStrucPtr victim, ItemPtr hitter)
{
	ULong n,m;

	ULong shooterteam,myteam,victimteam;
	Nationality myn=Persons2::PlayerSeenAC->AutoMoveInfo.nationality;

	AirStrucPtr shooter=NULL;
	
   	if (hitter)
	{
		shooter=(AirStrucPtr)hitter;
	}

	if (victim==shooter)
		shooter=NULL;

	if (victim)
	{
		bool commsplayerdead=false;
		for (n=0;n<MAXPLAYERS;n++)
		{
			if (AllocPacket[n]==victim->uniqueID.bitfieldUid.count)
			{
				commsplayerdead=true;
				break;
			}
		}

		if (commsplayerdead && deadplayers&1<<n)
			return;

		deadplayers|=1<<n;

		if (commsplayerdead)
			(H2H_Player+n)->deaths++;

		if (GameType!=DEATHMATCH)
		{
			if (victim->AutoMoveInfo.nationality==myn)
				MyTeamDeaths++;
			else
				EnemyTeamDeaths++;
		}

		if (shooter)
		{
			bool commsplayershooter=false;

			for (n=0;n<MAXPLAYERS;n++)
			{
				if (AllocPacket[n]==shooter->uniqueID.bitfieldUid.count)
				{
					commsplayershooter=true;
					break;
				}
			}


			if (shooter->AutoMoveInfo.nationality!=victim->AutoMoveInfo.nationality || GameType==DEATHMATCH)
			{
				if (commsplayershooter)
					(H2H_Player+n)->kills++;

				if (GameType!=DEATHMATCH)
				{
					if (shooter->AutoMoveInfo.nationality==myn)
						MyTeamKills++;
					else
						EnemyTeamKills++;
				}
			}

			if (commsplayershooter && commsplayerdead)
			{
				DisplayPlayerMessage((ItemPtr)victim,hitter,ST_PLAYERKILLEDPLAYER);
			}
			else if (commsplayerdead)
			{
				DisplayPlayerMessage((ItemPtr)victim,hitter,ST_PLAYERKILLEDCOMMS);
			}
			else if (commsplayershooter)
			{
				if (shooter->AutoMoveInfo.nationality==victim->AutoMoveInfo.nationality)
				{
					DisplayPlayerMessage((ItemPtr)victim,hitter,ST_PLAYERKILLEDFRIENDLY);
				}
				else
				{
					DisplayPlayerMessage((ItemPtr)victim,hitter,ST_PLAYERKILLEDENEMY);
				}
			}
		}
		else if (commsplayerdead)
		{
// player has crashed
			DisplayPlayerMessage((ItemPtr)victim,hitter,ST_PLAYERKILLEDCOMMS);
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		GetScoreHitter
//Author		Andy McMaster
//Date			Wed 9 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ItemPtr	DPlay::GetScoreHitter(ItemPtr item)
{
	ItemPtr ret=NULL;

	if (item->MigStatus.LaunchFlags.size==AIRSTRUCSIZE)
	{
		ret=item;
	}
	else
	if (item->MigStatus.LaunchFlags.size==TRANSIENTSIZE)
	{
		TransientItemPtr tip=(TransientItemPtr)item;

		ret=tip->Launcher;
	}

	return ret;
}

//------------------------------------------------------------------------------
//Procedure		NewAeroDevAction
//Author		Andy McMaster
//Date			Sept 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewAeroDevAction(UByte devID,UWord Attractor)
{
	if (!_Replay.Playback)
	{
		UByte position;

		switch (Attractor)
		{
		case 0x0000:
			position=0;
			break;
		case 0x1555:
			position=1;
			break;
		case 0x04000:
			position=2;
			break;
		default:
			position=0;
		}

		NewSpecial(PIDC_AERODEVICE, devID,1,position);
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessAeroDevAction
//Author		Andy McMaster
//Date			Sept 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessAeroDevAction(LPBASICPACKET packet, AirStrucPtr AC)
{
	AC->fly.pModel->ProcessAeroPacket(AC, packet->byte1, packet->byte2, packet->byte3);
}

//------------------------------------------------------------------------------
//Procedure		NewGearThingy
//Author		Andy McMaster
//Date			Fri 18 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewGearThingy(SWord val)
{
	UWord newval=abs(val);

	UByte b1,b2,b3;

	if (!_Replay.Playback)
	{
		if (val<0)
			b1=1;
		else
			b1=0;

		b2=newval>>8;
		b3=newval&255;

		NewSpecial(PIDC_GEAR,b1,b2,b3);
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessGearThingy
//Author		Andy McMaster
//Date			Fri 18 Sep 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessGearThingy(LPBASICPACKET packet, AirStrucPtr ac)
{
	SWord val;

	val=(packet->byte3)+(packet->byte2<<8);
	if (packet->byte1)
		val=-val;

	ac->fly.pModel->SetGearAnimPos(ac,val);
}

//------------------------------------------------------------------------------
//Procedure		SendKillTransientsMessage
//Author		Andy McMaster
//Date			Thu 8 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendKillTransientsMessage()
{
}

//------------------------------------------------------------------------------
//Procedure		KillPlayersTransients
//Author		Andy McMaster
//Date			Thu 8 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::KillPlayersTransients(AirStrucPtr ac)
{
// search transient list for items launched by packet owner and if their 
// ID is the same as the one sent add transient to dead list

	TransientItemPtr tip=TransientItem::TransientList;
	ULong frame;

	if (_DPlay.Implemented)
	{
// kill all transients for player sending packet and all AI AC owned by him

		if (ac!=Persons2::PlayerSeenAC)
		{
			while (tip)
			{
				bool remove=false;

				if (tip->Launcher==ac && tip->AutoMoveInfo.movecode!=AUTO_NOPPILOT)
				{
					remove=true;
				}
				else if (tip->Launcher)
				{
					UniqueID id=tip->Launcher->uniqueID.bitfieldUid.count;
					AirStrucPtr ac2;

					ac2=*Persons2::ConvertPtrUID(id);

					if (ac2)
					{
						if (ac2->CommsOwner==ac->uniqueID.bitfieldUid.count)
						{
							remove=true;
						}
					}
				}
				
				if (remove)
				{
					Trans_Obj.AddTransientItemToDeadList(tip);
				}

				tip=tip->nexttrans;
			}
		}
	}
	else if (_Replay.Playback)
	{
// remove what? nothing?

	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessPlayerSlotRequest
//Author		Andy McMaster
//Date			Thu 8 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessPlayerSlotRequest(char* message,ULong from,UByte ctrl)
{
// step through H2H_Player struc and see if anybody has this slot. If not then fill
// in slot and send a message to all players indicating that player has taken this slot
// in form of a pid_slotinfoupdate message

	LPSLOTPOSPACK p=(LPSLOTPOSPACK)message;

	int pos=p->position;
	int	squadnum=p->squadron;
	ULong i=0;
	bool	slotfree=true;

	while	(		(i < MAXPLAYERS)
			)
	{
		if  (		(H2H_Player[i].squadron == squadnum)
			&&		(H2H_Player[i].position == pos )
			&&		(H2H_Player[i].status!=CPS_NONE)

			)
			{
				slotfree=false;
				break;
			}
		i++;
	}
	
	SLOTPOSPACK s;

	s.PlayerID=from;
	s.PacketID=PID_SLOTINFOUPDATE;
	s.squadron=squadnum;
	s.position=pos;
	
	if (slotfree)
	{
		s.Slot=1;

		for (i=0;i<MAXPLAYERS;i++)
		{
			if (H2H_Player[i].dpid==from)
				break;
		}

		if (i!=MAXPLAYERS)
		{
			H2H_Player[i].squadron=squadnum;
			H2H_Player[i].position=pos;
		}
		UpdateFrag=TRUE;

// update ready room 

		DisplayInfo=TRUE;
	}
	else
		s.Slot=0;

	SendMessageToAll((char*)&s,sizeof(SLOTPOSPACK));

}

//------------------------------------------------------------------------------
//Procedure		SendSlotRequest
//Author		Andy McMaster
//Date			Thu 8 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::SendSlotRequest(int pos, int squadron)
{
	SLOTPOSPACK p;

	p.squadron=squadron;
	p.position=pos;
	p.PacketID=PID_SLOTREQUEST;
	p.Slot=mySlot;

	if (SendMessageToAll((char*)&p,sizeof(SLOTPOSPACK)))
		 return TRUE;

	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		ReceiveSlotRequestOK
//Author		Andy McMaster
//Date			Thu 8 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::ReceiveSlotRequestOK()
{
	char* Buffer;
	ULong	BufLen,From,n=0;
	bool	got=FALSE;
	LPSLOTPOSPACK pack;
	ULong now=_Miles.GetSysTime();
	CommsTimeout time(now,CommsTimeoutLength);

	while (!got)
	{
		now=_Miles.GetSysTime();
		if (time.TimedOut(now))
			return FALSE;

		ULong to=myDPlayID;
		while (ReceiveNextMessage(Buffer,BufLen,From,to,DPRECEIVE_TOPLAYER))
		{
			if (From==0)
			{
				_DPlay.ProcessSystemMessage(Buffer,CPS_FRAG);
			}
			else //if (BufLen!=sizeof(AGGSENDPACKET))
			{
				pack=(LPSLOTPOSPACK)Buffer;

				if (pack->PacketID==PID_SLOTINFOUPDATE)
				{
					if (pack->PlayerID==myDPlayID)
					{
						if (pack->Slot)
							return TRUE;
						else
							return FALSE;
					}
				}
				else
				{
					ProcessPlayerMessage(Buffer,BufLen,From,(H2H_Player+mySlot)->status);
				}
			}
		}
	}

	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		SlotFree
//Author		Andy McMaster
//Date			Mon 12 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::SlotFree(int pos, int squadron)
{
	ULong i=0;
	if (_DPlay.PlayerCreated)
	{
		if (Host)
		{
			while	(		(i < MAXPLAYERS)
			)
			{
				if  (		(H2H_Player[i].squadron == squadron)
					&&		(H2H_Player[i].position == pos)
					&& 		(H2H_Player[i].status!=CPS_NONE)	 

				)
				{
					return FALSE;
				}
				i++;
			}

			SLOTPOSPACK s;

			s.PacketID=PID_SLOTINFOUPDATE;
			s.squadron=squadron;
			s.position=pos;
			s.PlayerID=myDPlayID;

			SendMessageToAll((char*)&s,sizeof(SLOTPOSPACK));

			return TRUE;
		}
		else if (SendSlotRequest(pos,squadron))
			if (ReceiveSlotRequestOK())
				return TRUE;
		return FALSE;
	}else
		return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		ProcessSlotInfoUpdate
//Author		Andy McMaster
//Date			Mon 12 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessSlotInfoUpdate(char* pack)
{
	LPSLOTPOSPACK p=(LPSLOTPOSPACK)pack;
	ULong n;

	if (p->Slot)
	{
		for (n=0;n<MAXPLAYERS;n++)
		{
			if (H2H_Player[n].dpid==p->PlayerID)
				break;
		}

		if (n!=MAXPLAYERS)
		{
			H2H_Player[n].squadron=p->squadron;
			H2H_Player[n].position=p->position;
		}
	}
// need to update frag screen

	UpdateFrag=TRUE;

// update ready room 

	DisplayInfo=TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetGameDetails
//Author		Andy McMaster
//Date			Tue 13 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::GetGameDetails()
{
	SendGameDetailsRequest();

	if (!ReceiveGameDetails())
	 	return FALSE;

// this is now more of a "send me beginning info" packet rather than specifically for
// csquick struc

	SendCSRequest();

	if (GameType!=COMMSCAMPAIGN)								//DAW 22Jun99
	{															//DAW 22Jun99
		if (!GetCS())
			return FALSE;
	}															//DAW 22Jun99

	if (GameType>TEAMPLAY)
	{
		if (!GetPilotsStatus())
			return FALSE;
	}

	if (GameType==COMMSCAMPAIGN)
	{
		if (!ReceiveStartupSaveGame(true))
			return FALSE;
	}
	else if (GameType==COMMSQUICKMISSION)
	{
		if (!ReceiveCampaignStruc())
			return FALSE;
	}

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		SendGameDetailsRequest
//Author		Andy McMaster
//Date			Tue 13 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendGameDetailsRequest()
{
	GAMEDETAILS pack;

	ULong retries=10;

	pack.PacketID=PID_GAMEDETAILSREQUEST;

	while (!SendMessageToAll((char*)&pack,sizeof(GAMEDETAILS)))
	{
		ULong end=timeGetTime()+100; // 10th sec

		while (timeGetTime()<end)
		{
			Sleep(0);
		}
		retries--;

		if (!retries)
			break;
	}
}

//------------------------------------------------------------------------------
//Procedure		ReceiveGameDetails
//Author		Andy McMaster
//Date			Tue 13 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::ReceiveGameDetails()
{
	char* Buffer;
	ULong	BufLen,From,n=0;
	bool	got=FALSE;
	LPGAMEDETAILS pack;
	PCOMMONDATA cpack;

// NEEDTIMEOUT
	ULong now=_Miles.GetSysTime();

	CommsTimeout time(now,CommsTimeoutLength);

	while (!got)
	{
		now=_Miles.GetSysTime();


		if (time.TimedOut(now))
			return FALSE;

		ULong to=0;
		while (ReceiveNextMessage(Buffer,BufLen,From,to,0))
		{
			if (From==0)
				_DPlay.ProcessSystemMessage(Buffer,CPS_FRAG);
			else // if (BufLen!=sizeof(AGGSENDPACKET))
			{
				pack=(LPGAMEDETAILS)Buffer;

				if (pack->PacketID==PID_GAMEDETAILSINFO)
				{
					RateDivider=pack->DataRate;
					GameType=(GameTypes)pack->GameType;
					GameIndex=pack->GameIndex;
					return TRUE;
				}
				else
				{
					cpack=(PCOMMONDATA)Buffer;

					if (cpack->PacketID==PID_HOSTBUSY)
					{
						return FALSE;
					}
				}
			}
		}
	}

	return FALSE;
}


//------------------------------------------------------------------------------
//Procedure		ResetDeadPlayers
//Author		Andy McMaster
//Date			Tue 20 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ResetDeadPlayers(AirStrucPtr ac)
{
	ULong n;

	for (n=0;n<MAXPLAYERS;n++)
	{
		if (AllocPacket[n]==ac->uniqueID.bitfieldUid.count)
		{
			deadplayers&=(ULong)(~(1<<n));
			break;
		}
	}
}


//------------------------------------------------------------------------------
//Procedure		GetBattlePosition
//Author		Andy McMaster
//Date			Tue 27 Oct 1998
//
//Description	If player is joining a game then put ac near others.
//				This is an average of all the other players ac positions
//				plus 10000 above the highest already in so that no 
//				collision occurs.
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::GetBattlePosition(UByte joiningslot)
{
	ULong xpos=0,zpos=0,ypos=0,slot;
	UByte num=0;
	AirStrucPtr ac;
	WorldStuff &world = mobileitem::GetWorld();

	joiningplayerslot=-1;										//DAW 27Jun99

	for (slot=0;slot<MAXPLAYERS;slot++)
	{
		if (H2H_Player[slot].status==CPS_3D && slot!=joiningslot)
		{
			ac=(AirStrucPtr)Persons2::ConvertPtrUID((UniqueID)AllocPacket[slot]);

			if (ac)
			{
				xpos+=ac->World.X;
				zpos+=ac->World.Z;
				num++;

				if (ac->World.Y>ypos)
					ypos=ac->World.Y;
			}
		}
	}


	ac=(AirStrucPtr)Persons2::ConvertPtrUID((UniqueID)AllocPacket[joiningslot]);

 	UWord	oldsector=	world.GetSector(ac);

	if (num)
	{
		xpos/=num;
		zpos/=num;
		ypos+=10000;

		ac->World.X=xpos;
		ac->World.Y=ypos;
		ac->World.Z=zpos;

		if (ac==Persons2::PlayerSeenAC)
		{
			Persons2::PlayerGhostAC->World.X=xpos;
			Persons2::PlayerGhostAC->World.Y=ypos;
			Persons2::PlayerGhostAC->World.Z=zpos;

// make sure flight model knows whats going on

			Persons2::PlayerSeenAC->fly.pModel->UpdateModel(Persons2::PlayerSeenAC);
		}	
	}

	UWord	newsector=	world.GetSector(ac);
	if (newsector!=oldsector)
	{
		world.RemoveFromSector(ac,oldsector);
		world.AddToWorld(ac);
	}
}

//------------------------------------------------------------------------------
//Procedure		SwitchPlayerACToAIAC
//Author		Andy McMaster
//Date			Fri 30 Oct 1998
//
//Description	When a player leaves a coop game the AC that he is in
//				must be taken over by the AI.
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::SwitchPlayerACToAIAC(AirStrucPtr AC)
{
	if (AC->uniqueID.bitfieldUid.commsmove)
	{
// only switch AC if it is a comms ac
// AC must determine what its best options are at moment of taking control

		AC->AutoMoveInfo.movecode=AUTO_FOLLOWWP;
		AC->uniqueID.bitfieldUid.commsmove=FALSE;
	}
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		SwitchAIACToPlayerAC
//Author		Andy McMaster
//Date			Fri 30 Oct 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool	DPlay::SwitchAIACToPlayerAC(AirStrucPtr AC, ULong slot)
{
// is this enough? will force ac to go through prediction phase rather than moveitem
// so willl not do any AI stuff.
// NO NO NO NO
//	player needs to set up seenac, ghostac etc
// delete old fmodel?

	AirStrucPtr gac,sac;
	UniqueID target=AC->uniqueID.bitfieldUid.count;

	sac=Persons2::PlayerSeenAC;

	if (slot==mySlot)
	{
// player should never get here
#ifndef NDEBUG
		INT3;
#endif
	}
// for both remotes and player

// update alloc packet entry
	AC->uniqueID.bitfieldUid.commsmove=TRUE;
	AllocPacket[slot]=AC->uniqueID.bitfieldUid.count;

// need to update historical buffer. just fill up with ac position. it will be the same 
// on all machines and doesnt need to be massively accurate for AAA.

	FillHistBuffer(AC,slot);

	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		NewWingmanCommand
//Author		Andy McMaster
//Date			Sun 1 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewWingmanCommand(UByte decision, UByte option, UniqueID trg)
{
	UniqueID id1,id2;

	id1=trg;
	id2=(UniqueID)((option<<7)+decision);

	NewUIDSPacket(id1,id2,PIDC_WINGMANCOMMAND);
}

//------------------------------------------------------------------------------
//Procedure		ProcessWingmanCommand
//Author		Andy McMaster
//Date			Sun 1 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessWingmanCommand(UniqueID id1, UniqueID id2, ULong slot)
{
	AirStrucPtr caller,callee;

// caller/callee is always ghostac now
	
	if (_DPlay.Implemented)
	{
		AirStrucPtr ac=(AirStrucPtr)Persons2::ConvertPtrUID((UniqueID)AllocPacket[slot]); //AMM 16/05/99
		if (ac==Persons2::PlayerSeenAC) //AMM 16/05/99
			ac=Persons2::PlayerGhostAC; //AMM 16/05/99
		caller=callee=ac; //AMM 16/05/99
	}
	else
	{
		caller=callee=(AirStrucPtr)Persons2::PlayerGhostAC; //AMM 16/05/99
	}

	AirStrucPtr target=(AirStrucPtr)Persons2::ConvertPtrUID(id1);
	if (target==Persons2::PlayerSeenAC)
		target=Persons2::PlayerGhostAC;

	ULong	decision,option;

	decision=id2&0x7f;
	option=id2>>7;

	DecisionAI* dec=DecisionAI::optiontable[decision];
	DecisionAI::OptionRef* opt=dec->GetMsgOptions();
	opt+=option;

	MESSAGE_STRUC newMsg(opt->saidmsg,MSG_HIPRIORITY,callee,target,callee);
	OverLay.SendMessageAndCall(&newMsg,opt->airesult[0]);
}

//------------------------------------------------------------------------------
//Procedure		Process1stAIACPositionPacket
//Author		Andy McMaster
//Date			Mon 2 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::Process2ndAIACPositionPacket(LPBASICPACKET p,UByte slot)
{
	SLong X,Y,Z;
	SLong dx,dy,dz;
	SWord ddx,ddy,ddz;
	UWord roll;
	UniqueID	ID;
	UWord	fakeID;
	AirStrucPtr ac,acowner;
	ULong n,c;

	if (Implemented)
	{
		ddx=p->X<<8;
		ddx+=p->Y;

		if (ddx>0)
		{
			dx=ddx<<AIPOS_SHIFT;
		}
		else
		{
			dx=-(abs(ddx)<<AIPOS_SHIFT);
		}

		ddy=p->Z<<8;
		ddy+=p->Shift<<4;
		ddy+=p->Velocity;

		if (ddy>0)
		{
			dy=ddy<<AIPOS_SHIFT;
		}
		else
		{
			dy=-(abs(ddy)<<AIPOS_SHIFT);
		}

		ddz=p->Heading<<8;
		ddz+=p->Pitch;

		if (ddz>0)
		{
			dz=ddz<<AIPOS_SHIFT;
		}
		else
		{
			dz=-(abs(ddz)<<AIPOS_SHIFT);
		}

 		roll=p->Roll<<8;
		roll+=p->byte1;

		fakeID=p->byte2<<8;
		fakeID+=p->byte3;

		ID=(UniqueID)fakeID;
		ac=(AirStrucPtr)Persons2::ConvertPtrUID(ID);
		acowner=(AirStrucPtr)Persons2::ConvertPtrUID(ac->CommsOwner);

		if (acowner)
		{
			for (n=0;n<MAXPLAYERS;n++)
			{
				if (AllocPacket[n]==acowner->uniqueID.bitfieldUid.count && ActivePlayers&1<<n)
				{
					X=dx+HistPosBuffer[n].GetCurr()->Pos.X;
					Y=dy+HistPosBuffer[n].GetCurr()->Pos.Y;
					Z=dz+HistPosBuffer[n].GetCurr()->Pos.Z;

					break;
				}
			}
		}
		else
			n=MAXPLAYERS;

		if (n==MAXPLAYERS)
		{
			n=0;

			X=dx+HistPosBuffer[n].GetCurr()->Pos.X;
			Y=dy+HistPosBuffer[n].GetCurr()->Pos.Y;
			Z=dz+HistPosBuffer[n].GetCurr()->Pos.Z;
		}

// update ac position, check to make sure ac not already in

		if (!AIACPositions[slot])
		{
			AIACPositions[slot]=new ACPOSENTRY;

			AIACPositions[slot]->next=NULL;

			AIACPositions[slot]->Pos.X=X;
			AIACPositions[slot]->Pos.Y=Y;
			AIACPositions[slot]->Pos.Z=Z;
 			AIACPositions[slot]->Roll.a=roll;
			AIACPositions[slot]->ID=ID;
			AIACPositions[slot]->extrainfo=1;

		}
		else if(AIACPositions[slot]->ID==ID)
		{
			AIACPositions[slot]->Pos.X=X;
			AIACPositions[slot]->Pos.Y=Y;
			AIACPositions[slot]->Pos.Z=Z;
 			AIACPositions[slot]->Roll.a=roll;
			AIACPositions[slot]->extrainfo|=1;

			if (AIACPositions[slot]->extrainfo==3)
			{
// we now have a full position, update the ac and delete this entry

				UpdateACFromPositionList(AIACPositions[slot]);
				DeleteEntryFromAIACPositionsList(AIACPositions[slot],slot);
			}
		}
		else
		{
			LPACPOSENTRY p=AIACPositions[slot];

			while (p->next)
			{
				if (p->ID==ID)
				{
// eh up, this ac's already in here, lets update it

					p->Pos.X=X;
					p->Pos.Y=Y;
					p->Pos.Z=Z;
 					p->Roll.a=roll;
					p->extrainfo|=1;

					if (p->extrainfo==3)
					{
// we now have a full position, update the ac and delete this entry

						UpdateACFromPositionList(p);
						DeleteEntryFromAIACPositionsList(p,slot);
					}
					return;
				}
				p=p->next;
			}

			LPACPOSENTRY temp=new ACPOSENTRY;
			temp->next=NULL;

			temp->Pos.X=X;
			temp->Pos.Y=Y;
			temp->Pos.Z=Z;
 			temp->Roll.a=roll;
			temp->ID=ID;
			temp->extrainfo=1;

			p->next=temp;
			temp=NULL;

		}
	}
}

//------------------------------------------------------------------------------
//Procedure		Process2ndAIACPositionPacket
//Author		Andy McMaster
//Date			Mon 2 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::Process1stAIACPositionPacket(LPBASICPACKET p,UByte slot)
{
	UWord Pitch,Velocity;
	UniqueID ID,unf;
	AirStrucPtr ac;
	UWord hdg;

	if (Implemented)
	{
		Pitch=p->X<<8;
		Pitch+=p->Y;

		unf=(UniqueID)(((p->Z<<8)+(p->Shift<<4))+p->Velocity);

		Velocity=p->Heading<<8;
		Velocity=+p->Pitch;

		hdg=p->Roll<<8;
		hdg+=p->byte1;

		ID=(UniqueID)((p->byte2<<8)+p->byte3);

		ac=(AirStrucPtr)Persons2::ConvertPtrUID(ID);

// update ac position, check to make sure ac not already in

		if (!AIACPositions[slot])
		{
			AIACPositions[slot]=new ACPOSENTRY;

			AIACPositions[slot]->next=NULL;

			AIACPositions[slot]->Pitch.a=Pitch;
			AIACPositions[slot]->Velocity=Velocity;
			AIACPositions[slot]->Heading.a=hdg;

			AIACPositions[slot]->Unfriendly=unf;

			AIACPositions[slot]->ID=ID;
			AIACPositions[slot]->extrainfo=2;
		}
		else if (AIACPositions[slot]->ID==ID)
		{
			AIACPositions[slot]->Pitch.a=Pitch;
			AIACPositions[slot]->Velocity=Velocity;
			AIACPositions[slot]->Heading.a=hdg;

			AIACPositions[slot]->Unfriendly=unf;

			AIACPositions[slot]->extrainfo|=2;

			if (AIACPositions[slot]->extrainfo==3)
			{
// we now have a full position, update the ac and delete this entry

				UpdateACFromPositionList(AIACPositions[slot]);
				DeleteEntryFromAIACPositionsList(AIACPositions[slot],slot);
			}
		}
		else
		{
			LPACPOSENTRY p=AIACPositions[slot];

			while (p->next)
			{
				if (p->ID==ID)
				{
// eh up, this ac's already in here, lets update it

					p->Pitch.a=Pitch;
					p->Velocity=Velocity;
					p->Heading.a=hdg;

					p->Unfriendly=unf;

					p->extrainfo|=2;

					if (p->extrainfo==3)
					{
// we now have a full position, update the ac and delete this entry

						UpdateACFromPositionList(p);
						DeleteEntryFromAIACPositionsList(p,slot);
					}
					return;
				}
				p=p->next;
			}

			LPACPOSENTRY temp=new ACPOSENTRY;
			temp->next=NULL;

			temp->Pitch.a=Pitch;
			temp->Velocity=Velocity;
			temp->Heading.a=hdg;

			temp->Unfriendly=unf;

			temp->ID=ID;
			temp->extrainfo|=2;

			p->next=temp;
			temp=NULL;
		}
	}
}


//------------------------------------------------------------------------------
//Procedure		Make1stAIACPositionPacket
//Author		Andy McMaster
//Date			Mon 2 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::Make2ndAIACPositionPacket(LPACPACKET p)
{
// instead of sending absolute values, send offset from CommsOwner
// then can fit x,y,z in 16 bits. Still need 16 bits for orientation
// X,Y = X pos
// Z, Shift+vel = Y pos
// hdg , pitch = Z pos
// roll, byte1 = Heading
// byte2, byte3 = UniqueID

	ULong n;
	LPACPOSENTRY t=MyAIACPositionsToSend;
	AirStrucPtr ac2=(AirStrucPtr)Persons2::ConvertPtrUID(t->ID);

	t->extrainfo=2;

	SLong dx,dy,dz;
	SWord ddx,ddy,ddz;

// actually should use position from historical buffer
// CANT - DELTA PACKETS ARE LIKELY TO ARRIVE B4 THIS GETS AROUND AND
// BUFFER MAY ALTER!!!!!!!!!!!!!!!!! AAAAAARRRRRRGGGGGGHHHHHHH!!!!!!!

	for (n=0;n<MAXPLAYERS;n++)
	{
		if (ac2->CommsOwner==AllocPacket[n] && ActivePlayers&1<<n)
			break;
	}

	if (n==MAXPLAYERS)
		n=0;

	dx=ac2->World.X-HistPosBuffer[n].GetCurr()->Pos.X;
	dy=ac2->World.Y-HistPosBuffer[n].GetCurr()->Pos.Y;
	dz=ac2->World.Z-HistPosBuffer[n].GetCurr()->Pos.Z;

// test deltas to put aiac just above players, on same orientation

// if any delta is greater than +/- 32767 then ac is so out of range is doesnt matter if we truncate
// ac gets same position on all machines anyway - warp will not be seen
// deltas still too big - ignore bottom bits

	if ((abs(dx)>>AIPOS_SHIFT)>32767)
	{
		if (dx>0)
			dx=32767;
		else
			dx=-32767;
	}
	if ((abs(dy)>>AIPOS_SHIFT)>32767)
	{
		if (dy>0)
			dy=32767;
		else
			dy=-32767;
	}
	if ((abs(dz)>>AIPOS_SHIFT)>32767)
	{
		if (dz>0)
			dz=32767;
		else
			dz=-32767;
	}

// get values into SWord to get correct sign when shifting

	if (dx>0)
	{
		ddx=dx>>AIPOS_SHIFT;
	}
	else
	{
		ddx=-(abs(dx)>>AIPOS_SHIFT);
	}

	if (dy>0)
	{
		ddy=dy>>AIPOS_SHIFT;
	}
	else
	{
		ddy=-(abs(dy)>>AIPOS_SHIFT);
	}

	if (dz>0)
	{
		ddz=dz>>AIPOS_SHIFT;
	}
	else
	{
		ddz=-(abs(dz)>>AIPOS_SHIFT);
	}

	p->X=ddx>>8;
	p->Y=ddx&0xff;

	p->Z=ddy>>8;
	p->Shift=(ddy>>4)&0x0f;
	p->Velocity=ddy&0x0f;

	p->Heading=ddz>>8;
	p->Pitch=ddz&0xff;

// dont need frame num so can use byte1 for bottom byte of roll

	p->Roll=ac2->roll.a>>8;
	p->byte1=ac2->roll.a&0xff;

	p->byte2=t->ID>>8;
	p->byte3=t->ID&0xff;

	p->IDCode=PIDC_AIACPOSITION2;

// now have all info - delete entry

	MyAIACPositionsToSend=t->next;
	delete t;
	t=NULL;
}

//------------------------------------------------------------------------------
//Procedure		Make2ndAIACPositionPacket
//Author		Andy McMaster
//Date			Mon 2 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::Make1stAIACPositionPacket(LPACPACKET p)
{
// X,Y = Pitch
// Z,Shift+vel = Roll
// hdg, pitch = vel
// byte2,byte3= uid

	LPACPOSENTRY t=MyAIACPositionsToSend;
	AirStrucPtr ac2=(AirStrucPtr)Persons2::ConvertPtrUID(t->ID);
	AirStrucPtr ac=(AirStrucPtr)Persons2::ConvertPtrUID(ac2->CommsOwner);

	t->extrainfo=1;
	p->X=ac2->pitch.a>>8;
	p->Y=ac2->pitch.a&0xff;

	p->Heading=ac2->vel>>8;
	p->Pitch=ac2->vel&0xff;

// has this ac got an unfriendly 

	UniqueID unfriendly=UID_NULL;

	if (ac2->ai.unfriendly)
	{
		unfriendly=ac2->ai.unfriendly->uniqueID.bitfieldUid.count;
	}

	p->Z=unfriendly>>8;
	p->Shift=(unfriendly>>4)&0x0f;
	p->Velocity=(unfriendly)&0x0f;
	p->Roll=ac2->hdg.a>>8;
	p->byte1=ac2->hdg.a&0xff;
	p->byte2=t->ID>>8;
	p->byte3=t->ID&0xff;
	p->IDCode=PIDC_AIACPOSITION1;
}

//------------------------------------------------------------------------------
//Procedure		AddAIACToPositionList
//Author		Andy McMaster
//Date			Mon 2 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::AddAIACToPositionList(AirStrucPtr ac)
{
	LPACPOSENTRY	temp=MyAIACPositionsToSend;
	LPACPOSENTRY	acpos=new ACPOSENTRY;

	acpos->ID=ac->uniqueID.bitfieldUid.count;

// dont add position here, do it when info is about to be sent to that pos is as up to date
// as possible at least on owners machine

	acpos->extrainfo=0;
	acpos->next=NULL;

// if ac is already in list dont add

	if (temp)
	{
		while (temp->next)
		{
			if (acpos->ID==temp->ID)
				return;

			temp=temp->next;
		}

		if (acpos->ID!=temp->ID)
			temp->next=acpos;

		acpos=NULL;
	}
	else
	{
		MyAIACPositionsToSend=acpos;
		acpos=NULL;
	}
}

//------------------------------------------------------------------------------
//Procedure		DeleteAIACPositionList
//Author		Andy McMaster
//Date			Mon 2 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::DeleteMyAIACPositionToSendList()
{
	LPACPOSENTRY	temp=MyAIACPositionsToSend,temp1;

	while (temp)
	{
		temp1=temp->next;
		delete temp;
		temp=temp1;
	}
	MyAIACPositionsToSend=NULL;
}

//------------------------------------------------------------------------------
//Procedure		DeleteEntryFromAIACPositionsList
//Author		Andy McMaster
//Date			Mon 2 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::DeleteEntryFromAIACPositionsList(LPACPOSENTRY p,UByte slot)
{
	LPACPOSENTRY temp,temp1;

	temp=AIACPositions[slot];

	if (temp->ID==p->ID)
	{
		temp1=AIACPositions[slot]->next;
		delete AIACPositions[slot];
		AIACPositions[slot]=temp1;
	}
	else
	{
		temp1=temp->next;

		while (temp1)
		{
			if (temp1->ID==p->ID)
				break;

			temp1=temp1->next;
			temp=temp->next;
		}
		
		if (temp1)
		{
			temp=temp1->next;

			delete temp1;
			temp1=NULL;
		}
		else
		{
			_Error.SayAndQuit("Entry not in list");
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		DeleteAIACPositionsList
//Author		Andy McMaster
//Date			Mon 2 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::DeleteAIACPositionsList()
{
	LPACPOSENTRY temp,temp1;
	ULong n;

	for (n=0;n<MAXPLAYERS;n++)
	{
		temp=AIACPositions[n];

		while (temp)
		{
			temp1=temp->next;
			delete temp;
			temp=temp1;
		}
		AIACPositions[n]=NULL;
	}
}

//------------------------------------------------------------------------------
//Procedure		PutAIACInReadyState
//Author		Andy McMaster
//Date			Mon 2 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::PutAIACInReadyState(UniqueID id)
{
}

//------------------------------------------------------------------------------
//Procedure		UpdateACFromPositionList
//Author		Andy McMaster
//Date			Mon 2 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::UpdateACFromPositionList(LPACPOSENTRY p)
{
	AirStrucPtr ac=(AirStrucPtr)Persons2::ConvertPtrUID(p->ID);

// if ac has been killed whilst delta packets arrived then ignore them

	if (ac)
	{
		if (ac->AutoMoveInfo.movecode!=AUTO_NOPPILOT)
		{
			WorldStuff &world = mobileitem::GetWorld();
			UWord oldsector=world.GetSector(ac);

			ac->World.X=p->Pos.X;
			ac->World.Y=p->Pos.Y;
			ac->World.Z=p->Pos.Z;
			ac->hdg.a=p->Heading.a;
			ac->pitch.a=p->Pitch.a;
			ac->roll.a=p->Roll.a;
			ac->vel=p->Velocity;

// update model values

			ac->fly.pModel->UpdateModel(ac);

// if ac still has an unfriendly then keep ai stuff same for now
// otherwise ac will keep toggling between combat and followwp for ever

			if (!p->Unfriendly)
			{
// ac has broken off, reset ai stuff now

				ac->ai.unfriendly=NULL;
				ac->AutoMoveInfo.movecode=AUTO_FOLLOWWP;
				ac->CommsOwner=UID_NULL;
			}

			UWord	newsector=world.GetSector(ac);

			if (newsector!=oldsector)
			{
				world.RemoveFromSector(ac,oldsector);
				world.AddToWorld(ac);
			}
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		SendACDetails
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
void	DPlay::SendACDetails()
{
	AirStrucPtr		ac;
	ASPRIMARYVALUES aspv;
	UniqueID		id;
	FMPRIMARYVALUES	fmpv;
	PRIMARYENGINEVALUES eng;
	PRIMARYELMTVALUES	elmt;
	MIPRIMARYVALUES	mipv;
	PENGINE	pEngine;
	PMAINPLANE pMainPlane;
	FP output;
	UByteP pack,temp;
	ULong packsize,numanimpacks=0,num;
	COMMONDATA c;
	AERODEVVALUES adv;
	PAERODEVICE pAeroDevice;
	PGEAR pGear;

// send all position/data/flight model stuff

	ac=*AirStruc::ACList;

	int sent=0;

	while (ac)
	{
		if (!ac->uniqueID.bitfieldUid.commsmove || (PlayerSync && ac!=Persons2::PlayerSeenAC))
		{
// determine sizeof packet

			packsize=sizeof(ULong)
					+sizeof(UByte) // 0xff if AI slotnum if player
					+(2*sizeof(UWord))
					+sizeof(ASPRIMARYVALUES)
					+sizeof(MIPRIMARYVALUES);

			pAeroDevice=ac->fly.pModel->DeviceList;

			while (pAeroDevice)
			{
				packsize+=sizeof(AERODEVVALUES);
				pAeroDevice=pAeroDevice->List.NextItem();
			}

			if (!ac->ai.PilotSkills.simpleacm && ac->AutoMoveInfo.movecode==AUTO_COMBAT && !ac->MigStatus.LaunchFlags.deaded && !ac->uniqueID.bitfieldUid.commsmove)
			{
				packsize+=sizeof(FMPRIMARYVALUES);

				pEngine = ac->fly.pModel->EngineList;

				while (pEngine)
				{
					packsize+=sizeof(PRIMARYENGINEVALUES);
					pEngine=pEngine->List.NextItem();
				}

				pMainPlane = ac->fly.pModel->MainPlaneList;

				while (pMainPlane)
				{
					packsize+=sizeof(PRIMARYELMTVALUES);
					pMainPlane=pMainPlane->List.NextItem();
				}

				pGear=ac->fly.pModel->GearList;

				while (pGear)
				{
					packsize+=sizeof(FP);
					pGear=pGear->List.NextItem();
				}
			}
			else
			{
				packsize+=sizeof(FP);
			}

			pack=new UByte[packsize];
			temp=pack;

	// put info into packet

			*(ULong*)temp=PID_ACDATA;
			temp+=sizeof(ULong);

			if (ac->uniqueID.bitfieldUid.commsmove)
			{
				UByte slot=UID2Slot(ac->uniqueID.bitfieldUid.count);

				*(UByte*)temp=slot;
			}
			else
			{
				*(UByte*)temp=0xff;
			}
			temp+=sizeof(UByte);

			*(UWord*)temp=ac->uniqueID.bitfieldUid.count;
			temp+=sizeof(UWord);

			*(UWord*)temp=ac->CommsOwner;
			temp+=sizeof(UWord);

			_Replay.GetPrimaryASData(ac,&aspv);

			memcpy(temp,&aspv,sizeof(ASPRIMARYVALUES));
			temp+=sizeof(ASPRIMARYVALUES);

			_Replay.GetPrimaryMIData(ac,&mipv);

			memcpy(temp,&mipv,sizeof(MIPRIMARYVALUES));
			temp+=sizeof(MIPRIMARYVALUES);

			pAeroDevice=ac->fly.pModel->DeviceList;

			while (pAeroDevice)
			{
				_Replay.GetAeroDevData(&adv,pAeroDevice);

				memcpy(temp,&adv,sizeof(AERODEVVALUES));
				temp+=sizeof(AERODEVVALUES);

				pAeroDevice=pAeroDevice->List.NextItem();
			}

			if (!ac->ai.PilotSkills.simpleacm && ac->AutoMoveInfo.movecode==AUTO_COMBAT && !ac->MigStatus.LaunchFlags.deaded)
			{
				_Replay.GetPrimaryFMData(ac,&fmpv);

				memcpy(temp,&fmpv,sizeof(FMPRIMARYVALUES));
				temp+=sizeof(FMPRIMARYVALUES);

				pEngine = ac->fly.pModel->EngineList;

				while (pEngine)
				{
					_Replay.GetPrimaryEngineData(&eng,pEngine);

					memcpy(temp,&eng,sizeof(PRIMARYENGINEVALUES));
					temp+=sizeof(PRIMARYENGINEVALUES);

					pEngine=pEngine->List.NextItem();
				}

				pMainPlane = ac->fly.pModel->MainPlaneList;

				while (pMainPlane)
				{
					_Replay.GetPrimaryElmtData(&elmt,pMainPlane);

					memcpy(temp,&elmt,sizeof(PRIMARYELMTVALUES));
					temp+=sizeof(PRIMARYELMTVALUES);

					pMainPlane=pMainPlane->List.NextItem();
				}

				pGear=ac->fly.pModel->GearList;

				while (pGear)
				{
					output=_Replay.GetSuspensionData(pGear);

					*(FP*)temp=output;
					temp+=sizeof(FP);

					pGear = pGear->List.NextItem();
				}
			}
			else
			{
				*(FP*)temp=ac->fly.pModel->Mass;
			}

			SendGMessageToPlayers((char*)pack,packsize);
			sent++;

			delete [] pack;

// if we are sending other players positions dont send anim, that will
// have already been sent by player 

			if (!ac->uniqueID.bitfieldUid.commsmove)
			{
				num=SHAPE.ScanAnimData((ItemPtr)ac);

				if (num!=0)
					numanimpacks++;
			}
		}
		ac=*ac->nextmobile;
	}

// send number of ac anim packs

	SendNumAnims(numanimpacks,PID_NUMACANIMS);

	ac=*AirStruc::ACList;

	while (ac)
	{
		if (!ac->uniqueID.bitfieldUid.commsmove)
		{
			num=SHAPE.ScanAnimData(ac);

			if (num)
			{
				SendAnimData(PID_ACANIM,num,ac->uniqueID.bitfieldUid.count);
			}
		}
		ac=*ac->nextmobile;
	}
}

//------------------------------------------------------------------------------
//Procedure		GetACDetails
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
Bool	DPlay::GetACDetails()
{
	static ULong need;
	static ULong got;
#ifdef SYNCDATA
	FILE* fp;
#endif

	AirStrucPtr		ac;

	switch (SyncData.acstage)
	{
	case 0:
		ac=*AirStruc::ACList;

		need=0;
		got=0;
		while (ac)
		{
			if (!ac->uniqueID.bitfieldUid.commsmove || PlayerSync)
			{
				need++;
			}

			ac=*ac->nextmobile;
		}

		if (PlayerSync)
			need--;

		SyncData.acstage++;
		break;

	case 1:

		if (need!=0)
		{
			if (GetSyncData(PID_ACDATA, need, got))
			{
				SyncData.acstage++;
			}
		}
		else
		{
			SyncData.acstage++;
		}

		break;

	case 2:

		if (GetNumAnims(PID_NUMACANIMS,need))
		{
			got=0;
			SyncData.acstage++;
		}
		break;
		 
	case 3:

		if (need!=0)
		{
			if (!GetAllAnims(PID_ACANIM,need,got))
			{
				break;
			}
		}
		SyncData.acstage=0;
		return TRUE;
	}

	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		SendMobDetails
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
void	DPlay::SendMobDetails()
{
	MobileItemPtr item;
	MIPRIMARYVALUES	mipv;
	ULong num,numanimpacks=0,packsize;
	UByteP pack,temp;

	item=mobileitem::MobileList;

	while (item)
	{
// determine sizeof packet

		packsize=sizeof(ULong)
				+sizeof(UWord)
				+sizeof(MIPRIMARYVALUES);

		pack=new UByte[packsize];
		temp=pack;

// put info into packet
		*(ULong*)temp=PID_MOBDATA;
		temp+=sizeof(ULong);

		*(UWord*)temp=item->uniqueID.bitfieldUid.count;
		temp+=sizeof(UWord);

		_Replay.GetPrimaryMIData(item,&mipv);

		memcpy(temp,&mipv,sizeof(MIPRIMARYVALUES));
		temp+=sizeof(MIPRIMARYVALUES);

		SendGMessageToPlayers((char*)pack,packsize);

		delete [] pack;

		num=SHAPE.ScanAnimData((ItemPtr)item);

		if (num!=0)
			numanimpacks++;

		item=item->nextmobile;
	}

// send number of mob anim packs
	SendNumAnims(numanimpacks,PID_NUMMOBANIMS);

// send anim data
	item=mobileitem::MobileList;

	while (item)
	{
		num=SHAPE.ScanAnimData(item);

		if (num)
		{
			SendAnimData(PID_MOBANIM,num,item->uniqueID.bitfieldUid.count);
		}

		item=item->nextmobile;
	}
}

//------------------------------------------------------------------------------
//Procedure		GetMobDetails
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
Bool	DPlay::GetMobDetails()
{
	static ULong need;
	static ULong got;
#ifdef SYNCDATA
	FILE* fp;
#endif
	MobileItemPtr	item;

	switch (SyncData.mobstage)
	{
	case 0:
		item=mobileitem::MobileList;

		need=0;
		got=0;
		while (item)
		{
			need++;

			item=*item->nextmobile;
		}
		SyncData.mobstage++;
		break;

	case 1:	 

		if (need!=0)
		{
			if (GetSyncData(PID_MOBDATA, need, got))
			{
				SyncData.mobstage++;
			}
#ifdef SYNCDATA
			fp=fopen("sync.txt","at");
			fprintf(fp,"mob data need=%d got=%d\n",need,got);
			fclose(fp);
#endif
		}
		else
		{
#ifdef SYNCDATA
			fp=fopen("sync.txt","at");
			fprintf(fp,"got all mob data\n");
			fclose(fp);
#endif

			SyncData.mobstage++;
		}

		break;

	case 2:	 

		if (GetNumAnims(PID_NUMMOBANIMS,need))
		{
#ifdef SYNCDATA
			fp=fopen("sync.txt","at");
			fprintf(fp,"mob num anims=%d\n",need);
			fclose(fp);
#endif
			got=0;

			SyncData.mobstage++;
		}
		break;

	case 3:

		if (need!=0)
		{
			if (!GetAllAnims(PID_MOBANIM,need,got))
			{
#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
				fprintf(fp,"mob anim need=%d got=%d\n",need,got);
				fclose(fp);
#endif
				break;
			}
		}

		SyncData.mobstage=0;
		return TRUE;
	}

	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		SendItemDetails
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
void	DPlay::SendItemDetails()
{
}

//------------------------------------------------------------------------------
//Procedure		GetItemDetails
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
Bool	DPlay::GetItemDetails()
{
	return TRUE;
}

//------------------------------------------------------------------------------
//Procedure		GetSyncData
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
Bool	DPlay::GetSyncData(ULong type,ULong need,ULong& got)
{
	char* buffer;
	ULong	buflen,from;
	ULong to=myDPlayID;

	while (ReceiveNextMessage(buffer,buflen,from,to,DPRECEIVE_TOPLAYER))
	{
		if (from==0)
		{
			ProcessSystemMessage(buffer,CPS_RESYNCING);
		}
		else if (from!=aggID)
		{
			if (*(ULong*)buffer==type)
			{
				switch (type)
				{
				case PID_MOBDATA:
					ProcessMobData((UByteP)buffer);
					break;
				case PID_ACDATA:
					ProcessACData((UByteP)buffer);
					break;
				}

				got++;

				if (got==need)
					return TRUE;
			}
			else 
			{
				ProcessPlayerMessage(buffer,buflen,from,CPS_RESYNCING);
			}
		}
	}
	if (got==need)
		return TRUE;
	
	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		ProcessACData
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
void	DPlay::ProcessACData(UByteP pack)
{
	PENGINE	pEngine;
	PMAINPLANE pMainPlane;
	UniqueID id;
	AirStrucPtr ac;
	WorldStuff &world = mobileitem::GetWorld();
	PAERODEVICE pAeroDevice;
	PGEAR pGear;
	FP legpos;

	pack+=sizeof(ULong); // step over PID

	bool	commsac;

	if (*(UByte*)pack==0xff)
	{
		commsac=false;
		pack+=sizeof(UByte);
		id=(UniqueID)(*(UWord*)pack);
		pack+=sizeof(UWord);
		ac=(AirStrucPtr)Persons2::ConvertPtrUID(id);
	}
	else
	{
		commsac=true;
		UByte slot=*(UByte*)pack;
		pack+=sizeof(UByte);
		ac=*Persons2::ConvertPtrUID((UniqueID)AllocPacket[slot]);
		pack+=sizeof(UWord);
	}

#ifndef NDEBUG
	if (!ac)
		INT3;
#endif

	if (!ac)
		return;

	UWord	oldsector=	world.GetSector(ac);

	ac->CommsOwner=(UniqueID)(*(UWord*)pack);
	pack+=sizeof(UWord);

	_Replay.RestorePrimaryASValues(ac,(LPASPRIMARYVALUES)pack);
	pack+=sizeof(ASPRIMARYVALUES);

	_Replay.RestorePrimaryMIValues((MobileItemPtr)ac,(LPMIPRIMARYVALUES)pack);
	pack+=sizeof(MIPRIMARYVALUES);

	pAeroDevice=ac->fly.pModel->DeviceList;
	while (pAeroDevice)
	{
		_Replay.RestoreAeroDevData((LPAERODEVVALUES)pack,pAeroDevice);

		pack+=sizeof(AERODEVVALUES);

		pAeroDevice=pAeroDevice->List.NextItem();
	}

	if (!ac->ai.PilotSkills.simpleacm && ac->AutoMoveInfo.movecode==AUTO_COMBAT && !ac->MigStatus.LaunchFlags.deaded)
	{
		_Replay.RestorePrimaryFMData(ac,(LPFMPRIMARYVALUES)pack);
		pack+=sizeof(FMPRIMARYVALUES);

		pEngine = ac->fly.pModel->EngineList;

		while (pEngine)
		{
			_Replay.RestorePrimaryEngineData((LPPRIMARYENGINEVALUES)pack,pEngine);

			pack+=sizeof(PRIMARYENGINEVALUES);

			pEngine=pEngine->List.NextItem();
		}

		pMainPlane = ac->fly.pModel->MainPlaneList;

		while (pMainPlane)
		{
			_Replay.RestorePrimaryElmtData((LPPRIMARYELMTVALUES)pack,pMainPlane);

			pack+=sizeof(PRIMARYELMTVALUES);

			pMainPlane=pMainPlane->List.NextItem();
		}

		pGear=ac->fly.pModel->GearList;

		while (pGear)
		{
			legpos=*(FP*)pack;
			_Replay.RestoreSuspensionData(legpos,pGear);
			pack+=sizeof(FP);

	 		pGear = pGear->List.NextItem();
		}
	}
	else
	{
		ac->fly.pModel->Mass=*(FP*)pack;
		ac->fly.pModel->FirstCycle=TRUE;
	}


	_Replay.ResetSecondaryFMValues(ac);								  //AMM 17/06/99

	UWord	newsector=world.GetSector(ac);
	if (newsector!=oldsector)
	{
		world.RemoveFromSector(ac,oldsector);
		world.AddToWorld(ac);
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessMobData
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
void	DPlay::ProcessMobData(UByteP pack)
{
	UniqueID id;
	MobileItemPtr item;
	WorldStuff &world = mobileitem::GetWorld();

	pack+=sizeof(ULong); // step over PID

	id=(UniqueID)(*(UWord*)pack);
	pack+=sizeof(UWord);

	item=(MobileItemPtr)Persons2::ConvertPtrUID(id);
	UWord	oldsector=	world.GetSector(item);

#ifndef NDEBUG
	if (!item)
		INT3;
#endif

	if (!item)
		return;

	_Replay.RestorePrimaryMIValues(item,(LPMIPRIMARYVALUES)pack);

	UWord	newsector=world.GetSector(item);
	if (newsector!=oldsector)
	{
		world.RemoveFromSector(item,oldsector);
		world.AddToWorld(item);
	}
}

//------------------------------------------------------------------------------
//Procedure		SendAnimData
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
void	DPlay::SendAnimData(ULong type,ULong num, UniqueID id)
{
	ULong packsize;
	UByteP pack,temp;

	packsize=(2*sizeof(ULong)) // num+packet id
			+sizeof(UWord)
			+num*(sizeof(UWord)+sizeof(UByte));

	packsize+=sizeof(UWord); // shape
	pack=new UByte[packsize];
	temp=pack;

	*(ULong*)temp=type;
	temp+=sizeof(ULong);

	*(UWord*)temp=id;
	temp+=sizeof(UWord);

	ItemPtr i=*Persons2::ConvertPtrUID(id);

	MakeAnimPack(num,temp,i->shape);

	SendGMessageToPlayers((char*)pack,packsize);

	delete [] pack;
}

//------------------------------------------------------------------------------
//Procedure		SendNumAnims
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
void	DPlay::SendNumAnims(ULong num,ULong type)
{
	COMMONDATA c;

	c.PacketID=type;
	c.PlayerID=num;

	SendGMessageToPlayers((char*)&c,sizeof(COMMONDATA));

}

//------------------------------------------------------------------------------
//Procedure		GetAllAnims
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
Bool	DPlay::GetAllAnims(ULong type, ULong need, ULong& got)
{
	char* buffer;
	ULong buflen,from;
	ItemPtr item;
	UByteP pack;
	UniqueID id;
	ULong to=myDPlayID;

	while (ReceiveNextMessage(buffer,buflen,from,to,DPRECEIVE_TOPLAYER))
	{
		if (from==0)
		{
			ProcessSystemMessage(buffer,CPS_RESYNCING);
		}
		else if (from!=aggID)
		{
			if (*(ULong*)buffer==type)
			{
				pack=(UByteP)buffer;
				pack+=sizeof(ULong); // step over PID

				id=(UniqueID)(*(UWord*)pack);
				pack+=sizeof(UWord);

				item=(ItemPtr)Persons2::ConvertPtrUID(id);

				if (item) //AMM 04/06/99
					ProcessAnimPack(pack,item);

				got++;

				if (got==need)
					return TRUE;
			}
			else
			{
				ProcessPlayerMessage(buffer,buflen,from,CPS_RESYNCING);
			}
		}
	}
	if (got==need)
		return TRUE;

	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		GetNumAnims
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
Bool	DPlay::GetNumAnims(ULong type,ULong& num)
{
	char* buffer;
	ULong buflen,from;
	PCOMMONDATA c;
	ULong to=myDPlayID;

	while (ReceiveNextMessage(buffer,buflen,from,to,DPRECEIVE_TOPLAYER))
	{
		if (from==0)
		{
			ProcessSystemMessage(buffer,CPS_RESYNCING);
		}
		else if (from!=aggID)
		{
			c=(PCOMMONDATA)buffer;

			if (c->PacketID==type)
			{
				num=c->PlayerID;
				return TRUE;
			}
			else
			{
				ProcessPlayerMessage(buffer,buflen,from,CPS_RESYNCING);
			}
		}
	}

	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		MakeAnimPack
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
void	DPlay::MakeAnimPack(ULong num,UByteP temp,ShapeNum shape)
{
	UWord deltaoffset;
	UByte newbyte;
	ULong n;

// send shape in case AC has blown up or crashed etc

	*(UWord*)temp=(UWord)shape;
	temp+=sizeof(UWord);

	*(ULong*)temp=num;
	temp+=sizeof(ULong);

	for (n=0; n < num; n++)
	{
		deltaoffset=SHAPE.AnimDeltaList[n].deltaoffset;
		newbyte=SHAPE.AnimDeltaList[n].newbyte;

		*(UWord*)temp=deltaoffset;
		temp+=sizeof(UWord);
		*temp=newbyte;
		temp+=sizeof(UByte);
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessAnimPack
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
void	DPlay::ProcessAnimPack(UByteP datapack,ItemPtr item)
{
	UWord	deltaoffset;
	UByte	newbyte;
	ULong n,num,index=0;

	UByte	pack;

	UWord newshape;

	newshape=*(UWord*)datapack;
	datapack+=sizeof(UWord);

	item->shape=(ShapeNum)newshape;	 //AMM 07/06/99

	if (item->MigStatus.LaunchFlags.size==AirStrucSize)
		pack=((AirStrucPtr)item)->weap.AircraftStatus.Weapons;
	else
		pack=0;

	num=*(ULong*)datapack;
	datapack+=sizeof(ULong);

	if (SHAPE.ResetAnimData_NewShape(item,(ShapeNum)newshape,pack))
	{
		while (num--)
		{
			deltaoffset=*(UWord*)datapack;
			datapack+=sizeof(UWord);
			newbyte=*datapack;
			datapack++;

			SHAPE.AnimDeltaList[index].deltaoffset=deltaoffset;
			SHAPE.AnimDeltaList[index++].newbyte=newbyte;
		}

		SHAPE.PatchAnimData(item,index);
	}
	else
	{
#ifndef NDEBUG
		INT3;
#endif
	}
}

//------------------------------------------------------------------------------
//Procedure		SyncCommsWorld
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
Bool	DPlay::SyncCommsWorld()
{
#ifdef SYNCDATA
	FILE* fp;
#endif
	static ULong timeout;

	if (SyncData.resettime)
	{
		timeout=(_Miles.GetSysTime()+CommsTimeoutLength);
		SyncData.resettime=false;
	}

	if (_Miles.GetSysTime()>timeout || RestartWorldSync)
	{
#ifdef SYNCDATA
		fp=fopen("sync.txt","at");
		fprintf(fp,"TIMEOUT\n");
		fclose(fp);
#endif
		SyncData.resettime=true;

		InitSyncData(true);
		SendRestartWorldSyncMessage();
		return FALSE;
	}

	char* buffer;
	ULong	buflen,from,n;
	PCOMMONDATA p;

/* need all items in world to be the same on all machines after resync. 
Need: AC positions/data/flight models and anims
	   Mobiles positions/data and anims
	   Items anims.

  All AC and mob positions/data needs to be sent apart from players.
  - Players send their own info, host sends all AI info
  For anims, host counts number of anims in each section (AC,mob,item) and sends this
  followed by anim packets.

  //What about if bfields are out of step?
  //Is it possible to send this info whilst game is still going for other players? then just
  //accomodate player in like deathmatch resync?

  //- Need to send host bfield list, compare with one players have and if different do 
  //something about it, i.e. process battlefields  Only need to do this is quick mission/campaign
  -Dont need to do now, bfields are static

  - Does DeadStream need to be transmitted? Probably!!!!!
*/

	if (Host)
	{
		if (!SyncData.sentinfo)
		{
#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
			fprintf(fp,"not sent info\n");
				fclose(fp);
#endif
			if (!WorldSyncPhaseReady)
			{
#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
			fprintf(fp,"worldsyncphase NOT ready\n");
				fclose(fp);
#endif
				if (!SyncData.wsrneeded)
				{
					SyncData.wsrneeded=ActivePlayers^(1<<mySlot);
				}

				ULong to=myDPlayID;
				while (ReceiveNextMessage(buffer,buflen,from,to,DPRECEIVE_TOPLAYER))
				{
					if (from==0)
					{
						ProcessSystemMessage(buffer,CPS_RESYNCING);
					}
					else if (from!=aggID)
					{
						if (((PCOMMONDATA)buffer)->PacketID==PID_READYWORLD)
						{
							SyncData.wsrgot+=1<<DPID2Slot(from);
						}
						else
						{
							ProcessPlayerMessage(buffer,buflen,from,CPS_RESYNCING);
						}
					}
					if (SyncData.wsrgot==SyncData.wsrneeded)
					{
						break;
					}
				}

#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
	fprintf(fp,"okp: got=%d, need=%d\n",SyncData.wsrgot,SyncData.wsrneeded);
				fclose(fp);
#endif

				if (SyncData.wsrgot==SyncData.wsrneeded)
				{
					WorldSyncPhaseReady=true;
					SyncData.wsrgot=0;
				}
			}
			else
			{
#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
			fprintf(fp,"World sync phase READY\n");
				fclose(fp);
#endif

				SendACDetails();
				SendMobDetails();

 				SendAAAList();

// dead item anims and status are sent in dead stream. Alive items will have
// no anim data, so dont need to transmit - its all done by deadstream

				SendDeadStream();
				SendBackupRandomList(false);

#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
				fprintf(fp,"Sent all world data OK\n");
				fclose(fp);
#endif

				SyncData.wsmneeded=ActivePlayers^(1<<mySlot);

				SyncData.wsmgot=0;
				SyncData.sentinfo=true;
			}
		}
		else
		{
// need all gotworldmessages

			ULong to=myDPlayID;
			while (ReceiveNextMessage(buffer,buflen,from,to,DPRECEIVE_TOPLAYER))
			{
				if (from==0)
				{
					ProcessSystemMessage(buffer,CPS_RESYNCING);
				}
				else if (from!=aggID)
				{
					if (((PCOMMONDATA)buffer)->PacketID==PID_GOTWORLD)
					{
						SyncData.wsmgot+=1<<DPID2Slot(from);
					}
					else
					{
						ProcessPlayerMessage(buffer,buflen,from,CPS_RESYNCING);
					}
				}
			}

#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
				fprintf(fp,"okp: got=%d, need=%d\n",SyncData.wsmgot,SyncData.wsmneeded);
				fclose(fp);
#endif
			if (SyncData.wsmgot==SyncData.wsmneeded)
			{
				SyncData.sentinfo=FALSE;
				WorldSyncPhase=FALSE;
				WorldSyncPhaseReady=false;
				SyncData.resettime=true;
#ifdef INITCHECK
				FILE* pp=fopen("sync.txt","at");
				fprintf(pp,"WORLDSYNC OK\n");
				fclose(pp);
#endif
				PlayerSync=false;
				return TRUE;
			}
		}	
	}
	else
	{
		switch (SyncData.stage)
		{

		case 1:
// send an "I'm ready to receive world" packet

			COMMONDATA p;
			p.PacketID=PID_READYWORLD;
			SendGMessageToPlayers((char*)&p,sizeof(COMMONDATA));
			SyncData.stage++;
			break;

		case 2:
			if (GetACDetails())
			{
#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
				fprintf(fp,"got ac details\n");
				fclose(fp);
#endif
				SyncData.stage++;
			}

			break;
 
		case 3:
			if (GetMobDetails())
			{
#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
				fprintf(fp,"got mob details\n");
				fclose(fp);
#endif
				SyncData.stage++;
			}

			break;

		case 4:

 			if (GetAAAList())
			{
#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
				fprintf(fp,"got aaalist\n");
				fclose(fp);
#endif
				SyncData.stage++;
			}
			break;

		case 5:
			if (GetDeadStream())
			{
#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
				fprintf(fp,"got deadstream\n");
				fclose(fp);
#endif
				SyncData.stage++;
			}
			break;
		case 6:
			if (GetRandomList())
			{
#ifdef SYNCDATA
				fp=fopen("sync.txt","at");
				fprintf(fp,"got randlist\n");
				fclose(fp);
#endif
				SendGotWorldMessage();
				WorldSyncPhase=FALSE;
				WorldSyncPhaseReady=false;
				SyncData.stage=1;
				SyncData.resettime=true;
#ifdef INITCHECK
				FILE* pp=fopen("sync.txt","at");
				fprintf(pp,"WORLDSYNC OK\n");
				fclose(pp);
#endif
				PlayerSync=false;
				return TRUE;
			}
		}
	}
	return FALSE;
}

//------------------------------------------------------------------------------
//Procedure		SendGotWorldMessage
//Author		Andy McMaster
//Date			Thu 12 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendGotWorldMessage()
{
	COMMONDATA p;

	p.PacketID=PID_GOTWORLD;

	SendGMessageToPlayers((char*)&p,sizeof(COMMONDATA));
}


//------------------------------------------------------------------------------
//Procedure		FillHistBuffer
//Author		Andy McMaster
//Date			Mon 23 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::FillHistBuffer(AirStrucPtr ac, ULong slot)
{
	HistPosBuffer[slot].InitBuffer();
	HistPosBuffer[slot].SetTempCurr();
	do 
	{
		HistPosBuffer[slot].GetTemp()->Pos.X=ac->World.X;
		HistPosBuffer[slot].GetTemp()->Pos.Y=ac->World.Y;
		HistPosBuffer[slot].GetTemp()->Pos.Z=ac->World.Z;

		HistPosBuffer[slot].GetTemp()->Heading=ac->hdg;
		HistPosBuffer[slot].GetTemp()->Pitch=ac->pitch;
		HistPosBuffer[slot].GetTemp()->Roll=ac->roll;
		HistPosBuffer[slot].GetTemp()->Velocity=ac->vel;

		HistPosBuffer[slot].UpdateTemp();
	} 
	while (HistPosBuffer[slot].GetTemp()!=HistPosBuffer[slot].GetCurr());
}


//------------------------------------------------------------------------------
//Procedure		OwnerChange
//Author		Andy McMaster
//Date			Fri 27 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::OwnerChange(ULong newownerid,AirStrucPtr AIAC)
{
	UByte b1,b2,b3;
	ULong n=0;

	if (!AIAC->uniqueID.bitfieldUid.commsmove)
	{
// byte1 and byte2 contain ID of AIAC who wants to change owner
// byte3 contains slot of player who AIAC wants to be owner

		b1=(AIAC->uniqueID.bitfieldUid.count)>>8;
		b2=(AIAC->uniqueID.bitfieldUid.count)&0xff;

		if (newownerid==255)
		{
			b3=255;
		}
		else
		{
			for (n=0;n<MAXPLAYERS;n++)
			{
				if (AllocPacket[n]==newownerid)
				{
					b3=n;
					break;
				}
			}
		}

	// if player is bad, keep existing one
		
		if (n!=MAXPLAYERS)
		{
			NewSpecial(PIDC_OWNERCHANGE,b1,b2,b3);
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessOwnerChange
//Author		Andy McMaster
//Date			Fri 27 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessOwnerChange(LPBASICPACKET p)
{
	UniqueID id;
	UniqueID newowner;
	AirStrucPtr AIAC;
	AirStrucPtr CommsAC;

	id=(UniqueID)(((p->byte1)<<8)+p->byte2);

	AIAC=(AirStrucPtr)Persons2::ConvertPtrUID(id);

	if (AIAC)
	{
		newowner=(UniqueID)AllocPacket[p->byte3];

		if (newowner==UID_NULL)
		{
			AIAC->CommsOwner=UID_NULL;
		}
		else
		{
			CommsAC=(AirStrucPtr)Persons2::ConvertPtrUID(newowner);

			if (CommsAC)
			{
				AIAC->CommsOwner=newowner;
			}
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		InitialiseOwners
//Author		Andy McMaster
//Date			Fri 27 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::InitialiseOwners()
{
	AirStrucPtr ac;

	ac=*AirStruc::ACList;

	while (ac)
	{
		ac->CommsOwner=UID_NULL;
		ac=*ac->nextmobile;
	}

// possibly have ac in players flight starting with player as owner otherwise
// AI ac only get owner when they engage in combat with a player or an ac
// that already has a owner, in which case they inherit that owner
}


//------------------------------------------------------------------------------
//Procedure		AmOwner
//Author		Andy McMaster
//Date			Fri 27 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	DPlay::AmOwner(AirStrucPtr ac)
{
	AirStrucPtr temp;

	temp=(AirStrucPtr)Persons2::ConvertPtrUID(ac->CommsOwner);

	if (temp==Persons2::PlayerSeenAC)
		return true;

	return false;
}

//------------------------------------------------------------------------------
//Procedure		OwnerChangeDecision
//Author		Andy McMaster
//Date			Fri 27 Nov 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::OwnerChangeDecision(AirStrucPtr me, AirStrucPtr him)
{
	if (Implemented)							
	{								
		if (!me->CommsOwner && !me->uniqueID.bitfieldUid.commsmove)
		{
// only change if I dont have an owner

			if (him->uniqueID.bitfieldUid.commsmove)					
			{												
// engage with player, he becomes owner

				if (him->uniqueID.bitfieldUid.count!=me->CommsOwner)		
				{											
					OwnerChange(him->uniqueID.bitfieldUid.count,me);
				}											
			}
			else if (him->CommsOwner && !me->CommsOwner)
			{
// engage with ac with an owner, inherit his owner

				OwnerChange(him->CommsOwner,me);
			}
			else
			{
// an AI AC with no owner, lets just kick ass
			}
		}													
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessBFieldInfo
//Author		Andy McMaster
//Date			Fri 4 Dec 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessBFieldInfo(char* info,ULong& bfctrl,FileNum& filelist)
{
	char* ptr=info;
	ULong n,val;

	ptr += sizeof (ULong);
	bfctrl = *(ULong*)ptr;
	ptr += sizeof (ULong);
	filelist = *(FileNum*)ptr;
	ptr += sizeof (FileNum);

	n = 0;

	for (;;)
	{
		val = *(int*)ptr;
		if (val!=BAD_RV)
		{
			ptr += sizeof (int);

			Persons2::SetLoc (n, val);
			n++;
		}
		else
			break;
	}
}

//------------------------------------------------------------------------------
//Procedure		SendBFieldsToPlayers
//Author		Andy McMaster
//Date			Fri 4 Dec 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendBFieldsToPlayers(bool request)
{
	LPGRLIST	temp=_Replay.bfgrlist;
	ULong PackSize,n,MaxPackSize;
	char *pack,*tpack;
	LPREPGRENTRY vals;
	UWord ordernum=0;

	if (!Host)
		return;

	while (temp)
	{
// get packet size

		MaxPackSize=sizeof(ULong); //packet ID
		MaxPackSize+=sizeof(ULong); //battlefieldtype
		MaxPackSize+=sizeof(UWord); // bfield order num
		MaxPackSize+=sizeof(FileNum);

		PackSize=MaxPackSize;

		vals=temp->list;

		while (vals)
		{
			MaxPackSize+=sizeof(int)*2;
			vals=vals->next;
		}

// construct packet

		pack=new char[MaxPackSize];
		tpack=pack;

		*(ULong*)tpack=PID_BFIELDPACKETIDENT;
		tpack+=sizeof(ULong);

		*(UWord*)tpack=ordernum++;
		tpack+=sizeof(UWord);

		*(ULong*)tpack=temp->bfctrl;
		tpack+=sizeof(ULong);
		*(FileNum*)tpack=temp->file;
		tpack+=sizeof(FileNum);

		vals=temp->list;

		bool finished=false;
		ULong val;
		ULong m;
		ULong t[8];
		UByte set=0;

		while (!finished && vals)
		{
			for (m=0;m<8;m++)
			{
				val=vals->val;
				vals=vals->next;
				t[m]=val;

				if (t[m]!=0)
					set|=0x1<<m;

				if (t[m]==BAD_RV || !vals)
				{
					finished=TRUE;
					break;
				}
			}

			*(UByte*)tpack=set;
			tpack+=sizeof(UByte);
			PackSize+=sizeof(UByte);

			for (m=0;m<8;m++)
			{
				if (set&0x1<<m)
				{
					*(int*)tpack=t[m];
					tpack+=sizeof(int);
					PackSize+=sizeof(int);
				}
			}
			set=0;
		}

		Bool sentok = BOOL_FALSE;

		ULong now=_Miles.GetSysTime();
		CommsTimeout time(now,_DPlay.CommsTimeoutLength); 

		while (sentok == BOOL_FALSE)
		{
			now=_Miles.GetSysTime();

			if (time.TimedOut(now))
				_Error.SayAndQuit("Timed out (SBF)");

			if (request)
			{
				sentok=SendMessageToAll(pack,PackSize);
			}
			else
			{
				sentok=SendGMessageToPlayers(pack,PackSize);
			}
		}

		Sleep(10);

		delete [] pack;
		pack=NULL;

		temp=temp->next;
	}
}

//------------------------------------------------------------------------------
//Procedure		ReceiveBFieldInfo
//Author		Andy McMaster
//Date			Fri 4 Dec 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	DPlay::ReceiveBFieldInfo()
{
	char* buffer;
	ULong	buflen,from;
	bool	got=false;
	ULong bfctrl;
	FileNum	filelist;

	ULong to=myDPlayID;

	while (ReceiveNextMessage(buffer,buflen,from,to,DPRECEIVE_TOPLAYER))
	{
		if (from==0)
		{
			ProcessSystemMessage(buffer,CPS_RESYNCING);
		}
		else if (from!=aggID)
		{
			if(*(ULong*)buffer == PID_BFIELDPACKETIDENT)
			{
				ProcessBFieldInfo(buffer,bfctrl,filelist);
				_Replay.AddFileAndGlobRefsToList(filelist,bfctrl);

				if (bfctrl==Persons2::FINISHBF)
					got=true;
			}
			else
			{
				ProcessPlayerMessage(buffer,buflen,from,CPS_RESYNCING);
			}
		}
	}

	if (got)
	{
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
//Procedure		ExitCommsAndReplay3D
//Author		Andy McMaster
//Date			Fri 11 Dec 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ExitCommsAndReplay3D()
{
	if (Implemented)
	{
#ifdef TIMERTIME
		if (timer)
			fclose(timer);
#endif
		H2H_Player[mySlot].status=CPS_DEBRIEF;					//DAW 22Jun99
		_Agg.Running=FALSE;
		AllowJoins=TRUE;
		SendGonePacket();
		ResetCommsGame();
		FlyNowFlag=FALSE;										//DAW 22Jun99

// for comms build a new list each time
		_Replay.DeleteFileAndGlobRefList(_Replay.bfgrlist); 	//AMM 14Dec98
	}
	else if (_Replay.Record)
	{
		_Replay.StopRecord();
		_Replay.stopforaccel=false;
	}
	else if (_Replay.Playback)
	{
		_Replay.skipOK=false;
		delete [] _Replay.backupsmoke;
		_Replay.backupsmoke=NULL;
		_Replay.tempbackupsmoke=NULL;
	}
	_Replay.ResetReplayData();
}


//------------------------------------------------------------------------------
//Procedure		NewKillLauncher
//Author		Andy McMaster
//Date			Fri 15 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewKillLauncher(UniqueID id)
{
	if (id)
	{
		if (Implemented || _Replay.Record)
		{
			NewSpecial(PIDC_KILLLAUNCHER,(UByte)(id>>8),(UByte)(id&0xff),CommsKiller);
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessKillLauncher
//Author		Andy McMaster
//Date			Fri 15 Jan 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessKillLauncher(LPBASICPACKET p)
{
	UniqueID id;
	ULong	fake;
	AirStrucPtr ac;

	fake=p->byte1<<8;
	fake+=p->byte2;

	id=(UniqueID)fake;

	ac=(AirStrucPtr)Persons2::ConvertPtrUID(id);

	Trans_Obj.KillLauncherComms(ac);
	ac->MigStatus.LaunchFlags.deaded=TRUE;
	ac->MigStatus.LaunchFlags.deadtime=1;

	AirStrucPtr killer=NULL;

	if (p->byte3!=255)
	{
		killer=*Persons2::ConvertPtrUID((UniqueID)AllocPacket[p->byte3]);
	}

	SetScore(ac,killer);
}

//------------------------------------------------------------------------------
//Procedure		GetPlayerMessage
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
char*	DPlay::GetPlayerMessage(ULong index)
{
	ULong n,m;

// index is ([player]*MAXPLAYERS)+[messnum]

	for (n=0;n<MAXPLAYERS;n++)
	{
		for (m=0;m<NUMRADIOMESSAGES;m++)
		{
			if ((n*MAXPLAYERS)+m==index)
				return Messages[n][m];
		}
	}
	return NULL;
}

//------------------------------------------------------------------------------
//Procedure		GetPlayerName
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
char*	DPlay::GetPlayerName(ULong	n)
{
	return H2H_Player[n].name;
}

//------------------------------------------------------------------------------
//Procedure		GetPlayerNumber
//Author		Robert Slater
//Date			Mon 22 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ULong	DPlay::GetPlayerNumber(ULong	id)
{
 	ULong n;
 
 	for (n=0;n<MAXPLAYERS;n++)
 	{
 		if (AllocPacket[n]==id)
 			return n;
 	}

	return 0;
}


//------------------------------------------------------------------------------
//Procedure		ID2Slot
//Author		Andy McMaster
//Date			Fri 26 Feb 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ULong DPlay::DPID2Slot(ULong id)
{
	ULong n;
	for (n=0;n<MAXPLAYERS;n++)
		if (H2H_Player[n].dpid==id)
			break;
	return n;
}

//------------------------------------------------------------------------------
//Procedure		UID2Slot
//Author		Andy McMaster
//Date			Thu 17 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
inline ULong DPlay::UID2Slot(UWord id)
{
	ULong n;
	for (n=0;n<MAXPLAYERS;n++)
		if (AllocPacket[n]==id)
			break;

	return n;
}


//------------------------------------------------------------------------------
//Procedure		AddHitterScorer
//Author		Andy McMaster
//Date			Thu 11 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::AddHitterScorer(UByte pnum,LPCOMMSCOLLISION coll)
{
	LPCLAIMER temp=new CLAIMER;
	 
	temp->id=coll->id;
	temp->offset=coll->offset;
	temp->type=coll->type;
	temp->str=coll->str;
	temp->hitterscorer=coll->hitterscorer;
	temp->pnum=pnum;
	temp->next=NULL;

// want to search for scorer in order that collisions occured so must add
// new entries to end of list

	if (!HSB)
	{
		HSB=temp;
		temp=NULL;
	}
	else
	{
		LPCLAIMER temp2=HSB;

		while (temp2->next)
		{
			temp2=temp2->next;
		}
		temp2->next=temp;
		temp=NULL;
	}
}

//------------------------------------------------------------------------------
//Procedure		GetHitterScorer
//Author		Andy McMaster
//Date			Thu 11 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
ItemPtr	DPlay::GetHitterScorer(SLong uniqueid,SLong element,SLong hittype,SLong hitstrength,UByte pnum)
{
	LPCLAIMER temp=HSB,temp1=HSB;
   	ItemPtr retval=NULL;

	if (!HSB)
		return NULL;

	if (temp->pnum==pnum
	&& temp->id==uniqueid
	&& temp->offset==element
	&& temp->type==hittype
	&& temp->str==hitstrength)
	{
		retval=(ItemPtr)Persons2::ConvertPtrUID(temp->hitterscorer);
		HSB=temp->next;
		delete temp;
		return retval;
	}
	temp=temp->next;

	while (temp)
	{
		if (temp->pnum==pnum
		&& temp->id==uniqueid
		&& temp->offset==element
		&& temp->type==hittype
		&& temp->str==hitstrength)
		{
			retval=(ItemPtr)Persons2::ConvertPtrUID(temp->hitterscorer);
			temp1->next=temp->next;
			delete temp;
			break;
		}
		temp=temp->next;
		temp1=temp1->next;
	}

	return retval;
}

//------------------------------------------------------------------------------
//Procedure		NewScoreClaim
//Author		Andy McMaster
//Date			Thu 11 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewScoreClaim(ItemPtr hitterscorer,UniqueID trg)
{
	UByte b1,b2,b3;

	NewUIDSPacket(hitterscorer->uniqueID.bitfieldUid.count,trg,PIDC_SCORECLAIM);
}

//------------------------------------------------------------------------------
//Procedure		ProcessScoreClaim
//Author		Andy McMaster
//Date			Thu 11 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessScoreClaim(UniqueID hitterid,UniqueID trgid)
{
	ItemPtr hitter,trg;

	hitter=(ItemPtr)Persons2::ConvertPtrUID(hitterid);
	trg=(ItemPtr)Persons2::ConvertPtrUID(trgid);

	if (trg && hitter)
		trg->ScoreSimpleItemKilled(true,(AirStrucPtr)hitter);
// in coop games update score here as well

	if (hitter && hitter->MigStatus.LaunchFlags.size!=AIRSTRUCSIZE)
		hitter=NULL;

	SetScore((AirStrucPtr)trg,hitter);
}

//------------------------------------------------------------------------------
//Procedure		TestForUIDSPacket
//Author		Andy McMaster
//Date			Thu 11 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	DPlay::TestForUIDSPacket(LPSPECPACKET s)
{
  	if(UIDSBuffer.NumEntries()==0)
		return false;

	UWord id1,id2;

	id1=UIDSBuffer.GetCurr()->uid1;
	id2=UIDSBuffer.GetCurr()->uid2;
	
	s->ID=UIDSBuffer.GetCurr()->IDCode;
	s->ID+=(id1&0x0f);

	s->spec1=(id1>>4)&0xff;
	s->spec2=(id1>>12)&0x03;

	s->spec2+=(id2&0x3f)<<2;
	s->spec3=id2>>6;

	UIDSBuffer.UpdateCurr();

	return true;
}

//------------------------------------------------------------------------------
//Procedure		MakeUIDSPacket
//Author		Andy McMaster
//Date			Thu 11 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::MakeUIDSPacket(LPSPECPACKET s,LPBASICPACKET p)
{
	p->IDCode=s->ID;
	p->byte1=s->spec1;
	p->byte2=s->spec2;
	p->byte3=s->spec3;
}

//------------------------------------------------------------------------------
//Procedure		ProcessUIDSPacket
//Author		Andy McMaster
//Date			Thu 11 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessUIDSPacket(LPBASICPACKET p, ULong slot)
{
	UniqueID id1,id2;

	GetUIDSFromPacket(id1,id2,p);

	switch (p->IDCode&0xf0)
	{
	case PIDC_SCORECLAIM:
		ProcessScoreClaim(id1,id2);
		break;

	case PIDC_WINGMANCOMMAND:
		ProcessWingmanCommand(id1,id2,slot);
		break;

	default:
		break;
	}
}

//------------------------------------------------------------------------------
//Procedure		NewUIDSPacket
//Author		Andy McMaster
//Date			Thu 11 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewUIDSPacket(UniqueID id1,UniqueID id2,UByte code)
{
	if (Implemented || _Replay.Record)
	{
		if(UIDSBuffer.NumEntries()>=BUFFERLENGTH)				//AMM 02Jul99
		{	
			UIDSBuffer.UpdateCurr();
		}
		 
		UIDSBuffer.GetNext()->uid1=id1;
		UIDSBuffer.GetNext()->uid2=id2;
		UIDSBuffer.GetNext()->IDCode=code;

		UIDSBuffer.AddEntryAndUpdateNext();
	}
}

//------------------------------------------------------------------------------
//Procedure		GetUIDSFromPacket
//Author		Andy McMaster
//Date			Fri 12 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::GetUIDSFromPacket(UniqueID& id1, UniqueID& id2,LPBASICPACKET p)
{
	UWord uid1,uid2;

	uid1=p->IDCode&0x0f;
	uid1+=(p->byte1<<4);
	uid1+=((p->byte2&0x03)<<12);

	uid2=(p->byte2>>2)&0x3f;
	uid2+=p->byte3<<6;

	id1=(UniqueID)uid1;
	id2=(UniqueID)uid2;

}

//------------------------------------------------------------------------------
//Procedure		DisplayPlayerSelectedTimeAccelMessage
//Author		Andy McMaster
//Date			Mon 15 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::DisplayPlayerSelectedTimeAccelMessage(ULong slot,bool on)
{
	ItemPtr	i=(ItemPtr)Persons2::ConvertPtrUID((UniqueID)AllocPacket[slot]);

	if (on)
		DisplayPlayerMessage(i,i,ST_PLAYERSELACCEL);
	else
		DisplayPlayerMessage(i,i,ST_PLAYERDESELACCEL);
}

//------------------------------------------------------------------------------
//Procedure		DisplayPlayerDeselectedTimeAccelMessage
//Author		Andy McMaster
//Date			Mon 15 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::DisplayPlayerDeselectedTimeAccelMessage(ULong slot)
{
	ItemPtr i=(ItemPtr)Persons2::PlayerSeenAC;

	DisplayPlayerMessage(i,i,ST_PLAYERDESELACCEL);
}


//------------------------------------------------------------------------------
//Procedure		InitSyncData
//Author		Andy McMaster
//Date			Thu 18 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::InitSyncData(bool startup)
{
	SyncData.resettime=true;
	SyncData.sentinfo=false;
	SyncData.wsmneeded=0;
	SyncData.wsmgot=0;
	SyncData.stage=1;
	SyncData.wsrneeded=0;
	if (startup)
		SyncData.wsrgot=0;
	RestartWorldSync=false;

	SyncData.itemstage=0;
	SyncData.mobstage=0;
	SyncData.acstage=0;
	SyncData.deadstage=0;
}

//------------------------------------------------------------------------------
//Procedure		CalcVels
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
void	DPlay::CalcVels(UByte slot,UByte pframe)
{
	if (_DPlay.Implemented)
	{
// use comms historical buffer!

		UByte hframe;
		SWord num;
		AirStrucPtr ac=(AirStrucPtr)Persons2::ConvertPtrUID((UniqueID)AllocPacket[slot]);

		if (ac==Persons2::PlayerSeenAC)
			ac=Persons2::PlayerGhostAC;

// go back 2, this is last frame and previous frame. If this is the begining of the buffer then
// the previous frame will be the starting position		

		HistPosBuffer[slot].SetTempNext();
		HistPosBuffer[slot].TempPrev();
		HistPosBuffer[slot].TempPrev();

// get past any aiaipos packets

		while (!(HistPosBuffer[slot].GetTemp()->deltas.IDCode<PIDC_AIACPOSITION1))
		{
			HistPosBuffer[slot].TempPrev();
		}

// get number of frames between current frame and last frame in historical buffer

		hframe=HistPosBuffer[slot].GetTemp()->Frame;

		if (pframe>hframe)
		{
			num=pframe-hframe;
		}
		else
		{
			num=(pframe-hframe)+256;
		}

// multiply number of frames difference by rate divider to get number of
// move frame between positions

//		num*=RateDivider; // number of move frames

// if the number of frames is greater than the number required for accuracy
// i.e. 4, then work out vels, if not then track back if possible a number 
// of frames (should only need to go back 1 in theory) to get 4 move frames
// difference. If we cant track back (because its the start of the buffer) then
// tough, have to use what we've got

		UByte tvel=velcounter;

		while (num<POSBSIZE-1)
		{
			if (--tvel!=0)
			{
				HistPosBuffer[slot].TempPrev();
				if (!(HistPosBuffer[slot].GetTemp()->deltas.IDCode<PIDC_AIACPOSITION1))
				{
// aiacpos packet at start of buffer? Use what weve got already

					HistPosBuffer[slot].UpdateTemp();
					break;
				}
				num++;
			}
			else
				break;
		}

// dodgy catch all 

		if (!num)
			num=1;

		num*=RateDivider;

// calculate vels

		COORDS3D c=HistPosBuffer[slot].GetTemp()->Pos;

		HistPosBuffer[slot].SetTempNext();
		HistPosBuffer[slot].TempPrev();

		COORDS3D thisac=HistPosBuffer[slot].GetTemp()->Pos;

		ac->velx=10*(thisac.X-c.X)/(num*2);	//CSB 30/03/99	
		ac->vely=10*(thisac.Y-c.Y)/(num*2);	//CSB 30/03/99	
		ac->velz=10*(thisac.Z-c.Z)/(num*2);	//CSB 30/03/99

		SWord	sin_ang,cos_ang;								//AMM 30Jun99
		Math_Lib.high_sin_cos(ac->pitch,sin_ang,cos_ang);		//AMM 30Jun99
		ac->velhori=(ac->vel*cos_ang)>>ANGLES_SHIFT;			//AMM 30Jun99
	}
	else
	{
// use replay buffer!

		Persons2::PlayerGhostAC->velx=10*((*_Replay.PosBuffer.GetNext()).X-(*_Replay.PosBuffer.GetCurr()).X)/((POSBSIZE-1)*2);	//CSB 30/03/99	
		Persons2::PlayerGhostAC->vely=10*((*_Replay.PosBuffer.GetNext()).Y-(*_Replay.PosBuffer.GetCurr()).Y)/((POSBSIZE-1)*2);	//CSB 30/03/99	
		Persons2::PlayerGhostAC->velz=10*((*_Replay.PosBuffer.GetNext()).Z-(*_Replay.PosBuffer.GetCurr()).Z)/((POSBSIZE-1)*2);	//CSB 30/03/99	

		SLong wx, wy, wz;												//CSB 11/06/99	
		MMC.Sky.GetWind(Persons2::PlayerGhostAC->World.Y, wx, wy, wz);	//CSB 11/06/99	
		Persons2::PlayerGhostAC->velx -= wx;							//CSB 11/06/99	
		Persons2::PlayerGhostAC->vely -= wy;							//CSB 11/06/99	
		Persons2::PlayerGhostAC->velz -= wz;							//CSB 11/06/99	

		SWord	sin_ang,cos_ang;								//AMM 30Jun99
		Math_Lib.high_sin_cos(Persons2::PlayerGhostAC->pitch,sin_ang,cos_ang);//AMM 30Jun99
		Persons2::PlayerGhostAC->velhori=(Persons2::PlayerGhostAC->vel*cos_ang)>>ANGLES_SHIFT;//AMM 30Jun99

		_Replay.PosBuffer.UpdateCurr();
		_Replay.PosBuffer.AddEntryAndUpdateNext();
	}
}

//------------------------------------------------------------------------------
//Procedure		NewPlayerMessage
//Author		Andy McMaster
//Date			Mon 22 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewPlayerMessage(UByte message)
{
	NewSpecial(PIDC_PLAYERMESSAGE,message,(UByte)MsgRecipientAll,0);
}

//------------------------------------------------------------------------------
//Procedure		ProcessPlayerMessage
//Author		Andy McMaster
//Date			Mon 22 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessPlayerMessage(LPBASICPACKET p, ULong slot)
{
	ItemPtr i=(ItemPtr)Persons2::ConvertPtrUID((UniqueID)AllocPacket[slot]);

// message to everybody or I am on this guys team

	if (p->byte2 || H2H_Player[mySlot].MyTeam==H2H_Player[slot].MyTeam)
		DisplayPlayerMessage(i,NULL,ST_PLAYERMESSAGE,p->byte1);
}

//------------------------------------------------------------------------------
//Procedure		DisplayPlayerMessage
//Author		Andy McMaster
//Date			Mon 22 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::DisplayPlayerMessage(ItemPtr victim, ItemPtr hitterItem, UByte script_type,UByte messnum)//DAW 22Jun99
{
	if (H2H_Player[mySlot].status!=CPS_3D) // no messages unless in 3d
		return;

	if (hitterItem && hitterItem->MigStatus.LaunchFlags.size!=AIRSTRUCSIZE)
		return;

	MsgType msg=(MsgType)MSG_PLAYERKILLED;

	AirStrucPtr hitter=*hitterItem;

	switch (script_type)
	{
	case ST_PLAYERKILLEDCOMMS:
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERKILLEDCOMMS,msg,hitter,victim,Persons2::PlayerSeenAC));
		break;

	case ST_PLAYERKILLEDPLAYER:
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERKILLEDPLAYER,msg,hitter,victim,Persons2::PlayerSeenAC));
		break;

	case ST_PLAYERKILLEDFRIENDLY:
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERKILLEDFRIENDLY,msg,hitter,victim,Persons2::PlayerSeenAC));
		break;

	case ST_PLAYERKILLEDENEMY:
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERKILLEDPLAYERENEMY,msg,hitter,victim,Persons2::PlayerSeenAC));
		break;

	case ST_PLAYERLEFTGAME:
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERLEFTGAME,msg,hitter,victim,Persons2::PlayerSeenAC));
		break;

	case ST_PLAYERJOINEDGAME:
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERJOINEDGAME,msg,hitter,victim,Persons2::PlayerSeenAC));
		break;

	case ST_PLAYERJOINING:
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERJOINING,msg,hitter,victim,Persons2::PlayerSeenAC));
		break;

	case ST_PLAYERSELACCEL:
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERSELACCEL,(MsgType)(msg+MSG_HIPRIORITY),hitter,victim,Persons2::PlayerSeenAC));
		break;

	case ST_PLAYERDESELACCEL:
		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_PLAYERDESELACCEL,msg,hitter,victim,Persons2::PlayerSeenAC));
		break;

	case ST_PLAYERMESSAGE:
 		_Radio.TriggerMsg(MESSAGE_STRUC(SCRIPT_CALLER_USER_MESSAGE,msg,(MobileItemPtr)victim,NULL,Persons2::PlayerSeenAC,TARGET_INDEXES(messnum)));
		break;
	}
}

//------------------------------------------------------------------------------
//Procedure		UpdateHistBuffer
//Author		Andy McMaster
//Date			Tue 23 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::UpdateHistBuffer(LPACPACKET p,ULong slot)
{
	UByte shift;
	SLong val;

	AddPacketToHistBuffer(p,slot);

#ifdef HISTTEST
	FILE* pp=fopen("histtest.txt","at");
#endif
	while (HistPosBuffer[slot].GetNext()->OK)
	{

#ifdef HISTTEST
		AirStrucPtr gac=Persons2::PlayerGhostAC;
		AirStrucPtr sac=Persons2::PlayerSeenAC;
		fprintf(pp,"gX=%d gY=%d gZ=%d gH=%d gP=%d gR=%d gV=%d\n",gac->World.X,gac->World.Y,gac->World.Z,gac->hdg.a,gac->pitch.a,gac->roll.a,gac->vel);
		fprintf(pp,"sX=%d sY=%d sZ=%d sH=%d sP=%d sR=%d sV=%d\n",sac->World.X,sac->World.Y,sac->World.Z,sac->hdg.a,sac->pitch.a,sac->roll.a,sac->vel);
#endif

		HistPosBuffer[slot].SetTempNext();
		HistPosBuffer[slot].TempPrev();

		HistPosBuffer[slot].GetNext()->Pos.X=HistPosBuffer[slot].GetTemp()->Pos.X;
		HistPosBuffer[slot].GetNext()->Pos.Y=HistPosBuffer[slot].GetTemp()->Pos.Y;
		HistPosBuffer[slot].GetNext()->Pos.Z=HistPosBuffer[slot].GetTemp()->Pos.Z;
		HistPosBuffer[slot].GetNext()->Heading.a=HistPosBuffer[slot].GetTemp()->Heading.a;
		HistPosBuffer[slot].GetNext()->Pitch.a=HistPosBuffer[slot].GetTemp()->Pitch.a;
		HistPosBuffer[slot].GetNext()->Roll.a=HistPosBuffer[slot].GetTemp()->Roll.a;
		HistPosBuffer[slot].GetNext()->Velocity=HistPosBuffer[slot].GetTemp()->Velocity;

#ifdef HISTTEST
		fprintf(pp,"1: hX=%d hY=%d hZ=%d hH=%d hP=%d hR=%d hV=%d\n",HistPosBuffer[slot].GetNext()->Pos.X,
			HistPosBuffer[slot].GetNext()->Pos.Y,HistPosBuffer[slot].GetNext()->Pos.Z,
			HistPosBuffer[slot].GetNext()->Heading.a,HistPosBuffer[slot].GetNext()->Pitch.a,HistPosBuffer[slot].GetNext()->Roll.a,
			HistPosBuffer[slot].GetNext()->Velocity);
#endif

// pidc_initpack is from startup sequence, no prediction occured. Can still apply the packets
// as deltas because all deltas wil be zero

		if (HistPosBuffer[slot].GetNext()->deltas.IDCode!=PIDC_INITPACK)
			PredictGoodMovement(slot);

#ifdef HISTTEST

		fprintf(pp,"2: hX=%d hY=%d hZ=%d hH=%d hP=%d hR=%d hV=%d\n",HistPosBuffer[slot].GetNext()->Pos.X,
			HistPosBuffer[slot].GetNext()->Pos.Y,HistPosBuffer[slot].GetNext()->Pos.Z,
			HistPosBuffer[slot].GetNext()->Heading.a,HistPosBuffer[slot].GetNext()->Pitch.a,HistPosBuffer[slot].GetNext()->Roll.a,
			HistPosBuffer[slot].GetNext()->Velocity);
#endif

// dont add deltas for aiacpos packets!

		if (HistPosBuffer[slot].GetNext()->deltas.IDCode<PIDC_AIACPOSITION1)
		{
			shift=HistPosBuffer[slot].GetNext()->deltas.Shift;

			val=GetVal(HistPosBuffer[slot].GetNext()->deltas.X,shift);
			HistPosBuffer[slot].GetNext()->Pos.X+=val;

			val=GetVal(HistPosBuffer[slot].GetNext()->deltas.Y,shift);
			HistPosBuffer[slot].GetNext()->Pos.Y+=val;

			val=GetVal(HistPosBuffer[slot].GetNext()->deltas.Z,shift);
			HistPosBuffer[slot].GetNext()->Pos.Z+=val;

			val=GetVal(HistPosBuffer[slot].GetNext()->deltas.Heading,shift);
			HistPosBuffer[slot].GetNext()->Heading.a+=val;

			val=GetVal(HistPosBuffer[slot].GetNext()->deltas.Pitch,shift);
			HistPosBuffer[slot].GetNext()->Pitch.a+=val;

			val=GetVal(HistPosBuffer[slot].GetNext()->deltas.Roll,shift);
			HistPosBuffer[slot].GetNext()->Roll.a+=val;

			if(HistPosBuffer[slot].GetNext()->deltas.Velocity&8)
			{
				val=-((HistPosBuffer[slot].GetNext()->deltas.Velocity&7)<<shift);
			}
			else
			{
				val=+((HistPosBuffer[slot].GetNext()->deltas.Velocity&7)<<shift);
			}
			HistPosBuffer[slot].GetNext()->Velocity+=val;
		}

#ifdef HISTTEST
		fprintf(pp,"2: hX=%d hY=%d hZ=%d hH=%d hP=%d hR=%d hV=%d\n",HistPosBuffer[slot].GetNext()->Pos.X,
			HistPosBuffer[slot].GetNext()->Pos.Y,HistPosBuffer[slot].GetNext()->Pos.Z,
			HistPosBuffer[slot].GetNext()->Heading.a,HistPosBuffer[slot].GetNext()->Pitch.a,HistPosBuffer[slot].GetNext()->Roll.a,
			HistPosBuffer[slot].GetNext()->Velocity);
		fclose(pp);
#endif
		HistPosBuffer[slot].GetNext()->OK=false;
		HistPosBuffer[slot].UpdateNext();
	}
}

//------------------------------------------------------------------------------
//Procedure		AddPacketToHistBuffer
//Author		Andy McMaster
//Date			Tue 23 Mar 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::AddPacketToHistBuffer(LPACPACKET p,ULong slot)
{
	SWord diff,sub;
	SWord	thispacketcount;
	SWord	targetcount;

	HistPosBuffer[slot].SetTempNext();
	HistPosBuffer[slot].TempPrev();

	targetcount=HistPosBuffer[slot].GetTemp()->Frame;
	thispacketcount=p->Count;

	sub=thispacketcount-targetcount;

	if (((thispacketcount>=targetcount && sub<128)
	|| (thispacketcount<targetcount && abs(sub)>128)))
	{
		if (thispacketcount>=targetcount)
			diff=sub;
		else
			diff=(sub)+256;

		while (diff--)
			HistPosBuffer[slot].UpdateTemp();

		memcpy(&HistPosBuffer[slot].GetTemp()->deltas,p,sizeof(ACPACKET));
		HistPosBuffer[slot].GetTemp()->OK=true;
		HistPosBuffer[slot].GetTemp()->Frame=p->Count;
	}
}

//------------------------------------------------------------------------------
//Procedure		SendEndGameScore
//Author		Andy McMaster
//Date			Thu 1 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendEndGameScore()
{
	SendSaveGame(true,false);
}

//------------------------------------------------------------------------------
//Procedure		ReceiveEndGameScore
//Author		Andy McMaster
//Date			Thu 1 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ReceiveEndGameScore()
{
	ReceiveStartupSaveGame(false);
}

//------------------------------------------------------------------------------
//Procedure		SendExpandMigsMessage
//Author		Andy McMaster
//Date			Mon 12 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendExpandMigsMessage(UniqueID mig)
{
	UWord fake=(UWord)mig;

	UByte b1,b2;

	b1=fake&0xff;
	b2=fake>>8;

	NewSpecial(PIDC_EXPANDMIGS,b1,b2,0);
}

//------------------------------------------------------------------------------
//Procedure		ProcessExpandMigsMessage
//Author		Andy McMaster
//Date			Mon 12 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessExpandMigsMessage(LPBASICPACKET p)
{
	UniqueID ID;
	AirStrucPtr mig;

	ID=(UniqueID)(((p->byte2)<<8)+p->byte1);

	mig=*Persons2::ConvertPtrUID(ID);

	if (mig)
	{
		mig->MigStatus.LaunchFlags.Drawn=TRUE;
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessEjectMessage
//Author		Andy McMaster
//Date			Wed 21 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessEjectMessage(AirStrucPtr AC)
{
	Trans_Obj.LaunchCanopyEject(AC,*mobileitem::currworld);
}

//------------------------------------------------------------------------------
//Procedure		ProcessGuestLeavingMessage
//Author		Andy McMaster
//Date			Fri 23 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessGuestLeavingMessage(ULong id)
{
	ProcessPlayerGone(id,TRUE);	 

// if a player leaves whilst a sync is in process need to start again without him
// force a resync anyway because packet sent saying player is out could be out of sync

	if (Host && H2H_Player[mySlot].status==CPS_3D)
	{
		if (GameType>TEAMPLAY)
		{
			Initiator=TRUE;
			BeginSyncPhase();
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		NewShapePacket
//Author		Robert Slater
//Date			Wed 21 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewShapePacket(UWord	id, UWord	shape)
{
	UByte b1,b2,b3;

	b1=id&0xff;
	b2=(id>>8)&0x3f;
	b2+=(shape>>2)&0xc0;
	b3=shape&0xff;

#ifndef NDEBUG
	UWord uid;
	UWord oldshape;
	UWord newshape;

	ItemPtr i=(ItemPtr)Persons2::ConvertPtrUID((UniqueID)id);
	oldshape=i->shape;

	uid=b1;
	uid+=(b2&0x3f)<<8;

	newshape=b3+((b2&0xc0)<<2);
	SHAPE.ReplayAnimData_NewShape(uid,newshape);
	SHAPE.ReplayAnimData_NewShape(uid,oldshape);
#endif

	NewSpecial(PIDC_SHAPECHANGE,b1,b2,b3);
}

//------------------------------------------------------------------------------
//Procedure		ProcessNewShapePacket
//Author		Robert Slater
//Date			Wed 21 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessNewShapePacket(LPBASICPACKET	p)
{
	UWord uid,shape;

	uid=p->byte1;
	uid+=(p->byte2&0x3f)<<8;

	shape=p->byte3+((p->byte2&0xc0)<<2);

	SHAPE.ReplayAnimData_NewShape(uid,shape);
}

//------------------------------------------------------------------------------
//Procedure		ProcessResyncPacket
//Author		Andy McMaster
//Date			Thu 29 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessResyncPacket(LPACSTATEPACKET packet, ULong n)
{
	AirStrucPtr AC;

	resyncsgot|=(1<<n);

	AC=(AirStruc*)Persons2::ConvertPtrUID((UniqueID)AllocPacket[n]);

	if (AC)
	{
		if (!PlayerSync || !Host)
		{
// get state data

			WorldStuff &world = mobileitem::GetWorld();
			UWord	oldsector=	world.GetSector(AC);

			AC->World.X=packet->Pos.X;
			AC->World.Y=packet->Pos.Y;
			AC->World.Z=packet->Pos.Z;

			AC->hdg.a=packet->Heading.a;
			AC->pitch.a=packet->Pitch.a;
			AC->roll.a=packet->Roll.a;
			AC->vel=packet->Velocity;	
			AC->fly.cpitch=AC->pitch;

			UWord	newsector=world.GetSector(AC);
			if (newsector!=oldsector)
			{
				world.RemoveFromSector(AC,oldsector);
				world.AddToWorld(AC);
			}

			if (packet->alive)
			{
				AC->MigStatus.LaunchFlags.deadtime=0;
				AC->MigStatus.LaunchFlags.deaded=FALSE;
			}
			else
			{
				deadplayers|=1<<n;
				AC->MigStatus.LaunchFlags.deaded=TRUE;
				AC->MigStatus.LaunchFlags.deadtime=1;
			}

			if (packet->resurrect)
			{
				RepairAircraft(AC);
				AC->MigStatus.LaunchFlags.deaded=FALSE;
			}

			if (packet->joining)
				joiningplayerslot=n;

	// extract anim data
			UByteP	datapack;

			datapack=(UByte*)packet;
			datapack+=sizeof(ACSTATEPACKET);

	// if packet is from host then set timeofday

			if (n==0)
			{
				_Replay.inst->timeofday=*(int*)datapack;
			}
			datapack+=sizeof(int);

			ProcessAnimPack(datapack,(ItemPtr)AC);

			MinAnimData* mad = (MinAnimData* )AC->Anim;			//DAW 22Jun99

			if (packet->alive)
				mad->itemstate = ALIVE;

			mad->IsInvisible=0;
		}

		(H2H_Player+n)->kills=packet->MyKills;
		(H2H_Player+n)->deaths=packet->MyDeaths;
	}
}

//------------------------------------------------------------------------------
//Procedure		SendRestartWorldSyncMessage
//Author		Andy McMaster
//Date			Thu 29 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendRestartWorldSyncMessage()
{
	COMMONDATA p;

	p.PacketID=PID_RESTARTWORLDSYNC;

	SendMessageToPlayers((char*)&p,sizeof(COMMONDATA));
}

//------------------------------------------------------------------------------
//Procedure		SendDeadStream
//Author		Andy McMaster
//Date			Thu 29 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendDeadStream()
{
// traverse deadstream finding number of blocks.
// Send number of blocks with olddaytime, 
// followed by block themseleves!

	DeadStream::DeadBlock* block=NULL;
	WorldStuff &world = mobileitem::GetWorld();
	int	temptime;
	ULong num=0;

	DeadStream tempdead;

	tempdead.firstblock=NULL;
	tempdead.SetWorldDead(&world);

	block=tempdead.firstblock;
	temptime=tempdead.olddaytime;

	while (block)
	{
		num++;
		block=block->nextblock;
	}

	COMMONDATA p;

	p.PacketID=PID_NUMDEADBLOCKS;
	p.PlayerID=num;
	p.Slot=tempdead.olddaytime;

	SendGMessageToPlayers((char*)&p,sizeof(COMMONDATA));

	block=tempdead.firstblock;

	while (block)
	{
		UByteP pack,temp;
		ULong packsize;

		packsize=sizeof(ULong) // packetid
				+sizeof(UWord) // SGT
				+sizeof(UWord) // dataused
				+(sizeof(UByte)*block->dataused); // data

		pack=new UByte[packsize];
		temp=pack;

		*(ULong*)temp=PID_DEADBLOCKDATA;
		temp+=sizeof(ULong);

		*(UWord*)temp=block->SGT;
		temp+=sizeof(UWord);

		*(UWord*)temp=block->dataused;
		temp+=sizeof(UWord);

		memcpy(temp,block->data,block->dataused);

		SendGMessageToPlayers((char*)pack,packsize);

		delete [] pack;

		block=block->nextblock;
	}

}

//------------------------------------------------------------------------------
//Procedure		GetDeadStream
//Author		Andy McMaster
//Date			Thu 29 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	DPlay::GetDeadStream()
{
	static ULong need;
	static ULong got;

	switch (SyncData.deadstage)
	{
	case 0:
		int dtime;
		if (GetNumDeadBlocks(need,dtime))
		{
			Dead_Stream.ClrWorldDead();
			Dead_Stream.olddaytime=dtime;
			SyncData.deadstage++;
			got=0;
		}
		break;
	case 1:
		if (need!=0)
		{
			if (!GetAllDeadBlocks(need,got))
			{
				break;
			}
		}
		WorldStuff &world = mobileitem::GetWorld();
		Dead_Stream.GetWorldDead(&world);
		SyncData.deadstage=0;
		return true;
	}

	return false;
}

//------------------------------------------------------------------------------
//Procedure		GetNumDeadBlocks
//Author		Andy McMaster
//Date			Thu 29 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	DPlay::GetNumDeadBlocks(ULong& num,int& dtime)
{
	char* buffer;
	ULong	buflen,from;
	PCOMMONDATA c;

	ULong to=myDPlayID;

	while (ReceiveNextMessage(buffer,buflen,from,to,DPRECEIVE_TOPLAYER))
	{
		if (from==0)
		{
			ProcessSystemMessage(buffer,CPS_RESYNCING);
		}
		else if (from!=aggID)
		{
			c=(PCOMMONDATA)buffer;

			if (c->PacketID==PID_NUMDEADBLOCKS)
			{
				num=c->PlayerID;
				dtime=c->Slot;

				return true;
			}
			else
			{
				ProcessPlayerMessage(buffer,buflen,from,CPS_RESYNCING);
			}
		}
	}
	return true;
}

//------------------------------------------------------------------------------
//Procedure		GetAllDeadBlocks
//Author		Andy McMaster
//Date			Thu 29 Apr 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	DPlay::GetAllDeadBlocks(ULong need,ULong& got)
{
	char* buffer;
	ULong buflen,from;
	UByteP pack;

	ULong to=myDPlayID;

	while (ReceiveNextMessage(buffer,buflen,from,to,DPRECEIVE_TOPLAYER))
	{
		if (from==0)
		{
			ProcessSystemMessage(buffer,CPS_RESYNCING);
		}
		else if (from!=aggID)
		{
			if (*(ULong*)buffer==PID_DEADBLOCKDATA)
			{
				pack=(UByteP)buffer;
				pack+=sizeof(ULong);

				DeadStream::DeadBlock* block=new DeadStream::DeadBlock;

				UWord fake=(UWord)*(UWord*)pack;
				block->SGT=(UniqueID)fake;
				pack+=sizeof(UWord);

				block->dataused=(UWord)*(UWord*)pack;
				pack+=sizeof(UWord);

				memcpy(&block->data,pack,block->dataused);

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

				got++;

				if (got==need)
					return TRUE;
			}
			else
			{
				ProcessPlayerMessage(buffer,buflen,from,CPS_RESYNCING);
			}
		}
	}
	if (got==need)
		return true;

	return false;
}

//------------------------------------------------------------------------------
//Procedure		SendPilotsStatus
//Author		Andy McMaster
//Date			Thu 6 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendPilotsStatus(bool single,int pilot)
{
	if (single)
	{
// when I die send pilot status update

		COMMONDATA p;

		p.PacketID=PID_SINGLEPILOTSTATUS;
		p.PlayerID=pilot;
		p.Slot=mySlot;

		SendMessageToPlayers((char*)&p,sizeof(COMMONDATA));
	}
	else
	{
// when a player joins game send all pilots status

		ULong psize=sizeof(ULong)+(sizeof(UByte)*PILOTS_PER_CAMPAIGN);
		UByteP	pack=new UByte[psize];
		UByteP	temp=pack;
		ULong n;

		*(ULong*)temp=PID_ALLPILOTSSTATUS;
		temp+=sizeof(ULong);

		for (n=0;n<PILOTS_PER_CAMPAIGN;n++)
		{
			*(UByte*)temp=(UByte)MMC.Active_Pilots[n].status;
			temp+=sizeof(UByte);
		}

		SendMessageToAll((char*)pack,psize);

		delete [] pack;
	}
}

//------------------------------------------------------------------------------
//Procedure		GetPilotsStatus
//Author		Andy McMaster
//Date			Thu 6 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	DPlay::GetPilotsStatus()
{
	char* Buffer;
	ULong	BufferLen=0;
	bool	Got=false;
	ULong From;

	ULong now=_Miles.GetSysTime();
	CommsTimeout time(now,CommsTimeoutLength);

	while(!Got)
	{
		now=_Miles.GetSysTime();

		if (time.TimedOut(now))
			return false;
		
		ULong to=0;

		if (ReceiveNextMessage(Buffer,BufferLen,From,to,0))
		{
			if (From==0)
			{
				ProcessSystemMessage(Buffer,CPS_NONE);
			}
			else
			{
				if (*(ULong*)Buffer==PID_ALLPILOTSSTATUS)
				{
					ProcessPilotStatusPacket(false,(UByte*)Buffer);
					Got=true;
				}
			}
		}
	}

	return Got;
}

//------------------------------------------------------------------------------
//Procedure		ProcessPilotStatusPacket
//Author		Andy McMaster
//Date			Thu 6 May 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessPilotStatusPacket(bool single,UByteP p)
{
	ULong n;
	UByteP temp=p;
	PCOMMONDATA c;			

	if (single)
	{
		c=(PCOMMONDATA)p;

		MMC.Active_Pilots[c->PlayerID].status=KIA;
	}
	else
	{
		temp+=sizeof(ULong);

		for (n=0;n<PILOTS_PER_CAMPAIGN;n++)
		{
			BackupPilots[n]=*temp;

			temp+=sizeof(UByte);
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		GetAccelNumber
//Author		Andy McMaster
//Date			Mon 7 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
UByte	DPlay::GetAccelNumber()
{
	ULong n;
	UByte count=0;

	for (n=0;n<MAXPLAYERS;n++)
	{
		if (accelselected&(1<<n))
			count++;
	}

	return count;
}

void	DPlay::SendAccelGearUpMessage()
{
	NewSpecial(PIDC_ACCELGEARUP,0,0,0);
}

void	DPlay::ProcessAccelGearUpMessage()
{
	mobileitem::ResetACGears();							//RJS 01Jun99
	AircraftAnimData*	adptr;
	AirStrucPtr ac=Persons2::PlayerSeenAC;
	SHAPE.NavigationLightsActive(ac,FALSE);
	adptr = (AircraftAnimData*) ac->Anim;
	if (adptr)
	{
		adptr->acleglowerl = 255;
		adptr->acleglowerr = 255;
		adptr->acleglowerf = 255;
		adptr->acleglowerb = 255;
		adptr->animtoggle = 1;
	}
}

//------------------------------------------------------------------------------
//Procedure		SendStartAccelMessage
//Author		Andy McMaster
//Date			Thu 10 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendStartAccelMessage()
{
	NewSpecial(PIDC_STARTACCEL,0,0,0);
}

//------------------------------------------------------------------------------
//Procedure		ProcessStartAccelMessage
//Author		Andy McMaster
//Date			Thu 10 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessStartAccelMessage(ULong slot)
{
	accelselected|=(1<<slot);

	if (accelselected==ActivePlayers)
	{
		if (Host)
		{
// wont be sending packets in accel, stop aggregator

			_Agg.Running=FALSE;
			_Agg.StopResetAggregator();
		}

		PlayerSync=true;

// everybody ready lets accel

		OverLay.AccelSelection(Accelhilight,AccelComms);
	}
	else
	{
		DisplayPlayerSelectedTimeAccelMessage(slot,true);
	}
}

//------------------------------------------------------------------------------
//Procedure		SendQuitAccelMessage
//Author		Andy McMaster
//Date			Thu 10 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendQuitAccelMessage()
{
	COMMONDATA p;
	p.PacketID=PID_QUITACCEL;
	SendGMessageToPlayersTimeout((char*)&p,sizeof(COMMONDATA));
}

//------------------------------------------------------------------------------
//Procedure		ProcessQuitAccelMessage
//Author		Andy McMaster
//Date			Thu 10 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessQuitAccelMessage(ULong slot)
{
	DisplayPlayerDeselectedTimeAccelMessage(slot);

// all players will be brought out of accel
	accelselected=0;
	if (mobileitem::currworld->vp->Accel())
	{
		OverLay.CancelAccel();
	}

	if (Host)
	{
		Initiator=TRUE;
		BeginSyncPhase();
	}
}

//------------------------------------------------------------------------------
//Procedure		NewWPMessage
//Author		Andy McMaster
//Date			Thu 10 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewWPMessage(UniqueID wp)
{
	UByte b1,b2;

	b1=wp&0xff;
	b2=(wp>>8)&0xff;

	NewSpecial(PIDC_NEWWP,b1,b2,0);
}

//------------------------------------------------------------------------------
//Procedure		ProcessNewWPMessage
//Author		Andy McMaster
//Date			Thu 10 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessNewWPMessage(LPBASICPACKET p)
{
	UWord newwp;

	newwp=(p->byte1)+(p->byte2<<8);

	ItemPtr i=(ItemPtr)Persons2::ConvertPtrUID((UniqueID)newwp);

	if (i)
	{
	 	Persons2::PlayerSeenAC->waypoint=(WayPointPtr)i;
		Persons2::PlayerGhostAC->waypoint=(WayPointPtr)i;
	}
}

//------------------------------------------------------------------------------
//Procedure		SendDeselectAccelMessage
//Author		Andy McMaster
//Date			Fri 11 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendDeselectAccelMessage()
{
	if (accelselected!=ActivePlayers)
	{
// shouldnt be in accel
		NewSpecial(PIDC_DESELECTACCEL,0,0,0);
	}
}

//------------------------------------------------------------------------------
//Procedure		ProcessDeselectAccelMessage
//Author		Andy McMaster
//Date			Fri 11 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessDeselectAccelMessage(ULong slot)
{
	if (accelselected!=ActivePlayers)
	{
// if not in accel
		accelselected&=~(1<<slot);
	}
}

//------------------------------------------------------------------------------
//Procedure		SendAAAList
//Author		Andy McMaster
//Date			Wed 16 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::SendAAAList()
{
	ULong size=0;
	UWord array[40]; //32 may not be enough if every element in group is gun...

	UWord	num=0,num2;//,count=0;;
	SWord	skipsize=-1;

	while (skipsize<0)		//returns negative until end of list
	{
		skipsize=Three_Dee.livelist->ArchiveLiveList(array,40,num);
		num-=skipsize;
	}
	num+=skipsize+skipsize;	//total size is positive

	UWordP aaa=new UWord[num];

	skipsize=Three_Dee.livelist->ArchiveLiveList(aaa,num,0);
	num2=skipsize;

	assert(num==num2);

	ULong packsize=sizeof(ULong)	// PID
					+sizeof(UWord)	// num
					+(num*sizeof(UWord));		// data
	
    UByteP pack=new UByte[packsize];
	UByteP temp=pack;

	*(ULong*)temp=PID_AAAPACKET;
	temp+=sizeof(ULong);
	*(UWord*)temp=num;
	temp+=sizeof(UWord);

	while (num2--)
	{
		*(UWord*)temp=aaa[num2],sizeof(UWord);
		temp+=sizeof(UWord);
	}

	SendGMessageToPlayers((char*)pack,packsize);

 	Three_Dee.livelist->RestoreSleepList();
	Three_Dee.livelist->ApplyLiveList(aaa,num);

	delete [] aaa;
	delete [] pack;
}

//------------------------------------------------------------------------------
//Procedure		GetAAAList
//Author		Andy McMaster
//Date			Wed 16 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	DPlay::GetAAAList()
{
	char* buffer;
	ULong	buflen,from;
	ULong to=myDPlayID;

	while (ReceiveNextMessage(buffer,buflen,from,to,DPRECEIVE_TOPLAYER))
	{
		if (from==0)
		{
			ProcessSystemMessage(buffer,CPS_RESYNCING);
		}
		else if (from!=aggID)
		{
			if (*(ULong*)buffer==PID_AAAPACKET)
			{
				ProcessAAAPacket(buffer);
				return true;
			}
			else 
			{
				ProcessPlayerMessage(buffer,buflen,from,CPS_RESYNCING);
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------------
//Procedure		ProcessAAAPacket
//Author		Andy McMaster
//Date			Thu 17 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessAAAPacket(char* packet)
{
	packet+=sizeof(ULong); // get past PID

	UWord num=*(UWord*)packet; // get number of elements
	packet+=sizeof(UWord); 

	UWordP aaa=new UWord[num];

	UWord num2=num;

	while (num2--)
	{
		aaa[num2]=*(UWord*)packet;
		packet+=sizeof(UWord);
	}

	Three_Dee.livelist->RestoreSleepList();
	Three_Dee.livelist->ApplyLiveList(aaa,num);

	delete [] aaa;
}

//------------------------------------------------------------------------------
//Procedure		GetRandomList
//Author		Andy McMaster
//Date			Fri 18 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
bool	DPlay::GetRandomList()
{
	char* buffer;
	ULong	buflen,from;
	ULong to=myDPlayID;

	while (ReceiveNextMessage(buffer,buflen,from,to,DPRECEIVE_TOPLAYER))
	{
		if (from==0)
		{
			ProcessSystemMessage(buffer,CPS_RESYNCING);
		}
		else if (from!=aggID)
		{
			if (*(ULong*)buffer==PID_RANDOMLIST)
			{
				ProcessRandomList(buffer);
				return true;
			}
			else 
			{
				ProcessPlayerMessage(buffer,buflen,from,CPS_RESYNCING);
			}
		}
	}
	return false;
}

//------------------------------------------------------------------------------
//Procedure		NewPlayerMoving
//Author		Andy McMaster
//Date			Tue 22 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewPlayerMoving()
{
	NewSpecial(PIDC_PLAYERMOVING,0,0,0);
}

//------------------------------------------------------------------------------
//Procedure		ProcessPlayerMoving
//Author		Andy McMaster
//Date			Tue 22 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessPlayerMoving()
{
	AirStruc::playermoving=true;
}

//------------------------------------------------------------------------------
//Procedure		NewPlayerNotMovingHelAC
//Author		Andy McMaster
//Date			Tue 22 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::NewPlayerNotMovingHeldAC(UniqueID ac,UByte type)	//DAW 28Jun99
{
	UByte b1,b2,b3;												//DAW 28Jun99
	UWord id=ac;

	b1=id&0xff;
	b2=(id>>8)&0xff;
	b3=type;													//DAW 28Jun99

	NewSpecial(PIDC_PLAYERNOTMOVINGHELDAC,b1,b2,0);
}

//------------------------------------------------------------------------------
//Procedure		ProcessNewPlayerNotMovingHeldAC
//Author		Andy McMaster
//Date			Tue 22 Jun 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ProcessNewPlayerNotMovingHeldAC(LPBASICPACKET p)
{
	UWord id=(p->byte1)+(p->byte2<<8);

	AirStrucPtr ac=(AirStrucPtr)Persons2::ConvertPtrUID((UniqueID)id);

	if (ac)
	{
		AirStruc::playernotmovingheldac=ac;

		switch (p->byte3)
		{
		case AUTOLANDING1:
			ac->ai.PilotSkills.ManStep = 2;	
			ac->fly.pModel->SetGearAnimPos(ac, ANIM_GEAR_AUTO);

		case AUTOLANDING2:
			ac->SetManoeuvreTime(5*SECS100);
			break;
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		ResetCommsSurfaces
//Author		Andy McMaster
//Date			Mon 28 Jun 1999
//
//Description	Reset gear,brakes and flaps after comms resurrect
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ResetCommsSurfaces()
{
	AirStrucPtr ac=Persons2::PlayerSeenAC;
	AircraftAnimData*	adptr;

	if (ac->fly.pModel)
		ac->fly.pModel->ResetGearPos(false);

	SHAPE.NavigationLightsActive(ac,FALSE);

	adptr = (AircraftAnimData*) ac->Anim;
	if (adptr)
	{
		adptr->acleglowerl = 255;
		adptr->acleglowerr = 255;
		adptr->acleglowerf = 255;
		adptr->acleglowerb = 255;
		adptr->animtoggle = 1;
	}
}

//------------------------------------------------------------------------------
//Procedure		ExpandAggPacket
//Author		Andy McMaster
//Date			Wed 22 Sep 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void DPlay::ExpandAggPacket(LPAGGSENDPACKET dest, LPAGGSENDPACKET src)
{
	ULong n;
	UByteP ptr;

	dest->Count=src->Count;
	dest->PlayersInPacket=src->PlayersInPacket;

	ptr=(UByte*)(&src->player[0]);

	for (n=0;n<MAXPLAYERS;n++)
	{
		if (src->PlayersInPacket&1<<n)
		{
			memcpy(&dest->player[n], ptr, sizeof(ACPACKET));
			ptr+=sizeof(ACPACKET);
		}
		else
		{
			dest->player[n].IDCode=PIDC_PACKETERROR;
		}
	}
}

//------------------------------------------------------------------------------
//Procedure		ApplyPartialDeltas
//Author		Andy McMaster
//Date			Tue 30 Nov 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void	DPlay::ApplyPartialDeltas()
{
	ULong n;

	for (n=0;n<MAXPLAYERS;n++)
	{
		if (ActivePlayers & 1<<n)
		{
			if (n==mySlot)
			{
				Persons2::PlayerGhostAC->World.X+=DeltasToApply[mySlot][DeltaIndex].lastpos.X;
				Persons2::PlayerGhostAC->World.Y+=DeltasToApply[mySlot][DeltaIndex].lastpos.Y;
				Persons2::PlayerGhostAC->World.Z+=DeltasToApply[mySlot][DeltaIndex].lastpos.Z;

				Persons2::PlayerGhostAC->hdg.a+=DeltasToApply[mySlot][DeltaIndex].Heading.a;
				Persons2::PlayerGhostAC->fly.dhdg+=DeltasToApply[mySlot][DeltaIndex].Heading.a;

				Persons2::PlayerGhostAC->pitch.a+=DeltasToApply[mySlot][DeltaIndex].Pitch.a;
				Persons2::PlayerGhostAC->fly.cpitch=Persons2::PlayerGhostAC->pitch;

				Persons2::PlayerGhostAC->roll.a+=DeltasToApply[mySlot][DeltaIndex].Roll.a;
				Persons2::PlayerGhostAC->vel+=DeltasToApply[mySlot][DeltaIndex].Velocity;
			}
			else
			{
				AirStrucPtr AC=*Persons2::ConvertPtrUID((UniqueID)AllocPacket[n]);

				if (AC)
				{
					UWord	oldsector= MobileItem::currworld->GetSector(*AC);

					AC->World.X+=DeltasToApply[n][DeltaIndex].lastpos.X;
					AC->World.Y+=DeltasToApply[n][DeltaIndex].lastpos.Y;
					AC->World.Z+=DeltasToApply[n][DeltaIndex].lastpos.Z;

					if (oldsector!=MobileItem::currworld->GetSector(*AC))
					{
						MobileItem::currworld->RemoveFromSector(*AC,oldsector);
						MobileItem::currworld->AddToWorld(*AC);
					}

					AC->hdg.a+=DeltasToApply[n][DeltaIndex].Heading.a;
					AC->fly.dhdg+=DeltasToApply[mySlot][DeltaIndex].Heading.a;

					AC->pitch.a+=DeltasToApply[n][DeltaIndex].Pitch.a;
					AC->fly.cpitch=AC->pitch;

					AC->roll.a+=DeltasToApply[n][DeltaIndex].Roll.a;
					AC->vel+=DeltasToApply[n][DeltaIndex].Velocity;
				}
			}

			ZeroMemory(&DeltasToApply[n][DeltaIndex],sizeof(DELTARECORD));
		}
	}
	DeltaIndex++;

	if (DeltaIndex>=RateDivider)
		DeltaIndex=0;
}

//------------------------------------------------------------------------------
//Procedure		AddDeltasToApply
//Author		Andy McMaster
//Date			Tue 30 Nov 1999
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void		DPlay::AddDeltasToApply(LPACPACKET packet, ULong slot)
{
	UByte	shiftfactor=packet->Shift;
	SLong	val,valfrac;
	SLong 	frac;
	UByte	TempIndex,n;
	
	val=GetVal(packet->X,shiftfactor);

// yuk, messy

	{
		TempIndex=DeltaIndex;
		valfrac=val/RateDivider;
		frac=(val-(valfrac*RateDivider));

		DeltasToApply[slot][TempIndex].lastpos.X+=frac;
		for (n=0;n<RateDivider;n++)
		{
			DeltasToApply[slot][TempIndex].lastpos.X+=valfrac;

			TempIndex++;

			if (TempIndex>=RateDivider)
				TempIndex=0;
		}
	}

	val=GetVal(packet->Y,shiftfactor);

	{
		TempIndex=DeltaIndex;
		valfrac=val/RateDivider;
		frac=(val-(valfrac*RateDivider));

		DeltasToApply[slot][TempIndex].lastpos.Y+=frac;
		for (n=0;n<RateDivider;n++)
		{
			DeltasToApply[slot][TempIndex].lastpos.Y+=valfrac;

			TempIndex++;

			if (TempIndex>=RateDivider)
				TempIndex=0;
		}
	}

	val=GetVal(packet->Z,shiftfactor);

	{
		TempIndex=DeltaIndex;
		valfrac=val/RateDivider;
		frac=(val-(valfrac*RateDivider));

		DeltasToApply[slot][TempIndex].lastpos.Z+=frac;
		for (n=0;n<RateDivider;n++)
		{
			DeltasToApply[slot][TempIndex].lastpos.Z+=valfrac;

			TempIndex++;

			if (TempIndex>=RateDivider)
				TempIndex=0;
		}
	}

	val=GetVal(packet->Heading,shiftfactor);

	{
		TempIndex=DeltaIndex;
		valfrac=val/RateDivider;
		frac=(val-(valfrac*RateDivider));

		DeltasToApply[slot][TempIndex].Heading.a+=frac;
		for (n=0;n<RateDivider;n++)
		{
			DeltasToApply[slot][TempIndex].Heading.a+=valfrac;

			TempIndex++;

			if (TempIndex>=RateDivider)
				TempIndex=0;
		}
	}

	val=GetVal(packet->Pitch,shiftfactor);

	{
		TempIndex=DeltaIndex;
		valfrac=val/RateDivider;
		frac=(val-(valfrac*RateDivider));

		DeltasToApply[slot][TempIndex].Pitch.a+=frac;
		for (n=0;n<RateDivider;n++)
		{
			DeltasToApply[slot][TempIndex].Pitch.a+=valfrac;

			TempIndex++;

			if (TempIndex>=RateDivider)
				TempIndex=0;
		}
	}

	val=GetVal(packet->Roll,shiftfactor);

	{
		TempIndex=DeltaIndex;
		valfrac=val/RateDivider;
		frac=(val-(valfrac*RateDivider));

		DeltasToApply[slot][TempIndex].Roll.a+=frac;
		for (n=0;n<RateDivider;n++)
		{
			DeltasToApply[slot][TempIndex].Roll.a+=valfrac;

			TempIndex++;

			if (TempIndex>=RateDivider)
				TempIndex=0;
		}
	}

	if(packet->Velocity&8)
	{
		val=-((packet->Velocity&7)<<shiftfactor);
	}
	else
	{
		val=+((packet->Velocity&7)<<shiftfactor);
	}

	{
		TempIndex=DeltaIndex;
		valfrac=val/RateDivider;
		frac=(val-(valfrac*RateDivider));

		DeltasToApply[slot][TempIndex].Velocity+=frac;
		for (n=0;n<RateDivider;n++)
		{
			DeltasToApply[slot][TempIndex].Velocity+=valfrac;

			TempIndex++;

			if (TempIndex>=RateDivider)
				TempIndex=0;
		}
	}
}

void	DPlay::StoreReceivedBField(char* pack, ULong len)
{
	char* ptr=pack;

	LPGRLIST temp=new GRLIST;

	ptr+=sizeof(ULong);

	temp->ordernum=*(UWord*)ptr;
	ptr+=sizeof(UWord);

	temp->bfctrl=*(ULong*)ptr;
	ptr+=sizeof(ULong);

	temp->file=*(FileNum*)ptr;
	ptr+=sizeof(FileNum);

	temp->list=NULL;

// add gr list to entry

	bool finished=false;
	ULong val,m;
	UByte set;

	while (!finished)
	{
		set=*(UByte*)ptr;
		ptr+=sizeof(UByte);

		for (m=0;m<8;m++)
		{
			LPREPGRENTRY tempgr=new REPGRENTRY;
			tempgr->next=NULL;

			if (set&0x1<<m)
			{
				tempgr->val=*(int*)ptr;
				ptr+=sizeof(int);

				if (tempgr->val==BAD_RV)
					finished=true;
			}
			else
			{
				tempgr->val=0;
			}

			if (temp->list)
			{
				LPREPGRENTRY temp2=temp->list;

				while (temp2->next)
					temp2=temp2->next;

				temp2->next=tempgr;
			}
			else
			{
				temp->list=tempgr;
			}
			tempgr=NULL;
		}
	}

// add entry to list

	temp->next=_Replay.bfgrlist;
	_Replay.bfgrlist=temp;

	temp=NULL;
}

bool	DPlay::ProcessRequiredBFieldPacket(ULong& bfctrl, FileNum& filelist)
{
// traverse list and see if bfield with correct order number is present

	LPGRLIST temp=_Replay.bfgrlist;
	ULong n;
	int val;
	bool finished;

	while (temp)
	{
		if (temp->ordernum==BFieldToProcess)
		{
			bfctrl=temp->bfctrl;
			filelist=temp->file;

			finished=false;

			n=0;
			LPREPGRENTRY tempgr=temp->list;

			while (!finished)
			{
				if (tempgr)
				{
					val=tempgr->val;

					if (val!=BAD_RV)
					{
						Persons2::SetLoc(n,val);
						n++;
					}
					else
					{
						finished=true;
					}
					tempgr=tempgr->next;
				}
				else
				{
					finished=true;
				}
			}

			return true;
		}
		temp=temp->next;
	}

	return false;
}

//------------------------------------------------------------------------------
//Procedure		DPlay
//Author		R. Hyde 
//Date			Tue 3 Dec 1996
//
//Description	Moved constructor & destructor from winmove.h
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
CON		DPlay::DPlay ()
{
#ifndef NDEBUG
	CheckPacketSizes();
#endif

	CoInitialize(NULL);

	ULong n,m;

	lpDP4=NULL;
	lpDPL=NULL;
	templpDPC=NULL;

	ServiceList=NULL;

	FrameInc=0;
	RateDivider=2; //Can be e.g. 1, 2, 4.  =1 send/receive every frame

	UIPlayerType = 0;
	Implemented = FALSE;
	PlayerCreated = FALSE;
	FlyNowFlag = FALSE;
	FrameCount = 0;
	SyncBits = 0;
	ActivePlayers = 0;

	for (n=0; n<MAXPLAYERS; n++)
	{
		AIACPositions[n]=NULL;
	}

	Bool	dp_usepaintscheme;
	GameType=DEATHMATCH;
 	MyTeamKills=0;
 	EnemyTeamKills=0;
 	EnemyTeamDeaths=0;
	MyTeamDeaths=0;

	AllowSuicide=TRUE;

	ResyncPhase=FALSE;
	WorldSyncPhase=FALSE;
	WorldSyncPhaseReady=false;
	Initiator=FALSE;
	ServedGame=FALSE;

	GameAvailable=FALSE;
	ResendTime=20;

	Joining=FALSE;
	Lobbied=FALSE;
	VisitorsBook=NULL;
	GameRunning=FALSE;

	MainSheetCount=FALSE;

	Password[0]=0;
	SessionName[0]=0;
	PlayerName[0]=0;
	tempdpsessdesc2=NULL;
	SimulateExitKey=FALSE;
	LeaveCommsFlag=FALSE;
	HostQuitFlag=FALSE;
	MyAIACPositionsToSend=NULL;
	UpdateFrag=FALSE;
	UpdateMission=FALSE;
	DisplayInfo=FALSE;
	NextCommsOwner=NULL;
	sendtowho=255;// default is send to all players
	FragChanged=false;
	viewtime=true;
	singlesession=false;
	campaignloaded=false;
	megashootcheat=false;
	invulnerablecheat=false;
	HSB=NULL;
	ResendStatus=0;
	ResendPacket=0;

	MsgRecipientAll=true;
	SideSelected=false;
	CommsPacketPtr=NULL;
	CommsPacketLength=0;
	PossibleLobby=false;
}

CON DPlay::~DPlay ()
{
	ExitDirectPlay();
	DeleteVisitorBook();

	CoUninitialize(); // OK to call this now, all interfaces have been released
}


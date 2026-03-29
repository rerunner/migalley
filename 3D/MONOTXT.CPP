//------------------------------------------------------------------------------
//Filename       monotxt.cpp
//System         
//Author         Paul.   
//Date           Wed 16 Oct 1996
//Description    
//------------------------------------------------------------------------------
#include "WIN32_COMPAT.H" //RERUN
#define DIRECTINPUT_VERSION 0x700 // RERUN
#include "dinput_stub.h"

#include <stdio.h>

//TempCode JIM 14Nov00 #undef NDEBUG

#define NO_AFX_AUTO
//DEADCODE JIM 08/06/00 typedef union _LARGE_INTEGER {
//DEADCODE JIM 08/06/00     __int64 QuadPart;
//DEADCODE JIM 08/06/00 } LARGE_INTEGER;
//#include	"stdafx.h"
#define NOATL
#include	"DOSDEFS.H"
#include	"MONOTXT.H"
#include	"MYTIME.H"

#if 0 // RERUN stop shouting
#ifdef	NDEBUG
#pragma message (__HERE__ "Mono debugs disabled")
#else
#pragma message (__HERE__ "Mono debugs enabled!!!!")
#endif
#endif

const	SLong	MonoScreenWidth = 80;
const	SLong	MonoScreenHeight= 25;
const	SLong	MonoStartAddr = 0x0B0000;

class MonoText Mono_Text;

//------------------------------------------------------------------------------
//Procedure		MonoText
//Author		Paul.   
//Date			Wed 16 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MonoText::ClsMono()
{
#if 0 //RERUN #ifndef NDEBUG													//DAW 13Nov96

	HANDLE screen = GetStdHandle(STD_OUTPUT_HANDLE);
COORD pos = {0, 0};
DWORD written;
CONSOLE_SCREEN_BUFFER_INFO screen_attr;
unsigned size;

GetConsoleScreenBufferInfo(screen, &screen_attr);

size = screen_attr.dwSize.X * screen_attr.dwSize.Y;

FillConsoleOutputCharacter( screen,' ', size, pos, &written);
SetConsoleCursorPosition(screen, pos);

#endif
}

//------------------------------------------------------------------------------
//Procedure		SetCursor
//Author		Paul.   
//Date			Wed 16 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
#ifndef NDEBUG													//DAW 13Nov96
void MonoText::SetCursor(SLong rx,SLong ry)
{
	cx = rx; cy = ry;

}
#endif

//------------------------------------------------------------------------------
//Procedure		Print
//Author		Paul.   
//Date			Thu 24 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MonoText::Print(UByte* text,SLong length)
{
//RERUN TRACING(text);
	printf((char*)text);
/*
#ifndef NDEBUG													//DAW 13Nov96
	char str[1024];
	memcpy(&str, text, length);
	str[length] = '\0';

	UByte  chr;

	COORD pos = { cx, cy };
	HANDLE hConsole_c = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD len = strlen((const char *)str);
	DWORD dwBytesWritten = 0;
	WriteConsoleOutputCharacter(hConsole_c, (const char *)str, len, pos, &dwBytesWritten);
	CloseHandle(hConsole_c);

	int i = 0;
	while (str[i])
	{
		chr = str[i++];

		if (chr == '\n')	//newline
		{
			cx = 0;
			cy++;

		}
		else
		{

			cx++;

			if (cx >= MonoScreenWidth)
			{
				cx = 0;
				cy++;
			}

		}
	}
#endif
*/
}

//------------------------------------------------------------------------------
//Procedure		TextOutMono
//Author		Paul.   
//Date			Wed 16 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
#ifndef NDEBUG													//DAW 13Nov96
void MonoText::TextOutMono(UByte* str)
{
	printf((char*)str); //RERUN
/*
	UByte  chr;

	COORD pos = { cx, cy };
	HANDLE hConsole_c = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD len = strlen((const char *)str);
	DWORD dwBytesWritten = 0;
	WriteConsoleOutputCharacter(hConsole_c, (const char *)str, len, pos, &dwBytesWritten);
	CloseHandle(hConsole_c);

	while (*str)
	{
		chr = *str++;

		if (chr=='\n')	//newline
		{
			cx = 0;
			cy++;

		}
		else
		{

			cx++;

			if (cx>=MonoScreenWidth)
			{
				cx = 0;
				cy++;
			}

		}
	}
*/
}
#endif
//------------------------------------------------------------------------------
//Procedure		BinToASCII
//Author		Paul.   
//Date			Wed 16 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
#ifndef NDEBUG													//DAW 13Nov96
void MonoText::BinToASCII(SLong value,UByte* strval)
{
	SLong	ival;

	strval += 9;

	for (int counter=0;counter<10;counter++)
	{
		ival = value % 10;

		value = (value - ival)/10;

		*strval = '0' + (UByte )ival;

		strval--;
	}
}
#endif
//------------------------------------------------------------------------------
//Procedure		StartTimer
//Author		Paul.   
//Date			Wed 16 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MonoText::StartTimer(SLong& timevar)
{
#ifndef NDEBUG													//DAW 13Nov96
//TempCode PD 22Oct96 	timevar = Timer_Code.tus;
#endif
}

//------------------------------------------------------------------------------
//Procedure		StopTimer
//Author		Paul.   
//Date			Wed 16 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MonoText::StopTimer(SLong& timevar)
{
#ifndef NDEBUG													//DAW 13Nov96
//	SLong	stoptime;

//TempCode PD 22Oct96 	stoptime = Timer_Code.tus;

//	stoptime -= timevar;

//	timevar = stoptime;
#endif
}

//------------------------------------------------------------------------------
//Procedure		StopTimer
//Author		Paul.   
//Date			Wed 16 Oct 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void MonoText::StopTimer(SLong& timevar,UByte* string)
{
#ifndef NDEBUG													//DAW 13Nov96
//	SLong	stoptime;

//	UByte	ltstring[] = "1234567890 \n";

//	UByte*	tstring = ltstring;

//TempCode PD 22Oct96 	stoptime = Timer_Code.tus;

//	stoptime -= timevar;

//	TextOutMono(string);

//	BinToASCII(stoptime,tstring);

//	TextOutMono(tstring);
#endif
}

#ifndef NDEBUG													//DAW 13Nov96
void	MonoText::BinToHEX(SLong v,UByte* c)
{
	int p=7;
	while (p>=0)
	{
		int i=v&0xf;
		v>>=4;
		if (i>9)
			c[p]='A'-10+i;
		else
			c[p]='0'+i;
		p--;
	}
	c[8]=0;
}
#endif


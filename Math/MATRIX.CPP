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
//Filename       matrix.cpp
//System         
//Author         Paul.   
//Date           Tue 22 Aug 1995
//Description    3D transformation matrix generation code
//------------------------------------------------------------------------------
#include "WIN32_COMPAT.H" //RERUN
#include "SDL.h"
	#include	"DOSDEFS.H"
	#include	"MYANGLES.H"
	#include	"DISPLAY.H"
	#include	"3DDEFS.H"
	#include	"WORLDINC.H"

	#include	"MATRIX.H"
	#include 	"MYMATH.H"
	#include	"VECTOR.H"
	#include	"VERTEX.H"
	#include	"ENUMBITS.M"
	#include	"POLYGON.H"

	class	matrix	_matrix;


//	MATRIX ELEMENTS
//  ---------------

//	L11       CH.CR - SP.SH.SR
//  L21       - (SR.CH + SP.SH.CR)
//  L31       - CP.SH
//	L12       SR.CP
//	L22       CP.CR
//	L32       - SP
//	L13       SP.CH.SR + SH.CR
//	L23       CH.CR.SP  - SH.SR
//	L33       CP.CH


#ifdef	__WATCOMC__
extern	"C"	SLong ASMTransform(MATRIX_PTR,SLong&,SLong&,SLong&);
#else
//RERUN taken from BOB
SLong ASMTransform(MATRIX_PTR m, SLong& x, SLong& y, SLong& z)
{ //x0r tested
	int _EAX;
	unsigned long _ECX;

	int LocalX; // weak
	int LocalY; // weak
	int LocalZ; // weak
	int ScaleFactor; // weak
	long long int BodyX; // weak
	long long int BodyY; // weak
	long long int BodyZ; // weak

	LocalX = x;
	LocalY = y;
	LocalZ = z;

#define SAR(a)  ((((a) >> 31) ^ (a)) - ((a) >> 31)) 

	_EAX = SAR(x)
		| SAR(y)
		| SAR(z);
	if (_EAX < 32767)
	{
		ScaleFactor = 0;
	}
	else
	{
		//RERUN _BitScanReverse(&_ECX, _EAX);
#if defined(__MSVC__)
		__asm { mov eax, _EAX
		        bsr     ecx, eax
		        mov _ECX, ecx}
#elif defined(__GNUC__)
	asm ("bsrl %1, %0\n\t"
			"jnz  1f\n\t"
			"xorl %0, %0\n\t"   // force 0 if input was 0
			"1:"
			: "=c" (_ECX) : "a" (_EAX) : "cc");
#endif
		_ECX -= 14;
		LocalY >>= _ECX;
		LocalX >>= _ECX;
		LocalZ >>= _ECX;
		ScaleFactor = _ECX;
	}
	BodyX = 2 * (LocalX * m->L11
		+ LocalY * m->L12
		+ LocalZ * m->L13);

	BodyY = 2 * (LocalX * m->L21
		+ LocalY * m->L22
		+ LocalZ * m->L23);

	BodyZ = 2 * (LocalX * m->L31
		+ LocalY * m->L32
		+ LocalZ * m->L33);

	_EAX = SAR(BodyX >> 32)
		| SAR(BodyY >> 32)
		| SAR(BodyZ >> 32);
	if ((unsigned int)_EAX > 0x7FFF)
	{
		//RERUN _BitScanReverse(&_ECX, _EAX);
#if defined(__MSVC__)
		__asm { mov eax, _EAX
		        bsr     ecx, eax
		        mov _ECX, ecx}
#elif defined(__GNUC__) //RERUN
		asm ("bsrl %1, %0\n\t"
			"jnz  1f\n\t"
			"xorl %0, %0\n\t"   // force 0 if input was 0
			"1:"
			: "=c" (_ECX) : "a" (_EAX) : "cc");
#endif
		_ECX -= 14;
		ScaleFactor += _ECX;
		BodyX >>= (_ECX & 0x1F);
		BodyY >>= (_ECX & 0x1F);
		BodyZ >>= (_ECX & 0x1F);
	}


	x = BodyX & 0xffffffff;
	y = BodyY & 0xffffffff;
	z = BodyZ & 0xffffffff;
	return ScaleFactor;
}
#endif

//-----------------------------------------------------------------------------
// Procedure    ASMDoBigXProd
// Author       Paul
// Date         05/11/97
//
// Description  
//
// Inputs       
//
// Returns      
//
//-----------------------------------------------------------------------------
#ifdef	__WATCOMC__
extern	"C" Bool ASMDoBigXProd(SLong,SLong,SLong,SLong);		//PD 27Nov96
#else
#ifdef	__MSVC__
extern	"C" void XASMDoBigXProd(void);
#pragma warning (disable:4035)
inline	Bool ASMDoBigXProd(SLong a,SLong b,SLong c,SLong d)
{
	_asm {
		push	ecx
		mov		eax,a
		mov		edx,b
		mov		ebx,c
		mov		ecx,d
		call	XASMDoBigXProd
		pop		ecx
	}
}
#pragma warning (default:4035)
#elif defined(__GNUC__) //RERUN
inline	Bool ASMDoBigXProd(SLong a,SLong b,SLong c,SLong d)
{
    const std::int64_t ac  = std::int64_t(a) * c;
    const std::int64_t bd  = std::int64_t(b) * d;
    const std::int64_t sum = ac + bd;

    return (Bool)((((ac ^ sum) & (bd ^ sum)) >> 63) & 1);
}
#endif
#endif

extern	void ASMBody2Screen(SLong&,SLong&,SLong&,SLong&,SLong&,SLong&);
#ifdef __WATCOMC__
#pragma	aux	 ASMBody2Screen			=		\
"		fild	dword ptr ds:[ecx]			"\
"		fabs								"\
"		mov		ecx,00800000h				"\
"		fild	dword ptr ds:[edx]			"\
"		push	ecx							"\
"		fmulp	st(1),st					"\
"		fild	dword ptr ss:[esp]			"\
"		fdivrp	st(1),st					"\
"		add		esp,4						"\
"		fld		st(0)						"\
"		fild	dword ptr ds:[eax]			"\
"		fmulp	st(1),st					"\
"		fild	dword ptr ds:[ebx]			"\
"		fmulp	st(2),st					"\
"		fistp	dword ptr ds:[esi]			"\
"		fchs								"\
"		fistp	dword ptr ds:[edi]			"\
parm	[eax] [ebx] [ecx] [edx] [esi] [edi]	\
modify	[ecx]
#else
#ifdef __MSVC__
inline 	void ASMBody2Screen(SLong& num1,SLong& num2,SLong& num3,SLong& num4,SLong& num5,SLong& num6)
{
    __asm
    {
		mov eax,num1;
		mov ebx,num2;
		mov ecx,num3;
		mov edx,num4;
		mov esi,num5;
		mov edi,num6;
		fild	dword ptr ds:[ecx];
		fabs;
		mov		ecx,00800000h;
		fild	dword ptr ds:[edx];
		push	ecx;
		fmulp	st(1),st;
		fild	dword ptr ss:[esp];
		fdivrp	st(1),st;
		add		esp,4;
		fld		st(0);
		fild	dword ptr ds:[eax];
		fmulp	st(1),st;
		fild	dword ptr ds:[ebx];
		fmulp	st(2),st;
		fistp	dword ptr ds:[esi];
		fchs;
		fistp	dword ptr ds:[edi];
    }
}
#elif defined(__GNUC__) //RERUN
inline void ASMBody2Screen(SLong& xin,  SLong& yin,
                           SLong& z,    SLong& proj,
                           SLong& xout, SLong& yout)
{
    constexpr double FIXED_SCALE = 8388608.0;  // 0x00800000

    const double recip = FIXED_SCALE / (std::fabs(static_cast<double>(z)) *
                                        static_cast<double>(proj));

    const double sx = static_cast<double>(xin) * recip;
    const double sy = static_cast<double>(yin) * recip;

    xout = static_cast<SLong>(std::nearbyint(sx));
    yout = static_cast<SLong>(std::nearbyint(-sy));   // Y inversion for screen space
}
#endif
#endif

//Use maths coprocessor sincos function - converts Rowan degrees
//to radians first
//
extern	void fpSin_Cos(Angles angle, Float& sin_ang, Float& cos_ang);
#ifdef __WATCOMC__
#pragma	aux	 fpSin_Cos =					\
"		and		eax,0x0000FFFF				"\
"		fldpi								"\
"		push	eax							"\
"		fild	dword ptr ss:[esp]			"\
"		fmulp	st(1),st					"\
"		mov		dword ptr ss:[esp],32768	"\
"		fild	dword ptr ss:[esp]			"\
"		fdivp	st(1),st					"\
"		fsincos								"\
"		fstp	qword ptr ds:[ebx]			"\
"		fstp	qword ptr ds:[edx]			"\
"		add		esp,4						"\
parm	[eax] [edx] [ebx]					\
modify	[eax]
#else
#ifdef __MSVC__
inline	void fpSin_Cos(ANGLES angle, Float& sin_ang, Float& cos_ang)
{
	int	iang = angle;
    __asm
    {
		mov eax,iang;
		mov edx,sin_ang;
		mov ebx,cos_ang;
		and		eax,0x0000FFFF;
		fldpi;
		push	eax;
		fild	dword ptr ss:[esp];
		fmulp	st(1),st;
		mov		dword ptr ss:[esp],32768;
		fild	dword ptr ss:[esp];
		fdivp	st(1),st;
		fsincos;
		fstp	qword ptr ds:[ebx];
		fstp	qword ptr ds:[edx];
		add		esp,4;
    }
}

inline void fpTan(ANGLES ang,Float& tanAng)
{
	int iang=ang;
	_asm
	{
	mov eax,iang;
	mov edx,tanAng;
	and eax,0x0000FFFF;
	fldpi;
	push eax;
	fild dword ptr ss:[esp];
	fmulp st(1),st;
	mov dword ptr ss:[esp],32768;
	fild dword ptr ss:[esp];
	fdivp st(1),st;
	fsincos;
	fdivp st(1),st;
	fstp qword ptr ds:[edx];
	add esp,4;
	}
}
#elif defined(__GNUC__) //RERUN
inline void fpSin_Cos(ANGLES angle, Float& sin_ang, Float& cos_ang)
{
	uint16_t raw = static_cast<uint16_t>(angle.a);   // forces 0–65535, no sign extension
	double radians = raw * (M_PI / 32768.0);
	sin_ang = std::sin(radians);
	cos_ang = std::cos(radians);
}

inline void fpTan(ANGLES ang, Float& tanAng)
{
    constexpr double SCALE = M_PI / 32768.0;  // π / 32768
    uint16_t u16 = static_cast<uint16_t>(ang.a);        // 0..65535 exactly like the asm
    tanAng = std::tan(u16 * SCALE);
}
#endif
#endif

//------------------------------------------------------------------------------
//Procedure	matrix
//LastModified:	PD 20Dec95
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Tue 22 Aug 1995
//Modified	Removed old code and added coordinate scaling
//
//Description	
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void matrix::generate(ANGLES heading, ANGLES pitch, ANGLES roll, MATRIX_PTR matrix)//PD 25Jul96
{
	SWord	sh,sp,sr,ch,cp,cr;

	Math_Lib.high_sin_cos(heading,sh,ch);						//PD 23Feb96
																//RDH 30Oct95
	Math_Lib.high_sin_cos(pitch,sp,cp);							//PD 23Feb96

	Math_Lib.high_sin_cos(roll,sr,cr);							//PD 23Feb96

	SWord	sr_sh,sr_ch,cr_sh,cr_ch;

	sr_sh = (sh * sr)>>15;

	sr_ch = (sr * ch)>>15;

	cr_sh = (cr * sh)>>15;

	cr_ch = (cr * ch)>>15;

	matrix->L11 = cr_ch - ((sp * sr_sh)>>15);

	matrix->L21 = -(sr_ch + ((sp * cr_sh)>>15));

	matrix->L31 = -((cp * sh)>>15);


	matrix->L12 = (sr * cp)>>15;

	matrix->L22 = (cp * cr)>>15;

	matrix->L32 = -sp;


	matrix->L13 = ((sp * sr_ch)>>15) + cr_sh;

	matrix->L23 = ((sp * cr_ch)>>15) - sr_sh;

	matrix->L33 = (cp * ch)>>15;
}

//------------------------------------------------------------------------------
//Procedure		generate2
//Author		Paul.   
//Date			Mon 26 Feb 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::generate2(ANGLES heading, ANGLES pitch, ANGLES roll, MATRIX_PTR matrix)
{
	SWord	sh,sp,sr,ch,cp,cr;

	SWord	sp_sh,sr_ch,sp_ch,ch_cr;

	Math_Lib.high_sin_cos(heading,sh,ch);						//PD 23Feb96

	Math_Lib.high_sin_cos(pitch,sp,cp);							//PD 23Feb96

	Math_Lib.high_sin_cos(roll,sr,cr);							//PD 23Feb96

	sp_sh = (sp * sh)>>15;

	sr_ch = (sr * ch)>>15;

	sp_ch = (sp * ch)>>15;

	ch_cr = (ch * cr)>>15;

	matrix->L11 = ch_cr + ((sp_sh * sr)>>15);

	matrix->L12 = sr_ch - ((sp_sh * cr)>>15);

	matrix->L13 = (cp * sh)>>15;

	matrix->L21 = -((sr *cp)>>15);								//PD 26Feb96

	matrix->L22 = (cp * cr)>>15;

	matrix->L23 = sp;

	matrix->L31 = ((sp * sr_ch)>>15) - ((sh * cr)>>15);

	matrix->L32 = -(((cr * sp_ch)>>15) + ((sh * sr)>>15));

	matrix->L33 = (cp * ch)>>15;
}

//------------------------------------------------------------------------------
//Procedure		generater
//Author		Paul.   
//Date			Wed 5 Jun 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::generater(ANGLES roll,MATRIX_PTR m)
{
	SWord	sin_roll,cos_roll;

	roll = ANGLES_0Deg-roll;

	Math_Lib.high_sin_cos(roll,sin_roll,cos_roll);

	m->L11 =
		m->L22 = cos_roll;

	m->L21 = sin_roll;

	m->L12 = -sin_roll;

	m->L31 = m->L32 = m->L13 = m->L23 = 0;
	
	m->L33 = 0x07FFF;
}

//------------------------------------------------------------------------------
//Procedure		generatep
//Author		Paul.   
//Date			Thu 6 Jun 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::generatep(ANGLES pitch, MATRIX_PTR m)
{
	SWord	sin_pitch,cos_pitch;

	pitch = ANGLES_0Deg-pitch;

	Math_Lib.high_sin_cos(pitch,sin_pitch,cos_pitch);

	m->L11 = 0x07FFF;

	m->L21 = m->L31 = m->L12 = m->L13 = 0;

	m->L22 = m->L33 = cos_pitch;

	m->L32 = sin_pitch;

	m->L23 = -sin_pitch;
}

//------------------------------------------------------------------------------
//Procedure		generateh
//Author		Paul.   
//Date			Tue 2 Jul 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::generateh(ANGLES hdg,MATRIX_PTR m)
{
	SWord	sin_hdg,cos_hdg;

	Math_Lib.high_sin_cos(hdg,sin_hdg,cos_hdg);

	m->L11 =
		m->L33 = cos_hdg;

	m->L31 = -sin_hdg;

	m->L13 = sin_hdg;

	m->L21 = m->L12 = m->L32 = m->L23 = 0;

	m->L22 = 0x07FFF;
}

//------------------------------------------------------------------------------
//Procedure		inverse
//Author		Paul.   
//Date			Fri 17 Nov 1995
//
//Description	Generates the inverse of the current transformation
//				matrix.
//
//L11	=	CR.CH + SR.SP.SH
//L21	=	-SR.CP
//L31	=	SR.SP.CH - CR.SH
//
//
//L12	=	SR.CH - CR.SP.SH
//L22	=	CR.CP
//L32	=	-(SR.SH + CR.SP.CH)
//
//
//L13	=	CP.SH
//L23	=	SP
//L33	=	CP.CH
//
// For the following version, the angles don't need reversing
// uses cos(-theta) == cos(theta) and sin(-theta) == -sin(theta)
// to simplify the method above.
//	
//L11	=	CR.CH - SR.SP.SH
//L21	=	SR.CP
//L31	=	SR.SP.CH + CR.SH
//
//
//L12	=	-SR.CH - CR.SP.SH
//L22	=	CR.CP
//L32	=	-(SR.SH - CR.SP.CH)
//
//
//L13	=	-CP.SH
//L23	=	-SP
//L33	=	CP.CH
//
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::inverse(ANGLES heading, ANGLES pitch, ANGLES roll, MATRIX_PTR matrix)
{
	SWord	sh,sp,sr,ch,cp,cr;

	SLong	cr_ch,sr_sp,cr_sp,temp;

	//Get the sines/cosines of these angles

	Math_Lib.high_sin_cos(heading,sh,ch);						//PD 23Feb96

	Math_Lib.high_sin_cos(pitch,sp,cp);							//PD 23Feb96

	Math_Lib.high_sin_cos(roll,sr,cr);							//PD 23Feb96

	//Generate some intermediate values

	cr_ch = cr * ch;
	sr_sp = sr * sp;
	cr_sp = cr * sp;

	//Fill in the new matrix

	temp		=	cr_ch - ((sr_sp>>15) * sh);
	matrix->L11	=	temp>>15;

	temp		=	sr * cp;
	matrix->L21	=	temp>>15;

	temp		=	((sr_sp>>15) * ch) + (cr * sh);
	matrix->L31	=	temp>>15;

	temp		=	-(sr * ch) - ((cr_sp>>15) * sh);
	matrix->L12	=	temp>>15;

	temp		=	cr * cp;
	matrix->L22	=	temp>>15;

	temp		=	((cr_ch>>15) * sp)-(sr * sh);
	matrix->L32	=	temp>>15;

	temp		=	-(cp * sh);
	matrix->L13	=	temp>>15;

	matrix->L23	=	-sp;

	temp		=	cp * ch;
	matrix->L33	=	temp>>15;

}

//------------------------------------------------------------------------------
//Procedure		transform_y
//LastModified:	PD 13Mar96
//Author		Paul.   
//Date			Thu 23 Nov 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
	extern SLong TestOFlowY(SLong a);
#ifdef __WATCOMC__
	#pragma aux TestOFlowY =					\
				"mov	edx,eax"				\
				"sar	edx,0x1F"				\
				"add	eax,eax"				\
				"adc	edx,edx"				\
				"mov	ebx,eax"				\
				"sar	ebx,0x1F"				\
				"xor	ebx,edx"				\
				"jz		short noover"			\
				"sar	edx,1"					\
				"rcr	eax,1"					\
			"noover:"							\
				parm	[eax]					\
				modify	[edx ebx]				\
				value	[eax]
#else
#ifdef __MSVC__
inline  SLong TestOFlowY(SLong a)
{
	SLong	retval;
    __asm
    {
		mov eax,a;
		mov	edx,eax;
		sar	edx,0x1F;
		add	eax,eax;
		adc	edx,edx;
		mov	ebx,eax;
		sar	ebx,0x1F;
		xor	ebx,edx;
		jz		short noover;
		sar	edx,1;
		rcr	eax,1;
	noover:
		mov retval,eax;
    }
    return retval;
}
#elif defined(__GNUC__) //RERUN
inline SLong TestOFlowY(SLong a)
{
    SLong doubled = a << 1;

    // Signed overflow when shifting left by 1 occurs if:
    //   a == 0xC0000000  (-1073741824)  → becomes 0x80000000 on shift
    //   (only possible negative value that overflows on <<1)
    //   or any positive value >= 0x40000000 would overflow, but that's impossible in signed 32-bit

    // So actually only one value overflows: 0xC0000000
    // But the generic test works for any future changes:

    if (a == (doubled >> 1))          // no overflow → doubled is exactly a*2
        return doubled;
    else
        return a >> 1;                // overflow → saturate to a>>1
}
#endif
#endif

//------------------------------------------------------------------------------
//Procedure	transform
//LastModified:	PD 31Oct95
//------------------------------------------------------------------------------
//Author		Paul.   
//Date		Fri 25 Aug 1995
//Modified	
//
//Description	Transforms a point (Now includes a shift factor to avoid any
//				overflows later).
//
//Inputs	
//
//Returns	
//
//Externals
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
UWord matrix::transform(MATRIX_PTR T, SLong& x, SLong& y, SLong& z)
{
	return ((UWord )ASMTransform(T,x,y,z));
}

//------------------------------------------------------------------------------
//Procedure		rotate
//Author		Paul.   
//Date			Wed 5 Jun 1996
//
//Description	Transformation with no scaling tests
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::rotate(MATRIX_PTR T, SLong& x, SLong& y, SLong& z)
{
	SLong txa,tya,tza;

	SLong txb,tyb,tzb;

	SLong txc,tyc,tzc;

	txa = (T->L11)*x; txb = (T->L12)*y; txc = (T->L13)*z;

	tya = (T->L21)*x; tyb = (T->L22)*y; tyc = (T->L23)*z;

	tza = (T->L31)*x; tzb = (T->L32)*y; tzc = (T->L33)*z;

	x = (txa + txb + txc)/ANGLES_FRACT;

	y = (tya + tyb + tyc)/ANGLES_FRACT;

	z = (tza + tzb + tzc)/ANGLES_FRACT;
}

//------------------------------------------------------------------------------
//Procedure		multiply
//Author		Paul											//PD 27Feb96
//Date			Wed 25 Oct 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::multiply(MATRIX_PTR t,MATRIX_PTR sip)
{
	MATRIX		tmat;											//PD 25Jul96
	MATRIX_PTR	dip = &tmat;									//PD 25Jul96

	SWord	*si,*di;

	si = &sip->L11;
	di = &dip->L11;

	int	j;

	SLong	temp;

	for(j=0;j<3;j++)
	{
		temp = (*si * t->L11)
			+ (*(si+1) * t->L21)
				+ (*(si+2) * t->L31);

		*di++ = (SWord )(temp>>15);

		temp = (*si * t->L12)
			+ (*(si+1) * t->L22)
				+ (*(si+2) * t->L32);

		*di++ = (SWord )(temp>>15);

		temp = (*si * t->L13)
			+ (*(si+1) * t->L23)
				+ (*(si+2) * t->L33);

		*di++ = (SWord )(temp>>15);

		si += 3;
	}

	//copy the new matrix back to sip

	si = &t->L11;
	di = &dip->L11;

	for(j=0;j<9;j++)
	{
		*si++ = *di++;
	}

}

//------------------------------------------------------------------------------
//Procedure		scaleto16bit
//Author		Paul.   
//Date			Tue 31 Oct 1995
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
#if defined(__WATCOMC__)
extern	"C" SWord GetScale(SLong,SLong,SLong);
#else
#ifdef	__MSVC__
inline  SWord GetScale(SLong x, SLong y, SLong z)
{
	SWord	retval;
	__asm
	{
		mov eax, x;
		mov ebx, y;
		mov ecx, z;
		mov	edx, eax;
		sar	edx, 0x1F;
		xor	eax, edx;
		sub	eax, edx;
		mov	edx, ebx;
		sar	edx, 0x1F;
		xor	ebx, edx;
		sub	ebx, edx;
		mov	edx, ecx;
		sar	edx, 0x1F;
		xor	ecx, edx;
		sub	ecx, edx;
		or		eax, ebx;
		or		ecx, eax;
		bsr	eax, ecx;
		sub	eax, 0x0E;
		jnc	short done;
		xor	eax, eax;
	done:;
		mov retval, ax;
	}
	return retval;
}
#elif defined(__GNUC__) //RERUN
inline SWord GetScale(SLong x, SLong y, SLong z)
{
	uint32_t ax = static_cast<uint32_t>(x < 0 ? -x : x);
    uint32_t ay = static_cast<uint32_t>(y < 0 ? -y : y);
    uint32_t az = static_cast<uint32_t>(z < 0 ? -z : z);

    uint32_t m = ax | ay | az;
    if (m == 0) return 0;

    int bit = 31 - __builtin_clz(m);      // bsr equivalent
    int scale = bit - 14;
    return static_cast<SWord>(scale > 0 ? scale : 0);
}
#endif
#endif

UWord matrix::scaleto16bit(SLong& x, SLong& y, SLong& z)
{
	SWord	ret_val;

	ret_val = GetScale(x,y,z);

	if(ret_val!=0)
	{
		x>>=ret_val;

		y>>=ret_val;

		z>>=ret_val;
	}

	return(ret_val);
}

//------------------------------------------------------------------------------
//Procedure		Body2Screen
//Author		Paul.   
//Date			Mon 25 Sep 1995
//
//Description	Transform from body coordinate system to screen 
//				coordinate system
//
//Inputs		
//
//Returns		FALSE if the body Z for the point was negative.
//
//------------------------------------------------------------------------------
void matrix::body2screen(DoPointStruc &dopoint)					//PD 25Jul96
{
	if (mat_win->DoingHardware3D())
	{
		Float scalex,scaley;
		scalex=Float(mat_win->VirtualWidth)/Float(mat_win->PhysicalWidth);
		scaley=Float(mat_win->VirtualHeight)/Float(mat_win->PhysicalHeight);
		mat_win->DoBody2Screen(dopoint);
		dopoint.screenx.f*=scalex;
		dopoint.screeny.f*=scaley;
	}
	else
	{
		R3DVALUE rw=R3DVALUE(1./(POLYGON.viewdata.hoD*dopoint.bodyz.f));	
		dopoint.screenx.f=R3DVALUE(POLYGON.viewdata.scalex*dopoint.bodyx.f*rw+POLYGON.viewdata.originx);
		dopoint.screeny.f=R3DVALUE(-POLYGON.viewdata.scaley*dopoint.bodyy.f*rw+POLYGON.viewdata.originy);
	}
}


//------------------------------------------------------------------------------
//Procedure		Body2Screen
//LastModified:	PD 04Jan96
//Author		Paul.   
//Date			Mon 25 Sep 1995
//
//Description	Transform from body coordinate system to screen 
//				coordinate system
//
//Inputs		
//
//Returns		FALSE if the body Z for the point was negative.
//
//------------------------------------------------------------------------------
void matrix::body2screen(VERTEX &vertex)						//PD 25Jul96
{
	if (mat_win->DoingHardware3D())
	{
		DoPointStruc temp;
		temp.bodyx.f=vertex.bx.f;
		temp.bodyy.f=vertex.by.f;
		temp.bodyz.f=vertex.bz.f;
		Float scalex,scaley;
		scalex=Float(mat_win->VirtualWidth)/Float(mat_win->PhysicalWidth);
		scaley=Float(mat_win->VirtualHeight)/Float(mat_win->PhysicalHeight);
		mat_win->DoBody2Screen(temp);
		temp.screenx.f*=scalex;
		temp.screeny.f*=scaley;
		vertex.sx.f=temp.screenx.f;
		vertex.sy.f=temp.screeny.f;
	}
	else
	{
		DoPointStruc dopoint;
		dopoint.bodyx.f=vertex.bx.f;
		dopoint.bodyy.f=vertex.by.f;
		dopoint.bodyz.f=vertex.bz.f;
		R3DVALUE rw=R3DVALUE(1./(POLYGON.viewdata.hoD*dopoint.bodyz.f));	
		dopoint.screenx.f=R3DVALUE(POLYGON.viewdata.scalex*dopoint.bodyx.f*rw+POLYGON.viewdata.originx);
		dopoint.screeny.f=R3DVALUE(-POLYGON.viewdata.scaley*dopoint.bodyy.f*rw+POLYGON.viewdata.originy);
		vertex.sx.f=dopoint.screenx.f;
		vertex.sy.f=dopoint.screeny.f;
	}	
}

//------------------------------------------------------------------------------
//Procedure		crossproduct
//Author		Paul.   
//Date			Tue 17 Oct 1995
//
//Description	
//
//Inputs		
//
//Returns		TRUE for clockwise, FALSE for anti-clockwise
//
//------------------------------------------------------------------------------
Bool matrix::crossproduct(DoPointStruc &v1,DoPointStruc &v2,DoPointStruc &v3)
{
	Float	y[3],x[3];

	x[0]=v1.bodyx.f/v1.bodyz.f;
	y[0]=v1.bodyy.f/v1.bodyz.f;

	x[1]=v2.bodyx.f/v2.bodyz.f;
	y[1]=v2.bodyy.f/v2.bodyz.f;

	x[2]=v3.bodyx.f/v3.bodyz.f;
	y[2]=v3.bodyy.f/v3.bodyz.f;

	Float y2y1=y[1]-y[0];
	Float x2x3=x[1]-x[2];

	Float temp=y2y1*x2x3;

	Float x2x1=x[1]-x[0];
	Float y2y3=y[1]-y[2];

	Float temp2=x2x1*y2y3;

	temp-=temp2;

	if (temp<=0.)
		return TRUE;

	return FALSE;
}


//------------------------------------------------------------------------------
//Procedure		accuratecrossproduct
//Author		Paul.   
//Date			Wed 27 Nov 1996
//
//Description	Cross product test using body coordinates (not screen
//				coordinates). This means it can be used before zclipping
//				and should be more accurate.
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
Bool matrix::accuratecrossproduct(DoPointStruc &v1,DoPointStruc &v2,DoPointStruc &v3)
{
	//FIX ME!!!
	return(FALSE);

}

//------------------------------------------------------------------------------
//Procedure		TransformAngles
//Author		Paul.   
//Date			Wed 3 Jul 1996
//
//Description	Given a set of angular deltas in an items coordinate system
//				determine the resultant angles in the world coordinate system
//
//Inputs		hdg, pitch, roll = items current h,p,r
//				deltahdg, deltapitch, deltaroll = requested angular changes
//						in the item coordinate system
//Returns		deltahdg, deltapitch, deltaroll = new values for
//
//------------------------------------------------------------------------------
void matrix::TransformAngles(	ANGLES thdg, ANGLES tpitch, ANGLES troll,
								ANGLES& dhdg, ANGLES& dpitch, ANGLES& droll)
{
	MATRIX	tma,tmb,tmc;										//PD 25Jul96

	MATRIX_PTR	mNegHdg =&tma;									//PD 25Jul96
	MATRIX_PTR	mTemp	=&tmb;									//PD 25Jul96
	MATRIX_PTR	mDelta 	=&tmc;									//PD 25Jul96

	ANGLES	a,b,c;
	ANGLES	cdhdg,cdpitch,cdroll;
	SLong	x, y, z;

	a = dpitch; b = dhdg; c = droll;

	generate2(b,a,c,mDelta);			//PD 25Jul96
	generate2(thdg,tpitch,troll,mTemp);							//PD 25Jul96
	multiply(mDelta,mTemp);

	x = 0; y = 0; z = 0x7FFF;

	rotate(mDelta,x,y,z);

	cdhdg = Math_Lib.HighArcTan((SWord )x,(SWord )z);

	generateh((Angles)(0-cdhdg), mNegHdg);
	rotate(mNegHdg,x,y,z);

	cdpitch = Math_Lib.HighArcTan((SWord )y,(SWord )z);

	x = 0; y = 0x7FFF; z = 0;

	rotate(mDelta,x,y,z);
	rotate(mNegHdg,x,y,z);
	generatep((Angles)(0-cdpitch), mTemp);
	rotate(mTemp,x,y,z);

	cdroll = Math_Lib.HighArcTan((SWord )x,(SWord )y);

	dpitch = cdpitch; dhdg = cdhdg; droll = cdroll;
}

//------------------------------------------------------------------------------
//Procedure		TransformAngles
//Author		Paul.   
//Date			Wed 3 Jul 1996
//
//Description	Given a set of angular deltas in an items coordinate system
//				determine the resultant angles in the world coordinate system
//
//Inputs		hdg, pitch, roll = items current h,p,r
//				deltahdg, deltapitch, deltaroll = requested angular changes
//						in the item coordinate system
//Returns		deltahdg, deltapitch, deltaroll = new values for
//
//------------------------------------------------------------------------------
void matrix::ViewAnglesTransform(	ANGLES thdg, ANGLES tpitch, ANGLES troll,
									ANGLES reqhdg, ANGLES reqpitch,
									ANGLES& dhdg, ANGLES& dpitch, ANGLES& droll)
{
	MATRIX	tma,tmb,tmc;										//PD 25Jul96

	MATRIX_PTR	mNegHdg =&tma;									//PD 25Jul96
	MATRIX_PTR	mTemp	=&tmb;									//PD 25Jul96
	MATRIX_PTR	mDelta 	=&tmc;									//PD 25Jul96

	ANGLES	a,b,c;
	ANGLES	cdhdg,cdpitch,cdroll;
	SLong	x, y, z;

	a = dpitch; b = dhdg; c = droll;

	generate2(b,a,c,mDelta);			//PD 25Jul96
	generate2(thdg,tpitch,troll,mTemp);							//PD 25Jul96
	multiply(mDelta,mTemp);

	cdhdg = reqhdg;
	cdpitch = reqpitch;

	x = 0; y = 0x7FFF; z = 0;

	generateh((Angles)(0-cdhdg), mNegHdg);
	rotate(mDelta,x,y,z);
	rotate(mNegHdg,x,y,z);
	generatep((Angles)(0-cdpitch), mTemp);
	rotate(mTemp,x,y,z);
	cdroll = Math_Lib.HighArcTan((SWord )x,(SWord )y);
	dhdg = reqhdg;
	dpitch = reqpitch;
	droll = cdroll;
}

//------------------------------------------------------------------------------
//Procedure		TransformAnglesB
//Author		Paul.   
//Date			Thu 4 Jul 1996
//
//Description	
//
//Inputs		thdg, tpitch, troll = items current hdg,pitch,roll
//				dhdg, dpitch  		= requested new hdg, pitch in world
//									  coordinate system
//
//Returns		
//
//------------------------------------------------------------------------------
void matrix::TransformAnglesB(	SLong	x, SLong y, SLong z,
							  	ANGLES& itemhdg,
								ANGLES& itempitch,
								ANGLES& itemroll,
								ANGLES&	relhdg,
								ANGLES&	relpitch)
{
	ANGLES	cdhdg,cdpitch,cdroll;

	MATRIX	tma,tmb,tmc,tmd,tme;								//PD 03Sep96

	MATRIX_PTR	mHdg,mPitch,mRoll,mNegHdg,mNegPitch;			//PD 03Sep96

	mHdg = &tma; mPitch = &tmb; mRoll = &tmc; mNegHdg = &tmd;

	mNegPitch = &tme;											//PD 03Sep96

	generateh((Angles )(0-itemhdg),mHdg);
	generatep((Angles )(0-itempitch),mPitch);
	generater((Angles )(0-itemroll),mRoll);

	(void )scaleto16bit(x,y,z);

	rotate(mRoll, x,y,z);
	rotate(mPitch, x,y,z);
	rotate(mHdg, x,y,z);

	x>>=1; y>>=1; z>>=1;										//PD 03Sep96

	cdhdg = Math_Lib.HighArcTan((SWord )x,(SWord )z);

	generateh((Angles )(0-cdhdg),mNegHdg);
	rotate(mNegHdg, x,y,z);

	x>>=1; y>>=1; z>>=1;										//PD 03Sep96

	cdpitch = Math_Lib.HighArcTan((SWord )y,(SWord )z);

	generatep((Angles )(0-cdpitch),mNegPitch);						//PD 03Sep96
	rotate(mNegPitch,x,y,z);									//PD 03Sep96
	x>>=1; y>>=1; z>>=1;										//PD 03Sep96
	cdroll = Math_Lib.HighArcTan((SWord )x,(SWord )y);			//PD 03Sep96

	relhdg = cdhdg; relpitch = cdpitch;

	TransformAngles(itemhdg,itempitch,itemroll,
					cdhdg,cdpitch,cdroll);

	itemhdg 	= cdhdg;
	itempitch 	= cdpitch;
	itemroll 	= cdroll;
}

extern SWord winmode_w,winmode_h;

//------------------------------------------------------------------------------
//Procedure		SetZScale
//Author		Paul.   
//Date			Thu 25 Jul 1996
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::SetZScale(MigWindow *screen)
{
	fpzscale=(Float)FULLW/2;
	fpxscale=Float(winmode_w>>1);
	fpyscale=Float(winmode_h>>1);
}

//------------------------------------------------------------------------------
//Procedure		generate
//Author		Paul.   
//Date			Mon 6 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::generate(ANGLES heading, ANGLES pitch, ANGLES roll, FPMATRIX_PTR matrix)
{
	Float	sh,sp,sr,ch,cp,cr;

	fpSin_Cos(heading,	sh,	ch);
	fpSin_Cos(pitch,	sp,	cp);
	fpSin_Cos(roll,		sr,	cr);

	Float	sr_sh,sr_ch,cr_sh,cr_ch;

	sr_sh = sr * sh;
	sr_ch = sr * ch;
	cr_sh = cr * sh;
	cr_ch = cr * ch;

	matrix->L11 = cr_ch - (sp * sr_sh);
	matrix->L21 = -(sr_ch + (sp * cr_sh));
	matrix->L31 = -(cp * sh);

	matrix->L12 = sr * cp;
	matrix->L22 = cp * cr;
	matrix->L32 = -sp;

	matrix->L13 = (sp * sr_ch) + cr_sh;
	matrix->L23 = (sp * cr_ch) - sr_sh;
	matrix->L33 = cp * ch;
}

//------------------------------------------------------------------------------
//Procedure		generate2
//Author		Paul.   
//Date			Mon 6 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::generate2(ANGLES heading, ANGLES pitch, ANGLES roll, FPMATRIX_PTR matrix)
{
	Float	sh,sp,sr,ch,cp,cr;

	Float	sp_sh,sr_ch,sp_ch,ch_cr;

	fpSin_Cos(heading,	sh,	ch);
	fpSin_Cos(pitch,	sp,	cp);
	fpSin_Cos(roll,		sr,	cr);

	sp_sh = sp * sh;
	sr_ch = sr * ch;
	sp_ch = sp * ch;
	ch_cr = ch * cr;

	matrix->L11 = ch_cr + (sp_sh * sr);
	matrix->L12 = sr_ch - (sp_sh * cr);
	matrix->L13 = cp * sh;

	matrix->L21 = -(sr *cp);
	matrix->L22 = cp * cr;
	matrix->L23 = sp;

	matrix->L31 = (sp * sr_ch) - (sh * cr);
	matrix->L32 = -((cr * sp_ch) + (sh * sr));
	matrix->L33 = cp * ch;
}

//------------------------------------------------------------------------------
//Procedure		inverse
//Author		Paul.   
//Date			Mon 6 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::inverse(ANGLES heading, ANGLES pitch, ANGLES roll, FPMATRIX_PTR matrix)
{
	Float	sh,sp,sr,ch,cp,cr;

	Float	cr_ch,sr_sp,cr_sp,temp;

	//Get the sines/cosines of these angles

	fpSin_Cos(heading,	sh,	ch);
	fpSin_Cos(pitch,	sp,	cp);
	fpSin_Cos(roll,		sr,	cr);

	//Generate some intermediate values

	cr_ch = cr * ch;
	sr_sp = sr * sp;
	cr_sp = cr * sp;

	//Fill in the new matrix

	matrix->L11	=	cr_ch - (sr_sp * sh);
	matrix->L21	=	sr * cp;
	matrix->L31	=	(sr_sp * ch) + (cr * sh);

	matrix->L12	=	-(sr * ch) - (cr_sp * sh);
	matrix->L22	=	cr * cp;
	matrix->L32	=	(cr_ch * sp) - (sr * sh);

	matrix->L13	=	-(cp * sh);
	matrix->L23	=	-sp;
	matrix->L33	=	cp * ch;
}

//------------------------------------------------------------------------------
//Procedure		getxdeltas
//Author		Paul.   
//Date			Mon 6 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::getxdeltas(FPMATRIX_PTR T, IFShare& x, IFShare& y, IFShare& z)
{
	x.f = T->L11;
	y.f = T->L21;
	z.f = T->L31;
}

//------------------------------------------------------------------------------
//Procedure		getzdeltas
//Author		Paul.   
//Date			Mon 6 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::getzdeltas(FPMATRIX_PTR T, IFShare& x, IFShare& y, IFShare& z)
{
	x.f = T->L13;
	y.f = T->L23;
	z.f = T->L33;
}

//------------------------------------------------------------------------------
//Procedure		transform
//Author		Paul.   
//Date			Mon 6 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
UWord matrix::transform(FPMATRIX_PTR T, IFShare& x, IFShare& y, IFShare& z)
{
	Float	fx = (Float)x.i,
			fy = (Float)y.i,
			fz = (Float)z.i;

	x.f = (T->L11 * fx) + (T->L12 * fy) + (T->L13 * fz);
	y.f = (T->L21 * fx) + (T->L22 * fy) + (T->L23 * fz);
	z.f = (T->L31 * fx) + (T->L32 * fy) + (T->L33 * fz);

	//Now generates clip flags based 
	//on the current view cone

	UWord retval;
	_clipFB(retval,z.f,1.,fpMaximumZ);
	_clipLR(retval,x.f,z.f);
	_clipTB(retval,y.f,z.f);
	return retval;
}

//------------------------------------------------------------------------------
//Procedure		transformNC
//Author		Paul.   
//Date			Fri 24 Apr 1998
//------------------------------------------------------------------------------
UWord matrix::transformNC(FPMATRIX_PTR T, IFShare& x, IFShare& y, IFShare& z)
{
	Float	fx = (Float)x.i,
			fy = (Float)y.i,
			fz = (Float)z.i;

	x.f = (T->L11 * fx);
	x.f += (T->L12 * fy);
	x.f += (T->L13 * fz);

	y.f = (T->L21 * fx);
	y.f += (T->L22 * fy);
	y.f += (T->L23 * fz);

	z.f = (T->L31 * fx);
	z.f += (T->L32 * fy);
	z.f += (T->L33 * fz);

	return CF3D_NULL;
}

//------------------------------------------------------------------------------
//Procedure		transform
//Author		Jim Taylor
//Date			Wed 4 Mar 1998
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::transform(FPMATRIX_PTR T, DoPointStruc& dp)
{
	Float	fx = Float(dp.bodyx.i),
			fy = Float(dp.bodyy.i),
			fz = Float(dp.bodyz.i);
	dp.bodyx.f = (T->L11 * fx) + (T->L12 * fy) + (T->L13 * fz);
	dp.bodyy.f = (T->L21 * fx) + (T->L22 * fy) + (T->L23 * fz);
	dp.bodyz.f = (T->L31 * fx) + (T->L32 * fy) + (T->L33 * fz);

	//Now generates clip flags based 
	//on the current view cone

	SetClipFlags(dp);
}


//------------------------------------------------------------------------------
//Procedure		transform
//Author		Paul.   
//Date			Mon 6 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
UWord matrix::fptrans(FPMATRIX_PTR T, IFShare& x, IFShare& y, IFShare& z)
{
	Float	fx = x.f,
			fy = y.f,
			fz = z.f;
	x.f = (T->L11 * fx) + (T->L12 * fy) + (T->L13 * fz);
	y.f = (T->L21 * fx) + (T->L22 * fy) + (T->L23 * fz);
	z.f = (T->L31 * fx) + (T->L32 * fy) + (T->L33 * fz);

	//Now generates clip flags based 
	//on the current view cone

	UWord retval;
	_clipFB(retval,z.f,1.,fpMaximumZ);

	_clipLR(retval,x.f,z.f);
	_clipTB(retval,y.f,z.f);
	return retval;
}

//------------------------------------------------------------------------------
//Procedure		transform_y
//Author		Paul.   
//Date			Mon 6 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::transform_y(FPMATRIX_PTR T, SLong y,
						IFShare& rtx, IFShare& rty, IFShare& rtz)
{
	Float	fy = (Float)y;
	rtx.f = fy * T->L12;
	rty.f = fy * T->L22;
	rtz.f = fy * T->L32;
}

//------------------------------------------------------------------------------
//Procedure		multiply
//Author		Paul.   
//Date			Mon 6 Jan 1997
//
//Description	
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::multiply(FPMATRIX_PTR t, FPMATRIX_PTR sip)
{
	FPMATRIX		tmat;
	FPMATRIX_PTR	dip = &tmat;

	Float	*si,*di;

	int	j;

	si = &sip->L11;
	di = &dip->L11;

	for(j=0;j<3;j++)
	{
		*di++	= (*si * t->L11)
				+ (*(si+1) * t->L21)
				+ (*(si+2) * t->L31);

		*di++ 	= (*si * t->L12)
				+ (*(si+1) * t->L22)
				+ (*(si+2) * t->L32);

		*di++ 	= (*si * t->L13)
				+ (*(si+1) * t->L23)
				+ (*(si+2) * t->L33);

		si += 3;
	}

	//copy the new matrix back to sip

	*t = *dip;
}

//------------------------------------------------------------------------------
//Procedure		SetViewParams
//Author		Paul.   
//Date			Mon 2 Feb 1998
//
//Description	
//
//Inputs		view cone angle + distance to back clipping plane in cm	
//
//------------------------------------------------------------------------------
void matrix::SetViewParams(MigWindow* win,ANGLES viewCone,SLong viewRange)
{
	//Clip the view cone to min/max
	if (viewCone<ANGLES_40Deg)		viewCone=ANGLES_40Deg;
	else if (viewCone>ANGLES_90Deg)	viewCone=ANGLES_90Deg;

	viewConeAngle=Angles(UWord(viewCone)>>1);
	fpTan(viewConeAngle,FoV);

	aspectRatio=Float(win->VirtualHeight)/Float(win->VirtualWidth);
	aspectRatio*=FoV;

 	fpMaximumZ=Float(viewRange);

	win->DoSetAspectRatio(aspectRatio,FoV);
}

//------------------------------------------------------------------------------
//Procedure		generateh
//Author		Paul.   
//Date			Mon 24 Aug 1998
//------------------------------------------------------------------------------
void matrix::generateh(ANGLES hdg,FPMATRIX& m)
{
	Float sin_hdg,cos_hdg;
	fpSin_Cos(hdg,sin_hdg,cos_hdg);
	m.L11=m.L33=cos_hdg;
	m.L31=-sin_hdg;
	m.L13=sin_hdg;
	m.L21=m.L12=m.L32=m.L23=0.0;
	m.L22=1.0;
}

//------------------------------------------------------------------------------
//Procedure		generatep
//Author		Paul.   
//Date			Mon 24 Aug 1998
//------------------------------------------------------------------------------
void matrix::generatep(ANGLES pitch, FPMATRIX& m)
{
	Float sin_pitch,cos_pitch;
	pitch=ANGLES_0Deg-pitch;
	fpSin_Cos(pitch,sin_pitch,cos_pitch);
	m.L11=1.0;
	m.L21=m.L31=m.L12=m.L13=0.0;
	m.L22=m.L33=cos_pitch;
	m.L32=sin_pitch;
	m.L23=-sin_pitch;
}

//------------------------------------------------------------------------------
//Procedure		generater
//Author		Paul.   
//Date			Mon 24 Aug 1998
//------------------------------------------------------------------------------
void matrix::generater(ANGLES roll,FPMATRIX& m)
{
	Float sin_roll,cos_roll;
	roll=ANGLES_0Deg-roll;
	fpSin_Cos(roll,sin_roll,cos_roll);
	m.L11=m.L22=cos_roll;
	m.L21=sin_roll;
	m.L12=-sin_roll;
	m.L31=m.L32=m.L13=m.L23=0.0;
	m.L33=1.0;
}

//------------------------------------------------------------------------------
//Procedure		inverseMobileMatrix
//Author		Paul.   
//Date			Mon 24 Aug 1998
//------------------------------------------------------------------------------
void matrix::inverseMobileMatrix(ANGLES hdg,ANGLES pitch,ANGLES roll,FPMATRIX& m)
{
	FPMATRIX h,p,r;

	hdg=ANGLES_0Deg-hdg;
	pitch=ANGLES_0Deg-pitch;
	roll=ANGLES_0Deg-roll;

	generateh(hdg,h);
	generatep(pitch,p);
	generater(roll,r);

	m=h;
	multiply(&m,&p);
	multiply(&m,&r);

}

//------------------------------------------------------------------------------
//Procedure		Generate
//Author		Paul.   
//Date			Mon 12 Oct 1998
//------------------------------------------------------------------------------
void matrix::Generate(ANGLES heading,ANGLES pitch,ANGLES roll,FPMATRIX* matrix)
{
	Float	sh,sp,sr,ch,cp,cr;

	fpSin_Cos(heading,	sh,	ch);
	fpSin_Cos(pitch,	sp,	cp);
	fpSin_Cos(roll,		sr,	cr);

	Float	sr_sh,sr_ch,cr_sh,cr_ch;

	sr_sh = sr * sh;
	sr_ch = sr * ch;
	cr_sh = cr * sh;
	cr_ch = cr * ch;

	matrix->L11 = cr_ch - (sp * sr_sh);
	matrix->L21 = -(sr_ch + (sp * cr_sh));
	matrix->L31 = -(cp * sh);

	matrix->L12 = sr * cp;
	matrix->L22 = cp * cr;
	matrix->L32 = -sp;

	matrix->L13 = (sp * sr_ch) + cr_sh;
	matrix->L23 = (sp * cr_ch) - sr_sh;
	matrix->L33 = cp * ch;

	FPMATRIX viewMat;

	viewMat.L11=1./FoV;
	viewMat.L21=0.;
	viewMat.L31=0.;

	viewMat.L12=0.;
	viewMat.L22=1./aspectRatio;
	viewMat.L32=0.;

	viewMat.L13=0.;
	viewMat.L23=0.;
	viewMat.L33=1.;

	multiply(matrix,&viewMat);
}

void matrix::GenerateV(ANGLES heading,ANGLES pitch,ANGLES roll,FPMATRIX* matrix)
{
	Float	sh,sp,sr,ch,cp,cr;

	fpSin_Cos(heading,	sh,	ch);
	fpSin_Cos(pitch,	sp,	cp);
	fpSin_Cos(roll,		sr,	cr);

	Float	sr_sh,sr_ch,cr_sh,cr_ch;

	sr_sh = sr * sh;
	sr_ch = sr * ch;
	cr_sh = cr * sh;
	cr_ch = cr * ch;

	matrix->L11 = cr_ch - (sp * sr_sh);
	matrix->L21 = -(sr_ch + (sp * cr_sh));
	matrix->L31 = -(cp * sh);

	matrix->L12 = sr * cp;
	matrix->L22 = cp * cr;
	matrix->L32 = -sp;

	matrix->L13 = (sp * sr_ch) + cr_sh;
	matrix->L23 = (sp * cr_ch) - sr_sh;
	matrix->L33 = cp * ch;
}

//------------------------------------------------------------------------------
//Procedure		Generate2
//Author		Paul.   
//Date			Mon 12 Oct 1998
//------------------------------------------------------------------------------
void matrix::Generate2(	ANGLES heading,ANGLES pitch,ANGLES roll,
						Float scale,
						FPMATRIX* mp)
	{
	Float* matrix=(Float*)mp;
	Float sh,sp,sr,ch,cp,cr;
	Float sp_sh,sr_ch,sp_ch,ch_cr;

	fpSin_Cos(heading,sh,ch);
	fpSin_Cos(pitch,sp,cp);
	fpSin_Cos(roll,sr,cr);

	sp_sh=sp*sh;
	sr_ch=sr*ch;
	sp_ch=sp*ch;
	ch_cr=ch*cr;

	Float temp=Float(scale);
	matrix[0]=(ch_cr+(sp_sh*sr))*temp;
	matrix[1]=(sr_ch-(sp_sh*cr))*temp;
	matrix[2]=cp*sh*temp;

	matrix[3]=-sr*cp*temp;
	matrix[4]=cp*cr*temp;
	matrix[5]=sp*temp;

	matrix[6]=((sp*sr_ch)-(sh*cr))*temp;
	matrix[7]=-((cr*sp_ch)+(sh*sr))*temp;
	matrix[8]=cp*ch*temp;
}

//------------------------------------------------------------------------------
//Procedure		GenerateXY
//Author		Robert Slater
//Date			Wed 2 Dec 1998
//
//Description	For spheres and cylinders.......
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::GenerateXY(Float	&xscale,Float	&yscale)
{
	xscale = 1./FoV;
	yscale = 1./aspectRatio;
}

//------------------------------------------------------------------------------
//Procedure		GenerateRadar
//Author		Robert Slater
//Date			Fri 14 May 1999
//
//Description	Generates a 20 degree viewcone for radar locking
//
//Inputs		
//
//Returns	
//
//------------------------------------------------------------------------------
void matrix::GenerateRadar(ANGLES heading,ANGLES pitch,ANGLES roll,FPMATRIX* matrix)
{
	Float	sh,sp,sr,ch,cp,cr;

	fpSin_Cos(-heading,	sh,	ch);
	fpSin_Cos(-pitch,	sp,	cp);
	fpSin_Cos(-roll,		sr,	cr);

	Float	sr_sh,sr_ch,cr_sh,cr_ch;

	sr_sh = sr * sh;
	sr_ch = sr * ch;
	cr_sh = cr * sh;
	cr_ch = cr * ch;

	matrix->L11 = cr_ch - (sp * sr_sh);
	matrix->L21 = -(sr_ch + (sp * cr_sh));
	matrix->L31 = -(cp * sh);

	matrix->L12 = sr * cp;
	matrix->L22 = cp * cr;
	matrix->L32 = -sp;

	matrix->L13 = (sp * sr_ch) + cr_sh;
	matrix->L23 = (sp * cr_ch) - sr_sh;
	matrix->L33 = cp * ch;

	FPMATRIX viewMat;

	Float	MYFoV;

	fpTan(ANGLES_10Deg,MYFoV);

	viewMat.L11=1./MYFoV;
	viewMat.L21=0.;
	viewMat.L31=0.;

	viewMat.L12=0.;
	viewMat.L22=1./MYFoV;
	viewMat.L32=0.;

	viewMat.L13=0.;
	viewMat.L23=0.;
	viewMat.L33=1.;

	multiply(matrix,&viewMat);
}

void matrix::PushViewData()
{
}
void matrix::PopViewData()
{
}
void matrix::Set3DWin(SLong x,SLong y,SLong w,SLong h)
{
}

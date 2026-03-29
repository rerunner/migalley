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

// SqdnLBut.cpp : implementation file
//

#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "MIG.H"
#include "MIGVIEW.H"
#include "MISSSUB.H"
#include "RSTATIC.H" // RERUN for CRStatic
#include "RRADIO.H" // RERUN for CRRadio
#include "RCOMBO.H" // RERUN for CRCombo
#include "RCOMBOX.H" //RERUN
#include "RBUTTON.H" //RERUN for CRButton
#include "RTOOLBAR.H" //RERUN for CRToolBar
#include "REDIT.H" //RERUN for CREdit
#include "MISSMAN2.H"
#include "RIP_LIST.H"
#include "SAVEGAME.H"

#include "SQDNLBUT.H"
#include "MYMATH.H"
#include "MYTIME.H"

/* #ifdef _DEBUG
#define new DEBUG_NEW
#ifndef THIS_FILE_DEFINED
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif */
/////////////////////////////////////////////////////////////////////////////
// CSqdnlistBut dialog


CSqdnlistBut::CSqdnlistBut(int squad,CWnd* pParent /*=NULL*/)
	: RowanDialog(CSqdnlistBut::IDD, pParent)
{
	SetProjSpecific(-1,-1,squad);


	//{{AFX_DATA_INIT(CSqdnlistBut)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSqdnlistBut::DoDataExchange(CDataExchange* pDX)
{

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSqdnlistBut)
	DDX_Control(pDX, IDCX_BINGOFUEL, m_IDCX_BINGOFUEL);
	DDX_Control(pDX,IDC_RSTATICCTRL38,m_IDC_RSTATICCTRL38);
	DDX_Control(pDX,IDC_PILOTLOSSES,m_IDC_PILOTLOSSES);
	DDX_Control(pDX,IDC_ROTATEFREQ,m_IDC_ROTATEFREQ);
	DDX_Control(pDX,IDC_RSTATICCTRL39,m_IDC_RSTATICCTRL39);
	DDX_Control(pDX,IDC_RSTATICCTRL60,m_IDC_RSTATICCTRL60);
	DDX_Control(pDX,IDC_AVAILACNUM,m_IDC_AVAILACNUM);
	DDX_Control(pDX,IDC_BINGOFUEL,m_IDC_BINGOFUEL);
	//}}AFX_DATA_MAP


	

}


BEGIN_MESSAGE_MAP(CSqdnlistBut, CDialog)
	//{{AFX_MSG_MAP(CSqdnlistBut)
	ON_EN_UPDATE(IDCX_BINGOFUEL, OnUpdateBingofuel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSqdnlistBut message handlers

BEGIN_EVENTSINK_MAP(CSqdnlistBut, CDialog)
    //{{AFX_EVENTSINK_MAP(CSqdnlistBut)
	ON_EVENT(CSqdnlistBut, IDC_PILOTLOSSES, 1 /* Clicked */, OnClickedPilotlosses, VTS_NONE)
	ON_EVENT(CSqdnlistBut, IDC_ROTATEFREQ, 1 /* TextChanged */, OnTextChangedRotatefreq, VTS_BSTR)
	ON_EVENT(CSqdnlistBut, IDC_BINGOFUEL, 2 /* TextChanged */, OnTextChangedBingofuel, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


void CSqdnlistBut::OnClickedPilotlosses()
{
    MMC.currdate = MMC.currdate + SECSPERDAY;

    if (LoggedChild(1))
        CloseLoggedChildren();

    CRect r = Place(POSN_CENTRE, POSN_CENTRE, 0, 0);
    DialBox box(FIL_MAP_PILOTLOSSES, new CRIP_List);

    RDialog* d = MakeTopDialog(r, box);

    LogChild(1, d);
}

void CSqdnlistBut::OnTextChangedRotatefreq(LPCTSTR text) 
{
 	CRCombo* combo;
	combo=GETDLGITEM(IDC_ROTATEFREQ);
	MMC.squad[groupnum].rotatefreq = 
				combo->GetIndex();
}


BOOL CSqdnlistBut::OnInitDialog() 
{
	CDialog::OnInitDialog();

	
	char buffer[10];

  	CRCombo* combo;
	combo=GETDLGITEM(IDC_ROTATEFREQ);
	combo->RESCOMBO(ROTATE0,6)->SetIndex(MMC.squad[groupnum].rotatefreq);


	CRStatic*   s;
	s=GETDLGITEM(IDC_AVAILACNUM);
	std::snprintf(buffer, sizeof(buffer), "%d", MMC.squad[groupnum].ac_total_avail);

	s->SetString(buffer);

	s=GETDLGITEM(IDC_RSTATICCTRL60);

	CString txt =
		LoadResString(IDS_BINGOFUEL) + ", " +
		LoadResString(Save_Data.mass.abbrtype);

	s->SetString(txt);

	int fuel = MMC.squad[groupnum].bingofuel/(100*Save_Data.mass.gm);
	if (fuel>200)
		fuel = ((fuel + 50)/100)*100;
	if (m_IDCX_BINGOFUEL.GetHwnd()) //RERUN if (m_IDCX_BINGOFUEL)
	{
		m_IDCX_BINGOFUEL.SetWindowText(CSprintf("%4i",fuel));
		m_IDCX_BINGOFUEL.SetLimitText(5);
	}
	else
	{
		CREdit* b;
		b=GETDLGITEM(IDC_BINGOFUEL);
		b->SetCaption(CSprintf("%4i",fuel));
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSqdnlistBut::OnTextChangedBingofuel(LPTSTR text) 
{
		CREdit* edit;
///		CString	buffer;
		edit=GETDLGITEM(IDC_BINGOFUEL);
///		buffer = edit->GetCaption();

		trunc(text,4);
		if  (	(strlen(text) > 0 )	&&	(strlen(text) < 5 )	)
		{
			int fuel = atoi(text);
			fuel = fuel * (Save_Data.mass.gm*100);
			if (fuel < 0)
				fuel = 0;
			MMC.squad[groupnum].bingofuel = fuel;
		}
	
}

void CSqdnlistBut::OnUpdateBingofuel() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.
	
	// TODO: Add your control notification handler code here
	CString text;
	GETDLGITEM(IDCX_BINGOFUEL)->GetWindowText(text);
	int fuel = atoi(text);
	fuel = fuel * (Save_Data.mass.gm*100);
	if (fuel < 0)
		fuel = 0;
	MMC.squad[groupnum].bingofuel = fuel;
}

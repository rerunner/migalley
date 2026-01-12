#if !defined(AFX_MIGVIEW_H__DB0342A1_1B07_11D1_A1F0_444553540000__INCLUDED_)
#define AFX_MIGVIEW_H__DB0342A1_1B07_11D1_A1F0_444553540000__INCLUDED_

#include "DOSDEFS.H"
//RERUN #include "MIGDoc.h"
//RERUN #include "MapDlg.h"	// Added by ClassView
#include "FULLPANE.H"
//RERUN #include "rscrlbar.h"
#include "GLOBDEFS.H"
#include "UNIQUEID.H" //RERUN

class	ItemBase;
typedef	ItemBase* ItemBasePtr;
class	CScaleBar;

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

const int AIRSTRIPS = 4;
const int BRIDGES = 2;
//#include "CustBB.h"
#define ZOOMMIN 0.25
#define ZOOMMAX	50
//32
#define ZOOMTHRESHOLD1 0
#define ZOOMTHRESHOLD2 1
#define ZOOMTHRESHOLD3 4
#define ZOOMTHRESHOLDFRONTLINE 4
#define ZOOMTHRESHOLDDETAIL 16



#define currsheetname CMIGView

#define CreateWindowWithParams (childname,(paramlist)) \
static void rv1_##childname(int retcode); \
\
void currsheetname::On##childname() \
{ \
	if (##childname.m_hWnd == NULL) \
	/* Checks to see if window does not exist */\
		##childname.Init(C##childname::IDD,this,rv1_##childname,##paramlist); \
	/* if not create it */ \
	else \
		##childname.SetFocus(); \
	/* if it exists set the focus to it */ \
} \
\
static void rv2_##childname(int retcode); \
static void rv1_##childname(int retcode) \
{ \
	rv2_##childname(retcode); \
	##childname->DestroyWindow(); \
} \
static void rv2_##childname(int retcode)

//class CResizableDlgBar;
//RERUN class RDialog;
//RERUN class RFullPanelDial;
//RERUN class CRCustomButton;
//RERUN class	UniqueIDField;
//RERUN class CThumbnail;
class CMIGView //RERUN  : public CView
{

protected: // create from serialization only
	CMIGView();
//RERUN	DECLARE_DYNCREATE(CMIGView)

// Attributes
	//RERUN public:
	//RERUN CMIGDoc* GetDocument();

// Operations
public:

// Overrides
#if 0 //RERUN
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMIGView)
	public:
 	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual CScrollBar* GetScrollBarCtrl(int nBar) const;
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnBeginPrinting(HDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(HDC* pDC, CPrintInfo* pInfo);
	virtual void OnInitialUpdate( );
	//}}AFX_VIRTUAL
//
#endif //RERUN

// Implementation
public:
	void ScreenXY(ItemBasePtr p,int&px,int&py,int scrollx=0,int scrolly=0);
	void ScreenXY(int wx,int wz,int&px,int&py,int scrollx=0,int scrolly=0);
	int	WorldXY(int& wx,int& wz,int sx,int sy);	//returns icon radius
	int	WorldXY(Coords3D& w,int sx,int sy);	//returns icon radius
	BOOL IsIconVisible(ItemBasePtr id, int&px, int&py);
	void	DrawIcon(HDC hDC,int px,int py,FileNum f);
	void	DrawIcon(HDC hDC,int px,int py,BYTE* pData,BITMAPINFO* pInfo);
//RERUN	void	DrawIconText(CDC* pDC,int px,int py,CString name);

	void ZoomToUID(UniqueID uid,double zoom);
	void LaunchFullPane(FullScreen* startscreen);
	void UpdateAll();
	void LaunchMap(bool firsttime);
	void OnChangeToTitle();
//RERUN	CRScrlBar m_rVertScrollBar;
//RERUN	CRScrlBar m_rHorzScrollBar;
	void SetHiLightInfo(int	pack, int wave = 0, int	group = 0,
						UniqueID	wp=UID_NULL,bool fullredraw=false);

//RERUN	CScaleBar* m_pScaleBar;
	int m_currentpage;
	RFullPanelDial* m_pfullpane;
	BOOL m_drawing;
	BOOL m_LButtonDown;
	int m_doIexist;
	void UpdateScrollbars();
//RERUN	CPoint m_scrollpoint;
	int m_oldstarty;
	int m_oldstartx;
	int m_oldendy;
	int m_oldendx;
	int m_iconradius;
	BOOL m_isPaletteCreated;
	HDC m_hDC;
	fileblock* m_pfileblock;
//RERUN	void UpdateBitmaps(CDC* pDC);
//RERUN	void PlotRoutes(CDC* pDC,UniqueID onlythrough=UID_NULL);
	FileNum DrawIconTest(ItemBasePtr p, UniqueID i, bool hilight,UniqueIDField& redUIDs, FileNum* FL_Icon); 
protected:
	void InvalidateJapan();
//RERUN	void PlotRoute(CDC* pDC, int pack, int wave, int group, bool drawhilight,UniqueID onlythrough=UID_NULL);
//RERUN	void PlotFrontLine(CDC* pDC);
//RERUN	void PlotLine(CDC* pDC, UniqueID thiswp,UniqueID nextwp, BOOL japanwp=FALSE);
	void DrawIcons(HDC hDC, CDC* pDC,UniqueIDField& redUIDs, FileNum* FL_Icon);
//RERUN	void PlotMainRoute(CDC* pDC,SupplyLine* SL, int red, int green, int blue);
//RERUN	void PlotDebugInfo(CDC* pDC);
//RERUN	void PlotAirfields(CDC* pDC);
//RERUN	void PlotSupplyingRoutes(CDC* pDC);
//RERUN	void DrawLine(CDC* pDC, int wx,int wz,int &px1, int &py1,int &px2,int &py2, 
//RERUN		int scrollx,int scrolly, int red, int green, int blue, int width);
public:
	bool m_bridgesChecked;
	bool m_airstripsChecked;
//	bool m_bDontUpdateBitmaps;
	void UpdateBridges();
	void UpdateAirStrips();
	float m_zoom;
	float m_oldzoom;
	CPoint m_popupPoint;
	void Zoom(CPoint screenpoint,bool redrawscalebar=true);
//	CSize m_sizei;
	struct CSizeF
	{
		float cx,cy;
	}	m_size;
//RERUN	CMapDlg					m_mapdlg;
	static const FileNum m_MapFiles[32][20];
//RERUN	static CThumbnail* m_thumbnail;
	virtual ~CMIGView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
#if 0 //RERUN
public:
	afx_msg void OnZoomIn();
	afx_msg void OnZoomOut();
protected:
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	//{{AFX_MSG(CMIGView)
	afx_msg void OnShowAirStrips();
	afx_msg void OnShowBridges();
	afx_msg void OnPopupZoomIn();
	afx_msg void OnPopupZoomOut();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnScrollHorzScrollbar(long nPos);
	afx_msg void OnScrollVertScrollbar(long nPos);
	afx_msg void OnMove(int x, int y);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	afx_msg BYTE* OnGetFile(int filenum);
	afx_msg void OnReleaseLastFile();
	afx_msg CDC* OnGetOffScreenDC();
	MSG2_1(OnGetFile);
	MSG2_0v(OnReleaseLastFile);
	MSG2_0(OnGetOffScreenDC);
	DECLARE_MESSAGE_MAP()
#endif //RERUN
};

#ifndef _DEBUG  // debug version in MIGView.cpp
inline CMIGDoc* CMIGView::GetDocument()
   { return (CMIGDoc*)m_pDocument; }
#endif
inline	RFullPanelDial*	GetFullPanel(CMIGView* view)
{	return	view->m_pfullpane;	}


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MIGVIEW_H__DB0342A1_1B07_11D1_A1F0_444553540000__INCLUDED_)

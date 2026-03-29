#ifndef redtbt_INCLUDED
#pragma once
#define redtbt_INCLUDED
// RERUN: Replaced forwarding include with full definition to support OnPaint override
#include "MFC_stub.h"
#include "DLGITEM.H"

class CREdtBt : public DlgItem
{
private:
    bool m_bDepressed = false;
    OLE_COLOR m_foreColor = 0;
    long m_fontNum = 0;
    long m_pictureFileNum = 0;
    long m_dragDropID = 0;
protected:
	DECLARE_DYNCREATE(CREdtBt)
public:
    CREdtBt();
	CLSID const& GetClsid();
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle,
						const RECT& rect, CWnd* pParentWnd, UINT nID, 
						CCreateContext* pContext = NULL)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID); }

    BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle,
						const RECT& rect, CWnd* pParentWnd, UINT nID,
						CFile* pPersist = NULL, BOOL bStorage = FALSE,
						BSTR bstrLicKey = NULL)
	{ return CreateControl(GetClsid(), lpszWindowName, dwStyle, rect, pParentWnd, nID,
						pPersist, bStorage, bstrLicKey); }

// Attributes
public:
	OLE_COLOR GetForeColor();
	void SetForeColor(OLE_COLOR);
	CString GetCaption();
	void SetCaption(LPCTSTR);
	long GetFontNum();
	void SetFontNum(long);
	long GetDragAndDropID();
	void SetDragAndDropID(long);
	void SetPictureFileNum(long picture);
    virtual void OnPaint(); // RERUN: Added override
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnDestroy();
    DECLARE_MESSAGE_MAP()
};
#endif

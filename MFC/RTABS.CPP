/*
	 MiG Alley
	 Copyright (C) 1998, 1999, 2000, 2001 Empire Interactive (Europe) Ltd,
	 677 High Road, North Finchley, London N12 0DA
*/

#include "WIN32_COMPAT.H"
#include "MFC_stub.h"
#include "DLGITEM.H"
#include "RDIALOG.H"
#include "MIG.H"

#include "RTABS.H"
#include "RDIALOG.H"
#include "MIGVIEW.H"
#include <vector>
#include <string>
#include <map>
#include <iostream>

// Internal storage for tab data
struct TabEntry {
    std::string text;
    long data;
    CRect rect;
};

struct TabsData {
    std::vector<TabEntry> tabs;
    bool horz = true;
    long firstTab = 0;
    int currentSelection = 0;
    OLE_COLOR foreColor = RGB(0,0,0);
};

static std::map<CWnd*, TabsData> g_tabsData;

static TabsData& GetData(CWnd* wnd) {
    return g_tabsData[wnd];
}

IMPLEMENT_DYNCREATE(CRTabs, CWnd)

BEGIN_MESSAGE_MAP(CRTabs, CWnd)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

CRTabs::CRTabs() {}
CRTabs::~CRTabs() {
    g_tabsData.erase(this);
}

void CRTabs::SetHorzAlign(BOOL bHorz) {
    GetData(this).horz = bHorz;
}

BOOL CRTabs::GetHorzAlign() {
    return GetData(this).horz;
}

void CRTabs::AddTab(LPCTSTR text, long data) {
    TabEntry e;
    e.text = text ? text : "";
    e.data = data;
    GetData(this).tabs.push_back(e);
    Invalidate(); // RERUN: Ensure we redraw when a tab is added
}

long CRTabs::GetFirstTab() {
    return GetData(this).firstTab;
}

void CRTabs::SetFirstTab(long data) {
    GetData(this).firstTab = data;
}

long CRTabs::CalculateHeight(int width) {
    return 25; // Fixed height for horizontal tabs
}

long CRTabs::CalcWidestWord() {
    return 100; // Fixed width for vertical tabs
}

long CRTabs::SelectTab(int index) {
    TabsData& data = GetData(this);
    long rv = 0;
    if (index >= 0 && index < (int)data.tabs.size()) {
        data.currentSelection = index;
        
        // Manage visibility of child dialogs (which are passed as 'data')
        for (int i=0; i<(int)data.tabs.size(); ++i) {
            CWnd* w = (CWnd*)data.tabs[i].data;
            if (w && w->m_hWnd) {
                if (i == index)
                    w->ShowWindow(SW_SHOW);
                else
                    w->ShowWindow(SW_HIDE);
            }
        }
        Invalidate();
        rv = data.tabs[index].data;
    }
    return rv;
}

void CRTabs::SetForeColor(OLE_COLOR color) {
    GetData(this).foreColor = color;
}

void CRTabs::SetFont(CFont* pFont) {
    // Stored but currently unused in this simple implementation
}

void CRTabs::OnPaint() {
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);

    // std::cout << "[CRTabs] OnPaint: " << GetData(this).tabs.size() << " tabs. Rect: " << rect.left << "," << rect.top << "," << rect.right << "," << rect.bottom << std::endl;
    
    // Draw background
    dc.FillSolidRect(rect, RGB(192, 192, 192));
    
    TabsData& data = GetData(this);
    
    CFont* pOldFont = nullptr;
    CWnd* pParent = GetParent();
    if (pParent) {
        CFont* pGlobalFont = (CFont*)pParent->SendMessage(WM_GETGLOBALFONT, 0); 
        if (pGlobalFont) pOldFont = dc.SelectObject(pGlobalFont);
    }

    int x = 0;
    int y = 0;
    
    for (int i = 0; i < (int)data.tabs.size(); ++i) {
        TabEntry& tab = data.tabs[i];
        CSize sz = dc.GetTextExtent(tab.text.c_str());
        
        // std::cout << "  Tab " << i << ": '" << tab.text << "' Size=" << sz.cx << "x" << sz.cy << std::endl;

        CRect tabRect;
        if (data.horz) {
            int w = sz.cx + 20;
            tabRect = CRect(x, 0, x + w, rect.Height());
            x += w;
        } else {
            int h = sz.cy + 10;
            tabRect = CRect(0, y, rect.Width(), y + h);
            y += h;
        }
        tab.rect = tabRect;

        // Draw tab
        if (i == data.currentSelection) {
            dc.FillSolidRect(tabRect, RGB(255, 255, 255));
            dc.Draw3dRect(tabRect, RGB(0,0,0), RGB(128,128,128));
        } else {
            dc.FillSolidRect(tabRect, RGB(160, 160, 160));
            dc.Draw3dRect(tabRect, RGB(255,255,255), RGB(0,0,0));
        }
        
        dc.SetBkMode(TRANSPARENT);
        dc.SetTextColor(data.foreColor);
        dc.DrawText(tab.text.c_str(), tabRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }

    if (pOldFont) dc.SelectObject(pOldFont);
}

void CRTabs::OnLButtonDown(UINT nFlags, CPoint point) {
    TabsData& data = GetData(this);
    for (int i = 0; i < (int)data.tabs.size(); ++i) {
        if (data.tabs[i].rect.PtInRect(point)) {
            SelectTab(i);
            
            // Fire event to parent
            CWnd* pParent = GetParent();
            if (pParent) {
                const AFX_EVENTSINKMAP* pMap = pParent->GetSinkMap();
                if (pMap) {
                    for (; pMap->idFirst != 0; ++pMap) {
                        if (m_nID >= pMap->idFirst && m_nID <= pMap->idLast && pMap->eventid == 1) {
                            typedef void (CCmdTarget::*SelectFunc)(long);
                            SelectFunc f = (SelectFunc)pMap->pfn;
                            (pParent->*f)(data.tabs[i].data);
                            break;
                        }
                    }
                }
            }
            break;
        }
    }
}
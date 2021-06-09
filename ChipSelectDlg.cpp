// CScaleEditorDlg.cpp : implementation file
//


#include "stdafx.h"
//#include "afxdialogex.h"
#include <cstdlib>
#include <cmath>

#include "FamiTracker.h"
#include "FamiTrackerDoc.h"
#include "MainFrm.h"
#include "ChipSelectDlg.h"


// CScaleEditorDlg dialog

IMPLEMENT_DYNAMIC(CChipSelectDlg, CDialog)

CChipSelectDlg::CChipSelectDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CChipSelectDlg::IDD, pParent), m_pDocument(NULL)
{

}

CChipSelectDlg::~CChipSelectDlg()
{
}

void CChipSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CChipSelectDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_VSCROLL()
	
END_MESSAGE_MAP()

// CScaleEditorDlg message handlers

BOOL CChipSelectDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Get active document
	//CFrameWnd* pFrameWnd = static_cast<CFrameWnd*>(GetParent());
	//m_pDocument = static_cast<CFamiTrackerDoc*>(pFrameWnd->GetActiveDocument());

	RECT rcClient, rcWind;
	POINT ptDiff;
	GetClientRect(&rcClient);
	GetWindowRect(&rcWind);
	ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
	ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;

	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_ALL;
	info.nMin = 0;
	info.nMax = 388+ptDiff.y;
	info.nPage = 224;
	info.nPos = 0;
	info.nTrackPos = 0;
	InitializeFlatSB(m_hWnd);
	FlatSB_EnableScrollBar(m_hWnd, SB_VERT, ESB_ENABLE_BOTH);
	FlatSB_ShowScrollBar(m_hWnd, SB_VERT, true);
	FlatSB_SetScrollInfo(m_hWnd, SB_VERT, &info, true);


	return TRUE;
}

void CChipSelectDlg::OnBnClickedOk()
{


	
	OnOK();
}

void CChipSelectDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	
	//SCROLLINFO info;
	//FlatSB_GetScrollInfo(m_hWnd, SB_VERT, &info);
	//if (nSBCode == SB_THUMBTRACK) {
	//	ScrollWindow(0, GetScrollPos(SB_VERT)-nPos);
	//	FlatSB_SetScrollPos(m_hWnd, SB_VERT, nPos, true);
	//}

	// TODO: Add your message handler code here and/or call default
	int CurPos = FlatSB_GetScrollPos(m_hWnd, SB_VERT);
	RECT rcClient, rcWind;
	POINT ptDiff;
	GetClientRect(&rcClient);
	GetWindowRect(&rcWind);
	ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
	ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;

	

	// Determine the new position of scroll box.
	switch (nSBCode)
	{
	case SB_LEFT:      // Scroll to far left.
		CurPos = 0;
		break;
	case SB_RIGHT:      // Scroll to far right.
		CurPos = 122;
		break;
	case SB_ENDSCROLL:   // End scroll.
		break;
	case SB_LINEUP:      // Scroll up.
		//if (CurPos > 0)
			CurPos=std::max(0,CurPos-16);
		break;
	case SB_LINEDOWN:   // Scroll down.
		//if (CurPos < 388 + 32)
			CurPos = std::min(388 - 224 + (int)ptDiff.y,CurPos+16);
		break;
	case SB_PAGEUP:    // Scroll one page up.
	{
		// Get the page size. 
		CurPos = std::max(0, CurPos - 224);
	}
	break;
	case SB_PAGEDOWN:      // Scroll one page down
	{
		// Get the page size. 
		CurPos = std::min(388 - 224 + (int)ptDiff.y, CurPos + 224);
	}
	break;
	case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
		CurPos = nPos;      // of the scroll box at the end of the drag operation.
		break;
	case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
		CurPos = nPos;     // position that the scroll box has been dragged to.
		break;
	}

	// Set the new position of the thumb (scroll box).
	auto scrollRect = CRect((int)rcClient.left, 41, (int)rcClient.right, (int)rcClient.bottom + 388 + (int)ptDiff.y);
	auto clientRect = CRect((int)rcClient.left, 41, (int)rcClient.right, (int)rcClient.bottom);
	auto fixedRect = CRect((int)rcClient.left, (int)rcClient.top, (int)rcClient.right, 41);
	
	// Move controls
	HWND hwnd = ::GetTopWindow(this->GetSafeHwnd());
	while (hwnd)
	{
		UINT nID = ::GetDlgCtrlID(hwnd);
		CRect rect;
		::GetWindowRect(hwnd, &rect);
		ScreenToClient(&rect);
		if (nID != IDC_CHIP_BUTTON_BG && nID != IDOK && nID != IDCANCEL)
		{
			::MoveWindow(hwnd, rect.left,
				rect.top + FlatSB_GetScrollPos(m_hWnd, SB_VERT) - CurPos, rect.Width(),
				rect.Height(), FALSE);
			//HRGN clientRegion = CreateRectRgn(0,0,0,0);
			//int regionType = GetDlgItem(nID)->GetWindowRgn(clientRegion);
			//if (regionType != ERROR)
			//{
				/* hrgn contains window region */
				//GetDlgItem(nID)->SetWindowRgn(clientRegion, false);
			//}
			//DeleteObject(clientRegion); /* finished with region */
			CDC* pDC = CDC::FromHandle(::GetWindowDC(hwnd));
		}
		hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT);
	}

	// Scroll window
	ScrollWindow(0, FlatSB_GetScrollPos(m_hWnd, SB_VERT) - CurPos, scrollRect, clientRect);

	// move
	FlatSB_SetScrollPos(m_hWnd, SB_VERT, CurPos, true);

	RedrawWindow(fixedRect, NULL, RDW_NOERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

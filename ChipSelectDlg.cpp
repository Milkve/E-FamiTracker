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

	RECT rcClient, rcWind, rcMenu;
	POINT ptDiff;
	GetClientRect(&rcClient);
	GetWindowRect(&rcWind);
	ptDiff.x = (rcWind.right - rcWind.left) - rcClient.right;
	ptDiff.y = (rcWind.bottom - rcWind.top) - rcClient.bottom;
	
	m_pChipList.Create(IDD_CHIP_LIST, this);
	m_pChipList.ShowWindow(SW_SHOW);
	m_pChipList.GetWindowRect(&rcMenu);

	((CButton*)m_pChipList.GetDlgItem(IDC_CHIP_2A03))->SetCheck(1);

	SCROLLINFO info;
	info.cbSize = sizeof(SCROLLINFO);
	info.fMask = SIF_ALL;
	info.nMin = 0;
	info.nMax = rcMenu.bottom-rcMenu.top+ptDiff.y;
	info.nPage = rcClient.bottom;
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
	

	// Scroll window
	m_pChipList.ScrollWindow(0, FlatSB_GetScrollPos(m_hWnd, SB_VERT) - CurPos);
	FlatSB_SetScrollPos(m_hWnd, SB_VERT, CurPos, true);
	

	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

// CScaleEditorDlg.cpp : implementation file
//


#include "stdafx.h"
//#include "afxdialogex.h"
#include <cstdlib>
#include <cmath>

#include "FamiTracker.h"
#include "FamiTrackerDoc.h"
#include "MainFrm.h"
#include "ScaleEditorDlg.h"
#include "SoundGen.h"


// CScaleEditorDlg dialog

IMPLEMENT_DYNAMIC(CScaleEditorDlg, CDialog)

CScaleEditorDlg::CScaleEditorDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(CScaleEditorDlg::IDD, pParent), m_pDocument(NULL)
{

}

CScaleEditorDlg::~CScaleEditorDlg()
{
}

void CScaleEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CScaleEditorDlg, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_BN_CLICKED(IDC_SCALE_BUTTON_AB, OnBnClickedScaleAB)
	ON_BN_CLICKED(IDC_SCALE_BUTTON_CD, OnBnClickedScaleCD)
	ON_BN_CLICKED(IDC_SCALE_BUTTON_DE, OnBnClickedScaleDE)
	ON_BN_CLICKED(IDC_SCALE_BUTTON_FG, OnBnClickedScaleFG)
	ON_BN_CLICKED(IDC_SCALE_BUTTON_GA, OnBnClickedScaleGA)
	ON_CBN_SELCHANGE(IDC_SCALE_KEY, OnCbnSelChangeKey)
	ON_CBN_SELCHANGE(IDC_SCALE_SCALE, OnCbnSelChangeScale)
END_MESSAGE_MAP()

// CScaleEditorDlg message handlers

BOOL CScaleEditorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Get active document
	CFrameWnd* pFrameWnd = static_cast<CFrameWnd*>(GetParent());
	m_pDocument = static_cast<CFamiTrackerDoc*>(pFrameWnd->GetActiveDocument());

	for (char i = 0; i < 5; i++)
		m_pFlats[i] = m_pDocument->GetFlat(i);

	UpdateFlats();

	return TRUE;
}

void CScaleEditorDlg::OnBnClickedOk()
{

	for (char i = 0; i < 5; i++) {
		if (m_pDocument->GetFlat(i) != m_pFlats[i])
		{
			m_pDocument->SetModifiedFlag();
			m_pDocument->UpdateAllViews(NULL, UPDATE_PATTERN);
		}
		m_pDocument->SetFlat(i, m_pFlats[i]);
	}

	
	OnOK();
}


void CScaleEditorDlg::UpdateFlats()
{
	((CButton*)GetDlgItem(IDC_SCALE_BUTTON_AB))->SetWindowTextA(m_pFlats[0] ? _T("b") : _T("#"));
	((CButton*)GetDlgItem(IDC_SCALE_TEXT_AB))->SetWindowTextA(m_pFlats[0] ? _T("B") : _T("A"));
	((CButton*)GetDlgItem(IDC_SCALE_BUTTON_CD))->SetWindowTextA(m_pFlats[1] ? _T("b") : _T("#"));
	((CButton*)GetDlgItem(IDC_SCALE_TEXT_CD))->SetWindowTextA(m_pFlats[1] ? _T("D") : _T("C"));
	((CButton*)GetDlgItem(IDC_SCALE_BUTTON_DE))->SetWindowTextA(m_pFlats[2] ? _T("b") : _T("#"));
	((CButton*)GetDlgItem(IDC_SCALE_TEXT_DE))->SetWindowTextA(m_pFlats[2] ? _T("E") : _T("D"));
	((CButton*)GetDlgItem(IDC_SCALE_BUTTON_FG))->SetWindowTextA(m_pFlats[3] ? _T("b") : _T("#"));
	((CButton*)GetDlgItem(IDC_SCALE_TEXT_FG))->SetWindowTextA(m_pFlats[3] ? _T("G") : _T("F"));
	((CButton*)GetDlgItem(IDC_SCALE_BUTTON_GA))->SetWindowTextA(m_pFlats[4] ? _T("b") : _T("#"));
	((CButton*)GetDlgItem(IDC_SCALE_TEXT_GA))->SetWindowTextA(m_pFlats[4] ? _T("A") : _T("G"));
}

void CScaleEditorDlg::ResetKey()
{
	((CComboBox*)GetDlgItem(IDC_SCALE_KEY))->SetCurSel(-1);
	((CComboBox*)GetDlgItem(IDC_SCALE_SCALE))->SetCurSel(-1);
}

void CScaleEditorDlg::SetFlat(char idx)
{
	ResetKey();
	m_pFlats[idx] = !m_pFlats[idx];
	UpdateFlats();
}


void CScaleEditorDlg::OnBnClickedScaleAB() { SetFlat(0); }
void CScaleEditorDlg::OnBnClickedScaleCD() { SetFlat(1); }
void CScaleEditorDlg::OnBnClickedScaleDE() { SetFlat(2); }
void CScaleEditorDlg::OnBnClickedScaleFG() { SetFlat(3); }
void CScaleEditorDlg::OnBnClickedScaleGA() { SetFlat(4); }



void CScaleEditorDlg::OnCbnSelChangeKey()
{
	CComboBox* Key = ((CComboBox*)GetDlgItem(IDC_SCALE_KEY));
	m_pKey = Key->GetCurSel();

	if (m_pScale != -1) {
		char NewFlats = SelectionFromScale[m_pScale][m_pKey];
		for (char i = 0; i < 5; i++)
			m_pFlats[i] = (bool)(!((NewFlats >> i) & 0x1));
	}

	UpdateFlats();
}

void CScaleEditorDlg::OnCbnSelChangeScale()
{
	CComboBox* Scale = ((CComboBox*)GetDlgItem(IDC_SCALE_SCALE));
	m_pScale = Scale->GetCurSel();

	if (m_pKey != -1) {
		int NewFlats = SelectionFromScale[m_pScale][m_pKey];
		for (char i = 0; i < 5; i++)
			m_pFlats[i] = (bool)(!((NewFlats >> i) & 0x1));
	}

	UpdateFlats();
}
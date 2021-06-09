#pragma once


// CScaleEditorDlg dialog






class CChipSelectDlg : public CDialog
{
	DECLARE_DYNAMIC(CChipSelectDlg)

private:

	CFamiTrackerDoc* m_pDocument;

public:
	CChipSelectDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CChipSelectDlg();

// Dialog Data
	enum { IDD = IDD_CHIP_SELECT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

private:

};
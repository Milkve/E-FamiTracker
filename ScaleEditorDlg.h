#pragma once


// CScaleEditorDlg dialog




enum scalekey_t : unsigned char {
	KEY_A,
	KEY_ASHARP,
	KEY_BFLAT,
	KEY_B,
	KEY_C,
	KEY_CSHARP,
	KEY_DFLAT,
	KEY_D,
	KEY_DSHARP,
	KEY_EFLAT,
	KEY_E,
	KEY_F,
	KEY_FSHARP,
	KEY_GFLAT,
	KEY_G,
	KEY_GSHARP,
	KEY_AFLAT,

	KEY_COUNT
};


enum scale_t : unsigned char {
	SCALE_MAJOR,
	SCALE_MINOR,
	SCALE_HARMONICMINOR,
	SCALE_MELODICMINOR,
	SCALE_IONIAN,
	SCALE_DORIAN,
	SCALE_PHRYGIAN,
	SCALE_LYDIAN,
	SCALE_MIXOLYDIAN,
	SCALE_AEOLIAN,
	SCALE_LOCRIAN,

	SCALE_COUNT
};




class CScaleEditorDlg : public CDialog
{
	DECLARE_DYNAMIC(CScaleEditorDlg)

private:
	bool m_pFlats[5] = {false, false, false, false, false};
	int m_pKey = -1;
	int m_pScale = -1;

	CFamiTrackerDoc* m_pDocument;

	void UpdateFlats();
	void ResetKey();
	void SetFlat(char idx);

public:
	CScaleEditorDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CScaleEditorDlg();

// Dialog Data
	enum { IDD = IDD_SCALE_EDITOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedScaleAB();
	afx_msg void OnBnClickedScaleCD();
	afx_msg void OnBnClickedScaleDE();
	afx_msg void OnBnClickedScaleFG();
	afx_msg void OnBnClickedScaleGA();
	afx_msg void OnCbnSelChangeKey();
	afx_msg void OnCbnSelChangeScale();

private:
	int SelectionFromScale[SCALE_COUNT][KEY_COUNT] = {
		{0b11111,0b00000,0b00000,0b11111,0b11111,0b11111,0b00000,0b11111,0b00000,
		 0b00000,0b11111,0b00000,0b11111,0b00000,0b11111,0b00000,0b00000}, //SCALE_MAJOR
		{0b11111,0b11111,0b00000,0b11111,0b00000,0b11111,0b00000,0b00000,0b11111,
		 0b00000,0b11111,0b00000,0b11111,0b00000,0b00000,0b11111,0b00000}, //SCALE_MINOR
		{0b11111,0b11111,0b00000,0b11111,0b00000,0b11111,0b00000,0b01111,0b11111,
		 0b00000,0b11111,0b00000,0b11111,0b00000,0b00011,0b11111,0b00000}, //SCALE_HARMONICMINOR
		{0b11111,0b11111,0b00000,0b11111,0b00000,0b11111,0b00000,0b11111,0b11111,
		 0b00000,0b11111,0b00000,0b11111,0b00000,0b01111,0b11111,0b00000}, //SCALE_MELODICMINOR
		{0b11111,0b00000,0b00000,0b11111,0b11111,0b11111,0b00000,0b11111,0b00000,
		 0b00000,0b11111,0b00000,0b11111,0b00000,0b11111,0b00000,0b00000}, //SCALE_IONIAN
		{}, //SCALE_DORIAN
		{}, //SCALE_PHRYGIAN
		{}, //SCALE_LYDIAN
		{}, //SCALE_MIXOLYDIAN
		{0b11111,0b11111,0b00000,0b11111,0b00000,0b11111,0b00000,0b00000,0b11111,
		 0b00000,0b11111,0b00000,0b11111,0b00000,0b00000,0b11111,0b00000}, //SCALE_AEOLIAN
		{}, //SCALE_LOCRIAN
	};

};
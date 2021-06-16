/*
** FamiTracker - NES/Famicom sound tracker
** Copyright (C) 2005-2014  Jonathan Liss
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#pragma once

class CInstrumentSID;

// CInstrumentEditorSID dialog

class CInstrumentEditorSID : public CInstrumentEditPanel
{
	DECLARE_DYNAMIC(CInstrumentEditorSID)

public:
	CInstrumentEditorSID(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInstrumentEditorSID();
	virtual int GetIDD() const { return IDD; };
	virtual TCHAR *GetTitle() const { return _T("MOS Technology SID"); };

	// Public
	virtual void SelectInstrument(std::shared_ptr<CInstrument> pInst);

	// Dialog Data
	enum { IDD = IDD_INSTRUMENT_SID };

protected:

protected:
	std::shared_ptr<CInstrumentSID> m_pInstrument;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnPWMStartSpinChange();
	afx_msg void OnPWMEndSpinChange();
	afx_msg void OnPWMDisableClicked();
	afx_msg void OnPWMLoopClicked();
	afx_msg void OnPWMPingPongClicked();
	afx_msg void OnPWMOnceClicked();
	afx_msg void OnPWMSustainClicked();
private:
	void EnablePWM(bool Enabled = 1);

};

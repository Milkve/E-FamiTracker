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

#include <iterator> 
#include <sstream>
#include "stdafx.h"
#include "FamiTracker.h"
#include "Instrument.h"		// // //
#include "SeqInstrument.h"		// // //
#include "InstrumentSID.h"		// // //
#include "DPI.h"		// // //
#include "InstrumentEditPanel.h"
#include "InstrumentEditorSID.h"
#include "APU/Types.h"		// // //
#include "SoundGen.h"
#include "Clipboard.h"

// CInstrumentEditorSID dialog

IMPLEMENT_DYNAMIC(CInstrumentEditorSID, CInstrumentEditPanel)

CInstrumentEditorSID::CInstrumentEditorSID(CWnd* pParent) : CInstrumentEditPanel(CInstrumentEditorSID::IDD, pParent)
{
}

CInstrumentEditorSID::~CInstrumentEditorSID()
{
}

void CInstrumentEditorSID::DoDataExchange(CDataExchange* pDX)
{
	CInstrumentEditPanel::DoDataExchange(pDX);
}

void CInstrumentEditorSID::SelectInstrument(std::shared_ptr<CInstrument> pInst)
{
	m_pInstrument = std::dynamic_pointer_cast<CInstrumentSID>(pInst);
	ASSERT(m_pInstrument);

	static_cast<CSliderCtrl*>(GetDlgItem(IDC_SLIDER_ATTACK))->SetRange(0, 15);
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_SLIDER_DECAY))->SetRange(0, 15);
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_SLIDER_SUSTAIN))->SetRange(0, 15);
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_SLIDER_RELEASE))->SetRange(0, 15);

	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_START))->SetRange(0, 4095);
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_END))->SetRange(0, 4095);
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_SPEED))->SetRange(0, 255);

	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PWM_START_SPIN))->SetRange(0, 4095);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PWM_END_SPIN))->SetRange(0, 4095);

	static_cast<CSliderCtrl*>(GetDlgItem(IDC_SLIDER_ATTACK))->SetPos(15-m_pInstrument->GetEnvParam(ENV_ATTACK));
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_SLIDER_DECAY))->SetPos(15-m_pInstrument->GetEnvParam(ENV_DECAY));
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_SLIDER_SUSTAIN))->SetPos(15-m_pInstrument->GetEnvParam(ENV_SUSTAIN));
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_SLIDER_RELEASE))->SetPos(15-m_pInstrument->GetEnvParam(ENV_RELEASE));

	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_START))->SetPos(m_pInstrument->GetPWMStart());
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_END))->SetPos(m_pInstrument->GetPWMEnd());
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_SPEED))->SetPos(255 - m_pInstrument->GetPWMSpeed());

	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PWM_START_SPIN))->SetPos(m_pInstrument->GetPWMStart());
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PWM_END_SPIN))->SetPos(m_pInstrument->GetPWMEnd());


	int mode = m_pInstrument->GetPWMMode();
	switch (mode) {
	case PWM_DISABLED: CheckRadioButton(IDC_PWM_MODE1, IDC_DISABLE_PWM, IDC_DISABLE_PWM); EnablePWM(false); break;
	case PWM_LOOP: CheckRadioButton(IDC_PWM_MODE1, IDC_DISABLE_PWM, IDC_PWM_MODE1); EnablePWM(); break;
	case PWM_PINGPONG: CheckRadioButton(IDC_PWM_MODE1, IDC_DISABLE_PWM, IDC_PWM_MODE2); EnablePWM(); break;
	case PWM_ONCE: CheckRadioButton(IDC_PWM_MODE1, IDC_DISABLE_PWM, IDC_PWM_MODE3); EnablePWM(); break;
	case PWM_SUSTAIN: CheckRadioButton(IDC_PWM_MODE1, IDC_DISABLE_PWM, IDC_PWM_MODE4); EnablePWM(false); break;
	}

	CString ratetext = _T("Rate: ");
	ratetext.AppendFormat(_T("%d"), 255 - static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_SPEED))->GetPos());
	static_cast<CStatic*>(GetDlgItem(IDC_TEXT_RATE))->SetWindowTextA(ratetext);
	


}


BEGIN_MESSAGE_MAP(CInstrumentEditorSID, CInstrumentEditPanel)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_BOX_PWM_START, OnPWMStartSpinChange)
	ON_EN_CHANGE(IDC_BOX_PWM_END, OnPWMEndSpinChange)
	ON_BN_CLICKED(IDC_DISABLE_PWM, OnPWMDisableClicked)
	ON_BN_CLICKED(IDC_PWM_MODE1, OnPWMLoopClicked)
	ON_BN_CLICKED(IDC_PWM_MODE2, OnPWMPingPongClicked)
	ON_BN_CLICKED(IDC_PWM_MODE3, OnPWMOnceClicked)
	ON_BN_CLICKED(IDC_PWM_MODE4, OnPWMSustainClicked)
END_MESSAGE_MAP()

// CInstrumentEditorSID message handlers

BOOL CInstrumentEditorSID::OnInitDialog()
{
	CInstrumentEditPanel::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



void CInstrumentEditorSID::EnablePWM(bool Enabled)
{
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PWM_START_SPIN))->EnableWindow(Enabled);
	static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PWM_END_SPIN))->EnableWindow(Enabled);
	static_cast<CEdit*>(GetDlgItem(IDC_BOX_PWM_START))->EnableWindow(Enabled);
	static_cast<CEdit*>(GetDlgItem(IDC_BOX_PWM_END))->EnableWindow(Enabled);
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_START))->EnableWindow(Enabled);
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_END))->EnableWindow(Enabled);
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_SPEED))->EnableWindow(Enabled);
}



void CInstrumentEditorSID::OnPWMStartSpinChange()
{
	int pos = static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PWM_START_SPIN))->GetPos();
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_START))->SetPos(pos);
	if (m_pInstrument)
		m_pInstrument->SetPWMStart(pos);
}
void CInstrumentEditorSID::OnPWMEndSpinChange()
{
	int pos = static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PWM_END_SPIN))->GetPos();
	static_cast<CSliderCtrl*>(GetDlgItem(IDC_PWM_END))->SetPos(pos);
	if (m_pInstrument)
		m_pInstrument->SetPWMEnd(pos);
}
void CInstrumentEditorSID::OnPWMDisableClicked()
{
	EnablePWM(false);
	m_pInstrument->SetPWMMode(PWM_DISABLED);
}
void CInstrumentEditorSID::OnPWMLoopClicked()
{
	EnablePWM();
	m_pInstrument->SetPWMMode(PWM_LOOP);
}
void CInstrumentEditorSID::OnPWMPingPongClicked()
{
	EnablePWM();
	m_pInstrument->SetPWMMode(PWM_PINGPONG);
}
void CInstrumentEditorSID::OnPWMOnceClicked()
{
	EnablePWM();
	m_pInstrument->SetPWMMode(PWM_ONCE);
}
void CInstrumentEditorSID::OnPWMSustainClicked()
{
	EnablePWM(false);
	m_pInstrument->SetPWMMode(PWM_SUSTAIN);
}


void CInstrumentEditorSID::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl* pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);
	int pos = pSlider->GetPos();
	if (pSlider->GetDlgCtrlID() == IDC_SLIDER_ATTACK)
		m_pInstrument->SetEnvParam(ENV_ATTACK, 15 - pos);
	else if (pSlider->GetDlgCtrlID() == IDC_SLIDER_DECAY)
		m_pInstrument->SetEnvParam(ENV_DECAY, 15 - pos);
	else if (pSlider->GetDlgCtrlID() == IDC_SLIDER_SUSTAIN)
		m_pInstrument->SetEnvParam(ENV_SUSTAIN, 15 - pos);
	else if (pSlider->GetDlgCtrlID() == IDC_SLIDER_RELEASE)
		m_pInstrument->SetEnvParam(ENV_RELEASE, 15 - pos);
	else if (pSlider->GetDlgCtrlID() == IDC_PWM_SPEED) {
		CString ratetext = _T("Rate: ");
		ratetext.AppendFormat(_T("%d"), 255-pos);
		static_cast<CStatic*>(GetDlgItem(IDC_TEXT_RATE))->SetWindowTextA(ratetext);
		m_pInstrument->SetPWMSpeed(255 - pos);
	}

	CInstrumentEditPanel::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CInstrumentEditorSID::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CSliderCtrl* pSlider = reinterpret_cast<CSliderCtrl*>(pScrollBar);
	int pos = pSlider->GetPos();
	if (pSlider->GetDlgCtrlID() == IDC_PWM_START) {
		m_pInstrument->SetPWMStart(pos);
		static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PWM_START_SPIN))->SetPos(pos);
	}
	else if (pSlider->GetDlgCtrlID() == IDC_PWM_END) {
		m_pInstrument->SetPWMEnd(pos);
		static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_PWM_END_SPIN))->SetPos(pos);
	}
}
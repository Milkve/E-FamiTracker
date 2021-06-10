/*
** FamiTracker - NES/Famicom sound tracker
** Copyright (C) 2005-2014  Jonathan Liss
**
** 0CC-FamiTracker is (C) 2014-2015 HertzDevil
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

// MOS Technology 6581

#include "stdafx.h"
#include "FamiTrackerTypes.h"		// // //
#include "APU/Types.h"		// // //
#include "Sequence.h"		// // //
#include "Instrument.h"		// // //
#include "ChannelHandler.h"
#include "Channels6581.h"
#include "APU/APU.h"
#include "InstHandler.h"		// // //
#include "SeqInstHandler.h"		// // //
#include <map>

// Static member variables, for the shared stuff in 6581

// Class functions


//void CChannelHandler6581::UpdateRegs()		// // //
//{
	// Done only once
	//if (s_iNoiseFreq != s_iNoisePrev)		// // //
	//	WriteReg(0x06, (s_iNoisePrev = s_iNoiseFreq) ^ 0xFF);
	//WriteReg(0x07, s_iModes);
//}

// Instance functions

CChannelHandler6581::CChannelHandler6581() : 
	CChannelHandler(0xFFFF, 0xF),
	m_bUpdate(false)
{
	m_iDefaultDuty = 0;		// // //
}



//const  char CChannelHandler6581::MAX_DUTY = 0xFF;

int CChannelHandler6581::getDutyMax() const {
	return 0xFF;
}


bool CChannelHandler6581::HandleEffect(effect_t EffNum, unsigned char EffParam)
{
	switch (EffNum) {
	case EF_DUTY_CYCLE: {
		/*
		Translate Vxx bitmask to `enum DutyType` bitmask, using VXX_TO_DUTY
		as a conversion table.

		CSeqInstHandlerS5B::ProcessSequence loads m_iDutyPeriod from the top
		3 bits of an instrument's duty sequence. (The bottom 5 go to m_iNoiseFreq.)
		This function moves Vxx to the top 3 bits of m_iDutyPeriod.
		*/

		//unsigned char duty = 0;
		//for (auto const&[VXX, DUTY] : VXX_TO_DUTY) {
		//	if (EffParam & VXX) {
		//		duty |= DUTY;
		//	}
		//}

		m_iDefaultDuty = m_iDutyPeriod = (EffParam);// | (EffParam & 0x0F);
		break;
	}
	default: return CChannelHandler::HandleEffect(EffNum, EffParam);
	}

	return true;
}

void CChannelHandler6581::HandleNote(int Note, int Octave)		// // //
{
	CChannelHandler::HandleNote(Note, Octave);

	/*
	Vxx is handled above: CChannelHandler6581::HandleEffect, case EF_DUTY_CYCLE
	m_iDefaultDuty is Vxx.
	m_iDutyPeriod is Vxx plus instrument bit-flags. But it's not fully
		initialized yet (instruments are handled after notes) which is bad.
	https://docs.google.com/document/d/e/2PACX-1vQ8osh6mm4c4Ay_gVMIJCH8eRB5gBE180Xyeda1T5U6owG7BbKM-yNKVB8azg27HUD9QZ9Vf88crplE/pub
	*/
}

void CChannelHandler6581::HandleEmptyNote()
{
}

void CChannelHandler6581::HandleCut()
{
	CutNote();
	//m_iDutyPeriod = S5B_MODE_SQUARE;
	m_iNote = 0;
}

void CChannelHandler6581::HandleRelease()
{
	if (!m_bRelease)
		ReleaseNote();		// // //
}

bool CChannelHandler6581::CreateInstHandler(inst_type_t Type)
{
	switch (Type) {
	case INST_2A03: case INST_VRC6: case INST_N163: case INST_S5B: case INST_FDS: case INST_SID:
		switch (m_iInstTypeCurrent) {
		case INST_2A03: case INST_VRC6: case INST_N163: case INST_S5B: case INST_FDS: case INST_SID: break;
		default:
			m_pInstHandler.reset(new CSeqInstHandler(this, 0x0F, 0x0F, Type == INST_SID ? 0x40 : 0));
			return true;
		}
	}
	return false;
}

void CChannelHandler6581::WriteReg(int Reg, int Value)
{
	WriteRegister(0xD400+Reg, Value);
}

void CChannelHandler6581::ResetChannel()
{
	CChannelHandler::ResetChannel();

	m_iDefaultDuty = m_iDutyPeriod = 80;
	m_iPulseWidth = 0;
}

int CChannelHandler6581::CalculateVolume() const		// // //
{
	return LimitVolume((m_iVolume >> VOL_COLUMN_SHIFT) - GetTremolo() + m_iInstVolume - 15);
}

int CChannelHandler6581::ConvertDuty(int Duty)		// // //
{
	switch (m_iInstTypeCurrent) {
	case INST_2A03: case INST_VRC6: case INST_N163:
		return 80;
	default:
		return Duty;
	}
}

void CChannelHandler6581::ClearRegisters()
{
	//WriteReg(0x18, 0);		// Clear volume
}

CString CChannelHandler6581::GetCustomEffectString() const		// // //
{
	CString str = _T("");

	//if (s_iEnvelopeAType)
		//str.AppendFormat(_T(" J%02X"), s_iEnvelopeAType);

	return str;
}

void CChannelHandler6581::RefreshChannel()
{
	int Volume = CalculateVolume();
	if (Volume >= 0)
		WriteReg(0x18, Volume);

	WriteReg(0x00, 0x80);
	WriteReg(0x01, 0x80);
	WriteReg(0x02, 0x80);
	WriteReg(0x03, 0x80);
	WriteReg(0x04, 0x80);

}
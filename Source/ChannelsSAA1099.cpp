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

// Sunsoft 5B (YM2149/AY-3-8910)

#include "stdafx.h"
#include "FamiTrackerTypes.h"		// // //
#include "APU/Types.h"		// // //
#include "Sequence.h"		// // //
#include "Instrument.h"		// // //
#include "ChannelHandler.h"
#include "ChannelsSAA1099.h"
#include "APU/APU.h"
#include "InstHandler.h"		// // //
#include "SeqInstHandler.h"		// // //
#include <map>

// Static member variables, for the shared stuff in 5B
int			  CChannelHandlerSAA1099::s_iModes		= 0;
int			  CChannelHandlerSAA1099::s_iNoiseFreq = 0;
int			  CChannelHandlerSAA1099::s_iNoisePrev = -1;
int			  CChannelHandlerSAA1099::s_iDefaultNoise = 0;		// // //
int			  CChannelHandlerSAA1099::s_iNoiseANDMask = 0x00;		// // //
int			  CChannelHandlerSAA1099::s_iNoiseORMask = 0xFF;		// // //
int			  CChannelHandlerSAA1099::s_iEnvelopeAType = 0x00;		// // //
int			  CChannelHandlerSAA1099::s_iEnvelopeBType = 0x00;		// // //
int			  CChannelHandlerSAA1099::s_unused		= 0;		// // // 050B

// Class functions

void CChannelHandlerSAA1099::SetMode(int Chan, int Square, int Noise)
{
	Chan -= CHANID_SAA1099_CH1;

	switch (Chan) {
		case 0:
			s_iModes &= 0x36;
			break;
		case 1:
			s_iModes &= 0x2D;
			break;
		case 2:
			s_iModes &= 0x1B;
			break;
	}

	s_iModes |= (Noise << (3 + Chan)) | (Square << Chan);
}

void CChannelHandlerSAA1099::UpdateRegs()		// // //
{
	// Done only once
	if (s_iNoiseFreq != s_iNoisePrev)		// // //
		WriteReg(0x06, (s_iNoisePrev = s_iNoiseFreq) ^ 0xFF);
	WriteReg(0x07, s_iModes);
	WriteReg(0x19, s_iNoiseANDMask);
	WriteReg(0x1A, s_iNoiseORMask);
}

// Instance functions

CChannelHandlerSAA1099::CChannelHandlerSAA1099() : 
	CChannelHandler(0x7FF, 0x1F),
	m_bEnvelopeEnabled(false),		// // // 050B
	m_iAutoEnvelopeShift(0),		// // // 050B
	m_iEnvFreqHi(0),		// // // 050B
  m_iEnvFreqLo(0),		// // // 050B
  m_bEnvTrigger(false),		// // // 050B
  m_iEnvType(0),		// // // 050B
	m_bUpdate(false)
{
	m_iDefaultDuty = S5B_MODE_SQUARE;		// // //
	s_iDefaultNoise = 0;		// // //
}


using EffParamT = unsigned char;
static const std::map<EffParamT, ay8930_mode_t> VXX_TO_DUTY = {
	{1<<0, AY8930_MODE_SQUARE},
	{1<<1, AY8930_MODE_NOISE},
	{1<<2, AY8930_MODE_ENVELOPE},
};

const char CChannelHandlerSAA1099::MAX_DUTY = 0x07;		// = 1|2|4

int CChannelHandlerSAA1099::getDutyMax() const {
	return MAX_DUTY;
}


bool CChannelHandlerSAA1099::HandleEffect(effect_t EffNum, EffParamT EffParam)
{
	switch (EffNum) {
	case EF_SUNSOFT_ENV_TYPE: // H
		//m_bEnvTrigger = true;		// // // 050B
		//m_iEnvType = EffParam & 0x0F;
		//m_bUpdate = true;
		//m_bEnvelopeEnabled = EffParam != 0;
		//m_iAutoEnvelopeShift = EffParam >> 4;
		if (m_iChannelID <= CHANID_SAA1099_CH3) {
			s_iEnvelopeAType = EffParam;
		}
		else {
			s_iEnvelopeBType = EffParam;
		}
		break;
	//case EF_STEREO:
		//m_iExVolume = EffParam & 1;
		//break;
	case EF_DUTY_CYCLE: {
		/*
		Translate Vxx bitmask to `enum DutyType` bitmask, using VXX_TO_DUTY
		as a conversion table.

		CSeqInstHandlerS5B::ProcessSequence loads m_iDutyPeriod from the top
		3 bits of an instrument's duty sequence. (The bottom 5 go to m_iNoiseFreq.)
		This function moves Vxx to the top 3 bits of m_iDutyPeriod.
		*/

		unsigned char duty = 0;
		for (auto const&[VXX, DUTY] : VXX_TO_DUTY) {
			if (EffParam & VXX) {
				duty |= DUTY;
			}
		}

		m_iDefaultDuty = m_iDutyPeriod = (duty);// | (EffParam & 0x0F);
		break;
	}
	default: return CChannelHandler::HandleEffect(EffNum, EffParam);
	}

	return true;
}

void CChannelHandlerSAA1099::HandleNote(int Note, int Octave)		// // //
{
	CChannelHandler::HandleNote(Note, Octave);

	/*
	Vxx is handled above: CChannelHandlerSAA1099::HandleEffect, case EF_DUTY_CYCLE
	m_iDefaultDuty is Vxx.
	m_iDutyPeriod is Vxx plus instrument bit-flags. But it's not fully
		initialized yet (instruments are handled after notes) which is bad.
	https://docs.google.com/document/d/e/2PACX-1vQ8osh6mm4c4Ay_gVMIJCH8eRB5gBE180Xyeda1T5U6owG7BbKM-yNKVB8azg27HUD9QZ9Vf88crplE/pub
	*/
	
}

void CChannelHandlerSAA1099::HandleEmptyNote()
{
}

void CChannelHandlerSAA1099::HandleCut()
{
	CutNote();
	m_iDutyPeriod = S5B_MODE_SQUARE;
	m_iNote = 0;
}

void CChannelHandlerSAA1099::HandleRelease()
{
	if (!m_bRelease)
		ReleaseNote();		// // //
}

bool CChannelHandlerSAA1099::CreateInstHandler(inst_type_t Type)
{
	switch (Type) {
	case INST_2A03: case INST_VRC6: case INST_N163: case INST_S5B: case INST_FDS:
		switch (m_iInstTypeCurrent) {
		case INST_2A03: case INST_VRC6: case INST_N163: case INST_S5B: case INST_FDS: break;
		default:
			m_pInstHandler.reset(new CSeqInstHandler(this, 0x0F, 0x0F, Type == INST_S5B ? 0x40 : 0));
			return true;
		}
	}
	return false;
}

void CChannelHandlerSAA1099::WriteReg(int Reg, int Value)
{
	WriteRegister(0xC002, Reg);
	WriteRegister(0xE002, Value);
}

void CChannelHandlerSAA1099::ResetChannel()
{
	CChannelHandler::ResetChannel();

	m_iDefaultDuty = m_iDutyPeriod = S5B_MODE_SQUARE;
	s_iDefaultNoise = s_iNoiseFreq = 0;		// // //
	s_iNoiseORMask = 0x00;		// // //
	s_iNoiseANDMask = 0x0F;   // // //
	s_iNoisePrev = -1;		// // //
	m_bEnvelopeEnabled = false;
	m_iAutoEnvelopeShift = 0;
	m_iEnvFreqHi = 0;
	m_iEnvFreqLo = 0;
	m_iEnvType = 0;
	s_iEnvelopeAType = 0;
	s_iEnvelopeBType = 0;
	m_iPulseWidth = 0;
	s_unused = 0;		// // // 050B
	m_bEnvTrigger = false;
}

int CChannelHandlerSAA1099::CalculateVolume() const		// // //
{
	return LimitVolume((m_iVolume >> VOL_COLUMN_SHIFT) - GetTremolo() + m_iInstVolume - 15);
}

int CChannelHandlerSAA1099::ConvertDuty(int Duty) const		// // //
{
	switch (m_iInstTypeCurrent) {
	case INST_2A03: case INST_VRC6: case INST_N163:
		return S5B_MODE_SQUARE;
	default:
		return Duty;
	}
}

void CChannelHandlerSAA1099::ClearRegisters()
{
	WriteReg(8 + m_iChannelID - CHANID_SAA1099_CH1, 0);		// Clear volume
}

CString CChannelHandlerSAA1099::GetCustomEffectString() const		// // //
{
	CString str = _T("");

	if (s_iEnvelopeAType)
		str.AppendFormat(_T(" J%02X"), s_iEnvelopeAType);

	return str;
}

void CChannelHandlerSAA1099::RefreshChannel()
{
	int Period = CalculatePeriod();
	unsigned char LoPeriod = Period & 0xFF;
	unsigned char HiPeriod = (Period >> 8) & 0x07;
	int Volume = CalculateVolume();

	unsigned char Noise = (m_bGate && (m_iDutyPeriod & S5B_MODE_NOISE)) ? 0 : 1;
	unsigned char Square = (m_bGate && (m_iDutyPeriod & S5B_MODE_SQUARE)) ? 0 : 1;
	unsigned char Envelope = (m_bGate && (m_iDutyPeriod & S5B_MODE_ENVELOPE)) ? 0x10 : 0;

	unsigned int CurrHiPeriod = m_pAPU->GetReg(SNDCHIP_SAA1099, ((m_iChannelID - CHANID_SAA1099_CH1) >> 1) + 0x10);
	CurrHiPeriod = ((m_iChannelID - CHANID_SAA1099_CH1) & 1) ? ((CurrHiPeriod & 0x0F) | (HiPeriod << 4)) : ((CurrHiPeriod & 0xF0) | HiPeriod);

	unsigned int CurrSquare = m_pAPU->GetReg(SNDCHIP_SAA1099, 0x14);
	

	WriteReg((m_iChannelID - CHANID_SAA1099_CH1)               , Volume);
	WriteReg((m_iChannelID - CHANID_SAA1099_CH1)        + 0x08 , LoPeriod);
	WriteReg(((m_iChannelID - CHANID_SAA1099_CH1) >> 1) + 0x10 , CurrHiPeriod);

	int mask = 1 << (m_iChannelID - CHANID_SAA1099_CH1);
	WriteReg(0x14, (CurrSquare & ~mask) | (Square << (m_iChannelID - CHANID_SAA1099_CH1)));

	if (m_iChannelID == CHANID_SAA1099_CH6) {
		WriteReg(0x18 , s_iEnvelopeAType);
		WriteReg(0x19 , s_iEnvelopeBType);
	}

}
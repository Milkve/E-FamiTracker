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

// This file handles playing of 5E01 channels

#include "stdafx.h"
#include "FamiTracker.h"
#include "FamiTrackerTypes.h"		// // //
#include "APU/Types.h"		// // //
#include "APU/APU.h"		// // // for DPCM
#include "DSample.h"		// // //
#include "Instrument.h"
#include "ChannelHandler.h"
#include "Channels5E01.h"
#include "Settings.h"
#include "InstHandler.h"		// // //
#include "SeqInstHandler.h"		// // //
#include "InstHandlerDPCM.h"		// // //

CChannelHandler5E01::CChannelHandler5E01() :
	CChannelHandler(0x7FF, 0x0F),
	m_bHardwareEnvelope(false),
	m_bEnvelopeLoop(true),
	m_bResetEnvelope(false),
	m_iLengthCounter(1)
{
}

void CChannelHandler5E01::HandleNoteData(stChanNote *pNoteData, int EffColumns)
{
	// // //
	CChannelHandler::HandleNoteData(pNoteData, EffColumns);

	if (pNoteData->Note != NONE && pNoteData->Note != HALT && pNoteData->Note != RELEASE) {
		if (!m_bEnvelopeLoop || m_bHardwareEnvelope)		// // //
			m_bResetEnvelope = true;
	}
}

bool CChannelHandler5E01::HandleEffect(effect_t EffNum, unsigned char EffParam)
{
	switch (EffNum) {
	case EF_VOLUME:
		if (EffParam < 0x20) {		// // //
			m_iLengthCounter = EffParam;
			m_bEnvelopeLoop = false;
			m_bResetEnvelope = true;
		}
		else if (EffParam >= 0xE0 && EffParam < 0xE4) {
			if (!m_bEnvelopeLoop || !m_bHardwareEnvelope)
				m_bResetEnvelope = true;
			m_bHardwareEnvelope = ((EffParam & 0x01) == 0x01);
			m_bEnvelopeLoop = ((EffParam & 0x02) != 0x02);
		}
		break;
	case EF_DUTY_CYCLE:
		m_iDefaultDuty = m_iDutyPeriod = EffParam;
		break;
	default: return CChannelHandler::HandleEffect(EffNum, EffParam);
	}

	return true;
}

void CChannelHandler5E01::HandleEmptyNote()
{
	// // //
}

void CChannelHandler5E01::HandleCut()
{
	CutNote();
}

void CChannelHandler5E01::HandleRelease()
{
	if (!m_bRelease)
		ReleaseNote();
/*
	if (!m_bSweeping && (m_cSweep != 0 || m_iSweep != 0)) {
		m_iSweep = 0;
		m_cSweep = 0;
		m_iLastPeriod = 0xFFFF;
	}
	else if (m_bSweeping) {
		m_cSweep = m_iSweep;
		m_iLastPeriod = 0xFFFF;
	}
	*/
}

bool CChannelHandler5E01::CreateInstHandler(inst_type_t Type)
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

void CChannelHandler5E01::ResetChannel()
{
	CChannelHandler::ResetChannel();
	m_bEnvelopeLoop = true;		// // //
	m_bHardwareEnvelope = false;
	m_iLengthCounter = 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// // // 5E01 Square
///////////////////////////////////////////////////////////////////////////////////////////////////////////

C5E01Square::C5E01Square() :
	CChannelHandler5E01(),
	m_cSweep(0),
	m_bSweeping(0),
	m_iSweep(0)
{
}

const char C5E01Square::MAX_DUTY = 0x03;

int C5E01Square::getDutyMax() const {
	return static_cast<int>(MAX_DUTY);
}

void C5E01Square::RefreshChannel()
{
	int Period = CalculatePeriod();
	int Volume = CalculateVolume();
	char DutyCycle = (m_iDutyPeriod & MAX_DUTY);

	unsigned char HiFreq = (Period & 0xFF);
	unsigned char LoFreq = (Period >> 8);
	
	int Address = 0x4100 + m_iChannel * 4;		// // //
	if (m_bGate)		// // //
		WriteRegister(Address, (DutyCycle << 6) | (m_bEnvelopeLoop << 5) | (!m_bHardwareEnvelope << 4) | Volume);		// // //
	else {
		WriteRegister(Address, 0x30);
		m_iLastPeriod = 0xFFFF;
		return;
	}

	if (m_cSweep) {
		if (m_cSweep & 0x80) {
			WriteRegister(Address + 1, m_cSweep);
			m_cSweep &= 0x7F;
			WriteRegister(0x4117, 0x80);	// Clear sweep unit
			WriteRegister(0x4117, 0x00);
			WriteRegister(Address + 2, HiFreq);
			WriteRegister(Address + 3, LoFreq + (m_iLengthCounter << 3));		// // //
			m_iLastPeriod = 0xFFFF;
		}
	}
	else {
		WriteRegister(Address + 1, 0x08);
		//WriteRegister(0x4017, 0x80);	// Manually execute one APU frame sequence to kill the sweep unit
		//WriteRegister(0x4017, 0x00);
		WriteRegister(Address + 2, HiFreq);
		
		if (LoFreq != (m_iLastPeriod >> 8) || m_bResetEnvelope)		// // //
			WriteRegister(Address + 3, LoFreq + (m_iLengthCounter << 3));
	}

	m_iLastPeriod = Period;
	m_bResetEnvelope = false;		// // //
}

void C5E01Square::SetChannelID(int ID)		// // //
{
	CChannelHandler::SetChannelID(ID);
	m_iChannel = ID - CHANID_5E01_SQUARE1;
}

int C5E01Square::ConvertDuty(int Duty)		// // //
{
	switch (m_iInstTypeCurrent) {
	case INST_VRC6:	return DUTY_5E01_FROM_VRC6[Duty & 0x07];
	case INST_S5B:	return 0x03;
	default:		return Duty;
	}
}

void C5E01Square::ClearRegisters()
{
	int Address = 0x4100 + m_iChannel * 4;		// // //
	WriteRegister(Address + 0, 0x30);
	WriteRegister(Address + 1, 0x08);
	WriteRegister(Address + 2, 0x00);
	WriteRegister(Address + 3, 0x00);
	m_iLastPeriod = 0xFFFF;
}

void C5E01Square::HandleNoteData(stChanNote *pNoteData, int EffColumns)
{
	m_iSweep = 0;
	m_bSweeping = false;
	CChannelHandler5E01::HandleNoteData(pNoteData, EffColumns);
}

bool C5E01Square::HandleEffect(effect_t EffNum, unsigned char EffParam)
{
	switch (EffNum) {
	case EF_SWEEPUP:
		m_iSweep = 0x88 | (EffParam & 0x77);
		m_iLastPeriod = 0xFFFF;
		m_bSweeping = true;
		break;
	case EF_SWEEPDOWN:
		m_iSweep = 0x80 | (EffParam & 0x77);
		m_iLastPeriod = 0xFFFF;
		m_bSweeping = true;
		break;
	default: return CChannelHandler5E01::HandleEffect(EffNum, EffParam);
	}

	return true;
}

void C5E01Square::HandleEmptyNote()
{
	if (m_bSweeping)
		m_cSweep = m_iSweep;
}

void C5E01Square::HandleNote(int Note, int Octave)		// // //
{
	CChannelHandler5E01::HandleNote(Note, Octave);

	if (!m_bSweeping && (m_cSweep != 0 || m_iSweep != 0)) {
		m_iSweep = 0;
		m_cSweep = 0;
		m_iLastPeriod = 0xFFFF;
	}
	else if (m_bSweeping) {
		m_cSweep = m_iSweep;
		m_iLastPeriod = 0xFFFF;
	}
}

CString C5E01Square::GetCustomEffectString() const		// // //
{
	CString str = _T("");
	
	if (!m_bEnvelopeLoop)
		str.AppendFormat(_T(" E%02X"), m_iLengthCounter);
	if (!m_bEnvelopeLoop || m_bHardwareEnvelope)
		str.AppendFormat(_T(" EE%X"), !m_bEnvelopeLoop * 2 + m_bHardwareEnvelope);

	return str;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Triangle 
///////////////////////////////////////////////////////////////////////////////////////////////////////////

C5E01WaveformChan::C5E01WaveformChan() :		// // //
	CChannelHandler5E01(),
	m_iLinearCounter(-1)
{
}

void C5E01WaveformChan::RefreshChannel()
{
	int Freq = CalculatePeriod();

	char DutyCycle = (m_iDutyPeriod & MAX_DUTY);
	unsigned char HiFreq = (Freq & 0xFF);
	unsigned char LoFreq = (Freq >> 8);

	if (m_iInstVolume > 0 && m_iVolume > 0 && m_bGate) {
		WriteRegister(0x4108, (m_bEnvelopeLoop << 7) | (m_iLinearCounter & 0x7F));		// // //
		WriteRegister(0x4109, DutyCycle);		// // //
		WriteRegister(0x410A, HiFreq);
		if (m_bEnvelopeLoop || m_bResetEnvelope)		// // //
			WriteRegister(0x410B, LoFreq + (m_iLengthCounter << 3));
	}
	else
		WriteRegister(0x4108, 0);

	m_bResetEnvelope = false;		// // //
}

void C5E01WaveformChan::ResetChannel()
{
	CChannelHandler5E01::ResetChannel();
	m_iLinearCounter = -1;
}

int C5E01WaveformChan::GetChannelVolume() const
{
	return m_iVolume ? VOL_COLUMN_MAX : 0;
}

const char C5E01WaveformChan::MAX_DUTY = 0x03;

int C5E01WaveformChan::getDutyMax() const {
	return static_cast<int>(MAX_DUTY);
}

bool C5E01WaveformChan::HandleEffect(effect_t EffNum, unsigned char EffParam)
{
	switch (EffNum) {
	case EF_VOLUME:
		if (EffParam < 0x20) {		// // //
			m_iLengthCounter = EffParam;
			m_bEnvelopeLoop = false;
			m_bResetEnvelope = true;
			if (m_iLinearCounter == -1)	m_iLinearCounter = 0x7F;
		}
		else if (EffParam >= 0xE0 && EffParam < 0xE4) {
			if (!m_bEnvelopeLoop)
				m_bResetEnvelope = true;
			m_bEnvelopeLoop = ((EffParam & 0x01) != 0x01);
		}
		break;
	case EF_NOTE_CUT:
		if (EffParam >= 0x80) {
			m_iLinearCounter = EffParam - 0x80;
			m_bEnvelopeLoop = false;
			m_bResetEnvelope = true;
		}
		else {
			m_bEnvelopeLoop = true;
			return CChannelHandler5E01::HandleEffect(EffNum, EffParam); // true
		}
		break;
	default: return CChannelHandler5E01::HandleEffect(EffNum, EffParam);
	}

	return true;
}

void C5E01WaveformChan::ClearRegisters()
{
	WriteRegister(0x4108, 0);
	WriteRegister(0x4109, 0);
	WriteRegister(0x410A, 0);
	WriteRegister(0x410B, 0);
}

CString C5E01WaveformChan::GetCustomEffectString() const		// // //
{
	CString str = _T("");
	
	if (m_iLinearCounter > -1)
		str.AppendFormat(_T(" S%02X"), m_iLinearCounter | 0x80);
	if (!m_bEnvelopeLoop)
		str.AppendFormat(_T(" E%02X"), m_iLengthCounter);
	if (!m_bEnvelopeLoop)
		str.AppendFormat(_T(" EE%X"), !m_bEnvelopeLoop);

	return str;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Noise
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void C5E01NoiseChan::HandleNote(int Note, int Octave)
{
	CChannelHandler5E01::HandleNote(Note, Octave);		// // //

	int NewNote = (MIDI_NOTE(Octave, Note) & 0x1F) | 0x100;
	int NesFreq = TriggerNote(NewNote);

	// // // NesFreq = (NesFreq & 0x0F) | 0x10;

	// // // NewNote &= 0x0F;

	if (m_iPortaSpeed > 0 && m_iEffect == EF_PORTAMENTO) {
		if (m_iPeriod == 0)
			m_iPeriod = NesFreq;
		m_iPortaTo = NesFreq;
	}
	else
		m_iPeriod = NesFreq;

	m_bGate = true;

	m_iNote			= NewNote;
}

void C5E01NoiseChan::SetupSlide()		// // //
{
	#define GET_SLIDE_SPEED(x) (((x & 0xF0) >> 3) + 1)

	switch (m_iEffect) {
	case EF_PORTAMENTO:
		m_iPortaSpeed = m_iEffectParam;
		break;
	case EF_SLIDE_UP:
		m_iNote += (m_iEffectParam & 0xF);
		m_iPortaSpeed = GET_SLIDE_SPEED(m_iEffectParam);
		break;
	case EF_SLIDE_DOWN:
		m_iNote -= (m_iEffectParam & 0xF);
		m_iPortaSpeed = GET_SLIDE_SPEED(m_iEffectParam);
		break;
	}

	#undef GET_SLIDE_SPEED

	RegisterKeyState(m_iNote);
	m_iPortaTo = m_iNote;
}

int C5E01NoiseChan::LimitPeriod(int Period) const		// // //
{
	return Period; // no limit
}

int C5E01NoiseChan::LimitRawPeriod(int Period) const		// // //
{
	return Period; // no limit
}

const char C5E01NoiseChan::MAX_DUTY = 0x01;

int C5E01NoiseChan::getDutyMax() const {
	return MAX_DUTY;
}

void C5E01NoiseChan::RefreshChannel()
{
	int Period = CalculatePeriod();
	int Volume = CalculateVolume();
	char NoiseMode = (m_iDutyPeriod & MAX_DUTY) << 7;

	Period = Period & 0x1F;
	Period ^= 0x1F;
	
	if (m_bGate)		// // //
		WriteRegister(0x410C, (m_bEnvelopeLoop << 5) | (!m_bHardwareEnvelope << 4) | Volume);		// // //
	else {
		WriteRegister(0x410C, 0x30);
		return;
	}
	WriteRegister(0x410E, NoiseMode | Period);
	if (m_bEnvelopeLoop || m_bResetEnvelope)		// // //
		WriteRegister(0x410F, m_iLengthCounter << 3);

	m_bResetEnvelope = false;		// // //
}

void C5E01NoiseChan::ClearRegisters()
{
	WriteRegister(0x410C, 0x30);
	WriteRegister(0x410E, 0);
	WriteRegister(0x410F, 0);
}

CString C5E01NoiseChan::GetCustomEffectString() const		// // //
{
	CString str = _T("");

	if (!m_bEnvelopeLoop)
		str.AppendFormat(_T(" E%02X"), m_iLengthCounter);
	if (!m_bEnvelopeLoop || m_bHardwareEnvelope)
		str.AppendFormat(_T(" EE%X"), !m_bEnvelopeLoop * 2 + m_bHardwareEnvelope);

	return str;
}

int C5E01NoiseChan::TriggerNote(int Note)
{
	RegisterKeyState(Note);
	return Note | 0x100;		// // //
}
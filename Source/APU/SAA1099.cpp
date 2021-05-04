/*
** FamiTracker - NES/Famicom sound tracker
** Copyright (C) 2005-2015  Jonathan Liss
**
** 0CC-FamiTracker is (C) 2014-2016 HertzDevil
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

#include <algorithm>
#include "APU.h"
#include "SAA1099.h"
#include "../RegisterState.h"

// // // 050B
// Sunsoft 5B channel class


const int clock_divider = 256;

const int LEFT = 0x00;
const int RIGHT = 0x01;

const int32_t amplitude_lookup[16] = {
		0 * 32767 / 16,  1 * 32767 / 16,  2 * 32767 / 16,   3 * 32767 / 16,
		4 * 32767 / 16,  5 * 32767 / 16,  6 * 32767 / 16,   7 * 32767 / 16,
		8 * 32767 / 16,  9 * 32767 / 16, 10 * 32767 / 16, 11 * 32767 / 16,
	12 * 32767 / 16, 13 * 32767 / 16, 14 * 32767 / 16, 15 * 32767 / 16
};

const int32_t envelope[8][64] = {
	/* zero amplitude */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* maximum amplitude */
	{15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
		15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15, },
	/* single decay */
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* repetitive decay */
	{15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
	/* single triangular */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* repetitive triangular */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
	/* single attack */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	/* repetitive attack */
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 }
};

CSAA1099Channel::CSAA1099Channel(CMixer *pMixer, uint8_t ID) : CChannel(pMixer, SNDCHIP_SAA1099, ID),
	m_iVolume(0),
	m_iPeriod(0),
	m_iPeriodClock(0),
	m_iDutyCycle(0),
	m_iDutyCycleCounter(0),
	m_bEnvelopeHold(true),
	m_bSquareHigh(false),
	m_bSquareDisable(false),
	m_bNoiseDisable(false)
{
}

void CSAA1099Channel::Process(uint32_t Time, uint8_t &m_iEnvelopeALevel, uint8_t m_iEnvelopeABits, uint8_t& m_iEnvelopeBLevel, uint8_t m_iEnvelopeBBits)
{
	m_iPeriodClock += Time;
	if (m_iPeriodClock >= m_iPeriod) {
		m_iPeriodClock = 0;
		m_iDutyCycle = (m_iDutyCycle + 1) & 1;
		switch (m_iChanId) {
		case CHANID_SAA1099_CH2:
			m_iEnvelopeALevel = ((m_iEnvelopeALevel + 1 + m_iEnvelopeABits) & 0x3f) | (m_iEnvelopeALevel & 0x20);
			break;
		case CHANID_SAA1099_CH5:
			m_iEnvelopeBLevel = ((m_iEnvelopeBLevel + 1 + m_iEnvelopeBBits) & 0x3f) | (m_iEnvelopeBLevel & 0x20);
			break;
		}
	}
	m_iTime += Time;
}


void CSAA1099Channel::Reset()
{
	m_iVolume = 0;
	m_iPeriod = 0;
	m_iPeriodClock = 0;
	m_iOctave = 0;
	m_iOffset = 0;
	m_iDutyCycle = 0;
	m_bSquareHigh = false;
	m_bSquareDisable = true;
	m_bNoiseDisable = true;
	m_bEnvelopeHold = true;
}

uint32_t CSAA1099Channel::GetTime()
{
	if (m_iPeriod < 2U)
		return 0xFFFFFU;
	return m_iPeriod - m_iPeriodClock;
}

void CSAA1099Channel::Output(uint32_t NoiseA, uint32_t NoiseB, uint8_t LevelA, uint8_t LevelB)
{
	int Level = m_iVolume;
	int EnvelopeLevel = (m_iChanId == CHANID_SAA1099_CH3) ? LevelA : 16;
	EnvelopeLevel = (m_iChanId == CHANID_SAA1099_CH6) ? LevelB : EnvelopeLevel;
	int32_t Output = Level * EnvelopeLevel;
	if (!m_bSquareDisable && !(m_iDutyCycle & 1) && m_iPeriod >= 2U)
	  Output = 0;
	if (!m_bNoiseDisable && !(NoiseA & 1) && m_iChanId >= CHANID_SAA1099_CH1 && m_iChanId <= CHANID_SAA1099_CH3)
		Output = 0;
	else if (!m_bNoiseDisable && !(NoiseB & 1) && m_iChanId >= CHANID_SAA1099_CH4 && m_iChanId <= CHANID_SAA1099_CH6)
		Output = 0;
	Mix(Output);
}

double CSAA1099Channel::GetFrequency() const		// // //
{
	if (m_bSquareDisable || !m_iPeriod)
		return 0.;
	return CAPU::BASE_FREQ_NTSC / 2. / m_iPeriod;
}



// SAA1099 chip class

CSAA1099::CSAA1099(CMixer *pMixer) : CSoundChip(pMixer),
	m_cPort(0),
	m_iCounter(0)
{
	m_pRegisterLogger->AddRegisterRange(0x00, 0x1F);		// // //

	m_pChannel[0] = new CSAA1099Channel(pMixer, CHANID_SAA1099_CH1);
	m_pChannel[1] = new CSAA1099Channel(pMixer, CHANID_SAA1099_CH2);
	m_pChannel[2] = new CSAA1099Channel(pMixer, CHANID_SAA1099_CH3);
	m_pChannel[3] = new CSAA1099Channel(pMixer, CHANID_SAA1099_CH4);
	m_pChannel[4] = new CSAA1099Channel(pMixer, CHANID_SAA1099_CH5);
	m_pChannel[5] = new CSAA1099Channel(pMixer, CHANID_SAA1099_CH6);
	Reset();
}

CSAA1099::~CSAA1099()
{
	for (auto x : m_pChannel)
		if (x)
			delete x;
}

void CSAA1099::Reset()
{
	m_iCounter = 0;


	m_iNoiseAClock = 0;
	m_iNoiseAState = 0x1FFFF;
	m_iNoiseAMode = 0;

	m_iNoiseBClock = 0;
	m_iNoiseBState = 0x1FFFF;
	m_iNoiseBMode = 0;

	m_iEnvelopeAMode = 0;
	m_iEnvelopeABits = 0;
	m_iEnvelopeAEnable = 0;
	m_iEnvelopeALevel = 0;

	m_iEnvelopeBMode = 0;
	m_iEnvelopeBBits = 0;
	m_iEnvelopeBEnable = 0;
	m_iEnvelopeBLevel = 0;
	
	for (auto x : m_pChannel)
		x->Reset();
}

void CSAA1099::Process(uint32_t Time)
{
	while (Time > 0U) {
		uint32_t TimeToRun = Time;


		uint32_t NoiseAPeriod = m_iNoiseAMode < 3 ? CalcPeriod(9 - m_iNoiseAMode, 255) : m_pChannel[0]->m_iPeriod;
		uint32_t NoiseBPeriod = m_iNoiseBMode < 3 ? CalcPeriod(9 - m_iNoiseBMode, 255) : m_pChannel[3]->m_iPeriod;

		if (m_iNoiseAClock < NoiseAPeriod)
			TimeToRun = std::min<uint32_t>(NoiseAPeriod - m_iNoiseAClock, TimeToRun);
		if (m_iNoiseBClock < NoiseBPeriod)
			TimeToRun = std::min<uint32_t>(NoiseBPeriod - m_iNoiseBClock, TimeToRun);
		for (const auto x : m_pChannel)
			TimeToRun = std::min<uint32_t>(x->GetTime(), TimeToRun);

		m_iCounter += TimeToRun;
		Time -= TimeToRun;

		RunNoiseA(TimeToRun);
		RunNoiseB(TimeToRun);

		for (auto x : m_pChannel)
			x->Process(TimeToRun, m_iEnvelopeALevel, m_iEnvelopeABits, m_iEnvelopeBLevel, m_iEnvelopeBBits);


		uint8_t LevelA = m_iEnvelopeAEnable ? envelope[m_iEnvelopeAMode][m_iEnvelopeALevel] : 16;
		uint8_t LevelB = m_iEnvelopeBEnable ? envelope[m_iEnvelopeBMode][m_iEnvelopeBLevel] : 16;

		for (auto x : m_pChannel)
			x->Output(m_iNoiseAState & 0x01, m_iNoiseBState & 0x01, LevelA, LevelB);

	}
}

void CSAA1099::EndFrame()
{
	for (auto x : m_pChannel)
		x->EndFrame();
	m_iCounter = 0;
}

void CSAA1099::Write(uint16_t Address, uint8_t Value)
{
	switch (Address) {
	case 0xC002:
		m_cPort = Value & 0x1F;
		break;
	case 0xE002:
		WriteReg(m_cPort, Value);
		break;
	}
}

uint8_t CSAA1099::Read(uint16_t Address, bool &Mapped)
{
	Mapped = false;
	return 0U;
}

double CSAA1099::GetFreq(int Channel) const		// // //
{
	switch (Channel) {
	case 0: case 1: case 2: case 3: case 4: case 5:
		return m_pChannel[Channel]->GetFrequency();
	//case 3:
	//	if (!m_pChannel[0]->m_iEnvelopePeriod)
	//		return 0.;
	//	if (!(m_pChannel[0]->m_iEnvelopeShape & 0x08) || (m_pChannel[0]->m_iEnvelopeShape & 0x01))
	//		return 0.;
	//	return CAPU::BASE_FREQ_NTSC / ((m_pChannel[0]->m_iEnvelopeShape & 0x02) ? 64. : 32.) / m_pChannel[0]->m_iEnvelopePeriod;
	//case 4: TODO noise refresh rate
	}
	return 0.;
}

void CSAA1099::WriteReg(uint8_t Port, uint8_t Value)
{
	int ch;
	switch (Port) {
	/* channel i amplitude */
	case 0x00:  case 0x01:  case 0x02:  case 0x03:  case 0x04:  case 0x05:
		ch = Port & 7;
		m_pChannel[ch]->m_iVolume = Value & 0x0f;
		//m_pChannel[ch].amplitude[RIGHT] = amplitude_lookup[(data >> 4) & 0x0f];
		break;

	/* channel i frequency */
	case 0x08:  case 0x09:  case 0x0a:  case 0x0b:  case 0x0c:  case 0x0d:
		m_pChannel[Port & 7]->m_iOffset = Value & 0xff;
		m_pChannel[Port & 7]->m_iPeriod = CalcPeriod(m_pChannel[Port & 7]->m_iOctave, m_pChannel[Port & 7]->m_iOffset);
		break;

	/* channel i octave */
	case 0x10:  case 0x11:  case 0x12:
		ch = (Port - 0x10) << 1;
		m_pChannel[ch + 0]->m_iOctave = (Value & 0x07);
		m_pChannel[ch + 1]->m_iOctave = ((Value >> 4) & 0x07);
		m_pChannel[ch + 0]->m_iPeriod = CalcPeriod(m_pChannel[ch + 0]->m_iOctave, m_pChannel[ch + 0]->m_iOffset);
		m_pChannel[ch + 1]->m_iPeriod = CalcPeriod(m_pChannel[ch + 1]->m_iOctave, m_pChannel[ch + 1]->m_iOffset);
		break;

	/* channel i frequency enable */
	case 0x14:
		for (ch = 0; ch < 6; ch++)
			m_pChannel[ch]->m_bSquareDisable = Value & (0x01 << ch);
		break;

	/* channel i noise enable */
	case 0x15:
		for (ch = 0; ch < 6; ch++)
			m_pChannel[ch]->m_bNoiseDisable = Value & (0x01 << ch);
		break;

	/* noise generators parameters */
	case 0x16:
		m_iNoiseAMode = Value & 0x03;
		m_iNoiseBMode = (Value >> 4) & 0x03;
		break;

	/* envelope generators parameters */
	case 0x18:
		//m_env_reverse_right[ch] = BIT(data, 0);
		m_iEnvelopeAMode   = (Value >> 1) & 0x07;
		m_iEnvelopeABits   = (Value & 0b00010000) >> 4;
		m_iEnvelopeAEnable = (Value & 0b10000000) >> 7;
		/* reset the envelope */
		m_iEnvelopeALevel = m_iEnvelopeAEnable ? m_iEnvelopeALevel : 0;
		break;
	case 0x19:
		//m_env_reverse_right[ch] = BIT(data, 0);
		m_iEnvelopeBMode   = (Value >> 1) & 0x07;
		m_iEnvelopeBBits   = (Value & 0b00010000) >> 4;
		m_iEnvelopeBEnable = (Value & 0b10000000) >> 7;
		/* reset the envelope */
		m_iEnvelopeBLevel = m_iEnvelopeBEnable ? m_iEnvelopeBLevel : 0;
		break;
	}
}

void CSAA1099::Log(uint16_t Address, uint8_t Value)		// // //
{
	switch (Address) {
	case 0xC002: m_pRegisterLogger->SetPort(Value); break;
	case 0xE002: m_pRegisterLogger->Write(Value); break;
	}
}

void CSAA1099::RunNoiseA(uint32_t Time)
{
	m_iNoiseAClock += Time;

	uint32_t NoiseAPeriod = m_iNoiseAMode < 3 ? CalcPeriod(9 - m_iNoiseAMode, 255) : m_pChannel[0]->m_iPeriod;
	while (m_iNoiseAClock >= NoiseAPeriod && NoiseAPeriod > 0) {
		m_iNoiseAClock -= NoiseAPeriod;
		if (((m_iNoiseAState & 0x20000) == 0) != ((m_iNoiseAState & 0x0400) == 0))
			m_iNoiseAState = (m_iNoiseAState << 1) | 1;
		else
			m_iNoiseAState <<= 1;
	}
}

void CSAA1099::RunNoiseB(uint32_t Time)
{
	m_iNoiseBClock += Time;

	uint32_t NoiseBPeriod = m_iNoiseBMode < 3 ? CalcPeriod(9 - m_iNoiseBMode, 255) : m_pChannel[3]->m_iPeriod;
	while (m_iNoiseBClock >= NoiseBPeriod && NoiseBPeriod > 0) {
		m_iNoiseBClock -= NoiseBPeriod;
		if (((m_iNoiseBState & 0x20000) == 0) != ((m_iNoiseBState & 0x0400) == 0))
			m_iNoiseBState = (m_iNoiseBState << 1) | 1;
		else
			m_iNoiseBState <<= 1;
	}
}


uint32_t CSAA1099::CalcPeriod(uint8_t m_iOctave, uint8_t m_iOffset)
{
	return (uint32_t)(CAPU::BASE_FREQ_NTSC * (511.0 - double(m_iOffset)) / (15625.0 * double(1 << m_iOctave)));
}
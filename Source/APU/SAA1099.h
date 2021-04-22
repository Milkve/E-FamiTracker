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


#pragma once

#include "SoundChip.h"
#include "Channel.h"

// // // 050B

class CSAA1099Channel : public CChannel
{
public:
	friend class CSAA1099;

	CSAA1099Channel(CMixer *pMixer, uint8_t ID);

	void Process(uint32_t Time, uint8_t& m_iEnvelopeALevel, uint8_t m_iEnvelopeABits, uint8_t& m_iEnvelopeBLevel, uint8_t m_iEnvelopeBBits);
	void Reset();

	uint32_t GetTime();
	void Output(uint32_t NoiseA, uint32_t NoiseB, uint8_t LevelA, uint8_t LevelB);

	double GetFrequency() const;

private:
	uint8_t m_iVolume;
	uint32_t m_iPeriod;
	uint32_t m_iPeriodClock;

	uint8_t m_iOctave;
	uint8_t m_iOffset;

	uint8_t m_iDutyCycleCounter;
	uint8_t m_iDutyCycle;

	bool m_bSquareHigh;
	bool m_bSquareDisable;
	bool m_bNoiseDisable;


	bool m_bEnvelopeHold;
};

class CSAA1099 : public CSoundChip
{
public:
	CSAA1099(CMixer *pMixer);
	virtual ~CSAA1099();
	
	void	Reset();
	void	Process(uint32_t Time);
	void	EndFrame();

	void	Write(uint16_t Address, uint8_t Value);
	uint8_t	Read(uint16_t Address, bool &Mapped);
	void	Log(uint16_t Address, uint8_t Value);		// // //

	double	GetFreq(int Channel) const;		// // //

private:
	void	WriteReg(uint8_t Port, uint8_t Value);
	void  RunNoiseA(uint32_t Time);
	void  RunNoiseB(uint32_t Time);
	uint32_t CalcPeriod(uint8_t m_iOctave, uint8_t m_iOffset);

private:
	CSAA1099Channel *m_pChannel[6];

	uint8_t m_cPort;

	int m_iCounter;

	uint32_t m_iNoiseAState;
	uint32_t m_iNoiseAClock;
	uint8_t  m_iNoiseAMode;
	uint32_t m_iNoiseBState;
	uint32_t m_iNoiseBClock;
	uint8_t  m_iNoiseBMode;


	uint8_t m_iEnvelopeAMode;
	uint8_t m_iEnvelopeABits;
	uint8_t m_iEnvelopeAEnable;
	uint8_t m_iEnvelopeALevel;
	uint8_t m_iEnvelopeBMode;
	uint8_t m_iEnvelopeBBits;
	uint8_t m_iEnvelopeBEnable;
	uint8_t m_iEnvelopeBLevel;

};

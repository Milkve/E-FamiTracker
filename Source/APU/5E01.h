/*
** FamiTracker - I5E01/Famicom sound tracker
** Copyright (C) 2005-2014  Jonathan Liss
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
** MERCHANTABILITY or FITI5E01S FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/


#pragma once

#include "SoundChip2.h"
#include "ChannelLevelState.h"

#include "APU/nsfplay/xgm/devices/Sound/5e01_apu.h"
#include "APU/nsfplay/xgm/devices/Sound/5e01_dmc.h"
#include "APU/nsfplay/xgm/devices/device.h"

class C5E01 : public CSoundChip2
{
public:
	C5E01();

	void Reset() override;
	void UpdateFilter(blip_eq_t eq) override;
	void Process(uint32_t Time, Blip_Buffer& Output) override;
	void EndFrame(Blip_Buffer& Output, gsl::span<int16_t> TempBuffer) override;

	void Write(uint16_t Address, uint8_t Value) override;
	uint8_t Read(uint16_t Address, bool &Mapped) override;

	double GetFreq(int Channel) const override;		// // //
	int GetChannelLevel(int Channel) override;
	int GetChannelLevelRange(int Channel) const override;

public:
	void UpdateMixingAPU1(double v);
	void UpdateMixingAPU2(double v);

	void	ClockSequence();		// // //
	
	void	ChangeMachine(int Machine);
	
	//CSampleMem *GetSampleMemory();		// // //
	//uint8_t	GetSamplePos() const;
	//uint8_t	GetDeltaCounter() const;
	//bool	DPCMPlaying() const;

private:
	/// Referenced by m_Apu2.
	//CSampleMem m_SampleMem;

	xgm::I5E01_APU m_Apu1;
	xgm::I5E01_DMC m_Apu2;

	// [0..4] correspond to Pulse 1, Pulse 2, Triangle, Noise, and DPCM.
	ChannelLevelState<uint8_t> m_ChannelLevels[5];

	Blip_Synth<blip_good_quality> Synth5E01SS;
	Blip_Synth<blip_good_quality> Synth5E01TND;

	uint32_t	m_iTime = 0;  // Clock counter, used as a timestamp for Blip_Buffer, resets every new frame
};

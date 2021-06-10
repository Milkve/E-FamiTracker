/*
** FamiTracker - NES/Famicom sound tracker
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
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#include "../stdafx.h"
#include "../Common.h"
#include <algorithm>  // std::min
#include "APU.h"
#include "6581.h"
#include "../RegisterState.h"		// // //
#include "utils/variadic_minmax.h"
#include "resid/sid.h"		// // !!

// // // 6581 sound chip class

C6581::C6581() {
	m_pRegisterLogger->AddRegisterRange(0xD400, 0xD41C);		// // //

	m_Sid.set_chip_model(MOS6581);

	// Reset() is called by CAPU::SetExternalSound(), but let's call it ourself.
	Reset();
}

// TODO set range of Blip_Synth. Copy whatever CMixer does.

void C6581::Reset()
{
	m_Sid.reset();
	Synth6581.clear();

}

void C6581::UpdateFilter(blip_eq_t eq)
{
	Synth6581.treble_eq(eq);
}

void C6581::Process(uint32_t Time, Blip_Buffer& Output)
{
	uint32_t now = 0;

	auto get_output = [this](uint32_t dclocks, uint32_t now, Blip_Buffer& blip_buf) {
		m_Sid.clock(dclocks);

		int32_t out = m_Sid.output(8);
		Synth6581.update(m_iTime + now, out, &blip_buf);

		// channel levels
		m_ChannelLevels[0].update((uint8_t)(m_Sid.voice[0].output() * 0.000244140625 + 127.5));
		m_ChannelLevels[1].update((uint8_t)(m_Sid.voice[1].output() * 0.000244140625 + 127.5));
		m_ChannelLevels[2].update((uint8_t)(m_Sid.voice[2].output() * 0.000244140625 + 127.5));
	};

	while (now < Time) {
		// Due to how nsfplay is implemented, when ClocksUntilLevelChange() is used,
		// the result of `Tick(clocks); Render()` should be sent to Blip_Synth
		// at the instant in time *before* Tick() is called.
		// See https://docs.google.com/document/d/1BnXwR3Avol7S5YNa3d4duGdbI6GNMwuYWLHuYiMZh5Y/edit#heading=h.lnh9d8j1x3uc
		auto dclocks = 1;//Time - now, 1);
		get_output(dclocks, now, Output);
		now += dclocks;
	}

	m_iTime += Time;
}

void C6581::EndFrame(Blip_Buffer&, gsl::span<int16_t>)
{
	m_iTime = 0;
}

void C6581::Write(uint16_t Address, uint8_t Value)
{
	if (Address >= 0xD400 && Address <= 0xD41C)
		m_Sid.write(Address - 0xD400, Value);
}

uint8_t C6581::Read(uint16_t Address, bool &Mapped)
{
	if (Address >= 0xD400 && Address <= 0xD41C)
		return m_Sid.read(Address - 0xD400);
	return 0;
}

double C6581::GetFreq(int Channel) const		// // !!
{
	
	switch (Channel) {
	case 0: case 1: case 2:
		return 0;//Read(0xD400 + Channel * 7, true) * 0.0596;
	}
	return 0.0;
}

int C6581::GetChannelLevel(int Channel)
{
	if (0 <= Channel && Channel < 3) {
		return m_ChannelLevels[Channel].getLevel();
	}
	return 0;
}

int C6581::GetChannelLevelRange(int Channel) const
{
	switch (Channel) {
	case 0: case 1: case 2:
		return 255;

	default:
		// unknown channel, return 1 to avoid division by 0
		return 1;
	}
}


void C6581::UpdateMix(double v) {
	Synth6581.volume(v, 10000);
}


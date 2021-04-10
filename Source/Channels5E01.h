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

#pragma once

//
// Derived channels, 5E01
//

class CChannelHandler5E01 : public CChannelHandler {
public:
	CChannelHandler5E01();
	virtual void ResetChannel();

protected:
	void	HandleNoteData(stChanNote *pNoteData, int EffColumns) override;
	bool	HandleEffect(effect_t EffNum, unsigned char EffParam) override;		// // //
	void	HandleEmptyNote() override;
	void	HandleCut() override;
	void	HandleRelease() override;
	bool	CreateInstHandler(inst_type_t Type) override;		// // //

protected:
	// // //
	bool	m_bHardwareEnvelope;	// // // (constant volume flag, bit 4)
	bool	m_bEnvelopeLoop;		// // // (halt length counter flag, bit 5 / triangle bit 7)
	bool	m_bResetEnvelope;		// // //
	int		m_iLengthCounter;		// // //
};

// // // 5E01 Square
class C5E01Square : public CChannelHandler5E01 {
public:
	C5E01Square();
	void	RefreshChannel() override;
	void	SetChannelID(int ID) override;		// // //
	int getDutyMax() const override;
protected:
	static const char MAX_DUTY;

	int		ConvertDuty(int Duty) const override;		// // //
	void	ClearRegisters() override;

	void	HandleNoteData(stChanNote *pNoteData, int EffColumns) override;
	bool	HandleEffect(effect_t EffNum, unsigned char EffParam) override;		// // //
	void	HandleEmptyNote() override;
	void	HandleNote(int Note, int Octave) override;
	CString	GetCustomEffectString() const override;		// // //

	unsigned char m_iChannel;		// // //
	unsigned char m_cSweep;
	bool	m_bSweeping;
	int		m_iSweep;
	int		m_iLastPeriod;
};

// Triangle
class C5E01WaveformChan : public CChannelHandler5E01 {
public:
	C5E01WaveformChan();
	void	RefreshChannel() override;
	void	ResetChannel() override;		// // //
	int		GetChannelVolume() const override;		// // //
	int   getDutyMax() const override;
protected:
	static const char MAX_DUTY;

	bool	HandleEffect(effect_t EffNum, unsigned char EffParam) override;		// // //
	void	ClearRegisters() override;
	CString	GetCustomEffectString() const override;		// // //
private:
	int m_iLinearCounter;

};

// Noise
class C5E01NoiseChan : public CChannelHandler5E01 {
public:
	void	RefreshChannel();
	int getDutyMax() const override;
protected:
	static const char MAX_DUTY;

	void	ClearRegisters() override;
	CString	GetCustomEffectString() const override;		// // //
	void	HandleNote(int Note, int Octave) override;
	void	SetupSlide() override;		// // //

	int		LimitPeriod(int Period) const override;		// // //
	int		LimitRawPeriod(int Period) const override;		// // //

	int		TriggerNote(int Note) override;
};

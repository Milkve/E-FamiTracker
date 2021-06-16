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

#pragma once


#include <vector>
#include <memory>

#include "SeqInstrument.h"		// // //

enum envparam_t {
	ENV_ATTACK,
	ENV_DECAY,
	ENV_SUSTAIN,
	ENV_RELEASE
};


enum pwmdirection_t {
	PWM_DISABLED,
	PWM_LOOP,
	PWM_PINGPONG,
	PWM_ONCE,
	PWM_SUSTAIN,
};


class CInstrumentSID : public CSeqInstrument {
public:
	CInstrumentSID();
	CInstrument* Clone() const;
	void	Setup();
	void	Store(CDocumentFile *pDocFile);
	bool	Load(CDocumentFile *pDocFile);
	void	SaveFile(CInstrumentFile *pFile);
	bool	LoadFile(CInstrumentFile *pFile, int iVersion);
	int		Compile(CChunk *pChunk, int Index);
	bool	CanRelease() const;

public:
	int  	GetEnvParam(int EnvParam) const;		// // //
	void 	SetEnvParam(int EnvParam, int Value);		// // //
	int  	GetPWMStart() const { return m_pPWMStart; };		// // //
	void 	SetPWMStart(int Value) { m_pPWMStart = Value; InstrumentChanged(); };		// // //
	int  	GetPWMEnd() const { return m_pPWMEnd; };		// // //
	void 	SetPWMEnd(int Value) { m_pPWMEnd = Value; InstrumentChanged(); };		// // //
	int  	GetPWMSpeed() const { return m_pPWMSpeed; };		// // //
	void 	SetPWMSpeed(int Value) { m_pPWMSpeed = Value; InstrumentChanged(); };		// // //
	int  	GetPWMMode() const { return m_pPWMMode; };		// // //
	void 	SetPWMMode(int Value) { m_pPWMMode = Value; InstrumentChanged(); };		// // //

protected:
	virtual void	CloneFrom(const CInstrument *pInst);		// // //

private:
	void StoreSequence(CDocumentFile *pDocFile, const CSequence *pSeq);		// // //
	CSequence *LoadSequence(CDocumentFile *pDocFile) const;
	void StoreInstSequence(CInstrumentFile *pDocFile, const CSequence *pSeq);
	CSequence *LoadInstSequence(CInstrumentFile *pFile) const;
	void DoubleVolume() const;		// // //

public:
	static const int SEQUENCE_COUNT = 6;		// // //
	static LPCTSTR SEQUENCE_NAME[];
	LPCTSTR	GetSequenceName(int Index) const { return SEQUENCE_NAME[Index]; }		// // //
	
private:
	// Instrument data
	std::vector<std::unique_ptr<CSequence>> m_pSequence;
	int m_pEnvelopeAD;
	int m_pEnvelopeSR;

	int m_pPWMStart;
	int m_pPWMEnd;
	int m_pPWMSpeed;
	int m_pPWMMode;
	
public: // // // porting CSeqInstrument
	virtual int		GetSeqEnable(int Index) const;
	virtual int		GetSeqIndex(int Index) const;
	virtual void	SetSeqIndex(int Index, int Value);
	CSequence		*GetSequence(int SeqType) const;		// // //
	virtual void	SetSequence(int SeqType, CSequence *pSeq);		// // //
};

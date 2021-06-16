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

#include <vector>
#include <memory>
#include "stdafx.h"
#include "Sequence.h"		// // //
#include "ModuleException.h"		// // //
#include "Instrument.h"
#include "SeqInstrument.h"		// // //
#include "InstrumentSID.h"		// // //
#include "Chunk.h"
#include "ChunkRenderText.h"		// // //
#include "DocumentFile.h"

// https://stackoverflow.com/a/14997413/2683842
inline int modulo(int i, int n) {
	return (i % n + n) % n;
}


LPCTSTR CInstrumentSID::SEQUENCE_NAME[] = {_T("Volume"), _T("Arpeggio"), _T("Pitch"), _T("Hi-pitch"), _T("Pulse Width"), _T("Waveform")};

CInstrumentSID::CInstrumentSID() : CSeqInstrument(INST_SID), // // //
m_pEnvelopeAD(0x0A),
m_pEnvelopeSR(0x00),
m_pPWMStart(0x800),
m_pPWMEnd(0x800),
m_pPWMSpeed(0x00),
m_pPWMMode(PWM_DISABLED)
{
	m_pSequence.resize(SEQUENCE_COUNT);
	for (int i = 0; i < SEQUENCE_COUNT; ++i)
		m_pSequence[i].reset(new CSequence());
}

CInstrument * CInstrumentSID::Clone() const
{
	CInstrumentSID*inst = new CInstrumentSID();		// // //
	inst->CloneFrom(this);
	return inst;
}

void CInstrumentSID::CloneFrom(const CInstrument *pInst)
{
	CInstrument::CloneFrom(pInst);
	
	if (auto pNew = dynamic_cast<const CInstrumentSID*>(pInst)) {
	// Copy parameters
		SetEnvParam(ENV_ATTACK, pNew->GetEnvParam(ENV_ATTACK));
		SetEnvParam(ENV_DECAY, pNew->GetEnvParam(ENV_DECAY));
		SetEnvParam(ENV_SUSTAIN, pNew->GetEnvParam(ENV_SUSTAIN));
		SetEnvParam(ENV_RELEASE, pNew->GetEnvParam(ENV_RELEASE));

		SetPWMStart(pNew->GetPWMStart());
		SetPWMEnd(pNew->GetPWMEnd());
		SetPWMSpeed(pNew->GetPWMSpeed());
		SetPWMMode(pNew->GetPWMMode());

		// Copy sequences
		for (int i = 0; i < SEQUENCE_COUNT; ++i)		// // //
			SetSequence(i, new CSequence(*pNew->GetSequence(i)));
	}
}

void CInstrumentSID::Setup()
{
}

void CInstrumentSID::StoreInstSequence(CInstrumentFile *pFile, const CSequence *pSeq)
{
	// Store number of items in this sequence
	pFile->WriteInt(pSeq->GetItemCount());
	// Store loop point
	pFile->WriteInt(pSeq->GetLoopPoint());
	// Store release point (v4)
	pFile->WriteInt(pSeq->GetReleasePoint());
	// Store setting (v4)
	pFile->WriteInt(pSeq->GetSetting());
	// Store items
	for (unsigned i = 0; i < pSeq->GetItemCount(); ++i)
		pFile->WriteChar(pSeq->GetItem(i));
}

CSequence * CInstrumentSID::LoadInstSequence(CInstrumentFile *pFile) const
{
	int SeqCount = CModuleException::AssertRangeFmt(pFile->ReadInt(), 0U, 0xFFU, "Sequence item count", "%u");
	int Loop = CModuleException::AssertRangeFmt(static_cast<int>(pFile->ReadInt()), -1, SeqCount - 1, "Sequence loop point", "%u");
	int Release = CModuleException::AssertRangeFmt(static_cast<int>(pFile->ReadInt()), -1, SeqCount - 1, "Sequence release point", "%u");

	CSequence *pSeq = new CSequence();
	pSeq->SetItemCount(SeqCount > MAX_SEQUENCE_ITEMS ? MAX_SEQUENCE_ITEMS : SeqCount);
	pSeq->SetLoopPoint(Loop);
	pSeq->SetReleasePoint(Release);
	pSeq->SetSetting(static_cast<seq_setting_t>(pFile->ReadInt()));		// // //

	for (int i = 0; i < SeqCount; ++i)
		pSeq->SetItem(i, pFile->ReadChar());

	return pSeq;
}

void CInstrumentSID::StoreSequence(CDocumentFile *pDocFile, const CSequence *pSeq)
{
	// Store number of items in this sequence
	pDocFile->WriteBlockChar(pSeq->GetItemCount());
	// Store loop point
	pDocFile->WriteBlockInt(pSeq->GetLoopPoint());
	// Store release point (v4)
	pDocFile->WriteBlockInt(pSeq->GetReleasePoint());
	// Store setting (v4)
	pDocFile->WriteBlockInt(pSeq->GetSetting());
	// Store items
	for (unsigned int j = 0; j < pSeq->GetItemCount(); j++) {
		pDocFile->WriteBlockChar(pSeq->GetItem(j));
	}
}

CSequence * CInstrumentSID::LoadSequence(CDocumentFile *pDocFile) const
{
	int SeqCount = static_cast<unsigned char>(pDocFile->GetBlockChar());
	unsigned int LoopPoint = CModuleException::AssertRangeFmt(pDocFile->GetBlockInt(), -1, SeqCount - 1, "Sequence loop point", "%i");;
	unsigned int ReleasePoint = CModuleException::AssertRangeFmt(pDocFile->GetBlockInt(), -1, SeqCount - 1, "Sequence release point", "%i");;

	// CModuleException::AssertRangeFmt(SeqCount, 0, MAX_SEQUENCE_ITEMS, "Sequence item count", "%i");

	CSequence *pSeq = new CSequence();
	pSeq->SetItemCount(SeqCount > MAX_SEQUENCE_ITEMS ? MAX_SEQUENCE_ITEMS : SeqCount);
	pSeq->SetLoopPoint(LoopPoint);
	pSeq->SetReleasePoint(ReleasePoint);
	pSeq->SetSetting(static_cast<seq_setting_t>(pDocFile->GetBlockInt()));		// // //

	for (int x = 0; x < SeqCount; ++x) {
		char Value = pDocFile->GetBlockChar();
		pSeq->SetItem(x, Value);
	}

	return pSeq;
}

void CInstrumentSID::DoubleVolume() const
{
	//CSequence *pVol = m_pSequence[SEQ_VOLUME].get();
	//for (unsigned int i = 0; i < pVol->GetItemCount(); ++i)
		//pVol->SetItem(i, pVol->GetItem(i) * 2);
}

void CInstrumentSID::Store(CDocumentFile *pDocFile)
{
	pDocFile->WriteBlockInt(2);
	pDocFile->WriteBlockChar(m_pEnvelopeAD);
	pDocFile->WriteBlockChar(m_pEnvelopeSR);
	pDocFile->WriteBlockInt(m_pPWMStart);
	pDocFile->WriteBlockInt(m_pPWMEnd);
	pDocFile->WriteBlockChar(m_pPWMSpeed);
	pDocFile->WriteBlockChar(m_pPWMMode);
	// Sequences
	for (int i = 0; i < SEQUENCE_COUNT; ++i)		// // //
		StoreSequence(pDocFile, GetSequence(i));
}

bool CInstrumentSID::Load(CDocumentFile *pDocFile)
{
	
	unsigned int instversion = pDocFile->GetBlockInt();
	if (instversion <= 255) {
		m_pEnvelopeAD = pDocFile->GetBlockChar();
		m_pEnvelopeSR = pDocFile->GetBlockChar();
		if (instversion >= 2) {
			m_pPWMStart = pDocFile->GetBlockInt();
			m_pPWMEnd = pDocFile->GetBlockInt();
			m_pPWMSpeed = pDocFile->GetBlockChar();
			m_pPWMMode = pDocFile->GetBlockChar();
		}
		LoadSequence(pDocFile);
		LoadSequence(pDocFile);
		LoadSequence(pDocFile);
		LoadSequence(pDocFile);
		LoadSequence(pDocFile);
		LoadSequence(pDocFile);
	} else {
		pDocFile->RollbackPointer(4);
		unsigned int a = pDocFile->GetBlockInt();
		unsigned int b = pDocFile->GetBlockInt();
		pDocFile->RollbackPointer(8);
		if (a < 256 && (b & 0xFF) != 0x00) {
		}
		else {
			SetSequence(SEQ_VOLUME, LoadSequence(pDocFile));
			SetSequence(SEQ_ARPEGGIO, LoadSequence(pDocFile));
			SetSequence(SEQ_PITCH, LoadSequence(pDocFile));
		}
	}

//	}

	// Older files was 0-15, new is 0-31
	//if (pDocFile->GetBlockVersion() <= 3) DoubleVolume();

	return true;
}

void CInstrumentSID::SaveFile(CInstrumentFile *pFile)
{

	// Sequences
	for (int i = 0; i < SEQUENCE_COUNT; ++i)
		StoreInstSequence(pFile, GetSequence(i));
}

bool CInstrumentSID::LoadFile(CInstrumentFile *pFile, int iVersion)
{
	// Sequences
	for (int i = 0; i < SEQUENCE_COUNT; ++i)		// // //
		SetSequence(i, LoadInstSequence(pFile));

	if (iVersion <= 22) DoubleVolume();

	return true;
}

int CInstrumentSID::Compile(CChunk *pChunk, int Index)
{
	return 0;
}

bool CInstrumentSID::CanRelease() const
{
	const CSequence *pVol = GetSequence(SEQ_VOLUME);
	return pVol && pVol->GetItemCount() && pVol->GetReleasePoint() != -1;
}

int	CInstrumentSID::GetSeqEnable(int Index) const
{
	return m_iSeqEnable[Index];
}

int	CInstrumentSID::GetSeqIndex(int Index) const
{
	ASSERT(false);
	return 0;
}

void CInstrumentSID::SetSeqIndex(int Index, int Value)
{
	ASSERT(false);
}

CSequence * CInstrumentSID::GetSequence(int SeqType) const		// // //
{
	return m_pSequence[SeqType].get();
}

void CInstrumentSID::SetSequence(int SeqType, CSequence *pSeq)
{
	m_pSequence[SeqType].reset(pSeq);
}




int CInstrumentSID::GetEnvParam(int EnvParam) const
{
	switch (EnvParam) {
	case ENV_ATTACK:
		return (m_pEnvelopeAD & 0xF0) >> 4;
	case ENV_DECAY:
		return (m_pEnvelopeAD & 0x0F);
	case ENV_SUSTAIN:
		return (m_pEnvelopeSR & 0xF0) >> 4;
	case ENV_RELEASE:
		return (m_pEnvelopeSR & 0x0F);
	}
	return 0;
}

void CInstrumentSID::SetEnvParam(int EnvParam, int Value)
{
	switch (EnvParam) {
	case ENV_ATTACK:
		m_pEnvelopeAD = (m_pEnvelopeAD & 0x0F) | ((Value & 0x0F) << 4);
		break;
	case ENV_DECAY:
		m_pEnvelopeAD = (m_pEnvelopeAD & 0xF0) | (Value & 0x0F);
		break;
	case ENV_SUSTAIN:
		m_pEnvelopeSR = (m_pEnvelopeSR & 0x0F) | ((Value & 0x0F) << 4);
		break;
	case ENV_RELEASE:
		m_pEnvelopeSR = (m_pEnvelopeSR & 0xF0) | (Value & 0x0F);
		break;
	}
	InstrumentChanged();
}


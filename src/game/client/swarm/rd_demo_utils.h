#pragma once

#include "demofile/demoformat.h"

struct RD_Mission_t;
struct RD_Demo_Info_t
{
	char szFileName[MAX_PATH];
	unsigned int nFileSize;
	demoheader_t Header;
	const RD_Mission_t *pMission;
	wchar_t wszCantWatchReason[256];
};

class CRD_Auto_Record_System final : public CAutoGameSystemPerFrame
{
public:
	CRD_Auto_Record_System();

	void LevelInitPostEntity();
	void LevelShutdownPreEntity();
	void Update( float frametime ) override;

	void CleanRecordingsFolder( bool bLeaveEmptySlot );
	float RecomputeDemoDuration( const char *szName, bool bForce );
	void ReadDemoList( CUtlVector<RD_Demo_Info_t> &demos );

	bool m_bStartedRecording;
	bool m_bJustConnected;
};

extern CRD_Auto_Record_System g_RD_Auto_Record_System;

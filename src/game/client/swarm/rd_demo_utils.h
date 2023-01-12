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

struct RD_Auto_Recording_t
{
	int year, month, day;
	int hour, minute, second;
	char map[MAX_MAP_NAME];

	static int __cdecl Compare( const RD_Auto_Recording_t *a, const RD_Auto_Recording_t *b );
	inline bool operator<( const RD_Auto_Recording_t &other ) const { return Compare( this, &other ) < 0; }
	inline bool operator<=( const RD_Auto_Recording_t &other ) const { return Compare( this, &other ) <= 0; }
	inline bool operator>( const RD_Auto_Recording_t &other ) const { return Compare( this, &other ) > 0; }
	inline bool operator>=( const RD_Auto_Recording_t &other ) const { return Compare( this, &other ) >= 0; }

	bool Parse( const char *szName, bool bCheckDirectory = false );
	void Format( char *buf, int nBufSize ) const;
};

class CRD_Auto_Record_System final : public CAutoGameSystemPerFrame
{
public:
	CRD_Auto_Record_System();

	void PostInit() override;
	void LevelInitPostEntity() override;
	void LevelShutdownPreEntity() override;
	void Update( float frametime ) override;

	void CleanRecordingsFolder( bool bLeaveEmptySlot );
	float RecomputeDemoDuration( const char *szName, bool bForce );
	void ReadDemoList( CUtlVector<RD_Demo_Info_t> &demos );

	bool m_bStartedRecording;
	bool m_bJustConnected;
};

extern CRD_Auto_Record_System g_RD_Auto_Record_System;

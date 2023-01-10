#pragma once

class CRD_Auto_Record_System final : public CAutoGameSystemPerFrame
{
public:
	CRD_Auto_Record_System();

	void LevelInitPostEntity();
	void LevelShutdownPreEntity();
	void Update( float frametime ) override;

	void CleanRecordingsFolder( bool bLeaveEmptySlot );
	void RecomputeDemoDuration( const char *szName, bool bForce );

	bool m_bStartedRecording;
	bool m_bJustConnected;
};

extern CRD_Auto_Record_System g_RD_Auto_Record_System;

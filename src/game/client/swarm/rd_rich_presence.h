#ifndef RD_RICH_PRESENCE_H__
#define RD_RICH_PRESENCE_H__
#pragma once

#include "asw_gamerules.h"

class RD_Rich_Presence : public CAutoGameSystemPerFrame
{
public:
	RD_Rich_Presence() : CAutoGameSystemPerFrame( "RD_Rich_Presence" )
	{
		m_LastState = ASW_GS_NONE;
		m_nLastStateChangeTime = 0;
		m_nLastUpdateTime = 0;
	}
	virtual ~RD_Rich_Presence() {}

	virtual bool Init();
	virtual void Shutdown();
	virtual void Update( float frametime );

	void UpdatePresence();

	virtual void LevelInitPostEntity() { UpdatePresence(); }
	virtual void LevelShutdownPostEntity() { UpdatePresence(); }

private:
	ASW_GameState m_LastState;
	time_t m_nLastStateChangeTime;
	time_t m_nLastUpdateTime;
};

extern RD_Rich_Presence g_RD_Rich_Presence;

#endif // RD_RICH_PRESENCE_H__

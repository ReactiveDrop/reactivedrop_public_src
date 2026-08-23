#pragma once

#include "steam/isteamuserstats.h"

class C_RD_HUD_VScript;

class CRD_Infection_Deathmatch_Stats
{
public:
	CRD_Infection_Deathmatch_Stats() {}

	void OnUpdate( C_RD_HUD_VScript *pHUD );
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	STEAM_CALLBACK( CRD_Infection_Deathmatch_Stats, OnStatsLoaded, UserStatsReceived_t );
#else
	STEAM_CALLBACK( CRD_Infection_Deathmatch_Stats, OnStatsLoaded, UserStatsReceived_t );
#endif

	static bool ShouldInit( C_RD_HUD_VScript *pHUD );

	int m_iLastWinState{};
	int m_iLastNonZeroPlayerType{};
	int m_iLastZombieKills{};
	int m_iLastHumanKills{};
	bool m_bWasDead{};
	bool m_bEnoughPlayersThisRound{};
	bool m_bAtLeastTwoHumansThisRound{};

	bool m_bStatsRequested{};
	bool m_bStatsLoaded{};
	int32 m_KillsSaved[4]{};
	int32 m_DeathsSaved[4]{};
	int32 m_WinsSaved[4]{};
	int32 m_LossesSaved[4]{};
	int32 m_KillsPending[4]{};
	int32 m_DeathsPending[4]{};
	int32 m_WinsPending[4]{};
	int32 m_LossesPending[4]{};
};

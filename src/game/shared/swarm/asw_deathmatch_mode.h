#ifndef asw_deathmatch_mode_h__
#define asw_deathmatch_mode_h__
#ifdef _WIN32
#pragma once
#endif

#include "UtlSortVector.h"
#include <vector>
#include "asw_deathmatch_mode_light.h"

#define TEAM_ALPHA		FIRST_GAME_TEAM
#define TEAM_BETA		(TEAM_ALPHA + 1)

#define RD_FRAGS_LEFT_SIZE 6 // determines the size of array, how many "# frags left" sounds to play 
#define RD_MAX_GUNGAME_WEAPONS 32

#ifdef CLIENT_DLL
#define CASW_Marine_Resource C_ASW_Marine_Resource
#endif

class CASW_Marine;
class CASW_Player;
class CASW_Marine_Resource;

enum RdGameModes
{
	GAMEMODE_DEATHMATCH,
	GAMEMODE_INSTAGIB,
	GAMEMODE_GUNGAME,
	GAMEMODE_TEAMDEATHMATCH
};

class CASW_Deathmatch_Mode : public CBaseEntity
{
public:
	DECLARE_CLASS( CASW_Deathmatch_Mode, CBaseEntity );
	DECLARE_NETWORKCLASS();

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CASW_Deathmatch_Mode();
	virtual ~CASW_Deathmatch_Mode();

	bool IsDeathmatchEnabled() const { return m_iGameMode == GAMEMODE_DEATHMATCH; }
	bool IsInstagibEnabled() const { return m_iGameMode == GAMEMODE_INSTAGIB; }
	bool IsGunGameEnabled()	const { return m_iGameMode == GAMEMODE_GUNGAME; }
	bool IsTeamDeathmatchEnabled() const { return m_iGameMode == GAMEMODE_TEAMDEATHMATCH; }
	RdGameModes GetGameMode() const { return ( RdGameModes )m_iGameMode.Get(); }

#ifdef GAME_DLL
	int UpdateTransmitState() override	// always send to all clients
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	void LevelInitPostEntity();

	void ApplyDeathmatchConVars();
	void OnMissionStart();
	void OnMarineKilled( const CTakeDamageInfo &info, CASW_Marine *marine );
	void OnPlayerFullyJoined( CASW_Player *pPlayer );
	
	void DeathmatchEnable();

	void InstagibEnable();
	void InstagibDisable( bool bEnableWeaponRespawnTimer = true );

	void GunGameEnable();
	void GunGameDisable( bool bEnableWeaponRespawnTimer = true );
	
	void TeamDeathmatchEnable();
	void TeamDeathmatchDisable( bool bEnableWeaponRespawnTimer = true );

	void SpawnMarine( CASW_Player *player );
	void BroadcastLoadoutScreen();
	void PlayRoundEndMusic();

	void KillingSpreeIncrease( CASW_Marine_Resource *pMR );
	void KillingSpreeReset( CASW_Marine_Resource *pMR );
	void IncrementFragCount( CASW_Marine_Resource *pMR, int iAmount );
	void IncrementDeathCount( CASW_Marine_Resource *pMR, int iAmount );

	void DeathmatchThink();

private:
	void DisableWeaponRespawnTimer();
	void EnableWeaponRespawnTimer();

	void RemoveAllWeaponsFromMap();
	// Remove their weapons and give new one, heal, move to spawn point
	// if weapon_id == -1 then this is for GunGame mode, the weapon id will be
	// taken from gun_game_weapons_ array
	void PrepareMarinesForGunGameOrInstagib( int iWeaponID = -1 );

	void ResetScores();

	void ResetFragsLeftSaid()
	{
		for ( int i = 0; i < RD_FRAGS_LEFT_SIZE; ++i )
			s_bPlayedFragLimitSound[i] = false;
	}

	int m_iBotDeaths[ASW_MAX_MARINE_RESOURCES];
	float m_flBotLastFragTime[ASW_MAX_MARINE_RESOURCES];
	int m_iBotKillingSpree[ASW_MAX_MARINE_RESOURCES];
#endif // #ifdef GAME_DLL

	// shared code
public:
	int GetFragCount( CASW_Marine_Resource *pMR );
	int GetSmallestTeamNumber();
	int GetWeaponIndexByFragsCount( int nFrags );

private:
	// these network variables are shared and replicated to clients
	// in client code they are used mostly for UI code
	// they hold the current state of the game
	CNetworkVar( int, m_iGameMode ); // stores current game mode

	CNetworkVar( int, m_iGunGameWeaponCount );
	CNetworkArray( int, m_iGunGameWeapons, RD_MAX_GUNGAME_WEAPONS ); // holds indexes to weapons which will be given in Gun Game mode

#ifdef GAME_DLL
	bool s_bPlayedFragLimitSound[RD_FRAGS_LEFT_SIZE];
#endif // #ifdef GAME_DLL
};

#endif // asw_deathmatch_mode_h__

#ifndef asw_teamversus_mode_h__
#define asw_teamversus_mode_h__

#ifdef _WIN32
#pragma once
#endif

#include "UtlSortVector.h"
#include <vector>

#ifdef CLIENT_DLL
#define CASW_TeamVersus_Mode C_ASW_TeamVersus_Mode
#endif 

class CASW_Marine;

class CASW_TeamVersus_Mode : public CBaseEntity {
public:
	DECLARE_CLASS( CASW_TeamVersus_Mode, CBaseEntity );
	DECLARE_NETWORKCLASS();

#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif

	CASW_TeamVersus_Mode();
	virtual ~CASW_TeamVersus_Mode();

#ifdef GAME_DLL
	int UpdateTransmitState()	// always send to all clients
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

	void OnMissionStart();
	void OnMarineKilled(const CTakeDamageInfo &info, CASW_Marine *marine);

	bool IsInstagibEnabled() { return instagib_enabled; }
	void InstagibEnable();
	void InstagibDisable(bool enable_weapon_respawn_timer = true);

	bool IsGunGameEnabled() { return gungame_enabled_; }
	void GunGameEnable();
	void GunGameDisable(bool enable_weapon_respawn_timer = true); 
	int	 GetWeaponIndexByFragsCount(int frags_count);
	
	bool IsTeamsEnabled() { return teams_enabled; }
	void EnableTeams();
	void DisableTeams();
	
private:

	void DisableWeaponRespawnTimer();
	void EnableWeaponRespawnTimer();

	void RemoveAllWeaponsFromMap();
	/** Remove their weapons and give new one, heal, move to spawn point
		if weapon_id == -1 then this is for GunGame mode, the weapon id will be
		taken from gun_game_weapons_ array
	*/
	void PrepareMarinesForGunGameOrInstagib(int weapon_id = -1);

	

	// it is static to preserve its value between map changes
	static bool instagib_enabled; // = false; by default

	// it is static to preserve its value between map changes
	static bool gungame_enabled_; // = false; by default

	// it is static to preserve its value between map changes
	static bool teams_enabled;	// = false; by default

	std::vector<int> gun_game_weapons_;	// holds indexes to weapons which will be given in Gun Game mode
#endif 
};

// returns an object if asw_TeamVersus_mode entity is present in map
CASW_TeamVersus_Mode* ASWTeamVersusMode();


#endif // asw_teamversus_mode_h__
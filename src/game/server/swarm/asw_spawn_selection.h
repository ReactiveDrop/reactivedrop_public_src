#ifndef ASW_SPAWN_SELECTION_H
#define ASW_SPAWN_SELECTION_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_spawn_manager.h"

class CASW_Spawn_Set;
class CASW_Spawn_Definition;
class CASW_Spawn_NPC;

enum ASW_Spawn_Type
{
	ASW_SPAWN_TYPE_HORDE,
	ASW_SPAWN_TYPE_WANDERER,
	ASW_SPAWN_TYPE_HORDE_WANDERER,
	ASW_SPAWN_TYPE_PRESPAWN,
	ASW_SPAWN_TYPE_PACK,
	NUM_ASW_SPAWN_TYPES // keep this one last
};

class CASW_Spawn_Selection : public CAutoGameSystem
{
public:
	CASW_Spawn_Selection() : CAutoGameSystem( "CASW_Spawn_Selection" ), m_pSelectedSpawnSet( NULL ) {}

	virtual void LevelInitPreEntity();
	virtual void LevelShutdownPostEntity();

	void OnMissionStarted();
	void Dump();

	int RandomHordeSize();
	int RandomWandererCount();
	int RandomHordeWandererCount();
	int RandomPrespawnCount();
	int RandomPackCount();

	CASW_Spawn_Definition *RandomHordeDefinition();
	CASW_Spawn_Definition *RandomWandererDefinition();
	CASW_Spawn_Definition *RandomHordeWandererDefinition();
	CASW_Spawn_Definition *RandomPrespawnDefinition();
	CASW_Spawn_Definition *RandomPackDefinition();

private:
	CASW_Spawn_Definition *RandomDefinition(ASW_Spawn_Type iSpawnType);

	void PopulateSpawnSets( KeyValues *pKV );

	CUtlVectorAutoPurge<CASW_Spawn_Set *> m_SpawnSets;
	CASW_Spawn_Set *m_pSelectedSpawnSet;
};

CASW_Spawn_Selection *ASWSpawnSelection();

class CASW_Spawn_Set
{
public:
	CASW_Spawn_Set( KeyValues *pKV, bool bOverlay );
	bool MatchesLevel( string_t iszLevelName ) const;
	void ApplyOverlay( CASW_Spawn_Set *pTarget );
	void Dump();

	bool m_bOverlay;
	CUtlVector<string_t> m_iszAppliedOverlays;
	string_t m_iszName;
	string_t m_iszMap;
	int m_iMinSkill;
	int m_iMaxSkill;
	int m_iMinHordeSize;
	int m_iMaxHordeSize;
	bool m_bHasHordeSize;
	int m_iMinWanderers;
	int m_iMaxWanderers;
	bool m_bHasWanderers;
	int m_iMinHordeWanderers;
	int m_iMaxHordeWanderers;
	bool m_bHasHordeWanderers;
	int m_iMinPrespawn;
	int m_iMaxPrespawn;
	bool m_bHasPrespawn;
	int m_iMinPacks;
	int m_iMaxPacks;
	bool m_bHasPacks;
	float m_flTotalSelectionWeight[NUM_ASW_SPAWN_TYPES];
	CUtlVectorAutoPurge<CASW_Spawn_Definition *> m_SpawnDefinitions[NUM_ASW_SPAWN_TYPES];
};

class CASW_Spawn_Definition
{
public:
	CASW_Spawn_Definition( KeyValues *pKV );
	CASW_Spawn_Definition( const CASW_Spawn_Definition & def );
	void Dump( float flTotalWeight );

	float m_flSelectionWeight;
	CUtlVectorAutoPurge<CASW_Spawn_NPC *> m_NPCs;
};

class CASW_Spawn_NPC
{
public:
	CASW_Spawn_NPC( KeyValues *pKV );
	CASW_Spawn_NPC( const CASW_Spawn_NPC & npc );
	void Dump();

	ASW_Alien_Class_Entry *m_pAlienClass;
	int m_iHealthBonus;
	float m_flSpeedScale;
	float m_flSizeScale;
	bool m_bFlammable;
	bool m_bFreezable;
	bool m_bTeslable;
	bool m_bFlinches;
	string_t m_iszVScript;
	float m_flSpawnChance;
	CUtlVector<ConVar *> m_pRequireCVar;
};

#endif // ASW_SPAWN_SELECTION_H

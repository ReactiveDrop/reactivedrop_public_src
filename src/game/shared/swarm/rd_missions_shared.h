#pragma once

#include "asw_shareddefs.h"
#include "steam/steam_api.h"
#include "string_t.h"

#define DEFAULT_CREDITS_FILE "scripts/asw_credits"

#define RD_MAX_CAMPAIGNS 1024
#define RD_MAX_MISSIONS 8192

struct RD_Campaign_t;
struct RD_Campaign_Mission_t;
struct RD_Mission_t;
struct RD_Mission_MinimapSlice_t;

namespace ReactiveDropMissions
{
	extern int s_nDataResets;

#ifdef GAME_DLL
	void CreateNetworkStringTables();
	void ClearServerCache();
#else
	void InstallStringTableCallback( const char *tableName );
	void ClearClientCache();
#endif

	// Get the number of known campaigns or missions.
	int CountCampaigns();
	int CountMissions();

	// These functions return filenames. Returns NULL if the index is out of range.
	const char *CampaignName( int index );
	const char *MissionName( int index );

	// Get the decoded data of a campaign or mission. Returns NULL if index is out of range or the file cannot be parsed.
	const RD_Campaign_t *GetCampaign( int index );
	const RD_Mission_t *GetMission( int index );

	PublishedFileId_t CampaignWorkshopID( int index );
	PublishedFileId_t MissionWorkshopID( int index );

	int GetCampaignIndex( const char *name );
	int GetMissionIndex( const char *name );

	static inline const RD_Campaign_t *GetCampaign( const char *name ) { return GetCampaign( GetCampaignIndex( name ) ); }
	static inline const RD_Mission_t *GetMission( const char *name ) { return GetMission( GetMissionIndex( name ) ); }
}

class CampaignHandle
{
public:
	CampaignHandle() = default;
	explicit CampaignHandle( const char *szBaseName ) { SetCampaign( szBaseName ); }

	const RD_Campaign_t *Get();
	void SetCampaign( const char *szBaseName );

	inline operator bool() { return Get() != NULL; }
	inline bool operator!() { return Get() == NULL; }
	inline const RD_Campaign_t *operator->() { return Get(); }
	inline operator const RD_Campaign_t *() { return Get(); }

private:
	char m_szBaseName[64]{};
	const RD_Campaign_t *m_pCampaign{};
	int m_nDataResets{};
};

class CampaignMissionHandle
{
public:
	CampaignMissionHandle() = default;
	explicit CampaignMissionHandle( const char *szBaseName, int iMission ) { SetCampaignMission( szBaseName, iMission ); }

	const RD_Campaign_Mission_t *Get();
	void SetCampaignMission( const char *szBaseName, int iMission );

	inline operator bool() { return Get() != NULL; }
	inline bool operator!() { return Get() == NULL; }
	inline const RD_Campaign_Mission_t *operator->() { return Get(); }
	inline operator const RD_Campaign_Mission_t *( ) { return Get(); }

private:
	CampaignHandle m_Campaign{};
	int m_iMission{};
};

class MissionHandle
{
public:
	MissionHandle() = default;
	explicit MissionHandle( const char *szBaseName ) { SetMission( szBaseName ); }

	const RD_Mission_t *Get();
	void SetMission( const char *szBaseName );

	inline operator bool() { return Get() != NULL; }
	inline bool operator!() { return Get() == NULL; }
	inline const RD_Mission_t *operator->() { return Get(); }
	inline operator const RD_Mission_t *() { return Get(); }

private:
	char m_szBaseName[64]{};
	const RD_Mission_t *m_pMission{};
	int m_nDataResets{};
};

struct RD_Campaign_t
{
	char BaseName[64]{};
	PublishedFileId_t WorkshopID{ k_PublishedFileIdInvalid };
	bool Installed{ false };

	string_t CampaignName{ NULL_STRING }; // could be localized
	string_t CampaignDescription{ NULL_STRING }; // could be localized
	string_t CustomCreditsFile{ MAKE_STRING( DEFAULT_CREDITS_FILE ) };

	string_t ChooseCampaignTexture{ NULL_STRING }; // vgui texture
	string_t CampaignTextureName{ NULL_STRING }; // vgui texture
	string_t CampaignTextureLayer[3]{ NULL_STRING, NULL_STRING, NULL_STRING }; // vgui texture

	uint16_t GalaxyX{}; // 10 bits
	uint16_t GalaxyY{}; // 10 bits

	uint16_t SearchLightX[ASW_NUM_SEARCH_LIGHTS]{}; // 10 bits
	uint16_t SearchLightY[ASW_NUM_SEARCH_LIGHTS]{}; // 10 bits
	uint16_t SearchLightAngle[ASW_NUM_SEARCH_LIGHTS]{}; // degrees ccw from right

	CUtlVector<RD_Campaign_Mission_t> Missions{}; // starting mission is at index 1; index 0 is dummy
	CUtlVector<string_t> Tags{};

	bool HasTag( const char *tag ) const;
	const RD_Campaign_Mission_t *GetMission( int iMissionIndex ) const;
	const RD_Campaign_Mission_t *GetMissionByMapName( const char *szMapName ) const;
	bool AreMissionsLinked( int from, int to ) const;
};

struct RD_Campaign_Mission_t
{
	uint8_t CampaignIndex{}; // index into campaign Missions vector
	char MapName[64]{};
	string_t MissionName{ NULL_STRING }; // could be localized
	string_t LocationDescription{ NULL_STRING }; // could be localized
	string_t ShortBriefing{ NULL_STRING }; // could be localized
	string_t ThreatString{ NULL_STRING }; // could be localized
	uint16_t LocationX{}; // 10 bits
	uint16_t LocationY{}; // 10 bits
	int16_t DifficultyModifier{};
	bool AlwaysVisible{};
	bool NeedsMoreThanOneMarine{};
	CUtlVector<uint8_t> Links{}; // index into campaign Missions vector
};

static_assert( ( uint8_t( -1 ) >= ASW_MAX_MISSIONS_PER_CAMPAIGN ), "make sure CampaignIndex and Links can hold as much as we need them to" );

struct RD_Mission_t
{
	char BaseName[64]{};
	PublishedFileId_t WorkshopID{ k_PublishedFileIdInvalid };
	bool Installed{ false };

	int32_t PosX{};
	int32_t PosY{};
	float Scale{ 1.0f };

	string_t Material{ NULL_STRING }; // texture
	string_t BriefingMaterial{ NULL_STRING }; // texture
	CUtlVector<RD_Mission_MinimapSlice_t> VerticalSections{};

	string_t MissionTitle{ NULL_STRING }; // could be localized
	string_t Description{ NULL_STRING }; // could be localized
	string_t Image{ NULL_STRING }; // vgui texture
	string_t CustomCreditsFile{ MAKE_STRING( DEFAULT_CREDITS_FILE ) };

	string_t Author{ NULL_STRING };
	string_t Website{ NULL_STRING };
	string_t Version{ NULL_STRING };
	bool Builtin{};

	CUtlVector<string_t> Tags{};

	bool HasTag( const char *tag ) const;
};

struct RD_Mission_MinimapSlice_t
{
	string_t Material{ NULL_STRING }; // vgui texture
	float AltitudeMin{ MIN_COORD_FLOAT };
	float AltitudeMax{ MAX_COORD_FLOAT };
};

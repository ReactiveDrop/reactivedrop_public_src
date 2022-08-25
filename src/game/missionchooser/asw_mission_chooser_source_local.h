#ifndef _INCLUDED_ASW_MISSION_CHOOSER_SOURCE_LOCAL_H
#define _INCLUDED_ASW_MISSION_CHOOSER_SOURCE_LOCAL_H
#ifdef _WIN32
#pragma once
#endif

#include "missionchooser/iasw_mission_chooser_source.h"
#include "tier1/UtlSortVector.h"
#include "tier3/tier3dm.h"

class IASW_Random_Missions;

struct MapListName
{
	char szMapName[256];
};

struct MissionDetails_t
{
	char szMissionName[256];
	KeyValues *m_pMissionKeys;
};

struct CampaignDetails_t
{
	char szCampaignName[256];
	KeyValues *m_pCampaignKeys;
};

// provides lists of missions, saves and campaigns from the local disk
class CASW_Mission_Chooser_Source_Local : public IASW_Mission_Chooser_Source
{
	typedef IASW_Mission_Chooser_Source  BaseClass;

public:
	CASW_Mission_Chooser_Source_Local();
	virtual ~CASW_Mission_Chooser_Source_Local();

	virtual void Think();
	virtual void IdleThink();
	
	virtual void FindMissionsInCampaign( int iCampaignIndex, int nMissionOffset, int iNumSlots );
	virtual int GetNumMissionsInCampaign( int iCampaignIndex );

	virtual void FindMissions(int nMissionOffset, int iNumSlots, bool bRequireOverview);
	virtual ASW_Mission_Chooser_Mission* GetMissions();	// pass an array of mission names back
	virtual ASW_Mission_Chooser_Mission* GetMission( int nIndex, bool bRequireOverview );
	virtual int	 GetNumMissions(bool bRequireOverview);
	
	virtual void FindCampaigns(int nCampaignOffset, int iNumSlots);
	virtual ASW_Mission_Chooser_Mission* GetCampaigns();	// Passes an array of campaign names back
	virtual ASW_Mission_Chooser_Mission* GetCampaign( int nIndex );
	virtual PublishedFileId_t GetCampaignWorkshopID( int nIndex );
	virtual int	 GetNumCampaigns();

	virtual void FindSavedCampaigns(int nSaveOffset, int iNumSlots, bool bMultiplayer, const char *szFilterID);
	virtual ASW_Mission_Chooser_Saved_Campaign* GetSavedCampaigns();	// passes an array of summary data for each save
	virtual int	 GetNumSavedCampaigns(bool bMultiplayer, const char *szFilterID);
	virtual ASW_Mission_Chooser_Saved_Campaign* GetSavedCampaign( int nIndex, bool bMultiplayer, const char *szFilterID );
	virtual void RefreshSavedCampaigns();
	virtual void ResetCurrentPage() { }
	virtual void ClearCache();

	virtual void AddRequiredTag( const char *szTag );

	virtual KeyValues *GetMissionDetails( const char *szMissionName );
	virtual KeyValues *GetCampaignDetails( const char *szCampaignName );

	class MapNameLess
	{
	public:
        bool Less( const MapListName& src1, const MapListName& src2, void *pCtx );
	};

	class CampaignNameLess
	{
	public:
        bool Less( const MapListName& src1, const MapListName& src2, void *pCtx );
	};

	ASW_Mission_Chooser_Mission m_missions[ASW_SAVES_PER_PAGE]; // this array needs to be large enough for either ASW_SAVES_PER_SCREEN (for when showing save games), or ASW_MISSIONS_PER_SCREEN (for when showing missions) or ASW_CAMPAIGNS_PER_SCREEN (for when showing campaigns)
	void ClearMapList();
	void BuildMapList();
	void AddToMapList(const char *szMapName);
	CUtlSortVector<MapListName, MapNameLess> m_Items;
	CUtlSortVector<MapListName, MapNameLess> m_OverviewItems;

	ASW_Mission_Chooser_Mission m_campaigns[ASW_CAMPAIGNS_PER_PAGE]; // this array needs to be large enough for either ASW_SAVES_PER_SCREEN (for when showing save games), or ASW_MISSIONS_PER_SCREEN (for when showing missions) or ASW_CAMPAIGNS_PER_SCREEN (for when showing campaigns)
	void ClearCampaignList();
	void BuildCampaignList();
	void AddToCampaignList(const char *szMapName);
	CUtlSortVector<MapListName, CampaignNameLess> m_CampaignList;

	class SavedCampaignLess
	{
	public:
        bool Less( const ASW_Mission_Chooser_Saved_Campaign& src1, const ASW_Mission_Chooser_Saved_Campaign& src2, void *pCtx );
	};

	ASW_Mission_Chooser_Saved_Campaign m_savedcampaigns[ASW_SAVES_PER_PAGE]; // this array needs to be large enough for either ASW_SAVES_PER_SCREEN (for when showing save games), or ASW_MISSIONS_PER_SCREEN (for when showing missions) or ASW_CAMPAIGNS_PER_SCREEN (for when showing campaigns)	
	void ClearSavedCampaignList();
	void BuildSavedCampaignList();
	void AddToSavedCampaignList(const char *szMapName);
	CUtlSortVector<ASW_Mission_Chooser_Saved_Campaign, SavedCampaignLess> m_SavedCampaignList;
	virtual int GetNumMissionsCompleted(const char *szSaveName);

	virtual void OnSaveDeleted(const char *szSaveName);
	virtual void OnSaveUpdated(const char *szSaveName);	// called when a game is saved and its summary info needs to be updated

	// checking for existence
	bool MissionExists(const char *szMapName, bool bRequireOverview);
	bool CampaignExists(const char *szCampaignName);
	bool SavedCampaignExists(const char *szSaveName);

	const char *GetPrettyMissionName(const char *szMapName);
	const char *GetPrettyCampaignName(const char *szCampaignName);
	const char* GetPrettySavedCampaignName(const char *szSaveName);

	// creating a new savegame
	bool ASW_Campaign_CreateNewSaveGame(char *szFileName, int iFileNameMaxLen, const char *szCampaignName, bool bMultiplayerGame, const char *szStartingMission);	// szFileName arg is the desired filename or NULL for an autogenerated one.  Function sets szFileName with the filename used.
	virtual void NotifySaveDeleted(const char *szSaveName);

	void NotifyNewSave(const char *szSaveName);	
	bool SavePassesFilter(ASW_Mission_Chooser_Saved_Campaign* pSaved, const char *szFilterID);

	bool m_bBuiltMapList;
	bool m_bBuiltCampaignList;
	bool m_bBuiltSavedCampaignList;

	bool m_bBuildingMapList;
	bool m_bBuildingCampaignList;
	bool m_bBuildingSavedCampaignList;

	char const * m_pszMapFind;
	char const * m_pszCampaignFind;
	char const * m_pszSavedFind;

	CUtlVector<MissionDetails_t*> m_MissionDetails;
	CUtlVector<CampaignDetails_t*> m_CampaignDetails;
	CUtlStringList m_NextRequiredTags;
};

#endif // _INCLUDED_ASW_MISSION_CHOOSER_SOURCE_LOCAL_H

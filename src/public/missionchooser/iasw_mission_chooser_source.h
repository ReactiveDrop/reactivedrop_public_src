#ifndef _INCLUDED_IASW_MISSION_CHOOSER_SOURCE_H
#define _INCLUDED_IASW_MISSION_CHOOSER_SOURCE_H
#ifdef _WIN32
#pragma once
#endif

#include "tier0/platform.h"
#include "steam/steam_api.h"

class KeyValues;

struct ASW_Mission_Chooser_Mission
{
	char m_szMissionName[64];
};

struct ASW_Mission_Chooser_Saved_Campaign
{
	char m_szSaveName[64];
	char m_szCampaignName[64];
	char m_szDateTime[64];
	int m_iMissionsComplete;
	char m_szPlayerNames[256];
	char m_szPlayerIDs[512];
	bool m_bMultiplayer;
};

#define ASW_MISSIONS_PER_PAGE 8
#define ASW_CAMPAIGNS_PER_PAGE 3
#define ASW_SAVES_PER_PAGE 8

abstract_class IASW_Mission_Chooser_Source
{
public:
	virtual void Think() = 0;
	virtual void IdleThink() = 0;
	__declspec( deprecated( "use ReactiveDropMissions::GetCampaign instead" ) )
	virtual void FindMissionsInCampaign( int iCampaignIndex, int nMissionOffset, int iNumSlots ) = 0;
	__declspec( deprecated( "use ReactiveDropMissions::GetCampaign instead" ) )
	virtual int GetNumMissionsInCampaign( int iCampaignIndex ) = 0;

	__declspec( deprecated( "use ReactiveDropMissions::GetMission instead" ) )
	virtual void FindMissions(int nMissionOffset, int iNumSlots, bool bRequireOverview) = 0;
	__declspec( deprecated( "use ReactiveDropMissions::GetMission instead" ) )
	virtual ASW_Mission_Chooser_Mission* GetMissions() = 0;	// pass an array of mission names back
	__declspec( deprecated( "use ReactiveDropMissions::GetMission instead" ) )
	virtual ASW_Mission_Chooser_Mission* GetMission( int nIndex, bool bRequireOverview ) = 0;	// may return NULL if asking for a mission outside of the found range
	__declspec( deprecated( "use ReactiveDropMissions::GetMission instead" ) )
	virtual int	 GetNumMissions(bool bRequireOverview) = 0;

	__declspec( deprecated( "use ReactiveDropMissions::GetCampaign instead" ) )
	virtual void FindCampaigns(int nCampaignOffset, int iNumSlots) = 0;
	__declspec( deprecated( "use ReactiveDropMissions::GetCampaign instead" ) )
	virtual ASW_Mission_Chooser_Mission* GetCampaigns() = 0;	// Passes an array of campaign names back
	__declspec( deprecated( "use ReactiveDropMissions::GetCampaign instead" ) )
	virtual ASW_Mission_Chooser_Mission* GetCampaign( int nIndex ) = 0;		// may return NULL when requesting a campaign outside the found range
	__declspec( deprecated( "use ReactiveDropMissions::GetCampaign instead" ) )
	virtual int	 GetNumCampaigns() = 0;

	virtual void FindSavedCampaigns(int page, int iNumSlots, bool bMultiplayer, const char *szFilterID) = 0;
	virtual ASW_Mission_Chooser_Saved_Campaign* GetSavedCampaigns() = 0;	// passes an array of summary data for each save
	virtual ASW_Mission_Chooser_Saved_Campaign* GetSavedCampaign( int nIndex, bool bMultiplayer, const char *szFilterID ) = 0;	// may return NULL when requesting a save outside the found range
	virtual int	 GetNumSavedCampaigns(bool bMultiplayer, const char *szFilterID) = 0;
	virtual void RefreshSavedCampaigns() = 0;	// call when the saved campaigns list is dirty and should be refreshed
	virtual int GetNumMissionsCompleted(const char *szSaveName) = 0;
	virtual void OnSaveDeleted(const char *szSaveName) = 0;	// call when a particular save has been deleted
	virtual void OnSaveUpdated(const char *szSaveName) = 0;	// call when a particular save has been updated

	// following only supporter by the local mission source
	__declspec( deprecated( "use ReactiveDropMissions::GetMission instead" ) )
	virtual bool MissionExists(const char *szMapName, bool bRequireOverview) = 0;
	__declspec( deprecated( "use ReactiveDropMissions::GetCampaign instead" ) )
	virtual bool CampaignExists(const char *szCampaignName) = 0;
	virtual bool SavedCampaignExists(const char *szSaveName) = 0;
	virtual bool ASW_Campaign_CreateNewSaveGame(char *szFileName, int iFileNameMaxLen, const char *szCampaignName, bool bMultiplayerGame, const char *szStartingMission) = 0;
	virtual void NotifySaveDeleted(const char *szSaveName) = 0;
	__declspec( deprecated )
	virtual const char *GetCampaignSaveIntroMap( const char *szSaveName ) { return NULL; } // deprecated

	// returns nice version of the filenames (i.e. title from the overview.txt or from the campaign txt)
	__declspec( deprecated( "use ReactiveDropMissions::GetMission instead" ) )
	virtual const char* GetPrettyMissionName(const char *szMapName) = 0;
	__declspec( deprecated( "use ReactiveDropMissions::GetCampaign instead" ) )
	virtual const char* GetPrettyCampaignName(const char *szCampaignName) = 0;
	virtual const char* GetPrettySavedCampaignName(const char *szSaveName) = 0;	

	// needed by network source
	virtual void ResetCurrentPage() = 0;

	__declspec( deprecated( "use ReactiveDropMissions::GetMission instead" ) )
	virtual KeyValues *GetMissionDetails( const char *szMissionName ) = 0;
	__declspec( deprecated( "use ReactiveDropMissions::GetCampaign instead" ) )
	virtual KeyValues *GetCampaignDetails( const char *szCampaignName ) = 0;

	// New virtual functions have to be last or IMatchFramework crashes!
	virtual void ClearCache() = 0;
	virtual PublishedFileId_t GetCampaignWorkshopID( int nIndex ) = 0;

	// adds a required tag for the next call to a Find* method
	virtual void AddRequiredTag( const char *szTag ) = 0;
};

#endif // _INCLUDED_IASW_MISSION_CHOOSER_SOURCE_H

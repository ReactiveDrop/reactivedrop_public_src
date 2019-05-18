#ifndef RD_WORKSHOP_H__
#define RD_WORKSHOP_H__

#ifdef _WIN32
#pragma once
#endif

#include "igamesystem.h"
#include "steam/steam_api.h"
#include <utlvector.h>
#include <utlmap.h>
#include <utlstring.h>
#include <utlstringmap.h>
#ifdef CLIENT_DLL
#include "rd_workshop_preview_image.h"
#else
#include "steam/steam_gameserver.h"
#endif

namespace BaseModUI
{
	class AddonListItem;
	class Addons;
	class ReactiveDropChallengeSelectionListItem;
	class ReactiveDropChallengeSelection;
	class ReactiveDropWorkshop;
	class ReactiveDropWorkshopListItem;
}

class CReactiveDropWorkshop : public CAutoGameSystem
{
public:
	CReactiveDropWorkshop() :
		CAutoGameSystem( "CReactiveDropWorkshop" ),
#ifdef CLIENT_DLL
		m_hUpdate( k_UGCUpdateHandleInvalid ),
		m_hPublishedAddonsQuery( k_UGCQueryHandleInvalid ),
		m_bHaveAllPublishedAddons( false ),
#else
		m_bWorkshopSetupCompleted( false ),
#endif
		m_hEnabledAddonsQuery( k_UGCQueryHandleInvalid ),
		m_hFavoritedAddonsQuery( k_UGCQueryHandleInvalid ) {}

	virtual bool Init();
	void OnMissionStart();
	virtual void LevelInitPostEntity();
	virtual void LevelShutdownPreEntity();
#ifdef GAME_DLL
	void SetupThink();
	bool m_bWorkshopSetupCompleted;
#endif

	PublishedFileId_t FindAddonProvidingFile( const char *pszFileName );

	struct WorkshopItem_t
	{
		WorkshopItem_t()
		{
			memset( &details, 0, sizeof( details ) );
			nSubscriptions = 0;
			nFavorites = 0;
			nFollowers = 0;
			nUniqueSubscriptions = 0;
			nUniqueFavorites = 0;
			nUniqueFollowers = 0;
			nUniqueWebsiteViews = 0;
			nSecondsPlayed = 0;
			nPlaytimeSessions = 0;
			nComments = 0;
		}

		WorkshopItem_t( const WorkshopItem_t & other )
		{
			details = other.details;
			nSubscriptions = other.nSubscriptions;
			nFavorites = other.nFavorites;
			nFollowers = other.nFollowers;
			nUniqueSubscriptions = other.nUniqueSubscriptions;
			nUniqueFavorites = other.nUniqueFavorites;
			nUniqueFollowers = other.nUniqueFollowers;
			nUniqueWebsiteViews = other.nUniqueWebsiteViews;
			nSecondsPlayed = other.nSecondsPlayed;
			nPlaytimeSessions = other.nPlaytimeSessions;
			nComments = other.nComments;
#ifdef CLIENT_DLL
			pPreviewImage = other.pPreviewImage;
#endif
			for ( int i = 0; i < other.kvTags.GetNumStrings(); i++ )
			{
				CUtlStringList & values = kvTags[other.kvTags.String( i )];
				FOR_EACH_VEC( other.kvTags[i], j )
				{
					values.CopyAndAddToTail( other.kvTags[i][j] );
				}
			}
		}

		~WorkshopItem_t()
		{
		}

		operator bool() const
		{
			return details.m_nPublishedFileId != k_PublishedFileIdInvalid;
		}

		SteamUGCDetails_t details;
		uint64 nSubscriptions;
		uint64 nFavorites;
		uint64 nFollowers;
		uint64 nUniqueSubscriptions;
		uint64 nUniqueFavorites;
		uint64 nUniqueFollowers;
		uint64 nUniqueWebsiteViews;
		uint64 nSecondsPlayed;
		uint64 nPlaytimeSessions;
		uint64 nComments;
#ifdef CLIENT_DLL
		CUtlReference<CReactiveDropWorkshopPreviewImage> pPreviewImage;
#endif
		CUtlStringMap<CUtlStringList> kvTags;
	};

	WorkshopItem_t TryQueryAddon( PublishedFileId_t nPublishedFileID );
#ifdef CLIENT_DLL
	bool LoadAddonEarly( PublishedFileId_t nPublishedFileID );
#endif

private:
#ifdef CLIENT_DLL
	struct WorkshopPreviewRequest_t
	{
		WorkshopPreviewRequest_t( const SteamUGCDetails_t & );
		void Callback( RemoteStorageDownloadUGCResult_t *pResult, bool bIOFailure );

		bool m_bCancelled;
		PublishedFileId_t m_nFileID;
		UGCHandle_t m_nPreviewImage;
		CCallResult<CReactiveDropWorkshop::WorkshopPreviewRequest_t, RemoteStorageDownloadUGCResult_t> m_hCall;
	};
	CUtlVectorAutoPurge<WorkshopPreviewRequest_t *> m_PreviewRequests;
	void ClearOldPreviewRequests();
#endif

	void RestartEnabledAddonsQuery();
	CUtlVector<WorkshopItem_t> m_EnabledAddons;
	UGCQueryHandle_t m_hEnabledAddonsQuery;
	CUtlVector<PublishedFileId_t> m_EnabledAddonsForQuery;
	int m_iFavoritedAddonsPage;
	void RequestNextFavoritedAddonsPage();
	UGCQueryHandle_t m_hFavoritedAddonsQuery;
	int m_iPublishedAddonsPage;
	bool m_bHaveAllPublishedAddons;
	void RequestNextPublishedAddonsPage();
	UGCQueryHandle_t m_hPublishedAddonsQuery;
public:
	bool IsSubscribedToFile( PublishedFileId_t nPublishedFileId );
	void SetSubscribedToFile( PublishedFileId_t nPublishedFileId, bool bSubscribe );
	bool IsAddonEnabled( PublishedFileId_t nPublishedFileId );
	void SetAddonEnabled( PublishedFileId_t nPublishedFileId, bool bEnabled );

private:
	friend class BaseModUI::Addons;
	friend class BaseModUI::AddonListItem;
	friend class BaseModUI::ReactiveDropChallengeSelection;
	friend class BaseModUI::ReactiveDropChallengeSelectionListItem;
	friend class BaseModUI::ReactiveDropWorkshop;
	friend class BaseModUI::ReactiveDropWorkshopListItem;
	friend class CRD_VGUI_Workshop_Download_Progress;

#ifdef CLIENT_DLL
	STEAM_CALLBACK( CReactiveDropWorkshop, ScreenshotReadyCallback, ScreenshotReady_t );
#endif
	STEAM_CALLBACK( CReactiveDropWorkshop, DownloadItemResultCallback, DownloadItemResult_t );
#ifdef GAME_DLL
	STEAM_GAMESERVER_CALLBACK( CReactiveDropWorkshop, DownloadItemResultCallback_Server, DownloadItemResult_t );
#endif
	void AddAddonsToCache( SteamUGCQueryCompleted_t *pResult, bool bIOFailure, UGCQueryHandle_t & hQuery );
	CCallResult<CReactiveDropWorkshop, SteamUGCQueryCompleted_t> m_SteamUGCQueryCompleted;
	void SteamUGCQueryCompletedCallback( SteamUGCQueryCompleted_t *pResult, bool bIOFailure );
#ifdef CLIENT_DLL
	CCallResult<CReactiveDropWorkshop, SteamUGCQueryCompleted_t> m_SteamPublishedAddonsRequestCompleted;
	void SteamPublishedAddonsRequestCompleted( SteamUGCQueryCompleted_t *pResult, bool bIOFailure );
#endif
	CCallResult<CReactiveDropWorkshop, SteamUGCQueryCompleted_t> m_SteamFavoritedAddonsRequestCompleted;
	void SteamFavoritedAddonsRequestCompleted( SteamUGCQueryCompleted_t *pResult, bool bIOFailure );

	STEAM_CALLBACK( CReactiveDropWorkshop, OnSubscribed, RemoteStoragePublishedFileSubscribed_t );
	STEAM_CALLBACK( CReactiveDropWorkshop, OnUnsubscribed, RemoteStoragePublishedFileUnsubscribed_t );

#ifdef CLIENT_DLL
	void UploadWorkshopItem( const char *pszContentPath, const char *pszPreviewImagePath, const char *pszTitle, const char *pszDescription, const CUtlVector<const char *> & tags );
	void UpdateWorkshopItem( PublishedFileId_t nFileID, const char *pszContentPath, const char *pszPreviewImagePath, const char *pszChangeDescription );
	void SetWorkshopItemTags( PublishedFileId_t nFileID, const CUtlVector<const char *> & tags );
	bool IsAutoTag( const char *szTag );
public:
	// True if the page was opened in the overlay, false if the page was opened in the browser.
	bool OpenWorkshopPageForFile( PublishedFileId_t nPublishedFileID );
private:
	bool PrepareWorkshopVPK( const char *pszContentPath, CUtlString *pszDisallowedFiles = NULL );
	void RemoveDuplicateTags();
	void SetWorkshopKeyValues( UGCUpdateHandle_t hUpdate );
	CUtlString m_szContentPath;
	CUtlString m_szPreviewImagePath;
	CUtlString m_szTitle;
	CUtlString m_szDescription;
	CUtlString m_szUpdateChangeDescription;
	CUtlStringList m_aszTags;
	CUtlStringList m_aszIncludedCampaigns;
	CUtlStringList m_aszIncludedMissions;
	CUtlStringList m_aszIncludedChallenges;
	CUtlStringList m_aszIncludedDeathmatch;
	CUtlStringMap<CUtlString> m_IncludedCampaignNames;
	CUtlStringMap<CUtlStringList> m_IncludedCampaignMissions;
	CUtlStringMap<CUtlString> m_IncludedMissionNames;
	CUtlStringMap<CUtlString> m_IncludedChallengeNames;
	UGCQueryHandle_t m_hUpdateWorkshopItemQuery;
	UGCUpdateHandle_t m_hUpdate;
	PublishedFileId_t m_nLastPublishedFileID;
	bool m_bWantAutoTags;
	CCallResult<CReactiveDropWorkshop, CreateItemResult_t> m_CreateItemResultCallback;
	void CreateItemResultCallback( CreateItemResult_t *pResult, bool bIOFailure );
	CCallResult<CReactiveDropWorkshop, SubmitItemUpdateResult_t> m_SubmitItemUpdateResultCallback;
	void SubmitItemUpdateResultCallback( SubmitItemUpdateResult_t *pResult, bool bIOFailure );
	CCallResult<CReactiveDropWorkshop, SteamUGCQueryCompleted_t> m_UpdateWorkshopItemQueryResultCallback;
	void UpdateWorkshopItemQueryResultCallback( SteamUGCQueryCompleted_t *pResult, bool bIOFailure );
	friend static void ugc_create(const CCommand & args);
	friend static void ugc_update(const CCommand & args);
	friend static void ugc_updatetags(const CCommand & args);
	friend static void _ugc_update_progress(const CCommand & args);

	void CheckPublishedAddonConsistency();
	friend class CFixWorkshopKeyValueNames;
#endif
};

extern CReactiveDropWorkshop g_ReactiveDropWorkshop;

#endif // RD_WORKSHOP_H__

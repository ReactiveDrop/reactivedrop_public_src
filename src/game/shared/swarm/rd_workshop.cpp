#include "cbase.h"
#include "rd_workshop.h"
#include "filesystem.h"
#include "asw_gamerules.h"
#include "vpklib/packedstore.h"
#include "rd_challenges_shared.h"
#include "rd_missions_shared.h"
#include "ConfigManager.h"
#include "missionchooser/iasw_mission_chooser.h"
#include "missionchooser/iasw_mission_chooser_source.h"
#include "asw_util_shared.h"
#include "fmtstr.h"

#ifdef CLIENT_DLL
#include "c_asw_game_resource.h"
#include "c_asw_marine_resource.h"
#include "c_asw_marine.h"
#include "gameui/swarm/basemodpanel.h"
#include "gameui/swarm/vaddons.h"
#include "gameui/swarm/rd_workshop_frame.h"
#include <vgui/ISystem.h>
#include <vgui/ILocalize.h>

#ifdef IS_WINDOWS_PC
#undef INVALID_HANDLE_VALUE
#include <Windows.h>
#undef ShellExecute
#endif
#else
#include "gameinterface.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// 0 is "download right now". each number downloads at the same time as other downloads with the same number. higher numbers wait for lower numbers.
#define WORKSHOP_PREVIEW_IMAGE_PRIORITY 0

#define WORKSHOP_DISABLED_ADDONS_FILENAME "addonlist_workshop.txt"

#ifdef CLIENT_DLL
ConVar rd_download_workshop_previews( "rd_download_workshop_previews", "1", FCVAR_ARCHIVE, "If 0 game will not download preview images for workshop add-ons, improving performance at startup" );
ConVar cl_workshop_debug( "cl_workshop_debug", "0", FCVAR_NONE, "If 1 workshop debugging messages will be printed in console" );
#else
ConVar rd_workshop_update_every_round( "rd_workshop_update_every_round", "1", FCVAR_HIDDEN, "If 1 dedicated server will check for workshop items during each mission restart(workshop.cfg will be executed). If 0, workshop items will only update once during server startup" );
ConVar rd_workshop_use_reactivedrop_folder( "rd_workshop_use_reactivedrop_folder", "1", FCVAR_NONE, "If 1, use the reactivedrop folder. If 0, use the folder steam assigns by default", true, 0, true, 1 );
ConVar rd_workshop_unconditional_download_item( "rd_workshop_unconditional_download_item", "0", FCVAR_NONE, "Dedicated server only. If nonzero, call ISteamUGC::DownloadItem every [number] map loads, even if the API reports it being up-to-date." );
ConVar sv_workshop_debug( "sv_workshop_debug", "0", FCVAR_NONE, "If 1 workshop debugging messages will be printed in console" );
ConVar rd_workshop_official_addons( "rd_workshop_official_addons", "2", FCVAR_NONE, "0 = load workshop.cfg on official dedicated servers, 1 = load official addon list, 2 = load both" );
#endif

#if RD_NUM_WORKSHOP_CAMPAIGN_TAGS
const char *const g_RDWorkshopCampaignTags[] =
{
};
#endif

const char *const g_RDWorkshopMissionTags[] =
{
	"bonus", // browsable
	"deathmatch", // browsable
	"points", // for stats site to verify
	"upload_on_failure", // for stats site to verify
	"endless", // browsable
};

CReactiveDropWorkshop g_ReactiveDropWorkshop;

static void ClearCaches( const char *szReason );
static void GetActiveAddons( CUtlVector<PublishedFileId_t> & active );
static void UpdateAndLoadAddon( PublishedFileId_t id, bool bHighPriority = false, bool bUnload = false );
static void RealLoadAddon( PublishedFileId_t id );
static void LoadAddon( PublishedFileId_t id, bool bFromDownload );
static void RealUnloadAddon( PublishedFileId_t id );
static void UnloadAddon( PublishedFileId_t id );
static bool s_bStartingUp = false;
static CUtlStringMap<PublishedFileId_t> s_FileNameToAddon;
#ifdef CLIENT_DLL
static CUtlVector<PublishedFileId_t> s_DelayedLoadAddons;
static CUtlVector<PublishedFileId_t> s_DelayedUnloadAddons;
#else
static CUtlVector<PublishedFileId_t> s_ServerWorkshopAddons;
static bool s_bAnyServerUpdates = false;
#endif
static CUtlVector<PublishedFileId_t> s_DisabledAddons;

bool CReactiveDropWorkshop::Init()
{
#ifdef GAME_DLL
	if ( engine->IsDedicatedServer() )
	{
		return true;
	}
#endif

	if ( !SteamUGC() )
	{
		Warning( "No Steam connection. Skipping workshop.\n" );
		return true;
	}

	uint32 nSubscribed = SteamUGC()->GetNumSubscribedItems();
	if ( nSubscribed != 0 && !CommandLine()->FindParm( "-skiploadingworkshopaddons" ) )
	{
		int iStart = m_EnabledAddonsForQuery.AddMultipleToTail( nSubscribed );

		nSubscribed = SteamUGC()->GetSubscribedItems( m_EnabledAddonsForQuery.Base() + iStart, nSubscribed );
		m_EnabledAddonsForQuery.SetCountNonDestructively( iStart + nSubscribed );

		KeyValues::AutoDelete pKV( "WorkshopAddons" );
		ConVarRef cl_cloud_settings( "cl_cloud_settings" );
		bool bLoaded = false;
		if ( cl_cloud_settings.GetInt() != -1 && ( cl_cloud_settings.GetInt() & STEAMREMOTESTORAGE_CLOUD_DISABLED_WORKSHOP_ITEMS ) && SteamRemoteStorage() && SteamRemoteStorage()->FileExists( WORKSHOP_DISABLED_ADDONS_FILENAME ) )
		{
			CUtlBuffer buf;
			int32 iSize = SteamRemoteStorage()->GetFileSize( WORKSHOP_DISABLED_ADDONS_FILENAME );
			SteamRemoteStorage()->FileRead( WORKSHOP_DISABLED_ADDONS_FILENAME, buf.AccessForDirectRead( iSize ), iSize );
			filesystem->WriteFile( WORKSHOP_DISABLED_ADDONS_FILENAME, "MOD", buf );
			buf.SetBufferType( true, true );
			bLoaded = pKV->LoadFromBuffer( WORKSHOP_DISABLED_ADDONS_FILENAME, buf );
		}
		else
		{
			bLoaded = pKV->LoadFromFile( filesystem, WORKSHOP_DISABLED_ADDONS_FILENAME, "MOD" );
		}

		if ( bLoaded )
		{
			FOR_EACH_VALUE( pKV, pValue )
			{
				if ( pValue->GetBool() )
				{
					char* szEnd = NULL;
					PublishedFileId_t id = strtoull( pValue->GetName(), &szEnd, 10 );
					if ( !id || ( szEnd && *szEnd ) )
					{
						continue;
					}
					if ( !s_DisabledAddons.IsValidIndex( s_DisabledAddons.Find( id ) ) && m_EnabledAddonsForQuery.IsValidIndex( m_EnabledAddonsForQuery.Find( id ) ) && m_EnabledAddonsForQuery.Find(id) >= iStart )
					{
						s_DisabledAddons.AddToTail( id );
					}
				}
			}
		}

		s_bStartingUp = true;
		for ( int i = iStart; i < m_EnabledAddonsForQuery.Count(); i++ )
		{
			UpdateAndLoadAddon( m_EnabledAddonsForQuery[i] );
		}
		s_bStartingUp = false;

#ifdef CLIENT_DLL
		RestartEnabledAddonsQuery();
#endif
	}

#ifdef CLIENT_DLL
	ClearCaches( "initializing" );

	m_iPublishedAddonsPage = 0;
	RequestNextPublishedAddonsPage();
#endif

	return true;
}

static void SaveDisabledAddons()
{
	KeyValues::AutoDelete pKV( "WorkshopAddons" );
	FOR_EACH_VEC( s_DisabledAddons, i )
	{
		char szID[21];
		V_snprintf( szID, sizeof( szID ), "%llu", s_DisabledAddons[i] );
		pKV->SetBool( szID, true );
	}
	if ( !pKV->SaveToFile( filesystem, WORKSHOP_DISABLED_ADDONS_FILENAME, "MOD" ) )
	{
		Warning( "Could not save disabled workshop addons list!\n" );
		return;
	}

	ConVarRef cl_cloud_settings( "cl_cloud_settings" );
	if ( ( cl_cloud_settings.GetInt() & STEAMREMOTESTORAGE_CLOUD_DISABLED_WORKSHOP_ITEMS ) && SteamRemoteStorage() )
	{
		CUtlBuffer buf;
		if ( !filesystem->ReadFile( WORKSHOP_DISABLED_ADDONS_FILENAME, "MOD", buf ) )
		{
			Warning( "Could not load disabled workshop addons list!\n" );
			return;
		}
		if ( !SteamRemoteStorage()->FileWrite( WORKSHOP_DISABLED_ADDONS_FILENAME, buf.Base(), buf.TellMaxPut() ) )
		{
			Warning( "Could not write disabled workshop addons list to Steam Cloud!\n" );
			return;
		}
	}
	DevMsg( "Saved disabled workshop addons list.\n" );
}

#ifdef GAME_DLL
static bool DedicatedServerWorkshopSetup()
{
	if ( !SteamGameServer() || !SteamGameServer()->BLoggedOn() )
	{
		return false;
	}

	if ( !SteamGameServerUGC() )
	{
		Warning( "No Steam connection. Skipping workshop.\n" );
		return true;
	}

	if ( rd_workshop_use_reactivedrop_folder.GetBool() )
	{
		char szDir[MAX_PATH];
		UTIL_GetModDir( szDir, sizeof( szDir ) );
		char szWorkshopDir[MAX_PATH];
		V_ComposeFileName( szDir, "workshop", szWorkshopDir, sizeof( szWorkshopDir ) );

		bool bInit = SteamGameServerUGC()->BInitWorkshopForGameServer( 563560, szWorkshopDir );
		if ( !bInit )
		{
			DevWarning( "Workshop init failed! Trying to continue anyway...\n" );
		}
	}

	s_bAnyServerUpdates = false;
	s_bStartingUp = true;
	if ( rd_workshop_official_addons.GetInt() != 1 )
	{
		engine->ServerCommand( "exec workshop.cfg\n" );
		engine->ServerExecute();
	}
	s_bStartingUp = false;
	if ( s_bAnyServerUpdates )
	{
		ClearCaches( "dedicated server workshop setup found update" );
	}

	return true;
}

CON_COMMAND( rd_enable_workshop_item, "(dedicated servers only) enable a workshop addon by ID" )
{
	if ( !engine->IsDedicatedServer() )
	{
		Warning( "rd_enable_workshop_item can only be used on dedicated servers.\n" );
		return;
	}

	if ( !UTIL_IsCommandIssuedByServerAdmin() )
	{
		return;
	}

	for ( int i = 1; i < args.ArgC(); i++ )
	{
		PublishedFileId_t id = strtoull( args.Arg( i ), NULL, 10 );
		if ( !id )
		{
			Warning( "Could not parse argument \"%s\"\n", args.Arg( i ) );
			continue;
		}

		if ( !s_ServerWorkshopAddons.IsValidIndex( s_ServerWorkshopAddons.Find( id ) ) )
		{
			s_ServerWorkshopAddons.AddToTail( id );
		}

		UpdateAndLoadAddon( id, false, true );
	}
}
#endif

const CReactiveDropWorkshop::WorkshopItem_t & CReactiveDropWorkshop::TryQueryAddon( PublishedFileId_t nPublishedFileID )
{
	const static WorkshopItem_t nilWorkshopItem;

	if ( nPublishedFileID == k_PublishedFileIdInvalid )
	{
		return nilWorkshopItem;
	}

	FOR_EACH_VEC( m_EnabledAddons, i )
	{
		if ( m_EnabledAddons[i].details.m_nPublishedFileId == nPublishedFileID )
		{
			return m_EnabledAddons[i];
		}
	}

	FOR_EACH_VEC( m_EnabledAddonsForQuery, i )
	{
		if ( m_EnabledAddonsForQuery[i] == nPublishedFileID )
		{
			return nilWorkshopItem;
		}
	}

	m_EnabledAddonsForQuery.AddToTail( nPublishedFileID );
	RestartEnabledAddonsQuery();

	return nilWorkshopItem;
}

#ifdef CLIENT_DLL
void CReactiveDropWorkshop::ClearOldPreviewRequests()
{
	FOR_EACH_VEC_BACK( m_PreviewRequests, i )
	{
		if ( m_PreviewRequests[i]->m_hCall.IsActive() )
		{
			continue;
		}

		DevMsg( "Clearing completed preview request for workshop file %llu (preview ID %llu)\n", m_PreviewRequests[i]->m_nFileID, m_PreviewRequests[i]->m_nPreviewImage );

		delete m_PreviewRequests[i];
		m_PreviewRequests.Remove( i );
	}
}
#endif

void CReactiveDropWorkshop::RestartEnabledAddonsQuery()
{
#ifdef CLIENT_DLL
	ClearOldPreviewRequests();
#endif

	if ( s_bStartingUp || m_EnabledAddonsForQuery.Count() == 0 )
	{
		return;
	}

	ISteamUGC *pUGC = SteamUGC();
#ifdef GAME_DLL
	if ( !pUGC )
		pUGC = SteamGameServerUGC();
#endif
	if ( !pUGC )
	{
		DevWarning( "cannot query enabled addon metadata: no ISteamUGC!\n" );
		return;
	}

	if ( m_hEnabledAddonsQuery != k_UGCQueryHandleInvalid )
	{
		DevMsg( "Clearing previous Workshop metadata request\n" );
		pUGC->ReleaseQueryUGCRequest( m_hEnabledAddonsQuery );
	}

	DevMsg( "Sending Workshop metadata request\n" );

	UGCQueryHandle_t hQuery = pUGC->CreateQueryUGCDetailsRequest( m_EnabledAddonsForQuery.Base(), m_EnabledAddonsForQuery.Count() );
	m_hEnabledAddonsQuery = hQuery;
	pUGC->SetReturnLongDescription( hQuery, true );
	pUGC->SetReturnKeyValueTags( hQuery, true );
	SteamAPICall_t hAPICall = pUGC->SendQueryUGCRequest( hQuery );
	m_SteamUGCQueryCompleted.Set( hAPICall, this, &CReactiveDropWorkshop::SteamUGCQueryCompletedCallback );
}

#ifdef CLIENT_DLL
void CReactiveDropWorkshop::RequestNextPublishedAddonsPage()
{
	ISteamUGC *pUGC = SteamUGC();
	if ( !pUGC )
	{
		Warning( "cannot query published addon metadata: no ISteamUGC!\n" );
		return;
	}

	if ( m_hPublishedAddonsQuery != k_UGCQueryHandleInvalid )
	{
		pUGC->ReleaseQueryUGCRequest( m_hPublishedAddonsQuery );
	}

	m_iPublishedAddonsPage++;
	AccountID_t iAccount = SteamUser()->GetSteamID().GetAccountID();
	AppId_t iApp = SteamUtils()->GetAppID();
	m_hPublishedAddonsQuery = pUGC->CreateQueryUserUGCRequest( iAccount, k_EUserUGCList_Published, k_EUGCMatchingUGCType_Items_ReadyToUse, k_EUserUGCListSortOrder_CreationOrderAsc, iApp, iApp, m_iPublishedAddonsPage );
	pUGC->SetReturnLongDescription( m_hPublishedAddonsQuery, true );
	pUGC->SetReturnKeyValueTags( m_hPublishedAddonsQuery, true );
	SteamAPICall_t hAPICall = pUGC->SendQueryUGCRequest( m_hPublishedAddonsQuery );
	m_SteamPublishedAddonsRequestCompleted.Set( hAPICall, this, &CReactiveDropWorkshop::SteamPublishedAddonsRequestCompleted );
}
#endif

void CReactiveDropWorkshop::OnSubscribed( RemoteStoragePublishedFileSubscribed_t *pSubscribed )
{
	if ( pSubscribed->m_nAppID != SteamUtils()->GetAppID() )
	{
		return;
	}

	Msg( "Subscribed to workshop item %llu. Downloading with high priority.\n", pSubscribed->m_nPublishedFileId );

	UpdateAndLoadAddon( pSubscribed->m_nPublishedFileId, true );

#ifdef CLIENT_DLL
	m_EnabledAddonsForQuery.AddToTail( pSubscribed->m_nPublishedFileId );
	RestartEnabledAddonsQuery();
#endif
}

void CReactiveDropWorkshop::OnUnsubscribed( RemoteStoragePublishedFileUnsubscribed_t *pUnsubscribed )
{
	if ( pUnsubscribed->m_nAppID != SteamUtils()->GetAppID() )
	{
		return;
	}

	Msg( "Unsubscribed from workshop item %llu. Unloading.\n", pUnsubscribed->m_nPublishedFileId );
	UnloadAddon( pUnsubscribed->m_nPublishedFileId );

#ifdef CLIENT_DLL
	BaseModUI::Addons *pAddons = assert_cast<BaseModUI::Addons *>( BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_ADDONS ) );
	if ( pAddons )
	{
		pAddons->Activate();
	}
#endif
}

void CReactiveDropWorkshop::OnMissionStart()
{
#ifdef CLIENT_DLL
	if ( !SteamUGC() )
	{
		return;
	}

	CUtlVector<PublishedFileId_t> active;
	GetActiveAddons( active );

	if ( active.Count() == 0 )
	{
		return;
	}

	SteamUGC()->StartPlaytimeTracking( active.Base(), active.Count() );
#endif
}

void CReactiveDropWorkshop::LevelInitPostEntity()
{
#ifdef GAME_DLL
	if ( rd_workshop_update_every_round.GetBool() )
	{
		m_bWorkshopSetupCompleted = false;
	}
#endif
}

void CReactiveDropWorkshop::LevelShutdownPreEntity()
{
#ifdef CLIENT_DLL
	if ( !SteamUGC() )
	{
		return;
	}

	SteamUGC()->StopPlaytimeTrackingForAllItems();

	ClearOldPreviewRequests();

	FOR_EACH_VEC( s_DelayedLoadAddons, i )
	{
		RealLoadAddon( s_DelayedLoadAddons[i] );
	}
	s_DelayedLoadAddons.RemoveAll();

	FOR_EACH_VEC( s_DelayedUnloadAddons, i )
	{
		RealUnloadAddon( s_DelayedUnloadAddons[i] );
	}
	s_DelayedUnloadAddons.RemoveAll();
#endif
}

#ifdef GAME_DLL
void CReactiveDropWorkshop::SetupThink()
{
	if ( !engine->IsDedicatedServer() )
	{
		return;
	}

	static float s_flNextDownloadStatusMessage = 0;

	if ( !m_bWorkshopSetupCompleted )
	{
		m_bWorkshopSetupCompleted = DedicatedServerWorkshopSetup();
		s_flNextDownloadStatusMessage = 0;

		g_ReactiveDropWorkshop.RestartEnabledAddonsQuery();
	}

	if ( !m_bWorkshopSetupCompleted || !SteamGameServerUGC() || Plat_FloatTime() < s_flNextDownloadStatusMessage )
	{
		return;
	}

	s_flNextDownloadStatusMessage = Plat_FloatTime() + 1.0f;

	bool bWaitingForAny = false;
	int nPending = 0;
	FOR_EACH_VEC( s_ServerWorkshopAddons, i )
	{
		PublishedFileId_t id = s_ServerWorkshopAddons[i];
		uint32 iItemState = SteamGameServerUGC()->GetItemState( id );
		if ( iItemState & k_EItemStateDownloading )
		{
			uint64 nBytesDownloaded, nBytesTotal;
			if ( SteamGameServerUGC()->GetItemDownloadInfo( id, &nBytesDownloaded, &nBytesTotal ) )
			{
				Msg( "Downloading workshop item %llu: %llu / %llu bytes\n", id, nBytesDownloaded, nBytesTotal );
			}
			else
			{
				Msg( "Downloading workshop item %llu\n", id );
			}
			bWaitingForAny = true;
		}
		else if ( iItemState & k_EItemStateDownloadPending )
		{
			nPending++;
			bWaitingForAny = true;
		}
		else if ( iItemState & k_EItemStateNeedsUpdate )
		{
			Msg( "Workshop item %llu needs update\n", id );
			SteamGameServerUGC()->DownloadItem( id, false );
			bWaitingForAny = true;
		}
	}

	if ( nPending )
	{
		Msg( "Download pending for %d workshop item(s)\n", nPending );
	}

	if ( !bWaitingForAny )
	{
		s_flNextDownloadStatusMessage = FLT_MAX;
	}
}

// called by gameinterface
void WorkshopSetupThink()
{
	g_ReactiveDropWorkshop.SetupThink();
}
#endif

#ifdef CLIENT_DLL
void CReactiveDropWorkshop::ScreenshotReadyCallback( ScreenshotReady_t *pReady )
{
	ISteamScreenshots *pScreenshots = SteamScreenshots();
	Assert( pScreenshots );
	if ( !engine->IsInGame() || !ASWGameRules() || !pScreenshots )
	{
		return;
	}

	char szMission[256]{};
	const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( MapName() );
	if ( !pMission )
	{
		return;
	}

	if ( const wchar_t *pwszMission = g_pVGuiLocalize->Find( STRING( pMission->MissionTitle ) ) )
	{
		V_UnicodeToUTF8( pwszMission, szMission, sizeof( szMission ) );
	}
	else
	{
		V_strncpy( szMission, STRING( pMission->MissionTitle ), sizeof( szMission ) );
	}

	char szCampaign[256]{};
	if ( const RD_Campaign_t *pCampaign = ASWGameRules()->GetCampaignInfo() )
	{
		if ( const wchar_t *pwszCampaign = g_pVGuiLocalize->Find( STRING( pCampaign->CampaignName ) ) )
		{
			V_UnicodeToUTF8( pwszCampaign, szCampaign, sizeof( szCampaign ) );
		}
		else
		{
			V_strncpy( szCampaign, STRING( pCampaign->CampaignName ), sizeof( szCampaign ) );
		}

		V_strcat( szCampaign, ": ", sizeof( szCampaign ) );
	}

	char szName[512]{};
	extern ConVar rd_challenge;
	if ( !V_strcmp( rd_challenge.GetString(), "0" ) )
	{
		V_snprintf( szName, sizeof( szName ), "%s%s", szCampaign, szMission );
	}
	else
	{
		const char *pszChallenge = ReactiveDropChallenges::DisplayName( rd_challenge.GetString() );
		char szChallenge[256]{};
		if ( const wchar_t *pwszChallenge = g_pVGuiLocalize->Find( pszChallenge ) )
		{
			V_UnicodeToUTF8( pwszChallenge, szChallenge, sizeof( szChallenge ) );
		}
		else
		{
			V_strncpy( szChallenge, pszChallenge, sizeof( szChallenge ) );
		}

		V_snprintf( szName, sizeof( szName ), "%s%s (%s)", szCampaign, szMission, szChallenge );
	}

	pScreenshots->SetLocation( pReady->m_hLocal, szName );

	if ( ASWGameRules() && ASWGameRules()->GetGameState() == ASW_GS_INGAME && ASWGameResource() )
	{
		if ( ASWGameRules()->GetMarineDeathCamInterp( true ) != 0 )
		{
			// deathcam marine is always considered visible
			C_ASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( ASWGameRules()->m_nMarineForDeathCam );
			if ( pMR && pMR->IsInhabited() && pMR->GetCommander() )
			{
				pScreenshots->TagUser( pReady->m_hLocal, pMR->GetCommander()->GetSteamID() );
			}
		}

		for ( int i = 0; i < ASWGameResource()->GetMaxMarineResources(); i++ )
		{
			// tag on-screen marines
			C_ASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
			if ( pMR && pMR->IsInhabited() && pMR->GetCommander() && pMR->GetMarineEntity() )
			{
				Vector screenPos;
				if ( !debugoverlay->ScreenPosition( pMR->GetMarineEntity()->WorldSpaceCenter(), screenPos ) )
				{
					pScreenshots->TagUser( pReady->m_hLocal, pMR->GetCommander()->GetSteamID() );
				}
			}
		}
	}

	CUtlVector<PublishedFileId_t> active;
	GetActiveAddons( active );
	FOR_EACH_VEC( active, i )
	{
		pScreenshots->TagPublishedFile( pReady->m_hLocal, active[i] );
	}
}
#endif

bool CReactiveDropWorkshop::IsSubscribedToFile( PublishedFileId_t nPublishedFileId )
{
#ifdef GAME_DLL
	if ( engine->IsDedicatedServer() )
	{
		return true;
	}
#endif

	return ( SteamUGC()->GetItemState( nPublishedFileId ) & k_EItemStateSubscribed ) != 0;
}

void CReactiveDropWorkshop::SetSubscribedToFile( PublishedFileId_t nPublishedFileId, bool bSubscribe )
{
	if ( bSubscribe )
	{
		SteamUGC()->SubscribeItem( nPublishedFileId );
	}
	else
	{
		SteamUGC()->UnsubscribeItem( nPublishedFileId );
	}
}

bool CReactiveDropWorkshop::IsAddonEnabled( PublishedFileId_t nPublishedFileId )
{
	return IsSubscribedToFile( nPublishedFileId ) && !s_DisabledAddons.IsValidIndex( s_DisabledAddons.Find( nPublishedFileId ) );
}

void CReactiveDropWorkshop::SetAddonEnabled( PublishedFileId_t nPublishedFileId, bool bEnabled )
{
	if ( bEnabled )
	{
		if ( s_DisabledAddons.FindAndRemove( nPublishedFileId ) )
		{
			SaveDisabledAddons();
		}
		SetSubscribedToFile( nPublishedFileId, true );
		UpdateAndLoadAddon( nPublishedFileId );
		return;
	}

	if ( !IsSubscribedToFile( nPublishedFileId ) )
	{
		return;
	}

	if ( s_DisabledAddons.IsValidIndex( s_DisabledAddons.Find( nPublishedFileId ) ) )
	{
		return;
	}

	s_DisabledAddons.AddToTail( nPublishedFileId );
	SaveDisabledAddons();
	UnloadAddon( nPublishedFileId );
}

void CReactiveDropWorkshop::DownloadItemResultCallback( DownloadItemResult_t *pResult )
{
	if ( pResult->m_unAppID != SteamUtils()->GetAppID() )
	{
		DevMsg( "DownloadItemResult_t for %llu has AppID %u, but we are %u!\n", pResult->m_nPublishedFileId, pResult->m_unAppID, SteamUtils()->GetAppID() );
		return;
	}

	if ( pResult->m_eResult != k_EResultOK )
	{
		Warning( "Error downloading UGC item %llu: EResult %d (%s)\n", pResult->m_nPublishedFileId, pResult->m_eResult, UTIL_RD_EResultToString( pResult->m_eResult ) );
		return;
	}

	if ( IsSubscribedToFile( pResult->m_nPublishedFileId ) )
	{
		LoadAddon( pResult->m_nPublishedFileId, true );
	}
	else
	{
		Warning( "Download finished for workshop item %llu, but we aren't subscribed!\n", pResult->m_nPublishedFileId );
	}
}

#ifdef GAME_DLL
void CReactiveDropWorkshop::DownloadItemResultCallback_Server( DownloadItemResult_t *pResult )
{
	if ( pResult->m_eResult == k_EResultFileNotFound )
	{
		Warning( "Error downloading addon %llu: file not found\n", pResult->m_nPublishedFileId );
		return;
	}

	if ( pResult->m_eResult != k_EResultOK )
	{
		Warning( "Error downloading UGC item %llu: EResult %d (%s)\n", pResult->m_nPublishedFileId, pResult->m_eResult, UTIL_RD_EResultToString( pResult->m_eResult ) );
		return;
	}

	RealLoadAddon( pResult->m_nPublishedFileId );
}
#endif

void CReactiveDropWorkshop::AddAddonsToCache( SteamUGCQueryCompleted_t *pResult, bool bIOFailure, UGCQueryHandle_t & hQuery )
{
	if ( bIOFailure )
	{
		DevWarning( "Workshop metadata query failed: IO Failure\n" );
		return;
	}
	if ( pResult->m_eResult != k_EResultOK )
	{
		DevWarning( "Workshop metadata query failed: EResult %d (%s)\n", pResult->m_eResult, UTIL_RD_EResultToString( pResult->m_eResult ) );
		return;
	}

	ISteamUGC *pUGC = SteamUGC();
#ifdef GAME_DLL
	if ( !pUGC )
		pUGC = SteamGameServerUGC();
#endif
	if ( !pUGC )
	{
		Warning( "Workshop metadata query finished, but no ISteamUGC!\n" );
		return;
	}

	DevMsg( "Got Workshop metadata response\n" );

	int nextIndex = m_EnabledAddons.AddMultipleToTail( pResult->m_unNumResultsReturned );

	for ( uint32 i = 0; i < pResult->m_unNumResultsReturned; i++ )
	{
		bool bOK = pUGC->GetQueryUGCResult( hQuery, i, &m_EnabledAddons[nextIndex].details );
		if ( !bOK )
		{
			m_EnabledAddons.Remove( m_EnabledAddons.Count() - 1 );
			continue;
		}
		int index = nextIndex;
		bool bExists = false;
		for ( int j = 0; j < nextIndex; j++ )
		{
			// don't add the same addon to the list multiple times if it's published AND subscribed.
			if ( m_EnabledAddons[j].details.m_nPublishedFileId == m_EnabledAddons[index].details.m_nPublishedFileId )
			{
				m_EnabledAddons[j].details = m_EnabledAddons[nextIndex].details;
				index = j;
				bExists = true;
				m_EnabledAddons.Remove( m_EnabledAddons.Count() - 1 );
				break;
			}
		}
		if ( !bExists )
		{
			nextIndex++;
		}
		uint32 nTags = pUGC->GetQueryUGCNumTags( hQuery, i );
		for ( uint32 j = 0; j < nTags; j++ )
		{
			char szTag[1024];
			if ( pUGC->GetQueryUGCTag( hQuery, i, j, szTag, sizeof( szTag ) ) )
			{
				if ( FStrEq( szTag, "adminoverride_Bonus" ) )
				{
					m_EnabledAddons[index].bAdminOverrideBonus = true;
				}
				else if ( FStrEq( szTag, "adminoverride_Deathmatch" ) )
				{
					m_EnabledAddons[index].bAdminOverrideDeathmatch = true;
				}
			}
		}
		pUGC->GetQueryUGCStatistic( hQuery, i, k_EItemStatistic_NumSubscriptions, &m_EnabledAddons[index].nSubscriptions );
		pUGC->GetQueryUGCStatistic( hQuery, i, k_EItemStatistic_NumFavorites, &m_EnabledAddons[index].nFavorites );
		pUGC->GetQueryUGCStatistic( hQuery, i, k_EItemStatistic_NumFollowers, &m_EnabledAddons[index].nFollowers );
		pUGC->GetQueryUGCStatistic( hQuery, i, k_EItemStatistic_NumUniqueSubscriptions, &m_EnabledAddons[index].nUniqueSubscriptions );
		pUGC->GetQueryUGCStatistic( hQuery, i, k_EItemStatistic_NumUniqueFavorites, &m_EnabledAddons[index].nUniqueFavorites );
		pUGC->GetQueryUGCStatistic( hQuery, i, k_EItemStatistic_NumUniqueFollowers, &m_EnabledAddons[index].nUniqueFollowers );
		pUGC->GetQueryUGCStatistic( hQuery, i, k_EItemStatistic_NumUniqueWebsiteViews, &m_EnabledAddons[index].nUniqueWebsiteViews );
		pUGC->GetQueryUGCStatistic( hQuery, i, k_EItemStatistic_NumSecondsPlayed, &m_EnabledAddons[index].nSecondsPlayed );
		pUGC->GetQueryUGCStatistic( hQuery, i, k_EItemStatistic_NumPlaytimeSessions, &m_EnabledAddons[index].nPlaytimeSessions );
		pUGC->GetQueryUGCStatistic( hQuery, i, k_EItemStatistic_NumComments, &m_EnabledAddons[index].nComments );
		m_EnabledAddons[index].kvTags.Purge();
		uint32 nKeyValueTags = pUGC->GetQueryUGCNumKeyValueTags( hQuery, i );
		for ( uint32 j = 0; j < nKeyValueTags; j++ )
		{
			char szKey[1024];
			char szValue[1024];
			if ( pUGC->GetQueryUGCKeyValueTag( hQuery, i, j, szKey, sizeof( szKey ), szValue, sizeof( szValue ) ) )
			{
				m_EnabledAddons[index].kvTags[szKey].CopyAndAddToTail( szValue );
			}
		}
#ifdef CLIENT_DLL
		SteamFriends()->RequestUserInformation( CSteamID( m_EnabledAddons[index].details.m_ulSteamIDOwner ), true );
		if ( bExists )
		{
			FOR_EACH_VEC( m_PreviewRequests, j )
			{
				if ( m_PreviewRequests[j]->m_nFileID == m_EnabledAddons[index].details.m_nPublishedFileId )
				{
					DevMsg( "Cancelling %s preview request for workshop file %llu (preview ID %llu) as we have a new request\n", m_PreviewRequests[j]->m_hCall.IsActive() ? "pending" : "completed", m_PreviewRequests[j]->m_nFileID, m_PreviewRequests[j]->m_nPreviewImage );
					m_PreviewRequests[j]->m_bCancelled = true;
				}
			}
		}
		m_EnabledAddons[index].pPreviewImage = NULL;
		if ( rd_download_workshop_previews.GetBool() )
		{
			m_PreviewRequests.AddToTail( new WorkshopPreviewRequest_t( m_EnabledAddons[index].details ) );
		}
#endif
	}

	pUGC->ReleaseQueryUGCRequest( hQuery );
	hQuery = k_UGCQueryHandleInvalid;

#ifdef CLIENT_DLL
	BaseModUI::Addons *pAddons = assert_cast<BaseModUI::Addons *>( BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_ADDONS ) );
	if ( pAddons )
	{
		pAddons->Activate();
	}

	ReactiveDropMissions::ClearClientCache();
#else
	if ( engine->IsDedicatedServer() )
	{
		ClearCaches( "successfully retrieved workshop metadata" );
	}
#endif
}

void CReactiveDropWorkshop::SteamUGCQueryCompletedCallback( SteamUGCQueryCompleted_t *pResult, bool bIOFailure )
{
	AddAddonsToCache( pResult, bIOFailure, m_hEnabledAddonsQuery );
	m_EnabledAddonsForQuery.Purge();
}

#ifdef CLIENT_DLL
void CReactiveDropWorkshop::SteamPublishedAddonsRequestCompleted( SteamUGCQueryCompleted_t *pResult, bool bIOFailure )
{
	AddAddonsToCache( pResult, bIOFailure, m_hPublishedAddonsQuery );

	if ( pResult->m_unNumResultsReturned )
	{
		RequestNextPublishedAddonsPage();
	}
	else
	{
		m_bHaveAllPublishedAddons = true;
		if ( BaseModUI::CBaseModFrame *pFrame = BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_WORKSHOP ) )
		{
			pFrame->Activate();
		}
		CheckPublishedAddonConsistency();
	}
}

CReactiveDropWorkshop::WorkshopPreviewRequest_t::WorkshopPreviewRequest_t( const SteamUGCDetails_t & details )
{
	DevMsg( "Starting preview request for addon %llu \"%s\" (preview file %llu)\n", details.m_nPublishedFileId, details.m_rgchTitle, details.m_hPreviewFile );

	m_bCancelled = false;
	m_nFileID = details.m_nPublishedFileId;
	m_nPreviewImage = details.m_hPreviewFile;
	SteamAPICall_t hAPICall = SteamRemoteStorage()->UGCDownload( details.m_hPreviewFile, WORKSHOP_PREVIEW_IMAGE_PRIORITY );
	m_hCall.Set( hAPICall, this, &CReactiveDropWorkshop::WorkshopPreviewRequest_t::Callback );
}

void CReactiveDropWorkshop::WorkshopPreviewRequest_t::Callback( RemoteStorageDownloadUGCResult_t *pResult, bool bIOFailure )
{
	if ( bIOFailure )
	{
		Warning( "Could not download preview image %llu for workshop item %llu: IO Failure\n", m_nPreviewImage, m_nFileID );
		return;
	}
	if ( pResult->m_eResult != k_EResultOK )
	{
		Warning( "Could not download preview image %llu for workshop item %llu: EResult %d (%s)\n", m_nPreviewImage, m_nFileID, pResult->m_eResult, UTIL_RD_EResultToString( pResult->m_eResult ) );
		return;
	}

	if ( m_bCancelled )
	{
		DevMsg( "Completed preview request for addon %llu (preview file %llu), but a later request canceled this one. Leaving file open.\n", m_nFileID, m_nPreviewImage );
		return;
	}

	FOR_EACH_VEC( g_ReactiveDropWorkshop.m_EnabledAddons, i )
	{
		if ( g_ReactiveDropWorkshop.m_EnabledAddons[i].details.m_nPublishedFileId == m_nFileID )
		{
			DevMsg( "Completed preview request for addon %llu (preview file %llu, size %d bytes)\n", m_nFileID, m_nPreviewImage, pResult->m_nSizeInBytes );
			CUtlBuffer buf;
			SteamRemoteStorage()->UGCRead( m_nPreviewImage, buf.AccessForDirectRead( pResult->m_nSizeInBytes ), pResult->m_nSizeInBytes, 0, k_EUGCRead_Close );
			g_ReactiveDropWorkshop.m_EnabledAddons[i].pPreviewImage = new CReactiveDropWorkshopPreviewImage( buf );
			if ( g_ReactiveDropWorkshop.m_EnabledAddons[i].pPreviewImage->GetID() == -1 )
			{
				Warning( "Decoding preview image for addon %llu failed!\n", m_nFileID );
			}
			else
			{
				int wide, tall;
				g_ReactiveDropWorkshop.m_EnabledAddons[i].pPreviewImage->GetSize( wide, tall );
				DevMsg( "Decoding preview image for addon %llu succeeded. Size: %d by %d\n", m_nFileID, wide, tall );
			}
			if ( BaseModUI::Addons *pFrame = dynamic_cast<BaseModUI::Addons *>( BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_ADDONS ) ) )
			{
				DevMsg( "Preview image updated for addon %llu. Sending update to Addons window.\n", m_nFileID );
				pFrame->OnWorkshopPreviewReady( m_nFileID, g_ReactiveDropWorkshop.m_EnabledAddons[i].pPreviewImage );
			}
			if ( BaseModUI::ReactiveDropWorkshop *pFrame = dynamic_cast<BaseModUI::ReactiveDropWorkshop *>( BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_WORKSHOP ) ) )
			{
				DevMsg( "Preview image updated for addon %llu. Sending update to Manage window.\n", m_nFileID );
				pFrame->OnWorkshopPreviewReady( m_nFileID, g_ReactiveDropWorkshop.m_EnabledAddons[i].pPreviewImage );
			}
			return;
		}
	}

	Warning( "Completed preview request for addon %llu (preview file %llu, size %d bytes), but there is no addon metadata that matches! Discarding.\n", m_nFileID, m_nPreviewImage, pResult->m_nSizeInBytes );
	SteamRemoteStorage()->UGCRead( pResult->m_hFile, NULL, 0, 0, k_EUGCRead_Close );
}
#endif

static void MaybeAddAddon( CUtlVector<PublishedFileId_t> & addons, PublishedFileId_t id )
{
	if ( id != k_PublishedFileIdInvalid && addons.Find( id ) == -1 )
	{
		addons.AddToTail( id );
	}
}

static void MaybeAddAddonByFile( CUtlVector<PublishedFileId_t> & addons, const char *pszFileName )
{
	PublishedFileId_t id = g_ReactiveDropWorkshop.FindAddonProvidingFile( pszFileName );
	if ( id != k_PublishedFileIdInvalid )
	{
		MaybeAddAddon( addons, id );
		DevMsg( 2, "Workshop addon %llu is active because it contains %s\n", id, pszFileName );
	}
	else
	{
		DevMsg( 2, "No active addon contains file %s\n", pszFileName );
	}
}

static void GetActiveAddons( CUtlVector<PublishedFileId_t> & active )
{
	if ( !ASWGameRules() )
	{
		return;
	}

	// addon that includes the campaign file
	const RD_Campaign_t *pCampaign = ASWGameRules()->GetCampaignInfo();
	if ( pCampaign )
	{
		MaybeAddAddon( active, pCampaign->WorkshopID );
	}

	// addon that includes the overview file
	MaybeAddAddon( active, ASWGameRules()->m_iMissionWorkshopID.Get() );

	// addon that includes the challenge file
	extern ConVar rd_challenge;
	if ( V_strcmp( rd_challenge.GetString(), "0" ) )
	{
		MaybeAddAddon( active, ReactiveDropChallenges::WorkshopID( rd_challenge.GetString() ) );
	}
}

PublishedFileId_t CReactiveDropWorkshop::FindAddonProvidingFile( const char *pszFileName )
{
	UtlSymId_t sym = s_FileNameToAddon.Find( pszFileName );
	if ( sym == UTL_INVAL_SYMBOL )
	{
		return k_PublishedFileIdInvalid;
	}

	return s_FileNameToAddon[sym];
}

#ifdef CLIENT_DLL
CON_COMMAND( rd_dump_workshop_mapping_client, "" )
#else
CON_COMMAND( rd_dump_workshop_mapping_server, "" )
#endif
{
	const char *szPrefix = args.Arg( 1 );

	for ( int i = 0; i < s_FileNameToAddon.GetNumStrings(); i++ )
	{
		const char *szName = s_FileNameToAddon.String( i );
		if ( StringHasPrefix( szName, szPrefix ) )
		{
			ConMsg( "  %s -> %llu\n", szName, s_FileNameToAddon[i] );
		}
	}
}

static void ClearCaches( const char *szReason )
{
	if ( s_bStartingUp )
	{
		return;
	}

	DevMsg( "Workshop: clearing cache: %s\n", szReason );

#ifdef CLIENT_DLL
	ReactiveDropChallenges::ClearClientCache();
	ReactiveDropMissions::ClearClientCache();

	// #iss-speaker-reset 
	// calling snd_restart during game launch causes snd_surround_speakers to reset to default
	// prevent snd_restart to be called during game launch be call it when loading\unloading add-ons
	static bool bRestartSoundEngine = false;
	if ( bRestartSoundEngine )
		engine->ClientCmd_Unrestricted( "snd_restart; update_addon_paths; mission_reload; rd_loc_reload; snd_updateaudiocache; snd_restart" );
	bRestartSoundEngine = true;
	//
#endif
	missionchooser->LocalMissionSource()->ClearCache();

#ifdef GAME_DLL
	extern CServerGameDLL g_ServerGameDLL;
	if ( engine->IsDedicatedServer() && g_ServerGameDLL.m_bIsHibernating )
	{
		bool bCanReload = true;
		FOR_EACH_VEC( s_ServerWorkshopAddons, i )
		{
			uint32 iItemState = SteamGameServerUGC()->GetItemState( s_ServerWorkshopAddons[i] );
			if ( iItemState & k_EItemStateDownloadPending )
			{
				Msg( "Not restarting server: waiting for download of addon %llu\n", s_ServerWorkshopAddons[i] );
				bCanReload = false;
				break;
			}
		}

		if ( bCanReload )
		{
			engine->ServerCommand( "map lobby\n" );
		}
	}
#endif
}

struct LoadedAddonPath_t
{
	PublishedFileId_t m_ID;
	CUtlString m_Path;

	bool operator==( const LoadedAddonPath_t & other ) const
	{
		return m_ID == other.m_ID && m_Path == other.m_Path;
	}
	bool operator!=( const LoadedAddonPath_t & other ) const
	{
		return m_ID != other.m_ID || m_Path != other.m_Path;
	}
};

static bool ShouldUnconditionalDownload( PublishedFileId_t id )
{
#ifdef GAME_DLL
	if ( !engine->IsDedicatedServer() )
	{
		return false;
	}

	if ( rd_workshop_unconditional_download_item.GetBool() )
	{
		static CUtlMap<PublishedFileId_t, int> s_UnconditionalDownloadCooldown{ DefLessFunc( PublishedFileId_t ) };

		unsigned short index = s_UnconditionalDownloadCooldown.Find( id );
		if ( !s_UnconditionalDownloadCooldown.IsValidIndex( index ) )
		{
			index = s_UnconditionalDownloadCooldown.Insert( id, 0 );
		}

		int & cooldown = s_UnconditionalDownloadCooldown.Element( index );
		if ( cooldown == 0 )
		{
			cooldown = rd_workshop_unconditional_download_item.GetInt();
			return true;
		}

		cooldown--;
	}
#endif

	return false;
}

static CUtlVector<LoadedAddonPath_t> s_LoadedAddonPaths;

static void UpdateAndLoadAddon( PublishedFileId_t id, bool bHighPriority, bool bUnload )
{
	ISteamUGC *pWorkshop = SteamUGC();
#ifdef GAME_DLL
	if ( engine->IsDedicatedServer() )
	{
		pWorkshop = SteamGameServerUGC();
	}
#endif
	if ( !pWorkshop )
	{
		Warning( "Cannot install addon %llu: no access to the Steam Workshop API!\n", id );
		return;
	}

	// make sure we know the metadata (for admin override tags, mostly)
	g_ReactiveDropWorkshop.TryQueryAddon( id );

	uint32 iState = pWorkshop->GetItemState( id );
	if ( !ShouldUnconditionalDownload( id ) && ( iState & k_EItemStateInstalled ) && !( iState & k_EItemStateNeedsUpdate ) )
	{
#ifdef CLIENT_DLL
		if ( cl_workshop_debug.GetBool() )
#else 
		if ( sv_workshop_debug.GetBool() )
#endif
		{
			Msg( "Addon %llu is installed and does not need an update.\n", id );

			uint64 sizeOnDisk;
			char szFolder[MAX_PATH];
			uint32 timeStamp;
			if ( pWorkshop->GetItemInstallInfo( id, &sizeOnDisk, szFolder, sizeof( szFolder ), &timeStamp ) )
			{
				Msg( "  size: %llu bytes; timestamp: %u; folder: %s\n", sizeOnDisk, timeStamp, szFolder );
			}
		}
		LoadAddon( id, false );
		return;
	}
	if ( bUnload )
	{
		UnloadAddon( id );
	}
	if ( pWorkshop->DownloadItem( id, bHighPriority ) )
	{
		Msg( "Downloading addon %llu as it is %s.\n", id, ( iState & k_EItemStateInstalled ) ? ( iState & k_EItemStateNeedsUpdate ) ? "out of date" : "(reportedly) up-to-date" : "not installed" );
	}
	else
	{
		Warning( "Download request for addon %llu failed!\n", id );
	}
}

#ifdef GAME_DLL
class LoadWorkshopCollection_t
{
public:
	LoadWorkshopCollection_t( SteamAPICall_t hCall )
	{
		m_QueryCompletedCall.SetGameserverFlag();
		m_QueryCompletedCall.Set( hCall, this, &LoadWorkshopCollection_t::QueryCompletedCall );
	}

	CCallResult<LoadWorkshopCollection_t, SteamUGCQueryCompleted_t> m_QueryCompletedCall;
	void QueryCompletedCall( SteamUGCQueryCompleted_t *pResult, bool bIOFailure )
	{
		delete this;

		if ( bIOFailure )
		{
			Warning( "Workshop collection query failed: IO Failure\n" );
			return;
		}

		if ( pResult->m_eResult != k_EResultOK )
		{
			Warning( "Workshop collection query failed: EResult %d (%s)\n", pResult->m_eResult, UTIL_RD_EResultToString( pResult->m_eResult ) );
			SteamGameServerUGC()->ReleaseQueryUGCRequest( pResult->m_handle );
			return;
		}

		if ( !pResult->m_unNumResultsReturned )
		{
			Warning( "Workshop collection query failed. (no results)\n" );
			SteamGameServerUGC()->ReleaseQueryUGCRequest( pResult->m_handle );
			return;
		}

		SteamUGCDetails_t details;
		if ( !SteamGameServerUGC()->GetQueryUGCResult( pResult->m_handle, 0, &details ) )
		{
			Warning( "Workshop collection query failed. (failed to get result)\n" );
			SteamGameServerUGC()->ReleaseQueryUGCRequest( pResult->m_handle );
			return;
		}

		PublishedFileId_t *children = (PublishedFileId_t *)stackalloc( sizeof( PublishedFileId_t ) * details.m_unNumChildren );
		if ( !SteamGameServerUGC()->GetQueryUGCChildren( pResult->m_handle, 0, children, details.m_unNumChildren ) )
		{
			Warning( "Workshop collection query failed. (failed to get children)\n" );
			SteamGameServerUGC()->ReleaseQueryUGCRequest( pResult->m_handle );
			return;
		}

		for ( uint32_t i = 0; i < details.m_unNumChildren; i++ )
		{
			if ( !s_ServerWorkshopAddons.IsValidIndex( s_ServerWorkshopAddons.Find( children[i] ) ) )
			{
				s_ServerWorkshopAddons.AddToTail( children[i] );
			}

			UpdateAndLoadAddon( children[i], false, true );
		}

		SteamGameServerUGC()->ReleaseQueryUGCRequest( pResult->m_handle );
	}
};
#endif

static void RealLoadAddon( PublishedFileId_t id )
{
	if ( s_DisabledAddons.IsValidIndex( s_DisabledAddons.Find( id ) ) )
	{
		Msg( "Not loading addon %llu: disabled by user\n", id );
		return;
	}

	ISteamUGC *pWorkshop = SteamUGC();
#ifdef GAME_DLL
	if ( engine->IsDedicatedServer() )
	{
		pWorkshop = SteamGameServerUGC();
	}
#endif
	if ( !pWorkshop )
	{
		Warning( "Cannot load addon %llu: no access to the Steam Workshop API!\n", id );
		return;
	}

	if ( pWorkshop->GetItemState( id ) & k_EItemStateLegacyItem )
	{
#ifdef CLIENT_DLL
		if ( cl_workshop_debug.GetBool() )
#else
		if ( sv_workshop_debug.GetBool() )
#endif
			Msg( "UGC item %llu is a legacy item; assuming collection.\n", id );

#ifdef GAME_DLL
		if ( engine->IsDedicatedServer() )
		{
			UGCQueryHandle_t hQuery = pWorkshop->CreateQueryUGCDetailsRequest( &id, 1 );
			pWorkshop->SetReturnOnlyIDs( hQuery, true );
			pWorkshop->SetReturnChildren( hQuery, true );
			pWorkshop->SetAllowCachedResponse( hQuery, 300 );
			new LoadWorkshopCollection_t( pWorkshop->SendQueryUGCRequest( hQuery ) );
		}
		else
#endif
		{
			Warning( "Subscribing to collections is only supported on dedicated servers. Let Ben know if you saw this message in-game.\n" );
		}

		return;
	}

	char szFolderName[MAX_PATH];
	uint64 nSizeOnDisk;
	uint32 nTimeStamp;
	if ( !pWorkshop->GetItemInstallInfo( id, &nSizeOnDisk, szFolderName, sizeof( szFolderName ), &nTimeStamp ) )
	{
		Warning( "Could not get install location for UGC item %llu\n", id );
		return;
	}
	char vpkname[MAX_PATH];
	V_ComposeFileName( szFolderName, "addon.vpk", vpkname, sizeof( vpkname ) );

#ifdef CLIENT_DLL
	if ( cl_workshop_debug.GetBool() )
#else 
	if ( sv_workshop_debug.GetBool() )
#endif
		Msg( "Loading addon %llu\n", id );

	bool bDontClearCache = false;

	LoadedAddonPath_t path;
	path.m_ID = id;
	path.m_Path = vpkname;
	if ( s_LoadedAddonPaths.FindAndRemove( path ) )
	{
		filesystem->RemoveVPKFile( vpkname );
#ifdef GAME_DLL
		if ( engine->IsDedicatedServer() )
		{
			bDontClearCache = true;
		}
	}
	else if ( engine->IsDedicatedServer() )
	{
		s_bAnyServerUpdates = true;
#endif
	}
	s_LoadedAddonPaths.AddToTail( path );

	CPackedStore vpk( vpkname, filesystem );
	CUtlStringList filenames;
	vpk.GetFileList( filenames, false, false );
	FOR_EACH_VEC( filenames, i )
	{
		s_FileNameToAddon[filenames[i]] = id;
	}

	filesystem->AddVPKFile( vpkname, PATH_ADD_TO_HEAD );

	if ( !bDontClearCache )
	{
		ClearCaches( CFmtStr( "loaded addon %lld", id ) );
	}

#ifdef GAME_DLL
	if ( ASWGameRules() )
	{
		char szOverview[MAX_PATH];
		V_snprintf( szOverview, sizeof( szOverview ), "resource/overviews/%s.txt", STRING( gpGlobals->mapname ) );
		ASWGameRules()->m_iMissionWorkshopID = g_ReactiveDropWorkshop.FindAddonProvidingFile( szOverview );
	}
#endif
}

static void LoadAddon( PublishedFileId_t id, bool bFromDownload )
{
#ifdef CLIENT_DLL
	if ( engine->IsConnected() && ASWGameRules() && ASWGameRules()->GetGameState() == ASW_GS_INGAME )
	{
		s_DelayedUnloadAddons.FindAndRemove( id );
		if ( !s_DelayedLoadAddons.IsValidIndex( s_DelayedLoadAddons.Find( id ) ) )
		{
			Msg( "Queued addon %llu for loading at the end of the level.\n", id );
			s_DelayedLoadAddons.AddToTail( id );
		}
		return;
	}
#else
	if ( bFromDownload && !engine->IsDedicatedServer() )
	{
		return;
	}
#endif

	RealLoadAddon( id );
}

static void RealUnloadAddon( PublishedFileId_t id )
{
	LoadedAddonPath_t path;
	bool bFound = false;
	FOR_EACH_VEC( s_LoadedAddonPaths, i )
	{
		if ( s_LoadedAddonPaths[i].m_ID == id )
		{
			path = s_LoadedAddonPaths[i];
			s_LoadedAddonPaths.Remove( i );
			bFound = true;
			break;
		}
	}
	if ( !bFound )
	{
		return;
	}

#ifdef CLIENT_DLL
	// kill currently-playing sounds so the sound system doesn't try to load from a VPK we're removing asynchronously.
	engine->ClientCmd_Unrestricted( "snd_restart" );
#endif

	Msg( "Unloading addon %llu\n", id );

	filesystem->RemoveVPKFile( path.m_Path );

	s_FileNameToAddon.Purge();
	FOR_EACH_VEC( s_LoadedAddonPaths, i )
	{
		CPackedStore vpk( s_LoadedAddonPaths[i].m_Path, filesystem );
		CUtlStringList filenames;
		vpk.GetFileList( filenames, false, false );
		FOR_EACH_VEC( filenames, j )
		{
			s_FileNameToAddon[filenames[j]] = s_LoadedAddonPaths[i].m_ID;
		}
	}

	ClearCaches( CFmtStr( "unloaded addon %llu", id ) );
}

static void UnloadAddon( PublishedFileId_t id )
{
#ifdef CLIENT_DLL
	if ( engine->IsConnected() )
	{
		s_DelayedLoadAddons.FindAndRemove( id );
		if ( !s_DelayedUnloadAddons.IsValidIndex( s_DelayedUnloadAddons.Find( id ) ) )
		{
			Msg( "Queued addon %llu for unloading at the end of the level.\n", id );
			s_DelayedUnloadAddons.AddToTail( id );
		}
		return;
	}
#else
	if ( !engine->IsDedicatedServer() )
	{
		return;
	}
#endif

	RealUnloadAddon( id );
}

#ifdef CLIENT_DLL
bool CReactiveDropWorkshop::LoadAddonEarly( PublishedFileId_t nPublishedFileID )
{
	if ( !IsSubscribedToFile( nPublishedFileID ) )
	{
		return false;
	}

	if ( !IsAddonEnabled( nPublishedFileID ) )
	{
		SetAddonEnabled( nPublishedFileID, true );
	}

	if ( !s_DelayedLoadAddons.IsValidIndex( s_DelayedLoadAddons.Find( nPublishedFileID ) ) )
	{
		FOR_EACH_VEC( s_LoadedAddonPaths, i )
		{
			if ( s_LoadedAddonPaths[i].m_ID == nPublishedFileID )
			{
				return true;
			}
		}
		return false;
	}

	Msg( "Forcing addons to load early!\n" );
	FOR_EACH_VEC( s_DelayedLoadAddons, i )
	{
		RealLoadAddon( s_DelayedLoadAddons[i] );
	}
	s_DelayedLoadAddons.RemoveAll();

	return true;
}
#endif

#ifdef CLIENT_DLL
static const char *const s_AutoTags[] =
{
	"Campaign",
	"Challenge",
	"Deathmatch",
	"Bonus",
	"Endless",
	"Other",
};

bool CReactiveDropWorkshop::IsAutoTag( const char *szTag )
{
	for ( int i = 0; i < NELEMS( s_AutoTags ); i++ )
	{
		if ( !V_stricmp( s_AutoTags[i], szTag ) )
		{
			return true;
		}
	}
	return false;
}

// These files are frequently added by 
static const char *s_BlacklistedAddonFileNames[] =
{
	"maps/soundcache/_master.cache",
	"materials/vgui/ExampleCustomLogo/example_custom_logo.vmt",
	"materials/vgui/ExampleCustomLogo/example_custom_logo.vtf",
	"resource/CustomCampaignCredits.txt",
	"resource/campaigns/ExampleCampaign.txt",
	"resource/alien_selection.txt",
};

bool CReactiveDropWorkshop::PrepareWorkshopVPK( const char *pszContentPath, CUtlString *pszDisallowedFiles )
{
	if ( !m_szContentPath.IsEmpty() )
	{
		Warning( "Already uploading UGC content!\n" );
		return false;
	}

	m_aszTags.PurgeAndDeleteElements();
	m_aszIncludedCampaigns.PurgeAndDeleteElements();
	m_aszIncludedMissions.PurgeAndDeleteElements();
	m_aszIncludedChallenges.PurgeAndDeleteElements();
#if RD_NUM_WORKSHOP_CAMPAIGN_TAGS
	for ( int i = 0; i < RD_NUM_WORKSHOP_CAMPAIGN_TAGS; i++ )
		m_aszIncludedTaggedCampaigns[i].PurgeAndDeleteElements();
#endif
	for ( int i = 0; i < RD_NUM_WORKSHOP_MISSION_TAGS; i++ )
		m_aszIncludedTaggedMissions[i].PurgeAndDeleteElements();
	m_IncludedCampaignNames.Purge();
	m_IncludedCampaignMissions.Purge();
	m_IncludedMissionNames.Purge();
	m_IncludedChallengeNames.Purge();

	CPackedStore vpk( pszContentPath, filesystem );
	if ( vpk.IsEmpty() )
	{
		Warning( "Missing or empty VPK: %s\n", pszContentPath );
		return false;
	}

	bool bAnyBlacklistedFileNames = false;
	for ( int i = 0; i < NELEMS( s_BlacklistedAddonFileNames ); i++ )
	{
		if ( !vpk.OpenFile( s_BlacklistedAddonFileNames[i] ) )
		{
			continue;
		}

		if ( !bAnyBlacklistedFileNames )
		{
			bAnyBlacklistedFileNames = true;
			Warning( "Some files in your addon have commonly duplicated names and will need to be renamed:\n" );
		}
		Warning( "%s\n", s_BlacklistedAddonFileNames[i] );
		if ( pszDisallowedFiles )
		{
			*pszDisallowedFiles += s_BlacklistedAddonFileNames[i];
			*pszDisallowedFiles += "\n";
		}
	}

	if ( bAnyBlacklistedFileNames )
	{
		return false;
	}

	bool bHaveAutoTag = false;

	CUtlStringList filenames;
	vpk.GetFileList( filenames, false, false );
	FOR_EACH_VEC( filenames, i )
	{
		if ( StringHasPrefix( filenames[i], "resource/campaigns/" ) )
		{
			char szFileName[MAX_PATH];
			V_FileBase( filenames[i], szFileName, sizeof( szFileName ) );
			char szFileNameVerify[MAX_PATH];
			V_snprintf( szFileNameVerify, sizeof( szFileNameVerify ), "resource/campaigns/%s.txt", szFileName );

			if ( FStrEq( filenames[i], szFileNameVerify ) )
			{
				bHaveAutoTag = true;
				m_aszTags.CopyAndAddToTail( "Campaign" );
				m_aszIncludedCampaigns.CopyAndAddToTail( szFileName );

				KeyValues::AutoDelete pKV( "GAME" );
				CPackedStoreFileHandle hCampaign = vpk.OpenFile( szFileNameVerify );
				CUtlBuffer buf;
				hCampaign.Read( buf.AccessForDirectRead( hCampaign.m_nFileSize ), hCampaign.m_nFileSize );
				buf.SetBufferType( true, true );
				pKV->LoadFromBuffer( szFileNameVerify, buf );
				m_IncludedCampaignNames[szFileName] = pKV->GetString( "CampaignName", "Invalid Campaign" );
				CUtlStringList & aszCampaignMissions = m_IncludedCampaignMissions[szFileName];
				bool bSkippedFirst = false;
				FOR_EACH_TRUE_SUBKEY( pKV, pMission )
				{
					if ( V_stricmp( pMission->GetName(), "MISSION" ) )
					{
						continue;
					}
					if ( !bSkippedFirst )
					{
						bSkippedFirst = true;
						continue;
					}
					if ( const char *szMapName = pMission->GetString( "MapName", NULL ) )
					{
						aszCampaignMissions.CopyAndAddToTail( szMapName );
					}
				}

#if RD_NUM_WORKSHOP_CAMPAIGN_TAGS
				for ( int j = 0; j < RD_NUM_WORKSHOP_CAMPAIGN_TAGS; j++ )
				{
					FOR_EACH_VALUE( pKV, pValue )
					{
						if ( V_stricmp( pValue->GetName(), "tag" ) )
							continue;

						if ( V_stricmp( pValue->GetString(), g_RDWorkshopCampaignTags[j] ) )
							continue;

						m_aszIncludedTaggedCampaigns[j].CopyAndAddToTail( szFileName );

						for ( int k = 0; k < NELEMS( s_AutoTags ); k++ )
						{
							if ( !V_stricmp( g_RDWorkshopCampaignTags[j], s_AutoTags[k] ) )
							{
								bHaveAutoTag = true;
								m_aszTags.CopyAndAddToTail( s_AutoTags[k] );
								break;
							}
						}

						break;
					}
				}
#endif
			}
		}
		if ( StringHasPrefix( filenames[i], "resource/overviews/" ) )
		{
			char szFileName[MAX_PATH];
			V_FileBase( filenames[i], szFileName, sizeof( szFileName ) );
			char szFileNameVerify[MAX_PATH];
			V_snprintf( szFileNameVerify, sizeof( szFileNameVerify ), "resource/overviews/%s.txt", szFileName );

			if ( FStrEq( filenames[i], szFileNameVerify ) )
			{
				m_aszIncludedMissions.CopyAndAddToTail( szFileName );

				KeyValues::AutoDelete pKV( "GAME" );
				CPackedStoreFileHandle hMission = vpk.OpenFile( szFileNameVerify );
				CUtlBuffer buf;
				hMission.Read( buf.AccessForDirectRead( hMission.m_nFileSize ), hMission.m_nFileSize );
				buf.SetBufferType( true, true );
				pKV->LoadFromBuffer( szFileNameVerify, buf );
				m_IncludedMissionNames[szFileName] = pKV->GetString( "missiontitle", "Invalid Mission" );

				for ( int j = 0; j < RD_NUM_WORKSHOP_MISSION_TAGS; j++ )
				{
					FOR_EACH_VALUE( pKV, pValue )
					{
						if ( V_stricmp( pValue->GetName(), "tag" ) )
							continue;

						if ( V_stricmp( pValue->GetString(), g_RDWorkshopMissionTags[j] ) )
							continue;

						m_aszIncludedTaggedMissions[j].CopyAndAddToTail( szFileName );

						for ( int k = 0; k < NELEMS( s_AutoTags ); k++ )
						{
							if ( !V_stricmp( g_RDWorkshopMissionTags[j], s_AutoTags[k] ) )
							{
								bHaveAutoTag = true;
								m_aszTags.CopyAndAddToTail( s_AutoTags[k] );
								break;
							}
						}

						break;
					}
				}
			}
		}
		if ( StringHasPrefix( filenames[i], "resource/challenges/" ) )
		{
			char szFileName[MAX_PATH];
			V_FileBase( filenames[i], szFileName, sizeof( szFileName ) );
			char szFileNameVerify[MAX_PATH];
			V_snprintf( szFileNameVerify, sizeof( szFileNameVerify ), "resource/challenges/%s.txt", szFileName );

			if ( FStrEq( filenames[i], szFileNameVerify ) )
			{
				bHaveAutoTag = true;
				m_aszTags.CopyAndAddToTail( "Challenge" );
				m_aszIncludedChallenges.CopyAndAddToTail( szFileName );

				KeyValues::AutoDelete pKV( "CHALLENGE" );
				CPackedStoreFileHandle hChallenge = vpk.OpenFile( szFileNameVerify );
				CUtlBuffer buf;
				hChallenge.Read( buf.AccessForDirectRead( hChallenge.m_nFileSize ), hChallenge.m_nFileSize );
				buf.SetBufferType( true, true );
				pKV->LoadFromBuffer( szFileNameVerify, buf );
				m_IncludedChallengeNames[szFileName] = pKV->GetString( "name", "Invalid Challenge" );
			}
		}
	}
	if ( bAnyBlacklistedFileNames )
	{
		return false;
	}
	if ( !bHaveAutoTag )
	{
		m_aszTags.CopyAndAddToTail( "Other" );
	}

	char szModDir[MAX_PATH];
	if ( !V_GetCurrentDirectory( szModDir, sizeof( szModDir ) ) )
	{
		Warning( "Could not make temporary folder.\n" );
		return false;
	}
	char szPath[MAX_PATH];
	V_ComposeFileName( szModDir, VarArgs( "steam_ugc_temp_%llu", SteamUser()->GetSteamID().ConvertToUint64() ), szPath, sizeof( szPath ) );
	if ( filesystem->FileExists( szPath ) )
	{
		filesystem->RemoveFile( CUtlString::PathJoin( szPath, "addon.vpk" ) );
#if IS_WINDOWS_PC
		if ( !RemoveDirectory( szPath ) )
#else
#error need folder deletion code
#endif
		{
			Warning( "Please manually delete %s\n", szPath );
			return false;
		}
	}
	filesystem->CreateDirHierarchy( szPath );

	char buf[8192];
	FileHandle_t hRead = filesystem->Open( pszContentPath, "rb" );
	if ( hRead == FILESYSTEM_INVALID_HANDLE )
	{
		Warning( "Could not open %s for reading!\n", pszContentPath );
		return false;
	}
	char szWritePath[MAX_PATH];
	V_ComposeFileName( szPath, "addon.vpk", szWritePath, sizeof( szWritePath ) );
	FileHandle_t hWrite = filesystem->Open( szWritePath, "wb" );
	if ( hWrite == FILESYSTEM_INVALID_HANDLE )
	{
		filesystem->Close( hRead );
		Warning( "Could not copy addon VPK!\n" );
		return false;
	}
	for (;;)
	{
		int nBytes = filesystem->Read( buf, sizeof( buf ), hRead );
		if ( nBytes <= 0 )
		{
			break;
		}
		if ( filesystem->Write( buf, nBytes, hWrite ) != nBytes )
		{
			filesystem->Close( hRead );
			filesystem->Close( hWrite );
			Warning( "Could not copy addon VPK!\n" );
			return false;
		}
	}
	filesystem->Close( hRead );
	filesystem->Close( hWrite );

	m_szContentPath = szPath;

	return true;
}

void CReactiveDropWorkshop::UploadWorkshopItem( const char *pszContentPath, const char *pszPreviewImagePath, const char *pszTitle, const char *pszDescription, const CUtlVector<const char *> & tags )
{
	if ( !PrepareWorkshopVPK( pszContentPath ) )
	{
		return;
	}

	m_szPreviewImagePath = pszPreviewImagePath;
	m_szTitle = pszTitle;
	m_szDescription = pszDescription;
	FOR_EACH_VEC( tags, i )
	{
		if ( IsAutoTag( tags[i] ) )
		{
			continue;
		}
		m_aszTags.CopyAndAddToTail( tags[i] );
	}
	RemoveDuplicateTags();
	Msg( "Sent request to Steam Workshop server...\n" );
	SteamAPICall_t hAPICall = SteamUGC()->CreateItem( SteamUtils()->GetAppID(), k_EWorkshopFileTypeCommunity );
	m_CreateItemResultCallback.Set( hAPICall, this, &CReactiveDropWorkshop::CreateItemResultCallback );
}

void CReactiveDropWorkshop::RemoveDuplicateTags()
{
	CUtlSymbolTable existing;

	FOR_EACH_VEC_BACK( m_aszTags, i )
	{
		if ( existing.Find( m_aszTags[i] ) != UTL_INVAL_SYMBOL )
		{
			delete m_aszTags[i];
			m_aszTags.Remove( i );
			continue;
		}
		existing.AddString( m_aszTags[i] );
	}
}

void CReactiveDropWorkshop::SetWorkshopKeyValues( UGCUpdateHandle_t hUpdate )
{
	ISteamUGC *pUGC = SteamUGC();
	pUGC->RemoveItemKeyValueTags( hUpdate, "campaigns" );
	pUGC->RemoveItemKeyValueTags( hUpdate, "missions" );
	pUGC->RemoveItemKeyValueTags( hUpdate, "challenges" );

	for ( int i = 0; i < NELEMS( g_RDWorkshopMissionTags ); i++ )
	{
		pUGC->RemoveItemKeyValueTags( hUpdate, g_RDWorkshopMissionTags[i] );
	}

	pUGC->RemoveItemKeyValueTags( hUpdate, "campaign_name" );
	pUGC->RemoveItemKeyValueTags( hUpdate, "campaign_mission" );
	pUGC->RemoveItemKeyValueTags( hUpdate, "mission_name" );
	pUGC->RemoveItemKeyValueTags( hUpdate, "challenge_name" );

	FOR_EACH_VEC( m_aszIncludedCampaigns, i )
	{
		if ( !pUGC->AddItemKeyValueTag( hUpdate, "campaigns", m_aszIncludedCampaigns[i] ) )
		{
			Warning( "Adding campaign %s failed!\n", m_aszIncludedCampaigns[i] );
		}
	}

	FOR_EACH_VEC( m_aszIncludedMissions, i )
	{
		if ( !pUGC->AddItemKeyValueTag( hUpdate, "missions", m_aszIncludedMissions[i] ) )
		{
			Warning( "Adding mission %s failed!\n", m_aszIncludedMissions[i] );
		}
	}

	FOR_EACH_VEC( m_aszIncludedChallenges, i )
	{
		if ( !pUGC->AddItemKeyValueTag( hUpdate, "challenges", m_aszIncludedChallenges[i] ) )
		{
			Warning( "Adding challenge %s failed!\n", m_aszIncludedChallenges[i] );
		}
	}

#if RD_NUM_WORKSHOP_CAMPAIGN_TAGS
	for ( int i = 0; i < RD_NUM_WORKSHOP_CAMPAIGN_TAGS; i++ )
	{
		FOR_EACH_VEC( m_aszIncludedTaggedCampaigns[i], j )
		{
			if ( !pUGC->AddItemKeyValueTag( hUpdate, g_RDWorkshopCampaignTags[i], m_aszIncludedTaggedCampaigns[i][j] ) )
			{
				Warning( "Adding %s %s failed!\n", g_RDWorkshopCampaignTags[i], m_aszIncludedTaggedCampaigns[i][j] );
			}
		}
	}
#endif

	for ( int i = 0; i < RD_NUM_WORKSHOP_MISSION_TAGS; i++ )
	{
		FOR_EACH_VEC( m_aszIncludedTaggedMissions[i], j )
		{
			if ( !pUGC->AddItemKeyValueTag( hUpdate, g_RDWorkshopMissionTags[i], m_aszIncludedTaggedMissions[i][j] ) )
			{
				Warning( "Adding %s %s failed!\n", g_RDWorkshopMissionTags[i], m_aszIncludedTaggedMissions[i][j] );
			}
		}
	}

	for ( int i = 0; i < m_IncludedCampaignNames.GetNumStrings(); i++ )
	{
		if ( !pUGC->AddItemKeyValueTag( hUpdate, "campaign_name", CUtlString( m_IncludedCampaignNames.String( i ) ) + "/" + m_IncludedCampaignNames[i] ) )
		{
			Warning( "Adding campaign name: %s -> %s failed!\n", m_IncludedCampaignNames.String( i ), m_IncludedCampaignNames[i].Get() );
		}
	}

	for ( int i = 0; i < m_IncludedCampaignMissions.GetNumStrings(); i++ )
	{
		CUtlString szCampaignMission;
		FOR_EACH_VEC( m_IncludedCampaignMissions[i], j )
		{
			szCampaignMission.Format( "%s/%d/%s", m_IncludedCampaignMissions.String( i ), j, m_IncludedCampaignMissions[i][j] );
			if ( !pUGC->AddItemKeyValueTag( hUpdate, "campaign_mission",  szCampaignMission ) )
			{
				Warning( "Adding campaign mission: %s -> %d -> %s failed!\n", m_IncludedCampaignMissions.String( i ), j, m_IncludedCampaignMissions[i][j] );
			}
		}
	}

	for ( int i = 0; i < m_IncludedMissionNames.GetNumStrings(); i++ )
	{
		if ( !pUGC->AddItemKeyValueTag( hUpdate, "mission_name", CUtlString( m_IncludedMissionNames.String( i ) ) + "/" + m_IncludedMissionNames[i] ) )
		{
			Warning( "Adding mission name: %s -> %s failed!\n", m_IncludedMissionNames.String( i ), m_IncludedMissionNames[i].Get() );
		}
	}
	
	for ( int i = 0; i < m_IncludedChallengeNames.GetNumStrings(); i++ )
	{
		if ( !pUGC->AddItemKeyValueTag( hUpdate, "challenge_name", CUtlString( m_IncludedChallengeNames.String( i ) ) + "/" + m_IncludedChallengeNames[i] ) )
		{
			Warning( "Adding challenge name: %s -> %s failed!\n", m_IncludedChallengeNames.String( i ), m_IncludedChallengeNames[i].Get() );
		}
	}
}

CON_COMMAND_F( ugc_create, "Usage: ugc_create \"C:\\Path\\to\\content.vpk\" \"C:\\Path\\to\\preview\\image.jpeg\" \"Title\" \"Description\" \"Tag1\" \"Tag2\" ...\nCampaign, Challenge, and Deathmatch will automatically be added as tags if applicable.", FCVAR_NOT_CONNECTED )
{
	if ( !SteamUGC() )
	{
		Warning( "No Steam connection. Cannot interact with workshop.\n" );
		return;
	}

	if ( args.ArgC() < 5 )
	{
		Warning( "Missing arguments to ugc_create!\n" );
		Msg( "%s\n", ugc_create_command.GetHelpText() );
		return;
	}

	for ( int i = 0; i < args.ArgC(); i++ )
	{
		// make sure no args are empty
		if ( !*args[i] )
		{
			Warning( "Missing arguments to ugc_create!\n" );
			return;
		}
	}

	CUtlVector<const char *> tags;
	for ( int i = 5; i <= args.ArgC(); i++ )
	{
		tags.AddToTail( args[i] );
	}

	g_ReactiveDropWorkshop.UploadWorkshopItem( args[1], args[2], args[3], args[4], tags );
}

void CReactiveDropWorkshop::CreateItemResultCallback( CreateItemResult_t *pResult, bool bIOFailure )
{
	if ( bIOFailure || pResult->m_eResult != k_EResultOK )
	{
		m_szContentPath.Purge();
		m_szPreviewImagePath.Purge();
		m_szTitle.Purge();
		m_szDescription.Purge();
		m_aszTags.PurgeAndDeleteElements();
		m_aszIncludedCampaigns.PurgeAndDeleteElements();
		m_aszIncludedMissions.PurgeAndDeleteElements();
		m_aszIncludedChallenges.PurgeAndDeleteElements();
#if RD_NUM_WORKSHOP_CAMPAIGN_TAGS
		for ( int i = 0; i < RD_NUM_WORKSHOP_CAMPAIGN_TAGS; i++ )
			m_aszIncludedTaggedCampaigns[i].PurgeAndDeleteElements();
#endif
		for ( int i = 0; i < RD_NUM_WORKSHOP_MISSION_TAGS; i++ )
			m_aszIncludedTaggedMissions[i].PurgeAndDeleteElements();
		m_IncludedCampaignNames.Purge();
		m_IncludedCampaignMissions.Purge();
		m_IncludedMissionNames.Purge();
		m_IncludedChallengeNames.Purge();
		switch ( pResult->m_eResult )
		{
		case k_EResultInsufficientPrivilege:
			Warning( "You do not have permission to upload Reactive Drop workshop items.\n" );
			break;
		case k_EResultTimeout:
			Warning( "Steam Workshop operation took too long. Please try again later.\n" );
			break;
		case k_EResultNotLoggedOn:
			Warning( "You are not currently logged into Steam. Try restarting the Steam client and Reactive Drop.\n" );
			break;
		default:
			Warning( "Steam Workshop CreateItem API call failed! EResult: %d (%s)\n", pResult->m_eResult, UTIL_RD_EResultToString( pResult->m_eResult ) );
			break;
		}
		return;
	}

	if ( pResult->m_bUserNeedsToAcceptWorkshopLegalAgreement )
	{
		Warning( "Your addon will not be visible until you accept the Steam Workshop legal agreement. https://steamcommunity.com/sharedfiles/workshoplegalagreement\n" );
	}

	m_nLastPublishedFileID = pResult->m_nPublishedFileId;
	Msg( "Workshop assigned published file ID: %llu\n", pResult->m_nPublishedFileId );
	UGCUpdateHandle_t hUpdate = SteamUGC()->StartItemUpdate( SteamUtils()->GetAppID(), pResult->m_nPublishedFileId );
	if ( !SteamUGC()->SetItemTitle( hUpdate, m_szTitle ) )
	{
		Warning( "Setting title failed!\n" );
	}
	if ( !SteamUGC()->SetItemDescription( hUpdate, m_szDescription ) )
	{
		Warning( "Setting description failed!\n" );
	}
	if ( m_aszTags.Count() )
	{
		SteamParamStringArray_t tags;
		tags.m_nNumStrings = m_aszTags.Count();
		tags.m_ppStrings = const_cast<const char **>( m_aszTags.Base() );
		if ( !SteamUGC()->SetItemTags( hUpdate, &tags ) )
		{
			Warning( "Setting tags failed!\n" );
		}
	}
	SetWorkshopKeyValues( hUpdate );
	if ( !SteamUGC()->SetItemPreview( hUpdate, m_szPreviewImagePath ) )
	{
		Warning( "Setting preview image failed!\n" );
	}
	if ( !SteamUGC()->SetItemContent( hUpdate, m_szContentPath ) )
	{
		Warning( "Setting addon VPK failed!\n" );
	}
	m_hUpdate = hUpdate;
	SteamAPICall_t hAPICall = SteamUGC()->SubmitItemUpdate( hUpdate, "" );
	m_SubmitItemUpdateResultCallback.Set( hAPICall, this, &CReactiveDropWorkshop::SubmitItemUpdateResultCallback );
	engine->ClientCmd_Unrestricted( "_ugc_update_progress\n" );
}

void CReactiveDropWorkshop::SubmitItemUpdateResultCallback( SubmitItemUpdateResult_t *pResult, bool bIOFailure )
{
	// clear variables
	m_hUpdate = k_UGCUpdateHandleInvalid;
	m_szContentPath.Purge();
	m_szPreviewImagePath.Purge();
	m_szTitle.Purge();
	m_szDescription.Purge();
	m_aszTags.PurgeAndDeleteElements();
	m_aszIncludedCampaigns.PurgeAndDeleteElements();
	m_aszIncludedMissions.PurgeAndDeleteElements();
	m_aszIncludedChallenges.PurgeAndDeleteElements();
#if RD_NUM_WORKSHOP_CAMPAIGN_TAGS
	for ( int i = 0; i < RD_NUM_WORKSHOP_CAMPAIGN_TAGS; i++ )
		m_aszIncludedTaggedCampaigns[i].PurgeAndDeleteElements();
#endif
	for ( int i = 0; i < RD_NUM_WORKSHOP_MISSION_TAGS; i++ )
		m_aszIncludedTaggedMissions[i].PurgeAndDeleteElements();
	m_IncludedCampaignNames.Purge();
	m_IncludedCampaignMissions.Purge();
	m_IncludedMissionNames.Purge();
	m_IncludedChallengeNames.Purge();

	if ( bIOFailure || pResult->m_eResult != k_EResultOK )
	{
		Warning( "Steam Workshop SubmitItemUpdate API call failed! EResult: %d (%s)\n", pResult->m_eResult, UTIL_RD_EResultToString( pResult->m_eResult ) );
		return;
	}

	if ( OpenWorkshopPageForFile( m_nLastPublishedFileID ) )
	{
		Msg( "Addon upload complete! Opening the workshop page in the Steam overlay...\n" );
	}
	else
	{
		Msg( "Addon upload complete! Opening the workshop page in a browser...\n" );
	}
}

bool CReactiveDropWorkshop::OpenWorkshopPageForFile( PublishedFileId_t nPublishedFileID )
{
	if ( SteamFriends() && SteamUtils() && SteamUtils()->IsOverlayEnabled() )
	{
		SteamFriends()->ActivateGameOverlayToWebPage( VarArgs( "https://steamcommunity.com/sharedfiles/filedetails/?id=%llu", nPublishedFileID ), k_EActivateGameOverlayToWebPageMode_Modal );
		return true;
	}

	vgui::system()->ShellExecute( "open", VarArgs( "steam://url/CommunityFilePage/%llu", nPublishedFileID ) );
	return false;
}

CON_COMMAND_F( _ugc_update_progress, "", FCVAR_HIDDEN )
{
	if ( !SteamUGC() )
	{
		return;
	}

	if ( g_ReactiveDropWorkshop.m_hUpdate == k_UGCUpdateHandleInvalid )
	{
		return;
	}

	uint64 nBytesProcessed, nBytesTotal;
	EItemUpdateStatus status = SteamUGC()->GetItemUpdateProgress( g_ReactiveDropWorkshop.m_hUpdate, &nBytesProcessed, &nBytesTotal );
	Msg( "Uploading addon to Steam Workshop..." );
	if ( nBytesTotal != 0 )
	{
		Msg( " %llu / %llu", nBytesProcessed, nBytesTotal );
	}
	switch ( status )
	{
	case k_EItemUpdateStatusInvalid:
	default:
		Msg( "\n" );
		break;
	case k_EItemUpdateStatusPreparingConfig:
		Msg( " (processing configuration data)\n" );
		break;
	case k_EItemUpdateStatusPreparingContent:
		Msg( " (reading and processing content files)\n" );
		break;
	case k_EItemUpdateStatusUploadingContent:
		Msg( " (uploading content changes to Steam)\n" );
		break;
	case k_EItemUpdateStatusUploadingPreviewFile:
		Msg( " (uploading new preview file image)\n" );
		break;
	case k_EItemUpdateStatusCommittingChanges:
		Msg( " (committing all changes)\n" );
		break;
	}
	engine->ClientCmd_Unrestricted( "wait 60; _ugc_update_progress\n" );
}

void CReactiveDropWorkshop::UpdateWorkshopItem( PublishedFileId_t nFileID, const char *pszContentPath, const char *pszPreviewImagePath, const char *pszChangeDescription )
{
	if ( !PrepareWorkshopVPK( pszContentPath ) )
	{
		return;
	}

	m_nLastPublishedFileID = nFileID;
	UGCUpdateHandle_t hUpdate = SteamUGC()->StartItemUpdate( SteamUtils()->GetAppID(), nFileID );
	if ( !SteamUGC()->SetItemContent( hUpdate, m_szContentPath ) )
	{
		Warning( "Setting addon VPK failed!\n" );
	}
	if ( *pszPreviewImagePath && !SteamUGC()->SetItemPreview( hUpdate, pszPreviewImagePath ) )
	{
		Warning( "Setting preview image failed!\n" );
	}
	SetWorkshopKeyValues( hUpdate );
	m_hUpdate = hUpdate;
	m_szUpdateChangeDescription = pszChangeDescription;
	m_hUpdateWorkshopItemQuery = SteamUGC()->CreateQueryUGCDetailsRequest( &nFileID, 1 );
	m_bWantAutoTags = false;
	SteamAPICall_t hAPICall = SteamUGC()->SendQueryUGCRequest( m_hUpdateWorkshopItemQuery );
	m_UpdateWorkshopItemQueryResultCallback.Set( hAPICall, this, &CReactiveDropWorkshop::UpdateWorkshopItemQueryResultCallback );
}

void CReactiveDropWorkshop::UpdateWorkshopItemQueryResultCallback( SteamUGCQueryCompleted_t *pResult, bool bIOFailure )
{
	if ( bIOFailure || pResult->m_eResult != k_EResultOK )
	{
		m_hUpdate = k_UGCUpdateHandleInvalid;
		m_szContentPath.Purge();
		m_szPreviewImagePath.Purge();
		m_szTitle.Purge();
		m_szDescription.Purge();
		m_aszTags.PurgeAndDeleteElements();
		m_aszIncludedCampaigns.PurgeAndDeleteElements();
		m_aszIncludedMissions.PurgeAndDeleteElements();
		m_aszIncludedChallenges.PurgeAndDeleteElements();
#if RD_NUM_WORKSHOP_CAMPAIGN_TAGS
		for ( int i = 0; i < RD_NUM_WORKSHOP_CAMPAIGN_TAGS; i++ )
			m_aszIncludedTaggedCampaigns[i].PurgeAndDeleteElements();
#endif
		for ( int i = 0; i < RD_NUM_WORKSHOP_MISSION_TAGS; i++ )
			m_aszIncludedTaggedMissions[i].PurgeAndDeleteElements();
		m_IncludedCampaignNames.Purge();
		m_IncludedCampaignMissions.Purge();
		m_IncludedMissionNames.Purge();
		m_IncludedChallengeNames.Purge();

		Warning( "Steam Workshop query for existing tags failed! EResult: %d (%s)\n", pResult->m_eResult, UTIL_RD_EResultToString( pResult->m_eResult ) );
		return;
	}

	SteamUGCDetails_t details;
	if ( !SteamUGC()->GetQueryUGCResult( pResult->m_handle, 0, &details ) )
	{
		Warning( "Failed to retrieve tags! Manually specified tags may be lost in this update.\n" );
	}
	else
	{
		CSplitString existingTags( details.m_rgchTags, "," );
		FOR_EACH_VEC( existingTags, i )
		{
			if ( IsAutoTag( existingTags[i] ) != m_bWantAutoTags )
			{
				m_aszTags.CopyAndAddToTail( existingTags[i] );
			}
		}
	}

	SteamParamStringArray_t tags;
	tags.m_nNumStrings = m_aszTags.Count();
	tags.m_ppStrings = const_cast<const char **>( m_aszTags.Base() );
	if ( !SteamUGC()->SetItemTags( m_hUpdate, &tags ) )
	{
		Warning( "Setting tags failed!\n" );
	}
	Msg( "Sending update to workshop for addon ID %llu...\n", m_nLastPublishedFileID );
	SteamAPICall_t hAPICall = SteamUGC()->SubmitItemUpdate( m_hUpdate, m_szUpdateChangeDescription );
	m_SubmitItemUpdateResultCallback.Set( hAPICall, this, &CReactiveDropWorkshop::SubmitItemUpdateResultCallback );
	engine->ClientCmd_Unrestricted( "_ugc_update_progress\n" );
	m_szUpdateChangeDescription.Purge();
}

CON_COMMAND_F( ugc_update, "Usage: ugc_update 826481632 \"C:\\Path\\to\\content.vpk\" \"C:\\Path\\to\\preview.jpg\" \"Description of changes line 1\" \"Description of changes line 2\" ...\n(the number should be the number in the address of your workshop item after http://steamcommunity.com/sharedfiles/filedetails/?id=)\nIf the preview image should not be updated, use \"\" instead of a path.", FCVAR_NOT_CONNECTED )
{
	if ( !SteamUGC() )
	{
		Warning( "No Steam connection. Cannot interact with workshop.\n" );
		return;
	}

	if ( args.ArgC() < 4 )
	{
		Warning( "Missing arguments to ugc_update!\n" );
		Msg( "%s\n", ugc_update_command.GetHelpText() );
		return;
	}

	for ( int i = 0; i < args.ArgC(); i++ )
	{
		// make sure no args are empty
		if ( i != 3 && !*args[i] )
		{
			Warning( "Missing arguments to ugc_update!\n" );
			return;
		}
	}

	char *pEnd = NULL;
	PublishedFileId_t nFileID = strtoull( args[1], &pEnd, 10 );
	if ( nFileID == 0 || !pEnd || *pEnd )
	{
		Warning( "Invalid workshop file ID!\n" );
		return;
	}

	CUtlString changeDescription;
	for ( int i = 4; i <= args.ArgC(); i++ )
	{
		if ( i != 4 )
		{
			changeDescription += "\n\n";
		}
		changeDescription += args[i];
	}

	g_ReactiveDropWorkshop.UpdateWorkshopItem( nFileID, args[2], args[3], changeDescription );
}

void CReactiveDropWorkshop::SetWorkshopItemTags( PublishedFileId_t nFileID, const CUtlVector<const char *> & aszTags )
{
	if ( !m_szContentPath.IsEmpty() )
	{
		Warning( "Already uploading UGC content!\n" );
		return;
	}

	m_szContentPath = "dummy";

	m_aszTags.PurgeAndDeleteElements();
	FOR_EACH_VEC( aszTags, i )
	{
		if ( !IsAutoTag( aszTags[i] ) )
		{
			m_aszTags.CopyAndAddToTail( aszTags[i] );
		}
	}
	RemoveDuplicateTags();

	m_nLastPublishedFileID = nFileID;
	UGCUpdateHandle_t hUpdate = SteamUGC()->StartItemUpdate( SteamUtils()->GetAppID(), nFileID );
	m_hUpdate = hUpdate;
	m_szUpdateChangeDescription = "";
	m_hUpdateWorkshopItemQuery = SteamUGC()->CreateQueryUGCDetailsRequest( &nFileID, 1 );
	m_bWantAutoTags = true;
	SteamAPICall_t hAPICall = SteamUGC()->SendQueryUGCRequest( m_hUpdateWorkshopItemQuery );
	m_UpdateWorkshopItemQueryResultCallback.Set( hAPICall, this, &CReactiveDropWorkshop::UpdateWorkshopItemQueryResultCallback );
}

CON_COMMAND_F( ugc_updatetags, "Usage: ugc_updatetags 826481632 \"Tag1\" \"Tag2\" ...\n(the number should be the number in the address of your workshop item after http://steamcommunity.com/sharedfiles/filedetails/?id=)\nSome tags are automatically determined from the contents of your addon and cannot be added or removed using this command.", FCVAR_NOT_CONNECTED )
{
	if ( !SteamUGC() )
	{
		Warning( "No Steam connection. Cannot interact with workshop.\n" );
		return;
	}

	if ( args.ArgC() < 2 )
	{
		Warning( "Missing arguments to ugc_updatetags!\n" );
		Msg( "%s\n", ugc_updatetags_command.GetHelpText() );
		return;
	}

	for ( int i = 0; i < args.ArgC(); i++ )
	{
		// make sure no args are empty
		if ( !*args[i] )
		{
			Warning( "Missing arguments to ugc_updatetags!\n" );
			return;
		}
	}

	char *pEnd = NULL;
	PublishedFileId_t nFileID = strtoull( args[1], &pEnd, 10 );
	if ( nFileID == 0 || !pEnd || *pEnd )
	{
		Warning( "Invalid workshop file ID!\n" );
		return;
	}

	CUtlVector<const char *> tags;
	for ( int i = 2; i < args.ArgC(); i++ )
	{
		tags.AddToTail( args[i] );
	}

	g_ReactiveDropWorkshop.SetWorkshopItemTags( nFileID, tags );
}

class CFixWorkshopKeyValueNames
{
public:
	CFixWorkshopKeyValueNames( PublishedFileId_t nFile )
	{
		m_nFile = nFile;
		Msg( "Initializing fix for workshop display names for addon %llu\n", nFile );

		if ( SteamUGC()->GetItemState( nFile ) & k_EItemStateDownloadPending )
		{
			return;
		}

		if ( SteamUGC()->GetItemState( nFile ) & k_EItemStateInstalled )
		{
			PerformFix();
			return;
		}

		if ( !SteamUGC()->DownloadItem( nFile, false ) )
		{
			Warning( "Failed to apply fix: DownloadItem returned false for %llu\n", nFile );
			delete this;
		}
	}

	void PerformFix()
	{
		uint64 nSizeOnDisk;
		uint32 nTimeStamp;
		char szFolder[MAX_PATH];
		if ( !SteamUGC()->GetItemInstallInfo( m_nFile, &nSizeOnDisk, szFolder, sizeof( szFolder ), &nTimeStamp ) )
		{
			Warning( "Failed to apply fix: GetItemInstallInfo returned false for %llu\n", m_nFile );
			delete this;
			return;
		}

		char szVPK[MAX_PATH];
		V_ComposeFileName( szFolder, "addon.vpk", szVPK, sizeof( szVPK ) );

		g_ReactiveDropWorkshop.m_szContentPath.Purge();
		if ( !g_ReactiveDropWorkshop.PrepareWorkshopVPK( szVPK ) )
		{
			Warning( "Failed to apply fix: PrepareWorkshopVPK failed for %llu\n", m_nFile );
			delete this;
			return;
		}

		UGCUpdateHandle_t hUpdate = SteamUGC()->StartItemUpdate( SteamUtils()->GetAppID(), m_nFile );
		g_ReactiveDropWorkshop.SetWorkshopKeyValues( hUpdate );
		SteamAPICall_t hCall = SteamUGC()->SubmitItemUpdate( hUpdate, "[automated update to fix display names on leaderboards]" );
		m_SubmitCall.Set( hCall, this, &CFixWorkshopKeyValueNames::SubmitCompleted );
	}

	void SubmitCompleted( SubmitItemUpdateResult_t *pResult, bool bIOFailure )
	{
		if ( bIOFailure )
		{
			Warning( "Failed to apply fix: IO failure for %llu\n", m_nFile );
			delete this;
			return;
		}

		if ( pResult->m_eResult != k_EResultOK )
		{
			Warning( "Failed to apply fix: Submitting update failed for %llu with EResult %d (%s)\n", m_nFile, pResult->m_eResult, UTIL_RD_EResultToString( pResult->m_eResult ) );
			delete this;
			return;
		}

		Msg( "Submitted display name fix for addon %llu\n", m_nFile );
		delete this;
	}

	STEAM_CALLBACK( CFixWorkshopKeyValueNames, Steam_DownloadItemResult, DownloadItemResult_t );
	CCallResult<CFixWorkshopKeyValueNames, SubmitItemUpdateResult_t> m_SubmitCall;

	PublishedFileId_t m_nFile;
};

void CFixWorkshopKeyValueNames::Steam_DownloadItemResult( DownloadItemResult_t *pResult )
{
	if ( pResult->m_unAppID != SteamUtils()->GetAppID() || pResult->m_nPublishedFileId != m_nFile )
	{
		return;
	}

	if ( pResult->m_eResult != k_EResultOK )
	{
		Warning( "Failed to apply fix: Downloading addon %llu failed with EResult %d (%s)\n", m_nFile, pResult->m_eResult, UTIL_RD_EResultToString( pResult->m_eResult ) );
		delete this;
		return;
	}

	PerformFix();
}

void CReactiveDropWorkshop::CheckPublishedAddonConsistency()
{
	FOR_EACH_VEC( m_EnabledAddons, i )
	{
		if ( SteamUser()->GetSteamID() != m_EnabledAddons[i].details.m_ulSteamIDOwner )
		{
			continue;
		}
		
		bool bWant = m_EnabledAddons[i].kvTags.Defined( "campaigns" ) || m_EnabledAddons[i].kvTags.Defined( "missions" ) || m_EnabledAddons[i].kvTags.Defined( "challenges" );
		bool bHave = m_EnabledAddons[i].kvTags.Defined( "campaign_name" ) || m_EnabledAddons[i].kvTags.Defined( "mission_name" ) || m_EnabledAddons[i].kvTags.Defined( "challenge_name" );
		if ( bWant && bHave )
		{
			bool bAllInvalid = true;
			UtlSymId_t iCampaignName = m_EnabledAddons[i].kvTags.Find( "campaign_name" );
			UtlSymId_t iMissionName = m_EnabledAddons[i].kvTags.Find( "mission_name" );
			UtlSymId_t iChallengeName = m_EnabledAddons[i].kvTags.Find( "challenge_name" );
			if ( iCampaignName != m_EnabledAddons[i].kvTags.InvalidIndex() )
			{
				FOR_EACH_VEC( m_EnabledAddons[i].kvTags[iCampaignName], j )
				{
					if ( !V_strstr( m_EnabledAddons[i].kvTags[iCampaignName][j], "/Invalid Campaign" ) )
					{
						bAllInvalid = false;
						break;
					}
				}
			}
			if ( iMissionName != m_EnabledAddons[i].kvTags.InvalidIndex() )
			{
				FOR_EACH_VEC( m_EnabledAddons[i].kvTags[iMissionName], j )
				{
					if ( !V_strstr( m_EnabledAddons[i].kvTags[iMissionName][j], "/Invalid Mission" ) )
					{
						bAllInvalid = false;
						break;
					}
				}
			}
			if ( iChallengeName != m_EnabledAddons[i].kvTags.InvalidIndex() )
			{
				FOR_EACH_VEC( m_EnabledAddons[i].kvTags[iChallengeName], j )
				{
					if ( !V_strstr( m_EnabledAddons[i].kvTags[iChallengeName][j], "/Invalid Challenge" ) )
					{
						bAllInvalid = false;
						break;
					}
				}
			}
			if ( bAllInvalid )
			{
				bHave = false;
			}
		}
		if ( bWant && !bHave )
		{
			new CFixWorkshopKeyValueNames( m_EnabledAddons[i].details.m_nPublishedFileId );
		}
	}
}
#endif

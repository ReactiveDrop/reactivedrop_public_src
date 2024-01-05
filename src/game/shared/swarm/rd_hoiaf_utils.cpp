#include "cbase.h"
#include "rd_hoiaf_utils.h"
#include "filesystem.h"
#include "steam/steam_api.h"

#ifdef CLIENT_DLL
#include <vgui/ISurface.h>
#include "hud_basechat.h"
#include "gameui/swarm/basemodpanel.h"
#include "gameui/swarm/basemodframe.h"
#include "asw_medal_store.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Note: This system is documented in more detail at <https://developer.reactivedrop.com/iaf-intel.html>.

#ifdef CLIENT_DLL
extern ConVar hud_saytext_time;
#endif

CRD_HoIAF_System::CRD_HoIAF_System() :
	CAutoGameSystemPerFrame{ IsClientDll() ? "CRD_HoIAF_System (client)" : "CRD_HoIAF_System (server)" },
	m_pIAFIntel{ "II" }
{
}

void CRD_HoIAF_System::PostInit()
{
#ifdef GAME_DLL
	if ( !engine->IsDedicatedServer() )
		return;
#endif

	LoadCachedIAFIntel();
	// always grab new world state just in case we have a corrupt cached version with a ridiculous future expiration timestamp
	RefreshIAFIntel( false, true );
}

// Check data expiration timestamp once per frame.
#ifdef CLIENT_DLL
void CRD_HoIAF_System::Update( float frametime )
#else
void CRD_HoIAF_System::PreClientUpdate()
#endif
{
#ifdef GAME_DLL
	if ( !engine->IsDedicatedServer() )
		return;
#endif

	CheckIAFIntelUpToDate();
}

bool CRD_HoIAF_System::CheckIAFIntelUpToDate()
{
	return RefreshIAFIntel();
}

bool CRD_HoIAF_System::IsRankedServerIP( uint32_t ip ) const
{
	return m_RankedServerIPs.IsValidIndex( m_RankedServerIPs.Find( ip ) );
}

bool CRD_HoIAF_System::GetLastUpdateDate( int &year, int &month, int &day ) const
{
	int date = m_iLatestPatch;
	if ( !date )
	{
		return false;
	}

	day = date % 100;
	Assert( day >= 1 && day <= 31 );
	date /= 100;
	month = date % 100;
	Assert( month >= 1 && month <= 12 );
	date /= 100;
	year = date;
	Assert( year >= 2023 && year < 10000 );

	return true;
}

int CRD_HoIAF_System::CountFeaturedNews() const
{
	return m_FeaturedNews.Count();
}

const char *CRD_HoIAF_System::GetFeaturedNewsCaption( int index ) const
{
	if ( index < 0 || index >= CountFeaturedNews() )
		return NULL;

	return m_FeaturedNews[index]->Caption;
}

const char *CRD_HoIAF_System::GetFeaturedNewsURL( int index ) const
{
	if ( index < 0 || index >= CountFeaturedNews() )
		return NULL;

	return m_FeaturedNews[index]->URL;
}

#ifdef CLIENT_DLL
vgui::HTexture CRD_HoIAF_System::GetFeaturedNewsTexture( int index ) const
{
	if ( index < 0 || index >= CountFeaturedNews() )
		return NULL;

	return m_FeaturedNews[index]->TextureHandle;
}
#endif

int CRD_HoIAF_System::CountEventTimers() const
{
	return m_EventTimers.Count();
}

bool CRD_HoIAF_System::IsEventTimerActive( int index ) const
{
	int64_t iCurrentTime = SteamUtils() ? SteamUtils()->GetServerRealTime() : 0;
	return GetEventStartTime( index ) <= iCurrentTime || GetEventEndTime( index ) >= iCurrentTime;
}

const char *CRD_HoIAF_System::GetEventTimerCaption( int index ) const
{
	if ( index < 0 || index >= CountEventTimers() )
		return NULL;

	return m_EventTimers[index]->Caption;
}

const char *CRD_HoIAF_System::GetEventTimerURL( int index ) const
{
	if ( index < 0 || index >= CountEventTimers() )
		return NULL;

	return m_EventTimers[index]->URL;
}

int64_t CRD_HoIAF_System::GetEventStartTime( int index ) const
{
	if ( index < 0 || index >= CountEventTimers() )
		return NULL;

	return m_EventTimers[index]->Starts;
}

int64_t CRD_HoIAF_System::GetEventEndTime( int index ) const
{
	if ( index < 0 || index >= CountEventTimers() )
		return NULL;

	return m_EventTimers[index]->Ends;
}

#ifdef CLIENT_DLL
void CRD_HoIAF_System::InsertChatMessages( CBaseHudChat *pChat )
{
	int64_t iCurrentTime = SteamUtils() ? SteamUtils()->GetServerRealTime() : 0;
	FOR_EACH_VEC( m_ChatAnnouncements, i )
	{
		if ( m_ChatAnnouncements[i]->NotBefore > iCurrentTime )
			continue;
		if ( m_ChatAnnouncements[i]->NotAfter < iCurrentTime )
			continue;
		if ( m_ChatAnnouncementSeen.Find( m_ChatAnnouncements[i]->ID ).IsValid() )
			continue;

		m_ChatAnnouncementSeen.AddString( m_ChatAnnouncements[i]->ID );

		CBaseHudChatLine *line = pChat->FindUnusedChatLine();
		if ( !line )
			line = pChat->FindUnusedChatLine();
		Assert( line );
		if ( !line )
			continue;

		line->SetText( "" );
		line->SetExpireTime();

		line->SetVisible( false );
		line->SetNameStart( 0 );
		line->SetNameLength( 0 );

		if ( m_ChatAnnouncements[i]->Zbalermorna.IsEmpty() )
		{
			wchar_t wbuf[2048];
			V_UTF8ToUnicode( m_ChatAnnouncements[i]->Text, wbuf, sizeof( wbuf ) );

			if ( line->m_text )
			{
				delete[] line->m_text;
			}
			line->m_text = CloneWString( wbuf );
			line->m_textRanges.RemoveAll();

			TextRange range;
			range.start = 0;
			range.end = V_wcslen( wbuf );
			range.color = m_ChatAnnouncements[i]->Color;
			line->m_textRanges.AddToTail( range );

			line->Colorize();
		}
		else if ( pChat->GetChatHistory() )
		{
			pChat->GetChatHistory()->InsertString( "\n" );
			pChat->GetChatHistory()->InsertColorChange( m_ChatAnnouncements[i]->Color );
			pChat->GetChatHistory()->InsertFontChange( m_ChatAnnouncements[i]->Font );
			pChat->GetChatHistory()->InsertFade( hud_saytext_time.GetFloat(), CHAT_HISTORY_IDLE_FADE_TIME );
			pChat->GetChatHistory()->InsertZbalermornaString( m_ChatAnnouncements[i]->Zbalermorna );
			pChat->GetChatHistory()->InsertFade( -1, -1 );
		}
	}
}
#endif

#ifdef CLIENT_DLL
void CRD_HoIAF_System::MarkBountyAsCompleted( int iBountyID )
{
	FOR_EACH_VEC( m_HoIAFMissionBounties, i )
	{
		if ( m_HoIAFMissionBounties[i]->ID != iBountyID )
		{
			continue;
		}

		Assert( !V_stricmp( m_HoIAFMissionBounties[i]->Map, MapName() ) );
		if ( V_stricmp( m_HoIAFMissionBounties[i]->Map, MapName() ) )
		{
			Warning( "[HoIAF:%c] Server said to mark mission bounty complete for %s, but we are on map %s!\n", IsClientDll() ? 'C' : 'S', m_HoIAFMissionBounties[i]->Map.Get(), MapName() );
			return;
		}

		if ( C_ASW_Medal_Store *pMedalStore = GetMedalStore() )
		{
			pMedalStore->OnCompletedBounty( iBountyID );
		}

		return;
	}

	Warning( "[HoIAF:%c] Server said to mark mission bounty %d complete, but we don't know about that bounty!\n", IsClientDll() ? 'C' : 'S', iBountyID );
}
#endif

void CRD_HoIAF_System::ParseIAFIntel()
{
	m_iExpireAt = int64_t( m_pIAFIntel->GetUint64( "expires" ) );
	m_RankedServerIPs.Purge();
	m_iLatestPatch = m_pIAFIntel->GetInt( "latestPatch" );
	m_FeaturedNews.PurgeAndDeleteElements();
	m_EventTimers.PurgeAndDeleteElements();
	m_ChatAnnouncements.PurgeAndDeleteElements();
	m_HoIAFMissionBounties.PurgeAndDeleteElements();

	FOR_EACH_SUBKEY( m_pIAFIntel, pCommand )
	{
		const char *szName = pCommand->GetName();
		if ( !V_stricmp( szName, "expires" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_UINT64 );
			// already handled above
		}
		else if ( !V_stricmp( szName, "rankedIP" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_INT );
			m_RankedServerIPs.AddToTail( uint32_t( pCommand->GetInt() ) );
		}
		else if ( !V_stricmp( szName, "latestPatch" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_INT );
			// already handled above
		}
		else if ( !V_stricmp( szName, "featuredNews" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_NONE );
			Assert( m_FeaturedNews.Count() < 5 );

			FeaturedNews_t *pNews = new FeaturedNews_t;

			LoadTranslatedString( pNews->Caption, pCommand, "caption_%s" );
			pNews->URL = pCommand->GetString( "url" );

#ifdef CLIENT_DLL
			const char *szTextureURL = pCommand->GetString( "texture_url" );
			pNews->TextureCRC = pCommand->GetInt( "texture_crc" );
			pNews->TextureIndex = pCommand->GetInt( "texture_index" );
			Assert( pNews->TextureIndex >= 1 && pNews->TextureIndex <= 5 );

			char szTextureName[MAX_PATH];
			V_snprintf( szTextureName, sizeof( szTextureName ), "materials/vgui/swarm/news_showcase_%d.vtf", pNews->TextureIndex );

			CUtlBuffer buf;
			if ( g_pFullFileSystem->ReadFile( szTextureName, "GAME", buf ) && CRC32_ProcessSingleBuffer( buf.Base(), buf.TellPut() ) == pNews->TextureCRC )
			{
				pNews->TextureHandle = vgui::surface()->CreateNewTextureID();
				vgui::surface()->DrawSetTextureFile( pNews->TextureHandle, &szTextureName[strlen( "materials/" )], 1, false );
			}
			else if ( ISteamHTTP *pHTTP = SteamHTTP() )
			{
				pNews->m_hTextureRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodGET, szTextureURL );
				pHTTP->SetHTTPRequestUserAgentInfo( pNews->m_hTextureRequest, "Reactive Drop News Showcase Icon Fetch" );

				SteamAPICall_t hCall = k_uAPICallInvalid;
				if ( pHTTP->SendHTTPRequest( pNews->m_hTextureRequest, &hCall ) )
				{
					pNews->m_TextureRequestFinished.Set( hCall, pNews, &CRD_HoIAF_System::FeaturedNews_t::OnHTTPRequestCompleted );
				}
			}
#endif

			m_FeaturedNews.AddToTail( pNews );
		}
		else if ( !V_stricmp( szName, "eventTimer" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_NONE );
			Assert( m_EventTimers.Count() < 3 );

			EventTimer_t *pTimer = new EventTimer_t;

			LoadTranslatedString( pTimer->Caption, pCommand, "caption_%s" );
			pTimer->URL = pCommand->GetString( "url" );
			pTimer->Starts = pCommand->GetUint64( "starts" );
			pTimer->Ends = pCommand->GetUint64( "ends" );

			m_EventTimers.AddToTail( pTimer );
		}
		else if ( !V_stricmp( szName, "chatAnnouncement" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_NONE );

			ChatAnnouncement_t *pAnnouncement = new ChatAnnouncement_t;

			pAnnouncement->ID = pCommand->GetString( "id" );
			LoadTranslatedString( pAnnouncement->Text, pCommand, "text_%s" );
			pAnnouncement->Zbalermorna = pCommand->GetString( "text_zbalermorna" );
			pAnnouncement->NotBefore = pCommand->GetUint64( "not_before" );
			pAnnouncement->NotAfter = pCommand->GetUint64( "not_after" );
			pAnnouncement->Color = pCommand->GetColor( "color", Color{ 255, 0, 0, 255 } );
#ifdef CLIENT_DLL
			vgui::HScheme hScheme = vgui::scheme()->LoadSchemeFromFileEx( NULL, "resource/ChatScheme.res", "ChatScheme" );
			vgui::IScheme *pScheme = vgui::scheme()->GetIScheme( hScheme );
			pAnnouncement->Font = pScheme->GetFont( pCommand->GetString( "font", "ChatFont" ), false );
#endif

			m_ChatAnnouncements.AddToTail( pAnnouncement );
		}
		else if ( !V_stricmp( szName, "hoiafMissionBounty" ) )
		{
			Assert( pCommand->GetDataType() == KeyValues::TYPE_NONE );

			HoIAFMissionBounty_t *pBounty = new HoIAFMissionBounty_t;

			pBounty->ID = pCommand->GetInt( "id" );
			pBounty->Starts = pCommand->GetUint64( "starts" );
			pBounty->Ends = pCommand->GetUint64( "ends" );
			pBounty->Map = pCommand->GetString( "map" );
			pBounty->Points = pCommand->GetInt( "points" );

			m_HoIAFMissionBounties.AddToTail( pBounty );
		}
		else
		{
			AssertMsg1( false, "Unhandled IAF Intel command %s", szName );
		}
	}

#ifdef CLIENT_DLL
	BaseModUI::CBaseModFrame *pMainMenu = BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_MAINMENU );
	if ( pMainMenu && pMainMenu->IsVisible() )
	{
		pMainMenu->Activate();
	}
#endif
}

void CRD_HoIAF_System::LoadCachedIAFIntel()
{
	Assert( g_pFullFileSystem );
	if ( !g_pFullFileSystem )
		return;

	CUtlBuffer buf;
	if ( g_pFullFileSystem->ReadFile( "cfg/iicache.dat", "MOD", buf ) )
	{
		m_pIAFIntel->Clear();
		if ( !m_pIAFIntel->ReadAsBinary( buf ) )
		{
			DevMsg( "[HoIAF:%c] Failed to load cached IAF Intel. Continuing regardless.\n", IsClientDll() ? 'C' : 'S' );
		}
	}
	else
	{
		DevMsg( "[HoIAF:%c] No cached IAF Intel found. This is a fresh install or somebody deleted it.\n", IsClientDll() ? 'C' : 'S' );
	}

	ParseIAFIntel();
}

bool CRD_HoIAF_System::RefreshIAFIntel( bool bOnlyIfExpired, bool bForceNewRequest )
{
#ifdef CLIENT_DLL
	ISteamUtils *pUtils = SteamUtils();
	ISteamHTTP *pHTTP = SteamHTTP();
#else
	ISteamUtils *pUtils = engine->IsDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	ISteamHTTP *pHTTP = engine->IsDedicatedServer() ? SteamGameServerHTTP() : SteamHTTP();
#endif
	Assert( pUtils );
	Assert( pHTTP );
	if ( !pUtils || !pHTTP )
	{
		return false;
	}

	int64_t iNow = pUtils->GetServerRealTime();
	if ( bOnlyIfExpired && iNow < m_iExpireAt )
	{
		return true;
	}

	// Call PrioritizeHTTPRequest again to check if the request still exists and didn't magically disappear.
	if ( !bForceNewRequest && m_hIAFIntelRefreshRequest != INVALID_HTTPREQUEST_HANDLE && pHTTP->PrioritizeHTTPRequest( m_hIAFIntelRefreshRequest ) )
	{
		return false;
	}

	if ( m_iBackoffUntil > iNow )
	{
		// We need updated data, but we just failed to request it. Wait until a little later to retry again.
		return false;
	}

	DevMsg( "[HoIAF:%c] Requesting updated IAF Intel; previous intel expired %lld seconds ago.%s\n", IsClientDll() ? 'C' : 'S', iNow - m_iExpireAt, bOnlyIfExpired ? "" : " (forced)" );

	if ( m_hIAFIntelRefreshRequest != INVALID_HTTPREQUEST_HANDLE )
	{
		pHTTP->ReleaseHTTPRequest( m_hIAFIntelRefreshRequest );
	}

	m_hIAFIntelRefreshRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodGET, "https://stats.reactivedrop.com/game_dynamic_state.bin" );
	if ( m_hIAFIntelRefreshRequest == INVALID_HTTPREQUEST_HANDLE )
	{
		Warning( "[HoIAF:%c] Game global state update request: CreateHTTPRequest failed!\n", IsClientDll() ? 'C' : 'S' );
		return false;
	}

	pHTTP->SetHTTPRequestUserAgentInfo( m_hIAFIntelRefreshRequest, IsClientDll() ? "Reactive Drop Client" : "Reactive Drop Server" );
	SteamAPICall_t hCall = k_uAPICallInvalid;
	if ( !pHTTP->SendHTTPRequest( m_hIAFIntelRefreshRequest, &hCall ) )
	{
		Warning( "[HoIAF:%c] Game global state update request: SendHTTPRequest failed!\n", IsClientDll() ? 'C' : 'S' );
		return false;
	}

	// We want this before any of the optional things we might be downloading.
	pHTTP->PrioritizeHTTPRequest( m_hIAFIntelRefreshRequest );

	m_IAFIntelRefresh.Set( hCall, this, &CRD_HoIAF_System::OnHTTPRequestCompleted );

	return false;
}

void CRD_HoIAF_System::OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
{
	if ( bIOFailure )
	{
		Warning( "[HoIAF:%c] Game global state update request: Lost connection to Steam!\n", IsClientDll() ? 'C' : 'S' );
		OnRequestFailed();
		m_hIAFIntelRefreshRequest = INVALID_HTTPREQUEST_HANDLE;
		return;
	}

	Assert( pParam && pParam->m_hRequest == m_hIAFIntelRefreshRequest );

#ifdef CLIENT_DLL
	ISteamUtils *pUtils = SteamUtils();
	ISteamHTTP *pHTTP = SteamHTTP();
#else
	ISteamUtils *pUtils = engine->IsDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
	ISteamHTTP *pHTTP = engine->IsDedicatedServer() ? SteamGameServerHTTP() : SteamHTTP();
#endif
	Assert( pUtils );
	Assert( pHTTP );
	if ( !pHTTP )
	{
		Warning( "[HoIAF:%c] Game global state update request: Somehow lost ISteamHTTP interface!\n", IsClientDll() ? 'C' : 'S' );
		OnRequestFailed();
		// can't release request or get body, so just give up
		m_hIAFIntelRefreshRequest = INVALID_HTTPREQUEST_HANDLE;
		return;
	}

	if ( !pParam->m_bRequestSuccessful )
	{
		Warning( "[HoIAF:%c] Game global state update request: Network failure!\n", IsClientDll() ? 'C' : 'S' );
		OnRequestFailed();
		goto cleanup;
	}

	{
		CUtlBuffer body{ 0, int( pParam->m_unBodySize ) };
		body.SeekPut( CUtlBuffer::SEEK_HEAD, pParam->m_unBodySize );
		if ( !pHTTP->GetHTTPResponseBodyData( pParam->m_hRequest, ( uint8 * )body.Base(), pParam->m_unBodySize ) )
		{
			Warning( "[HoIAF:%c] Game global state update request: GetHTTPResponseBodyData failed!\n", IsClientDll() ? 'C' : 'S' );
			OnRequestFailed();
			goto cleanup;
		}

		if ( pParam->m_eStatusCode != k_EHTTPStatusCode200OK )
		{
			Warning( "[HoIAF:%c] Game global state update request: Received HTTP status code %d (should be 200!)\n", IsClientDll() ? 'C' : 'S', pParam->m_eStatusCode );
			OnRequestFailed();
			goto cleanup;
		}

		Assert( g_pFullFileSystem );
		if ( g_pFullFileSystem )
		{
			g_pFullFileSystem->WriteFile( "cfg/iicache.dat", "MOD", body );
		}

		KeyValues *pNewData = new KeyValues( "II" );
		if ( !pNewData->ReadAsBinary( body ) )
		{
			Warning( "[HoIAF:%c] Game global state update request: failed to parse global state data\n", IsClientDll() ? 'C' : 'S' );
			OnRequestFailed();
			pNewData->deleteThis();
			goto cleanup;
		}

		m_pIAFIntel->deleteThis();
		m_pIAFIntel.Assign( pNewData );

		ParseIAFIntel();

		int64_t iNow = pUtils ? pUtils->GetServerRealTime() : 0;
		Assert( m_iExpireAt > iNow );
		if ( m_iExpireAt > iNow )
		{
			OnNewIntelReceived();
		}
		else
		{
			OnRequestFailed();
		}

		DevMsg( "[HoIAF:%c] Received updated IAF Intel; expires in %lld seconds.\n", IsClientDll() ? 'C' : 'S', m_iExpireAt - iNow );
	}

cleanup:
	pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
	m_hIAFIntelRefreshRequest = INVALID_HTTPREQUEST_HANDLE;
}

void CRD_HoIAF_System::OnRequestFailed()
{
#ifdef CLIENT_DLL
	ISteamUtils *pUtils = SteamUtils();
#else
	ISteamUtils *pUtils = engine->IsDedicatedServer() ? SteamGameServerUtils() : SteamUtils();
#endif
	Assert( pUtils );

	m_iExponentialBackoff++;
	if ( m_iExponentialBackoff > 12 )
		m_iExponentialBackoff = 12; // cap at just over an hour

	// if we can't get the current time, give up until the game is restarted
	m_iBackoffUntil = pUtils ? ( pUtils->GetServerRealTime() + ( 1 << m_iExponentialBackoff ) ) : ~0u;
}

void CRD_HoIAF_System::OnNewIntelReceived()
{
	m_iBackoffUntil = 0;
	m_iExponentialBackoff = 0;

#ifdef CLIENT_DLL
	C_ASW_Medal_Store *pMedalStore = GetMedalStore();
	if ( pMedalStore )
	{
		CUtlVector<int> activeBounties;
		activeBounties.SetCount( m_HoIAFMissionBounties.Count() );
		FOR_EACH_VEC( m_HoIAFMissionBounties, i )
		{
			activeBounties[i] = m_HoIAFMissionBounties[i]->ID;
		}

		pMedalStore->RemoveBountiesExcept( activeBounties );
	}
#endif
}

#ifdef CLIENT_DLL
void CRD_HoIAF_System::LoadTranslatedString( CUtlString &str, KeyValues *pKV, const char *szTemplate )
{
	char szLocalized[256], szEnglish[256];
	V_snprintf( szLocalized, sizeof( szLocalized ), szTemplate, SteamApps() ? SteamApps()->GetCurrentGameLanguage() : "" );
	V_snprintf( szEnglish, sizeof( szEnglish ), szTemplate, "english" );
	str = pKV->GetString( szLocalized, pKV->GetString( szEnglish ) );
}
#endif

CRD_HoIAF_System::FeaturedNews_t::~FeaturedNews_t()
{
#ifdef CLIENT_DLL
	if ( TextureHandle && vgui::surface() )
	{
		vgui::surface()->DestroyTextureID( TextureHandle );
		TextureHandle = NULL;
	}

	if ( m_hTextureRequest != INVALID_HTTPREQUEST_HANDLE && SteamHTTP() )
	{
		SteamHTTP()->ReleaseHTTPRequest( m_hTextureRequest );
		m_hTextureRequest = INVALID_HTTPREQUEST_HANDLE;
	}
#endif
}

#ifdef CLIENT_DLL
void CRD_HoIAF_System::FeaturedNews_t::OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
{
	if ( bIOFailure )
	{
		Warning( "[HoIAF:%c] Failed to download icon for featured news item with URL %s (IO failure)\n", IsClientDll() ? 'C' : 'S', URL.Get() );
		m_hTextureRequest = INVALID_HTTPREQUEST_HANDLE;
		return;
	}

	Assert( m_hTextureRequest == pParam->m_hRequest );

	ISteamHTTP *pHTTP = SteamHTTP();
	Assert( pHTTP );
	if ( !pHTTP )
	{
		Warning( "[HoIAF:%c] Failed to download icon for featured news item with URL %s (lost ISteamHTTP interface!)\n", IsClientDll() ? 'C' : 'S', URL.Get() );
		// can't release request or get body, so just give up
		m_hTextureRequest = INVALID_HTTPREQUEST_HANDLE;
		return;
	}

	if ( !pParam->m_bRequestSuccessful )
	{
		Warning( "[HoIAF:%c] Failed to download icon for featured news item with URL %s (network failure)\n", IsClientDll() ? 'C' : 'S', URL.Get() );
		goto cleanup;
	}

	{
		CUtlBuffer body{ 0, int( pParam->m_unBodySize ) };
		body.SeekPut( CUtlBuffer::SEEK_HEAD, pParam->m_unBodySize );
		if ( !pHTTP->GetHTTPResponseBodyData( m_hTextureRequest, ( uint8 * )body.Base(), pParam->m_unBodySize ) )
		{
			Warning( "[HoIAF:%c] Failed to download icon for featured news item with URL %s (GetHTTPResponseBodyData failed)\n", IsClientDll() ? 'C' : 'S', URL.Get() );
			goto cleanup;
		}

		Assert( CRC32_ProcessSingleBuffer( body.Base(), body.TellPut() ) == TextureCRC );

		char szTextureName[MAX_PATH];
		V_snprintf( szTextureName, sizeof( szTextureName ), "materials/vgui/swarm/news_showcase_%d.vtf", TextureIndex );

		g_pFullFileSystem->CreateDirHierarchy( "materials/vgui/swarm", "MOD" );
		g_pFullFileSystem->WriteFile( szTextureName, "MOD", body );
		TextureHandle = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( TextureHandle, &szTextureName[strlen( "materials/" )], 1, false );

		BaseModUI::CBaseModFrame *pMainMenu = BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_MAINMENU );
		if ( pMainMenu && pMainMenu->IsVisible() )
		{
			pMainMenu->Activate();
		}
	}

cleanup:
	pHTTP->ReleaseHTTPRequest( m_hTextureRequest );
	m_hTextureRequest = INVALID_HTTPREQUEST_HANDLE;
}
#endif

static CRD_HoIAF_System s_HoIAFSystem;

CRD_HoIAF_System *HoIAF()
{
	return &s_HoIAFSystem;
}

#ifdef CLIENT_DLL
CON_COMMAND_F( rd_hoiaf_mark_bounty_completed, "mark a bounty as completed", FCVAR_HIDDEN | FCVAR_SERVER_CAN_EXECUTE | FCVAR_DONTRECORD )
{
	if ( args.ArgC() != 2 )
		return;

	int iBountyID = V_atoi( args[1] );
	if ( !iBountyID )
		return;

	HoIAF()->MarkBountyAsCompleted( iBountyID );
}
#endif

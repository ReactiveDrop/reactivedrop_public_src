#include "cbase.h"
#include "rd_hoiaf_utils.h"
#include "filesystem.h"
#include "steam/steam_api.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Note: This system is documented in more detail at <https://developer.reactivedrop.com/iaf-intel.html>.

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

void CRD_HoIAF_System::ParseIAFIntel()
{
	m_iExpireAt = int64_t( m_pIAFIntel->GetUint64( "expires" ) );
	m_RankedServerIPs.Purge();

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
		else
		{
			AssertMsg1( false, "Unhandled IAF Intel command %s", szName );
		}
	}

	m_RankedServerIPs.AddToTail();
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
			DevMsg( "[HoIAF:%c] Failed to load cached IAF Intel continuing regardless.\n", IsClientDll() ? 'C' : 'S' );
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
			m_iBackoffUntil = 0;
			m_iExponentialBackoff = 0;
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

static CRD_HoIAF_System s_HoIAFSystem;

CRD_HoIAF_System *HoIAF()
{
	return &s_HoIAFSystem;
}

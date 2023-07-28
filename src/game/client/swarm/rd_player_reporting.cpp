#include "cbase.h"
#include "rd_player_reporting.h"
#include "c_asw_player.h"
#include "inetchannelinfo.h"
#include "asw_gamerules.h"
#include "rd_lobby_utils.h"
#include "rd_workshop.h"
#include "asw_util_shared.h"
#include "vgui/ILocalize.h"
#include "gameui/swarm/basemodpanel.h"
#include "gameui/swarm/vgenericwaitscreen.h"
#include "gameui/swarm/vgenericconfirmation.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

CRD_Player_Reporting g_RD_Player_Reporting;

static void WriteJSONRaw( CUtlBuffer &buf, const char *szRaw )
{
	buf.PutString( szRaw );
}

static void WriteJSONString( CUtlBuffer &buf, const char *szString )
{
	if ( !szString )
	{
		buf.PutString( "null" );
		return;
	}

	int nBufLen = ( V_strlen( szString ) + 1 ) * sizeof( wchar_t );
	wchar_t *wszString = static_cast< wchar_t * >( stackalloc( nBufLen ) );
	V_UTF8ToUnicode( szString, wszString, nBufLen );
	buf.PutChar( '"' );
	for ( const wchar_t *pwsz = wszString; *pwsz; pwsz++ )
	{
		if ( *pwsz == L'\\' || *pwsz == L'"' )
		{
			// JSON string control characters
			buf.PutChar( '\\' );
			buf.PutChar( char( *pwsz ) );
		}
		else if ( *pwsz >= L' ' && *pwsz <= L'~' )
		{
			// ASCII printable region
			buf.PutChar( char( *pwsz ) );
		}
		else
		{
			// unicode!
			char szUnicode[8];
			V_snprintf( szUnicode, sizeof( szUnicode ), "\\u%04x", *pwsz );
			buf.PutString( szUnicode );
		}
	}
	buf.PutChar( '"' );
}

static void WriteJSONFormat( CUtlBuffer &buf, const char *szFormat, ... )
{
	char szBuf[2048];
	va_list arg_ptr;

	va_start( arg_ptr, szFormat );
	V_vsnprintf( szBuf, sizeof( szBuf ), szFormat, arg_ptr );
	va_end( arg_ptr );

	buf.PutString( szBuf );
}

static void WriteJSONHex( CUtlBuffer &buf, const CUtlBuffer &data )
{
	int nLen = data.TellPut() - data.TellGet();
	CUtlMemory<char> szData{ 0, nLen * 2 + 1 };
	V_binarytohex( static_cast< const byte * >( data.PeekGet() ), nLen, szData.Base(), szData.Count() );

	buf.PutChar( '"' );
	buf.PutString( szData.Base() );
	buf.PutChar( '"' );
}

static void WriteServerInfo( CUtlBuffer &buf )
{
	C_AlienSwarm *pAlienSwarm = ASWGameRules();
	Assert( pAlienSwarm );
	INetChannelInfo *pNCI = engine->GetNetChannelInfo();
	Assert( pNCI );

	// current map and challenge
	extern ConVar rd_challenge;
	char szChallengeFileName[MAX_PATH];
	V_snprintf( szChallengeFileName, sizeof( szChallengeFileName ), "resource/challenges/%s.txt", rd_challenge.GetString() );
	PublishedFileId_t nChallengeFileID = g_ReactiveDropWorkshop.FindAddonProvidingFile( szChallengeFileName );
	if ( UTIL_RD_GetCurrentLobbyID().IsValid() )
	{
		const char *pszChallengeFileID = UTIL_RD_GetCurrentLobbyData( "game:challengeinfo:workshop", "" );
		if ( *pszChallengeFileID )
		{
			nChallengeFileID = std::strtoull( pszChallengeFileID, NULL, 16 );
		}
	}

	WriteJSONString( buf, "map" );
	WriteJSONRaw( buf, ":{" );
	WriteJSONString( buf, "name" );
	WriteJSONRaw( buf, ":" );
	WriteJSONString( buf, engine->GetLevelNameShort() );
	WriteJSONRaw( buf, "," );
	WriteJSONString( buf, "challenge" );
	WriteJSONRaw( buf, ":" );
	WriteJSONString( buf, rd_challenge.GetString() );
	if ( pAlienSwarm && pAlienSwarm->m_iMissionWorkshopID != 0 && pAlienSwarm->m_iMissionWorkshopID != k_PublishedFileIdInvalid )
	{
		WriteJSONRaw( buf, "," );
		WriteJSONString( buf, "map_workshop" );
		WriteJSONFormat( buf, ":\"%llu\"", pAlienSwarm->m_iMissionWorkshopID );
	}
	if ( nChallengeFileID != 0 && nChallengeFileID != k_PublishedFileIdInvalid )
	{
		WriteJSONRaw( buf, "," );
		WriteJSONString( buf, "challenge_workshop" );
		WriteJSONFormat( buf, ":\"%llu\"", nChallengeFileID );
	}
	WriteJSONRaw( buf, "}," );

	// current server IP and/or lobby id
	WriteJSONString( buf, "server" );
	WriteJSONRaw( buf, ":{" );
	if ( pNCI )
	{
		WriteJSONString( buf, "ip" );
		WriteJSONRaw( buf, ":" );
		WriteJSONString( buf, pNCI->GetAddress() );
		WriteJSONRaw( buf, "," );
	}
	WriteJSONString( buf, "lobby" );
	WriteJSONFormat( buf, ":\"%llu\"", UTIL_RD_GetCurrentLobbyID().ConvertToUint64() );
	WriteJSONRaw( buf, "}," );

	// any other players connected to this host
	WriteJSONString( buf, "witnesses" );
	WriteJSONRaw( buf, ":[" );
	bool bFirst = true;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		C_ASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
		if ( pPlayer )
		{
			if ( bFirst )
				bFirst = false;
			else
				WriteJSONRaw( buf, "," );
			WriteJSONFormat( buf, "\"%llu\"", pPlayer->GetSteamID().ConvertToUint64() );
		}
	}
	WriteJSONRaw( buf, "]," );

	if ( pNCI )
	{
		// some network and latency info
		float flServerFrameTime, flServerFrameTimeStdDev;
		pNCI->GetRemoteFramerate( &flServerFrameTime, &flServerFrameTimeStdDev );

		WriteJSONString( buf, "diagnostics" );
		WriteJSONRaw( buf, ":{" );
		WriteJSONString( buf, "client" );
		WriteJSONRaw( buf, ":{" );
		WriteJSONString( buf, "gametime" );
		WriteJSONFormat( buf, ":%f,", gpGlobals->curtime );
		WriteJSONString( buf, "latency" );
		WriteJSONFormat( buf, ":%f,", pNCI->GetAvgLatency( FLOW_OUTGOING ) );
		WriteJSONString( buf, "choke" );
		WriteJSONFormat( buf, ":%f,", pNCI->GetAvgChoke( FLOW_OUTGOING ) );
		WriteJSONString( buf, "loss" );
		WriteJSONFormat( buf, ":%f,", pNCI->GetAvgLoss( FLOW_OUTGOING ) );
		WriteJSONString( buf, "packets" );
		WriteJSONFormat( buf, ":%f,", pNCI->GetAvgPackets( FLOW_OUTGOING ) );
		WriteJSONString( buf, "frametime" );
		WriteJSONFormat( buf, ":%f},", gpGlobals->frametime );
		WriteJSONString( buf, "server" );
		WriteJSONRaw( buf, ":{" );
		WriteJSONString( buf, "latency" );
		WriteJSONFormat( buf, ":%f,", pNCI->GetAvgLatency( FLOW_INCOMING ) );
		WriteJSONString( buf, "choke" );
		WriteJSONFormat( buf, ":%f,", pNCI->GetAvgChoke( FLOW_INCOMING ) );
		WriteJSONString( buf, "loss" );
		WriteJSONFormat( buf, ":%f,", pNCI->GetAvgLoss( FLOW_INCOMING ) );
		WriteJSONString( buf, "packets" );
		WriteJSONFormat( buf, ":%f,", pNCI->GetAvgPackets( FLOW_INCOMING ) );
		WriteJSONString( buf, "frametime" );
		WriteJSONFormat( buf, ":%f}},", flServerFrameTime );
	}
}

bool CRD_Player_Reporting::IsInProgress()
{
	if ( m_hTicket != k_HAuthTicketInvalid )
		return true;

	if ( m_HTTPRequestCompleted.IsActive() )
		return true;

	return false;
}

void CRD_Player_Reporting::UpdateServerInfo( bool bForce )
{
	if ( !bForce && !engine->IsInGame() )
		return;

	m_ServerInfoCache.Purge();
	WriteServerInfo( m_ServerInfoCache );
}

bool CRD_Player_Reporting::PrepareReportForSend( const char *szCategory, const char *szDescription, CSteamID reportedPlayer, CUtlVector<CUtlBuffer> &screenshots, bool bShowWaitScreen )
{
	Assert( !IsInProgress() );

	// can't queue multiple reports simultaneously. UI doesn't support this, so don't write an error message.
	if ( IsInProgress() )
		return false;

	ISteamUser *pSteamUser = SteamUser();
	Assert( pSteamUser );
	Assert( SteamHTTP() );

	m_bShowWaitScreen = bShowWaitScreen;

	// we need to be connected to a game and also Steam in order to send a player report.
	if ( !pSteamUser || !SteamHTTP() )
	{
		TryLocalize( "#rd_reporting_error_not_connected", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		ShowWaitScreen();

		return false;
	}

	m_hTicket = pSteamUser->GetAuthTicketForWebApi( "playerreport" );
	TryLocalize( "#rd_reporting_wait_requesting_ticket", m_wszLastMessage, sizeof( m_wszLastMessage ) );
	ShowWaitScreen();

	m_Buffer.Purge();

	// basic info
	WriteJSONRaw( m_Buffer, "{" );
	WriteJSONString( m_Buffer, "category" );
	WriteJSONRaw( m_Buffer, ":" );
	WriteJSONString( m_Buffer, szCategory );
	WriteJSONRaw( m_Buffer, "," );
	WriteJSONString( m_Buffer, "player" );
	WriteJSONRaw( m_Buffer, ":{" );
	if ( reportedPlayer.IsValid() )
	{
		WriteJSONString( m_Buffer, "steam64" );
		WriteJSONFormat( m_Buffer, ":\"%llu\"", reportedPlayer.ConvertToUint64() );
	}
	WriteJSONRaw( m_Buffer, "}," );
	WriteJSONString( m_Buffer, "description" );
	WriteJSONRaw( m_Buffer, ":" );
	WriteJSONString( m_Buffer, szDescription );
	WriteJSONRaw( m_Buffer, "," );

	m_Buffer.Put( m_ServerInfoCache.Base(), m_ServerInfoCache.TellPut() );

	// attach any screenshots
	WriteJSONString( m_Buffer, "screenshots" );
	WriteJSONRaw( m_Buffer, ":[" );
	bool bFirst = true;
	FOR_EACH_VEC( screenshots, i )
	{
		if ( bFirst )
			bFirst = false;
		else
			WriteJSONRaw( m_Buffer, "," );
		WriteJSONRaw( m_Buffer, "{" );
		WriteJSONString( m_Buffer, "filename" );
		WriteJSONFormat( m_Buffer, ":\"screenshot%02d.jpeg\",", i );
		WriteJSONString( m_Buffer, "mimetype" );
		WriteJSONRaw( m_Buffer, ":" );
		WriteJSONString( m_Buffer, "image/jpeg" );
		WriteJSONRaw( m_Buffer, "," );
		WriteJSONString( m_Buffer, "data" );
		WriteJSONRaw( m_Buffer, ":" );
		WriteJSONHex( m_Buffer, screenshots[i] );
		WriteJSONRaw( m_Buffer, "}" );
	}
	WriteJSONRaw( m_Buffer, "]," );

	WriteJSONString( m_Buffer, "ticket" );
	WriteJSONRaw( m_Buffer, ":" );

	// processing will continue in the OnGetTicketForWebApiResponse callback

	return true;
}

void CRD_Player_Reporting::OnGetTicketForWebApiResponse( GetTicketForWebApiResponse_t *pParam )
{
	// we only care if it's *our* ticket
	if ( pParam->m_hAuthTicket != m_hTicket )
		return;

	// since this is our ticket, we don't need to wait for one anymore.
	m_hTicket = k_HAuthTicketInvalid;

	if ( pParam->m_eResult == k_EResultOK )
	{
		// we got a ticket! finish writing the report and send it off to the reporting server.
		WriteJSONHex( m_Buffer, CUtlBuffer{ pParam->m_rgubTicket, pParam->m_cubTicket, CUtlBuffer::READ_ONLY } );
		WriteJSONRaw( m_Buffer, "}" );

		ISteamHTTP *pSteamHTTP = SteamHTTP();
		Assert( pSteamHTTP );

		HTTPRequestHandle hRequest = pSteamHTTP->CreateHTTPRequest( k_EHTTPMethodPOST, "https://reporting.reactivedrop.com/playerreport" );
		pSteamHTTP->SetHTTPRequestUserAgentInfo( hRequest, "RDPlayerReporting" );
		pSteamHTTP->SetHTTPRequestRawPostBody( hRequest, "application/json", static_cast< uint8 * >( m_Buffer.Base() ), m_Buffer.TellPut() );

		m_Buffer.Purge();

		SteamAPICall_t hCall = k_uAPICallInvalid;
		if ( !pSteamHTTP->SendHTTPRequest( hRequest, &hCall ) )
		{
			// if we didn't get a valid request handle somehow, we'll end up here.
			TryLocalize( "#rd_reporting_error_sending_unknown", m_wszLastMessage, sizeof( m_wszLastMessage ) );
			ShowWaitScreen();
			return;
		}

		TryLocalize( "#rd_reporting_sending_report_to_server", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		m_HTTPRequestCompleted.Set( hCall, this, &CRD_Player_Reporting::OnHTTPRequestCompleted );
		ShowWaitScreen();
	}
	else
	{
		m_Buffer.Purge();

		// we didn't get a ticket! aw, man! tell the player something broke.
		wchar_t wszEResultID[16];
		wchar_t wszEResultName[128];
		V_snwprintf( wszEResultID, NELEMS( wszEResultID ), L"%d", pParam->m_eResult );
		V_UTF8ToUnicode( UTIL_RD_EResultToString( pParam->m_eResult ), wszEResultName, sizeof( wszEResultName ) );
		g_pVGuiLocalize->ConstructString( m_wszLastMessage, sizeof( m_wszLastMessage ), g_pVGuiLocalize->Find( "#rd_reporting_error_getting_ticket" ), 2, wszEResultID, wszEResultName );
		ShowWaitScreen();
	}
}

void CRD_Player_Reporting::OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
{
	if ( bIOFailure )
	{
		// entirely lost connection to the Steam client.
		TryLocalize( "#rd_reporting_error_sending_client", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		ShowWaitScreen();
		return;
	}

	if ( !pParam->m_bRequestSuccessful )
	{
		SteamHTTP()->ReleaseHTTPRequest( pParam->m_hRequest );
		TryLocalize( "#rd_reporting_error_sending_network", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		ShowWaitScreen();
		return;
	}

	CUtlMemory<char> body{ 0, int( pParam->m_unBodySize ) + 1 };
	SteamHTTP()->GetHTTPResponseBodyData( pParam->m_hRequest, reinterpret_cast< uint8 * >( body.Base() ), body.Count() - 1 );
	body[pParam->m_unBodySize] = '\0';
	SteamHTTP()->ReleaseHTTPRequest( pParam->m_hRequest );

	Assert( pParam->m_eStatusCode == k_EHTTPStatusCode200OK );
	( void )body; // potentially useful for debugging, not currently shown to user.

	if ( pParam->m_eStatusCode == k_EHTTPStatusCode200OK )
	{
		// report queued for processing
		TryLocalize( "#rd_reporting_success", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		ShowWaitScreen();
		return;
	}

	wchar_t wszHTTPStatus[8];
	V_snwprintf( wszHTTPStatus, NELEMS( wszHTTPStatus ), L"%03d", pParam->m_eStatusCode );
	g_pVGuiLocalize->ConstructString( m_wszLastMessage, sizeof( m_wszLastMessage ), g_pVGuiLocalize->Find( "#rd_reporting_error_http" ), 1, wszHTTPStatus );
	ShowWaitScreen();
}

void CRD_Player_Reporting::ShowWaitScreen()
{
	char szMessageUTF8[2048];
	V_UnicodeToUTF8( m_wszLastMessage, szMessageUTF8, sizeof( szMessageUTF8 ) );
	Msg( "[Player Reporting] %s\n", szMessageUTF8 );

	if ( !m_bShowWaitScreen )
		return;

	CBaseModFrame *pCurrentWindow = CBaseModPanel::GetSingleton().GetWindow( CBaseModPanel::GetSingleton().GetActiveWindowType() );

	if ( IsInProgress() )
	{
		GenericWaitScreen *pWaitScreen = assert_cast< GenericWaitScreen * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICWAITSCREEN, pCurrentWindow, false ) );
		if ( pWaitScreen )
		{
			// we will dismiss the wait message ourself; don't dismiss the message on a timer.
			pWaitScreen->ClearData();
			pWaitScreen->AddMessageText( m_wszLastMessage, INFINITY );
		}
	}
	else
	{
		GenericWaitScreen *pWaitScreen = assert_cast< GenericWaitScreen * >( CBaseModPanel::GetSingleton().GetWindow( WT_GENERICWAITSCREEN ) );
		if ( pWaitScreen )
			pWaitScreen->Close();

		GenericConfirmation *pConfirm = assert_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, pCurrentWindow, false ) );
		if ( pConfirm )
		{
			GenericConfirmation::Data_t data;
			data.pWindowTitle = "#rd_reporting_finished_title";
			data.pMessageTextW = m_wszLastMessage;
			data.bOkButtonEnabled = true;
			pConfirm->SetUsageData( data );
		}

		m_bShowWaitScreen = false;
	}
}

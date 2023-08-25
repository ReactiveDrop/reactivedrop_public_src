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
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

// number of seconds a report can be filed after someone leaves the server
#define REPORTING_SNAPSHOT_LIFETIME 300.0f
// number of seconds a quick (descriptionless) report is considered to be "still active"
#define REPORTING_QUICK_TIMEOUT 604800

// CURRENTLY-DEFINED REPORT CATEGORIES
//
// my_account - for self-reported problems about items or achievements or whatever
//
// player_cheating - reporting a player for cheating
// player_abusive_gameplay - reporting a player for griefing
// player_abusive_communication - repoting a player for being abusive in text or voice chat or with another in-game communication tool
// player_commend - reporting a player for doing something good
//
// server_technical - reporting a technical problem with a server
// server_abuse - reporting an abusive server
// server_other - reporting a server-specific issue that does not fit into the above catgories
//
// game_bug - reporting a bug in the game
// other - any non-quick report that does not fit into the above categories
//
// Quick reports have no description and will tell the player they already filed one
// if it's within a week of their last report on the player in the given category.
//
// quick_commend_friendly - reporting a player for being friendly
// quick_commend_leader - reporting a player for being a good leader
// quick_commend_teacher - reporting a player for being a good teacher
//
// quick_abusive_cheating - reporting a player for cheating
// quick_abusive_gameplay - reporting a player for griefing
// quick_abusive_communication - reporting a player for abusive text or voice chat or other communication tool abuse
//
// quick_auto_votekick - sent automatically when voting to kick a player on the scoreboard
// quick_auto_voteleader - sent automatically when voting to make a player the leader on the scoreboard
// quick_auto_mute - sent automatically when muting a player on the scoreboard

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

struct ReportingServerSnapshot_t
{
	// snapshots expire after 5 minutes (counted from when the snapshot was taken to the start of the reporting process)
	float RecordedAt;

	// diagnostic data about game state
	CUtlString MissionName;
	CUtlString ChallengeName;
	PublishedFileId_t MissionWorkshop{ k_PublishedFileIdInvalid };
	PublishedFileId_t ChallengeWorkshop{ k_PublishedFileIdInvalid };

	// diagnostic data about the lobby
	CUtlString ServerIP;
	CSteamID LobbyID;

	// players who were online at the time of this snapshot
	CUtlVector<CSteamID> Witnesses;

	// diagnostic data about connection quality
	bool HaveConnectionQuality{ false };
	float CurTime{};
	float AvgLatency[2]{};
	float AvgChoke[2]{};
	float AvgLoss[2]{};
	float AvgPackets[2]{};
	float FrameTime[2]{};

	void WriteJSON( CUtlBuffer &buf ) const
	{
		WriteJSONString( buf, "map" );
		WriteJSONRaw( buf, ":{" );
		WriteJSONString( buf, "name" );
		WriteJSONRaw( buf, ":" );
		WriteJSONString( buf, MissionName );
		WriteJSONRaw( buf, "," );
		WriteJSONString( buf, "challenge" );
		WriteJSONRaw( buf, ":" );
		WriteJSONString( buf, ChallengeName );
		if ( MissionWorkshop != k_PublishedFileIdInvalid )
		{
			WriteJSONRaw( buf, "," );
			WriteJSONString( buf, "map_workshop" );
			WriteJSONFormat( buf, ":\"%llu\"", MissionWorkshop );
		}
		if ( ChallengeWorkshop != k_PublishedFileIdInvalid )
		{
			WriteJSONRaw( buf, "," );
			WriteJSONString( buf, "challenge_workshop" );
			WriteJSONFormat( buf, ":\"%llu\"", ChallengeWorkshop );
		}
		WriteJSONRaw( buf, "}," );

		// current server IP and/or lobby id
		WriteJSONString( buf, "server" );
		WriteJSONRaw( buf, ":{" );
		if ( !ServerIP.IsEmpty() )
		{
			WriteJSONString( buf, "ip" );
			WriteJSONRaw( buf, ":" );
			WriteJSONString( buf, ServerIP );
			WriteJSONRaw( buf, "," );
		}
		WriteJSONString( buf, "lobby" );
		WriteJSONFormat( buf, ":\"%llu\"", LobbyID.ConvertToUint64() );
		WriteJSONRaw( buf, "}," );

		WriteJSONString( buf, "witnesses" );
		WriteJSONRaw( buf, ":[" );
		FOR_EACH_VEC( Witnesses, i )
		{
			if ( i != 0 )
				WriteJSONRaw( buf, "," );

			WriteJSONFormat( buf, "\"%llu\"", Witnesses[i].ConvertToUint64() );
		}
		WriteJSONRaw( buf, "]," );

		if ( HaveConnectionQuality )
		{
			WriteJSONString( buf, "diagnostics" );
			WriteJSONRaw( buf, ":{" );
			WriteJSONString( buf, "client" );
			WriteJSONRaw( buf, ":{" );
			WriteJSONString( buf, "gametime" );
			WriteJSONFormat( buf, ":%f,", CurTime );
			WriteJSONString( buf, "latency" );
			WriteJSONFormat( buf, ":%f,", AvgLatency[FLOW_OUTGOING] );
			WriteJSONString( buf, "choke" );
			WriteJSONFormat( buf, ":%f,", AvgChoke[FLOW_OUTGOING] );
			WriteJSONString( buf, "loss" );
			WriteJSONFormat( buf, ":%f,", AvgLoss[FLOW_OUTGOING] );
			WriteJSONString( buf, "packets" );
			WriteJSONFormat( buf, ":%f,", AvgPackets[FLOW_OUTGOING] );
			WriteJSONString( buf, "frametime" );
			WriteJSONFormat( buf, ":%f},", FrameTime[FLOW_OUTGOING] );
			WriteJSONString( buf, "server" );
			WriteJSONRaw( buf, ":{" );
			WriteJSONString( buf, "latency" );
			WriteJSONFormat( buf, ":%f,", AvgLatency[FLOW_INCOMING] );
			WriteJSONString( buf, "choke" );
			WriteJSONFormat( buf, ":%f,", AvgChoke[FLOW_INCOMING] );
			WriteJSONString( buf, "loss" );
			WriteJSONFormat( buf, ":%f,", AvgLoss[FLOW_INCOMING] );
			WriteJSONString( buf, "packets" );
			WriteJSONFormat( buf, ":%f,", AvgPackets[FLOW_INCOMING] );
			WriteJSONString( buf, "frametime" );
			WriteJSONFormat( buf, ":%f}},", FrameTime[FLOW_INCOMING] );
		}
	}
};

bool CRD_Player_Reporting::IsInProgress() const
{
	if ( m_hTicket != k_HAuthTicketInvalid )
		return true;

	if ( m_HTTPRequestCompleted.IsActive() )
		return true;

	return false;
}

bool CRD_Player_Reporting::HasRecentServer() const
{
	if ( m_RecentData.Count() == 0 )
		return false;

	// latest snapshot is always at the end
	return m_RecentData[m_RecentData.Count() - 1]->RecordedAt > Plat_FloatTime() - REPORTING_SNAPSHOT_LIFETIME;
}

bool CRD_Player_Reporting::RecentlyReportedPlayer( const char *szCategory, CSteamID player )
{
	if ( m_pRecentReports == NULL )
	{
		if ( !SteamUser() )
			return false;

		CFmtStr szFileName{ "cfg/clientrqpr_%llu.dat", SteamUser()->GetSteamID().ConvertToUint64() };
		CUtlBuffer buf;

		m_pRecentReports.Assign( new KeyValues{ "RQPR" } );
		if ( g_pFullFileSystem->ReadFile( szFileName, "MOD", buf ) )
		{
			m_pRecentReports->ReadAsBinary( buf );
		}
	}

	CFmtStr szKey{ "%s/%llu", szCategory, player.ConvertToUint64() };
	return SteamUtils() && uint64_t( SteamUtils()->GetServerRealTime() ) - REPORTING_QUICK_TIMEOUT < m_pRecentReports->GetUint64( szKey );
}

void CRD_Player_Reporting::GetRecentlyPlayedWith( CUtlVector<CSteamID> &players ) const
{
	CSteamID self = SteamUser() ? SteamUser()->GetSteamID() : k_steamIDNil;
	float flExpireTime = Plat_FloatTime() - REPORTING_SNAPSHOT_LIFETIME;

	FOR_EACH_VEC_BACK( m_RecentData, i )
	{
		if ( m_RecentData[i]->RecordedAt < flExpireTime )
			break;

		FOR_EACH_VEC_BACK( m_RecentData[i]->Witnesses, j )
		{
			CSteamID player = m_RecentData[i]->Witnesses[j];
			if ( player == self )
				continue;

			if ( !players.IsValidIndex( players.Find( player ) ) )
			{
				players.AddToTail( player );
			}
		}
	}
}

void CRD_Player_Reporting::UpdateServerInfo()
{
	if ( !engine->IsConnected() )
		return;

	DeleteExpiredSnapshots();

	ReportingServerSnapshot_t *pSnapshot = NewSnapshot();

	// if the previous snapshot was the same server with the same players in the same order, delete it.
	if ( m_RecentData.Count() != 0 && m_RecentData[m_RecentData.Count() - 1]->ServerIP == pSnapshot->ServerIP && m_RecentData[m_RecentData.Count() - 1]->Witnesses.Count() == pSnapshot->Witnesses.Count() )
	{
		bool bMismatch = false;
		FOR_EACH_VEC( pSnapshot->Witnesses, i )
		{
			if ( pSnapshot->Witnesses[i] != m_RecentData[m_RecentData.Count() - 1]->Witnesses[i] )
			{
				bMismatch = true;
				break;
			}
		}

		if ( !bMismatch )
		{
			delete m_RecentData[m_RecentData.Count() - 1];
			m_RecentData[m_RecentData.Count() - 1] = pSnapshot;

			return;
		}
	}

	m_RecentData.AddToTail( pSnapshot );
}

bool CRD_Player_Reporting::PrepareReportForSend( const char *szCategory, const char *szDescription, CSteamID reportedPlayer, const CUtlVector<CUtlBuffer> &screenshots, bool bShowWaitScreen )
{
	Assert( szCategory );
	Assert( szDescription || ( reportedPlayer.BIndividualAccount() && screenshots.Count() == 0 ) );
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

	if ( !szDescription )
	{
		V_snprintf( m_szQuickReport, sizeof( m_szQuickReport ), "%s/%llu", szCategory, reportedPlayer.ConvertToUint64() );
	}
	else
	{
		Assert( m_szQuickReport[0] == '\0' );
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
	if ( szDescription )
	{
		WriteJSONString( m_Buffer, "description" );
		WriteJSONRaw( m_Buffer, ":" );
		WriteJSONString( m_Buffer, szDescription );
		WriteJSONRaw( m_Buffer, "," );
	}

	if ( ReportingServerSnapshot_t *pSnapshot = GetRelevantOrLatestSnapshot( reportedPlayer ) )
	{
		pSnapshot->WriteJSON( m_Buffer );
	}

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

		HTTPRequestHandle hRequest = pSteamHTTP->CreateHTTPRequest( k_EHTTPMethodPOST, "https://gamereport.reactivedrop.com/playerreport" );
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
		// remember that we successfully quick-reported this player so we can show that in the UI.
		if ( m_szQuickReport[0] != '\0' )
		{
			RecentlyReportedPlayer( "", k_steamIDNil ); // call for side effect of initializing m_pRecentReports

			Assert( SteamUtils() );
			if ( m_pRecentReports != NULL && SteamUtils() )
			{
				uint64_t iNow = SteamUtils()->GetServerRealTime();
				uint64_t iExpireBefore = iNow - REPORTING_QUICK_TIMEOUT;

				FOR_EACH_SUBKEY( m_pRecentReports, pCategory )
				{
					KeyValues *pNext = pCategory->GetFirstSubKey();
					while ( pNext )
					{
						KeyValues *pCur = pNext;
						pNext = pNext->GetNextKey();

						if ( pCur->GetUint64() < iExpireBefore )
						{
							pCategory->RemoveSubKey( pCur );
							pCur->deleteThis();
						}
					}
				}

				m_pRecentReports->SetUint64( m_szQuickReport, iNow );

				CUtlBuffer buf;
				CFmtStr szFileName{ "cfg/clientrqpr_%llu.dat", SteamUser()->GetSteamID().ConvertToUint64() };
				if ( !m_pRecentReports->WriteAsBinary( buf ) )
				{
					Warning( "Failed to encode quick report cache\n" );
				}
				else if ( !g_pFullFileSystem->WriteFile( szFileName, "MOD", buf ) )
				{
					Warning( "Failed to write quick report cache\n" );
				}
			}

			m_szQuickReport[0] = '\0';
		}

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

	if ( !IsInProgress() )
	{
		// clear quick report key if we've exited the reporting process with an error
		m_szQuickReport[0] = '\0';
	}

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

ReportingServerSnapshot_t *CRD_Player_Reporting::NewSnapshot() const
{
	C_AlienSwarm *pAlienSwarm = ASWGameRules();
	Assert( pAlienSwarm );
	INetChannelInfo *pNCI = engine->GetNetChannelInfo();
	Assert( pNCI );

	ReportingServerSnapshot_t *pSnapshot = new ReportingServerSnapshot_t();

	pSnapshot->RecordedAt = Plat_FloatTime();

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

	pSnapshot->MissionName = engine->GetLevelNameShort();
	pSnapshot->ChallengeName = rd_challenge.GetString();
	if ( pAlienSwarm && pAlienSwarm->m_iMissionWorkshopID > 1000000 )
		pSnapshot->MissionWorkshop = pAlienSwarm->m_iMissionWorkshopID;
	if ( nChallengeFileID > 1000000 )
		pSnapshot->ChallengeWorkshop = nChallengeFileID;

	if ( pNCI )
		pSnapshot->ServerIP = pNCI->GetAddress();
	pSnapshot->LobbyID = UTIL_RD_GetCurrentLobbyID();

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		C_ASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
		if ( pPlayer && !pPlayer->IsAnyBot() )
		{
			Assert( pPlayer->GetSteamID().BIndividualAccount() );
			pSnapshot->Witnesses.AddToTail( pPlayer->GetSteamID() );
		}
	}

	if ( pNCI )
	{
		pSnapshot->HaveConnectionQuality = true;
		pSnapshot->CurTime = gpGlobals->curtime;
		pSnapshot->FrameTime[FLOW_OUTGOING] = gpGlobals->frametime;

		float flServerFrameTime, flServerFrameTimeStdDev;
		pNCI->GetRemoteFramerate( &flServerFrameTime, &flServerFrameTimeStdDev );
		pSnapshot->FrameTime[FLOW_INCOMING] = flServerFrameTime;

		for ( int flow = FLOW_OUTGOING; flow <= FLOW_INCOMING; flow++ )
		{
			pSnapshot->AvgLatency[flow] = pNCI->GetAvgLatency( flow );
			pSnapshot->AvgChoke[flow] = pNCI->GetAvgChoke( flow );
			pSnapshot->AvgLoss[flow] = pNCI->GetAvgLoss( flow );
			pSnapshot->AvgPackets[flow] = pNCI->GetAvgPackets( flow );
		}
	}

	return pSnapshot;
}

ReportingServerSnapshot_t *CRD_Player_Reporting::GetRelevantOrLatestSnapshot( CSteamID player ) const
{
	if ( m_RecentData.Count() == 0 )
		return NULL;

	FOR_EACH_VEC_BACK( m_RecentData, i )
	{
		if ( m_RecentData[i]->Witnesses.IsValidIndex( m_RecentData[i]->Witnesses.Find( player ) ) )
		{
			return m_RecentData[i];
		}
	}

	Assert( !player.IsValid() );
	return m_RecentData[m_RecentData.Count() - 1];
}

void CRD_Player_Reporting::DeleteExpiredSnapshots()
{
	// remove snapshots that have expired from the start of the list
	float flExpireBefore = Plat_FloatTime() - REPORTING_SNAPSHOT_LIFETIME;
	while ( m_RecentData.Count() != 0 && m_RecentData[0]->RecordedAt < flExpireBefore )
	{
		delete m_RecentData[0];
		m_RecentData.Remove( 0 );
	}

	// remove snapshots that have no new players from before the end of the list
	CUtlVector<CSteamID> SeenWitnesses;
	FOR_EACH_VEC_BACK( m_RecentData, i )
	{
		bool bAnyNew = false;
		FOR_EACH_VEC( m_RecentData[i]->Witnesses, j )
		{
			if ( !SeenWitnesses.IsValidIndex( SeenWitnesses.Find( m_RecentData[i]->Witnesses[j] ) ) )
			{
				SeenWitnesses.AddToTail( m_RecentData[i]->Witnesses[j] );
				bAnyNew = true;
			}
		}

		// keep the latest snapshot (it should have players in it, but just in case)
		if ( !bAnyNew && i != m_RecentData.Count() - 1 )
		{
			delete m_RecentData[i];
			m_RecentData.Remove( i );
		}
	}
}

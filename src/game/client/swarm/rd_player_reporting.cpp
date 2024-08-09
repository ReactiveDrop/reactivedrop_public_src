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
#include "rd_image_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

// number of seconds a report can be filed after someone leaves the server
#define REPORTING_SNAPSHOT_LIFETIME 3600.0f
// number of seconds a quick (descriptionless) report is considered to be "still active"
#define REPORTING_QUICK_TIMEOUT 604800

// CURRENTLY-DEFINED REPORT CATEGORIES
//
// my_account - for self-reported problems about items or achievements or whatever
//
// player_cheating - reporting a player for cheating
// player_abusive_gameplay - reporting a player for griefing
// player_abusive_communication - reporting a player for being abusive in text or voice chat or with another in-game communication tool
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

static const char s_szHexDigits[] = "0123456789abcdef";

static void WriteJSONHex( CUtlBuffer &buf, const CUtlBuffer &data )
{
	// V_binarytohex uses strcat, so we're doing it inline instead so it doesn't take multiple minutes to encode a jpeg.
	int nLen = data.TellPut() - data.TellGet();
	const byte *bytes = static_cast< const byte * >( data.PeekGet() );
	buf.EnsureCapacity( buf.TellPut() + nLen * 2 + 2 );
	buf.PutChar( '"' );
	for ( int i = 0; i < nLen; i++ )
	{
		buf.PutChar( s_szHexDigits[bytes[i] >> 4] );
		buf.PutChar( s_szHexDigits[bytes[i] & 0xf] );
	}
	buf.PutChar( '"' );
}

void ReportingServerSnapshot_t::WriteJSON( CUtlBuffer &buf ) const
{
	WriteJSONString( buf, "snapshot_taken" );
	WriteJSONFormat( buf, ":%u,", SnapshotTaken );
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
	WriteJSONFormat( buf, ":\"%llu\",", LobbyID.ConvertToUint64() );
	WriteJSONString( buf, "dedicated" );
	WriteJSONRaw( buf, ":" );
	WriteJSONRaw( buf, IsDedicatedServer ? "true" : "false" );
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
	FOR_EACH_VEC_BACK( m_RecentData, i )
	{
		if ( m_RecentData[i]->RecordedAt <= Plat_FloatTime() - REPORTING_SNAPSHOT_LIFETIME )
			break;

		if ( m_RecentData[i]->IsDedicatedServer )
			return true;
	}

	return false;
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

bool CRD_Player_Reporting::HasRecentlyPlayedWith() const
{
	float flExpireTime = Plat_FloatTime() - REPORTING_SNAPSHOT_LIFETIME;

	FOR_EACH_VEC_BACK( m_RecentData, i )
	{
		if ( m_RecentData[i]->RecordedAt < flExpireTime )
			break;

		if ( m_RecentData[i]->Witnesses.Count() > 1 )
			return true;
	}

	return false;
}

bool CRD_Player_Reporting::HasRecentlyPlayedWith( CSteamID player ) const
{
	float flExpireTime = Plat_FloatTime() - REPORTING_SNAPSHOT_LIFETIME;

	FOR_EACH_VEC_BACK( m_RecentData, i )
	{
		if ( m_RecentData[i]->RecordedAt < flExpireTime )
			break;

		FOR_EACH_VEC_BACK( m_RecentData[i]->Witnesses, j )
		{
			if ( player == m_RecentData[i]->Witnesses[j] )
			{
				return true;
			}
		}
	}

	return false;
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

const ReportingServerSnapshot_t *CRD_Player_Reporting::PinRelevantSnapshot( CSteamID player, bool bRequireDedicatedServer )
{
	if ( m_pPinnedSnapshot )
	{
		delete m_pPinnedSnapshot;
	}

	ReportingServerSnapshot_t *pRelevant = GetRelevantOrLatestSnapshot( player, bRequireDedicatedServer );
	if ( !pRelevant )
	{
		m_pPinnedSnapshot = NULL;
		return NULL;
	}

	m_pPinnedSnapshot = new ReportingServerSnapshot_t{ *pRelevant };

	return m_pPinnedSnapshot;
}

bool CRD_Player_Reporting::PrepareReportForSend( const char *szCategory, const char *szDescription, CSteamID reportedPlayer, const CUtlVector<CUtlBuffer> &screenshots, const ReportingServerSnapshot_t *pForceSnapshot )
{
	Assert( szCategory );
	Assert( szDescription || ( reportedPlayer.BIndividualAccount() && screenshots.Count() == 0 ) );
	Assert( !IsInProgress() );

	// can't queue multiple reports simultaneously. UI doesn't support this, so don't write an error message.
	if ( IsInProgress() )
		return false;

	V_strncpy( m_szLastCategory, szCategory, sizeof( m_szLastCategory ) );

	ISteamUser *pSteamUser = SteamUser();
	Assert( pSteamUser );
	Assert( SteamHTTP() );

	// we need to be connected to a game and also Steam in order to send a player report.
	if ( !pSteamUser || !SteamHTTP() )
	{
		TryLocalize( "#rd_reporting_error_not_connected", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		LogProgressMessage();

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
	LogProgressMessage();

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
		WriteJSONString( m_Buffer, "steamid64" );
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

	if ( !pForceSnapshot )
	{
		// mark the report so that we know whether the user chose the snapshot or whether it was just one picked at sending time.
		WriteJSONString( m_Buffer, "no_user_snapshot" );
		WriteJSONRaw( m_Buffer, ":true," );
	}
	if ( const ReportingServerSnapshot_t *pSnapshot = pForceSnapshot ? pForceSnapshot : GetRelevantOrLatestSnapshot( reportedPlayer ) )
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

void CRD_Player_Reporting::TakeScreenshot()
{
	int iWidth, iHeight;
	GetHudSize( iWidth, iHeight );

	CUtlMemory<unsigned char> screenPixels{ 0, iWidth * iHeight * 3 };
	{
		CMatRenderContextPtr pRenderContext( materials );
		pRenderContext->ReadPixels( 0, 0, iWidth, iHeight, screenPixels.Base(), IMAGE_FORMAT_RGB888 );
	}

	unsigned char *jpegbuffer = NULL;
	size_t jpegsize = 0;

	struct jpeg_compress_struct jpegInfo;
	ValveJpegErrorHandler_t jerr;
	jerr.InitCompress( &jpegInfo );

	if ( setjmp( jerr.m_ErrorContext ) )
	{
		jpeg_destroy_compress( &jpegInfo );
		free( jpegbuffer );

		// error occurred; clear latest screenshot as we don't have one
		m_LatestScreenshot.Purge();

		return;
	}

	jpegInfo.image_width = ScreenWidth();
	jpegInfo.image_height = ScreenHeight();
	jpegInfo.input_components = 3;
	jpegInfo.in_color_space = JCS_RGB;
	jpeg_set_defaults( &jpegInfo );

	jpeg_default_colorspace( &jpegInfo );

	jpeg_mem_dest( &jpegInfo, &jpegbuffer, &jpegsize );

	jpeg_set_quality( &jpegInfo, 90, false );

	if ( ScreenWidth() >= 3072 || ScreenHeight() >= 2048 )
	{
		// If we're on a huge screen, downsample the screenshot so it doesn't take absurd amounts of memory.
		int iDownsample = MAX( ScreenWidth(), ScreenHeight() ) >> 11;
		Assert( iDownsample > 1 );
		Assert( jpegInfo.scale_num == 1 );
		jpegInfo.scale_denom = iDownsample;
	}
	jpeg_calc_jpeg_dimensions( &jpegInfo );

	jpeg_start_compress( &jpegInfo, true );

	JSAMPROW scanline[1];
	for ( int y = 0; y < iHeight; y++ )
	{
		scanline[0] = screenPixels.Base() + ( y * iWidth * 3 );
		jpeg_write_scanlines( &jpegInfo, scanline, 1 );
	}

	jpeg_finish_compress( &jpegInfo );
	jpeg_destroy_compress( &jpegInfo );

	m_LatestScreenshot.Purge();
	m_LatestScreenshot.Put( jpegbuffer, jpegsize );

	free( jpegbuffer );
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
			LogProgressMessage();
			return;
		}

		TryLocalize( "#rd_reporting_sending_report_to_server", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		m_HTTPRequestCompleted.Set( hCall, this, &CRD_Player_Reporting::OnHTTPRequestCompleted );
		LogProgressMessage();
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
		LogProgressMessage();
	}
}

void CRD_Player_Reporting::OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
{
	if ( bIOFailure )
	{
		// entirely lost connection to the Steam client.
		TryLocalize( "#rd_reporting_error_sending_client", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		LogProgressMessage();
		return;
	}

	if ( !pParam->m_bRequestSuccessful )
	{
		SteamHTTP()->ReleaseHTTPRequest( pParam->m_hRequest );
		TryLocalize( "#rd_reporting_error_sending_network", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		LogProgressMessage();
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
		if ( StringHasPrefix( m_szLastCategory, "quick_commend_" ) )
			TryLocalize( "#rd_reporting_success_quick_commend", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		else if ( StringHasPrefix( m_szLastCategory, "quick_abusive_" ) )
			TryLocalize( "#rd_reporting_success_quick_report", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		else if ( StringHasPrefix( m_szLastCategory, "quick_auto_" ) )
			TryLocalize( "#rd_reporting_success_quick_auto", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		else
			TryLocalize( "#rd_reporting_success", m_wszLastMessage, sizeof( m_wszLastMessage ) );
		LogProgressMessage();
		return;
	}

	wchar_t wszHTTPStatus[8];
	V_snwprintf( wszHTTPStatus, NELEMS( wszHTTPStatus ), L"%03d", pParam->m_eStatusCode );
	g_pVGuiLocalize->ConstructString( m_wszLastMessage, sizeof( m_wszLastMessage ), g_pVGuiLocalize->Find( "#rd_reporting_error_http" ), 1, wszHTTPStatus );
	LogProgressMessage();
}

void CRD_Player_Reporting::LogProgressMessage()
{
	char szMessageUTF8[2048];
	V_UnicodeToUTF8( m_wszLastMessage, szMessageUTF8, sizeof( szMessageUTF8 ) );
	Msg( "[Player Reporting:%s] %s\n", m_szLastCategory, szMessageUTF8 );

	if ( !IsInProgress() )
	{
		// clear quick report key if we've exited the reporting process with an error
		m_szQuickReport[0] = '\0';
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
	pSnapshot->SnapshotTaken = SteamUtils() ? SteamUtils()->GetServerRealTime() : 0;

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

		pSnapshot->IsDedicatedServer = *UTIL_RD_GetCurrentLobbyData( "system:rd_dedicated_server", "" ) != '\0';
	}
	else
	{
		pSnapshot->IsDedicatedServer = false;
	}

	pSnapshot->MissionName = engine->GetLevelNameShort();
	pSnapshot->ChallengeName = rd_challenge.GetString();
	if ( pAlienSwarm && pAlienSwarm->m_iMissionWorkshopID > RD_MIN_WORKSHOP_FILE_ID )
		pSnapshot->MissionWorkshop = pAlienSwarm->m_iMissionWorkshopID;
	if ( nChallengeFileID > RD_MIN_WORKSHOP_FILE_ID )
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

ReportingServerSnapshot_t *CRD_Player_Reporting::GetRelevantOrLatestSnapshot( CSteamID player, bool bRequireDedicatedServer ) const
{
	if ( m_RecentData.Count() == 0 )
		return NULL;

	FOR_EACH_VEC_BACK( m_RecentData, i )
	{
		if ( bRequireDedicatedServer && !m_RecentData[i]->IsDedicatedServer )
		{
			continue;
		}

		if ( m_RecentData[i]->Witnesses.IsValidIndex( m_RecentData[i]->Witnesses.Find( player ) ) )
		{
			return m_RecentData[i];
		}

		if ( bRequireDedicatedServer && !player.IsValid() )
		{
			return m_RecentData[i];
		}
	}

	Assert( !player.IsValid() && !bRequireDedicatedServer );
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
	bool bSeenDedicated = false;
	FOR_EACH_VEC_BACK( m_RecentData, i )
	{
		bool bAnyNew = false;
		if ( !bSeenDedicated && m_RecentData[i]->IsDedicatedServer )
		{
			bSeenDedicated = true;
			bAnyNew = true;
		}
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

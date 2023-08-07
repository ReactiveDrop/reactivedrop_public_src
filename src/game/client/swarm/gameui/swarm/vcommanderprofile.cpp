#include "cbase.h"
#include "vcommanderprofile.h"
#include "c_asw_player.h"
#include "asw_medal_store.h"
#include "asw_util_shared.h"
#include "rd_vgui_commander_mini_profile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

CommanderProfile::CommanderProfile( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	SetProportional( true );

	m_pMiniProfile = new CRD_VGUI_Commander_Mini_Profile( this, "CommanderMiniProfile" );
	m_pMiniProfile->m_bEmbedded = true;

	SetTitle( "", false );
	SetDeleteSelfOnClose( true );
	SetLowerGarnishEnabled( false );
	SetMoveable( false );
}

CommanderProfile::~CommanderProfile()
{
}

void CommanderProfile::SetDataSettings( KeyValues *pSettings )
{
	if ( pSettings->GetBool( "showLocalPlayer" ) )
	{
		InitForLocalPlayer();
		return;
	}

	if ( int iEntIndex = pSettings->GetInt( "playerEntIndex" ) )
	{
		C_ASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( iEntIndex ) );
		Assert( pPlayer );
		if ( pPlayer )
		{
			InitForNetworkedPlayer( pPlayer );
			return;
		}
	}

	if ( uint64 iSteamID = pSettings->GetUint64( "steamid" ) )
	{
		InitForSteamID( CSteamID{ iSteamID } );
		return;
	}

	Assert( !"CommanderProfile not initialized" );
}

void CommanderProfile::InitForLocalPlayer()
{
	Assert( SteamUser() );
	if ( !SteamUser() )
		return;

	m_Mode = MODE_LOCAL_PLAYER;
	m_SteamID = SteamUser()->GetSteamID();

	C_ASW_Medal_Store *pMedalStore = GetMedalStore();
	Assert( pMedalStore );
	if ( pMedalStore )
	{
		m_iExperience = pMedalStore->GetExperience();
		m_iPromotion = pMedalStore->GetPromotion();
	}

	FetchLocalMedals();
	FetchHoIAFSeasonStats();

	TempOpenStatsWebsiteAndClose();
}

void CommanderProfile::InitForNetworkedPlayer( C_ASW_Player *pPlayer )
{
	m_Mode = MODE_NETWORKED_PLAYER;
	m_SteamID = pPlayer->GetSteamID();
	m_iExperience = pPlayer->GetExperience();
	m_iPromotion = pPlayer->GetPromotion();

	COMPILE_TIME_ASSERT( RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS == 3 );
	m_Medals[0] = pPlayer->m_EquippedItemDataStatic.m_Medal0;
	m_Medals[1] = pPlayer->m_EquippedItemDataStatic.m_Medal1;
	m_Medals[2] = pPlayer->m_EquippedItemDataStatic.m_Medal2;

	FetchHoIAFSeasonStats();

	TempOpenStatsWebsiteAndClose();
}

void CommanderProfile::InitForSteamID( CSteamID steamID )
{
	Assert( steamID.IsValid() && steamID.BIndividualAccount() );

	m_Mode = MODE_STEAM_ID;
	m_SteamID = steamID;

	FetchSteamIDMedalsAndExperience();
	FetchHoIAFSeasonStats();

	TempOpenStatsWebsiteAndClose();
}

void CommanderProfile::TempOpenStatsWebsiteAndClose()
{
	// TODO FIXME: until the actual page is implemented, open the stats website in the Steam Overlay
	char szStatsWeb[256];
	V_snprintf( szStatsWeb, sizeof( szStatsWeb ), "https://stats.reactivedrop.com/profiles/%I64u?lang=%s&utm_source=profilewip", m_SteamID.ConvertToUint64(), SteamApps()->GetCurrentGameLanguage() );
	BaseModUI::CUIGameData::Get()->ExecuteOverlayUrl( szStatsWeb );

	m_iTempCloseAfter = 5;
}

void CommanderProfile::OnThink()
{
	BaseClass::OnThink();

	if ( m_iTempCloseAfter )
	{
		m_iTempCloseAfter--;

		if ( m_iTempCloseAfter <= 0 )
		{
			BaseModUI::CBaseModPanel::GetSingleton().OpenFrontScreen();
		}
	}
}

void CommanderProfile::FetchLocalMedals()
{
	Assert( !"TODO" );
}

void CommanderProfile::FetchSteamIDMedalsAndExperience()
{
	Assert( m_SteamID.IsValid() && m_SteamID.BIndividualAccount() );

	ISteamHTTP *pHTTP = SteamHTTP();
	Assert( pHTTP );
	if ( !pHTTP )
	{
		Assert( !"TODO: error message" );
		return;
	}

	m_hHTTPRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodGET, "https://stats.reactivedrop.com/api/get_equipped_medals" );
	pHTTP->SetHTTPRequestGetOrPostParameter( m_hHTTPRequest, "steamid", CFmtStr{ "%llu", m_SteamID.ConvertToUint64() } );
	pHTTP->SetHTTPRequestUserAgentInfo( m_hHTTPRequest, "CommanderProfileFetch" );

	SteamAPICall_t hCall = k_uAPICallInvalid;
	pHTTP->SendHTTPRequest( m_hHTTPRequest, &hCall );
	// user is looking, so try to be quick
	pHTTP->PrioritizeHTTPRequest( m_hHTTPRequest );

	m_SteamIDProfileDataFetched.Set( hCall, this, &CommanderProfile::OnSteamIDProfileDataFetched );
}

void CommanderProfile::FetchHoIAFSeasonStats()
{
	Assert( m_SteamID.IsValid() && m_SteamID.BIndividualAccount() );

	ISteamUserStats *pUserStats = SteamUserStats();
	Assert( pUserStats );
	if ( !pUserStats )
	{
		Assert( !"TODO: error message" );
		return;
	}

	SteamAPICall_t hCall = pUserStats->DownloadLeaderboardEntriesForUsers( STEAM_LEADERBOARD_HOIAF_CURRENT_SEASON, &m_SteamID, 1 );
	m_HoIAFSeasonStatsFetched.Set( hCall, this, &CommanderProfile::OnHoIAFSeasonStatsFetched );
}

void CommanderProfile::OnHoIAFSeasonStatsFetched( LeaderboardScoresDownloaded_t *pParam, bool bIOFailure )
{
	ISteamUserStats *pUserStats = SteamUserStats();
	Assert( pUserStats );

	if ( bIOFailure )
	{
		Assert( !"TODO: failed to send request" );
		return;
	}

	if ( pParam->m_cEntryCount == 0 )
	{
		Assert( !"TODO: no HoIAF score this season" );
		return;
	}

	bool bOK = pUserStats->GetDownloadedLeaderboardEntry( pParam->m_hSteamLeaderboardEntries, 0, &m_HoIAFLeaderboardEntry, reinterpret_cast< int32 * >( &m_HoIAFDetails ), sizeof( m_HoIAFDetails ) / sizeof( int32 ) );
	Assert( bOK ); ( void )bOK;
	Assert( m_HoIAFLeaderboardEntry.m_cDetails == sizeof( m_HoIAFDetails ) / sizeof( int32 ) );

	Assert( !"TODO: what now?" );
}

void CommanderProfile::OnSteamIDProfileDataFetched( HTTPRequestCompleted_t *pParam, bool bIOFailure )
{
	ISteamHTTP *pHTTP = SteamHTTP();
	Assert( pHTTP );

	Assert( bIOFailure || ( pParam && pParam->m_hRequest == m_hHTTPRequest ) );
	if ( bIOFailure || !pParam->m_bRequestSuccessful )
	{
		Assert( !"TODO: failed to send request at all" );
		pHTTP->ReleaseHTTPRequest( m_hHTTPRequest );
		m_hHTTPRequest = INVALID_HTTPREQUEST_HANDLE;
		return;
	}

	if ( pParam->m_eStatusCode != k_EHTTPStatusCode200OK )
	{
		Assert( !"TODO: error returned by server" );
		pHTTP->ReleaseHTTPRequest( m_hHTTPRequest);
		m_hHTTPRequest = INVALID_HTTPREQUEST_HANDLE;
		return;
	}

	uint32 nDataSize{};
	pHTTP->GetHTTPResponseBodySize( m_hHTTPRequest, &nDataSize );
	CUtlBuffer buf;
	buf.EnsureCapacity( nDataSize );
	buf.SeekPut( CUtlBuffer::SEEK_HEAD, nDataSize );
	pHTTP->GetHTTPResponseBodyData( m_hHTTPRequest, static_cast< uint8 * >( buf.Base() ), nDataSize );

	pHTTP->ReleaseHTTPRequest( m_hHTTPRequest );
	m_hHTTPRequest = INVALID_HTTPREQUEST_HANDLE;

	KeyValues::AutoDelete pKV{ "EM" };
	if ( !pKV->ReadAsBinary( buf ) )
	{
		Assert( !"TODO: handle bad data" );
		return;
	}
	Assert( buf.TellGet() == buf.TellPut() );

	KeyValuesDumpAsDevMsg( pKV );

	m_iExperience = pKV->GetInt( "e" );
	m_iPromotion = pKV->GetInt( "p" );

	for ( int i = 0; i < NELEMS( m_Medals ); i++ )
	{
		m_Medals[i].Reset();

		KeyValues *pMedal = pKV->FindKey( CFmtStr{ "m%d", i } );
		if ( pMedal )
		{
			ReactiveDropInventory::ItemInstance_t instance{ pMedal };
			m_Medals[i].SetFromInstance( instance );
		}
	}

	Assert( !"TODO: what now?" );
}

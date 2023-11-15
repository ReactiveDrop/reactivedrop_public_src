//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "VFoundGames.h"
#include "VGenericPanelList.h"
#include "EngineInterface.h"
#include "VFooterPanel.h"
#include "VHybridButton.h"
#include "VDropDownMenu.h"
#include "VFlyoutMenu.h"
#include "UIGameData.h"
#include "vdownloadcampaign.h"
#include "gameui_util.h"

#include "rd_workshop.h"
#include "filesystem.h"

#include "vgui/ISurface.h"
#include "vgui/IBorder.h"
#include "vgui/ISystem.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui/ILocalize.h"
#include "VGenericConfirmation.h"
#include "VGameSettings.h"
#include "vgetlegacydata.h"
#include "cdll_util.h"
#include "nb_header_footer.h"
#include "nb_button.h"
#include "fmtstr.h"
#include "smartptr.h"
#include "rd_missions_shared.h"
#include "rd_lobby_utils.h"
#include "mapentities_shared.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

ConVar ui_foundgames_spinner_time( "ui_foundgames_spinner_time", "1", FCVAR_DEVELOPMENTONLY );
ConVar ui_foundgames_update_time( "ui_foundgames_update_time", "1", FCVAR_DEVELOPMENTONLY );
ConVar ui_foundgames_fake_content( "ui_foundgames_fake_content", "0", FCVAR_DEVELOPMENTONLY );
ConVar ui_foundgames_fake_count( "ui_foundgames_fake_count", "0", FCVAR_DEVELOPMENTONLY );
ConVar rd_lobby_ping_low( "rd_lobby_ping_low", "120", FCVAR_NONE, "lobbies with an estimated ping below this many milliseconds are considered \"low ping\"." );
ConVar rd_lobby_ping_high( "rd_lobby_ping_high", "250", FCVAR_NONE, "lobbies with an estimated ping above this many milliseconds are considered \"high ping\"." );

void Demo_DisableButton( Button *pButton );

const char *COM_GetModDirectory();

static char const * NoTeamGameMode( char const * szGameMode )
{
	if ( char const *szNoTeamMode = StringAfterPrefix( szGameMode, "team" ) )
		return szNoTeamMode;
	else
		return szGameMode;
}

bool BaseModUI::FoundGameListItem::Info::IsJoinable() const
{
	if ( IsOtherTitle() )
		return false;
	if ( IsDownloadable() )
		return false;

	return *GetNonJoinableShortHint( false ) == '\0';
}

bool FoundGameListItem::Info::HaveMap() const
{
	return CompareMapVersion() != INT_MIN;
}

int BaseModUI::FoundGameListItem::Info::CompareMapVersion() const
{
	char szBSPName[MAX_PATH]{};
	int iExpectedVersion = m_iMapVersion;
	if ( m_szMissionFile[0] != '\0' )
	{
		V_snprintf( szBSPName, sizeof( szBSPName ), "maps/%s.bsp", m_szMissionFile );
	}
	else
	{
		return INT_MIN;
	}

	if ( !filesystem->FileExists( szBSPName, "GAME" ) )
	{
		return INT_MIN;
	}

	if ( iExpectedVersion == 0 )
	{
		// We've done all we can. The lobby or dedicated server didn't tell us
		// what version of the map we needed to have.
		return 0;
	}

	// Only load the map version from each map once per session. Workshop maps
	// could technically change, but the game isn't set up to handle that
	// gracefully in a lot of other places, so just assume the player will
	// restart the game if needed.
	static CUtlStringMap<int> s_LocalMapVersion;

	if ( s_LocalMapVersion.Defined( szBSPName ) )
	{
		return iExpectedVersion - s_LocalMapVersion[szBSPName];
	}

	FileHandle_t hFile = filesystem->Open( szBSPName, "rb", "GAME" );
	Assert( hFile );
	if ( !hFile )
	{
		return INT_MIN;
	}

	BSPHeader_t header;
	int iRead = filesystem->Read( &header, sizeof( header ), hFile );
	Assert( iRead == sizeof( header ) );
	if ( iRead != sizeof( header ) )
	{
		filesystem->Close( hFile );
		return INT_MIN;
	}

	Assert( header.ident == IDBSPHEADER );
	filesystem->Seek( hFile, header.lumps[LUMP_ENTITIES].fileofs, FILESYSTEM_SEEK_HEAD );

	CUtlMemory<char> entities{ 0, header.lumps[LUMP_ENTITIES].filelen };
	iRead = filesystem->Read( entities.Base(), entities.Count(), hFile );
	Assert( iRead == entities.Count() );

	filesystem->Close( hFile );

	int iMapVersion = 0;

	Assert( *entities.Base() == '{' );

	char szMapVersion[MAPKEY_MAXLENGTH]{};
	if ( MapEntity_ExtractValue( entities.Base() + 1, "mapversion", szMapVersion ) )
	{
		iMapVersion = atoi( szMapVersion );
	}

	s_LocalMapVersion[szBSPName] = iMapVersion;

	return iExpectedVersion - iMapVersion;
}

char const *FoundGameListItem::Info::IsOtherTitle() const
{
	if ( m_szOtherTitle[0] )
		return m_szOtherTitle;

	return NULL;
}

bool BaseModUI::FoundGameListItem::Info::IsDownloadable() const
{
	char const *szWebsite = m_szMissionWebsite;
	PublishedFileId_t iWorkshopFile = m_iMissionWorkshopID;
	if ( !*szWebsite && iWorkshopFile == k_PublishedFileIdInvalid )
		return false;

	const char *szMissionName = m_szMissionFile;
	const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( szMissionName );
	if ( !pMission || !pMission->Installed )
		return true;

	char *pEndPos = NULL;
	float flApproximateMapVersion = strtof( STRING( pMission->Version ), &pEndPos );
	return V_stricmp( STRING( pMission->Version ), m_szMissionVersion ) &&
		// BenLubar: ugh, we're dealing with multiple layers of string->floating point->string conversions.
		// matchmaking.dll uses Valve KeyValues, which automatically reformats float-like strings, and we're using
		// our own implementation that handles longer strings without corrupting them.
		// just check if the numbers are about the same, assuming they're numbers.
		( !m_flMissionVersion || !CloseEnough( flApproximateMapVersion, m_flMissionVersion ) );
}

const char *BaseModUI::FoundGameListItem::Info::GetNonJoinableShortHint( bool bWarnOnNoHint ) const
{
	if ( !m_iMaxPlayers )
		return "";

	if ( m_iCurPlayers >= m_iMaxPlayers )
		return "#L4D360UI_WaitScreen_GameFull";

	if ( int iMapVersionDiff = CompareMapVersion() )
	{
		if ( iMapVersionDiff == INT_MIN )
		{
			return "#L4D360UI_Lobby_CampaignUnavailable";
		}

		if ( iMapVersionDiff < 0 )
		{
			return "#L4D360UI_Lobby_LocalMapNewer";
		}

		return "#L4D360UI_Lobby_LocalMapOlder";
	}

	if ( bWarnOnNoHint )
	{
		Assert( !"No specific hint for non-joinable lobby" );
	}

	return "";
}

const char *BaseModUI::FoundGameListItem::Info::GetJoinButtonHint() const
{
	if ( IsOtherTitle() )
	{
		return "#L4D360UI_FoundGames_Join_Modded";
	}
	if ( IsDownloadable() )
	{
		return "#L4D360UI_FoundGames_Join_Download";
	}
	if ( int iMapVersionDiff = CompareMapVersion() )
	{
		if ( iMapVersionDiff == INT_MIN )
		{
			return "#L4D360UI_Lobby_CampaignUnavailable";
		}

		if ( iMapVersionDiff < 0 )
		{
			return "#L4D360UI_Lobby_LocalMapNewer";
		}

		return "#L4D360UI_Lobby_LocalMapOlder";
	}
	if ( IsJoinable() )
	{
		return "#L4D360UI_FoundGames_Join_Success";
	}

	if ( m_iMaxPlayers <= 0 )
		return "#L4D360UI_FoundGames_Join_Fail_Not_In_Joinable";

	if ( m_iCurPlayers >= m_iMaxPlayers )
		return "#L4D360UI_FoundGames_Join_Fail_No_Slots";

	return "#L4D360UI_FoundGames_Join_Fail_Not_In_Joinable";
}

void FoundGameListItem::Info::SetOtherTitleFromLobby()
{
	if ( V_stricmp( m_szModDir, COM_GetModDirectory() ) )
	{
		V_strncpy( m_szOtherTitle, m_szModDir, sizeof( m_szOtherTitle ) );
	}
	else if ( m_szNetworkVersion[0] != '\0' && V_strcmp( m_szNetworkVersion, engine->GetProductVersionString() ) )
	{
		if ( m_szGameBranch[0] != '\0' )
			V_strncpy( m_szOtherTitle, m_szGameBranch, sizeof( m_szOtherTitle ) );
		else if ( m_iGameVersion != 0 )
			V_snprintf( m_szOtherTitle, sizeof( m_szOtherTitle ), "%d", m_iGameVersion );
		else
			V_strncpy( m_szOtherTitle, "?", sizeof( m_szOtherTitle ) );
	}
}

static void TryLocalizeWithFallback( const char *szTokenOrLocalizedString, const char *szFallbackString, wchar_t *wszDest, int nDestSizeInBytes )
{
	if ( szTokenOrLocalizedString[0] != '#' )
	{
		V_UTF8ToUnicode( szTokenOrLocalizedString, wszDest, nDestSizeInBytes );
	}
	else if ( const wchar_t *wszLocalized = g_pVGuiLocalize->Find( szTokenOrLocalizedString ) )
	{
		V_wcsncpy( wszDest, wszLocalized, nDestSizeInBytes );
	}
	else
	{
		V_UTF8ToUnicode( szFallbackString, wszDest, nDestSizeInBytes );
	}
}

static void TryParseWorkshopID( PublishedFileId_t &id, const char *szID )
{
	if ( *szID == '\0' )
	{
		id = k_PublishedFileIdInvalid;
		return;
	}

	char *pEnd;
	id = strtoull( szID, &pEnd, 16 );

	const char *pActualEnd = szID + V_strlen( szID );
	Assert( pEnd == pActualEnd );
	if ( pEnd != pActualEnd )
	{
		id = k_PublishedFileIdInvalid;
	}
}

template<typename E, size_t N>
static void TryParseEnum( E &value, const char *const ( &names )[N], const char *szValue, E defaultValue )
{
	for ( size_t i = 0; i < N; i++ )
	{
		if ( !V_stricmp( szValue, names[i] ) )
		{
			value = E( i );
			return;
		}
	}

	value = defaultValue;
}

static void TryParseBoolean( bool &bValue, const char *szValue, bool bDefault = false )
{
	if ( *szValue == '\0' )
	{
		bValue = bDefault;
		return;
	}

	if ( !V_strcmp( szValue, "1" ) )
	{
		bValue = true;
	}
	else
	{
		Assert( !V_strcmp( szValue, "0" ) );
		bValue = false;
	}
}

static FoundGameListItem::Info::GAME_PING GetPingCategory( int ms )
{
	if ( ms <= 0 )
		return FoundGameListItem::Info::PING_NONE;
	if ( ms < rd_lobby_ping_low.GetInt() )
		return FoundGameListItem::Info::PING_LOW;
	if ( ms <= rd_lobby_ping_high.GetInt() )
		return FoundGameListItem::Info::PING_MEDIUM;

	return FoundGameListItem::Info::PING_HIGH;
}

static const char *const s_szGameModeNames[] = { "", "campaign", "single_mission" };
static const char *const s_szGameStateNames[] = { "ingame", "briefing" };
static const char *const s_szGameDifficultyNames[] = { "easy", "normal", "hard", "insane", "imba" };
static const char *const s_szServerTypeNames[] = { "listen", "dedicated" };
static const char *const s_szLobbyAccessNames[] = { "public", "friends" };

bool FoundGameListItem::Info::SetFromFriend( CSteamID friendID, const FriendGameInfo_t &info )
{
	if ( info.m_steamIDLobby.IsLobby() && SetFromLobby( info.m_steamIDLobby ) )
	{
		m_Friends.AddToTail( friendID );

		return true;
	}

	if ( info.m_steamIDLobby.IsLobby() )
	{
		// ask for the lobby; maybe we'll have it next time
		SteamMatchmaking()->RequestLobbyData( info.m_steamIDLobby );

		return false;
	}

	Assert( !"TODO: handle friend in non-lobby server?" );

	return false;
}

bool FoundGameListItem::Info::SetFromLobby( CSteamID lobby )
{
	Assert( lobby.IsLobby() );

	ISteamMatchmaking *pMatchmaking = SteamMatchmaking();
	Assert( pMatchmaking );
	if ( !pMatchmaking )
		return false;

#define LOBBY_DATA( key ) pMatchmaking->GetLobbyData( lobby, key )

	if ( V_strcmp( LOBBY_DATA( "game:dlcrequired" ), "0" ) || V_strcmp( LOBBY_DATA( "game:state" ), "game" ) || V_strcmp( LOBBY_DATA( "system:lock" ), "" ) || V_strcmp( LOBBY_DATA( "system:network" ), "LIVE" ) )
	{
		// Lobby is not from AS:RD (might be a mod or someone running a different game with the wrong AppID)
		return false;
	}

	m_Type = FoundGameListItem::TYPE_LOBBY;
	m_LobbyID = lobby;

	V_strncpy( m_szCampaignFile, LOBBY_DATA( "game:campaign" ), sizeof( m_szCampaignFile ) );
	V_strncpy( m_szChallengeFile, LOBBY_DATA( "game:challenge" ), sizeof( m_szChallengeFile ) );
	V_strncpy( m_szMissionFile, LOBBY_DATA( "game:mission" ), sizeof( m_szMissionFile ) );

	V_strncpy( m_szMissionImage, LOBBY_DATA( "game:missioninfo:image" ), sizeof( m_szMissionImage ) );
	V_strncpy( m_szMissionWebsite, LOBBY_DATA( "game:missioninfo:website" ), sizeof( m_szMissionWebsite ) );
	V_strncpy( m_szMissionVersion, LOBBY_DATA( "game:missioninfo:version" ), sizeof( m_szMissionVersion ) );
	m_flMissionVersion = V_atof( m_szMissionVersion );

	TryLocalizeWithFallback( LOBBY_DATA( "game:challengeinfo:displaytitle" ), m_szChallengeFile, m_wszChallengeDisplayName, sizeof( m_wszChallengeDisplayName ) );
	TryLocalizeWithFallback( LOBBY_DATA( "game:missioninfo:displaytitle" ), m_szMissionFile, m_wszMissionDisplayName, sizeof( m_wszMissionDisplayName ) );
	TryLocalize( LOBBY_DATA( "game:missioninfo:author" ), m_wszAuthorName, sizeof( m_wszAuthorName ) );

	V_wcsncpy( m_wszLobbyDisplayName, m_wszMissionDisplayName, sizeof( m_wszLobbyDisplayName ) );

	TryParseWorkshopID( m_iChallengeWorkshopID, LOBBY_DATA( "game:challengeinfo:workshop" ) );
	TryParseWorkshopID( m_iMissionWorkshopID, LOBBY_DATA( "game:missioninfo:workshop" ) );

	CSplitString RequiredItems{ LOBBY_DATA( "game:required_workshop_items" ), "," };
	FOR_EACH_VEC( RequiredItems, i )
	{
		PublishedFileId_t id;
		TryParseWorkshopID( id, RequiredItems[i] );

		Assert( id != k_PublishedFileIdInvalid || ( RequiredItems.Count() == 1 && i == 0 && *RequiredItems[0] == '\0' ) );
		if ( id != k_PublishedFileIdInvalid )
		{
			m_RequiredWorkshopItems.AddToTail( id );
		}
	}

	TryParseBoolean( m_bMissionBuiltIn, LOBBY_DATA( "game:missioninfo:builtin" ) );
	TryParseBoolean( m_bMissionOfficial, LOBBY_DATA( "game:missioninfo:official" ) );

	TryParseEnum( m_eGameMode, s_szGameModeNames, LOBBY_DATA( "game:mode" ), MODE_UNKNOWN );
	TryParseEnum( m_eGameState, s_szGameStateNames, LOBBY_DATA( "game:swarmstate" ), STATE_INGAME );

	TryParseEnum( m_eGameDifficulty, s_szGameDifficultyNames, LOBBY_DATA( "game:difficulty" ), DIFFICULTY_NORMAL );
	TryParseBoolean( m_bOnslaught, LOBBY_DATA( "game:onslaught" ) );
	TryParseBoolean( m_bHardcoreFF, LOBBY_DATA( "game:hardcoreFF" ) );

	TryParseEnum( m_eServerType, s_szServerTypeNames, LOBBY_DATA( "options:server" ), SERVER_LISTEN );
	if ( m_eServerType == SERVER_DEDICATED )
	{
		netadr_t ip{ LOBBY_DATA( "system:rd_dedicated_server" ) };
		if ( ip.IsValid() )
		{
			// netadr_t wants big endian, but servernetadr_t wants little endian.
			m_ServerIP.Init( DWordSwap( ip.GetIP() ), ip.GetPort(), ip.GetPort() );
		}
	}
	TryParseEnum( m_eLobbyAccess, s_szLobbyAccessNames, LOBBY_DATA( "system:access" ), ACCESS_PUBLIC );
	V_strncpy( m_szModDir, LOBBY_DATA( "game:dir" ), sizeof( m_szModDir ) );
	V_strncpy( m_szNetworkVersion, LOBBY_DATA( "system:game_version" ), sizeof( m_szNetworkVersion ) );
	V_strncpy( m_szGameBranch, LOBBY_DATA( "system:game_branch" ), sizeof( m_szGameBranch ) );
	m_iGameVersion = V_atoi( LOBBY_DATA( "system:game_build" ) );
	m_iMapVersion = V_atoi( LOBBY_DATA( "system:map_version" ) );

	m_iCurPlayers = V_atoi( LOBBY_DATA( "members:numPlayers" ) );
	m_iMaxPlayers = V_atoi( LOBBY_DATA( "members:numSlots" ) );

	if ( ISteamNetworkingUtils *pNetworkingUtils = SteamNetworkingUtils() )
	{
		if ( pNetworkingUtils->ParsePingLocationString( LOBBY_DATA( "system:rd_lobby_location" ), m_PingLocation ) )
		{
			m_iPingMS = pNetworkingUtils->EstimatePingTimeFromLocalHost( m_PingLocation );
		}
	}

	m_ePingCategory = GetPingCategory( m_iPingMS );

	return true;
}

bool FoundGameListItem::Info::SetFromServer( CReactiveDropServerListHelper &helper, int i, FoundGameListItem::Type_t eType )
{
	gameserveritem_t *pDetails = helper.GetDetails( i );
	Assert( pDetails );
	if ( !pDetails )
		return false;

	m_Type = eType;
	if ( eType == FoundGameListItem::TYPE_SERVER )
	{
		// only set specific type if it's not a different specific type already (LAN or favorite)
		if ( !pDetails->m_bSecure )
			m_Type = FoundGameListItem::TYPE_INSECURESERVER;
		else if ( helper.IsHoIAFServer( pDetails ) )
			m_Type = FoundGameListItem::TYPE_RANKEDSERVER;
	}

	m_ServerIP = pDetails->m_NetAdr;

	V_strncpy( m_szMissionFile, pDetails->m_szMap, sizeof( m_szMissionFile ) );

	V_UTF8ToUnicode( pDetails->GetName(), m_wszLobbyDisplayName, sizeof( m_wszLobbyDisplayName ) );
	if ( V_strcmp( pDetails->m_szGameDescription, "Alien Swarm: Reactive Drop" ) )
		V_UTF8ToUnicode( pDetails->m_szGameDescription, m_wszChallengeDisplayName, sizeof( m_wszChallengeDisplayName ) );

	m_eServerType = SERVER_DEDICATED;
	m_eLobbyAccess = pDetails->m_bPassword ? ACCESS_PASSWORD : ACCESS_PUBLIC;
	V_strncpy( m_szModDir, pDetails->m_szGameDir, sizeof( m_szModDir ) );
	V_snprintf( m_szNetworkVersion, sizeof( m_szNetworkVersion ), "%d.%d.%d.%d", pDetails->m_nServerVersion / 1000, pDetails->m_nServerVersion / 100 % 10, pDetails->m_nServerVersion / 10 % 10, pDetails->m_nServerVersion % 10 );

	m_iCurPlayers = pDetails->m_nPlayers;
	m_iMaxPlayers = pDetails->m_nMaxPlayers;

	m_iPingMS = pDetails->m_nPing;
	m_ePingCategory = eType == FoundGameListItem::TYPE_LANSERVER ? PING_SYSTEMLINK : GetPingCategory( m_iPingMS );

	CSplitString Tags{ pDetails->m_szGameTags, "," };
	// non-sv_tags tags are always in the same order
	int iCurTag = 0;
	if ( Tags.Count() > iCurTag && !V_strcmp( Tags[iCurTag], "HoIAF" ) )
		iCurTag++; // already handled HoIAF tag earlier
	if ( Tags.Count() > iCurTag && !V_strcmp( Tags[iCurTag], "HardcoreFF" ) )
	{
		m_bHardcoreFF = true;
		iCurTag++;
	}
	if ( Tags.Count() > iCurTag && !V_strcmp( Tags[iCurTag], "Onslaught" ) )
	{
		m_bOnslaught = true;
		iCurTag++;
	}
	if ( Tags.Count() > iCurTag && !V_strcmp( Tags[iCurTag], "Easy" ) )
	{
		m_eGameDifficulty = DIFFICULTY_EASY;
		iCurTag++;
	}
	else if ( Tags.Count() > iCurTag && !V_strcmp( Tags[iCurTag], "Normal" ) )
	{
		m_eGameDifficulty = DIFFICULTY_NORMAL;
		iCurTag++;
	}
	else if ( Tags.Count() > iCurTag && !V_strcmp( Tags[iCurTag], "Hard" ) )
	{
		m_eGameDifficulty = DIFFICULTY_HARD;
		iCurTag++;
	}
	else if ( Tags.Count() > iCurTag && !V_strcmp( Tags[iCurTag], "Insane" ) )
	{
		m_eGameDifficulty = DIFFICULTY_INSANE;
		iCurTag++;
	}
	else if ( Tags.Count() > iCurTag && !V_strcmp( Tags[iCurTag], "Imba" ) )
	{
		m_eGameDifficulty = DIFFICULTY_BRUTAL;
		iCurTag++;
	}
	else
	{
		Assert( !"no difficulty tag for server" );
	}
	if ( Tags.Count() > iCurTag && m_wszChallengeDisplayName[0] != '\0' )
	{
		// assume this tag is the challenge filename
		V_strncpy( m_szChallengeFile, Tags[iCurTag], sizeof( m_szChallengeFile ) );
		iCurTag++;
	}
	if ( Tags.Count() > iCurTag && !V_strcmp( Tags[iCurTag], "Briefing" ) )
	{
		m_eGameState = STATE_BRIEFING;
		iCurTag++;
	}
	if ( Tags.Count() > iCurTag && !V_strcmp( Tags[iCurTag], "empty" ) )
		iCurTag++;
	if ( Tags.Count() > iCurTag && !V_strncmp( Tags[iCurTag], "*grp:", 5 ) && Tags[iCurTag][V_strlen( Tags[iCurTag] ) - 1] == 'i' )
	{
		V_strncpy( m_szGroupID, Tags[iCurTag] + 5, sizeof( m_szGroupID ) );
		m_szGroupID[V_strlen( m_szGroupID ) - 1] = '\0';
		iCurTag++;
	}

	SetDefaultMissionData();

	return true;
}

bool FoundGameListItem::Info::Merge( const Info &info )
{
	bool bChanged = false;

	FOR_EACH_VEC( info.m_Friends, i )
	{
		if ( !m_Friends.IsValidIndex( m_Friends.Find( info.m_Friends[i] ) ) )
		{
			m_Friends.AddToTail( info.m_Friends[i] );
			bChanged = true;
		}
	}

	if ( m_Type == TYPE_LOBBY && info.m_Type != TYPE_LOBBY )
	{
		m_Type = info.m_Type;
		bChanged = true;
	}
	else
	{
		Assert( m_Type == info.m_Type || info.m_Type == TYPE_LOBBY );
	}

	return bChanged;
}

void FoundGameListItem::Info::SetDefaultMissionData()
{
	if ( const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( m_szMissionFile ) )
	{
		if ( m_iMissionWorkshopID == k_PublishedFileIdInvalid )
			m_iMissionWorkshopID = pMission->WorkshopID;

		m_bMissionBuiltIn = pMission->Builtin;

		if ( pMission->Image != NULL_STRING && m_szMissionImage[0] == '\0' )
			V_strncpy( m_szMissionImage, STRING( pMission->Image ), sizeof( m_szMissionImage ) );

		if ( pMission->Website != NULL_STRING && m_szMissionWebsite[0] == '\0' )
			V_strncpy( m_szMissionWebsite, STRING( pMission->Website ), sizeof( m_szMissionWebsite ) );

		if ( pMission->Version != NULL_STRING && m_szMissionVersion[0] == '\0' )
			V_strncpy( m_szMissionVersion, STRING( pMission->Version ), sizeof( m_szMissionVersion ) );

		if ( pMission->Author != NULL_STRING && m_wszAuthorName[0] == L'\0' )
			TryLocalize( STRING( pMission->Author ), m_wszAuthorName, sizeof( m_wszAuthorName ) );

		if ( pMission->MissionTitle != NULL_STRING && m_wszMissionDisplayName[0] == L'\0' )
			TryLocalize( STRING( pMission->MissionTitle ), m_wszMissionDisplayName, sizeof( m_wszMissionDisplayName ) );

		if ( m_wszLobbyDisplayName[0] == L'\0' )
			V_wcsncpy( m_wszLobbyDisplayName, m_wszMissionDisplayName, sizeof( m_wszLobbyDisplayName ) );
	}
}

//=============================================================================
//
//=============================================================================
FoundGameListItem::FoundGameListItem( vgui::Panel *parent, const char *panelName ):
	BaseClass( parent, panelName ),
	m_pListCtrlr( ( GenericPanelList * ) parent )
{
	m_pImgPing = NULL;
	m_pImgPingSmall = NULL;
	m_pLblPing = NULL;
	m_pLblPlayerGamerTag = NULL;
	m_pLblDifficulty = NULL;
	m_pLblChallenge = NULL;
	m_pLblSwarmState = NULL;
	m_pLblPlayers = NULL;
	m_pLblNotJoinable = NULL;

	m_wszChallengeName[0] = 0;

	SetProportional( true );

	SetPaintBackgroundEnabled( true );

	m_iBaseTall = 10;
	m_sweep = 0;
	m_bSelected = false;
	m_bHasMouseover = false;

	CBaseModFrame::AddFrameListener( this );
}

//=============================================================================
FoundGameListItem::~FoundGameListItem()
{
	CBaseModFrame::RemoveFrameListener( this );
}

void FoundGameListItem::SetSweep( bool sweep )
{
	m_sweep = sweep;
}

bool FoundGameListItem::IsSweep() const
{
	return m_sweep;
}

void FoundGameListItem::SetGameIndex( const Info& fi )
{
	m_FullInfo = fi;

	SetGamePing( fi.m_ePingCategory );

	if ( fi.m_iPingMS > 0 )
		SetControlString( "LblPing", VarArgs( "%d", fi.m_iPingMS ) );

	if ( IsPC() )
		SetControlVisible( "ImgPingSmall", ( fi.m_iPingMS > 0 ) );

	SetGamerTag( fi.m_wszLobbyDisplayName );

	vgui::ImagePanel *imgAvatar = dynamic_cast< vgui::ImagePanel * >( FindChildByName( "PnlGamerPic" ) );
	if ( imgAvatar )
	{
		SetControlVisible( "ImgAvatarBG", false );

		imgAvatar->SetVisible( true );
		switch ( fi.m_Type )
		{
		case TYPE_LOBBY:
			imgAvatar->SetImage( "icon_lobby" );
			break;
		case TYPE_SERVER:
			imgAvatar->SetImage( "icon_server" );
			break;
		case TYPE_LANSERVER:
			imgAvatar->SetImage( "icon_lan" );
			break;
		case TYPE_FAVORITESERVER:
			imgAvatar->SetImage( "icon_server_favorite" );
			break;
		case TYPE_INSECURESERVER:
			imgAvatar->SetImage( "swarm/ticker/warning" );
			break;
		case TYPE_RANKEDSERVER:
			imgAvatar->SetImage( "icon_server_ranked" );
			break;
		default:
			imgAvatar->SetVisible( false );
			break;
		}
	}

	if ( char const *szOtherTitle = fi.IsOtherTitle() )
	{
		if( m_pLblNotJoinable )
		{
			wchar_t convertedString[64];
			g_pVGuiLocalize->ConvertANSIToUnicode( szOtherTitle, convertedString, sizeof( convertedString ) );
			wchar_t finalString[128];
			g_pVGuiLocalize->ConstructString( finalString, sizeof( finalString ), g_pVGuiLocalize->Find( "#L4D360UI_Not_Joinable_Mod" ), 1, convertedString );

			m_pLblNotJoinable->SetText( finalString ); 
		}
	}
	else if ( fi.IsJoinable() || fi.IsDownloadable() )
	{
		SetGamePlayerCount( fi.m_iCurPlayers, fi.m_iMaxPlayers );

		char buf[64];
		V_snprintf( buf, sizeof( buf ), "#L4D360UI_Difficulty_%s_%s", s_szGameDifficultyNames[fi.m_eGameDifficulty], "campaign" );
		SetGameDifficulty( buf );
		SetGameChallenge( fi.m_wszChallengeDisplayName );

		V_snprintf( buf, sizeof( buf ), "#L4D360UI_%s", s_szGameStateNames[fi.m_eGameState] );
		SetSwarmState( buf );
	}
	else
	{
		if ( m_pLblNotJoinable )
		{
			char const *szHint = fi.GetNonJoinableShortHint( true );
			if ( !szHint || !*szHint )
				szHint = "#L4D360UI_Lobby_NotInJoinableGame";

			m_pLblNotJoinable->SetText( szHint );
		}
	}
}

//=============================================================================
const FoundGameListItem::Info& FoundGameListItem::GetFullInfo()
{
	return m_FullInfo;
}

//=============================================================================
void FoundGameListItem::SetGamerTag( const wchar_t *gamerTag )
{
	if ( m_pLblPlayerGamerTag )
	{
		m_pLblPlayerGamerTag->SetText( gamerTag ? gamerTag : L"" );
	}
}

//=============================================================================
void FoundGameListItem::SetGamePing( Info::GAME_PING ping )
{
	if ( m_pImgPingSmall )
	{
		switch ( ping )
		{
		case Info::PING_LOW:
			m_pImgPingSmall->SetImage( "icon_con_high" );
			break;

		case Info::PING_MEDIUM:
			m_pImgPingSmall->SetImage( "icon_con_medium" );
			break;

		case Info::PING_HIGH:
			m_pImgPingSmall->SetImage( "icon_con_low" );
			break;

		case Info::PING_SYSTEMLINK:
			m_pImgPingSmall->SetImage( "icon_lan" );
			break;

		case Info::PING_NONE:
			m_pImgPingSmall->SetImage( "" );
			break;
		}
	}
}

//=============================================================================
void FoundGameListItem::SetGameDifficulty( const char *difficultyName )
{
	if( m_pLblDifficulty )
	{
		m_pLblDifficulty->SetText( difficultyName ? difficultyName : "" );
	}
}

//=============================================================================
void FoundGameListItem::SetGameChallenge( const wchar_t *challengeName )
{
	V_wcsncpy( m_wszChallengeName, challengeName, sizeof( m_wszChallengeName ) );
	if ( m_pLblChallenge )
	{
		m_pLblChallenge->SetText( m_wszChallengeName );
	}
}

//=============================================================================
void FoundGameListItem::SetSwarmState( const char *szSwarmStateText )
{
	if( m_pLblSwarmState )
	{
		m_pLblSwarmState->SetText( szSwarmStateText ? szSwarmStateText : "" );
	}
}

//=============================================================================
void FoundGameListItem::SetGamePlayerCount( int current, int max )
{
	if ( m_pLblPlayers )
	{
		if ( current >= 0 && max != 0 )
		{
			char countText[256];
			V_snprintf( countText, 256, "%d/%d", current, max );
			m_pLblPlayers->SetText( countText );
		}
		else
		{
			m_pLblPlayers->SetText( "" );
		}
	}
}

//=============================================================================
void FoundGameListItem::DrawListItemLabel( vgui::Label* label, bool bSmallFont, bool bEastAligned /* = false */ )
{
	int panelTall = m_iBaseTall;

	if ( label )
	{
		bool bHasFocus = HasFocus() || HasMouseover() || IsSelected();

		Color col( 100, 100, 100, 255 );
		if ( bHasFocus )
		{
			col.SetColor( 255, 255, 255, 255 );
		}
		if ( m_wszChallengeName[0] )
		{
			col.SetColor( col.r(), col.g(), 0, col.a() );
		}

		int x, y;
		wchar_t szUnicode[512];
		
		label->GetText( szUnicode, sizeof( szUnicode ) );
		label->GetPos( x, y );

		HFont drawFont = ( bSmallFont ) ? ( m_hSmallTextFont ) : ( m_hTextFont );
		HFont blurFont = ( bSmallFont ) ? ( m_hSmallTextBlurFont ) : ( m_hTextBlurFont );

		int len = V_wcslen( szUnicode );
		int textWide, textTall;
		surface()->GetTextSize( drawFont, szUnicode, textWide, textTall );	// this is just ballpark numbers as they don't count & characters

		// If we drew labels properly I wouldn't be here on a saturday writing code like this:
		// Cannot ask surface about whole text size as it will skip & characters that can be
		// in player names
		int labelWide = label->GetWide();
		if ( labelWide > 0 )
		{
			textWide = 0;
			HFont wideFont = bHasFocus ? blurFont : drawFont;
			for ( int i=0;i<len;i++ )
			{
				textWide += surface()->GetCharacterWidth( wideFont, szUnicode[i] );

				if ( textWide > labelWide )
				{
					int dotWide = surface()->GetCharacterWidth( wideFont, '.' );
					for ( int k = 3; k -- > 0; )
					{
						if ( i > k )
						{
							textWide += dotWide - surface()->GetCharacterWidth( wideFont, szUnicode[i-k-1] );
							szUnicode[i-k-1] = '.';
						}
					}
					
					szUnicode[i] = 0;
					len = i;

					break;
				}
			}
		}

		// vertical center
		y = ( panelTall - textTall ) / 2;

		if ( bEastAligned )
		{
			x += labelWide - textWide;
		}

		vgui::surface()->DrawSetTextFont( drawFont );
		vgui::surface()->DrawSetTextPos( x, y );
		vgui::surface()->DrawSetTextColor( col );
		vgui::surface()->DrawPrintText( szUnicode, len );

		if ( bHasFocus )
		{
			// draw glow
			int alpha = 60.0f + 30.0f * sin( Plat_FloatTime() * 4.0f );
			vgui::surface()->DrawSetTextColor( Color( 255, 255, 255, alpha ) );
			vgui::surface()->DrawSetTextFont( blurFont );
			vgui::surface()->DrawSetTextPos( x, y );
			vgui::surface()->DrawPrintText( szUnicode, len );
		}
	}
}

//=============================================================================
void FoundGameListItem::PaintBackground()
{
	if ( !m_pListCtrlr->IsPanelItemVisible( this ) )
		return;

	// if we're hilighted, background
	if ( IsSelected() )
	{
		int y;
		int x;
		GetPos( x, y );
		int tall = GetTall() * 0.9;
		y = ( GetTall() - tall ) / 2;
		int wide = GetWide();

		// draw border lines
		surface()->DrawSetColor( Color( 65, 74, 96, 255 ) );
		surface()->DrawFilledRectFade( x, y, x + 0.5f * wide, y+2, 0, 255, true );
		surface()->DrawFilledRectFade( x + 0.5f * wide, y, x + wide, y+2, 255, 0, true );
		surface()->DrawFilledRectFade( x, y+tall-2, x + 0.5f * wide, y+tall, 0, 255, true );
		surface()->DrawFilledRectFade( x + 0.5f * wide, y+tall-2, x + wide, y+tall, 255, 0, true );

		int blotchWide = GetWide();
		int blotchX = 0;
		surface()->DrawFilledRectFade( blotchX, y, blotchX + 0.25f * blotchWide, y+tall, 0, 150, true );
		surface()->DrawFilledRectFade( blotchX + 0.25f * blotchWide, y, blotchX + blotchWide, y+tall, 150, 0, true );
	}

	DrawListItemLabel( m_pLblPing, true, !IsPC() );

	DrawListItemLabel( m_pLblPlayerGamerTag, true );

	// Depending on the game info different labels get rendered in the list
	const Info &fi = m_FullInfo;

	if ( fi.IsJoinable() || fi.IsDownloadable() )
	{
		DrawListItemLabel( m_pLblDifficulty, true );
		DrawListItemLabel( m_pLblChallenge, true );
		DrawListItemLabel( m_pLblPlayers, true );
		DrawListItemLabel( m_pLblSwarmState, true );
	}
	else
	{
		DrawListItemLabel( m_pLblNotJoinable, true );
	}

	if ( m_FullInfo.m_Friends.Count() )
	{
		int wide = GetWide();
		int iFriendLabelX = wide;
		int iFriendLabelY = m_iBaseTall - YRES( 12 );
		int iFriendLabelXOffset, discard;
		m_pLblPlayerGamerTag->GetPos( iFriendLabelXOffset, discard );
		Color FriendColor = HasFocus() || HasMouseover() || IsSelected() ? Color( 255, 255, 255, 255 ) : Color( 100, 100, 100, 255 );
		if ( ISteamFriends *pSteamFriends = SteamFriends() )
		{
			vgui::surface()->DrawSetTextFont( m_hTextFont );
			vgui::surface()->DrawSetTextColor( FriendColor );

			FOR_EACH_VEC( m_FullInfo.m_Friends, i )
			{
				wchar_t wszFriendName[k_cwchPersonaNameMax];
				V_UTF8ToUnicode( pSteamFriends->GetFriendPersonaName( m_FullInfo.m_Friends[i] ), wszFriendName, sizeof( wszFriendName ) );

				int w, t;
				vgui::surface()->GetTextSize( m_hTextFont, wszFriendName, w, t );
				int iFriendLabelWide = YRES( 16 ) + w;
				if ( iFriendLabelX + iFriendLabelWide > wide )
				{
					iFriendLabelY += YRES( 10 );
					iFriendLabelX = iFriendLabelXOffset;
				}

				vgui::IImage *pAvatarImage = CUIGameData::Get()->GetAvatarImage( m_FullInfo.m_Friends[i].ConvertToUint64() );
				if ( pAvatarImage )
				{
					pAvatarImage->SetPos( iFriendLabelX + YRES( 2 ), iFriendLabelY );
					pAvatarImage->SetSize( YRES( 10 ), YRES( 10 ) );
					pAvatarImage->SetColor( FriendColor );
					pAvatarImage->Paint();
				}

				vgui::surface()->DrawSetTextPos( iFriendLabelX + YRES( 14 ), iFriendLabelY );
				vgui::surface()->DrawUnicodeString( wszFriendName );
				iFriendLabelX += iFriendLabelWide;
			}
		}
	}
}

//=============================================================================
void FoundGameListItem::CmdJoinGame()
{
	CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );

	if ( m_FullInfo.m_LobbyID.IsLobby() )
	{
		UTIL_RD_JoinByLobbyID( m_FullInfo.m_LobbyID );
	}
	else
	{
		engine->ClientCmd_Unrestricted( VarArgs( "connect %s\n", m_FullInfo.m_ServerIP.GetConnectionAddressString() ) );
	}
}

//=============================================================================
void FoundGameListItem::OnKeyCodePressed( KeyCode code )
{
	int iUserSlot = GetJoystickForCode( code );
	CBaseModPanel::GetSingleton().SetLastActiveUserId( iUserSlot );

	switch( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_A:
	case KEY_ENTER:
		CmdJoinGame();
		break;

	case KEY_XSTICK1_RIGHT:
	case KEY_XSTICK2_RIGHT:
	case KEY_XBUTTON_RIGHT:
	case KEY_RIGHT:
		if( m_pListCtrlr->NavigateRight() )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );
			NavigateFrom();
		}
		break;

	case KEY_XSTICK1_LEFT:
	case KEY_XSTICK2_LEFT:
	case KEY_XBUTTON_LEFT:
	case KEY_LEFT:
		if( m_pListCtrlr->NavigateLeft() )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );
			NavigateFrom();
		}
		break;

	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

void FoundGameListItem::OnKeyCodeTyped( vgui::KeyCode code )
{
	switch( code )
	{
	case KEY_TAB:
		if( m_pListCtrlr->NavigateDown() )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_FOCUS );
			NavigateFrom();
		}
		break;

	default:
		BaseClass::OnKeyCodeTyped( code );
		break;
	}
}

//=============================================================================
void FoundGameListItem::OnMousePressed( vgui::MouseCode code )
{
	FlyoutMenu::CloseActiveMenu();
	switch ( code )
	{
	case MOUSE_LEFT:
		m_pListCtrlr->SelectPanelItemByPanel( this );
		break;
	}
	BaseClass::OnMousePressed( code );
}

bool FoundGameListItem::IsHardcoreDifficulty()
{
	return m_FullInfo.m_eGameDifficulty == m_FullInfo.DIFFICULTY_INSANE ||
		m_FullInfo.m_eGameDifficulty == m_FullInfo.DIFFICULTY_BRUTAL;
}

//=============================================================================
void FoundGameListItem::ApplySettings( KeyValues *inResourceData )
{
	BaseClass::ApplySettings( inResourceData );
}

//=============================================================================
void FoundGameListItem::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( CFmtStr( "Resource/UI/BaseModUI/%s.res", GetName() ) );

	m_iBaseTall = GetTall();

	//////////////////////////////////////////////////////
	// !!!!! THE ENABLED STATES CONTROL VISIBILITY !!!! //
	//////////////////////////////////////////////////////
	// We paint the controls ourselves to achieve the new look, so they must be non-visible to allow that	
	// Toggle the enabled to make them draw/notdraw

	m_pImgPing = dynamic_cast< vgui::ImagePanel * > ( FindChildByName( "ImgPing" ) );
	if ( m_pImgPing )
	{
		m_pImgPing->SetVisible( false );
	}

	// this one is drawn normally
	m_pImgPingSmall = dynamic_cast< vgui::ImagePanel * > ( FindChildByName( "ImgPingSmall" ) );

	m_pLblPing = dynamic_cast< vgui::Label * > ( FindChildByName( "LblPing" ) );
	if ( m_pLblPing )
	{
		m_pLblPing->SetVisible( false );
	}

	m_pLblPlayerGamerTag = dynamic_cast< vgui::Label * > ( FindChildByName( "LblGamerTag" ) );
	if ( m_pLblPlayerGamerTag )
	{
		m_pLblPlayerGamerTag->SetVisible( false );
	}
	m_pLblDifficulty = dynamic_cast< vgui::Label * > ( FindChildByName( "LblDifficulty" ) );
	if ( m_pLblDifficulty )
	{
		m_pLblDifficulty->SetVisible( false );
	}
	m_pLblChallenge = dynamic_cast< vgui::Label * > ( FindChildByName( "LblChallenge" ) );
	if ( m_pLblChallenge )
	{
		m_pLblChallenge->SetVisible( false );
	}
	m_pLblSwarmState = dynamic_cast< vgui::Label * > ( FindChildByName( "LblSwarmState" ) );
	if ( m_pLblSwarmState )
	{
		m_pLblSwarmState->SetVisible( false );
	}
	m_pLblPlayers = dynamic_cast< vgui::Label * > ( FindChildByName( "LblNumPlayers" ) );
	if ( m_pLblPlayers )
	{
		m_pLblPlayers->SetVisible( false );
	}

	m_pLblNotJoinable = dynamic_cast< vgui::Label * > ( FindChildByName( "LblNotJoinable" ) );
	if ( m_pLblNotJoinable )
	{
		m_pLblNotJoinable->SetVisible( false );
	}
	
	m_hTextFont = pScheme->GetFont( "Default", true );
	m_hTextBlurFont = pScheme->GetFont( "DefaultBlur", true );

	m_hSmallTextFont = pScheme->GetFont( "DefaultMedium", true );
	m_hSmallTextBlurFont = pScheme->GetFont( "DefaultMediumBlur", true );

	if ( m_FullInfo.m_Type != TYPE_UNKNOWN )
	{
		// Parse our own info again now that we have controls
		SetGameIndex( m_FullInfo );
	}
}

//=============================================================================
void FoundGameListItem::PerformLayout()
{
	BaseClass::PerformLayout();

	// set all our children (image panel and labels) to not accept mouse input so they
	// don't eat any mouse input and it all goes to us
	for ( int i = 0; i < GetChildCount(); i++ )
	{
		Panel *panel = GetChild( i );
		Assert( panel );
		panel->SetMouseInputEnabled( false );
	}

	int wide = GetWide();
	int iFriendLabelX = wide;
	int iFriendLabelTall = m_FullInfo.m_Friends.Count() ? YRES( 2 ) : 0;
	int iFriendLabelXOffset, discard;
	m_pLblPlayerGamerTag->GetPos( iFriendLabelXOffset, discard );
	if ( ISteamFriends *pSteamFriends = SteamFriends() )
	{
		FOR_EACH_VEC( m_FullInfo.m_Friends, i )
		{
			wchar_t wszFriendName[k_cwchPersonaNameMax];
			V_UTF8ToUnicode( pSteamFriends->GetFriendPersonaName( m_FullInfo.m_Friends[i] ), wszFriendName, sizeof( wszFriendName ) );

			int w, t;
			vgui::surface()->GetTextSize( m_hTextFont, wszFriendName, w, t );
			int iFriendLabelWide = YRES( 16 ) + w;
			if ( iFriendLabelX + iFriendLabelWide > wide )
			{
				iFriendLabelTall += YRES( 10 );
				iFriendLabelX = iFriendLabelXOffset;
			}
			iFriendLabelX += iFriendLabelWide;
		}
	}

	SetTall( m_iBaseTall + iFriendLabelTall );
}

//=============================================================================
void FoundGameListItem::OnCursorEntered() 
{ 
	if( GetParent() )
	{
		GetParent()->NavigateToChild( this );
	}
	else
	{
		NavigateTo();
	}
}

void FoundGameListItem::NavigateTo( void )
{
	SetHasMouseover( true );
	RequestFocus();

	BaseClass::NavigateTo();
}

void FoundGameListItem::NavigateFrom( void )
{
	SetHasMouseover( false );

	BaseClass::NavigateFrom();
}

//=============================================================================
//
//=============================================================================

//=============================================================================
FoundGames::FoundGames( Panel *parent, const char *panelName ):
	BaseClass( parent, panelName, true, true, false ),
	m_flSearchStartedTime( 0.0f ), m_flSearchEndTime( 0.0f ),
	m_pDataSettings( NULL ),
	m_pPreviousSelectedItem( NULL ),
	m_flScheduledUpdateGameDetails( 0.0f )
{
	SetProportional( true );
	SetPaintBackgroundEnabled( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( true );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 80, 315 );

	m_pTitle = new vgui::Label( this, "Title", "" );
	m_pSubTitle = new vgui::Label( this, "SubTitle", "" );

	m_GplGames = new GenericPanelList( this, "GplGames", GenericPanelList::ISM_PERITEM );
	m_GplGames->SetPaintBackgroundEnabled( true );

	m_LastEngineSpinnerTime = 0.0f;
	m_CurrentSpinnerValue = 0;

	SetLowerGarnishEnabled( true );

	OnItemSelected( NULL );

	SetDeleteSelfOnClose( true );
}

//=============================================================================
FoundGames::~FoundGames()
{
	g_pMatchFramework->GetEventsSubscription()->Unsubscribe( this );

	if ( m_pDataSettings )
		m_pDataSettings->deleteThis();
	m_pDataSettings = NULL;

	delete m_GplGames;

	RemoveFrameListener( this );
}

//=============================================================================
void FoundGames::Activate()
{
	BaseClass::Activate();

	m_bShowHardcoreDifficulties = true;

	AddFrameListener( this );

	UpdateGameDetails();
	m_GplGames->NavigateTo();

	if ( BaseModHybridButton *pWndCreateGame = dynamic_cast< BaseModHybridButton * >( FindChildByName( "DrpCreateGame" ) ) )
	{
		pWndCreateGame->SetVisible( CanCreateGame() );
		pWndCreateGame->SetText( CFmtStr( "#L4D360UI_FoudGames_CreateNew_%s", "campaign" ) );
	}
	if ( CNB_Button *pWndCreateGame = dynamic_cast< CNB_Button * >( FindChildByName( "BtnCreateNewGame" ) ) )
	{
		pWndCreateGame->SetVisible( CanCreateGame() );
		pWndCreateGame->SetText( CFmtStr( "#L4D360UI_FoudGames_CreateNew_%s", "campaign" ) );
		pWndCreateGame->SetControllerButton( KEY_XBUTTON_X );
	}

	if ( Panel *pLabelX = FindChildByName( "LblPressX" ) )
		pLabelX->SetVisible( CanCreateGame() );
}

//=============================================================================
void FoundGames::OnCommand( const char *command )
{
	if( V_strcmp( command, "Back" ) == 0 )
	{
		// Act as though 360 B button was pressed
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else if( V_strcmp( command, "CreateGame" ) == 0 )
	{
		if ( !CanCreateGame() )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_INVALID );
			return;
		}

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
				" network LIVE "
				" access friends "
			" } "
			" game { "
				" mode = "
				" campaign = "
				" mission = "
			" } "
			" options { "
				" action create "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		char const *szGameMode = "campaign";
		pSettings->SetString( "game/mode", szGameMode );
		pSettings->SetString( "game/campaign", "jacob" );
		pSettings->SetString( "game/mission", "asi-jac1-landingbay_01" );

		if ( !CUIGameData::Get()->SignedInToLive() )
		{
			pSettings->SetString( "system/network", "lan" );
			pSettings->SetString( "system/access", "public" );
		}

		if ( StringHasPrefix( szGameMode, "team" ) )
		{
			pSettings->SetString( "system/netflag", "teamlobby" );
		}
		else if ( !Q_stricmp( "custommatch", m_pDataSettings->GetString( "options/action", "" ) ) )
		{
			pSettings->SetString( "system/access", "public" );
		}

		// TCR: We need to respect the default difficulty
		pSettings->SetString( "game/difficulty", GameModeGetDefaultDifficulty( szGameMode ) );

		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
		CBaseModPanel::GetSingleton().CloseAllWindows();
		CBaseModPanel::GetSingleton().OpenWindow( WT_GAMESETTINGS, NULL, true, pSettings );
	}
	else if ( V_strcmp( command, "JoinSelected" ) == 0 )
	{
		FoundGameListItem *pSelectedItem = 	static_cast< FoundGameListItem * >( m_GplGames->GetSelectedPanelItem() );
		if ( pSelectedItem )
		{
			if ( m_bShowHardcoreDifficulties || !pSelectedItem->IsHardcoreDifficulty() )
			{
				PostMessage( pSelectedItem, new KeyValues( "JoinGame" ) );
			}
			else
			{
				CBaseModPanel::GetSingleton().PlayUISound( UISOUND_DENY );

				CBaseModFrame *pWaitScreen = CBaseModPanel::GetSingleton().GetWindow( WT_GENERICWAITSCREEN );
				if ( pWaitScreen )
				{
					return;
				}

				CUIGameData::Get()->OpenWaitScreen( "#rd_reach_level_to_unlock_public_difficulty" );
				CUIGameData::Get()->CloseWaitScreen( NULL, NULL );
			}
		}
	}
	else if ( !V_strcmp( command, "DownloadSelected" ) || !V_strcmp( command, "Website" ) )
	{
		FoundGameListItem *pSelectedItem = 	static_cast< FoundGameListItem * >( m_GplGames->GetSelectedPanelItem() );
		if ( pSelectedItem )
		{
			const FoundGameListItem::Info &info = pSelectedItem->GetFullInfo();

			// Open the download window
			if ( info.m_iMissionWorkshopID != k_PublishedFileIdInvalid && !V_strcmp( command, "DownloadSelected" ) )
			{
				g_ReactiveDropWorkshop.OpenWorkshopPageForFile( info.m_iMissionWorkshopID );

				return;
			}

			KeyValues *pSelectedDetails = new KeyValues( "settings" );
			pSelectedDetails->SetWString( "game/missioninfo/displaytitle", info.m_wszMissionDisplayName );
			pSelectedDetails->SetWString( "game/missioninfo/author", info.m_wszAuthorName );
			pSelectedDetails->SetString( "game/missioninfo/website", info.m_szMissionWebsite );
			pSelectedDetails->SetString( "game/missioninfo/from", "Join" );
			pSelectedDetails->SetString( "game/missioninfo/action", command );
			CBaseModPanel::GetSingleton().OpenWindow( WT_DOWNLOADCAMPAIGN, this, true, pSelectedDetails );
		}
	}
	else if ( V_strcmp( command, "PlayerDropDown" ) == 0 )
	{
		DropDownMenu *pDrpPlayer = dynamic_cast< DropDownMenu * > ( FindChildByName( "DrpSelectedPlayerName" ) );
		if ( pDrpPlayer )
		{
			BaseModHybridButton *pBtnPlayerGamerTag = dynamic_cast< BaseModHybridButton * > ( pDrpPlayer->FindChildByName( "BtnSelectedPlayerName" ) );
			if ( pBtnPlayerGamerTag )
			{
				int x, y;
				pBtnPlayerGamerTag->GetPos( x, y );
				int tall = pBtnPlayerGamerTag->GetTall();
				pBtnPlayerGamerTag->LocalToScreen( x, y );
				OpenPlayerFlyout( pBtnPlayerGamerTag, m_SelectedGamePlayerID, x, y + tall + 1 );
			}
		}
	}
	else if ( V_strcmp( command, "#L4D360UI_SendMessage" ) == 0 )
	{
		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
		CUIGameData::Get()->ExecuteOverlayCommand( "chat", m_SelectedGamePlayerID );
	}
	else if ( V_strcmp( command, "#L4D360UI_ViewSteamID" ) == 0 )
	{
		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
		CUIGameData::Get()->ExecuteOverlayCommand( "steamid", m_SelectedGamePlayerID );
	}
}

void FoundGames::OnEvent( KeyValues *pEvent )
{
	char const *szName = pEvent->GetName();
	
	if ( !Q_stricmp( "OnMatchPlayerMgrUpdate", szName ) )
	{
		char const *szUpdate = pEvent->GetString( "update", "" );
		if ( !Q_stricmp( "searchstarted", szUpdate ) )
		{
			m_flSearchStartedTime = Plat_FloatTime();
			m_flSearchEndTime = m_flSearchStartedTime + ui_foundgames_spinner_time.GetFloat();
			OnThink();
		}
		else if ( !Q_stricmp( "searchfinished", szUpdate ) )
		{
			m_flSearchStartedTime = 0.0f;
			UpdateGameDetails();
		}
		else if ( !Q_stricmp( "friend", szUpdate ) )
		{
			// Friend's game details have been updated
			if ( !m_flScheduledUpdateGameDetails )
				m_flScheduledUpdateGameDetails = Plat_FloatTime();
		}
	}
}

//=============================================================================
void FoundGames::OpenPlayerFlyout( BaseModHybridButton *button, uint64 playerId, int x, int y )
{
#ifdef NO_STEAM
	Error( "FoundGames::OpenPlayerFlyout does not exist on the Xbox 360." );
#else
	if ( playerId == 0 )
		return;

	FlyoutMenu *flyout = NULL;

	CSteamID objSteamId( playerId );
	if ( !objSteamId.IsValid() )
		return;

	if ( !Q_stricmp( "groupserver", m_pDataSettings->GetString( "options/action" ) ) && objSteamId.BClanAccount() )
	{
		flyout = dynamic_cast< FlyoutMenu * >( FindChildByName( "FlmPlayerFlyout_SteamGroup" ) );
	}
	else if ( SteamFriends()->GetFriendRelationship( playerId ) == k_EFriendRelationshipFriend )
	{
		flyout = dynamic_cast< FlyoutMenu * >( FindChildByName( "FlmPlayerFlyout" ) );
	}
	else
	{
		flyout = dynamic_cast< FlyoutMenu * >( FindChildByName( "FlmPlayerFlyout_NotFriend" ) );
	}

	if ( flyout )
	{
		// If one is open for this player, close it
		if ( playerId == m_flyoutPlayerId && flyout->IsVisible() )
		{
			flyout->CloseMenu( button );
			return;
		}

		int wndX, wndY;
		GetPos( wndX, wndY );

		m_flyoutPlayerId = playerId;
		flyout->OpenMenu( button );
		flyout->SetPos( x, y - wndY );
		flyout->SetOriginalTall( 0 );
	}
#endif
}


//=============================================================================
void FoundGames::OnKeyCodePressed( KeyCode code )
{
	int iUserSlot = GetJoystickForCode( code );
	CBaseModPanel::GetSingleton().SetLastActiveUserId( iUserSlot );

	switch( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_X:
		OnCommand( "CreateGame" );
		break;

	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

bool FoundGames::CanCreateGame()
{
	//char const *szGameMode = m_pDataSettings->GetString( "game/mode", NULL );
	bool bGroupServerList = !Q_stricmp( "groupserver", m_pDataSettings->GetString( "options/action", "" ) );

	//return ( szGameMode && *szGameMode && !bGroupServerList );
	return !bGroupServerList;
}

void FoundGames::UpdateTitle()
{
	if ( const char *gameMode = m_pDataSettings->GetString( "game/mode", NULL ) )
	{
		gameMode = NoTeamGameMode( gameMode );
		m_pTitle->SetText( CFmtStr( "#L4D360UI_FoundFriendGames_Title_%s", gameMode ) );
		//BaseClass::DrawDialogBackground( CFmtStr( "#L4D360UI_FoundFriendGames_Title_%s", gameMode ), NULL, "#L4D360UI_FoundGames_Description", NULL, NULL );
	}
	else
	{
		m_pTitle->SetText( CFmtStr( "#L4D360UI_FoundGames_AllGames" ) );
		//BaseClass::DrawDialogBackground( "#L4D360UI_FoundGames_AllGames", NULL, "#L4D360UI_FoundGames_Description", NULL, NULL );
	}
}

//=============================================================================
void FoundGames::OnThink()
{
	BaseClass::OnThink();

	UpdateTitle();

	if ( m_flScheduledUpdateGameDetails && m_flScheduledUpdateGameDetails + ui_foundgames_update_time.GetFloat() < Plat_FloatTime() )
	{
		UpdateGameDetails();
	}

	vgui::ImagePanel *pStillSearchingTag = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "SearchingIcon" ) );
	if ( pStillSearchingTag )
	{
		vgui::Label* label = dynamic_cast< vgui::Label* >( FindChildByName( "LblSearching" ) );
		if ( label )
		{
			label->SetVisible( m_GplGames->GetPanelItemCount() == 0 );
		}

		// If we're searching (or haven't reached the top of the spinner) animate the spinner
		if ( m_flSearchStartedTime > 0.0f || Plat_FloatTime() < m_flSearchEndTime ||
			( m_CurrentSpinnerValue % pStillSearchingTag->GetNumFrames() != 1 ) )
		{
			// clock the anim at 10hz
			float time = Plat_FloatTime();
			if ( ( m_LastEngineSpinnerTime + 0.1f ) < time )
			{
				m_LastEngineSpinnerTime = time;
				pStillSearchingTag->SetFrame( m_CurrentSpinnerValue++ );
			}

			pStillSearchingTag->SetAlpha( 255 );

			if ( label )
				label->SetText( "#L4D360UI_FoundGames_Searching" );
		}
		else
		{
			// Fade
			pStillSearchingTag->SetAlpha( 0 );

			if ( label )
			{
				bool bGroupServer = !Q_stricmp( "groupserver", m_pDataSettings->GetString( "options/action", "" ) );
				char const *szGameMode = m_pDataSettings->GetString( "game/mode", "" );
				szGameMode = NoTeamGameMode( szGameMode );
				
				if ( bGroupServer && szGameMode && *szGameMode )
					label->SetText( CFmtStr( "#L4D360UI_FoundGames_NoGamesFound_%s", szGameMode ) );
				else
					label->SetText( "#L4D360UI_FoundGames_NoGamesFound" );
			}
		}

		if ( m_GplGames->GetPanelItemCount() > 0 && label )
		{
			if ( char const *szNonSearchingCaptionText = GetListHeaderText() )
			{
				label->SetText( szNonSearchingCaptionText );
			}
		}
	}
}

//=============================================================================
void FoundGames::PaintBackground()
{
}

//=============================================================================
void FoundGames::LoadLayout()
{
	BaseClass::LoadLayout();

	// Re-trigger selection to update game details
	OnItemSelected( "" );
}

void FoundGames::SetFoundDesiredText( bool bFoundGame )
{
	vgui::Label* label = dynamic_cast< vgui::Label* >( FindChildByName( "LblNoGamesFound" ) );
	if( label )
	{
		char const *szGameMode = m_pDataSettings->GetString( "game/mode", NULL );
		if ( !szGameMode )
		{
			label->SetVisible( false );
		}
		else
		{
			label->SetVisible( !bFoundGame && m_GplGames->GetPanelItemCount() > 0 );
			
			szGameMode = NoTeamGameMode( szGameMode );
			label->SetText( CFmtStr( "#L4D360UI_FoundGames_NoGamesFound_%s", szGameMode ) );
		}
	}
}

bool FoundGames::AddGameFromDetails( const FoundGameListItem::Info &fi, bool bOnlyMerge )
{
	FoundGameListItem *pGame = NULL;

	for ( int i = 0; i < m_GplGames->GetPanelItemCount(); i++ )
	{
		FoundGameListItem *pItem = assert_cast< FoundGameListItem * >( m_GplGames->GetPanelItem( i ) );
		Assert( pItem );
		if ( !pItem )
			continue;

		if ( IsADuplicateServer( pItem, fi ) )
		{
			pGame = pItem;
			break;
		}
	}

	if ( !pGame && bOnlyMerge )
		return false;

	if ( !pGame )
	{
		pGame = m_GplGames->AddPanelItem<FoundGameListItem>( "FoundGameListItem" );
	}
	else if ( !pGame->IsSweep() )
	{
		return MergeServerEntry( pGame, fi );
	}

	pGame->SetSweep( false );

	FoundGameListItem::Info fiCopy = fi;

	extern ConVar cl_names_debug;
	if ( cl_names_debug.GetInt() )
		V_wcsncpy( fiCopy.m_wszLobbyDisplayName, L"WWWWWWWWWWWWWWW", sizeof( fiCopy.m_wszLobbyDisplayName ) );

	pGame->SetGameIndex( fiCopy );

	return true;
}

bool FoundGames::IsADuplicateServer( FoundGameListItem *item, FoundGameListItem::Info const &fi )
{
	FoundGameListItem::Info const &ii = item->GetFullInfo();

	if ( fi.m_Type == FoundGameListItem::TYPE_LOBBY && fi.m_eServerType == fi.SERVER_LISTEN )
	{
		return fi.m_LobbyID == ii.m_LobbyID;
	}

	Assert( fi.m_Type != FoundGameListItem::TYPE_UNKNOWN );

	return fi.m_ServerIP.GetIP() == ii.m_ServerIP.GetIP() &&
		fi.m_ServerIP.GetConnectionPort() == ii.m_ServerIP.GetConnectionPort();
}

bool FoundGames::MergeServerEntry( FoundGameListItem *item, FoundGameListItem::Info const &fi )
{
	FoundGameListItem::Info info = item->GetFullInfo();
	if ( info.Merge( fi ) )
	{
		item->SetGameIndex( info );
		return true;
	}

	return false;
}

void FoundGames::SetDetailsPanelVisible( bool bIsVisible )
{
	SetControlVisible( "LblCampaign", bIsVisible );
	SetControlVisible( "LblChapter", bIsVisible );
	SetControlVisible( "LblPlayerAccess", bIsVisible );
	SetControlVisible( "LblPlayerAccessText", bIsVisible );
	SetControlVisible( "LblGameDifficulty", bIsVisible );
	SetControlVisible( "LblGameDifficultyText", bIsVisible );
	SetControlVisible( "LblGameChallenge", bIsVisible );
	SetControlVisible( "LblNumPlayers", bIsVisible );
	SetControlVisible( "LblNumPlayersText", bIsVisible );
	SetControlVisible( "LblViewingGames", bIsVisible );
	SetControlVisible( "LblGameStatus", bIsVisible );
	SetControlVisible( "LblSelectedPlayerName", bIsVisible );
	SetControlVisible( "ImgSelectedAvatar", bIsVisible );

	if ( !bIsVisible )
	{
		SetControlVisible( "ImgLevelImage", bIsVisible );
		SetControlVisible( "ImgFrame", bIsVisible );
		SetControlVisible( "BtnDownloadSelected", bIsVisible );
		SetControlVisible( "LblNewVersion", bIsVisible );
		SetControlVisible( "BtnJoinSelected", bIsVisible );
		SetControlVisible( "ImgAvatarBG", bIsVisible );
		SetControlVisible( "DrpSelectedPlayerName", bIsVisible );
		SetControlVisible( "IconForwardArrow", bIsVisible );
	}
}

void FoundGames::AddServersToList()
{
	AddFriendGamesToList();
}

void FoundGames::AddFriendGamesToList( bool bMergeOnly )
{
	ISteamFriends *pFriends = SteamFriends();
	Assert( pFriends );
	if ( !pFriends )
		return;

	const static AppId_t s_iAppID = SteamUtils()->GetAppID();

	int count = pFriends->GetFriendCount( k_EFriendFlagImmediate );
	for ( int i = 0; i < count; i++ )
	{
		CSteamID friendID = pFriends->GetFriendByIndex( i, k_EFriendFlagImmediate );
		FriendGameInfo_t gameInfo;
		if ( !pFriends->GetFriendGamePlayed( friendID, &gameInfo ) )
			continue; // not playing a game
		if ( !gameInfo.m_gameID.IsSteamApp() || gameInfo.m_gameID.AppID() != s_iAppID )
			continue; // not playing AS:RD

		FoundGameListItem::Info info;
		if ( info.SetFromFriend( friendID, gameInfo ) )
			AddGameFromDetails( info, bMergeOnly );
	}
}

void FoundGames::AddFakeServersToList()
{
	for ( int i = 0; i < ui_foundgames_fake_count.GetInt(); i++ )
	{
		int n = i + ui_foundgames_fake_content.GetInt();

		FoundGameListItem::Info fi;
		fi.m_Type = FoundGameListItem::TYPE_LOBBY;

		V_snwprintf( fi.m_wszLobbyDisplayName, NELEMS( fi.m_wszLobbyDisplayName ), L"Fake Content #%02d", n );
		fi.m_iPingMS = ( 123 * i ) % 500 + 5;
		fi.m_ePingCategory = GetPingCategory( fi.m_iPingMS );

		fi.m_iCurPlayers = 1 + n % 3;
		fi.m_iMaxPlayers = 4;
		V_strncpy( fi.m_szMissionFile, "asi-jac2-deima", sizeof( fi.m_szMissionFile ) );
		fi.SetDefaultMissionData();

		AddGameFromDetails( fi );
	}
}

static FoundGames *s_pSortingFoundGames;
static int __cdecl CompareListItem( vgui::Panel *const *a, vgui::Panel *const *b )
{
	const FoundGameListItem::Info &ia = assert_cast< FoundGameListItem * >( *a )->GetFullInfo();
	const FoundGameListItem::Info &ib = assert_cast< FoundGameListItem * >( *b )->GetFullInfo();

	// sort lobbies with friends first
	if ( ia.m_Friends.Count() == 0 && ib.m_Friends.Count() != 0 )
		return 1;
	if ( ia.m_Friends.Count() != 0 && ib.m_Friends.Count() == 0 )
		return -1;

	// if a server is empty, put it lower on the list
	if ( ia.m_iCurPlayers == 0 && ib.m_iCurPlayers != 0 )
		return 1;
	if ( ia.m_iCurPlayers != 0 && ib.m_iCurPlayers == 0 )
		return -1;

	// put full lobbies right above empty ones
	if ( ia.m_iCurPlayers == ia.m_iMaxPlayers && ib.m_iCurPlayers != ib.m_iMaxPlayers )
		return 1;
	if ( ia.m_iCurPlayers != ia.m_iMaxPlayers && ib.m_iCurPlayers == ib.m_iMaxPlayers )
		return -1;

	// if we don't have a map, prefer the lobby where we do have it
	if ( ia.CompareMapVersion() != 0 && ib.CompareMapVersion() == 0 )
		return 1;
	if ( ia.CompareMapVersion() == 0 && ib.CompareMapVersion() != 0 )
		return -1;

	if ( !s_pSortingFoundGames->m_bShowHardcoreDifficulties )
	{
		// if we don't want to join lobbies with hardcore difficulties, show them lower in the list
		if ( ia.m_eGameDifficulty >= ia.DIFFICULTY_INSANE && ib.m_eGameDifficulty < ib.DIFFICULTY_INSANE )
			return 1;
		if ( ia.m_eGameDifficulty < ia.DIFFICULTY_INSANE && ib.m_eGameDifficulty >= ib.DIFFICULTY_INSANE )
			return -1;
	}

	// finally, sort by ping
	if ( ia.m_ePingCategory < ia.PING_SYSTEMLINK && ib.m_ePingCategory < ib.PING_SYSTEMLINK )
	{
		return ia.m_iPingMS - ib.m_iPingMS;
	}

	return 0;
}

void FoundGames::SortListItems()
{
	s_pSortingFoundGames = this;
	m_GplGames->SortPanelItems( CompareListItem );
}

void FoundGames::UpdateGameDetails()
{
	m_flScheduledUpdateGameDetails = 0.0f;
	bool bEmpty = !m_GplGames->GetPanelItemCount();

	// Mark all as stale
	for ( int i = 0; i < m_GplGames->GetPanelItemCount(); ++i )
	{
		FoundGameListItem *game = dynamic_cast< FoundGameListItem * >( m_GplGames->GetPanelItem( i ) );
		if ( game )
		{
			game->SetSweep( true );
		}
	}

	if ( ui_foundgames_fake_content.GetBool() )
		AddFakeServersToList();
	else
		AddServersToList();

	// Remove stale games, it's a vector, so removing an item puts the next item into the current index
	for ( int i = 0; i < m_GplGames->GetPanelItemCount(); /* */ )
	{
		FoundGameListItem *game = dynamic_cast< FoundGameListItem * >( m_GplGames->GetPanelItem( i ) );
		if ( game && game->IsSweep() )
		{
			m_GplGames->RemovePanelItem( static_cast< unsigned short >( i ) );
			continue;
		}
		++i;
	}

	//
	// Sort the list
	//
	SortListItems();

	//
	// Update whether we found the game we were looking for
	//
	if ( m_GplGames->GetPanelItemCount() == 0 )
	{
		SetDetailsPanelVisible( false );
		SetFoundDesiredText( false );
	}
	else
	{
		SetFoundDesiredText( true );
	}

	// Handle adding new item to empty result list
	if ( bEmpty )
		m_GplGames->SelectPanelItem( 0, GenericPanelList::SD_DOWN, true, false );

	// Re-trigger selection to update game details
	OnItemSelected( "" );
}

//=============================================================================
void FoundGames::OnItemSelected( const char *panelName )
{
	if ( !m_bLayoutLoaded )
		return;

	FoundGameListItem *gameListItem = static_cast< FoundGameListItem * >( m_GplGames->GetSelectedPanelItem() );

	bool bChangedSelection = ( gameListItem != m_pPreviousSelectedItem );
	m_pPreviousSelectedItem = gameListItem;

	// Set active state
	for ( int i = 0; i < m_GplGames->GetPanelItemCount(); /* */ )
	{
		FoundGameListItem *pItem = dynamic_cast< FoundGameListItem * >( m_GplGames->GetPanelItem( i ) );

		if ( pItem )
		{
			pItem->SetSelected( pItem == gameListItem );
		}
		++i;
	}

	wchar_t wszCampaignName[256]{};
	TryLocalize( "#L4D360UI_CampaignName_Unknown", wszCampaignName, sizeof( wszCampaignName ) );
	char szDifficulty[64] = "#L4D360UI_Unknown";
	const char *szLobbyAccess = "#L4D360UI_Unknown";
	char szMissionImage[256] = "swarm/MissionPics/UnknownMissionPic";

	wchar_t wszDownloadAuthor[256]{};
	const char *szDownloadWebsite = "";
	bool bDownloadableCampaign = false;

	const int stringSize = 256;
	char playerCountText[stringSize] = "";

	enum DetailsDisplayType_t { DETAILS_NONE, DETAILS_PRESENCE, DETAILS_INGAME };
	DetailsDisplayType_t eDetails = DETAILS_INGAME;

	vgui::Label *lblChapter = dynamic_cast< vgui::Label * >( FindChildByName( "LblChapter" ) );
	vgui::Label *lblGameStatus = dynamic_cast< vgui::Label * >( FindChildByName( "LblGameStatus" ) );
	vgui::Label *lblGameDifficulty = dynamic_cast< vgui::Label * >( FindChildByName( "LblGameDifficulty" ) );
	vgui::Label *lblGameChallenge = dynamic_cast< vgui::Label * >( FindChildByName( "LblGameChallenge" ) );

	CNB_Button *joinButton = dynamic_cast< CNB_Button * >( FindChildByName( "BtnJoinSelected" ) );
	BaseModUI::BaseModHybridButton *downloadButton = dynamic_cast< BaseModUI::BaseModHybridButton * >( FindChildByName( "BtnDownloadSelected" ) );
	vgui::Label *downloadVersionLabel = dynamic_cast< vgui::Label * >( FindChildByName( "LblNewVersion" ) );
	vgui::ImagePanel *imgAvatar = dynamic_cast< vgui::ImagePanel * >( FindChildByName( "ImgSelectedAvatar" ) );
	DropDownMenu *pDrpPlayer = dynamic_cast< DropDownMenu * > ( FindChildByName( "DrpSelectedPlayerName" ) );
	int bBuiltIn = 0;

	if ( gameListItem )
	{
		if ( lblGameStatus )
		{
			lblGameStatus->SetText( "" );
		}

		const FoundGameListItem::Info &fi = gameListItem->GetFullInfo();

		if ( !Q_stricmp( "noplayerinfo", m_pDataSettings->GetString( "UI/display", "" ) ) )
		{
			if ( imgAvatar )
				imgAvatar->SetVisible( false );

			if ( pDrpPlayer )
				pDrpPlayer->SetVisible( false );
		}
		else
		{
			if ( bChangedSelection )
			{
				if ( pDrpPlayer )
				{
					pDrpPlayer->SetVisible( true );

					BaseModHybridButton *pBtnPlayerGamerTag = dynamic_cast< BaseModHybridButton * > ( pDrpPlayer->FindChildByName( "BtnSelectedPlayerName" ) );
					if ( pBtnPlayerGamerTag )
					{
						pBtnPlayerGamerTag->SetText( fi.m_wszLobbyDisplayName );

						FlyoutMenu *flyout = dynamic_cast< FlyoutMenu * >( FindChildByName( "FlmPlayerFlyout" ) );
						if ( flyout && flyout->IsVisible() )
						{
							flyout->CloseMenu( pBtnPlayerGamerTag );
						}

						flyout = dynamic_cast< FlyoutMenu * >( FindChildByName( "FlmPlayerFlyout_NotFriend" ) );
						if ( flyout && flyout->IsVisible() )
						{
							flyout->CloseMenu( pBtnPlayerGamerTag );
						}

						pBtnPlayerGamerTag->SetShowDropDownIndicator( false );
					}
				}
			}

			if ( imgAvatar )
			{
				SetControlVisible( "ImgAvatarBG", false );

				imgAvatar->SetVisible( true );
				switch ( fi.m_Type )
				{
				case FoundGameListItem::TYPE_LOBBY:
					imgAvatar->SetImage( "icon_lobby" );
					break;
				case FoundGameListItem::TYPE_SERVER:
					imgAvatar->SetImage( "icon_server" );
					break;
				case FoundGameListItem::TYPE_LANSERVER:
					imgAvatar->SetImage( "icon_lan" );
					break;
				case FoundGameListItem::TYPE_FAVORITESERVER:
					imgAvatar->SetImage( "icon_server_favorite" );
					break;
				case FoundGameListItem::TYPE_INSECURESERVER:
					imgAvatar->SetImage( "swarm/ticker/warning" );
					break;
				case FoundGameListItem::TYPE_RANKEDSERVER:
					imgAvatar->SetImage( "icon_server_ranked" );
					break;
				default:
					imgAvatar->SetVisible( false );
					break;
				}
			}
		}

		if ( char const *szOtherTitle = fi.IsOtherTitle() )
		{
			wszCampaignName[0] = L'\0';
			szDifficulty[0] = '\0';
			szLobbyAccess = NULL;
			V_strncpy( szMissionImage, "swarm/MissionPics/AddonMissionPic", sizeof( szMissionImage ) );
			if ( lblGameStatus )
				lblGameStatus->SetText( "" );
		}
		else if ( fi.m_Type != FoundGameListItem::TYPE_UNKNOWN )
		{
			const RD_Mission_t *pMission = NULL;
			if ( fi.m_szMissionFile[0] )
			{
				pMission = ReactiveDropMissions::GetMission( fi.m_szMissionFile );
			}

			if ( pMission )
			{
				if ( pMission->Image != NULL_STRING )
				{
					V_strncpy( szMissionImage, STRING( pMission->Image ), sizeof( szMissionImage ) );
				}
				if ( pMission->MissionTitle != NULL_STRING )
				{
					TryLocalize( STRING( pMission->MissionTitle ), wszCampaignName, sizeof( wszCampaignName ) );
				}
				if ( pMission->Author != NULL_STRING )
				{
					TryLocalize( STRING( pMission->Author ), wszDownloadAuthor, sizeof( wszDownloadAuthor ) );
				}
				if ( pMission->Website != NULL_STRING )
				{
					szDownloadWebsite = STRING( pMission->Website );
				}
			}
			else
			{
				bDownloadableCampaign = true;
			}

			if ( fi.m_wszMissionDisplayName[0] != L'\0' )
				V_wcsncpy( wszCampaignName, fi.m_wszMissionDisplayName, sizeof( wszCampaignName ) );
			if ( fi.m_wszAuthorName[0] != L'\0' )
				V_wcsncpy( wszDownloadAuthor, fi.m_wszAuthorName, sizeof( wszDownloadAuthor ) );
			if ( fi.m_szMissionWebsite[0] != '\0' )
				szDownloadWebsite = fi.m_szMissionWebsite;
			bBuiltIn = fi.m_bMissionBuiltIn;

			if ( bBuiltIn )
				szDownloadWebsite = "";	// no website access for builtin campaigns

			V_snprintf( szDifficulty, sizeof( szDifficulty ), "#L4D360UI_Difficulty_%s_%s", s_szGameDifficultyNames[fi.m_eGameDifficulty], "campaign" );

			if ( fi.m_eLobbyAccess == fi.ACCESS_FRIENDS )
				szLobbyAccess = "#L4D360UI_Access_Friends";
			else if ( fi.m_eLobbyAccess == fi.ACCESS_PUBLIC )
				szLobbyAccess = "#L4D360UI_Access_Public";

			if ( !fi.m_iMaxPlayers )
				V_strncpy( playerCountText, "1", sizeof( playerCountText ) );
			else
				V_snprintf( playerCountText, sizeof( playerCountText ), "%d/%d", fi.m_iCurPlayers, fi.m_iMaxPlayers );
		}

		if ( fi.IsOtherTitle() )
		{
			// Don't do anything below
		}
		else if ( fi.IsDownloadable() )
		{
			if ( joinButton )
			{
				joinButton->SetVisible( false );
				SetControlVisible( "IconForwardArrow", false );
			}
			if ( downloadButton )
			{
				downloadButton->SetVisible( true );
				downloadButton->SetEnabled( !!*szDownloadWebsite );
				//downloadButton->SetHelpText( fi.GetJoinButtonHint(), true );
			}
			if ( downloadVersionLabel )
			{
				downloadVersionLabel->SetVisible( !bDownloadableCampaign );
			}
		}
		else if ( joinButton )
		{
			if ( downloadButton )
			{
				downloadButton->SetVisible( false );
			}
			if ( downloadVersionLabel )
			{
				downloadVersionLabel->SetVisible( false );
			}

			bool bGameJoinable = fi.IsJoinable();
			if ( !playerCountText[0] )
				bGameJoinable = false;	// single player games or offline

			joinButton->SetVisible( true );
			joinButton->SetEnabled( bGameJoinable );
			joinButton->SetControllerButton( KEY_XBUTTON_A );
			//joinButton->SetHelpText( fi.GetJoinButtonHint(), bGameJoinable );

			SetControlVisible( "IconForwardArrow", bGameJoinable );
		}
		else
		{
			eDetails = DETAILS_NONE;

			if ( downloadButton )
			{
				downloadButton->SetVisible( false );
			}
			if ( downloadVersionLabel )
			{
				downloadVersionLabel->SetVisible( false );
			}
			if ( joinButton )
			{
				joinButton->SetVisible( false );
			}

			SetControlVisible( "IconForwardArrow", false );
		}
	}

	vgui::Label *lblCampaign = dynamic_cast< vgui::Label * >( FindChildByName( "LblCampaign" ) );
	if ( lblCampaign )
	{
		lblCampaign->SetText( wszCampaignName );
	}

	if ( lblChapter )
	{
		lblChapter->SetText( "" );
	}

	//
	// Image
	//
	if ( szMissionImage[0] != '\0' )
	{
		if ( vgui::ImagePanel *imgLevelImage = dynamic_cast< vgui::ImagePanel * >( FindChildByName( "ImgLevelImage" ) ) )
			imgLevelImage->SetImage( szMissionImage );

		SetControlVisible( "ImgLevelImage", true );
		SetControlVisible( "ImgFrame", true );
	}
	else
	{
		SetControlVisible( "ImgLevelImage", false );
		SetControlVisible( "ImgFrame", false );
	}

	wchar_t finalString[MAX_PATH] = L"";
	wchar_t convertedString[MAX_PATH] = L"";

	vgui::Label *lblAuthor = dynamic_cast< vgui::Label * >( FindChildByName( "LblAuthor" ) );
	if ( lblAuthor )
	{
		finalString[0] = 0;
		if ( wszDownloadAuthor[0] != L'\0' )
		{
			const wchar_t *authorFormat = g_pVGuiLocalize->Find( "#L4D360UI_FoundGames_Author" );
			if ( authorFormat )
			{
				g_pVGuiLocalize->ConstructString( finalString, sizeof( finalString ), authorFormat, 1, wszDownloadAuthor );
			}
		}

		lblAuthor->SetText( finalString );
		lblAuthor->SetVisible( finalString[0] != 0 );


		//lblAuthor->SetText( "test" );
		//lblAuthor->SetFgColor( Color( 255, 255, 255, 255 ) );
		//lblAuthor->SetVisible( true );

		//Msg( "author zpos = %d\n", lblAuthor->GetZPos() );
	}

	BaseModHybridButton *btnWebsite = dynamic_cast< BaseModHybridButton * >( FindChildByName( "BtnWebsite" ) );
	if ( btnWebsite )
	{
		finalString[0] = 0;
		if ( *szDownloadWebsite )
		{
			const wchar_t *websiteFormat = g_pVGuiLocalize->Find( "#L4D360UI_FoundGames_WebSite" );
			g_pVGuiLocalize->ConvertANSIToUnicode( szDownloadWebsite, convertedString, sizeof( convertedString ) );
			if ( websiteFormat )
			{
				g_pVGuiLocalize->ConstructString( finalString, sizeof( finalString ), websiteFormat, 1, convertedString );
			}
		}

		btnWebsite->SetText( finalString );
		btnWebsite->SetVisible( finalString[0] != 0 && ( gameListItem->GetFullInfo().m_iMissionWorkshopID == k_PublishedFileIdInvalid || !SteamUtils() || !SteamUtils()->IsOverlayEnabled() ) );
	}

	vgui::Label *lblAccess = dynamic_cast< vgui::Label * >( FindChildByName( "LblPlayerAccess" ) );
	if ( lblAccess )
	{
		lblAccess->SetText( szLobbyAccess ? szLobbyAccess : "" );
	}

	if ( lblGameDifficulty )
	{
		lblGameDifficulty->SetText( szDifficulty );
	}

	if ( lblGameChallenge )
	{
		lblGameChallenge->SetText( gameListItem ? gameListItem->m_wszChallengeName : L"" );
	}

	vgui::Label *lblNumPlayers = dynamic_cast< vgui::Label * >( FindChildByName( "LblNumPlayers" ) );
	if ( lblNumPlayers && ( eDetails == DETAILS_INGAME ) )
	{
		finalString[0] = 0;
		const wchar_t *playersFormat = g_pVGuiLocalize->Find( "#L4D360UI_FoundGames_Players" );
		if ( playersFormat && playerCountText[0] )
		{
			g_pVGuiLocalize->ConvertANSIToUnicode( playerCountText, convertedString, sizeof( convertedString ) );
			g_pVGuiLocalize->ConstructString( finalString, sizeof( finalString ), playersFormat, 1, convertedString );
		}

		lblNumPlayers->SetText( finalString );
	}
	else if ( lblNumPlayers )
	{
		lblNumPlayers->SetText( "" );
	}

	vgui::Label *lblViewingGames = dynamic_cast< vgui::Label * >( FindChildByName( "LblViewingGames" ) );
	if ( lblViewingGames )
	{
		if ( m_GplGames->GetPanelItemCount() )
		{
			char viewingGames[128];
			int firstItem = m_GplGames->GetFirstVisibleItemNumber() + 1;
			int lastItem = m_GplGames->GetLastVisibleItemNumber() + 1;
			lastItem = MIN( lastItem, m_GplGames->GetPanelItemCount() );
			Q_snprintf( viewingGames, 128, "%d-%d", firstItem, lastItem );
			lblViewingGames->SetVisible( true );
			lblViewingGames->SetText( viewingGames );
		}
		else
		{
			lblViewingGames->SetVisible( false );
		}
	}

	if ( !gameListItem )
	{
		eDetails = DETAILS_NONE;
	}

	// Are we on the "Play Online" menus?
	if ( CBaseModPanel::GetSingleton().GetWindow( WT_FOUNDPUBLICGAMES ) )
	{
		BaseModUI::BaseModHybridButton *m_pInstallSupportBtn = dynamic_cast< BaseModUI::BaseModHybridButton * >( FindChildByName( "BtnInstallSupport" ) );
		vgui::Label *m_pSupportRequiredDetails = dynamic_cast< vgui::Label * >( FindChildByName( "LblSupportRequiredDetails" ) );
		vgui::Label *lblInstalling = dynamic_cast< vgui::Label * >( FindChildByName( "LblInstalling" ) );
		vgui::Label *lblInstallingDetails = dynamic_cast< vgui::Label * >( FindChildByName( "LblInstallingDetails" ) );

		// If a SDK map is selected
		if ( !bBuiltIn )
		{
			// If the Legacy SDK is NOT installed
			if ( !GetLegacyData::IsInstalled() )
			{
				// If we are not already installing it
				if ( !GetLegacyData::IsInstalling() )
				{
					// Hide all buttons and prompt the user to download the SDK
					if ( joinButton && downloadButton && btnWebsite && m_pSupportRequiredDetails && m_pInstallSupportBtn )
					{
						joinButton->SetVisible( false );
						downloadButton->SetVisible( false );
						btnWebsite->SetVisible( false );
						m_pSupportRequiredDetails->SetVisible( true );
						m_pInstallSupportBtn->SetVisible( true );
					}

					SetControlVisible( "IconForwardArrow", false );
				}

			}
			else // The SDK is installed or is installing
			{
				// Hide the SDK install prompt buttons
				if ( m_pSupportRequiredDetails && m_pInstallSupportBtn )
				{
					m_pSupportRequiredDetails->SetVisible( false );
					m_pInstallSupportBtn->SetVisible( false );
				}

				// If we are currently installing, display the install message
				if ( GetLegacyData::IsInstalling() )
				{
					if ( lblInstalling && lblInstallingDetails )
					{
						lblInstalling->SetVisible( true );
						lblInstallingDetails->SetVisible( true );
						//joinButton->SetEnabled( false ); // comment in to force the Join Game button to only be active when the SDK is installed
					}
				}
			}
		}
		else
		{
			// A built-in map is selected. Hide any SDK install prompts
			if ( lblInstalling && lblInstallingDetails && m_pSupportRequiredDetails && m_pInstallSupportBtn )
			{
				lblInstalling->SetVisible( false );
				lblInstallingDetails->SetVisible( false );
				m_pSupportRequiredDetails->SetVisible( false );
				m_pInstallSupportBtn->SetVisible( false );
				//joinButton->SetEnabled( true ); // comment in to force the Join Game button to only be active when the SDK is installed
			}
		}
	}

	SetDetailsPanelVisible( eDetails != DETAILS_NONE );
}

//=============================================================================
void FoundGames::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundEnabled( true );
	SetupAsDialogStyle();

	m_pPreviousSelectedItem = NULL;

	// Subscribe to the matchmaking events
	g_pMatchFramework->GetEventsSubscription()->Subscribe( this );

	Activate();

	if( m_GplGames && m_ActiveControl != m_GplGames )
	{
		if ( m_ActiveControl )
			m_ActiveControl->NavigateFrom();

		m_GplGames->NavigateTo();
		m_ActiveControl = m_GplGames;
	}
}

//=============================================================================
FoundGameListItem* FoundGames::GetGameItem( int index )
{
	FoundGameListItem *result = NULL;
	if( index < m_GplGames->GetPanelItemCount() && index >= 0 )
	{
		result = dynamic_cast< FoundGameListItem* >( m_GplGames->GetPanelItem( index ) );
	}

	return result;
}

//=============================================================================
void FoundGames::OnOpen()
{
	SetVisible( true );

	BaseClass::OnOpen();

	m_GplGames->SetScrollBarVisible( IsPC() );

	// trigger an explicit update
	UpdateGameDetails();
}

void FoundGames::SetDataSettings( KeyValues *pSettings )
{
	if ( m_pDataSettings )
		m_pDataSettings->deleteThis();
	m_pDataSettings = pSettings ? pSettings->MakeCopy() : NULL;

	BaseClass::SetDataSettings( pSettings );
}

//=============================================================================
void FoundGames::OnClose()
{
	BaseClass::OnClose();

	RemoveFrameListener( this );
}

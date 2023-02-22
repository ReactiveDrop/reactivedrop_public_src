#include "cbase.h"
#include "rd_rich_presence.h"

#include "c_asw_player.h"
#include "c_asw_marine.h"
#include "asw_marine_profile.h"
#include "asw_gamerules.h"
#include "rd_lobby_utils.h"
#include "rd_challenges_shared.h"
#include "c_playerresource.h"
#include "asw_briefing.h"
#include "localize/ilocalize.h"
#include "gameui_interface.h"
#include "c_asw_game_resource.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include "c_asw_steamstats.h"
#include "asw_util_shared.h"
#include "inetchannelinfo.h"
#include "rd_missions_shared.h"

#include <ctime>

#include "steam/isteammatchmaking.h"
#include "steam/isteamfriends.h"

#define DISCORD_RICH_PRESENCE_ENABLED	// comment this out to disable Discord's rich presence support in game client
#ifdef DISCORD_RICH_PRESENCE_ENABLED
	#undef INT8_MIN
	#undef INT16_MIN
	#undef INT32_MIN
	#undef INT64_MIN
	#undef INT8_MAX
	#undef INT16_MAX
	#undef INT32_MAX
	#undef INT64_MAX
	#undef UINT8_MAX
	#undef UINT16_MAX
	#undef UINT32_MAX
	#undef UINT64_MAX
	#include "discord_rpc.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

RD_Rich_Presence g_RD_Rich_Presence;

extern ConVar rd_challenge;

#define DISCORD_CLIENT_ID "457248854685777932"
#define DISCORD_STEAM_ID "563560"

#ifdef DISCORD_RICH_PRESENCE_ENABLED
static void handleDiscordReady( const DiscordUser *pRequest )
{
	DevMsg( "Discord: connected to user %s#%s\n", pRequest->username, pRequest->discriminator );
}
static void handleDiscordError( int iErrorCode, const char *pszMessage )
{
	DevWarning( "Discord: error %d: %s\n", iErrorCode, pszMessage );
}
static void handleDiscordJoinGame( const char *pszJoinSecret )
{
	if ( const char *pszLobbyID = StringAfterPrefix( pszJoinSecret, "lobby:" ) )
	{
		char *pEndOfString = NULL;
		uint64 nLobbyID = strtoull( pszLobbyID, &pEndOfString, 16 );
		if ( pEndOfString && pEndOfString[0] )
		{
			Warning( "Discord: could not parse join game message: %s\n", pszJoinSecret );
			return;
		}

		DevMsg( "Discord: connecting to Steam lobby: %llu\n", nLobbyID );
		UTIL_RD_JoinByLobbyID( CSteamID( nLobbyID ) );
	}
	else
	{
		Warning( "Discord: could not parse join game message: %s\n", pszJoinSecret );
	}
}
static void handleDiscordJoinRequest( const DiscordUser *pRequest )
{
	Msg( "Discord: auto-accepting request from %s#%s\n", pRequest->username, pRequest->discriminator );
	Discord_Respond( pRequest->userId, DISCORD_REPLY_YES );
}

#endif

bool IsOfficialCampaign();

bool RD_Rich_Presence::Init()
{
#ifdef DISCORD_RICH_PRESENCE_ENABLED
	DiscordEventHandlers discordHandlers;
	V_memset( &discordHandlers, 0, sizeof( discordHandlers ) );
	discordHandlers.ready = &handleDiscordReady;
	discordHandlers.errored = &handleDiscordError;
	discordHandlers.disconnected = &handleDiscordError;
	discordHandlers.joinGame = &handleDiscordJoinGame;
	discordHandlers.joinRequest = &handleDiscordJoinRequest;
	Discord_Initialize( DISCORD_CLIENT_ID, &discordHandlers, 1, DISCORD_STEAM_ID );
#endif

	return true;
}

void RD_Rich_Presence::Shutdown()
{
	ISteamFriends *pSteamFriends = SteamFriends();
	if ( pSteamFriends )
	{
		pSteamFriends->ClearRichPresence();
	}

#ifdef DISCORD_RICH_PRESENCE_ENABLED
	Discord_ClearPresence();
	Discord_Shutdown();
#endif
}

void RD_Rich_Presence::Update( float frametime )
{
#ifdef DISCORD_RICH_PRESENCE_ENABLED
	Discord_RunCallbacks();
#endif

	if ( m_nLastUpdateTime < time( NULL ) - 5 )
	{
		m_nLastUpdateTime = time( NULL );
		UpdatePresence();
	}
}

void RD_Rich_Presence::UpdatePresence()
{
	ISteamFriends *pSteamFriends = SteamFriends();
	if ( !pSteamFriends )
		return;					// no friends, no fun

#ifdef DISCORD_RICH_PRESENCE_ENABLED
	DiscordRichPresence discordPresence;
	V_memset( &discordPresence, 0, sizeof( discordPresence ) );
#endif

	if ( !engine->IsConnected() )
	{
		pSteamFriends->SetRichPresence( "status", "Main Menu" );
		pSteamFriends->SetRichPresence( "connect", NULL );
		pSteamFriends->SetRichPresence( "steam_display", "#Main_Menu" );
		pSteamFriends->SetRichPresence( "steam_player_group", NULL );
		pSteamFriends->SetRichPresence( "steam_player_group_size", NULL );

#ifdef DISCORD_RICH_PRESENCE_ENABLED
		discordPresence.largeImageKey = "default";
		discordPresence.largeImageText = "Main Menu";
		discordPresence.smallImageKey = "marine_none";
		discordPresence.smallImageText = "Main Menu";
		discordPresence.state = "Main Menu";
#endif
	}
	else if ( engine->IsPlayingDemo() )
	{
		pSteamFriends->SetRichPresence( "status", "Watching a Demo" );
		pSteamFriends->SetRichPresence( "connect", NULL );
		pSteamFriends->SetRichPresence( "steam_display", "#Watching_a_Demo" );
		pSteamFriends->SetRichPresence( "steam_player_group", NULL );
		pSteamFriends->SetRichPresence( "steam_player_group_size", NULL );

#ifdef DISCORD_RICH_PRESENCE_ENABLED
		discordPresence.largeImageKey = "default";
		discordPresence.largeImageText = "Watching a Demo";
		discordPresence.smallImageKey = "marine_none";
		discordPresence.smallImageText = "Watching a Demo";
		discordPresence.state = "Watching a Demo";
#endif
	}
	else
	{
		if ( GameUI().HasLoadingBackgroundDialog() )
		{
			// Don't update during loading screens.
			return;
		}

		CSteamID currentLobby = UTIL_RD_GetCurrentLobbyID();
		if ( currentLobby.IsValid() )
		{
			static char szCurrentLobbyID[17];
			V_snprintf( szCurrentLobbyID, sizeof( szCurrentLobbyID ), "%016llx", currentLobby.ConvertToUint64() );

			static char szConnectString[40];
			V_snprintf( szConnectString, sizeof( szConnectString ), "+connect_lobby %llu", currentLobby.ConvertToUint64() );
			pSteamFriends->SetRichPresence( "steam_player_group", szCurrentLobbyID );
			pSteamFriends->SetRichPresence( "connect", szConnectString );
#ifdef DISCORD_RICH_PRESENCE_ENABLED
			discordPresence.partyId = szCurrentLobbyID;
#endif

			ISteamMatchmaking *pSteamMatchmaking = SteamMatchmaking();
			if ( pSteamMatchmaking )
			{
				int memberCount = pSteamMatchmaking->GetNumLobbyMembers( currentLobby );
				if ( pSteamFriends )
				{
					static char szGroupSize[4];
					V_snprintf( szGroupSize, sizeof( szGroupSize ), "%d", memberCount );
					pSteamFriends->SetRichPresence( "steam_player_group_size", szGroupSize );

					pSteamFriends->SetRichPresence( "num_players", szGroupSize );
					V_snprintf( szGroupSize, sizeof( szGroupSize ), "%d", gpGlobals->maxClients );
					pSteamFriends->SetRichPresence( "max_players", szGroupSize );
				}
#ifdef DISCORD_RICH_PRESENCE_ENABLED
				discordPresence.partySize = memberCount;
				discordPresence.partyMax = gpGlobals->maxClients;
				if ( gpGlobals->maxClients > memberCount )
				{
					static char szJoinSecret[24];
					V_snprintf( szJoinSecret, sizeof( szJoinSecret ), "lobby:%s", szCurrentLobbyID );
					discordPresence.joinSecret = szJoinSecret;
				}
#endif
			}
			else if ( pSteamFriends )
			{
				pSteamFriends->SetRichPresence( "steam_player_group_size", NULL );
			}
		}
		else if ( engine->IsConnected() && ASWGameResource() && ASWGameResource()->IsOfflineGame() )
		{
			// playing Singleplayer mode
#ifdef DISCORD_RICH_PRESENCE_ENABLED
			discordPresence.partyId = NULL;
			discordPresence.partySize = 1;
			discordPresence.partyMax = 1;
			discordPresence.joinSecret = NULL;
#endif

			if ( pSteamFriends )
			{
				pSteamFriends->SetRichPresence( "connect", NULL );
				pSteamFriends->SetRichPresence( "steam_player_group", NULL );
				pSteamFriends->SetRichPresence( "steam_player_group_size", NULL );
				pSteamFriends->SetRichPresence( "num_players", "1" );
				pSteamFriends->SetRichPresence( "max_players", "1" );
			}
		}
		else if ( engine->IsConnected() )
		{
#ifdef DISCORD_RICH_PRESENCE_ENABLED
			discordPresence.partySize = UTIL_ASW_GetNumPlayers();
			discordPresence.partyMax = gpGlobals->maxClients;
			discordPresence.joinSecret = NULL;
#endif

			if ( pSteamFriends )
			{
				INetChannelInfo *nci = engine->GetNetChannelInfo();
				if ( nci )
				{
					const char *pAddr = nci->GetAddress();
					if ( pAddr )
					{
						static char szHostAdress[32];
						V_strncpy( szHostAdress, pAddr, sizeof( szHostAdress ) );

						pSteamFriends->SetRichPresence( "steam_player_group", szHostAdress );
#ifdef DISCORD_RICH_PRESENCE_ENABLED
						discordPresence.partyId = szHostAdress;
#endif 

						static char szConnectString[40];
						V_snprintf( szConnectString, sizeof( szConnectString ), "+connect %s", pAddr );
						pSteamFriends->SetRichPresence( "connect", szConnectString );
					}
				}

				static char szGroupSize[4];
				V_snprintf( szGroupSize, sizeof( szGroupSize ), "%d", UTIL_ASW_GetNumPlayers() );
				pSteamFriends->SetRichPresence( "steam_player_group_size", szGroupSize );

				pSteamFriends->SetRichPresence( "num_players", szGroupSize );
				V_snprintf( szGroupSize, sizeof( szGroupSize ), "%d", gpGlobals->maxClients );
				pSteamFriends->SetRichPresence( "max_players", szGroupSize );
			}
		}

		C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
		C_AlienSwarm *pASW = ASWGameRules();
		if ( pPlayer && pASW )
		{
			static char szSteamDisplay[128];
			static char szDetails[128];
			static char szLargeImageText[128];
			static char szState[128];
			szSteamDisplay[0] = 0;
			szDetails[0] = 0;
			szLargeImageText[0] = 0;
			szState[0] = 0;

			if ( const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( engine->GetLevelNameShort() ) )
			{
				if ( ASWDeathmatchMode() )
				{
					V_strncpy( szSteamDisplay, "#Deathmatch_", sizeof( szSteamDisplay ) );

					switch ( ASWDeathmatchMode()->GetGameMode() )
					{
					case GAMEMODE_DEATHMATCH:
					default:
						V_strcat( szSteamDisplay, "DM", sizeof( szSteamDisplay ) );
						V_strncpy( szDetails, "Deathmatch", sizeof( szDetails ) );
						V_snprintf( szState, sizeof( szState ), "Score: %d", g_PR->GetPlayerScore( pPlayer->entindex() ) );
						break;
					case GAMEMODE_TEAMDEATHMATCH:
						V_strcat( szSteamDisplay, "TDM", sizeof( szSteamDisplay ) );
						V_snprintf( szDetails, sizeof( szDetails ), "Team Deathmatch (%s)", g_PR->GetTeamName( pPlayer->GetTeamNumber() ) );
						V_snprintf( szState, sizeof( szState ), "Score: %d (%d team)", g_PR->GetPlayerScore( pPlayer->entindex() ), g_PR->GetTeamScore( pPlayer->GetTeamNumber() ) );
						break;
					case GAMEMODE_GUNGAME:
						V_strcat( szSteamDisplay, "GG", sizeof( szSteamDisplay ) );
						V_strncpy( szDetails, "Gun Game", sizeof( szDetails ) );
						{
							int iWeaponIndex = ASWDeathmatchMode()->GetWeaponIndexByFragsCount( g_PR->GetPlayerScore( pPlayer->entindex() ) );
							if ( CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( g_ASWEquipmentList.GetRegular( iWeaponIndex )->m_szEquipClass ) )
							{
								char szWeaponName[128];
								if ( wchar_t *pwszTranslatedWeaponName = g_pLocalize->Find( pWeaponInfo->szPrintName ) )
								{
									V_UnicodeToUTF8( pwszTranslatedWeaponName, szWeaponName, sizeof( szWeaponName ) );
								}
								else
								{
									V_strncpy( szWeaponName, pWeaponInfo->szPrintName, sizeof( szWeaponName ) );
								}
								V_snprintf( szState, sizeof( szState ), "Score: %d (%s)", g_PR->GetPlayerScore( pPlayer->entindex() ), szWeaponName );
							}
						}
						break;
					case GAMEMODE_INSTAGIB:
						V_strcat( szSteamDisplay, "IG", sizeof( szSteamDisplay ) );
						V_strncpy( szDetails, "InstaGib", sizeof( szDetails ) );
						V_snprintf( szState, sizeof( szState ), "Score: %d", g_PR->GetPlayerScore( pPlayer->entindex() ) );
						break;
					}
					V_strcat( szSteamDisplay, "_Campaign", sizeof( szSteamDisplay ) );
				}
				else if ( !V_strcmp( rd_challenge.GetString(), "0" ) )
				{
					V_strncpy( szSteamDisplay, "#Campaign_Difficulty", sizeof( szSteamDisplay ) );
				}
				else
				{
					V_strncpy( szSteamDisplay, "#Campaign_Difficulty_Challenge", sizeof( szSteamDisplay ) );
				}

				static char szMissionName[128];
				if ( IsOfficialCampaign() )
				{
					V_snprintf( szMissionName, sizeof( szMissionName ), "#official_mission_%s", pMission->BaseName );
				}
				else if ( wchar_t *pwszTranslatedMissionName = g_pLocalize->Find( STRING( pMission->MissionTitle ) ) )
				{
					V_UnicodeToUTF8( pwszTranslatedMissionName, szMissionName, sizeof( szMissionName ) );
				}
				else
				{
					V_strncpy( szMissionName, STRING( pMission->MissionTitle ), sizeof( szMissionName ) );
				}
				pSteamFriends->SetRichPresence( "rd_mission", szMissionName );
				V_strncpy( szLargeImageText, szMissionName, sizeof( szLargeImageText ) );

				if ( !ASWDeathmatchMode() )
				{
					// set difficulty
					int nSkillLevel = ASWGameRules()->GetSkillLevel();
					const char *szDifficulty = NULL;
					switch ( nSkillLevel )
					{
					case 1: szDifficulty = "Easy"; break;
					default:
					case 2: szDifficulty = "Normal"; break;
					case 3: szDifficulty = "Hard"; break;
					case 4: szDifficulty = "Insane"; break;
					case 5: szDifficulty = "Brutal"; break;
					}
					pSteamFriends->SetRichPresence( "rd_difficulty", szDifficulty );

					// set challenge
					if ( V_strcmp( rd_challenge.GetString(), "0" ) )
					{
						const char *pszDisplayName = ReactiveDropChallenges::DisplayName( rd_challenge.GetString() );
						static char szChallengeName[128];
						if ( ReactiveDropChallenges::IsOfficial( rd_challenge.GetString() ) )
						{
							V_snprintf( szChallengeName, sizeof( szChallengeName ), "#official_challenge_%s", rd_challenge.GetString() );
						}
						else if ( wchar_t *pwszTranslatedChallengeName = g_pLocalize->Find( pszDisplayName ) )
						{
							V_UnicodeToUTF8( pwszTranslatedChallengeName, szChallengeName, sizeof( szChallengeName ) );
						}
						else
						{
							V_strncpy( szChallengeName, pszDisplayName, sizeof( szChallengeName ) );
						}
						pSteamFriends->SetRichPresence( "rd_challenge", szChallengeName );
						V_strncpy( szDetails, szChallengeName, sizeof( szDetails ) );
						V_strcat( szDetails, " (", sizeof( szDetails ) );
						V_strcat( szDetails, szDifficulty, sizeof( szDetails ) );
						V_strcat( szDetails, ")", sizeof( szDetails ) );
					}
					else
					{
						V_strncpy( szDetails, szDifficulty, sizeof( szDetails ) );
					}
				}

				switch ( pASW->GetGameState() )
				{
				default:
				case ASW_GS_NONE:
				case ASW_GS_BRIEFING:
				case ASW_GS_LAUNCHING:
					if ( m_LastState != ASW_GS_BRIEFING )
					{
						m_LastState = ASW_GS_BRIEFING;
						m_nLastStateChangeTime = time( NULL );
					}
					if ( !ASWDeathmatchMode() )
						V_strncpy( szState, "Briefing", sizeof( szState ) );
					break;
				case ASW_GS_INGAME:
					if ( m_LastState != ASW_GS_INGAME )
					{
						m_LastState = ASW_GS_INGAME;
						m_nLastStateChangeTime = time( NULL );
					}
					if ( !ASWDeathmatchMode() )
						V_strncpy( szState, "In Mission", sizeof( szState ) );
					break;
				case ASW_GS_DEBRIEF:
				case ASW_GS_CAMPAIGNMAP:
				case ASW_GS_OUTRO:
					if ( m_LastState != ASW_GS_DEBRIEF )
					{
						m_LastState = ASW_GS_DEBRIEF;
						m_nLastStateChangeTime = time( NULL );
					}
					if ( !ASWDeathmatchMode() )
						V_strncpy( szState, "Debriefing", sizeof( szState ) );
					break;
				}
#ifdef DISCORD_RICH_PRESENCE_ENABLED
				discordPresence.startTimestamp = m_nLastStateChangeTime;
#endif
			}
			else
			{
				V_strncpy( szSteamDisplay, "#Generic", sizeof( szSteamDisplay ) );
			}
#ifdef DISCORD_RICH_PRESENCE_ENABLED
			discordPresence.state = szState;
			discordPresence.details = szDetails;
			discordPresence.largeImageText = szLargeImageText;
			if ( g_ASW_Steamstats.IsOfficialCampaign() )
			{
				static char szLargeImageKey[32];
				V_strncpy( szLargeImageKey, MapName(), sizeof( szLargeImageKey ) );
				discordPresence.largeImageKey = V_strlower( szLargeImageKey );
			}
			else
			{
				discordPresence.largeImageKey = "default";
			}
#endif
			pSteamFriends->SetRichPresence( "status", szDetails );
			pSteamFriends->SetRichPresence( "steam_display", szSteamDisplay );

			CASW_Marine_Profile *pProfile = NULL;
			if ( C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pPlayer->GetNPC() ) )
			{
				pProfile = pMarine->GetMarineProfile();
			}
			else if ( pASW->GetGameState() == ASW_GS_BRIEFING )
			{
				if ( IBriefing *pBriefing = Briefing() )
				{
					pProfile = pBriefing->GetMarineProfile( 0 );
				}
			}
#ifdef DISCORD_RICH_PRESENCE_ENABLED
			if ( pProfile )
			{
				static char marineImageKey[32];
				V_snprintf( marineImageKey, sizeof( marineImageKey ), "marine_%s", pProfile->m_PortraitName );
				discordPresence.smallImageKey = V_strlower( marineImageKey );
				static char szMarineName[128];
				if ( wchar_t *pwszMarineName = g_pLocalize->Find( pProfile->m_ShortName ) )
				{
					V_UnicodeToUTF8( pwszMarineName, szMarineName, sizeof( szMarineName ) );
				}
				else
				{
					V_strncpy( szMarineName, pProfile->m_ShortName, sizeof( szMarineName ) );
				}
				discordPresence.smallImageText = szMarineName;
			}
			else
			{
				discordPresence.smallImageKey = "marine_none";
				discordPresence.smallImageText = "No marine selected";
			}
#endif
		}
	}

#ifdef DISCORD_RICH_PRESENCE_ENABLED
	Discord_UpdatePresence( &discordPresence );
#endif
}

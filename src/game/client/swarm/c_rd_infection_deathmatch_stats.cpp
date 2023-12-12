#include "cbase.h"
#include "c_rd_infection_deathmatch_stats.h"
#include "rd_hud_vscript_shared.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"
#include "c_asw_marine_resource.h"
#include "c_asw_game_resource.h"
#include "rd_inventory_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar rd_challenge;

enum InfectionTeam_t
{
	IT_NONE = 0,
	IT_ZOMBIES = 1,
	IT_HUMANS = 2,
};

enum InfectionPlayerType_t
{
	IPT_NONE = 0,
	IPT_ZOMBIE_PRIME = 1,
	IPT_ZOMBIE_CONVERTED = 2,
	IPT_HUMAN = 3,
	IPT_HUMAN_LAST_STAND = 4,
};

void CRD_Infection_Deathmatch_Stats::OnUpdate( C_RD_HUD_VScript *pHUD )
{
	C_ASW_Player *pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
	Assert( pLocalPlayer );
	Assert( ShouldInit( pHUD ) );
	if ( !pLocalPlayer || pHUD->m_hDataEntity.Get() != pLocalPlayer )
		return;

	if ( !m_bStatsRequested && SteamUserStats() )
	{
		SteamUserStats()->RequestCurrentStats();
		m_bStatsRequested = true;
	}

	int iWinState = pHUD->GetInt( 2 );
	int iStartingCountdown = pHUD->GetInt( 5 );
	int iKillCountZombie = pHUD->GetInt( 8 );
	int iKillCountHuman = pHUD->GetInt( 9 );
	int iCurrentPlayerType = pHUD->GetInt( 10 );

	// update kills if we played this round
	if ( m_iLastNonZeroPlayerType != IPT_NONE )
	{
		if ( m_iLastHumanKills > iKillCountHuman )
		{
			Assert( iKillCountHuman == 0 );
		}
		else if ( m_iLastHumanKills < iKillCountHuman )
		{
			Assert( m_iLastNonZeroPlayerType == IPT_HUMAN || m_iLastNonZeroPlayerType == IPT_HUMAN_LAST_STAND );
			m_KillsPending[m_iLastNonZeroPlayerType - 1] += iKillCountHuman - m_iLastHumanKills;
		}
		m_iLastHumanKills = iKillCountHuman;

		if ( m_iLastZombieKills > iKillCountZombie )
		{
			Assert( iKillCountZombie == 0 );
		}
		else if ( m_iLastZombieKills < iKillCountZombie )
		{
			Assert( m_iLastNonZeroPlayerType == IPT_ZOMBIE_PRIME || m_iLastNonZeroPlayerType == IPT_ZOMBIE_CONVERTED );
			m_KillsPending[m_iLastNonZeroPlayerType - 1] += iKillCountZombie - m_iLastZombieKills;
		}
		m_iLastZombieKills = iKillCountZombie;
	}

	// update deaths if the round has started
	if ( m_iLastWinState == IT_NONE && m_iLastNonZeroPlayerType != IPT_NONE && !m_bWasDead && ( iCurrentPlayerType == IPT_NONE || ( iCurrentPlayerType == IPT_ZOMBIE_CONVERTED && m_iLastNonZeroPlayerType == IPT_HUMAN ) ) )
	{
		m_DeathsPending[m_iLastNonZeroPlayerType - 1]++;
		m_bWasDead = iCurrentPlayerType == IPT_NONE;
	}

	if ( m_iLastWinState != iWinState )
	{
		Assert( m_iLastWinState == IT_NONE || iWinState == IT_NONE );

		if ( iWinState != IT_NONE )
		{
			Assert( iWinState == IT_ZOMBIES || iWinState == IT_HUMANS );
			if ( m_iLastNonZeroPlayerType != IPT_NONE )
			{
				// if we were playing and the round ended, regardless of whether we are eligible for stats, check if we need to spawn the medal.
				if ( iCurrentPlayerType == IPT_HUMAN || iCurrentPlayerType == IPT_HUMAN_LAST_STAND || iKillCountZombie > 0 )
					ReactiveDropInventory::AddPromoItem( 43 );

				// record our win or loss in the pending array
				bool bWasInHumanTeam = m_iLastNonZeroPlayerType == IPT_HUMAN || m_iLastNonZeroPlayerType == IPT_HUMAN_LAST_STAND;
				if ( bWasInHumanTeam == ( iWinState == IT_HUMANS ) )
					m_WinsPending[m_iLastNonZeroPlayerType - 1]++;
				else
					m_LossesPending[m_iLastNonZeroPlayerType - 1]++;
			}

			if ( m_bEnoughPlayersThisRound )
			{
				// if we were playing and there were enough players to count the match for stats, record the stats now

				// if we were a human (not using last stand) and the humans won, record a win
				if ( iWinState == IT_HUMANS && iCurrentPlayerType == IPT_HUMAN )
				{
					Assert( m_DeathsPending[IPT_HUMAN] == 0 );
					ReactiveDropInventory::IncrementStrangePropertyOnStartingItems( 43, 1, 0 );
				}
				// if we scored any kills as zombie, record those
				ReactiveDropInventory::IncrementStrangePropertyOnStartingItems( 43, m_KillsPending[IPT_ZOMBIE_PRIME - 1] + m_KillsPending[IPT_ZOMBIE_CONVERTED - 1], 1 );
				// if we killed zombies in last stand mode, record the kills
				ReactiveDropInventory::IncrementStrangePropertyOnStartingItems( 43, m_KillsPending[IPT_HUMAN_LAST_STAND - 1], 2 );

				ReactiveDropInventory::CommitDynamicProperties();

				// store any pending stats before we clear them
				ISteamUserStats *pUserStats = SteamUserStats();
				if ( pUserStats && m_bStatsLoaded )
				{
					bool bAnyStatChanged = false;
#define SAVE_PER_PLAYER_TYPE_STAT( statname, playertypename, statvar, playertypeid ) \
					if ( statvar##Pending[playertypeid - 1] != 0 ) \
					{ \
						statvar##Saved[playertypeid - 1] += statvar##Pending[playertypeid - 1]; \
						pUserStats->SetStat( "infectiondm." statname "." playertypename, statvar##Saved[playertypeid - 1] ); \
						bAnyStatChanged = true; \
					}
#define SAVE_PER_PLAYER_TYPE_STATS( statname, statvar ) \
					SAVE_PER_PLAYER_TYPE_STAT( statname, "prime", statvar, IPT_ZOMBIE_PRIME ) \
					SAVE_PER_PLAYER_TYPE_STAT( statname, "zombie", statvar, IPT_ZOMBIE_CONVERTED ) \
					SAVE_PER_PLAYER_TYPE_STAT( statname, "human", statvar, IPT_HUMAN ) \
					SAVE_PER_PLAYER_TYPE_STAT( statname, "laststand", statvar, IPT_HUMAN_LAST_STAND )

					SAVE_PER_PLAYER_TYPE_STATS( "kills", m_Kills );
					SAVE_PER_PLAYER_TYPE_STATS( "deaths", m_Deaths );
					SAVE_PER_PLAYER_TYPE_STATS( "wins", m_Wins );
					SAVE_PER_PLAYER_TYPE_STATS( "losses", m_Losses );

					if ( bAnyStatChanged )
					{
						pUserStats->StoreStats();
					}
				}
			}

			// reset eligibility for the next round
			m_iLastNonZeroPlayerType = IPT_NONE;
			m_bEnoughPlayersThisRound = false;
			m_iLastZombieKills = 0;
			m_iLastHumanKills = 0;
			V_memset( m_KillsPending, 0, sizeof( m_KillsPending ) );
			V_memset( m_DeathsPending, 0, sizeof( m_DeathsPending ) );
			V_memset( m_WinsPending, 0, sizeof( m_WinsPending ) );
			V_memset( m_LossesPending, 0, sizeof( m_LossesPending ) );
		}
		else
		{
			// we should already be fully reset at this point
			Assert( m_iLastNonZeroPlayerType == IPT_NONE );
			Assert( m_iLastZombieKills == 0 );
			Assert( m_iLastHumanKills == 0 );
			Assert( m_bWasDead );
			Assert( !m_bEnoughPlayersThisRound );
		}
	}

	if ( iWinState == IT_NONE && iStartingCountdown <= 0 )
	{
		if ( iCurrentPlayerType != IPT_NONE && !m_bEnoughPlayersThisRound )
		{
			if ( C_ASW_Game_Resource *pGameResource = ASWGameResource() )
			{
				int iPlayerCount = 0;

				for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
				{
					C_ASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
					if ( pMR && pMR->IsInhabited() )
					{
						iPlayerCount++;
					}
				}

				if ( iPlayerCount >= 5 )
				{
					m_bEnoughPlayersThisRound = true;
				}
			}
		}

		if ( iCurrentPlayerType != IPT_NONE && m_iLastNonZeroPlayerType != iCurrentPlayerType )
		{
			Assert( ( m_iLastNonZeroPlayerType == IPT_NONE && iCurrentPlayerType == IPT_HUMAN ) ||
				( m_iLastNonZeroPlayerType == IPT_HUMAN && iCurrentPlayerType == IPT_ZOMBIE_PRIME ) ||
				( m_iLastNonZeroPlayerType == IPT_HUMAN && iCurrentPlayerType == IPT_ZOMBIE_CONVERTED ) ||
				( m_iLastNonZeroPlayerType == IPT_HUMAN && iCurrentPlayerType == IPT_HUMAN_LAST_STAND ) ||
				( m_iLastNonZeroPlayerType == IPT_HUMAN_LAST_STAND && iCurrentPlayerType == IPT_ZOMBIE_CONVERTED ) );

			m_iLastNonZeroPlayerType = iCurrentPlayerType;
		}

		if ( iCurrentPlayerType != IPT_NONE )
		{
			m_bWasDead = false;
		}
	}
}

void CRD_Infection_Deathmatch_Stats::OnStatsLoaded( UserStatsReceived_t *pParam )
{
	Assert( pParam->m_nGameID == SteamUtils()->GetAppID() );
	if ( pParam->m_nGameID == SteamUtils()->GetAppID() && pParam->m_steamIDUser == SteamUser()->GetSteamID() && pParam->m_eResult == k_EResultOK )
	{
		ISteamUserStats *pUserStats = SteamUserStats();
#define INIT_PER_PLAYER_TYPE_STAT( statname, playertypename, statvar, playertypeid ) \
		if ( !pUserStats->GetStat( "infectiondm." statname "." playertypename, &statvar##Saved[playertypeid - 1] ) ) \
		{ \
			Warning( "Failed to load Steam user stat infectiondm." statname "." playertypename "; setting to 0\n" ); \
			statvar##Saved[playertypeid - 1] = 0; \
		}
#define INIT_PER_PLAYER_TYPE_STATS( statname, statvar ) \
		INIT_PER_PLAYER_TYPE_STAT( statname, "prime", statvar, IPT_ZOMBIE_PRIME ) \
		INIT_PER_PLAYER_TYPE_STAT( statname, "zombie", statvar, IPT_ZOMBIE_CONVERTED ) \
		INIT_PER_PLAYER_TYPE_STAT( statname, "human", statvar, IPT_HUMAN ) \
		INIT_PER_PLAYER_TYPE_STAT( statname, "laststand", statvar, IPT_HUMAN_LAST_STAND )

		INIT_PER_PLAYER_TYPE_STATS( "kills", m_Kills );
		INIT_PER_PLAYER_TYPE_STATS( "deaths", m_Deaths );
		INIT_PER_PLAYER_TYPE_STATS( "wins", m_Wins );
		INIT_PER_PLAYER_TYPE_STATS( "losses", m_Losses );

		m_bStatsLoaded = true;
	}
}

constexpr static const char *const s_szInfectionDeathmatchChallenges[] =
{
	"asw_infection",
	"asw_infection_fp",
	"asw_infection_tp",
	"asw_infectionclassic",
	"asw_infectionclassic_fp",
	"asw_infectionclassic_tp",
};

bool CRD_Infection_Deathmatch_Stats::ShouldInit( C_RD_HUD_VScript *pHUD )
{
	if ( V_strcmp( pHUD->m_szClientVScript.Get(), "challenge_asw_infection_hud.nut" ) )
		return false;

	if ( pHUD->m_hDataEntity.Get() != C_ASW_Player::GetLocalASWPlayer() )
		return false;

	if ( !ASWDeathmatchMode() )
		return false;

	for ( int i = 0; i < NELEMS( s_szInfectionDeathmatchChallenges ); i++ )
	{
		if ( !V_strcmp( rd_challenge.GetString(), s_szInfectionDeathmatchChallenges[i] ) )
			return true;
	}

	return false;
}

#include "cbase.h"

#include "engine/ienginesound.h"
#include "asw_shareddefs.h"
#include "asw_deathmatch_mode.h"
#include "filesystem.h"
#include "asw_gamerules.h"
#ifdef GAME_DLL
#include "asw_spawn_manager.h"
#include "asw_spawn_group.h"
#include "asw_game_resource.h"
#include "asw_marine_resource.h"
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "asw_equipment_list.h"
#include "asw_alien.h"
#include "asw_buzzer.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "logicentities.h"
#include "team.h"
#include "team_spawnpoint.h"
#else
#include "c_asw_marine_resource.h"
#include "c_asw_game_resource.h"
#include "c_playerresource.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Deathmatch_Mode, DT_ASW_Deathmatch_Mode )

BEGIN_NETWORK_TABLE( CASW_Deathmatch_Mode, DT_ASW_Deathmatch_Mode )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iGameMode ) ),
	RecvPropInt( RECVINFO( m_iGunGameWeaponCount ) ),
	RecvPropArray3( RECVINFO_ARRAY( m_iGunGameWeapons ), RecvPropInt( RECVINFO( m_iGunGameWeapons[0] ) ) ),
#else
	SendPropInt( SENDINFO( m_iGameMode ) ),
	SendPropInt( SENDINFO( m_iGunGameWeaponCount ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iGunGameWeapons ), SendPropInt( SENDINFO_ARRAY( m_iGunGameWeapons ) ) ),
#endif
END_NETWORK_TABLE()

#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( asw_deathmatch_mode, CASW_Deathmatch_Mode );

BEGIN_DATADESC( CASW_Deathmatch_Mode )

	DEFINE_THINKFUNC(DeathmatchThink),
	
END_DATADESC()

#endif

CASW_Deathmatch_Mode *g_pDeathmatchMode = NULL;
CASW_Deathmatch_Mode* ASWDeathmatchMode() { return g_pDeathmatchMode; }

extern ConVar asw_blink_charge_time;
extern ConVar asw_cam_marine_dist;
extern ConVar asw_marine_death_cam_slowdown;
extern ConVar asw_marine_names;
extern ConVar asw_marine_speed_scale_easy;
extern ConVar asw_marine_speed_scale_normal;
extern ConVar asw_marine_speed_scale_hard;
extern ConVar asw_marine_speed_scale_insane;
extern ConVar asw_sentry_friendly_fire_scale;
extern ConVar asw_sentry_friendly_target;
extern ConVar asw_world_healthbars;
extern ConVar asw_world_usingbars;
extern ConVar asw_marine_rolls;
extern ConVar asw_vindicator_grenade_fuse;
//extern ConVar rd_deathmatch_ending_time;	not needed to change
extern ConVar rd_default_weapon;
extern ConVar rd_chatter_about_ff;
extern ConVar rd_chatter_about_marine_death;
extern ConVar rd_chainsaw_slows_down;
extern ConVar rd_firemine_target_marine;
extern ConVar rd_frags_limit;
extern ConVar rd_jumpjet_knockdown_marines;
extern ConVar rd_laser_mine_takes_damage;
extern ConVar rd_laser_mine_targets_everything;
extern ConVar rd_marine_chatter_enabled;
extern ConVar rd_marine_ff_fist;
extern ConVar rd_marine_ignite_immediately;
extern ConVar rd_medgun_infinite_ammo;
extern ConVar rd_mininglaser_slows_down;
extern ConVar rd_paint_marine_blips;
extern ConVar rd_paint_scanner_blips;
extern ConVar rd_reassign_marines;
extern ConVar rd_request_experience;
extern ConVar rd_rocket_target_marine;
extern ConVar rd_sentry_take_damage_from_marine;
extern ConVar rd_show_arrow_to_marine;
extern ConVar rd_show_others_laser_pointer;
extern ConVar rd_weapon_on_ground_time;

extern ConVar asw_marine_death_protection;
extern ConVar asw_skill;
extern ConVar asw_marine_ff_absorption;
extern ConVar asw_skill_melee_dmg_base;

extern ConVar rd_shotgun_dmg_base;
extern ConVar rd_rifle_dmg_base;
extern ConVar rd_prifle_dmg_base;
extern ConVar rd_autogun_dmg_base;
extern ConVar rd_pistol_dmg_base;
extern ConVar rd_pdw_dmg_base;
extern ConVar rd_flamer_dmg_base;
extern ConVar rd_minigun_dmg_base;
extern ConVar rd_grenade_launcher_dmg_base;
extern ConVar rd_deagle_dmg_base;
extern ConVar rd_devastator_dmg_base;
extern ConVar rd_medrifle_dmg_base;

ConVar rd_killingspree_time_limit( "rd_killingspree_time_limit", "3",  FCVAR_REPLICATED, "Time in seconds. If player doesn't kill anybody during this time his killing spree is ended");
ConVar rd_quake_sounds( "rd_quake_sounds", "2",  FCVAR_REPLICATED, "Enable or disable quake sounds like doublekill, monsterkill");

template<typename T>
static void SaveSetConvar( ConVar & cvar, T value )
{
	Assert( ASWGameRules() );

	ASWGameRules()->SaveConvar( &cvar );
	cvar.SetValue( value );
}

CASW_Deathmatch_Mode::CASW_Deathmatch_Mode()
{
	Assert( !g_pDeathmatchMode );
	g_pDeathmatchMode = this;

	// apply deathmatch rules
#ifdef GAME_DLL
	SaveSetConvar( asw_cam_marine_dist, 600 );
	SaveSetConvar( asw_blink_charge_time, 5 );	// 5 seconds to reload blink 
	SaveSetConvar( asw_marine_names, 0 );
	SaveSetConvar( asw_marine_death_cam_slowdown, 1 );

	SaveSetConvar( asw_marine_rolls, 0 );

	SaveSetConvar( asw_marine_speed_scale_easy, 1.5f );
	SaveSetConvar( asw_marine_speed_scale_normal, 1.5f );
	SaveSetConvar( asw_marine_speed_scale_hard, 1.5f );
	SaveSetConvar( asw_marine_speed_scale_insane, 1.5f );

	SaveSetConvar( asw_sentry_friendly_fire_scale, 0.1f );
	SaveSetConvar( asw_sentry_friendly_target, 1 );
	SaveSetConvar( asw_skill, 3 ); // 3 stands for Hard
	SaveSetConvar( asw_marine_ff_absorption, 0 );
	SaveSetConvar( rd_mininglaser_slows_down, 0 );
	SaveSetConvar( asw_world_healthbars, 0 );
	SaveSetConvar( asw_world_usingbars, 0 );

	SaveSetConvar( asw_marine_death_protection, 0 ); // disable 1 HP protection
	SaveSetConvar( asw_vindicator_grenade_fuse, 1.0f );

	SaveSetConvar( rd_chatter_about_ff, 0 );
	SaveSetConvar( rd_chatter_about_marine_death, 0 );
	SaveSetConvar( rd_firemine_target_marine, 1 );
	SaveSetConvar( rd_laser_mine_takes_damage, 1 );
	SaveSetConvar( rd_laser_mine_targets_everything, 1 );
	SaveSetConvar( rd_marine_chatter_enabled, 0 );
	SaveSetConvar( rd_marine_ignite_immediately, 1 );
	SaveSetConvar( rd_medgun_infinite_ammo, 1 );
	SaveSetConvar( rd_rocket_target_marine, 1 );
	SaveSetConvar( rd_sentry_take_damage_from_marine, 1 );
	SaveSetConvar( rd_weapon_on_ground_time, 30 );

	SaveSetConvar( rd_chainsaw_slows_down, 0 );
	SaveSetConvar( rd_jumpjet_knockdown_marines, 0 );
	SaveSetConvar( rd_marine_ff_fist, 1 );
	SaveSetConvar( rd_paint_marine_blips, 2 );
	SaveSetConvar( rd_paint_scanner_blips, 0 );
	SaveSetConvar( rd_reassign_marines, 0 );
	SaveSetConvar( rd_request_experience, 0 );
	
	SaveSetConvar( rd_show_arrow_to_marine, 0 );
	SaveSetConvar( rd_show_others_laser_pointer, 0 );

	SaveSetConvar( rd_shotgun_dmg_base, 25 );
	SaveSetConvar( rd_rifle_dmg_base, 5 );
	SaveSetConvar( rd_prifle_dmg_base, 5 );
	SaveSetConvar( rd_autogun_dmg_base, 8 );	// was 7
	SaveSetConvar( rd_pistol_dmg_base, 10 );	// was 14
	SaveSetConvar( rd_pdw_dmg_base, 5 );		// was 7
	SaveSetConvar( rd_flamer_dmg_base, 25 );	// was 2
	SaveSetConvar( rd_minigun_dmg_base, 9 );	// was 7
	SaveSetConvar( rd_grenade_launcher_dmg_base, 40 );	// was 80
	SaveSetConvar( rd_deagle_dmg_base, 22 );	// was 104
	SaveSetConvar( rd_devastator_dmg_base, 15 );
	SaveSetConvar( rd_medrifle_dmg_base, 5 );


	spawn_point = NULL;
	ResetFragsLeftSaid();

	CAI_BaseNPC::SetDefaultFactionRelationship( FACTION_MARINES, FACTION_MARINES, D_HATE, 0 );
#endif	// GAME_DLL 
}

CASW_Deathmatch_Mode::~CASW_Deathmatch_Mode()
{
	Assert( g_pDeathmatchMode == this );
	g_pDeathmatchMode = NULL;

#ifdef GAME_DLL
	// store the current game mode in static variable for map change preserving 
	game_mode_		 = (RdGameModes)m_iGameMode.Get();

	// clear global teams if we have them 
	for ( int i = 0; i < g_Teams.Count(); ++i )
	{
		UTIL_Remove( g_Teams[i] );
	}
	g_Teams.Purge();

	spawn_point = NULL;

	CAI_BaseNPC::SetDefaultFactionRelationship( FACTION_MARINES, FACTION_MARINES, D_LIKE, 0 );

#endif
}

#ifdef GAME_DLL // for server only

RdGameModes CASW_Deathmatch_Mode::game_mode_ = GAMEMODE_DEATHMATCH;
CBaseEntity *CASW_Deathmatch_Mode::spawn_point = NULL;

void CASW_Deathmatch_Mode::OnMissionStart() 
{
	// enable InstaGib game mode after the map was changed
	if ( GAMEMODE_INSTAGIB == game_mode_ )
	{
		InstagibEnable();
	}
	else if ( GAMEMODE_GUNGAME == game_mode_ )
	{
		GunGameEnable();
	}
	else if ( GAMEMODE_TEAMDEATHMATCH == game_mode_ )
	{
		TeamDeathmatchEnable();
	}
	else if ( GAMEMODE_DEATHMATCH == game_mode_)
	{
		// do nothing
	}
	else 
	{
		Warning( "Unknown game mode on mission start \n" );
	}
}

void CASW_Deathmatch_Mode::IncrementFragCount( CASW_Marine_Resource *pMR, int iAmount )
{
	pMR->m_TimelineKillsTotal.RecordValue( iAmount );

	if ( pMR->IsInhabited() )
	{
		CASW_Player *pPlayer = pMR->GetCommander();
		if ( pPlayer )
		{
			pPlayer->IncrementFragCount( iAmount );
		}
	}
	else
	{
		pMR->m_iBotFrags += iAmount;
	}
}

void CASW_Deathmatch_Mode::IncrementDeathCount( CASW_Marine_Resource *pMR, int iAmount )
{
	if ( pMR->IsInhabited() )
	{
		CASW_Player *pPlayer = pMR->GetCommander();
		if ( pPlayer )
		{
			pPlayer->IncrementDeathCount( iAmount );
		}
	}
	else
	{
		m_iBotDeaths[ASWGameResource()->GetMarineResourceIndex( pMR )] += iAmount;
	}
}

void CASW_Deathmatch_Mode::OnMarineKilled( const CTakeDamageInfo &info, CASW_Marine *pMarine )
{
	CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
	if ( !pMR )
		return;

	// don't increment kills if we already finishing the round
	if ( ASWGameRules()->m_fDeathmatchFinishTime != 0.0f )
		return;

	IncrementDeathCount( pMR, 1 );
	KillingSpreeReset( pMR );

	// increment Frags count for killer for deathmatch
	CASW_Marine *pOtherMarine = CASW_Marine::AsMarine( info.GetAttacker() );
	if ( pOtherMarine )
	{
		CASW_Marine_Resource *pOtherMR = pOtherMarine->GetMarineResource();
		// if killed himself decrement his frags count
		if ( pMR == pOtherMR )
		{
			IncrementFragCount( pMR, -1 );
		}
		else if ( IsTeamDeathmatchEnabled() )
		{
			int iOtherTeam = pOtherMR->GetTeamNumber();

			// if killed a teammate in TDM then decrement frags count and team frags count
			if ( pMR->GetTeamNumber() == iOtherTeam )
			{
				IncrementFragCount( pOtherMR, -1 );
				GetGlobalTeam( iOtherTeam )->AddScore( -1 );
			}
			else
			{
				IncrementFragCount( pOtherMR, 1 );
				KillingSpreeIncrease( pOtherMR );

				GetGlobalTeam( iOtherTeam )->AddScore( 1 );
			}
		}
		else
		{
			IncrementFragCount( pOtherMR, 1 );
			KillingSpreeIncrease( pOtherMR );
		}

		int frags_left = rd_frags_limit.GetInt() - GetFragCount( pOtherMR );

		if ( IsGunGameEnabled() )
		{
			Assert( m_iGunGameWeaponCount > 0 );
			frags_left = m_iGunGameWeaponCount - GetFragCount( pOtherMR );
		}
		else if ( GAMEMODE_TEAMDEATHMATCH == GetGameMode() )
		{
			if ( GetGlobalTeam( TEAM_ALPHA ) && GetGlobalTeam( TEAM_BETA ) )
			{
				int team1frags = GetGlobalTeam( TEAM_ALPHA )->GetScore();
				int team2frags = GetGlobalTeam( TEAM_BETA )->GetScore();
				frags_left = rd_frags_limit.GetInt() - MAX( team1frags, team2frags );
			}
			else
			{
				Warning( "Can't find one of the teams TEAM_ALPHA or TEAM_BETA" );
			}
		}

		switch ( frags_left )
		{
			case 0:
				ASWGameRules()->FinishDeathmatchRound( pOtherMR );
				ASWGameRules()->BroadcastSound( "rd_song.round_end_music" );
				break;
			case 1:
				if ( !frags_left_said[1] )
				{
					ASWGameRules()->BroadcastSound( "DM.one_frag_left" );
					frags_left_said[1] = true;
				}
				break;
			case 2:
				if ( !frags_left_said[2] )
				{
					ASWGameRules()->BroadcastSound( "DM.two_frags_left" );
					frags_left_said[2] = true;
				}
				break;
			case 3:
				if ( !frags_left_said[3] )
				{
					ASWGameRules()->BroadcastSound( "DM.three_frags_left" );
					frags_left_said[3] = true;
				}
				break;
			case 4:
				if ( !frags_left_said[4] )
				{
					ASWGameRules()->BroadcastSound( "DM.four_frags_left" );
					frags_left_said[4] = true;
				}
				break;
			case 5:
				if ( !frags_left_said[5] )
				{
					ASWGameRules()->BroadcastSound( "DM.five_frags_left" );
					frags_left_said[5] = true;
				}
				break;
			//default:
				// do nothing
		}

		// remove current weapon and give new one
		if ( ASWDeathmatchMode()->IsGunGameEnabled() )
		{
			for (int i = 0; i < 3; ++i)
			{
				pOtherMarine->RemoveWeapon( i, true );
			}

			int weapon_id = ASWDeathmatchMode()->GetWeaponIndexByFragsCount( GetFragCount( pOtherMR ) );
			ASWGameRules()->GiveStartingWeaponToMarine( pOtherMarine, weapon_id, 0 );
		}
	}
	else // else if marine was killed by aliens or anything else decrement frags
	{
		IncrementFragCount( pMR, -1 );
	}
}


void CASW_Deathmatch_Mode::OnPlayerFullyJoined(CASW_Player *pPlayer) 
{
	if ( GAMEMODE_TEAMDEATHMATCH == GetGameMode() )
	{
		// assign player to the team which have least players
		CTeam *alpha = GetGlobalTeam( TEAM_ALPHA );
		CTeam *beta  = GetGlobalTeam( TEAM_BETA );
		Assert( alpha );
		Assert( beta );
		if ( alpha && beta )
		{
			int team_id_for_new_player = alpha->GetNumPlayers() < beta->GetNumPlayers() ? TEAM_ALPHA : TEAM_BETA;
			pPlayer->ChangeTeam( team_id_for_new_player );
		}
	}
}

static void RemoveEntitiesByClassName( const char *entity_class_name )
{
	CBaseEntity* pEntity = NULL;
	while ((pEntity = gEntList.FindEntityByClassname( pEntity, entity_class_name )) != NULL)
	{
		UTIL_Remove( pEntity );
	}
}

void CASW_Deathmatch_Mode::DeathmatchEnable()
{
	if (IsGunGameEnabled())
		GunGameDisable(true);	// false - don't turn on weapon respawn timer

	if (IsTeamDeathmatchEnabled())
		TeamDeathmatchDisable(true);

	if (IsInstagibEnabled())
		InstagibDisable(true); // false - don't turn on weapon respawn timer
}


void CASW_Deathmatch_Mode::InstagibEnable() 
{
	if ( IsGunGameEnabled() )
		GunGameDisable(false);	// false - don't turn on weapon respawn timer

	if ( IsTeamDeathmatchEnabled() )
		TeamDeathmatchDisable(false);

	m_iGameMode = GAMEMODE_INSTAGIB;

	/* 
		- set default weapon to rail rifle
		- set rail rifle damage to a high value (it's done in railgun code)
		- disable 1 HP protection and enable Hardcore FF
		- kill players
		- disable respawn of weapons: 
			disable respawn timer
			remove all pickup items
		- reset scores
	*/

	// 10 is for railgun
	SaveSetConvar( rd_default_weapon, 10 );
	SaveSetConvar( asw_skill_melee_dmg_base, 200 );

	DisableWeaponRespawnTimer();
	RemoveAllWeaponsFromMap();
	PrepareMarinesForGunGameOrInstagib( rd_default_weapon.GetInt() );

	this->ResetScores();

	ASWGameRules()->BroadcastSound( "DM.instagib_activated" );

	UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#rd_instagib_activated");

	ASWGameRules()->OnSkillLevelChanged( ASWGameRules()->m_iSkillLevel );
}

void CASW_Deathmatch_Mode::InstagibDisable(bool enable_weapon_respawn_timer/* = true*/) 
{
	m_iGameMode = GAMEMODE_DEATHMATCH;

	/**
		- Revert default weapon
		- Enable Weapon Respawn
		- Reset Scores
	*/

	ASWGameRules()->RevertSingleConvar( &rd_default_weapon );

	if ( enable_weapon_respawn_timer )
	{
		EnableWeaponRespawnTimer();
	}

	this->ResetScores();

	UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#rd_instagib_deactivated");

	ASWGameRules()->OnSkillLevelChanged( ASWGameRules()->m_iSkillLevel );
}

void CASW_Deathmatch_Mode::GunGameEnable() 
{
	/*
		Stop weapon respawn timer
		Remove all weapons from map
		For all live marines remove all weapons and give one weapon
		Set health of all live marines to 100 %
		Move all live marines to different Spawn Points
		Reset Scores

		not here:
		On spawn give him a weapon depending on his frags count
		On kill take away marine weapon and give him a new one
		Don't drop weapon on death
	*/

	if ( IsInstagibEnabled() )
		InstagibDisable(false); // false - don't turn on weapon respawn timer

	if ( IsTeamDeathmatchEnabled() )
		TeamDeathmatchDisable(false);

	m_iGameMode = GAMEMODE_GUNGAME; 

	// TODO load from config or generate random
	const int weapons_ids[] = { 8, 16, 12, 0, 4, 1, 3, 2, 21, 22, 23, 15, 10, 20, 18 };
	m_iGunGameWeaponCount = NELEMS( weapons_ids );
	for ( int i = 0; i < m_iGunGameWeaponCount; i++ )
	{
		m_iGunGameWeapons.Set( i, weapons_ids[i] );
	}

	DisableWeaponRespawnTimer();
	RemoveAllWeaponsFromMap();
	PrepareMarinesForGunGameOrInstagib( -1 );

	this->ResetScores();

	ASWGameRules()->BroadcastSound( "DM.gungame_activated" );

	UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#rd_gungame_activated");

	ASWGameRules()->OnSkillLevelChanged( ASWGameRules()->m_iSkillLevel );
}

void CASW_Deathmatch_Mode::GunGameDisable(bool enable_weapon_respawn_timer /*= true*/) 
{
	if ( !IsGunGameEnabled() )
		return;

	m_iGameMode = GAMEMODE_DEATHMATCH;

	/**
		- Enable Weapon Respawn
		- Reset Scores
	*/

	if ( enable_weapon_respawn_timer )
	{
		EnableWeaponRespawnTimer();
	}

	this->ResetScores();

	UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#rd_gungame_deactivated");

	ASWGameRules()->OnSkillLevelChanged( ASWGameRules()->m_iSkillLevel );
}

#pragma warning( push )
#pragma warning( disable: 4706 )
void CASW_Deathmatch_Mode::TeamDeathmatchEnable() 
{
	if ( IsInstagibEnabled() )
		InstagibDisable(false); // false - don't turn on weapon respawn timer 

	if ( IsGunGameEnabled() )
		GunGameDisable(false);	// false - don't turn on weapon respawn timer 

	m_iGameMode = GAMEMODE_TEAMDEATHMATCH; 

	EnableWeaponRespawnTimer();
	//DisableWeaponRespawnTimer();
	//RemoveAllWeaponsFromMap();

	// create teams 
	CTeam * t0 = (CTeam*) CreateEntityByName( "rd_team" );
	t0->Init( "Unassigned", TEAM_UNASSIGNED );
	
	CTeam *t1 = (CTeam*) CreateEntityByName( "rd_team" );
	t1->Init ( "Spectator", TEAM_SPECTATOR );

	CTeam *t2 = (CTeam*) CreateEntityByName( "rd_team" );
	t2->Init( "Terrorists", TEAM_ALPHA );

	CTeam *t3 = (CTeam*) CreateEntityByName( "rd_team" );
	t3->Init( "Counter-Terrorists", TEAM_BETA );

	CTeamSpawnPoint *p = NULL;
	// Fill each CTeam with spawnpoints 
	while ( p = (CTeamSpawnPoint*)gEntList.FindEntityByClassname(p, "info_player_start_team1") )
	{
		t2->AddSpawnpoint( p );
	}

	p = NULL;

	while ( p = (CTeamSpawnPoint*)gEntList.FindEntityByClassname(p, "info_player_start_team2") )
	{
		t3->AddSpawnpoint( p );
	}
#pragma warning( pop )
	int count = 0;
	// Move each player to a random team 
	for ( int i = 1; i <= gpGlobals->maxClients; ++i )
	{
		CASW_Player* pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
		if ( !pPlayer || !pPlayer->IsConnected() )
			continue;
		pPlayer->ChangeTeam( count % 2 ? TEAM_ALPHA : TEAM_BETA, false, true );
		count++;
	}

	// Set team number for every marine resource
	// including bots
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );

		if ( !pMR )
		{
			continue;
		}

		if ( !pMR->IsInhabited() )
		{
			pMR->ChangeTeam( count % 2 ? TEAM_ALPHA : TEAM_BETA );
			count++;
		}
		else if ( pMR->GetCommander() )
		{
			pMR->ChangeTeam( pMR->GetCommander()->GetTeamNumber() );
		}
	}

	// Kill each marine 
	ASWGameRules()->KillAllMarines(); 

	// decrease moving speed 
//	SaveSetConvar( asw_marine_speed_scale_easy, 0.9f );
//	SaveSetConvar( asw_marine_speed_scale_normal, 0.9f );
//	SaveSetConvar( asw_marine_speed_scale_hard, 0.9f );
//	SaveSetConvar( asw_marine_speed_scale_insane, 0.9f );

	// show team members names and healths 
	SaveSetConvar( asw_world_healthbars, 1 );
	SaveSetConvar( asw_marine_names, 1 );

	BroadcastLoadoutScreen();

	ResetScores();

	ASWGameRules()->BroadcastSound( "DM.tdm_activated" );

	UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "Team Deathmatch Activated "); // "#rd_teamdeathmatch_activated"

	ASWGameRules()->OnSkillLevelChanged( ASWGameRules()->m_iSkillLevel );
}

void CASW_Deathmatch_Mode::TeamDeathmatchDisable(bool enable_weapon_respawn_timer/* = true*/) 
{
	if ( !IsTeamDeathmatchEnabled() )
		return;

	m_iGameMode = GAMEMODE_DEATHMATCH;

	/**
		- Remove and delete teams 
		- Enable Weapon Respawn
		- Reset Scores 
		- Restore moving speed
	*/

	for ( int i = 0; i < g_Teams.Count(); ++i )
	{
		UTIL_Remove( g_Teams[i] );
	}
	g_Teams.Purge();

	if ( enable_weapon_respawn_timer ) 
	{
		EnableWeaponRespawnTimer(); 
	}

	this->ResetScores(); 

	SaveSetConvar( asw_marine_speed_scale_easy, 1.5f );
	SaveSetConvar( asw_marine_speed_scale_normal, 1.5f );
	SaveSetConvar( asw_marine_speed_scale_hard, 1.5f );
	SaveSetConvar( asw_marine_speed_scale_insane, 1.5f );

	// don't show team members names and healths 
	SaveSetConvar( asw_world_healthbars, 0 );
	SaveSetConvar( asw_marine_names, 0 );

	ASWGameRules()->OnSkillLevelChanged( ASWGameRules()->m_iSkillLevel );
}


void CASW_Deathmatch_Mode::SpawnMarine( CASW_Player *player )
{
	// for deathmatch instead of spectating next marine we respawn this player
	if ( ASWGameRules() && ASWGameResource() )
	{
		if ( ASWGameRules()->m_fDeathmatchFinishTime > 0 )
		{
			// don't allow respawning after the round ends
			return;
		}

		CASW_Marine_Resource *pMR = NULL;

		const int max_marines = ASWGameResource()->GetMaxMarineResources();
		for ( int i = 0; i < max_marines; ++i )
		{
			pMR = ASWGameResource()->GetMarineResource( i );
			if ( pMR && pMR->GetCommander() == player && pMR->IsInhabited() && pMR->GetHealthPercent() <= 0 )
			{
				break;
			}
			pMR = NULL;
		}

		if ( !pMR )
		{
			return;
		}

		if ( IsTeamDeathmatchEnabled() )
		{
			CTeam *t = player->GetTeam();
			Assert( t != NULL );
			if ( t )
			{
				CBaseEntity *spawn_point_for_marine = t->SpawnPlayer(player);
				if ( spawn_point_for_marine )
				{
					ASWGameRules()->SpawnMarineAt( pMR, spawn_point_for_marine->GetAbsOrigin(), spawn_point_for_marine->GetAbsAngles(), false);
					player->SwitchMarine( pMR );
				}
				else
				{
					Warning( "Can't spawn marine. No valid spawn points found \n" );
				}
			}
			else
			{
				Warning( "This player doesn't have a team. Can't spawn \n" );
			}
		}
		else
		{
			spawn_point = ASWGameRules()->GetMarineSpawnPoint(spawn_point);
			if (!spawn_point)
				spawn_point = ASWGameRules()->GetMarineSpawnPoint(NULL);
			if (spawn_point) {
				ASWGameRules()->SpawnMarineAt( pMR, spawn_point->GetAbsOrigin(), spawn_point->GetAbsAngles(), false);
				player->SwitchMarine( pMR );
			}
		}
	}
}


void CASW_Deathmatch_Mode::DisableWeaponRespawnTimer() 
{
	// disabling respawn timer 
	CBaseEntity* pEntity = NULL;
	while ((pEntity = gEntList.FindEntityByName( pEntity, "wpn_respawn_timer" )) != NULL)
	{
		CTimerEntity* respawn_timer = dynamic_cast<CTimerEntity*>(pEntity);
		if (respawn_timer)
		{
			respawn_timer->Disable();
		}
		else
		{
			Warning("wpn_respawn_timer not found! \n");
		}
	}
}

void CASW_Deathmatch_Mode::RemoveAllWeaponsFromMap() 
{
	// remove all pickup items
	RemoveEntitiesByClassName( "asw_pickup_flamer" );
	RemoveEntitiesByClassName( "asw_pickup_heal_grenade" );
	RemoveEntitiesByClassName( "asw_pickup_medkit" );
	RemoveEntitiesByClassName( "asw_pickup_mines" );
	RemoveEntitiesByClassName( "asw_pickup_mining_laser" );
	RemoveEntitiesByClassName( "asw_pickup_shotgun" );
	RemoveEntitiesByClassName( "asw_pickup_stim" );
	RemoveEntitiesByClassName( "asw_pickup_railgun" );
	RemoveEntitiesByClassName( "asw_pickup_vindicator" );
	RemoveEntitiesByClassName( "asw_pickup_autogun" );
	RemoveEntitiesByClassName( "asw_pickup_pdw" );
	RemoveEntitiesByClassName( "asw_pickup_chainsaw" );
	RemoveEntitiesByClassName( "asw_pickup_prifle" );
	RemoveEntitiesByClassName( "asw_pickup_pistol" );
	RemoveEntitiesByClassName( "asw_pickup_sentry" );
	RemoveEntitiesByClassName( "asw_pickup_sentry_cannon" );
	RemoveEntitiesByClassName( "asw_pickup_sentry_flamer" );
	RemoveEntitiesByClassName( "asw_pickup_grenades" );
	RemoveEntitiesByClassName( "asw_pickup_rifle" );

	RemoveEntitiesByClassName( "asw_weapon_rifle" );
	RemoveEntitiesByClassName( "asw_weapon_prifle" );
	RemoveEntitiesByClassName( "asw_weapon_autogun" );
	RemoveEntitiesByClassName( "asw_weapon_vindicator" );
	RemoveEntitiesByClassName( "asw_weapon_pistol" );
	RemoveEntitiesByClassName( "asw_weapon_sentry" );
	RemoveEntitiesByClassName( "asw_weapon_heal_grenade" );
	RemoveEntitiesByClassName( "asw_weapon_ammo_satchel" );
	RemoveEntitiesByClassName( "asw_weapon_shotgun" );
	RemoveEntitiesByClassName( "asw_weapon_tesla_gun" );
	RemoveEntitiesByClassName( "asw_weapon_railgun" );
	RemoveEntitiesByClassName( "asw_weapon_heal_gun" );
	RemoveEntitiesByClassName( "asw_weapon_healamp_gun" );
	RemoveEntitiesByClassName( "asw_weapon_pdw" );
	RemoveEntitiesByClassName( "asw_weapon_flamer" );
	RemoveEntitiesByClassName( "asw_weapon_sentry_freeze" );
	RemoveEntitiesByClassName( "asw_weapon_minigun" );
	RemoveEntitiesByClassName( "asw_weapon_sniper_rifle" );
	RemoveEntitiesByClassName( "asw_weapon_sentry_flamer" );
	RemoveEntitiesByClassName( "asw_weapon_chainsaw" );
	RemoveEntitiesByClassName( "asw_weapon_sentry_cannon" );    
	RemoveEntitiesByClassName( "asw_weapon_grenade_launcher" );
	RemoveEntitiesByClassName( "asw_weapon_medkit" );
	RemoveEntitiesByClassName( "asw_weapon_welder" );
	RemoveEntitiesByClassName( "asw_weapon_flares" );
	RemoveEntitiesByClassName( "asw_weapon_gas_grenades" );
	RemoveEntitiesByClassName( "asw_weapon_laser_mines" );
	RemoveEntitiesByClassName( "asw_weapon_normal_armor" );
	RemoveEntitiesByClassName( "asw_weapon_buff_grenade" );
	RemoveEntitiesByClassName( "asw_weapon_hornet_barrage" );
	RemoveEntitiesByClassName( "asw_weapon_freeze_grenades" );
	RemoveEntitiesByClassName( "asw_weapon_stim" );    
	RemoveEntitiesByClassName( "asw_weapon_tesla_trap" );
	RemoveEntitiesByClassName( "asw_weapon_electrified_armor" );
	RemoveEntitiesByClassName( "asw_weapon_mines" );
	RemoveEntitiesByClassName( "asw_weapon_flashlight" );
	RemoveEntitiesByClassName( "asw_weapon_grenades" );
	RemoveEntitiesByClassName( "asw_weapon_night_vision" );
	RemoveEntitiesByClassName( "asw_weapon_smart_bomb" );
	RemoveEntitiesByClassName( "asw_weapon_fire_extinguisher" );
	RemoveEntitiesByClassName( "asw_weapon_mining_laser" );
	RemoveEntitiesByClassName( "asw_weapon_t75" );    
	RemoveEntitiesByClassName( "asw_weapon_blink" );
	RemoveEntitiesByClassName( "asw_weapon_jump_jet" );
}

void CASW_Deathmatch_Mode::EnableWeaponRespawnTimer() 
{
	CBaseEntity* pEntity = NULL;
	while ((pEntity = gEntList.FindEntityByName( pEntity, "wpn_respawn_timer" )) != NULL)
	{
		CTimerEntity* respawn_timer = dynamic_cast<CTimerEntity*>(pEntity);
		if (respawn_timer)
		{
			respawn_timer->Enable();
			respawn_timer->FireTimer();
		}
		else
		{
			Warning("wpn_respawn_timer not found. \n");
		}
	}
}



void CASW_Deathmatch_Mode::PrepareMarinesForGunGameOrInstagib(int weapon_id/* = -1*/) 
{
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( !pGameResource )
		return;

	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		if (pGameResource->GetMarineResource(i) != NULL && pGameResource->GetMarineResource(i)->GetMarineEntity())
		{
			CASW_Marine *pMarine = pGameResource->GetMarineResource(i)->GetMarineEntity();

			// remove all weapons from marine
			for ( int i = 0; i < 3; ++i )
			{
				pMarine->RemoveWeapon(i, true); // true - no swap to other weapon
			}

			// give default weapon
			if ( ASWGameRules() )
			{
				if ( weapon_id == -1) 
				{
					ASWGameRules()->GiveStartingWeaponToMarine( pMarine, GetWeaponIndexByFragsCount(0), 0);
				}
				else 
				{
					ASWGameRules()->GiveStartingWeaponToMarine( pMarine, weapon_id, 0);
				}
			}
			
			// heal marine
			pMarine->AddSlowHeal( pMarine->GetMaxHealth() - pMarine->GetHealth(), 3, NULL );

			// move to spawnpoint
			CBaseEntity *(&spawn_point) = CASW_Player::spawn_point;
			spawn_point = ASWGameRules()->GetMarineSpawnPoint(spawn_point);
			if (!spawn_point)
				spawn_point = ASWGameRules()->GetMarineSpawnPoint(NULL);
			if (spawn_point) {
				pMarine->Teleport(&spawn_point->GetAbsOrigin(), &spawn_point->GetAbsAngles(), &vec3_origin);
			}
		}
	}
}

void CASW_Deathmatch_Mode::BroadcastLoadoutScreen() 
{
/*	// 2 ways of doing this, one is commented
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )	
	{
		CASW_Player* pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
		if ( !pPlayer || !pPlayer->IsConnected() )
			continue;

		engine->ClientCommand( pPlayer->edict(), "cl_select_loadout_noclose");
	} 
*/	
	CBroadcastRecipientFilter filter;
	filter.MakeReliable();

	UserMessageBegin ( filter, "BroadcastClientCmd" );
		WRITE_STRING( "cl_select_loadout_noclose" );
	MessageEnd();

}

void CASW_Deathmatch_Mode::ResetScores()
{
	ResetFragsLeftSaid();

	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
		if ( pMR )
		{
			pMR->m_iBotFrags = 0;
		}
		m_iBotDeaths[i] = 0;
		m_flBotLastFragTime[i] = 0.0f;
		m_iBotKillingSpree[i] = 0;
	}

	ASWGameRules()->ResetScores();
}

void CASW_Deathmatch_Mode::KillingSpreeIncrease( CASW_Marine_Resource *pMR )
{
	if ( !rd_quake_sounds.GetBool() )
		return;

	// using pointers to simplify code
	float *pflLastFragTime = NULL;
	int *piKillingSpree = NULL;

	if ( pMR->IsInhabited() )
	{
		CASW_Player *pPlayer = pMR->GetCommander();
		if ( pPlayer )
		{
			pflLastFragTime = &pPlayer->m_fLastFragTime;
			piKillingSpree = &pPlayer->m_iKillingSpree;
		}
		else
		{
			return;
		}
	}
	else
	{
		int index = ASWGameResource()->GetMarineResourceIndex( pMR );

		pflLastFragTime = &m_flBotLastFragTime[index];
		piKillingSpree = &m_iBotKillingSpree[index];
	}

	// reactivedrop: 3 seconds for checking for killingspree
	if ( gpGlobals->curtime - *pflLastFragTime < rd_killingspree_time_limit.GetFloat() )
	{
		(*piKillingSpree)++;
		switch ( *piKillingSpree )
		{
		case 2:
			ASWGameRules()->BroadcastSound( "DM.double_kill" );
			break;
		case 3:
			ASWGameRules()->BroadcastSound( "DM.tripple_kill" );
			break;
		case 4:
			ASWGameRules()->BroadcastSound( "DM.monster_kill" );
			break;
		case 5:
			ASWGameRules()->BroadcastSound( "DM.godlike" );
			break;
		}
	}
	else
	{
		*piKillingSpree = 1;
	}

	*pflLastFragTime = gpGlobals->curtime;
}

void CASW_Deathmatch_Mode::KillingSpreeReset( CASW_Marine_Resource *pMR )
{
	if ( !rd_quake_sounds.GetBool() )
		return;

	if ( pMR->IsInhabited() )
	{
		CASW_Player *pPlayer = pMR->GetCommander();
		if ( pPlayer )
		{
			pPlayer->m_iKillingSpree = 0;
			pPlayer->m_fLastFragTime = 0.0f;
		}
	}
	else
	{
		int index = ASWGameResource()->GetMarineResourceIndex( pMR );
		m_iBotKillingSpree[index] = 0;
		m_flBotLastFragTime[index] = 0.0f;
	}
}

void CASW_Deathmatch_Mode::DeathmatchThink()
{
	// respawn bots 
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		if ( ASWGameRules()->m_fDeathmatchFinishTime > 0 )
		{
			// don't allow respawning after the round ends
			break;
		}

		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
		if ( pMR && !pMR->GetMarineEntity() && !pMR->IsInhabited() )
		{
			if ( IsTeamDeathmatchEnabled() )
			{
				CTeam *pTeam = pMR->GetTeam();
				CBaseEntity *pSpawnPoint = pTeam->SpawnPlayer( NULL );
				if ( pSpawnPoint )
				{
					if ( ASWGameRules()->SpawnMarineAt( pMR, pSpawnPoint->GetAbsOrigin(), pSpawnPoint->GetAbsAngles(), false ) )
					{
						CASW_Marine *pBot = pMR->GetMarineEntity();
						if ( pBot )
							pBot->SetASWOrders( ASW_ORDER_HOLD_POSITION ); // suprisingly but this makes them find enemy marines and chase them
					}
					break;
				}
				continue;
			}
			spawn_point = ASWGameRules()->GetMarineSpawnPoint( spawn_point );
			if (!spawn_point)
			{
				spawn_point = ASWGameRules()->GetMarineSpawnPoint( NULL );
			}

			if ( spawn_point )
			{
				if ( ASWGameRules()->SpawnMarineAt( pMR, spawn_point->GetAbsOrigin(), spawn_point->GetAbsAngles(), false ) )
				{
					CASW_Marine *pBot = pMR->GetMarineEntity();
					if ( pBot )
						pBot->SetASWOrders( ASW_ORDER_HOLD_POSITION ); // suprisingly but this makes them find enemy marines and chase them
				}
			}
			break; // don't do two in a frame
		}
	}

	SetThink( &CASW_Deathmatch_Mode::DeathmatchThink );
	SetNextThink( gpGlobals->curtime + 1.0f );
}


void CASW_Deathmatch_Mode::LevelInitPostEntity()
{
	DevMsg( "CASW_Deathmatch_Mode::LevelInitPostEntity()\n" );
	SetThink( &CASW_Deathmatch_Mode::DeathmatchThink );
	SetNextThink( gpGlobals->curtime + 1.0f );
}



#endif	// ifdef GAME_DLL

int CASW_Deathmatch_Mode::GetFragCount( CASW_Marine_Resource *pMR )
{
	if ( pMR->IsInhabited() )
	{
		CASW_Player *pPlayer = pMR->GetCommander();
		if ( pPlayer )
		{
#ifdef GAME_DLL
			return pPlayer->FragCount();
#else
			return g_PR->GetPlayerScore( pPlayer->index );
#endif
		}
		return 0;
	}
	return pMR->m_iBotFrags;
}


int CASW_Deathmatch_Mode::GetSmallestTeamNumber()
{
	int iAlpha = 0;
	int iBeta = 0;

	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
		if ( pMR && pMR->GetTeamNumber() == TEAM_ALPHA )
		{
			iAlpha++;
		}
		else if ( pMR && pMR->GetTeamNumber() == TEAM_BETA )
		{
			iBeta++;
		}
	}
	return iAlpha <= iBeta ? TEAM_ALPHA : TEAM_BETA;
}

int CASW_Deathmatch_Mode::GetWeaponIndexByFragsCount( int nFrags )
{
	Assert( m_iGunGameWeaponCount > 0 );

	// it can be < 0 if marine suicided
	return m_iGunGameWeapons[MAX(nFrags, 0) % m_iGunGameWeaponCount];
}

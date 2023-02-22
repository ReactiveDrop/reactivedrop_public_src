#include "cbase.h"
#include "c_asw_steamstats.h"
#include "c_asw_debrief_stats.h"
#include "asw_gamerules.h"
#include "c_asw_game_resource.h"
#include "fmtstr.h"
#include "asw_equipment_list.h"
#include "string_t.h"
#include "asw_util_shared.h"
#include "asw_marine_profile.h"
#include <vgui/ILocalize.h>
#include "asw_shareddefs.h"
#include "c_asw_marine_resource.h"
#include "c_asw_campaign_save.h"
#include "asw_deathmatch_mode.h"
#include "rd_workshop.h"
#include "rd_lobby_utils.h"
#include "clientmode_asw.h"
#include "missioncompleteframe.h"
#include "missioncompletepanel.h"
#include "rd_missions_shared.h"
#include "c_user_message_register.h"
#include "asw_alien_classes.h"
#include "rd_cause_of_death.h"
#include "rd_inventory_shared.h"

CASW_Steamstats g_ASW_Steamstats;

ConVar asw_stats_leaderboard_debug( "asw_stats_leaderboard_debug", "0", FCVAR_NONE );
ConVar rd_leaderboard_enabled_client( "rd_leaderboard_enabled_client", "1", FCVAR_ARCHIVE, "If 0 player leaderboard scores will not be set or updated on mission complete. Client only." );

namespace 
{
#define FETCH_STEAM_STATS( apiname, membername ) \
	{ const char * szApiName = apiname; \
	if( !SteamUserStats()->GetUserStat( playerSteamID, szApiName, &membername ) ) \
	{ \
		bOK = false; \
		DevMsg( "STEAMSTATS: Failed to retrieve stat %s.\n", szApiName ); \
} }

#define SEND_STEAM_STATS( apiname, membername ) \
	{ const char * szApiName = apiname; \
	SteamUserStats()->SetStat( szApiName, membername ); }

	// Define some strings used by the stats API
	const char* szGamesTotal = ".games.total";
	const char* szGamesSuccess = ".games.success";
	const char* szGamesSuccessPercent = ".games.success.percent";
	const char* szKillsTotal = ".kills.total";
	const char* szDamageTotal = ".damage.total";
	const char* szFFTotal = ".ff.total";
	const char* szAccuracy = ".accuracy.avg";
	const char* szShotsTotal = ".shotsfired.total";
	const char* szShotsHit = ".shotshit.total";
	const char* szHealingTotal = ".healing.total";
	const char* szTimeTotal = ".time.total";
	const char* szTimeSuccess = ".time.success";
	const char* szKillsAvg = ".kills.avg";
	const char* szDamageAvg = ".damage.avg";
	const char* szFFAvg = ".ff.avg";
	const char* szTimeAvg = ".time.avg";
	const char* szBestDifficulty = ".difficulty.best";
	const char* szBestTime = ".time.best";
	const char* szBestSpeedrunDifficulty = ".time.best.difficulty";

	// difficulty names used when fetching steam stats
	const char* g_szDifficulties[] =
	{
		"Easy",
		"Normal",
		"Hard",
		"Insane",
		"imba"
	};

	const char *const g_OfficialCampaigns[] =
	{
		"jacob",
		"rd-operationcleansweep",
		"rd_research7",
		"rd-area9800",
		"rd-tarnorcampaign1",
#ifdef RD__CAMPAIGNS_DEADCITY
		"rd_deadcity",
#endif
		"tilarus5",
		"rd_lanasescape_campaign",
#ifdef RD__CAMPAIGNS_REDUCTION
		"rd_reduction_campaign",
#endif
		"rd_paranoia",
		"rd_nh_campaigns",
		"rd_biogen_corporation",
#ifdef RD_6A_CAMPAIGNS_ACCIDENT32
		"rd_accident32",
#endif
#ifdef RD_6A_CAMPAIGNS_ADANAXIS
		"rd_adanaxis",
#endif
	};
	const char *const g_OfficialMaps[] =
	{
		"dm_desert",
		"dm_deima",
		"dm_residential",
		"dm_testlab",
		"asi-jac1-landingbay_01",
		"asi-jac1-landingbay_02",
		"asi-jac2-deima",
		"asi-jac3-rydberg",
		"asi-jac4-residential",
		"asi-jac6-sewerjunction",
		"asi-jac7-timorstation",
		"rd-ocs1storagefacility",
		"rd-ocs2landingbay7",
		"rd-ocs3uscmedusa",
		"rd-res1forestentrance",
		"rd-res2research7",
		"rd-res3miningcamp",
		"rd-res4mines",
		"rd-area9800lz",
		"rd-area9800pp1",
		"rd-area9800pp2",
		"rd-area9800wl",
		"rd-tft1desertoutpost",
		"rd-tft2abandonedmaintenance",
		"rd-tft3spaceport",
#ifdef RD__CAMPAIGNS_DEADCITY
		"rd-dc1_omega_city",
		"rd-dc2_breaking_an_entry",
		"rd-dc3_search_and_rescue",
#endif
		"rd-til1midnightport",
		"rd-til2roadtodawn",
		"rd-til3arcticinfiltration",
		"rd-til4area9800",
		"rd-til5coldcatwalks",
		"rd-til6yanaurusmine",
		"rd-til7factory",
		"rd-til8comcenter",
		"rd-til9syntekhospital",
		"rd-lan1_bridge",
		"rd-lan2_sewer",
		"rd-lan3_maintenance",
		"rd-lan4_vent",
		"rd-lan5_complex",
#ifdef RD__CAMPAIGNS_REDUCTION
		"rd-reduction1",
		"rd-reduction2",
		"rd-reduction3",
		"rd-reduction4",
		"rd-reduction5",
		"rd-reduction6",
#endif
		"rd-par1unexpected_encounter",
		"rd-par2hostile_places",
		"rd-par3close_contact",
		"rd-par4high_tension",
		"rd-par5crucial_point",
		"rd-nh01_logisticsarea",
		"rd-nh02_platformxvii",
		"rd-nh03_groundworklabs",
		"rd-bio1operationx5",
		"rd-bio2invisiblethreat",
		"rd-bio3biogenlabs",
#ifdef RD_6A_CAMPAIGNS_ACCIDENT32
		"rd-acc1_infodep",
		"rd-acc2_powerhood",
		"rd-acc3_rescenter",
		"rd-acc4_confacility",
		"rd-acc5_j5connector",
		"rd-acc6_labruins",
		"rd-acc_complex",
#endif
#ifdef RD_6A_CAMPAIGNS_ADANAXIS
		"rd-ada_sector_a9",
		"rd-ada_nexus_subnode",
		"rd-ada_neon_carnage",
		"rd-ada_fuel_junction",
		"rd-ada_dark_path",
		"rd-ada_forbidden_outpost",
		"rd-ada_new_beginning",
		"rd-ada_anomaly",
#endif
		"rd-bonus_mission1",
		"rd-bonus_mission2",
		"rd-bonus_mission3",
		"rd-bonus_mission4",
		"rd-bonus_mission5",
		"rd-bonus_mission6",
		"rd-bonus_mission7",
	};

	const char *const g_OfficialNonCampaignMaps[] =
	{
		"rd-acc_complex",
		"rd-ada_new_beginning",
		"rd-ada_anomaly",
		"dm_desert",
		"dm_deima",
		"dm_residential",
		"dm_testlab",
		"rd-bonus_mission1",
		"rd-bonus_mission2",
		"rd-bonus_mission3",
		"rd-bonus_mission4",
		"rd-bonus_mission5",
		"rd-bonus_mission6",
		"rd-bonus_mission7",
	};
}

bool IsOfficialCampaign()
{
	if ( !ASWGameRules()->IsCampaignGame() )
	{
		const char *szMapName = engine->GetLevelNameShort();
		for ( int i = 0; i < ARRAYSIZE( g_OfficialNonCampaignMaps ); i++ )
		{
			if ( FStrEq( szMapName, g_OfficialNonCampaignMaps[i] ) )
			{
				return true;
			}
		}

		return false;
	}

	CASW_Campaign_Save *pCampaign = ASWGameRules()->GetCampaignSave();

	const char *szMapName = engine->GetLevelNameShort();
	const char *szCampaignName = pCampaign->GetCampaignName();

	bool bCampaignMatches = false;
	for ( int i = 0; i < ARRAYSIZE( g_OfficialCampaigns ); i++ )
	{
		if ( FStrEq( szCampaignName, g_OfficialCampaigns[i] ) )
		{
			bCampaignMatches = true;
			break;
		}
	}

	if ( !bCampaignMatches )
	{
		return false;
	}

	for ( int i = 0; i < ARRAYSIZE( g_OfficialMaps ); i++ )
	{
		if ( FStrEq( szMapName, g_OfficialMaps[i] ) )
		{
				return true;
		}
	}

	return false;
}

bool IsWorkshopCampaign()
{
	if ( !engine->IsInGame() )
		return false;

	CASW_Campaign_Save *pCampaign = ASWGameRules()->GetCampaignSave();
	if ( pCampaign && ASWGameRules()->IsCampaignGame() )
	{
		const char *szCampaignName = pCampaign->GetCampaignName();

		if ( szCampaignName && *szCampaignName && g_ReactiveDropWorkshop.FindAddonProvidingFile( CFmtStr( "resource/campaigns/%s.txt", szCampaignName ) ) == k_PublishedFileIdInvalid )
		{
			return false;
		}
	}

	const char *szMapName = engine->GetLevelNameShort();
	return g_ReactiveDropWorkshop.FindAddonProvidingFile( CFmtStr( "resource/overviews/%s.txt", szMapName ) ) != k_PublishedFileIdInvalid;
}

// Returns true if leaderboard entries can be uploaded for non-successful missions.
bool ShouldUploadOnFailure()
{
	if ( !engine->IsInGame() )
		return false;

	const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( engine->GetLevelNameShort() );
	if ( !pMission )
		return false;

	return pMission->HasTag( "upload_on_failure" );
}

bool IsDamagingWeapon( const char* szWeaponName, bool bIsExtraEquip )
{
	if( bIsExtraEquip )
	{
		// Check for the few damaging weapons
		if( !Q_strcmp( szWeaponName, "asw_weapon_laser_mines" ) || 
			!Q_strcmp( szWeaponName, "asw_weapon_hornet_barrage" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_mines" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_grenades" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_smart_bomb" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_tesla_trap" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_gas_grenades" ))
			return true;
		else
			return false;
	}
	else
	{
		// Check for the few non-damaging weapons
		if( !Q_strcmp( szWeaponName, "asw_weapon_heal_grenade" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_ammo_satchel" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_heal_gun" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_healamp_gun" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_fire_extinguisher" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_t75" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_ammo_bag" ) ||
			!Q_strcmp( szWeaponName, "asw_weapon_medical_satchel" ) )
			return false;
		else
			return true;
	}
}

Class_T GetDamagingWeaponClassFromName( const char *szClassName )
{
	if( FStrEq( szClassName, "asw_weapon_rifle") )
		return (Class_T)CLASS_ASW_RIFLE;
	else if( FStrEq( szClassName, "asw_weapon_prifle") )
		return (Class_T)CLASS_ASW_PRIFLE;
	else if( FStrEq( szClassName, "asw_weapon_autogun") )
		return (Class_T)CLASS_ASW_AUTOGUN;
	else if( FStrEq( szClassName, "asw_weapon_vindicator") )
		return (Class_T)CLASS_ASW_ASSAULT_SHOTGUN;
	else if( FStrEq( szClassName, "asw_weapon_pistol") )
		return (Class_T)CLASS_ASW_PISTOL;
	else if( FStrEq( szClassName, "asw_weapon_sentry") )
		return (Class_T)CLASS_ASW_SENTRY_GUN;
	else if( FStrEq( szClassName, "asw_weapon_shotgun") )
		return (Class_T)CLASS_ASW_SHOTGUN;
	else if( FStrEq( szClassName, "asw_weapon_tesla_gun") )
		return (Class_T)CLASS_ASW_TESLA_GUN;
	else if( FStrEq( szClassName, "asw_weapon_railgun") )
		return (Class_T)CLASS_ASW_RAILGUN;
	else if( FStrEq( szClassName, "asw_weapon_pdw") )
		return (Class_T)CLASS_ASW_PDW;
	else if( FStrEq( szClassName, "asw_weapon_flamer") )
		return (Class_T)CLASS_ASW_FLAMER;
	else if( FStrEq( szClassName, "asw_weapon_sentry_freeze") )
		return (Class_T)CLASS_ASW_SENTRY_FREEZE;
	else if( FStrEq( szClassName, "asw_weapon_minigun") )
		return (Class_T)CLASS_ASW_MINIGUN;
	else if( FStrEq( szClassName, "asw_weapon_sniper_rifle") )
		return (Class_T)CLASS_ASW_SNIPER_RIFLE;
	else if( FStrEq( szClassName, "asw_weapon_sentry_flamer") )
		return (Class_T)CLASS_ASW_SENTRY_FLAMER;
	else if( FStrEq( szClassName, "asw_weapon_chainsaw") )
		return (Class_T)CLASS_ASW_CHAINSAW;
	else if( FStrEq( szClassName, "asw_weapon_sentry_cannon") )
		return (Class_T)CLASS_ASW_SENTRY_CANNON;
	else if( FStrEq( szClassName, "asw_weapon_grenade_launcher") )
		return (Class_T)CLASS_ASW_GRENADE_LAUNCHER;
	else if( FStrEq( szClassName, "asw_weapon_mining_laser") )
		return (Class_T)CLASS_ASW_MINING_LASER;
	else if( FStrEq( szClassName, "asw_weapon_combat_rifle") )
		return (Class_T)CLASS_ASW_COMBAT_RIFLE;
	else if( FStrEq( szClassName, "asw_weapon_deagle") )
		return (Class_T)CLASS_ASW_DEAGLE;
	else if( FStrEq( szClassName, "asw_weapon_devastator") )
		return (Class_T)CLASS_ASW_DEVASTATOR;
	else if( FStrEq( szClassName, "asw_weapon_healamp_gun") )
		return (Class_T)CLASS_ASW_HEALAMP_GUN;
	else if( FStrEq( szClassName, "asw_weapon_50calmg") )
		return (Class_T)CLASS_ASW_50CALMG;
	else if ( FStrEq( szClassName, "asw_weapon_heavy_rifle" ) )
		return (Class_T) CLASS_ASW_HEAVY_RIFLE;
	else if ( FStrEq( szClassName, "asw_weapon_medrifle" ) )
		return (Class_T) CLASS_ASW_MEDRIFLE;
	else if ( FStrEq( szClassName, "asw_weapon_ar2" ) )
		return (Class_T) CLASS_ASW_AR2;

	else if( FStrEq( szClassName, "asw_weapon_laser_mines") )
		return (Class_T)CLASS_ASW_LASER_MINES;
	else if( FStrEq( szClassName, "asw_weapon_hornet_barrage") )
		return (Class_T)CLASS_ASW_HORNET_BARRAGE;
	else if( FStrEq( szClassName, "asw_weapon_mines") )
		return (Class_T)CLASS_ASW_MINES;
	else if( FStrEq( szClassName, "asw_weapon_grenades") )
		return (Class_T)CLASS_ASW_GRENADES;
	else if( FStrEq( szClassName, "asw_weapon_smart_bomb") )
		return (Class_T)CLASS_ASW_SMART_BOMB;
	else if( FStrEq( szClassName, "asw_weapon_t75") )
		return (Class_T)CLASS_ASW_T75;
	else if( FStrEq( szClassName, "asw_weapon_tesla_trap") )
		return (Class_T)CLASS_ASW_TESLA_TRAP;
	else if( FStrEq( szClassName, "asw_weapon_gas_grenades") )
		return (Class_T)CLASS_ASW_GAS_GRENADES;

	else if( FStrEq( szClassName, "asw_rifle_grenade") )
		return (Class_T)CLASS_ASW_RIFLE_GRENADE;
	else if( FStrEq( szClassName, "asw_prifle_grenade") )
		return (Class_T)CLASS_ASW_GRENADE_PRIFLE;
	else if( FStrEq( szClassName, "asw_vindicator_grenade") )
		return (Class_T)CLASS_ASW_GRENADE_VINDICATOR;
	else if( FStrEq( szClassName, "asw_combat_rifle_shotgun") )
		return (Class_T)CLASS_ASW_COMBAT_RIFLE_SHOTGUN;
	else if( FStrEq( szClassName, "prop_combine_ball" ) )
		return (Class_T)CLASS_ASW_COMBINE_BALL;

	else
		return (Class_T)CLASS_ASW_UNKNOWN;
}

static void __MsgFunc_RDAlienKillStat( bf_read &msg )
{
	short iAlienClass = msg.ReadShort();
	CFmtStr szApiName;
	if ( iAlienClass == -1 )
	{
		szApiName.sprintf( "lifetime_alien_kills.%s", "asw_egg" );
	}
	else
	{
		Assert( iAlienClass >= 0 && iAlienClass < NELEMS( g_Aliens ) );
		if ( iAlienClass < 0 || iAlienClass >= NELEMS( g_Aliens ) )
		{
			return;
		}

		szApiName.sprintf( "lifetime_alien_kills.%s", g_Aliens[iAlienClass].m_pszAlienClass );
	}

	int32_t nCount = 0;
	if ( SteamUserStats() && SteamUserStats()->GetStat( szApiName, &nCount ) )
	{
		SteamUserStats()->SetStat( szApiName, nCount + 1 );
	}
	else
	{
		DevMsg( "STEAMSTATS: Failed to retrieve stat %s.\n", szApiName.Access() );
	}
}
USER_MESSAGE_REGISTER( RDAlienKillStat );

extern void CheckDeathTypeCount();

static void __MsgFunc_RDCauseOfDeath( bf_read &msg )
{
	RD_Cause_of_Death_t iCause = ( RD_Cause_of_Death_t )msg.ReadShort();
	Assert( iCause >= DEATHCAUSE_UNKNOWN && iCause < DEATHCAUSE_COUNT );
	if ( iCause < DEATHCAUSE_UNKNOWN || iCause >= DEATHCAUSE_COUNT )
		return;

	const char *szApiName = g_szDeathCauseStatName[iCause];

	int32_t nCount = 0;
	if ( SteamUserStats() && SteamUserStats()->GetStat( szApiName, &nCount ) )
	{
		SteamUserStats()->SetStat( szApiName, nCount + 1 );
	}
	else
	{
		DevMsg( "STEAMSTATS: Failed to retrieve stat %s.\n", szApiName );
	}

	CheckDeathTypeCount();
}
USER_MESSAGE_REGISTER( RDCauseOfDeath );

bool CASW_Steamstats::FetchStats( CSteamID playerSteamID, CASW_Player *pPlayer )
{
	bool bOK = true;

	m_PrimaryEquipCounts.Purge();
	m_SecondaryEquipCounts.Purge();
	m_ExtraEquipCounts.Purge();
	m_MarineSelectionCounts.Purge();
	m_MissionPlayerCounts.Purge();
	m_DifficultyCounts.Purge();
	m_WeaponStats.Purge();

	// Returns true so we don't re-fetch stats
	if( !IsOfficialCampaign() && !IsWorkshopCampaign() )
		return true;

	// Fetch the player's overall stats
	FETCH_STEAM_STATS( "iTotalKills", m_iTotalKills );
	FETCH_STEAM_STATS( "fAccuracy", m_fAccuracy );
	FETCH_STEAM_STATS( "iFriendlyFire", m_iFriendlyFire );
	FETCH_STEAM_STATS( "iDamage", m_iDamage );
	FETCH_STEAM_STATS( "iShotsFired", m_iShotsFired );
	FETCH_STEAM_STATS( "iShotsHit", m_iShotsHit );
	FETCH_STEAM_STATS( "iAliensBurned", m_iAliensBurned );
	FETCH_STEAM_STATS( "iBiomassIgnited", m_iBiomassIgnited );
	FETCH_STEAM_STATS( "iHealing", m_iHealing );
	FETCH_STEAM_STATS( "iFastHacks", m_iFastHacks );
	FETCH_STEAM_STATS( "iGamesTotal", m_iGamesTotal );
	FETCH_STEAM_STATS( "iGamesSuccess", m_iGamesSuccess );
	FETCH_STEAM_STATS( "fGamesSuccessPercent", m_fGamesSuccessPercent );
	FETCH_STEAM_STATS( "ammo_deployed", m_iAmmoDeployed );
	FETCH_STEAM_STATS( "sentryguns_deployed", m_iSentryGunsDeployed );
	FETCH_STEAM_STATS( "sentry_flamers_deployed", m_iSentryFlamerDeployed );
	FETCH_STEAM_STATS( "sentry_freeze_deployed", m_iSentryFreezeDeployed );
	FETCH_STEAM_STATS( "sentry_cannon_deployed", m_iSentryCannonDeployed );
	FETCH_STEAM_STATS( "medkits_used", m_iMedkitsUsed );
	FETCH_STEAM_STATS( "flares_used", m_iFlaresUsed );
	FETCH_STEAM_STATS( "adrenaline_used", m_iAdrenalineUsed );
	FETCH_STEAM_STATS( "tesla_traps_deployed", m_iTeslaTrapsDeployed );
	FETCH_STEAM_STATS( "freeze_grenades_thrown", m_iFreezeGrenadesThrown );
	FETCH_STEAM_STATS( "electric_armor_used", m_iElectricArmorUsed );
	FETCH_STEAM_STATS( "healgun_heals", m_iHealGunHeals );
	FETCH_STEAM_STATS( "healbeacon_heals", m_iHealBeaconHeals );
	FETCH_STEAM_STATS( "healgun_heals_self", m_iHealGunHeals_Self );
	FETCH_STEAM_STATS( "healbeacon_heals_self", m_iHealBeaconHeals_Self );
	FETCH_STEAM_STATS( "damage_amps_used", m_iDamageAmpsUsed );
	FETCH_STEAM_STATS( "healbeacons_deployed", m_iHealBeaconsDeployed );
	FETCH_STEAM_STATS( "medkit_heals_self", m_iMedkitHeals_Self );
	FETCH_STEAM_STATS( "grenade_extinguish_marines", m_iGrenadeExtinguishMarine );
	FETCH_STEAM_STATS( "grenade_freeze_aliens", m_iGrenadeFreezeAlien );
	FETCH_STEAM_STATS( "damage_amp_amps", m_iDamageAmpAmps );
	FETCH_STEAM_STATS( "normal_armor_reduction", m_iNormalArmorReduction );
	FETCH_STEAM_STATS( "electric_armor_reduction", m_iElectricArmorReduction );
	FETCH_STEAM_STATS( "healampgun_heals", m_iHealAmpGunHeals );
	FETCH_STEAM_STATS( "healampgun_amps", m_iHealAmpGunAmps );
	FETCH_STEAM_STATS( "medrifle_heals", m_iMedRifleHeals );
	FETCH_STEAM_STATS( "leadership.procs.accuracy", m_iLeadershipProcsAccuracy );
	FETCH_STEAM_STATS( "leadership.procs.resist", m_iLeadershipProcsResist );
	FETCH_STEAM_STATS( "leadership.damage.accuracy", m_iLeadershipDamageAccuracy );
	FETCH_STEAM_STATS( "leadership.damage.resist", m_iLeadershipDamageResist );
	FETCH_STEAM_STATS( "playtime.total", m_iTotalPlayTime );

	// Fetch starting equip information
	for ( int i = 0; i < ASW_NUM_EQUIP_REGULAR; i++ )
	{
		// Get weapon information
		if ( IsDamagingWeapon( g_ASWEquipmentList.GetRegular( i )->m_EquipClass, false ) )
		{
			int weaponIndex = m_WeaponStats.AddToTail();
			const char *szClassname = g_ASWEquipmentList.GetRegular( i )->m_EquipClass;
			m_WeaponStats[weaponIndex].FetchWeaponStats( playerSteamID, szClassname );
			m_WeaponStats[weaponIndex].m_iWeaponIndex = GetDamagingWeaponClassFromName( szClassname );
			m_WeaponStats[weaponIndex].m_bIsExtra = false;
			m_WeaponStats[weaponIndex].m_szClassName = const_cast< char * >( szClassname );
		}

		// For primary equips
		int32 iTempCount;
		FETCH_STEAM_STATS( CFmtStr( "equips.%s.primary", g_ASWEquipmentList.GetRegular( i )->m_EquipClass ), iTempCount );
		m_PrimaryEquipCounts.AddToTail( iTempCount );

		// For secondary equips
		iTempCount;
		FETCH_STEAM_STATS( CFmtStr( "equips.%s.secondary", g_ASWEquipmentList.GetRegular( i )->m_EquipClass ), iTempCount );
		m_SecondaryEquipCounts.AddToTail( iTempCount );
	}

	for ( int i = 0; i < ASW_NUM_EQUIP_EXTRA; i++ )
	{
		// Get weapon information
		if ( IsDamagingWeapon( g_ASWEquipmentList.GetExtra( i )->m_EquipClass, true ) )
		{
			int weaponIndex = m_WeaponStats.AddToTail();
			const char *szClassname = g_ASWEquipmentList.GetExtra( i )->m_EquipClass;
			m_WeaponStats[weaponIndex].FetchWeaponStats( playerSteamID, szClassname );
			m_WeaponStats[weaponIndex].m_iWeaponIndex = GetDamagingWeaponClassFromName( szClassname );
			m_WeaponStats[weaponIndex].m_bIsExtra = true;
			m_WeaponStats[weaponIndex].m_szClassName = const_cast< char * >( szClassname );
		}

		int32 iTempCount;
		FETCH_STEAM_STATS( CFmtStr( "equips.%s.total", g_ASWEquipmentList.GetExtra( i )->m_EquipClass ), iTempCount );
		m_ExtraEquipCounts.AddToTail( iTempCount );
	}

	// Get weapon stats for rifle grenade and vindicator grenade
	int weaponIndex = m_WeaponStats.AddToTail();
	char *szClassname = "asw_rifle_grenade";
	m_WeaponStats[weaponIndex].FetchWeaponStats( playerSteamID, szClassname );
	m_WeaponStats[weaponIndex].m_iWeaponIndex = GetDamagingWeaponClassFromName( szClassname );
	m_WeaponStats[weaponIndex].m_bIsExtra = false;
	m_WeaponStats[weaponIndex].m_szClassName = szClassname;

	weaponIndex = m_WeaponStats.AddToTail();
	szClassname = "asw_prifle_grenade";
	m_WeaponStats[weaponIndex].FetchWeaponStats( playerSteamID, szClassname );
	m_WeaponStats[weaponIndex].m_iWeaponIndex = GetDamagingWeaponClassFromName( szClassname );
	m_WeaponStats[weaponIndex].m_bIsExtra = false;
	m_WeaponStats[weaponIndex].m_szClassName = szClassname;

	weaponIndex = m_WeaponStats.AddToTail();
	szClassname = "asw_vindicator_grenade";
	m_WeaponStats[weaponIndex].FetchWeaponStats( playerSteamID, szClassname );
	m_WeaponStats[weaponIndex].m_iWeaponIndex = GetDamagingWeaponClassFromName( szClassname );
	m_WeaponStats[weaponIndex].m_bIsExtra = false;
	m_WeaponStats[weaponIndex].m_szClassName = szClassname;

	weaponIndex = m_WeaponStats.AddToTail();
	szClassname = "asw_combat_rifle_shotgun";
	m_WeaponStats[weaponIndex].FetchWeaponStats( playerSteamID, szClassname );
	m_WeaponStats[weaponIndex].m_iWeaponIndex = GetDamagingWeaponClassFromName( szClassname );
	m_WeaponStats[weaponIndex].m_bIsExtra = false;
	m_WeaponStats[weaponIndex].m_szClassName = szClassname;

	weaponIndex = m_WeaponStats.AddToTail();
	szClassname = "prop_combine_ball";
	m_WeaponStats[weaponIndex].FetchWeaponStats( playerSteamID, szClassname );
	m_WeaponStats[weaponIndex].m_iWeaponIndex = GetDamagingWeaponClassFromName( szClassname );
	m_WeaponStats[weaponIndex].m_bIsExtra = false;
	m_WeaponStats[weaponIndex].m_szClassName = szClassname;


	// Fetch marine counts
	for ( int i = 0; i < ASW_NUM_MARINE_PROFILES; i++ )
	{
		int32 iTempCount;
		FETCH_STEAM_STATS( CFmtStr( "marines.%i.total", i ), iTempCount );
		m_MarineSelectionCounts.AddToTail( iTempCount );
		FETCH_STEAM_STATS( CFmtStr( "player_count.%d.missions", i + 1 ), iTempCount );
		m_MissionPlayerCounts.AddToTail( iTempCount );
	}

	// Get difficulty counts
	for ( int i = 0; i < 5; i++ )
	{
		int32 iTempCount;
		FETCH_STEAM_STATS( CFmtStr( "%s.games.total", g_szDifficulties[i] ), iTempCount );
		m_DifficultyCounts.AddToTail( iTempCount );

		// Fetch all stats for that difficulty
		bOK &= m_DifficultyStats[i].FetchDifficultyStats( playerSteamID, i + 1 );
	}

	// Get stats for this mission/difficulty/marine
	bOK &= m_MissionStats.FetchMissionStats( playerSteamID );
	
	return bOK;
}

void CASW_Steamstats::PrepStatsForSend( CASW_Player *pPlayer )
{
	if( !SteamUserStats() )
		return;

	// Update stats from the briefing screen
	if( !GetDebriefStats() 
		|| !ASWGameResource() 
#ifndef DEBUG 
		|| ASWGameRules()->m_bCheated 
#endif
		|| engine->IsPlayingDemo()
		)
		return;

	if ( !IsOfficialCampaign() && !IsWorkshopCampaign() )
		return;

	PrepStatsForSend_Leaderboard( pPlayer, !IsOfficialCampaign() );

	if ( m_MarineSelectionCounts.Count() == 0 ||
		m_MissionPlayerCounts.Count() == 0 ||
		m_DifficultyCounts.Count() == 0 ||
		m_PrimaryEquipCounts.Count() == 0 ||
		m_SecondaryEquipCounts.Count() == 0 ||
		m_ExtraEquipCounts.Count() == 0 )
		return;

	CASW_Marine_Resource *pMR = ASWGameResource()->GetFirstMarineResourceForPlayer( pPlayer );
	if ( !pMR )
		return;
	
	int iMarineIndex = ASWGameResource()->GetMarineResourceIndex( pMR );
	if ( iMarineIndex == -1 )
		return;

	int iMarineProfileIndex = pMR->m_MarineProfileIndex;

	int iPlayersWithMarines = 0;
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		C_ASW_Player *pOtherPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
		if ( pOtherPlayer && ASWGameResource()->GetFirstMarineResourceForPlayer( pOtherPlayer ) )
		{
			iPlayersWithMarines++;
		}
	}

	m_iTotalKills += GetDebriefStats()->GetKills( iMarineIndex );
	m_iFriendlyFire += GetDebriefStats()->GetFriendlyFire( iMarineIndex );
	m_iDamage += GetDebriefStats()->GetDamageTaken( iMarineIndex );
	m_iShotsFired += GetDebriefStats()->GetShotsFired( iMarineIndex );
	m_iShotsHit += GetDebriefStats()->GetShotsHit( iMarineIndex );
	m_fAccuracy = ( m_iShotsFired > 0 ) ? ( m_iShotsHit / (float)m_iShotsFired * 100.0f ) : 0;
	m_iAliensBurned += GetDebriefStats()->GetAliensBurned( iMarineIndex );
	m_iBiomassIgnited += GetDebriefStats()->GetBiomassIgnited( iMarineIndex );
	m_iHealing += GetDebriefStats()->GetHealthHealed( iMarineIndex );
	m_iFastHacks += GetDebriefStats()->GetFastHacks( iMarineIndex );
	m_iGamesTotal++;
	if ( m_DifficultyCounts.IsValidIndex( ASWGameRules()->GetSkillLevel() - 1 ) )
		m_DifficultyCounts[ASWGameRules()->GetSkillLevel() - 1] += 1;
	m_iGamesSuccess += ASWGameRules()->GetMissionSuccess() ? 1 : 0;
	m_fGamesSuccessPercent = m_iGamesSuccess / m_iGamesTotal * 100.0f;
	if ( m_MarineSelectionCounts.IsValidIndex( iMarineProfileIndex ) )
		m_MarineSelectionCounts[iMarineProfileIndex] += 1;
	if ( m_MissionPlayerCounts.IsValidIndex( iPlayersWithMarines - 1 ) )
		m_MissionPlayerCounts[iPlayersWithMarines - 1] += 1;
	m_iAmmoDeployed += GetDebriefStats()->GetAmmoDeployed( iMarineIndex );
	m_iSentryGunsDeployed += GetDebriefStats()->GetSentrygunsDeployed( iMarineIndex );
	m_iSentryFlamerDeployed += GetDebriefStats()->GetSentryFlamersDeployed( iMarineIndex );
	m_iSentryFreezeDeployed += GetDebriefStats()->GetSentryFreezeDeployed( iMarineIndex );
	m_iSentryCannonDeployed += GetDebriefStats()->GetSentryCannonDeployed( iMarineIndex );
	m_iMedkitsUsed += GetDebriefStats()->GetMedkitsUsed( iMarineIndex );
	m_iFlaresUsed += GetDebriefStats()->GetFlaresUsed( iMarineIndex );
	m_iAdrenalineUsed += GetDebriefStats()->GetAdrenalineUsed( iMarineIndex );
	m_iTeslaTrapsDeployed += GetDebriefStats()->GetTeslaTrapsDeployed( iMarineIndex );
	m_iFreezeGrenadesThrown += GetDebriefStats()->GetFreezeGrenadesThrown( iMarineIndex );
	m_iElectricArmorUsed += GetDebriefStats()->GetElectricArmorUsed( iMarineIndex );
	m_iHealGunHeals += GetDebriefStats()->GetHealGunHeals( iMarineIndex );
	m_iHealBeaconHeals += GetDebriefStats()->GetHealBeaconHeals( iMarineIndex );
	m_iHealGunHeals_Self += GetDebriefStats()->GetHealGunHeals_Self( iMarineIndex );
	m_iHealBeaconHeals_Self += GetDebriefStats()->GetHealBeaconHeals_Self( iMarineIndex );
	m_iDamageAmpsUsed += GetDebriefStats()->GetDamageAmpsUsed( iMarineIndex );
	m_iHealBeaconsDeployed += GetDebriefStats()->GetHealBeaconsDeployed( iMarineIndex );
	m_iMedkitHeals_Self += GetDebriefStats()->GetMedkitHeals_Self( iMarineIndex );
	m_iGrenadeExtinguishMarine += GetDebriefStats()->GetGrenadeExtinguishMarine( iMarineIndex );
	m_iGrenadeFreezeAlien += GetDebriefStats()->GetGrenadeFreezeAlien( iMarineIndex );
	m_iDamageAmpAmps += GetDebriefStats()->GetDamageAmpAmps( iMarineIndex );
	m_iNormalArmorReduction += GetDebriefStats()->GetNormalArmorReduction( iMarineIndex );
	m_iElectricArmorReduction += GetDebriefStats()->GetElectricArmorReduction( iMarineIndex );
	m_iHealAmpGunHeals += GetDebriefStats()->GetHealampgunHeals( iMarineIndex );
	m_iHealAmpGunAmps += GetDebriefStats()->GetHealampgunAmps( iMarineIndex );
	m_iMedRifleHeals += GetDebriefStats()->GetMedRifleHeals( iMarineIndex );
	m_iLeadershipProcsAccuracy += GetDebriefStats()->GetLeadershipProcsAccuracy( iMarineIndex );
	m_iLeadershipProcsResist += GetDebriefStats()->GetLeadershipProcsResist( iMarineIndex );
	m_iLeadershipDamageAccuracy += GetDebriefStats()->GetLeadershipDamageAccuracy( iMarineIndex );
	m_iLeadershipDamageResist += GetDebriefStats()->GetLeadershipDamageResist( iMarineIndex );
	m_iTotalPlayTime += (int)GetDebriefStats()->m_fTimeTaken;

	// Get starting equips
	int iPrimaryIndex = GetDebriefStats()->GetStartingPrimaryEquip( iMarineIndex );
	int iSecondaryIndex = GetDebriefStats()->GetStartingSecondaryEquip( iMarineIndex );
	int iExtraIndex = GetDebriefStats()->GetStartingExtraEquip( iMarineIndex );
	CASW_EquipItem *pPrimary = g_ASWEquipmentList.GetRegular( iPrimaryIndex );
	CASW_EquipItem *pSecondary = g_ASWEquipmentList.GetRegular( iSecondaryIndex );
	CASW_EquipItem *pExtra = g_ASWEquipmentList.GetExtra( iExtraIndex );

	Assert( pPrimary && pSecondary && pExtra );

	m_PrimaryEquipCounts[iPrimaryIndex]++;
	m_SecondaryEquipCounts[iSecondaryIndex]++;
	m_ExtraEquipCounts[iExtraIndex]++;

	m_DifficultyStats[ASWGameRules()->GetSkillLevel() - 1].PrepStatsForSend( pPlayer );
	m_MissionStats.PrepStatsForSend( pPlayer );

	// Send player's overall stats
	SEND_STEAM_STATS( "iTotalKills", m_iTotalKills );
	SEND_STEAM_STATS( "fAccuracy", m_fAccuracy );
	SEND_STEAM_STATS( "iFriendlyFire", m_iFriendlyFire );
	SEND_STEAM_STATS( "iDamage", m_iDamage );
	SEND_STEAM_STATS( "iShotsFired", m_iShotsFired );
	SEND_STEAM_STATS( "iShotsHit", m_iShotsHit );
	SEND_STEAM_STATS( "iAliensBurned", m_iAliensBurned );
	SEND_STEAM_STATS( "iBiomassIgnited", m_iBiomassIgnited );
	SEND_STEAM_STATS( "iHealing", m_iHealing );
	SEND_STEAM_STATS( "iFastHacks", m_iFastHacks );
	SEND_STEAM_STATS( "iGamesTotal", m_iGamesTotal );
	SEND_STEAM_STATS( "iGamesSuccess", m_iGamesSuccess );
	SEND_STEAM_STATS( "fGamesSuccessPercent", m_fGamesSuccessPercent );

	SEND_STEAM_STATS( "ammo_deployed", m_iAmmoDeployed );
	SEND_STEAM_STATS( "sentryguns_deployed", m_iSentryGunsDeployed );
	SEND_STEAM_STATS( "sentry_flamers_deployed", m_iSentryFlamerDeployed );
	SEND_STEAM_STATS( "sentry_freeze_deployed", m_iSentryFreezeDeployed );
	SEND_STEAM_STATS( "sentry_cannon_deployed", m_iSentryCannonDeployed );
	SEND_STEAM_STATS( "medkits_used", m_iMedkitsUsed );
	SEND_STEAM_STATS( "flares_used", m_iFlaresUsed );
	SEND_STEAM_STATS( "adrenaline_used", m_iAdrenalineUsed );
	SEND_STEAM_STATS( "tesla_traps_deployed", m_iTeslaTrapsDeployed );
	SEND_STEAM_STATS( "freeze_grenades_thrown", m_iFreezeGrenadesThrown );
	SEND_STEAM_STATS( "electric_armor_used", m_iElectricArmorUsed );
	SEND_STEAM_STATS( "healgun_heals", m_iHealGunHeals );
	SEND_STEAM_STATS( "healbeacon_heals", m_iHealBeaconHeals );
	SEND_STEAM_STATS( "healgun_heals_self", m_iHealGunHeals_Self );
	SEND_STEAM_STATS( "healbeacon_heals_self", m_iHealBeaconHeals_Self );
	SEND_STEAM_STATS( "damage_amps_used", m_iDamageAmpsUsed );
	SEND_STEAM_STATS( "healbeacons_deployed", m_iHealBeaconsDeployed );
	SEND_STEAM_STATS( "medkit_heals_self", m_iMedkitHeals_Self );
	SEND_STEAM_STATS( "grenade_extinguish_marines", m_iGrenadeExtinguishMarine );
	SEND_STEAM_STATS( "grenade_freeze_aliens", m_iGrenadeFreezeAlien );
	SEND_STEAM_STATS( "damage_amp_amps", m_iDamageAmpAmps );
	SEND_STEAM_STATS( "normal_armor_reduction", m_iNormalArmorReduction );
	SEND_STEAM_STATS( "electric_armor_reduction", m_iElectricArmorReduction );
	SEND_STEAM_STATS( "healampgun_heals", m_iHealAmpGunHeals );
	SEND_STEAM_STATS( "healampgun_amps", m_iHealAmpGunAmps );
	SEND_STEAM_STATS( "medrifle_heals", m_iMedRifleHeals );
	SEND_STEAM_STATS( "leadership.procs.accuracy", m_iLeadershipProcsAccuracy );
	SEND_STEAM_STATS( "leadership.procs.resist", m_iLeadershipProcsResist );
	SEND_STEAM_STATS( "leadership.damage.accuracy", m_iLeadershipDamageAccuracy );
	SEND_STEAM_STATS( "leadership.damage.resist", m_iLeadershipDamageResist );
	SEND_STEAM_STATS( "playtime.total", m_iTotalPlayTime );

	if ( pPrimary && m_PrimaryEquipCounts.IsValidIndex( iPrimaryIndex ) )
		SEND_STEAM_STATS( CFmtStr( "equips.%s.primary", pPrimary->m_EquipClass ), m_PrimaryEquipCounts[iPrimaryIndex] );
	if ( pSecondary && m_SecondaryEquipCounts.IsValidIndex( iSecondaryIndex ) )
		SEND_STEAM_STATS( CFmtStr( "equips.%s.secondary", pSecondary->m_EquipClass ), m_SecondaryEquipCounts[iSecondaryIndex] );
	if ( pExtra && m_ExtraEquipCounts.IsValidIndex( iExtraIndex ) )
		SEND_STEAM_STATS( CFmtStr( "equips.%s.total", pExtra->m_EquipClass ), m_ExtraEquipCounts[iExtraIndex] );
	if ( m_MarineSelectionCounts.IsValidIndex( iMarineProfileIndex ) )
		SEND_STEAM_STATS( CFmtStr( "marines.%i.total", iMarineProfileIndex ), m_MarineSelectionCounts[iMarineProfileIndex] );
	if ( m_MissionPlayerCounts.IsValidIndex( iPlayersWithMarines - 1 ) )
		SEND_STEAM_STATS( CFmtStr( "player_count.%i.missions", iPlayersWithMarines ), m_MissionPlayerCounts[iPlayersWithMarines - 1] );
	int iLevel = pPlayer->GetLevel();
	SEND_STEAM_STATS( "level", iLevel );
	int iPromotion = pPlayer->GetPromotion();
	float flXPRequired = ( iLevel == NELEMS( g_iLevelExperience ) ) ? 0 : g_iLevelExperience[ iLevel ];
	flXPRequired *= g_flPromotionXPScale[ iPromotion ];
	SEND_STEAM_STATS( "level.xprequired", (int) flXPRequired );

	// Send favorite equip info
	SEND_STEAM_STATS( "equips.primary.fav", GetFavoriteEquip(0) );
	SEND_STEAM_STATS( "equips.secondary.fav", GetFavoriteEquip(1) );
	SEND_STEAM_STATS( "equips.extra.fav", GetFavoriteEquip(2) );
	SEND_STEAM_STATS( "equips.primary.fav.pct", GetFavoriteEquipPercent(0) );
	SEND_STEAM_STATS( "equips.secondary.fav.pct", GetFavoriteEquipPercent(1) );
	SEND_STEAM_STATS( "equips.extra.fav.pct", GetFavoriteEquipPercent(2) );

	// Send favorite marine info
	SEND_STEAM_STATS( "marines.fav", GetFavoriteMarine() );
	SEND_STEAM_STATS( "marines.class.fav", GetFavoriteMarineClass() );
	SEND_STEAM_STATS( "marines.fav.pct", GetFavoriteMarinePercent() );
	SEND_STEAM_STATS( "marines.class.fav.pct", GetFavoriteMarineClassPercent() );

	// Send favorite difficulty info
	SEND_STEAM_STATS( "difficulty.fav", GetFavoriteDifficulty() );
	SEND_STEAM_STATS( "difficulty.fav.pct", GetFavoriteDifficultyPercent() );

	// Send weapon stats
	for( int i=0; i<m_WeaponStats.Count(); ++i )
	{
		m_WeaponStats[i].PrepStatsForSend( pPlayer );
	}

	int32_t iLastPlayedDay = 0;
	if ( SteamUserStats()->GetStat( "last_played_day", &iLastPlayedDay ) )
	{
		int32_t iToday = SteamUtils()->GetServerRealTime() / 86400u;
		if ( iLastPlayedDay < iToday )
		{
			int32_t iTotalDaysPlayed = 0;
			if ( SteamUserStats()->GetStat( "played_on_days", &iTotalDaysPlayed ) )
			{
				SteamUserStats()->SetStat( "last_played_day", iToday );
				SteamUserStats()->SetStat( "played_on_days", iTotalDaysPlayed + 1 );
			}
		}
	}

	ReactiveDropInventory::CheckPlaytimeItemGenerators( MarineProfileList()->GetProfile( iMarineProfileIndex )->GetMarineClass() );

	char szBetaBranch[256]{};
	if ( SteamInventory() && SteamApps()->GetCurrentBetaName( szBetaBranch, sizeof( szBetaBranch ) ) && !V_stricmp( szBetaBranch, "beta" ) )
	{
		// beta tester medal
		ReactiveDropInventory::AddPromoItem( 13 );
	}
}

int CASW_Steamstats::GetFavoriteEquip( int iSlot )
{
	Assert( iSlot < ASW_MAX_EQUIP_SLOTS );

	StatList_Int_t *pList = NULL;
	if( iSlot == 0 )
		pList = &m_PrimaryEquipCounts;
	else if( iSlot == 1 )
		pList = &m_SecondaryEquipCounts;
	else
		pList = &m_ExtraEquipCounts;

	int iFav = 0;
	for( int i=0; i<pList->Count(); ++i )
	{
		if( pList->operator []( iFav ) < pList->operator [](i) )
			iFav = i;
	}

	return iFav;
}

int CASW_Steamstats::GetFavoriteMarine( void )
{
	int iFav = 0;
	for( int i=0; i<m_MarineSelectionCounts.Count(); ++i )
	{
		if( m_MarineSelectionCounts[ iFav ] < m_MarineSelectionCounts[i] )
			iFav = i;
	}

	return iFav;
}

int CASW_Steamstats::GetFavoriteMarineClass( void )
{
	// Find the marine's class
	CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( GetFavoriteMarine() );
	Assert( pProfile );
	return pProfile ? pProfile->GetMarineClass() : 0;
}

int CASW_Steamstats::GetFavoriteDifficulty( void )
{
	int iFav = 0;
	for( int i=0; i<m_DifficultyCounts.Count(); ++i )
	{
		if( m_DifficultyCounts[ iFav ] < m_DifficultyCounts[i] )
			iFav = i;
	}

	return iFav + 1;
}

float CASW_Steamstats::GetFavoriteEquipPercent( int iSlot )
{
	Assert( iSlot < ASW_MAX_EQUIP_SLOTS );

	StatList_Int_t *pList = NULL;
	if( iSlot == 0 )
		pList = &m_PrimaryEquipCounts;
	else if( iSlot == 1 )
		pList = &m_SecondaryEquipCounts;
	else
		pList = &m_ExtraEquipCounts;

	int iFav = 0;
	float fTotal = 0;
	for( int i=0; i<pList->Count(); ++i )
	{
		fTotal += pList->operator [](i);
		if( iFav < pList->operator [](i) )
			iFav = pList->operator [](i);
	}
	return ( fTotal > 0.0f ) ? ( iFav / fTotal * 100.0f ) : 0.0f;
}

float CASW_Steamstats::GetFavoriteMarinePercent( void )
{
	int iFav = 0;
	float fTotal = 0;
	for( int i=0; i<m_MarineSelectionCounts.Count(); ++i )
	{
		fTotal += m_MarineSelectionCounts[i];
		if( m_MarineSelectionCounts[iFav] < m_MarineSelectionCounts[i] )
			iFav = i;
	}

	return ( fTotal > 0.0f ) ? ( m_MarineSelectionCounts[iFav] / fTotal * 100.0f ) : 0.0f;
}

float CASW_Steamstats::GetFavoriteMarineClassPercent( void )
{
	int iFav = 0;
	float fTotal = 0;
	int iClassCounts[NUM_MARINE_CLASSES] = {0};
	for( int i=0; i<m_MarineSelectionCounts.Count(); ++i )
	{
		// Find the marine's class
		CASW_Marine_Profile *pProfile = MarineProfileList()->GetProfile( i );
		Assert( pProfile );
		if( !pProfile )
			continue;

		int iProfileClass;
		if (pProfile->GetMarineClass() != MARINE_CLASS_UNDEFINED)
		{
			iProfileClass = pProfile->GetMarineClass();
		}
		else
		{
			iProfileClass = MARINE_CLASS_NCO;
		}
		iClassCounts[iProfileClass] += m_MarineSelectionCounts[i];
		fTotal += m_MarineSelectionCounts[i];
		if( iClassCounts[iFav] < iClassCounts[iProfileClass] )
			iFav = iProfileClass;
	}

	return ( fTotal > 0.0f ) ? ( iClassCounts[iFav] / fTotal * 100.0f ) : 0.0f;
}

float CASW_Steamstats::GetFavoriteDifficultyPercent( void )
{
	int iFav = 0;
	float fTotal = 0;
	for( int i=0; i<m_DifficultyCounts.Count(); ++i )
	{
		fTotal += m_DifficultyCounts[i];
		if( iFav < m_DifficultyCounts[i] )
			iFav = m_DifficultyCounts[i];
	}

	return ( fTotal > 0.0f ) ? ( iFav / fTotal * 100.0f ) : 0.0f;
}

bool DifficultyStats_t::FetchDifficultyStats( CSteamID playerSteamID, int iDifficulty )
{
	if( !ASWGameRules() )
		return false;

	bool bOK = true;
	char* szDifficulty = NULL;

	switch( iDifficulty )
	{
		case 1: szDifficulty = "easy"; 
			break;
		case 2: szDifficulty = "normal";
			break;
		case 3: szDifficulty = "hard";
			break;
		case 4: szDifficulty = "insane";
			break;
		case 5: szDifficulty = "imba";
			break;
	}
	if( szDifficulty )
	{
		FETCH_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szGamesTotal ), m_iGamesTotal );
		FETCH_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szGamesSuccess ), m_iGamesSuccess );
		FETCH_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szGamesSuccessPercent ), m_fGamesSuccessPercent );
		FETCH_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szKillsTotal ), m_iKillsTotal );
		FETCH_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szDamageTotal ), m_iDamageTotal );
		FETCH_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szFFTotal ), m_iFFTotal );
		FETCH_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szAccuracy ), m_fAccuracyAvg );
		FETCH_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szShotsHit ), m_iShotsHitTotal );
		FETCH_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szShotsTotal ), m_iShotsFiredTotal );
		FETCH_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szHealingTotal ), m_iHealingTotal );
	}

	return bOK;
}

void DifficultyStats_t::PrepStatsForSend( CASW_Player *pPlayer )
{
	if( !SteamUserStats() )
		return;

	// Update stats from the briefing screen
	if( !GetDebriefStats() || !ASWGameResource() )
		return;

	CASW_Marine_Resource *pMR = ASWGameResource()->GetFirstMarineResourceForPlayer( pPlayer );
	if ( pMR )
	{
		int iMarineIndex = ASWGameResource()->GetMarineResourceIndex( pMR );
		if ( iMarineIndex != -1 )
		{
			m_iKillsTotal += GetDebriefStats()->GetKills( iMarineIndex );
			m_iFFTotal += GetDebriefStats()->GetFriendlyFire( iMarineIndex );
			m_iDamageTotal += GetDebriefStats()->GetDamageTaken( iMarineIndex );
			m_iShotsFiredTotal += GetDebriefStats()->GetShotsFired( iMarineIndex );
			m_iHealingTotal += GetDebriefStats()->GetHealthHealed( iMarineIndex );
			m_iShotsHitTotal += GetDebriefStats()->GetShotsHit( iMarineIndex );
			m_fAccuracyAvg = ( m_iShotsFiredTotal > 0 ) ? ( m_iShotsHitTotal / (float)m_iShotsFiredTotal * 100.0f ) : 0;
			m_iGamesTotal++;
			m_iGamesSuccess += ASWGameRules()->GetMissionSuccess() ? 1 : 0;
			m_fGamesSuccessPercent = m_iGamesSuccess / (float)m_iGamesTotal * 100.0f;
		}
	}
	char* szDifficulty = NULL;
	int iDifficulty = ASWGameRules()->GetSkillLevel();

	switch( iDifficulty )
	{
	case 1: szDifficulty = "easy"; 
		break;
	case 2: szDifficulty = "normal";
		break;
	case 3: szDifficulty = "hard";
		break;
	case 4: szDifficulty = "insane";
		break;
	case 5: szDifficulty = "imba";
		break;
	}
	if( szDifficulty )
	{
		SEND_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szGamesTotal ), m_iGamesTotal );
		SEND_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szGamesSuccess ), m_iGamesSuccess );
		SEND_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szGamesSuccessPercent ), m_fGamesSuccessPercent );
		SEND_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szKillsTotal ), m_iKillsTotal );
		SEND_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szDamageTotal ), m_iDamageTotal );
		SEND_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szFFTotal ), m_iFFTotal );
		SEND_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szAccuracy ), m_fAccuracyAvg );
		SEND_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szShotsHit ), m_iShotsHitTotal );
		SEND_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szShotsTotal ), m_iShotsFiredTotal );
		SEND_STEAM_STATS( CFmtStr( "%s%s", szDifficulty, szHealingTotal ), m_iHealingTotal );
	}
}

bool MissionStats_t::FetchMissionStats( CSteamID playerSteamID )
{
	if ( !IsOfficialCampaign() )
		return true;

	bool bOK = true;
	const char* szLevelName = NULL;
	if( !engine )
		return false;

	szLevelName = engine->GetLevelNameShort();
	if( !szLevelName )
		return false;

	// deathmatch maps don't have per-mission stats
	if ( ASWDeathmatchMode() )
		return true;

	// Fetch stats. Skip the averages, they're write only
	FETCH_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szGamesTotal ), m_iGamesTotal );
	FETCH_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szGamesSuccess ), m_iGamesSuccess );
	FETCH_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szGamesSuccessPercent ), m_fGamesSuccessPercent );
	FETCH_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szKillsTotal ), m_iKillsTotal );
	FETCH_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szDamageTotal ), m_iDamageTotal );
	FETCH_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szFFTotal ), m_iFFTotal );
	FETCH_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szTimeTotal ), m_iTimeTotal );
	FETCH_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szTimeSuccess ), m_iTimeSuccess );
	FETCH_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szBestDifficulty ), m_iHighestDifficulty );
	for( int i=0; i<5; ++i )
	{
		FETCH_STEAM_STATS( CFmtStr( "%s%s.%s", szLevelName, szBestTime, g_szDifficulties[i] ), m_iBestSpeedrunTimes[i] );
	}

	return bOK;
}

void MissionStats_t::PrepStatsForSend( CASW_Player *pPlayer )
{
	// Update stats from the briefing screen
	if( !GetDebriefStats() || !ASWGameResource() )
		return;

	// Custom campaigns don't have manually created stats.
	if ( !IsOfficialCampaign() )
		return;

	// deathmatch maps don't have per-mission stats
	if ( ASWDeathmatchMode() )
		return;

	CASW_Marine_Resource *pMR = ASWGameResource()->GetFirstMarineResourceForPlayer( pPlayer );
	int iDifficulty = ASWGameRules()->GetSkillLevel();
	if ( pMR )
	{
		int iMarineIndex = ASWGameResource()->GetMarineResourceIndex( pMR );
		if ( iMarineIndex != -1 )
		{
			m_iKillsTotal += GetDebriefStats()->GetKills( iMarineIndex );
			m_iFFTotal += GetDebriefStats()->GetFriendlyFire( iMarineIndex );
			m_iDamageTotal += GetDebriefStats()->GetDamageTaken( iMarineIndex );
			m_iGamesTotal++;
			m_iGamesSuccess += ASWGameRules()->GetMissionSuccess() ? 1 : 0;
			m_fGamesSuccessPercent = m_iGamesSuccess / (float)m_iGamesTotal * 100.0f;
			m_iTimeTotal += GetDebriefStats()->m_fTimeTaken;
			if( ASWGameRules()->GetMissionSuccess() )
			{
				m_iTimeSuccess += GetDebriefStats()->m_fTimeTaken;

				if( iDifficulty > m_iHighestDifficulty )
					m_iHighestDifficulty = iDifficulty;

				if( (unsigned int)m_iBestSpeedrunTimes[ iDifficulty - 1 ] > GetDebriefStats()->m_fTimeTaken )
				{
					m_iBestSpeedrunTimes[ iDifficulty - 1 ] = GetDebriefStats()->m_fTimeTaken;
				}
			}
			
			
			// Safely compute averages
			m_fKillsAvg = m_iKillsTotal / (float)m_iGamesTotal;
			m_fFFAvg = m_iFFTotal / (float)m_iGamesTotal;
			m_fDamageAvg = m_iDamageTotal / (float)m_iGamesTotal;
			m_iTimeAvg = m_iTimeTotal / (float)m_iGamesTotal;
		}
	}

	const char* szLevelName = NULL;
	if( !engine )
		return;

	szLevelName = engine->GetLevelNameShort();
	if( !szLevelName )
		return;

	// Send stats
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szGamesTotal ), m_iGamesTotal );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szGamesSuccess ), m_iGamesSuccess );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szGamesSuccessPercent ), m_fGamesSuccessPercent );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szKillsTotal ), m_iKillsTotal );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szDamageTotal ), m_iDamageTotal );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szFFTotal ), m_iFFTotal );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szTimeTotal ), m_iTimeTotal );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szTimeSuccess ), m_iTimeSuccess );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szKillsAvg ), m_fKillsAvg );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szDamageAvg ), m_fDamageAvg );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szFFAvg ), m_fFFAvg );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szTimeAvg ), m_iTimeAvg );
	SEND_STEAM_STATS( CFmtStr( "%s%s", szLevelName, szBestDifficulty ), m_iHighestDifficulty );
	SEND_STEAM_STATS( CFmtStr( "%s%s.%s", szLevelName, szBestTime, g_szDifficulties[ iDifficulty - 1 ] ), m_iBestSpeedrunTimes[ iDifficulty - 1 ] );
}

bool WeaponStats_t::FetchWeaponStats( CSteamID playerSteamID, const char *szClassName )
{
	bool bOK = true;

	const char *szDeathmatchPrefix = "";
	if ( ASWDeathmatchMode() )
	{
		szDeathmatchPrefix = "dm.";
	}

	// Fetch stats. Skip the averages, they're write only
	FETCH_STEAM_STATS( CFmtStr( "%sequips.%s.damage", szDeathmatchPrefix, szClassName ), m_iDamage );
	FETCH_STEAM_STATS( CFmtStr( "%sequips.%s.ff_damage", szDeathmatchPrefix, szClassName ), m_iFFDamage );
	FETCH_STEAM_STATS( CFmtStr( "%sequips.%s.shots_fired", szDeathmatchPrefix, szClassName ), m_iShotsFired );
	FETCH_STEAM_STATS( CFmtStr( "%sequips.%s.shots_hit", szDeathmatchPrefix, szClassName ), m_iShotsHit );
	FETCH_STEAM_STATS( CFmtStr( "%sequips.%s.kills", szDeathmatchPrefix, szClassName ), m_iKills );

	return bOK;
}

void WeaponStats_t::PrepStatsForSend( CASW_Player *pPlayer )
{
	if( !GetDebriefStats() || !ASWGameResource() )
		return;

	// Check to see if the weapon is in the debrief stats
	CASW_Marine_Resource *pMR = ASWGameResource()->GetFirstMarineResourceForPlayer( pPlayer );
	if ( !pMR )
		return;
	
	int iMarineIndex = ASWGameResource()->GetMarineResourceIndex( pMR );
	if ( iMarineIndex == -1 )
		return;

	
	if( !m_szClassName )
		return;
	int iDamage, iFFDamage, iShotsFired, iShotsHit, iKills = 0;

	if( GetDebriefStats()->GetWeaponStats( iMarineIndex, m_iWeaponIndex, iDamage, iFFDamage, iShotsFired, iShotsHit, iKills ) )
	{
		const char *szDeathmatchPrefix = "";
		if ( ASWDeathmatchMode() )
		{
			szDeathmatchPrefix = "dm.";
		}

		SEND_STEAM_STATS( CFmtStr( "%sequips.%s.damage", szDeathmatchPrefix, m_szClassName ), m_iDamage + iDamage );
		SEND_STEAM_STATS( CFmtStr( "%sequips.%s.ff_damage", szDeathmatchPrefix, m_szClassName ), m_iFFDamage + iFFDamage );
		SEND_STEAM_STATS( CFmtStr( "%sequips.%s.shots_fired", szDeathmatchPrefix, m_szClassName ), m_iShotsFired + iShotsFired );
		SEND_STEAM_STATS( CFmtStr( "%sequips.%s.shots_hit", szDeathmatchPrefix, m_szClassName ), m_iShotsHit + iShotsHit );
		SEND_STEAM_STATS( CFmtStr( "%sequips.%s.kills", szDeathmatchPrefix, m_szClassName ), m_iKills + iKills );
	}
}

bool CASW_Steamstats::IsOfficialCampaign()
{
	return ::IsOfficialCampaign();
}

static uint32 GetGameVersion()
{
	static uint32 s_iGameVersion = 0;
	if ( s_iGameVersion == 0 )
	{
		CUtlVectorAutoPurge<char *> productVersionParts;
		V_SplitString( engine->GetProductVersionString(), ".", productVersionParts );

		Assert( productVersionParts.Count() == 4 );
		while ( productVersionParts.Count() < 4 )
		{
			productVersionParts.AddToTail( new char[2]{ '0', '\0' } );
		}

		s_iGameVersion = strtoul( productVersionParts[0], NULL, 10 ) << 24;
		s_iGameVersion |= strtoul( productVersionParts[1], NULL, 10 ) << 16;
		s_iGameVersion |= strtoul( productVersionParts[2], NULL, 10 ) << 8;
		s_iGameVersion |= strtoul( productVersionParts[3], NULL, 10 );
	}

	return s_iGameVersion;
}

void CASW_Steamstats::PrepStatsForSend_Leaderboard( CASW_Player *pPlayer, bool bUnofficial )
{
	if ( !SteamUserStats() || ASWDeathmatchMode() || !ASWGameRules() || !ASWGameResource() || !GetDebriefStats() || engine->IsPlayingDemo() || ASWGameRules()->m_bCheated )
	{
		return;
	}

	if ( !ASWGameRules()->GetMissionSuccess() && !ShouldUploadOnFailure() )
	{
		if ( asw_stats_leaderboard_debug.GetBool() )
		{
			DevWarning( "Not sending leaderboard entry: Mission failed!\n" );
		}
		engine->ServerCmd( "cl_leaderboard_ready\n" );
		return;
	}

	extern ConVar rd_leaderboard_enabled;
	if ( !rd_leaderboard_enabled.GetBool() || !rd_leaderboard_enabled_client.GetBool() )
	{
		if ( asw_stats_leaderboard_debug.GetBool() )
		{
			DevWarning( "Not sending leaderboard entry: rd_leaderboard_enabled is set to 0!\n" );
		}
		engine->ServerCmd( "cl_leaderboard_ready\n" );
		return;
	}

	CASW_Marine_Resource *pMR = ASWGameResource()->GetFirstMarineResourceForPlayer( pPlayer );
	if ( !pMR )
	{
		if ( asw_stats_leaderboard_debug.GetBool() )
		{
			DevWarning( "Not sending leaderboard entry: No marine!\n" );
		}
		engine->ServerCmd( "cl_leaderboard_ready\n" );
		return;
	}

	int iMR = ASWGameResource()->GetMarineResourceIndex( pMR );

	char szMissionFileName[MAX_PATH];
	Q_snprintf( szMissionFileName, sizeof( szMissionFileName ), "resource/overviews/%s.txt", IGameSystem::MapName() );
	PublishedFileId_t nWorkshopFileID = g_ReactiveDropWorkshop.FindAddonProvidingFile( szMissionFileName );
	if ( nWorkshopFileID == k_PublishedFileIdInvalid ? bUnofficial : nWorkshopFileID < 1000000 )
	{
		if ( asw_stats_leaderboard_debug.GetBool() )
		{
			DevWarning( "Not sending leaderboard entry: Unofficial map %s but no workshop ID!\n", IGameSystem::MapName() );
		}
		engine->ServerCmd( "cl_leaderboard_ready\n" );
		return;
	}

	extern ConVar rd_challenge;
	char szChallengeFileName[MAX_PATH];
	Q_snprintf( szChallengeFileName, sizeof( szChallengeFileName ), "resource/challenges/%s.txt", rd_challenge.GetString() );
	PublishedFileId_t nChallengeFileID = g_ReactiveDropWorkshop.FindAddonProvidingFile( szChallengeFileName );
	if ( UTIL_RD_GetCurrentLobbyID().IsValid() )
	{
		const char *pszChallengeFileID = UTIL_RD_GetCurrentLobbyData( "game:challengeinfo:workshop", "" );
		if ( *pszChallengeFileID )
		{
			nChallengeFileID = std::strtoull( pszChallengeFileID, NULL, 16 );
		}
	}

	ELeaderboardSortMethod eSortMethod;
	ELeaderboardDisplayType eDisplayType;
	char szLeaderboardName[k_cchLeaderboardNameMax];
	SpeedRunLeaderboardName( szLeaderboardName, sizeof( szLeaderboardName ), IGameSystem::MapName(), nWorkshopFileID, rd_challenge.GetString(), nChallengeFileID, &eSortMethod, &eDisplayType );
	char szDifficultyLeaderboardName[k_cchLeaderboardNameMax];
	DifficultySpeedRunLeaderboardName( szDifficultyLeaderboardName, sizeof( szDifficultyLeaderboardName ), ASWGameRules()->GetSkillLevel(), IGameSystem::MapName(), nWorkshopFileID, rd_challenge.GetString(), nChallengeFileID );

	if ( asw_stats_leaderboard_debug.GetBool() )
	{
		DevMsg( "Preparing leaderboard entry for leaderboards %s and %s\n", szLeaderboardName, szDifficultyLeaderboardName );
	}

	m_iLeaderboardScore = GetDebriefStats()->m_iLeaderboardScore[iMR];
	m_LeaderboardScoreDetails.m_iVersion = 2;
	m_LeaderboardScoreDetails.m_iMarine = pMR->GetProfileIndex();
	m_LeaderboardScoreDetails.m_iSquadSize = ASWGameResource()->GetNumMarineResources();
	m_LeaderboardScoreDetails.m_iPrimaryWeapon = GetDebriefStats()->m_iStartingEquip0.Get( iMR );
	m_LeaderboardScoreDetails.m_iSecondaryWeapon = GetDebriefStats()->m_iStartingEquip1.Get( iMR );
	m_LeaderboardScoreDetails.m_iExtraWeapon = GetDebriefStats()->m_iStartingEquip2.Get( iMR );
	m_LeaderboardScoreDetails.m_iSquadDead = pMR->GetHealthPercent() == 0 ? 1 : 0;
	int iSquadPosition = 0;
	for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
	{
		C_ASW_Marine_Resource *pSquadMR = ASWGameResource()->GetMarineResource( i );
		if ( pSquadMR && pSquadMR != pMR )
		{
			C_ASW_Player *pCommander = pSquadMR->GetCommander();
			if ( pCommander && ASWGameResource()->GetFirstMarineResourceForPlayer( pCommander ) == pSquadMR )
			{
				m_LeaderboardScoreDetails.m_iSquadMarineSteam[iSquadPosition] = pCommander->GetSteamID().ConvertToUint64();
			}
			else
			{
				m_LeaderboardScoreDetails.m_iSquadMarineSteam[iSquadPosition] = 0;
			}
			m_LeaderboardScoreDetails.m_iSquadMarine[iSquadPosition] = pSquadMR->GetProfileIndex();
			m_LeaderboardScoreDetails.m_iSquadPrimaryWeapon[iSquadPosition] = GetDebriefStats()->m_iStartingEquip0.Get( i );
			m_LeaderboardScoreDetails.m_iSquadSecondaryWeapon[iSquadPosition] = GetDebriefStats()->m_iStartingEquip1.Get( i );
			m_LeaderboardScoreDetails.m_iSquadExtraWeapon[iSquadPosition] = GetDebriefStats()->m_iStartingEquip2.Get( i );

			iSquadPosition++;

			if ( pSquadMR->GetHealthPercent() == 0 )
			{
				m_LeaderboardScoreDetails.m_iSquadDead |= 1 << (unsigned) iSquadPosition;
			}
		}
	}
	for ( int i = iSquadPosition; i < 7; i++ )
	{
		m_LeaderboardScoreDetails.m_iSquadMarineSteam[i] = 0;
		m_LeaderboardScoreDetails.m_iSquadMarine[i] = 0;
		m_LeaderboardScoreDetails.m_iSquadPrimaryWeapon[i] = 0;
		m_LeaderboardScoreDetails.m_iSquadSecondaryWeapon[i] = 0;
		m_LeaderboardScoreDetails.m_iSquadExtraWeapon[i] = 0;
	}
	m_LeaderboardScoreDetails.m_iTimestamp = SteamUtils()->GetServerRealTime();
	const char *pszCountry = SteamUtils()->GetIPCountry();
	m_LeaderboardScoreDetails.m_CountryCode[0] = pszCountry[0];
	m_LeaderboardScoreDetails.m_CountryCode[1] = pszCountry[1];
	m_LeaderboardScoreDetails.m_iDifficulty = ASWGameRules()->GetSkillLevel();
	m_LeaderboardScoreDetails.m_iModeFlags = ( ASWGameRules()->IsOnslaught() ? 1 : 0 ) | ( ASWGameRules()->IsHardcoreFF() ? 2 : 0 ) | ( ASWGameRules()->GetMissionSuccess() ? 0 : 4 );
	m_LeaderboardScoreDetails.m_iGameVersion = GetGameVersion();
	if ( asw_stats_leaderboard_debug.GetBool() )
	{
		DevMsg( "Leaderboard score: %d\n", m_iLeaderboardScore );
		DevMsg( 2, "Leaderboard payload: " );
		const byte *pLeaderboardDetails = reinterpret_cast<const byte *>( &m_LeaderboardScoreDetails );
		for ( int i = 0; i < sizeof( m_LeaderboardScoreDetails ) / sizeof( byte ); i++ )
		{
			DevMsg( 2, "%02x", pLeaderboardDetails[i] );
		}
		DevMsg( 2, "\n" );
	}

	if ( IsLBWhitelisted( szLeaderboardName ) )
	{
		SteamAPICall_t hAPICall = SteamUserStats()->FindOrCreateLeaderboard( szLeaderboardName, eSortMethod, eDisplayType );
		m_LeaderboardFindResultCallback.Set( hAPICall, this, &CASW_Steamstats::LeaderboardFindResultCallback );
	}
	else if ( asw_stats_leaderboard_debug.GetBool() )
	{
		DevMsg( "Not sending leaderboard entry: Not whitelisted %s\n", szLeaderboardName );
	}

	if ( IsLBWhitelisted( szDifficultyLeaderboardName ) )
	{
		SteamAPICall_t hAPICall = SteamUserStats()->FindOrCreateLeaderboard( szDifficultyLeaderboardName, eSortMethod, eDisplayType );
		m_LeaderboardDifficultyFindResultCallback.Set( hAPICall, this, &CASW_Steamstats::LeaderboardDifficultyFindResultCallback );
	}
	else if ( asw_stats_leaderboard_debug.GetBool() )
	{
		DevMsg( "Not sending leaderboard entry: Not whitelisted %s\n", szDifficultyLeaderboardName );
	}
}

void CASW_Steamstats::LeaderboardFindResultCallback( LeaderboardFindResult_t *pResult, bool bIOFailure )
{
	if ( bIOFailure || !pResult->m_bLeaderboardFound )
	{
		engine->ServerCmd( "cl_leaderboard_ready\n" );

		if ( asw_stats_leaderboard_debug.GetBool() )
		{
			DevWarning( "Not sending leaderboard entry: IO:%d Found:%d\n", bIOFailure, pResult ? pResult->m_bLeaderboardFound : false );
		}
		return;
	}

	if ( asw_stats_leaderboard_debug.GetBool() )
	{
		DevMsg( "Sending leaderboard entry to leaderboard ID: %llu\n", pResult->m_hSteamLeaderboard );
	}

	ACTIVE_SPLITSCREEN_PLAYER_GUARD( 0 );
	MissionCompleteFrame *pMissionCompleteFrame = GetClientModeASW() ? assert_cast<MissionCompleteFrame *>( GetClientModeASW()->m_hMissionCompleteFrame.Get() ) : NULL;
	MissionCompletePanel *pMissionCompletePanel = pMissionCompleteFrame ? pMissionCompleteFrame->m_pMissionCompletePanel : NULL;
	if ( pMissionCompletePanel )
	{
		pMissionCompletePanel->OnLeaderboardFound( pResult->m_hSteamLeaderboard );
	}

	SteamAPICall_t hAPICall = SteamUserStats()->UploadLeaderboardScore( pResult->m_hSteamLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest,
		m_iLeaderboardScore, reinterpret_cast<const int32 *>( &m_LeaderboardScoreDetails ), sizeof( m_LeaderboardScoreDetails ) / sizeof( int32 ) );
	m_LeaderboardScoreUploadedCallback.Set( hAPICall, this, &CASW_Steamstats::LeaderboardScoreUploadedCallback );
}

void CASW_Steamstats::LeaderboardDifficultyFindResultCallback( LeaderboardFindResult_t *pResult, bool bIOFailure )
{
	if ( bIOFailure || !pResult->m_bLeaderboardFound )
	{
		if ( asw_stats_leaderboard_debug.GetBool() )
		{
			DevWarning( "Not sending leaderboard entry (difficulty): IO:%d Found:%d\n", bIOFailure, pResult->m_bLeaderboardFound );
		}
		return;
	}

	if ( asw_stats_leaderboard_debug.GetBool() )
	{
		DevMsg( "Sending leaderboard entry to (difficulty) leaderboard ID: %llu\n", pResult->m_hSteamLeaderboard );
	}

	SteamAPICall_t hAPICall = SteamUserStats()->UploadLeaderboardScore( pResult->m_hSteamLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest,
		m_iLeaderboardScore, reinterpret_cast<const int32 *>( &m_LeaderboardScoreDetails ), sizeof( m_LeaderboardScoreDetails ) / sizeof( int32 ) );
	m_LeaderboardDifficultyScoreUploadedCallback.Set( hAPICall, this, &CASW_Steamstats::LeaderboardDifficultyScoreUploadedCallback );
}

void CASW_Steamstats::LeaderboardScoreUploadedCallback( LeaderboardScoreUploaded_t *pResult, bool bIOFailure )
{
	engine->ServerCmd( "cl_leaderboard_ready\n" );

	if ( bIOFailure || !pResult->m_bSuccess )
	{
		if ( asw_stats_leaderboard_debug.GetBool() )
		{
			DevWarning( "Failed to send leaderboard entry: IO:%d Success:%d\n", bIOFailure, pResult ? pResult->m_bSuccess : false );
		}
		return;
	}

	ACTIVE_SPLITSCREEN_PLAYER_GUARD( 0 );
	MissionCompleteFrame *pMissionCompleteFrame = GetClientModeASW() ? assert_cast<MissionCompleteFrame *>( GetClientModeASW()->m_hMissionCompleteFrame.Get() ) : NULL;
	MissionCompletePanel *pMissionCompletePanel = pMissionCompleteFrame ? pMissionCompleteFrame->m_pMissionCompletePanel : NULL;
	if ( pMissionCompletePanel && pResult->m_bScoreChanged )
	{
		RD_LeaderboardEntry_t entry;
		entry.entry.m_steamIDUser = SteamUser()->GetSteamID();
		entry.entry.m_nGlobalRank = pResult->m_nGlobalRankNew;
		entry.entry.m_nScore = pResult->m_nScore;
		entry.entry.m_cDetails = sizeof( LeaderboardScoreDetails_v2_t ) / sizeof( int32 );
		entry.entry.m_hUGC = k_UGCHandleInvalid;
		entry.details.v2 = m_LeaderboardScoreDetails;
		pMissionCompletePanel->OnLeaderboardScoreUploaded( entry, pResult->m_bScoreChanged );
	}

	if ( asw_stats_leaderboard_debug.GetBool() )
	{
		DevMsg( "Leaderboard score uploaded: Score:%d PersonalBest:%d LeaderboardID:%llu GlobalRank(Previous:%d New:%d)\n", pResult->m_nScore, pResult->m_bScoreChanged, pResult->m_hSteamLeaderboard, pResult->m_nGlobalRankPrevious, pResult->m_nGlobalRankNew );
	}
}

void CASW_Steamstats::LeaderboardDifficultyScoreUploadedCallback( LeaderboardScoreUploaded_t *pResult, bool bIOFailure )
{
	if ( bIOFailure || !pResult->m_bSuccess )
	{
		if ( asw_stats_leaderboard_debug.GetBool() )
		{
			DevWarning( "Failed to send leaderboard entry (difficulty): IO:%d Success:%d\n", bIOFailure, pResult ? pResult->m_bSuccess : false );
		}
		return;
	}

	if ( asw_stats_leaderboard_debug.GetBool() )
	{
		DevMsg( "Leaderboard score uploaded (difficulty): Score:%d PersonalBest:%d LeaderboardID:%llu GlobalRank(Previous:%d New:%d)\n", pResult->m_nScore, pResult->m_bScoreChanged, pResult->m_hSteamLeaderboard, pResult->m_nGlobalRankPrevious, pResult->m_nGlobalRankNew );
	}
}

void CASW_Steamstats::SpeedRunLeaderboardName( char *szBuf, size_t bufSize, const char *szMap, PublishedFileId_t nMapID, const char *szChallenge, PublishedFileId_t nChallengeID, ELeaderboardSortMethod *pESortMethod, ELeaderboardDisplayType *pEDisplayType )
{
	char szChallengeLeaderboardName[k_cchLeaderboardNameMax];
	if ( !Q_strcmp( szChallenge, "0" ) )
	{
		Q_snprintf( szChallengeLeaderboardName, sizeof( szChallengeLeaderboardName ), "%llu", nChallengeID );
	}
	else
	{
		Q_snprintf( szChallengeLeaderboardName, sizeof( szChallengeLeaderboardName ), "%llu_%s", nChallengeID, szChallenge );
	}

	if ( pESortMethod )
		*pESortMethod = k_ELeaderboardSortMethodAscending;
	if ( pEDisplayType )
		*pEDisplayType = k_ELeaderboardDisplayTypeTimeMilliSeconds;

	const char *szCategory = "SpeedRun";
	if ( const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( szMap ) )
	{
		if ( pMission->HasTag( "points" ) )
		{
			szCategory = "MapPoints";

			if ( pESortMethod )
				*pESortMethod = k_ELeaderboardSortMethodDescending;
			if ( pEDisplayType )
				*pEDisplayType = k_ELeaderboardDisplayTypeNumeric;
		}
		else if ( pMission->HasTag( "endless" ) )
		{
			if ( pESortMethod )
				*pESortMethod = k_ELeaderboardSortMethodDescending;
		}
	}

	Q_snprintf( szBuf, bufSize, "RD_%s_%s/%llu_%s", szCategory, szChallengeLeaderboardName, nMapID, szMap );
}

void CASW_Steamstats::DifficultySpeedRunLeaderboardName( char *szBuf, size_t bufSize, int iSkill, const char *szMap, PublishedFileId_t nMapID, const char *szChallenge, PublishedFileId_t nChallengeID )
{
	char szChallengeLeaderboardName[k_cchLeaderboardNameMax];
	if ( !Q_strcmp( szChallenge, "0" ) )
	{
		Q_snprintf( szChallengeLeaderboardName, sizeof( szChallengeLeaderboardName ), "%llu", nChallengeID );
	}
	else
	{
		Q_snprintf( szChallengeLeaderboardName, sizeof( szChallengeLeaderboardName ), "%llu_%s", nChallengeID, szChallenge );
	}

	const char *szCategory = "SpeedRun";
	if ( const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( szMap ) )
	{
		if ( pMission->HasTag( "points" ) )
		{
			szCategory = "MapPoints";
		}
	}

	Q_snprintf( szBuf, bufSize, "RD_%s_%s_%s/%llu_%s", g_szDifficulties[iSkill - 1], szCategory, szChallengeLeaderboardName, nMapID, szMap );
}

void CASW_Steamstats::ReadDownloadedLeaderboard( CUtlVector<RD_LeaderboardEntry_t> & entries, SteamLeaderboardEntries_t hEntries, int nCount )
{
	entries.SetCount( nCount );

	for ( int i = 0; i < nCount; i++ )
	{
		SteamUserStats()->GetDownloadedLeaderboardEntry( hEntries, i, &entries[i].entry, reinterpret_cast<int32 *>( &entries[i].details ), sizeof( entries[i].details ) / sizeof( int32 ) );
	}
}

void CASW_Steamstats::ReadDownloadedLeaderboard( CUtlVector<RD_LeaderboardEntry_Points_t> & entries, SteamLeaderboardEntries_t hEntries, int nCount )
{
	entries.SetCount( nCount );

	for (int i = 0; i < nCount; i++)
	{
		SteamUserStats()->GetDownloadedLeaderboardEntry( hEntries, i, &entries[i].entry, reinterpret_cast<int32 *>( &entries[i].details ), sizeof( entries[i].details ) / sizeof( int32 ) );
	}
}


static const char *LB_whitelist[] =
{
	"RD_MapPoints_0/",
	"RD_SpeedRun_0/",
	"RD_imba_SpeedRun_0/",

	"RD_SpeedRun_0_asbi/",
	"RD_imba_SpeedRun_0_asbi/",

	"RD_SpeedRun_0_difficulty_tier1/",
	"RD_imba_SpeedRun_0_difficulty_tier1/",

	"RD_SpeedRun_0_difficulty_tier2/",
	"RD_imba_SpeedRun_0_difficulty_tier2/",

	"RD_SpeedRun_0_energy_weapons/",
	"RD_imba_SpeedRun_0_energy_weapons/",

	"RD_SpeedRun_0_level_one/",
	"RD_imba_SpeedRun_0_level_one/",

	"RD_SpeedRun_0_one_hit/",
	"RD_imba_SpeedRun_0_one_hit/",

	"RD_SpeedRun_0_riflemod_classic/",
	"RD_imba_SpeedRun_0_riflemod_classic/",

	"RD_SpeedRun_1366599495_asbipro/",
	"RD_imba_SpeedRun_1366599495_asbipro/",

	"RD_SpeedRun_1374886583_asb2/",
	"RD_imba_SpeedRun_1374886583_asb2/",

	"RD_SpeedRun_1374886583_asb2_carnage/",
	"RD_imba_SpeedRun_1374886583_asb2_carnage/",

	"RD_SpeedRun_1568035792_asbisolo/",
	"RD_imba_SpeedRun_1568035792_asbisolo/",

	"RD_SpeedRun_1358596669_asbi_classic/",
	"RD_imba_SpeedRun_1358596669_asbi_classic/",

	"RD_SpeedRun_1098363725_vanasbi/",
	"RD_imba_SpeedRun_1098363725_vanasbi/",

	"RD_SpeedRun_1447743649_onehitasbi/",
	"RD_imba_SpeedRun_1447743649_onehitasbi/",

	"RD_SpeedRun_1125436820_single_player/",
	"RD_imba_SpeedRun_1125436820_single_player/",

	"RD_SpeedRun_1429436524_single_player_asbi/",
	"RD_imba_SpeedRun_1429436524_single_player_asbi/",

	"RD_SpeedRun_1274862258_minefield/",
	"RD_imba_SpeedRun_1274862258_minefield/",

	"RD_SpeedRun_1274862258_minefield_light/",
	"RD_imba_SpeedRun_1274862258_minefield_light/",

	"RD_SpeedRun_1274862258_minefield_asbi/",
	"RD_imba_SpeedRun_1274862258_minefield_asbi/",

	"RD_SpeedRun_1274862258_minefieldnotech_asbi/",
	"RD_imba_SpeedRun_1274862258_minefieldnotech_asbi/",

	"RD_SpeedRun_1274862258_minefieldnotech/",
	"RD_imba_SpeedRun_1274862258_minefieldnotech/",

	"RD_SpeedRun_1873361988_strafejumpsair/",
	"RD_imba_SpeedRun_1873361988_strafejumpsair/",

	"RD_SpeedRun_1873361988_strafejumps/",
	"RD_imba_SpeedRun_1873361988_strafejumps/",

	"RD_SpeedRun_1873361988_asbi_strafe_air/",
	"RD_imba_SpeedRun_1873361988_asbi_strafe_air/",

	"RD_SpeedRun_1873361988_asbi_strafe/",
	"RD_imba_SpeedRun_1873361988_asbi_strafe/",

	"RD_SpeedRun_1167497265_asbit1/",
	"RD_imba_SpeedRun_1167497265_asbit1/",

	"RD_SpeedRun_1167497265_asbit1x2/",
	"RD_imba_SpeedRun_1167497265_asbit1x2/",

	"RD_SpeedRun_935767408_asbicarnagex2/",
	"RD_imba_SpeedRun_935767408_asbicarnagex2/",

	"RD_SpeedRun_2082369328_asbi_weapons_balancing_rng/",
	"RD_SpeedRun_2082369328_asbi_weapons_balancing_rng_c2/",
	"RD_SpeedRun_2082369328_asbi_weapons_balancing_rng2/",
	"RD_SpeedRun_2082369328_asbi_weapons_balancing_rng2_c2/",
	"RD_SpeedRun_2082369328_asbi_weapons_balancing_rng3/",
	"RD_SpeedRun_2082369328_asbi_weapons_balancing_rng3_c2/",
	"RD_SpeedRun_2082369328_asbi_weapons_balancing_rng4/",
	"RD_SpeedRun_2082369328_asbi_weapons_balancing_rng4_c2/",

	"RD_SpeedRun_2178770089_turbo/",
	"RD_SpeedRun_2178770089_turboasbi/",
	"RD_SpeedRun_2178770089_turbosingleplayer/",
	"RD_SpeedRun_2178770089_turboasbisingleplayer/",
	"RD_SpeedRun_2178770089_turbo_asbi_wb_rng2/",
	"RD_SpeedRun_2178770089_turbo_asbi_wb_rng2_c2/",

	"RD_SpeedRun_2381921032_asbi2077/",

	"RD_SpeedRun_2461568606_elite/",

	"RD_SpeedRun_2461568606_elite_c2/",

	"RD_SpeedRun_2461568606_asbi_elite/",

	"RD_SpeedRun_2461568606_asbi_elite_c2/",

	"RD_SpeedRun_1940930023_asbi_carnage2_classic/",

	"RD_SpeedRun_2647127742_asbi_ultimate/",

	"RD_SpeedRun_2811007850_campaignexecution/",
	"RD_SpeedRun_2811007850_asbi_campaignexecution/",
};

static bool StartsWith( const char *str, const char *pre )
{
	return Q_strncmp( pre, str, Q_strlen( pre ) ) == 0;
}

bool CASW_Steamstats::IsLBWhitelisted( const char *name )
{
	bool r = false;

	for ( size_t i = 0; i < ARRAYSIZE( LB_whitelist ); ++i )
	{
		if ( StartsWith( name, LB_whitelist[ i ] ) )
		{
			r = true;
			break;
		}
	}

	return r;
}

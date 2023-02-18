#include "cbase.h"
#include "rd_cause_of_death.h"

#ifdef CLIENT_DLL
#else
#include "asw_deathmatch_mode_light.h"
#include "asw_marine.h"
#include "asw_gamerules.h"
#include "asw_sentry_top.h"
#include "props.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// we send death cause as a short; if this fails we need to change how we send it.
ASSERT_INVARIANT( DEATHCAUSE_COUNT <= INT16_MAX );

const char *const g_szDeathCauseStatName[] =
{
	"cause_of_death.unknown",
	"cause_of_death.dm_self",
	"cause_of_death.dm_player",
	"cause_of_death.dm_bot",
	"cause_of_death.dm_other",
	"cause_of_death.physics",
	"cause_of_death.falling",
	"cause_of_death.trigger",
	"cause_of_death.countdown",
	"cause_of_death.command",
	"cause_of_death.ff_while_infested",
	"cause_of_death.infestation",
	"cause_of_death.marine_melee",
	"cause_of_death.marine_buckshot",
	"cause_of_death.marine_bullet",
	"cause_of_death.chainsaw",
	"cause_of_death.zapped",
	"cause_of_death.sentry_placed",
	"cause_of_death.sentry_mapper",
	"cause_of_death.door",
	"cause_of_death.explosive_barrel_self",
	"cause_of_death.explosive_barrel",
	"cause_of_death.radioactive_barrel",
	"cause_of_death.explosive_prop_self",
	"cause_of_death.explosive_prop",
	"cause_of_death.stood_on_alien",
	"cause_of_death.drone_claws",
	"cause_of_death.ranger_spit",
	"cause_of_death.shieldbug_claws",
	"cause_of_death.buzzer_sting",
	"cause_of_death.boomer_kick",
	"cause_of_death.boomer_blob",
	"cause_of_death.biomass_skin",
	"cause_of_death.xenomite_burst",
	"cause_of_death.harvester_skin",
	"cause_of_death.mortarbug_skin",
	"cause_of_death.mortarbug_shell",
	"cause_of_death.queen_tentacles",
	"cause_of_death.queen_claws",
	"cause_of_death.queen_spit",
	"cause_of_death.jacob_toxic_pit",
	"cause_of_death.jacob_turbine",
	"cause_of_death.jacob_nuke",
	"cause_of_death.area9800_bridge",
	"cause_of_death.res7_fan_explode",
	"cause_of_death.res7_rock_explode",
	"cause_of_death.res7_lava",
	"cause_of_death.tft_egg",
	"cause_of_death.tft_laser",
	"cause_of_death.til_hospital_toxin",
	"cause_of_death.lana_trash_compactor",
	"cause_of_death.lana_sewer_mixer",
	"cause_of_death.lana_vent_piston",
	"cause_of_death.lana_nuke",
	"cause_of_death.nh_door_security",
	"cause_of_death.nh_nuke",
	"cause_of_death.bio_vent_hole",
	"cause_of_death.bio_elevator",
	"cause_of_death.bio_fan",
	"cause_of_death.acc32_bridge",
	"cause_of_death.acc32_reactor",
	"cause_of_death.acc32_electrifier",
	"cause_of_death.acc32_forklift",
	"cause_of_death.acc32_crusher",
	"cause_of_death.acc32_doors",
	"cause_of_death.acc32_train",
	"cause_of_death.run_over",
	"cause_of_death.headcrab_classic",
	"cause_of_death.headcrab_fast",
	"cause_of_death.headcrab_poison",
	"cause_of_death.zombine",
	"cause_of_death.zombine_grenade",
	"cause_of_death.zombie_classic",
	"cause_of_death.zombie_classic_torso",
	"cause_of_death.zombie_fast",
	"cause_of_death.zombie_fast_torso",
	"cause_of_death.zombie_poison",
	"cause_of_death.combine_shove",
	"cause_of_death.combine_smg",
	"cause_of_death.combine_shotgun",
	"cause_of_death.combine_ar2",
	"cause_of_death.combine_grenade",
	"cause_of_death.combine_ball",
	"cause_of_death.hunter_flechette",
	"cause_of_death.hunter_flechette_explode",
	"cause_of_death.hunter_charge",
	"cause_of_death.hunter_kick",
	"cause_of_death.antlion_shove",
	"cause_of_death.antlion_worker_shove",
	"cause_of_death.antlion_worker_spit",
	"cause_of_death.antlion_guard_charge",
	"cause_of_death.antlion_guard_shove",
	"cause_of_death.antlion_guardian_charge",
	"cause_of_death.antlion_guardian_shove",
	"cause_of_death.combine_large_synth",
	"cause_of_death.mining_laser",
	"cause_of_death.railgun",
	"cause_of_death.dissolve_other",
	"cause_of_death.firewall",
	"cause_of_death.tripmine",
	"cause_of_death.vindicator_grenade",
	"cause_of_death.rocket",
	"cause_of_death.grenade_gas",
	"cause_of_death.grenade_launcher",
	"cause_of_death.grenade_other",
	"cause_of_death.flamer",
	"cause_of_death.env_explosion",
	"cause_of_death.env_fire",
	"cause_of_death.env_laser",
	"cause_of_death.burn_other",
	"cause_of_death.grenade_launcher_self",
	"cause_of_death.ricochet_bullet",
	"cause_of_death.bleed_out",
};

#ifndef CLIENT_DLL
static const struct
{
	const char *szMapName;
	const char *szEntityName;
	RD_Cause_of_Death_t iDeathCause;
} s_InterestingMapDeaths[] =
{
	{ "asi-jac1-landingbay_02", "hurt_gaspit", DEATHCAUSE_JACOB_TOXIC_PIT },
	{ "asi-jac1-landingbay_pract", "hurt_gaspit", DEATHCAUSE_JACOB_TOXIC_PIT },
	{ "rd-bonus_mission4", "hurt_gaspit", DEATHCAUSE_JACOB_TOXIC_PIT },
	{ "asi-jac6-sewerjunction", "Crush_Trigger_Turbine", DEATHCAUSE_JACOB_TURBINE },
	{ "asi-jac7-timorstation", "countdown_escape", DEATHCAUSE_JACOB_NUKE },
	{ "rd-area9800wl", "BridgeExplosion_07", DEATHCAUSE_AREA9800_BRIDGE },
	{ "rd-res2research7", "fanexp2", DEATHCAUSE_RES7_FAN_EXPLODE },
	{ "rd-res3miningcamp", "tntexplosion", DEATHCAUSE_RES7_ROCK_EXPLODE },
	{ "rd-res3miningcamp", "tntexplosion2", DEATHCAUSE_RES7_ROCK_EXPLODE },
	{ "rd-res4mines", "lava1", DEATHCAUSE_RES7_LAVA },
	{ "rd-res4mines", "lava2", DEATHCAUSE_RES7_LAVA },
	{ "rd-tft1desertoutpost", "egg_rock01_kill_trigger", DEATHCAUSE_TFT_EGG },
	{ "rd-tft2abandonedmaintenance", "injector_laser", DEATHCAUSE_TFT_LASER },
	{ "rd-til9syntekhospital", "objective_count_down", DEATHCAUSE_TIL_HOSPITAL_TOXIN },
	{ "rd-lan1_bridge", "Marine_hurt_wall", DEATHCAUSE_LANA_TRASH_COMPACTOR },
	{ "rd-lan2_sewer", "gordons_hurt_trigger_1", DEATHCAUSE_LANA_SEWER_MIXER },
	{ "rd-lan4_vent", "piston_hurt_area_1", DEATHCAUSE_LANA_VENT_PISTON },
	{ "rd-lan4_vent", "piston_hurt_area_2", DEATHCAUSE_LANA_VENT_PISTON },
	{ "rd-lan5_complex", "Nuclear countdown", DEATHCAUSE_LANA_NUKE },
	{ "rd-nh03_groundworklabs", "LaserGate_*", DEATHCAUSE_NH_DOOR_SECURITY },
	{ "rd-nh03_groundworklabs", "Objectif 5", DEATHCAUSE_NH_NUKE },
	{ "rd-bio1operationx5", "trigger_kill_vent1", DEATHCAUSE_BIO_VENT_HOLE },
	{ "rd-bio1operationx5", "trigger_kill_vent2", DEATHCAUSE_BIO_VENT_HOLE },
	{ "rd-bio2invisiblethreat", "fall_elevator", DEATHCAUSE_BIO_ELEVATOR },
	{ "rd-bio2invisiblethreat", "fall_fan", DEATHCAUSE_BIO_FAN },
	{ "rd-acc3_rescenter", "fall_bridge", DEATHCAUSE_ACC32_BRIDGE },
	{ "rd-acc_complex", "fall_bridge", DEATHCAUSE_ACC32_BRIDGE },
	{ "rd-acc4_confacility", "hurt_reactor", DEATHCAUSE_ACC32_REACTOR },
	{ "rd-acc4_confacility", "hurt_electrifier", DEATHCAUSE_ACC32_ELECTRIFIER },
	{ "rd-acc4_confacility", "hurt_electrifier2", DEATHCAUSE_ACC32_ELECTRIFIER },
	{ "rd-acc6_labruins", "fall_forklift", DEATHCAUSE_ACC32_FORKLIFT },
	{ "rd-acc6_labruins", "hurt_crusher_left", DEATHCAUSE_ACC32_CRUSHER },
	{ "rd-acc6_labruins", "hurt_crusher_right", DEATHCAUSE_ACC32_CRUSHER },
	{ "rd-acc6_labruins", "trigger_hurt_doorblock", DEATHCAUSE_ACC32_DOORS },
	{ "rd-acc6_labruins", "fall_train", DEATHCAUSE_ACC32_TRAIN },
};

static void WarnEntityHelper( const char *szPrefix, CBaseEntity *pEntity )
{
	if ( !pEntity )
	{
		Warning( "  %s (none)\n", szPrefix );
		return;
	}

	Warning( "  %s %s #%d (%s)\n", szPrefix, pEntity->GetClassname(), pEntity->entindex(), pEntity->GetDebugName() );
}

RD_Cause_of_Death_t GetCauseOfDeath( CBaseEntity *pVictim, const CTakeDamageInfo &info )
{
	Assert( pVictim );
	CASW_Marine *pMarineVictim = CASW_Marine::AsMarine( pVictim );
	CASW_Marine *pMarineAttacker = CASW_Marine::AsMarine( info.GetAttacker() );

	if ( ASWDeathmatchMode() )
	{
		if ( pVictim == info.GetAttacker() )
		{
			return DEATHCAUSE_DM_SELF;
		}

		if ( pMarineAttacker )
		{
			return pMarineAttacker->IsInhabited() ? DEATHCAUSE_DM_PLAYER : DEATHCAUSE_DM_BOT;
		}

		return DEATHCAUSE_DM_OTHER;
	}

	if ( ASWGameRules() && ASWGameRules()->m_iMissionWorkshopID == 0 && info.GetAttacker() && info.GetAttacker()->GetEntityName() != NULL_STRING )
	{
		for ( int i = 0; i < NELEMS( s_InterestingMapDeaths ); i++ )
		{
			if ( V_stricmp( STRING( gpGlobals->mapname ), s_InterestingMapDeaths[i].szMapName ) )
			{
				continue;
			}

			if ( info.GetAttacker()->NameMatches( s_InterestingMapDeaths[i].szEntityName ) )
			{
				return s_InterestingMapDeaths[i].iDeathCause;
			}
		}
	}

	if ( pMarineVictim && pMarineVictim->m_bPreventKnockedOut )
	{
		return DEATHCAUSE_BLEED_OUT;
	}

	if ( info.GetDamageType() & DMG_CRUSH )
	{
		return ( info.GetDamageType() & DMG_VEHICLE ) ? DEATHCAUSE_RUN_OVER : DEATHCAUSE_PHYSICS;
	}

	if ( info.GetDamageType() & DMG_FALL )
	{
		return DEATHCAUSE_FALLING;
	}

	if ( info.GetAttacker() && V_stristr( info.GetAttacker()->GetClassname(), "_hurt" ) )
	{
		return DEATHCAUSE_TRIGGER;
	}

	if ( info.GetAttacker() && !V_stricmp( info.GetAttacker()->GetClassname(), "asw_objective_countdown" ) )
	{
		return DEATHCAUSE_COUNTDOWN;
	}

	if ( info.GetDamageType() == DMG_NEVERGIB && pMarineVictim && pMarineVictim == pMarineAttacker )
	{
		return DEATHCAUSE_COMMAND;
	}

	if ( pMarineVictim && pMarineAttacker && pMarineVictim->IsInfested() )
	{
		return DEATHCAUSE_FF_WHILE_INFESTED;
	}

	if ( info.GetDamageType() & DMG_INFEST )
	{
		return DEATHCAUSE_INFESTATION;
	}

	if ( pMarineAttacker && info.GetDamageType() == DMG_CLUB )
	{
		return DEATHCAUSE_MARINE_MELEE;
	}

	if ( pMarineAttacker && info.GetWeapon() && ( info.GetDamageType() & DMG_BUCKSHOT ) )
	{
		return DEATHCAUSE_MARINE_BUCKSHOT;
	}

	if ( info.GetWeapon() && info.GetWeapon()->Classify() == CLASS_ASW_RICOCHET )
	{
		return DEATHCAUSE_RICOCHET_BULLET;
	}

	if ( pMarineAttacker && info.GetWeapon() && ( info.GetDamageType() & DMG_BULLET ) )
	{
		return DEATHCAUSE_MARINE_BULLET;
	}

	if ( info.GetWeapon() && info.GetWeapon()->Classify() == CLASS_ASW_CHAINSAW )
	{
		return DEATHCAUSE_CHAINSAW;
	}

	if ( info.GetDamageType() & DMG_SHOCK )
	{
		return DEATHCAUSE_ZAPPED;
	}

	CASW_Sentry_Top *pSentryTopAttacker = dynamic_cast< CASW_Sentry_Top * >( info.GetAttacker() );
	if ( pSentryTopAttacker )
	{
		return pSentryTopAttacker->GetSentryBase() ? DEATHCAUSE_SENTRY_PLACED : DEATHCAUSE_SENTRY_MAPPER;
	}

	if ( info.GetInflictor() && info.GetInflictor()->Classify() == CLASS_ASW_DOOR )
	{
		return DEATHCAUSE_DOOR;
	}

	if ( info.GetInflictor() && info.GetInflictor()->Classify() == CLASS_ASW_EXPLOSIVE_BARREL )
	{
		return ( pVictim == info.GetAttacker() ) ? DEATHCAUSE_EXPLOSIVE_BARREL_SELF : DEATHCAUSE_EXPLOSIVE_BARREL;
	}

	if ( info.GetInflictor() && info.GetInflictor()->ClassMatches( "asw_radiation_volume" ) )
	{
		if ( info.GetInflictor()->GetMoveParent() && info.GetInflictor()->GetMoveParent()->ClassMatches( "asw_barrel_radioactive" ) )
			return DEATHCAUSE_RADIOACTIVE_BARREL;
		Assert( info.GetWeapon() );
		return DEATHCAUSE_GRENADE_GAS;
	}

	if ( ( info.GetDamageType() & DMG_BLAST ) && info.GetInflictor() && dynamic_cast< CBaseProp * >( info.GetInflictor() ) )
	{
		return ( pVictim == info.GetAttacker() ) ? DEATHCAUSE_EXPLOSIVE_PROP_SELF : DEATHCAUSE_EXPLOSIVE_PROP;
	}

	if ( pMarineVictim && pMarineVictim->GetGroundEntity() && pMarineVictim->GetGroundEntity() == info.GetAttacker() && ( info.GetDamageType() & DMG_SLASH ) )
	{
		return DEATHCAUSE_STOOD_ON_ALIEN;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_DRONE && ( info.GetDamageType() & DMG_SLASH ) )
	{
		return DEATHCAUSE_DRONE_CLAWS;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_RANGER )
	{
		return DEATHCAUSE_RANGER_SPIT;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_SHIELDBUG && ( info.GetDamageType() & DMG_SLASH ) )
	{
		return DEATHCAUSE_SHIELDBUG_CLAWS;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_BUZZER )
	{
		return DEATHCAUSE_BUZZER_STING;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_BOOMER && ( info.GetDamageType() & DMG_SLASH ) )
	{
		return DEATHCAUSE_BOOMER_KICK;
	}

	if ( info.GetInflictor() && info.GetInflictor()->Classify() == CLASS_ASW_BOOMER_BLOB )
	{
		return DEATHCAUSE_BOOMER_BLOB;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_ALIEN_GOO && ( info.GetDamageType() & DMG_ACID ) )
	{
		return DEATHCAUSE_BIOMASS_SKIN;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_PARASITE && ( info.GetDamageType() & DMG_ACID ) )
	{
		return DEATHCAUSE_XENOMITE_BURST;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_HARVESTER && ( info.GetDamageType() & DMG_SLASH ) )
	{
		return DEATHCAUSE_HARVESTER_SKIN;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_MORTAR_BUG && ( info.GetDamageType() & DMG_SLASH ) )
	{
		return DEATHCAUSE_MORTARBUG_SKIN;
	}

	if ( info.GetInflictor() && info.GetInflictor()->Classify() == CLASS_ASW_MORTAR_SHELL )
	{
		return DEATHCAUSE_MORTARBUG_SHELL;
	}

	if ( info.GetInflictor() && info.GetInflictor()->Classify() == CLASS_ASW_QUEEN_GRABBER )
	{
		return DEATHCAUSE_QUEEN_TENTACLES;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_QUEEN && ( info.GetDamageType() & DMG_SLASH ) )
	{
		return DEATHCAUSE_QUEEN_CLAWS;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_QUEEN && ( info.GetDamageType() & DMG_ACID ) )
	{
		return DEATHCAUSE_QUEEN_SPIT;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_HEADCRAB )
	{
		if ( info.GetAttacker()->ClassMatches( "npc_headcrab" ) )
			return DEATHCAUSE_HEADCRAB_CLASSIC;
		if ( info.GetAttacker()->ClassMatches( "npc_headcrab_fast" ) )
			return DEATHCAUSE_HEADCRAB_FAST;
		Assert( info.GetAttacker()->ClassMatches( "npc_headcrab_poison" ) );
		return DEATHCAUSE_HEADCRAB_POISON;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ZOMBIE )
	{
		if ( info.GetAttacker()->ClassMatches( "npc_zombine" ) )
			return ( info.GetDamageType() & DMG_BLAST ) ? DEATHCAUSE_ZOMBINE_GRENADE : DEATHCAUSE_ZOMBINE;
		if ( info.GetAttacker()->ClassMatches( "npc_zombie" ) )
			return DEATHCAUSE_ZOMBIE_CLASSIC;
		if ( info.GetAttacker()->ClassMatches( "npc_zombie_torso" ) )
			return DEATHCAUSE_ZOMBIE_CLASSIC_TORSO;
		if ( info.GetAttacker()->ClassMatches( "npc_fastzombie" ) )
			return DEATHCAUSE_ZOMBIE_FAST;
		if ( info.GetAttacker()->ClassMatches( "npc_fastzombie_torso" ) )
			return DEATHCAUSE_ZOMBIE_FAST_TORSO;
		Assert( info.GetAttacker()->ClassMatches( "npc_poisonzombie" ) );
		return DEATHCAUSE_ZOMBIE_POISON;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_COMBINE )
	{
		if ( info.GetDamageType() & DMG_CLUB )
			return DEATHCAUSE_COMBINE_SHOVE;
		if ( info.GetDamageType() & DMG_DISSOLVE )
			return DEATHCAUSE_COMBINE_BALL;
		if ( info.GetDamageType() & DMG_BLAST )
			return DEATHCAUSE_COMBINE_GRENADE;
		if ( info.GetDamageType() & DMG_BUCKSHOT )
			return DEATHCAUSE_COMBINE_SHOTGUN;
		Assert( info.GetDamageType() & DMG_BULLET );
		if ( info.GetWeapon() && info.GetWeapon()->Classify() == CLASS_ASW_AR2 )
			return DEATHCAUSE_COMBINE_AR2;
		return DEATHCAUSE_COMBINE_SMG;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_COMBINE_HUNTER )
	{
		if ( info.GetDamageType() & DMG_DISSOLVE )
			return ( info.GetDamageType() & DMG_NEVERGIB ) ? DEATHCAUSE_HUNTER_FLECHETTE : DEATHCAUSE_HUNTER_FLECHETTE_EXPLODE;
		if ( info.GetDamageType() & DMG_CLUB )
			return DEATHCAUSE_HUNTER_CHARGE;
		Assert( info.GetDamageType() & DMG_SLASH );
		return DEATHCAUSE_HUNTER_KICK;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ANTLION )
	{
		if ( info.GetAttacker()->ClassMatches( "npc_antlion_worker" ) )
			return ( info.GetDamageType() & ( DMG_ACID | DMG_POISON ) ) ? DEATHCAUSE_ANTLION_WORKER_SPIT : DEATHCAUSE_ANTLION_WORKER_SHOVE;
		Assert( info.GetAttacker()->ClassMatches( "npc_antlion" ) );
		return DEATHCAUSE_ANTLION_SHOVE;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_ANTLIONGUARD )
	{
		if ( info.GetAttacker()->ClassMatches( "npc_antlionguard_cavern" ) )
			return ( info.GetDamageType() & DMG_CLUB ) ? DEATHCAUSE_ANTLION_GUARDIAN_CHARGE : DEATHCAUSE_ANTLION_GUARDIAN_SHOVE;
		Assert( info.GetAttacker()->ClassMatches( "npc_antlionguard_normal" ) );
		return ( info.GetDamageType() & DMG_CLUB ) ? DEATHCAUSE_ANTLION_GUARD_CHARGE : DEATHCAUSE_ANTLION_GUARD_SHOVE;
	}

	if ( info.GetAttacker() && ( info.GetAttacker()->Classify() == CLASS_COMBINE_STRIDER || info.GetAttacker()->Classify() == CLASS_COMBINE_HUNTER ) )
	{
		return DEATHCAUSE_COMBINE_LARGE_SYNTH;
	}

	if ( info.GetWeapon() && info.GetWeapon()->Classify() == CLASS_ASW_MINING_LASER )
	{
		return DEATHCAUSE_MINING_LASER;
	}

	if ( info.GetWeapon() && info.GetWeapon()->Classify() == CLASS_ASW_RAILGUN )
	{
		return DEATHCAUSE_RAILGUN;
	}

	if ( info.GetDamageType() & DMG_DISSOLVE )
	{
		return DEATHCAUSE_DISSOLVE_OTHER;
	}

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_FIRE )
	{
		return DEATHCAUSE_ENV_FIRE;
	}

	if ( info.GetWeapon() && info.GetWeapon()->Classify() == CLASS_ASW_MINES )
	{
		return DEATHCAUSE_FIREWALL;
	}

	if ( info.GetInflictor() && info.GetInflictor()->Classify() == CLASS_ASW_LASER_MINE_PROJECTILE )
	{
		return DEATHCAUSE_TRIPMINE;
	}

	if ( info.GetWeapon() && info.GetWeapon()->Classify() == CLASS_ASW_ASSAULT_SHOTGUN )
	{
		Assert( info.GetDamageType() & DMG_BURN );
		return DEATHCAUSE_VINDICATOR_GRENADE;
	}

	if ( info.GetWeapon() && info.GetWeapon()->Classify() == CLASS_ASW_FLAMER )
	{
		return DEATHCAUSE_FLAMER;
	}

	if ( info.GetInflictor() && info.GetInflictor()->Classify() == CLASS_MISSILE )
	{
		return DEATHCAUSE_ROCKET;
	}

	if ( info.GetWeapon() && ( info.GetDamageType() & DMG_BLAST ) )
	{
		if ( info.GetWeapon()->Classify() == CLASS_ASW_GRENADE_LAUNCHER )
			return pVictim == info.GetAttacker() ? DEATHCAUSE_GRENADE_LAUNCHER_SELF : DEATHCAUSE_GRENADE_LAUNCHER;
		return DEATHCAUSE_GRENADE_OTHER;
	}

	if ( info.GetAttacker() && info.GetAttacker()->ClassMatches( "env_laser" ) )
	{
		return DEATHCAUSE_ENV_LASER;
	}

	if ( info.GetAttacker() && ( info.GetAttacker()->ClassMatches( "env_explosion" ) || info.GetAttacker()->ClassMatches( "asw_env_explosion" ) ) )
	{
		return DEATHCAUSE_ENV_EXPLOSION;
	}

	if ( info.GetDamageType() & DMG_BURN )
	{
		return DEATHCAUSE_BURN_OTHER;
	}

	Warning( "Could not determine cause of death from damage info:\n" );
	Warning( "  map %s\n", STRING( gpGlobals->mapname ) );
	WarnEntityHelper( "victim", pVictim );
	WarnEntityHelper( "attacker", info.GetAttacker() );
	WarnEntityHelper( "weapon", info.GetWeapon() );
	WarnEntityHelper( "inflictor", info.GetInflictor() );
	char buf[512]{};
	CTakeDamageInfo::DebugGetDamageTypeString( info.GetDamageType(), buf, sizeof( buf ) );
	Warning( "  damage type %s\n", buf );
	Warning( "\n" );

	Assert( !"damage info did not resolve to a death cause" );
	return DEATHCAUSE_UNKNOWN;
}
#endif

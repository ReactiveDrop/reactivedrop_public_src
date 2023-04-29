#include "cbase.h"
#include "asw_equipment_list.h"
#include "filesystem.h"
#include "weapon_parse.h"
#include <KeyValues.h>
#ifdef CLIENT_DLL
	#include <vgui/ISurface.h>
	#include <vgui/ISystem.h>
	#include <vgui_controls/Panel.h>
#endif
#include "ammodef.h"
#include "asw_weapon_parse.h"
#include "asw_gamerules.h"
#include "asw_util_shared.h"
#include "gamestringpool.h"
#include "rd_inventory_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_EquipItem::CASW_EquipItem( int iItemIndex, const char *szEquipClass, const char *szShortName, const char *szLongName,
	const char *szDescription1, const char *szAltFireDescription, const char *szAttributeDescription,
	bool bSelectableInBriefing, bool bIsExtra, const char *szAmmo1, const char *szAmmo2,
	const char *szEquipIcon, ConVar *pDefaultAmmo1, ConVar *pMaxAmmo1, ConVar *pDefaultAmmo2, ConVar *pMaxAmmo2,
	int iRequiredClass, bool bIsUnique )
	: m_iItemIndex{ iItemIndex },
	m_iRequiredClass{ iRequiredClass },
	m_pDefaultAmmo1{ pDefaultAmmo1 },
	m_pMaxAmmo1{ pMaxAmmo1 },
	m_pDefaultAmmo2{ pDefaultAmmo2 },
	m_pMaxAmmo2{ pMaxAmmo2 },
	m_szEquipClass{ szEquipClass },
	m_szShortName{ szShortName },
	m_szLongName{ szLongName },
	m_szDescription1{ szDescription1 },
	m_szAltFireDescription{ szAltFireDescription },
	m_szAttributeDescription{ szAttributeDescription },
	m_szAmmo1{ szAmmo1 },
	m_szAmmo2{ szAmmo2 },
	m_szEquipIcon{ szEquipIcon },
	m_bSelectableInBriefing{ bSelectableInBriefing },
	m_bIsExtra{ bIsExtra },
	m_bIsUnique{ bIsUnique }
{
	m_EquipClass = NULL_STRING;
}

int CASW_EquipItem::DefaultAmmo1() const
{
	return m_pDefaultAmmo1 ? m_pDefaultAmmo1->GetInt() : WEAPON_NOCLIP;
}
int CASW_EquipItem::MaxAmmo1() const
{
	return m_pMaxAmmo1 ? m_pMaxAmmo1->GetInt() : WEAPON_NOCLIP;
}
int CASW_EquipItem::DefaultAmmo2() const
{
	return m_pDefaultAmmo2 ? m_pDefaultAmmo2->GetInt() : WEAPON_NOCLIP;
}
int CASW_EquipItem::MaxAmmo2() const
{
	return m_pMaxAmmo2 ? m_pMaxAmmo2->GetInt() : WEAPON_NOCLIP;
}

void CASW_EquipItem::LevelInitPreEntity()
{
	m_iAmmo1 = *m_szAmmo1 ? GetAmmoDef()->Index( m_szAmmo1 ) : -1;
	m_iAmmo2 = *m_szAmmo2 ? GetAmmoDef()->Index( m_szAmmo2 ) : -1;
	Assert( !*m_szAmmo1 || m_iAmmo1 != -1 );
	Assert( !*m_szAmmo2 || m_iAmmo2 != -1 );

	m_EquipClass = AllocPooledString( m_szEquipClass );
}

ConVar asw_ammo_count_internal_sentry( "asw_ammo_count_internal_sentry", "1", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );
ConVar asw_ammo_count_internal_chainsaw( "asw_ammo_count_internal_chainsaw", "111", FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED );

ConVar asw_ammo_count_rifle( "asw_ammo_count_rifle", "98", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_rifle_grenade( "asw_ammo_count_rifle_grenade", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_rifle_grenade_max( "asw_ammo_count_rifle_grenade_max", "8", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_prifle_grenade( "asw_ammo_count_prifle_grenade", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_prifle_grenade_max( "asw_ammo_count_prifle_grenade_max", "8", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_autogun( "asw_ammo_count_autogun", "250", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_vindicator( "asw_ammo_count_vindicator", "14", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_vindicator_grenade( "asw_ammo_count_vindicator_grenade", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_vindicator_grenade_max( "asw_ammo_count_vindicator_grenade_max", "8", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_pistol( "asw_ammo_count_pistol", "24", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_heal_beacon( "asw_ammo_count_heal_beacon", "9", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_ammo_satchel( "asw_ammo_count_ammo_satchel", "3", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_shotgun( "asw_ammo_count_shotgun", "4", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_tesla_gun( "asw_ammo_count_tesla_gun", "20", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_railgun( "asw_ammo_count_railgun", "1", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_heal_gun( "asw_ammo_count_heal_gun", "100", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_pdw( "asw_ammo_count_pdw", "80", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_flamer( "asw_ammo_count_flamer", "80", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_sniper_rifle( "asw_ammo_count_sniper_rifle", "12", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_grenade_launcher( "asw_ammo_count_grenade_launcher", "6", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_deagle( "asw_ammo_count_deagle", "7", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_devastator( "asw_ammo_count_devastator", "70", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_combat_rifle_shotgun( "asw_ammo_count_combat_rifle_shotgun", "8", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_healampgun_heal( "asw_ammo_count_healampgun_heal", "50", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_healampgun_amp( "asw_ammo_count_healampgun_amp", "100", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_heavy_rifle( "asw_ammo_count_heavy_rifle", "98", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_heavy_rifle_charge( "asw_ammo_count_heavy_rifle_charge", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_medrifle( "asw_ammo_count_medrifle", "72", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_medrifle_heal( "asw_ammo_count_medrifle_heal", "50", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_fire_extinguisher( "asw_ammo_count_fire_extinguisher", "200", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_mining_laser( "asw_ammo_count_mining_laser", "50", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_50calmg( "asw_ammo_count_50calmg", "200", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_flechette( "asw_ammo_count_flechette", "60", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_medsatchel( "asw_ammo_count_medsatchel", "9", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_ar2( "asw_ammo_count_ar2", "30", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_ar2_grenade( "asw_ammo_count_ar2_grenade", "3", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_cryo_cannon( "asw_ammo_count_cryo_cannon", "200", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_energy_shield( "asw_ammo_count_energy_shield", "8", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_medkit( "asw_ammo_count_medkit", "1", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_flares( "asw_ammo_count_flares", "15", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_laser_mines( "asw_ammo_count_laser_mines", "12", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_buff_grenade( "asw_ammo_count_buff_grenade", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_hornet_barrage( "asw_ammo_count_hornet_barrage", "3", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_freeze_grenades( "asw_ammo_count_freeze_grenades", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_stim( "asw_ammo_count_stim", "3", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_tesla_trap( "asw_ammo_count_tesla_trap", "3", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_electrified_armor( "asw_ammo_count_electrified_armor", "3", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_mines( "asw_ammo_count_mines", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_grenades( "asw_ammo_count_grenades", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_smart_bomb( "asw_ammo_count_smart_bomb", "1", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_gas_grenades( "asw_ammo_count_gas_grenades", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_t75( "asw_ammo_count_t75", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_jump_jet( "asw_ammo_count_jump_jet", "5", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_bait( "asw_ammo_count_bait", "9", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_stun_grenades( "asw_ammo_count_stun_grenades", "10", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_incendiary_grenades( "asw_ammo_count_incendiary_grenades", "7", FCVAR_CHEAT | FCVAR_REPLICATED );
ConVar asw_ammo_count_shield_bubble( "asw_ammo_count_shield_bubble", "3", FCVAR_CHEAT | FCVAR_REPLICATED );

#define WEAPON_NAME( name ) "asw_weapon_" #name, "#asw_weapon_" #name, "#asw_weaponl_" #name, "#asw_wdesc_" #name, "#asw_weapon_" #name "_altfire", "#asw_weapon_" #name "_attributes"

static CASW_EquipItem s_RegularEquips[ASW_NUM_EQUIP_REGULAR] =
{
	// default weapons
	{
		ASW_EQUIP_RIFLE, WEAPON_NAME( rifle ),
		true, false, "ASW_R", "ASW_R_G",
		"swarm/EquipIcons/EquipRifle",
		&asw_ammo_count_rifle, &asw_ammo_count_rifle,
		&asw_ammo_count_rifle_grenade, &asw_ammo_count_rifle_grenade_max,
	},
	{
		ASW_EQUIP_PRIFLE, WEAPON_NAME( prifle ),
		true, false, "ASW_R", "ASW_R_G",
		"swarm/EquipIcons/EquipPRifle",
		&asw_ammo_count_rifle, &asw_ammo_count_rifle,
		&asw_ammo_count_prifle_grenade, &asw_ammo_count_prifle_grenade_max,
		MARINE_CLASS_TECH,
	},
	{
		ASW_EQUIP_AUTOGUN, WEAPON_NAME( autogun ),
		true, false, "ASW_AG", "",
		"swarm/EquipIcons/EquipAutogun",
		&asw_ammo_count_autogun, &asw_ammo_count_autogun,
		NULL, NULL,
		MARINE_CLASS_SPECIAL_WEAPONS, true,
	},
	{
		ASW_EQUIP_VINDICATOR, WEAPON_NAME( vindicator ),
		true, false, "ASW_ASG", "ASW_ASG_G",
		"swarm/EquipIcons/EquipShotgunAssault",
		&asw_ammo_count_vindicator, &asw_ammo_count_vindicator,
		&asw_ammo_count_vindicator_grenade, &asw_ammo_count_vindicator_grenade_max,
		MARINE_CLASS_NCO,
	},
	{
		ASW_EQUIP_PISTOL, WEAPON_NAME( pistol ),
		true, false, "ASW_P", "",
		"swarm/EquipIcons/EquipPistol",
		&asw_ammo_count_pistol, &asw_ammo_count_pistol,
		NULL,NULL,
	},
	{
		ASW_EQUIP_SENTRY, WEAPON_NAME( sentry ),
		true, false, "", "",
		"swarm/EquipIcons/EquipSentry",
		&asw_ammo_count_internal_sentry, &asw_ammo_count_internal_sentry,
		NULL,NULL,
	},
	{
		ASW_EQUIP_HEAL_GRENADE, WEAPON_NAME( heal_grenade ),
		true, false, "", "",
		"swarm/EquipIcons/EquipHealGrenade",
		&asw_ammo_count_heal_beacon, &asw_ammo_count_heal_beacon,
		NULL, NULL,
		MARINE_CLASS_MEDIC,
	},
	{
		ASW_EQUIP_AMMO_SATCHEL, WEAPON_NAME( ammo_satchel ),
		true, false, "", "",
		"swarm/EquipIcons/EquipAmmoSatchel",
		&asw_ammo_count_ammo_satchel, &asw_ammo_count_ammo_satchel,
		NULL,NULL,
	},

	// level-unlockable weapons
	{
		ASW_EQUIP_SHOTGUN, WEAPON_NAME( shotgun ),
		true, false, "ASW_SG", "",
		"swarm/EquipIcons/EquipShotgun",
		&asw_ammo_count_shotgun, &asw_ammo_count_shotgun,
		NULL, NULL,
	},
	{
		ASW_EQUIP_TESLA_GUN, WEAPON_NAME( tesla_gun ),
		true, false, "ASW_TG", "",
		"swarm/EquipIcons/EquipTeslaGun",
		&asw_ammo_count_tesla_gun, &asw_ammo_count_tesla_gun,
		NULL, NULL,
	},
	{
		ASW_EQUIP_RAILGUN, WEAPON_NAME( railgun ),
		true, false, "ASW_RG", "",
		"swarm/EquipIcons/EquipRailgun",
		&asw_ammo_count_railgun, &asw_ammo_count_railgun,
		NULL, NULL,
	},
	{
		ASW_EQUIP_HEAL_GUN, WEAPON_NAME( heal_gun ),
		true, false, "", "",
		"swarm/EquipIcons/EquipHealGun",
		&asw_ammo_count_heal_gun, &asw_ammo_count_heal_gun,
		NULL, NULL,
		MARINE_CLASS_MEDIC, true,
	},
	{
		ASW_EQUIP_PDW, WEAPON_NAME( pdw ),
		true, false, "ASW_PDW", "",
		"swarm/EquipIcons/EquipPDW",
		&asw_ammo_count_pdw, &asw_ammo_count_pdw,
		NULL, NULL,
	},
	{
		ASW_EQUIP_FLAMER, WEAPON_NAME( flamer ),
		true, false, "ASW_F", "",
		"swarm/EquipIcons/EquipFlamer",
		&asw_ammo_count_flamer, &asw_ammo_count_flamer,
		NULL, NULL,
	},
	{
		ASW_EQUIP_SENTRY_FREEZE, WEAPON_NAME( sentry_freeze ),
		true, false, "", "",
		"swarm/EquipIcons/EquipSentryFreeze",
		&asw_ammo_count_internal_sentry, &asw_ammo_count_internal_sentry,
		NULL, NULL,
	},
	{
		ASW_EQUIP_MINIGUN, WEAPON_NAME( minigun ),
		true, false, "ASW_AG", "",
		"swarm/EquipIcons/EquipMiniGun",
		&asw_ammo_count_autogun, &asw_ammo_count_autogun, // minigun has to use the same ammo count as autogun because they share an ammo type - it doubles the numbers visually and keeps track of half-bullets
		NULL, NULL,
		MARINE_CLASS_SPECIAL_WEAPONS, true,
	},
	{
		ASW_EQUIP_SNIPER_RIFLE, WEAPON_NAME( sniper_rifle ),
		true, false, "ASW_SNIPER", "",
		"swarm/EquipIcons/EquipSniperRifle",
		&asw_ammo_count_sniper_rifle, &asw_ammo_count_sniper_rifle,
		NULL, NULL,
	},
	{
		ASW_EQUIP_SENTRY_FLAMER, WEAPON_NAME( sentry_flamer ),
		true, false, "", "",
		"swarm/EquipIcons/EquipSentryFlamer",
		&asw_ammo_count_internal_sentry, &asw_ammo_count_internal_sentry,
		NULL, NULL,
	},
	{
		ASW_EQUIP_CHAINSAW, WEAPON_NAME( chainsaw ),
		true, false, "", "",
		"swarm/EquipIcons/EquipChainsaw",
		&asw_ammo_count_internal_chainsaw, &asw_ammo_count_internal_chainsaw,
		NULL, NULL,
	},
	{
		ASW_EQUIP_SENTRY_CANNON, WEAPON_NAME( sentry_cannon ),
		true, false, "", "",
		"swarm/EquipIcons/EquipSentryCannon",
		&asw_ammo_count_internal_sentry, &asw_ammo_count_internal_sentry,
		NULL, NULL,
	},
	{
		ASW_EQUIP_GRENADE_LAUNCHER, WEAPON_NAME( grenade_launcher ),
		true, false, "ASW_GL", "",
		"swarm/EquipIcons/EquipGrenadeLauncher",
		&asw_ammo_count_grenade_launcher, &asw_ammo_count_grenade_launcher,
		NULL, NULL,
	},
	{
		ASW_EQUIP_DEAGLE, WEAPON_NAME( deagle ),
		true, false, "ASW_DEAGLE", "",
		"swarm/EquipIcons/EquipDeagle",
		&asw_ammo_count_deagle, &asw_ammo_count_deagle,
		NULL, NULL,
	},
	{
		ASW_EQUIP_DEVASTATOR, WEAPON_NAME( devastator ),
		true, false, "ASW_DEVASTATOR", "",
		"swarm/EquipIcons/EquipDevastator",
		&asw_ammo_count_devastator, &asw_ammo_count_devastator,
		NULL, NULL,
		MARINE_CLASS_SPECIAL_WEAPONS, true,
	},
	{
		ASW_EQUIP_COMBAT_RIFLE, WEAPON_NAME( combat_rifle ),
		true, false, "ASW_R", "ASW_SG_G",
		"swarm/EquipIcons/EquipCombatRifle",
		&asw_ammo_count_rifle, &asw_ammo_count_rifle,
		&asw_ammo_count_combat_rifle_shotgun, &asw_ammo_count_combat_rifle_shotgun,
	},
	{
		ASW_EQUIP_HEALAMP_GUN, WEAPON_NAME( healamp_gun ),
		true, false, "", "",
		"swarm/EquipIcons/EquipHealAmpGun",
		&asw_ammo_count_healampgun_heal, &asw_ammo_count_healampgun_heal,
		&asw_ammo_count_healampgun_amp, &asw_ammo_count_healampgun_amp,
		MARINE_CLASS_MEDIC, true,
	},
	{
		ASW_EQUIP_HEAVY_RIFLE, WEAPON_NAME( heavy_rifle ),
		true, false, "ASW_HR", "ASW_HR_G",
		"swarm/EquipIcons/EquipHeavyRifle",
		&asw_ammo_count_heavy_rifle, &asw_ammo_count_heavy_rifle,
		&asw_ammo_count_heavy_rifle_charge, &asw_ammo_count_heavy_rifle_charge,
	},
	{
		ASW_EQUIP_MEDRIFLE, WEAPON_NAME( medrifle ),
		true, false, "ASW_MEDRIFLE", "",
		"swarm/EquipIcons/EquipMedRifle",
		&asw_ammo_count_medrifle, &asw_ammo_count_medrifle,
		&asw_ammo_count_medrifle_heal, &asw_ammo_count_medrifle_heal,
		MARINE_CLASS_MEDIC,
	},

	// hidden weapons
	{
		ASW_EQUIP_FIRE_EXTINGUISHER, WEAPON_NAME( fire_extinguisher ),
		false, false, "", "",
		"swarm/EquipIcons/EquipFireExt",
		&asw_ammo_count_fire_extinguisher, &asw_ammo_count_fire_extinguisher,
		NULL, NULL,
	},
	{
		ASW_EQUIP_MINING_LASER, WEAPON_NAME( mining_laser ),
		false, false, "ASW_ML", "",
		"swarm/EquipIcons/EquipMiningLaser",
		&asw_ammo_count_mining_laser, &asw_ammo_count_mining_laser,
		NULL, NULL,
	},
	{
		ASW_EQUIP_50CALMG, WEAPON_NAME( 50calmg ),
		false, false, "ASW_50CALMG", "",
		"swarm/EquipIcons/Equip50calmg",
		&asw_ammo_count_50calmg, &asw_ammo_count_50calmg,
		NULL, NULL,
	},
	{
		ASW_EQUIP_FLECHETTE, WEAPON_NAME( flechette ),
		false, false, "ASW_FLECHETTE", "",
		"swarm/EquipIcons/EquipFlechette",
		&asw_ammo_count_flechette, &asw_ammo_count_flechette,
		NULL, NULL,
	},
	{
		ASW_EQUIP_RICOCHET, WEAPON_NAME( ricochet ),
		false, false, "ASW_R", "ASW_SG_G",
		"swarm/EquipIcons/EquipRicochet",
		&asw_ammo_count_rifle, &asw_ammo_count_rifle,
		NULL, NULL,
	},
	{
		ASW_EQUIP_AMMO_BAG, WEAPON_NAME( ammo_bag ),
		false, false, "", "",
		"swarm/EquipIcons/EquipAmmoBag",
		NULL, NULL,
		NULL, NULL,
	},
	{
		ASW_EQUIP_MEDICAL_SATCHEL, WEAPON_NAME( medical_satchel ),
		false, false, "", "",
		"swarm/EquipIcons/EquipMedSatchel",
		&asw_ammo_count_medsatchel, &asw_ammo_count_medsatchel,
		NULL, NULL,
		MARINE_CLASS_MEDIC,
	},
	{
		ASW_EQUIP_AR2, WEAPON_NAME( ar2 ),
		false, false, "AR2", "AR2G",
		"swarm/EquipIcons/EquipAR2",
		&asw_ammo_count_ar2, &asw_ammo_count_ar2,
		&asw_ammo_count_ar2_grenade, &asw_ammo_count_ar2_grenade,
	},
#ifdef RD_7A_WEAPONS
	{
		ASW_EQUIP_CRYO_CANNON, WEAPON_NAME( cryo_cannon ),
		false, false, "ASW_CRYO", "",
		"swarm/EquipIcons/EquipCryoCannon",
		&asw_ammo_count_cryo_cannon, &asw_ammo_count_cryo_cannon,
		NULL, NULL,
		MARINE_CLASS_SPECIAL_WEAPONS,
	},
	{
		ASW_EQUIP_PLASMA_THROWER, WEAPON_NAME( plasma_thrower ),
		false, false, "ASW_F", "",
		"swarm/EquipIcons/EquipPlasmaThrower",
		&asw_ammo_count_flamer, &asw_ammo_count_flamer,
		NULL, NULL,
		MARINE_CLASS_SPECIAL_WEAPONS,
	},
	{
		ASW_EQUIP_HACK_TOOL, WEAPON_NAME( hack_tool ),
		false, false, "", "",
		"swarm/EquipIcons/EquipHackTool",
		NULL, NULL,
		NULL, NULL,
		MARINE_CLASS_UNDEFINED, true,
	},
	{
		ASW_EQUIP_SENTRY_RAILGUN, WEAPON_NAME( sentry_railgun ),
		false, false, "", "",
		"swarm/EquipIcons/EquipSentryRailgun",
		&asw_ammo_count_internal_sentry, &asw_ammo_count_internal_sentry,
		NULL, NULL,
	},
	{
		ASW_EQUIP_ENERGY_SHIELD, WEAPON_NAME( energy_shield ),
		false, false, "ASW_ESHIELD", "",
		"swarm/EquipIcons/EquipEnergyShield",
		&asw_ammo_count_energy_shield, &asw_ammo_count_energy_shield,
		NULL, NULL,
		MARINE_CLASS_TECH,
	},
	{
		ASW_EQUIP_REVIVE_TOOL, WEAPON_NAME( revive_tool ),
		false, false, "", "",
		"swarm/EquipIcons/EquipReviveTool",
		NULL, NULL,
		NULL, NULL,
		MARINE_CLASS_MEDIC, true,
	},
#endif
};

static CASW_EquipItem s_ExtraEquips[ASW_NUM_EQUIP_EXTRA] =
{
	// default equipment
	{
		ASW_EQUIP_MEDKIT, WEAPON_NAME( medkit ),
		true, true, "", "",
		"swarm/EquipIcons/EquipMedkit",
		&asw_ammo_count_medkit, &asw_ammo_count_medkit,
		NULL, NULL,
	},
	{
		ASW_EQUIP_WELDER, WEAPON_NAME( welder ),
		true, true, "", "",
		"swarm/EquipIcons/EquipWelder",
		NULL, NULL,
		NULL, NULL,
	},
	{
		ASW_EQUIP_FLARES, WEAPON_NAME( flares ),
		true, true, "", "",
		"swarm/EquipIcons/EquipFlares",
		&asw_ammo_count_flares, &asw_ammo_count_flares,
		NULL, NULL,
	},
	{
		ASW_EQUIP_LASER_MINES, WEAPON_NAME( laser_mines ),
		true, true, "", "",
		"swarm/EquipIcons/EquipLaserMines",
		&asw_ammo_count_laser_mines, &asw_ammo_count_laser_mines,
		NULL, NULL,
	},

	// level-unlockable equipment
	{
		ASW_EQUIP_NORMAL_ARMOR, WEAPON_NAME( normal_armor ),
		true, true, "", "",
		"swarm/EquipIcons/EquipNormalArmor",
		NULL, NULL,
		NULL, NULL,
	},
	{
		ASW_EQUIP_BUFF_GRENADE, WEAPON_NAME( buff_grenade ),
		true, true, "", "",
		"swarm/EquipIcons/EquipBuffGrenade",
		&asw_ammo_count_buff_grenade, &asw_ammo_count_buff_grenade,
		NULL, NULL,
	},
	{
		ASW_EQUIP_HORNET_BARRAGE, WEAPON_NAME( hornet_barrage ),
		true, true, "", "",
		"swarm/EquipIcons/EquipHornetBarrage",
		&asw_ammo_count_hornet_barrage, &asw_ammo_count_hornet_barrage,
		NULL, NULL,
	},
	{
		ASW_EQUIP_FREEZE_GRENADES, WEAPON_NAME( freeze_grenades ),
		true, true, "", "",
		"swarm/EquipIcons/EquipFreezeGrenade",
		&asw_ammo_count_freeze_grenades, &asw_ammo_count_freeze_grenades,
		NULL, NULL,
	},
	{
		ASW_EQUIP_STIM, WEAPON_NAME( stim ),
		true, true, "", "",
		"swarm/EquipIcons/EquipStims",
		&asw_ammo_count_stim, &asw_ammo_count_stim,
		NULL, NULL,
	},
	{
		ASW_EQUIP_TESLA_TRAP, WEAPON_NAME( tesla_trap ),
		true, true, "", "",
		"swarm/EquipIcons/EquipTeslaTrap",
		&asw_ammo_count_tesla_trap, &asw_ammo_count_tesla_trap,
		NULL, NULL,
	},
	{
		ASW_EQUIP_ELECTRIFIED_ARMOR, WEAPON_NAME( electrified_armor ),
		true, true, "", "",
		"swarm/EquipIcons/EquipElectrifiedArmor",
		&asw_ammo_count_electrified_armor, &asw_ammo_count_electrified_armor,
		NULL, NULL,
	},
	{
		ASW_EQUIP_MINES, WEAPON_NAME( mines ),
		true, true, "", "",
		"swarm/EquipIcons/EquipMines",
		&asw_ammo_count_mines, &asw_ammo_count_mines,
		NULL, NULL,
		MARINE_CLASS_NCO,
	},
	{
		ASW_EQUIP_FLASHLIGHT, WEAPON_NAME( flashlight ),
		true, true, "", "",
		"swarm/EquipIcons/EquipFlashlight",
		NULL, NULL,
		NULL, NULL,
	},
	{
		ASW_EQUIP_FIST, WEAPON_NAME( fist ),
		true, true, "", "",
		"swarm/EquipIcons/EquipPowerFist",
		NULL, NULL,
		NULL, NULL,
	},
	{
		ASW_EQUIP_GRENADES, WEAPON_NAME( grenades ),
		true, true, "", "",
		"swarm/EquipIcons/EquipGrenade",
		&asw_ammo_count_grenades, &asw_ammo_count_grenades,
		NULL, NULL,
	},
	{
		ASW_EQUIP_NIGHT_VISION, WEAPON_NAME( night_vision ),
		true, true, "", "",
		"swarm/EquipIcons/EquipGoggles",
		NULL, NULL,
		NULL, NULL,
	},
	{
		ASW_EQUIP_SMART_BOMB, WEAPON_NAME( smart_bomb ),
		true, true, "", "",
		"swarm/EquipIcons/EquipSmartBomb",
		&asw_ammo_count_smart_bomb, &asw_ammo_count_smart_bomb,
		NULL, NULL,
	},
	{
		ASW_EQUIP_GAS_GRENADES, WEAPON_NAME( gas_grenades ),
		true, true, "", "",
		"swarm/EquipIcons/EquipGasGrenades",
		&asw_ammo_count_gas_grenades, &asw_ammo_count_gas_grenades,
		NULL, NULL,
		MARINE_CLASS_MEDIC,
	},

	// hidden equipment
	{
		ASW_EQUIP_T75, WEAPON_NAME( t75 ),
		false, true, "", "",
		"swarm/EquipIcons/EquipT75",
		&asw_ammo_count_t75, &asw_ammo_count_t75,
		NULL, NULL,
	},
	{
		ASW_EQUIP_BLINK, WEAPON_NAME( blink ),
		false, true, "", "",
		"swarm/EquipIcons/EquipBlink",
		NULL, NULL,
		NULL, NULL,
	},
	{
		ASW_EQUIP_JUMP_JET, WEAPON_NAME( jump_jet ),
		false, true, "", "",
		"swarm/EquipIcons/EquipJumpJet",
		&asw_ammo_count_jump_jet, &asw_ammo_count_jump_jet,
		NULL, NULL,
	},
	{
		ASW_EQUIP_BAIT, WEAPON_NAME( bait ),
		false, true, "", "",
		"swarm/EquipIcons/EquipBait",
		&asw_ammo_count_bait, &asw_ammo_count_bait,
		NULL, NULL,
	},
#ifdef RD_7A_WEAPONS
	{
		ASW_EQUIP_STUN_GRENADES, WEAPON_NAME( stun_grenades ),
		false, true, "", "",
		"swarm/EquipIcons/EquipStunGrenades",
		&asw_ammo_count_stun_grenades, &asw_ammo_count_stun_grenades,
		NULL, NULL,
	},
	{
		ASW_EQUIP_INCENDIARY_GRENADES, WEAPON_NAME( incendiary_grenades ),
		false, true, "", "",
		"swarm/EquipIcons/EquipIncendiaryGrenades",
		&asw_ammo_count_incendiary_grenades, &asw_ammo_count_incendiary_grenades,
		NULL, NULL,
		MARINE_CLASS_NCO,
	},
	{
		ASW_EQUIP_SPEED_BURST, WEAPON_NAME( speed_burst ),
		false, true, "", "",
		"swarm/EquipIcons/EquipSpeedBurst",
		NULL, NULL,
		NULL, NULL,
	},
	{
		ASW_EQUIP_SHIELD_BUBBLE, WEAPON_NAME( shield_bubble ),
		false, true, "", "",
		"swarm/EquipIcons/EquipShieldBubble",
		&asw_ammo_count_shield_bubble, &asw_ammo_count_shield_bubble,
		NULL, NULL,
		MARINE_CLASS_SPECIAL_WEAPONS,
	},
#endif
};

CASW_EquipmentList::CASW_EquipmentList()
	: CAutoGameSystem{ "CASW_EquipmentList" }
{
#ifdef CLIENT_DLL
	m_bLoadedTextures = false;
#endif
}

void CASW_EquipmentList::LevelInitPreEntity()
{
	for ( int i = 0; i < NELEMS( s_RegularEquips ); i++ )
	{
		s_RegularEquips[i].LevelInitPreEntity();
	}
	for ( int i = 0; i < NELEMS( s_ExtraEquips ); i++ )
	{
		s_ExtraEquips[i].LevelInitPreEntity();
	}
}

// gets the weapondata for a particular class of weapon (assumes the weapon has been precached already)
CASW_WeaponInfo *CASW_EquipmentList::GetWeaponDataFor( const char *szWeaponClass )
{
	WEAPON_FILE_INFO_HANDLE hWeaponFileInfo;
	ReadWeaponDataFromFileForSlot( filesystem, szWeaponClass, &hWeaponFileInfo, ASWGameRules() ? ASWGameRules()->GetEncryptionKey() : NULL );
	return assert_cast< CASW_WeaponInfo * >( GetFileWeaponInfoFromHandle( hWeaponFileInfo ) );
}

CASW_EquipItem *CASW_EquipmentList::GetEquipItemFor( const char *szWeaponClass )
{
	for ( int i = 0; i < NELEMS( s_RegularEquips ); i++ )
	{
		if ( FStrEq( s_RegularEquips[i].m_szEquipClass, szWeaponClass ) )
		{
			return &s_RegularEquips[i];
		}
	}

	for ( int i = 0; i < NELEMS( s_ExtraEquips ); i++ )
	{
		if ( FStrEq( s_ExtraEquips[i].m_szEquipClass, szWeaponClass ) )
		{
			return &s_ExtraEquips[i];
		}
	}

	return NULL;
}

CASW_EquipItem *CASW_EquipmentList::GetRegular( int index )
{
	Assert( index >= -1 && index < int( NELEMS( s_RegularEquips ) ) );
	if ( index < 0 || index >= NELEMS( s_RegularEquips ) )
		return NULL;

	return &s_RegularEquips[index];
}

int CASW_EquipmentList::GetNumRegular( bool bIncludeHidden )
{
	return bIncludeHidden ? ASW_NUM_EQUIP_REGULAR : ASW_FIRST_HIDDEN_EQUIP_REGULAR;
}

CASW_EquipItem *CASW_EquipmentList::GetExtra( int index )
{
	Assert( index >= -1 && index < int( NELEMS( s_ExtraEquips ) ) );
	if ( index < 0 || index >= NELEMS( s_ExtraEquips ) )
		return NULL;

	return &s_ExtraEquips[index];
}

int CASW_EquipmentList::GetNumExtra( bool bIncludeHidden )
{
	return bIncludeHidden ? ASW_NUM_EQUIP_EXTRA : ASW_FIRST_HIDDEN_EQUIP_EXTRA;
}

CASW_EquipItem *CASW_EquipmentList::GetItemForSlot( int iWpnSlot, int index )
{
	return ( iWpnSlot <= 1 ) ? GetRegular( index ) : GetExtra( index );
}

int CASW_EquipmentList::GetRegularIndex( const char *szWeaponClass )
{
	for ( int i = 0; i < NELEMS( s_RegularEquips ); i++ )
	{
		if ( !V_stricmp( s_RegularEquips[i].m_szEquipClass, szWeaponClass ) )
			return i;
	}

	return -1;
}

int CASW_EquipmentList::GetExtraIndex( const char *szWeaponClass )
{
	for ( int i = 0; i < NELEMS( s_ExtraEquips ); i++ )
	{
		if ( !V_stricmp( s_ExtraEquips[i].m_szEquipClass, szWeaponClass ) )
			return i;
	}

	return -1;
}

int CASW_EquipmentList::GetIndexForSlot( int iWpnSlot, const char *szWeaponClass )
{
	return ( iWpnSlot <= 1 ) ? GetRegularIndex( szWeaponClass ) : GetExtraIndex( szWeaponClass );
}

// loads the weapon icon textures for all our equipment
#ifdef CLIENT_DLL
void CASW_EquipmentList::LoadTextures()
{
	if ( m_bLoadedTextures )
		return;

	LoadTextures( m_iRegularTexture, s_RegularEquips, NELEMS( s_RegularEquips ) );
	LoadTextures( m_iExtraTexture, s_ExtraEquips, NELEMS( s_ExtraEquips ) );
	m_bLoadedTextures = true;
}

void CASW_EquipmentList::LoadTextures( CUtlVector<int> &TextureIDs, CASW_EquipItem *pEquipItems, int nEquipItems )
{
	char buffer[256];

	TextureIDs.SetCount( nEquipItems );

	for ( int i = 0; i < nEquipItems; i++ )
	{
		int t = vgui::surface()->CreateNewTextureID();
		CASW_EquipItem *pItem = &pEquipItems[i];
		gWR.LoadWeaponSprites( LookupWeaponInfoSlot( pItem->m_szEquipClass ) ); // make sure we load in its .txt + sprites now too
		V_snprintf( buffer, sizeof( buffer ), "vgui/%s", pItem->m_szEquipIcon );
		vgui::surface()->DrawSetTextureFile( t, buffer, true, false );
		TextureIDs[i] = t;
	}
}

int CASW_EquipmentList::GetEquipIconTexture( bool bRegular, int iIndex )
{
	if ( !m_bLoadedTextures )
		LoadTextures();

	if ( iIndex < 0 )
		return -1;

	if ( bRegular )
	{
		if ( iIndex > m_iRegularTexture.Count() )
			return -1;
		return m_iRegularTexture[iIndex];
	}

	if ( iIndex > m_iExtraTexture.Count() )
		return -1;
	return m_iExtraTexture[iIndex];
}
#endif

CASW_EquipmentList g_ASWEquipmentList;

#ifdef CLIENT_DLL
CON_COMMAND( asw_list_equipment, "list weapons and their IDs" )
{
	Msg( "Regular\n");
	int nRegular = g_ASWEquipmentList.GetNumRegular( true );
	for ( int i = 0; i < nRegular; i++ )
	{
		CASW_EquipItem *pRegular = g_ASWEquipmentList.GetRegular( i );
		Assert( pRegular );
		Msg( "%d: %s%s\n", i, pRegular->m_szEquipClass, pRegular->m_bSelectableInBriefing ? "" : " (hidden)" );
	}
	Msg( "\n" );

	Msg( "Extra\n" );
	int nExtra = g_ASWEquipmentList.GetNumExtra( true );
	for ( int i = 0; i < nExtra; i++ )
	{
		CASW_EquipItem *pExtra = g_ASWEquipmentList.GetExtra( i );
		Assert( pExtra );
		Msg( "%d: %s%s\n", i, pExtra->m_szEquipClass, pExtra->m_bSelectableInBriefing ? "" : " (hidden)" );
	}
}
#endif

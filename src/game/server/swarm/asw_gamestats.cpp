//====== Copyright © 1996-2006, Valve Corporation, All rights reserved. =======//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "asw_gamestats.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_burning.h"
#include "asw_alien.h"
#include "asw_buzzer.h"
#include "asw_weapon_heal_gun_shared.h"
#include "asw_weapon_healamp_gun_shared.h"
#include "asw_weapon_healgrenade_shared.h"
#include "asw_weapon_medkit_shared.h"
#include "asw_weapon_medrifle_shared.h"
#include "asw_game_resource.h"
#include "asw_sentry_top.h"
#include "asw_sentry_base.h"
#include "asw_deathmatch_mode.h"

#define WEAPON_INIT \
	CASW_Marine *pAttackerMarine = CASW_Marine::AsMarine( info.GetAttacker() ); \
	CBaseEntity *pWeapon = info.GetWeapon(); \
	if ( !pAttackerMarine && !pWeapon ) \
	{ \
		CASW_Sentry_Top *pSentry = dynamic_cast<CASW_Sentry_Top *>( info.GetAttacker() ); \
		if ( pSentry && pSentry->GetSentryBase() && pSentry->GetSentryBase()->m_hDeployer.Get() ) \
		{ \
			pAttackerMarine = pSentry->GetSentryBase()->m_hDeployer; \
			pWeapon = pSentry; \
		} \
	} \
	if ( !pAttackerMarine || !pWeapon ) \
	{ \
		return; \
	} \
	CASW_Marine_Resource *pMR = pAttackerMarine->GetMarineResource(); \
	if ( !pMR ) \
	{ \
		return; \
	} \
	Class_T weaponClass = GetWeaponClassFromDamageInfo( info, pWeapon )

// Must run with -gamestats to be able to turn on/off stats with ConVar below.
static ConVar asw_stats_track( "asw_stats_track", "0", FCVAR_ARCHIVE, "Turn on//off Infested stats tracking." );
ConVar asw_stats_verbose( "asw_stats_verbose", "0", FCVAR_NONE, "Turn on//off verbose logging of stats." );
static ConVar asw_stats_nogameplaycheck( "asw_stats_nogameplaycheck", "0", FCVAR_NONE , "Disable normal check for valid gameplay, send stats regardless." );

CASWGameStats CASW_GameStats;

static Class_T GetWeaponClassFromDamageInfo( const CTakeDamageInfo & info, CBaseEntity *pWeapon )
{
	Class_T weaponClass = pWeapon->Classify();
	if ( weaponClass == CLASS_ASW_RIFLE )
	{
		if ( info.GetDamageType() & DMG_BLAST )
		{
			return (Class_T)CLASS_ASW_RIFLE_GRENADE;
		}
	}
	else if ( weaponClass == CLASS_ASW_PRIFLE )
	{
		if ( info.GetDamageType() & DMG_SHOCK )
		{
			return (Class_T)CLASS_ASW_GRENADE_PRIFLE;
		}
	}
	else if ( weaponClass == CLASS_ASW_ASSAULT_SHOTGUN )
	{
		if ( info.GetDamageType() & DMG_BURN )
		{
			return (Class_T)CLASS_ASW_GRENADE_VINDICATOR;
		}
	}
	else if ( weaponClass == CLASS_ASW_COMBAT_RIFLE )
	{
		if ( info.GetDamageType() & DMG_BUCKSHOT )
		{
			return (Class_T)CLASS_ASW_COMBAT_RIFLE_SHOTGUN;
		}
	}
	else if ( weaponClass == CLASS_ASW_GAS_GRENADE )
	{
		// the box of grenades gets credit for the grenade's hard work
		// this says a lot about our society
		return (Class_T)CLASS_ASW_GAS_GRENADES;
	}
	else if ( weaponClass == CLASS_ASW_AR2 )
	{
		if ( info.GetDamageType() & DMG_DISSOLVE )
		{
			return (Class_T)CLASS_ASW_COMBINE_BALL;
		}
	}
	return weaponClass;
}

void CASWGameStats::Event_MarineKilled( CASW_Marine *pMarine, const CTakeDamageInfo &info )
{
	WEAPON_INIT;

	if ( pAttackerMarine->IRelationType( pMarine ) != D_LI )
	{
		pMR->IncrementWeaponStats( weaponClass, 0, 0, 0, 0, 1 );
	}
}

void CASWGameStats::Event_AlienKilled( CBaseEntity *pAlien, const CTakeDamageInfo &info )
{
	if ( ASWDeathmatchMode() )
	{
		return;
	}

	WEAPON_INIT;

	pMR->IncrementWeaponStats( weaponClass, 0, 0, 0, 0, 1 );
}

void CASWGameStats::Event_MarineTookDamage( CASW_Marine *pMarine, const CTakeDamageInfo &info )
{
	WEAPON_INIT;

	int nHits = 1;
	
	CBaseEntity* pInflictor = info.GetInflictor();
	if ( pInflictor && pInflictor->Classify() == CLASS_ASW_BURNING )
	{
		nHits = 0;
	}

	int32 nDamage = info.GetDamage();
	int32 nFFDamage = 0;

	if ( pAttackerMarine->IRelationType( pMarine ) == D_LI )
	{
		nFFDamage = nDamage;
		nDamage = 0;
		nHits = 0;
	}

	pMR->IncrementWeaponStats( weaponClass, nDamage, nFFDamage, 0, nHits, 0 );
}

void CASWGameStats::Event_AlienTookDamage( CBaseEntity *pAlien, const CTakeDamageInfo &info )
{
	if ( ASWDeathmatchMode() )
	{
		return;
	}

	WEAPON_INIT;

	int nHits = 1;
	CBaseEntity* pInflictor = info.GetInflictor();
	if ( pInflictor && pInflictor->Classify() == CLASS_ASW_BURNING )
	{
		nHits = 0;
	}
	else if ( info.GetDamageType() & DMG_BURN )
	{
		if ( pAlien )
		{
			CASW_Alien* pBaseAlien = NULL;
			CASW_Buzzer* pBuzzer = NULL;

			if ( pAlien->IsAlienClassType() )
			{
				pBaseAlien = assert_cast<CASW_Alien*>(pAlien);

				if ( pBaseAlien->m_bFlammable )
				{
					if ( asw_stats_verbose.GetBool() )
					{
						DevMsg( "marine %d weaponclass %d (burned %s, %d+%d)\n", ASWGameResource()->GetMarineResourceIndex(pMR), weaponClass, pBaseAlien->GetClassname(), pMR->m_iAliensBurned, 1 );
					}

					if ( !pBaseAlien->m_bWasOnFireForStats )
					{
						pBaseAlien->m_bWasOnFireForStats = true;
						pMR->m_iAliensBurned++;
					}
				}
			}
			else if ( pAlien->Classify() == CLASS_ASW_BUZZER )
			{
				pBuzzer = assert_cast<CASW_Buzzer*>(pAlien);
				if ( pBuzzer->m_bFlammable )
				{
					if ( asw_stats_verbose.GetBool() )
					{
						DevMsg( "marine %d weaponclass %d (burned %s, %d+%d)\n", ASWGameResource()->GetMarineResourceIndex(pMR), weaponClass, pBuzzer->GetClassname(), pMR->m_iAliensBurned, 1 );
					}

					if ( !pBuzzer->m_bWasOnFireForStats )
					{
						pBuzzer->m_bWasOnFireForStats = true;
						pMR->m_iAliensBurned++;
					}
				}
			}
		}
	}

	pMR->IncrementWeaponStats( weaponClass, info.GetDamage(), 0, 0, nHits, 0 );
	if ( pAttackerMarine->GetDamageBuffEndTime() > gpGlobals->curtime && pAttackerMarine->m_hLastDamageBuffApplier.Get() && pAttackerMarine->m_hLastDamageBuffApplier->GetMarineResource() )
	{
		CASW_Marine_Resource *pAmpMR = pAttackerMarine->m_hLastDamageBuffApplier->GetMarineResource();
		int iAmount = info.GetDamage() / 2;
		if ( pAttackerMarine->m_iLastDamageBuffType == CLASS_ASW_BUFF_GRENADE )
		{
			if ( asw_stats_verbose.GetBool() )
			{
				DevMsg( "marine %d (%s %d+%d)\n", ASWGameResource()->GetMarineResourceIndex( pAmpMR ), "m_iDamageAmpAmps", pAmpMR->m_iDamageAmpAmps, iAmount );
			}
			pAmpMR->m_iDamageAmpAmps += iAmount;
		}
		else if ( pAttackerMarine->m_iLastDamageBuffType == CLASS_ASW_HEALAMP_GUN )
		{
			if ( asw_stats_verbose.GetBool() )
			{
				DevMsg( "marine %d (%s %d+%d)\n", ASWGameResource()->GetMarineResourceIndex( pAmpMR ), "m_iHealAmpGunAmps", pAmpMR->m_iHealAmpGunAmps, iAmount );
			}
			pAmpMR->m_iHealAmpGunAmps += iAmount;
		}
	}
}

#define ADD_STAT( field, amount ) \
			if ( asw_stats_verbose.GetBool() ) \
			{ \
				DevMsg( "marine %d (%s %d+%d)\n", ASWGameResource()->GetMarineResourceIndex( pMR ), #field, pMR->field, amount ); \
			} \
			pMR->field += amount;

void CASWGameStats::Event_MarineHealed( CASW_Marine *pMarine, int amount, CBaseEntity *pHealingWeapon )
{
	if ( !pHealingWeapon )
	{
		return;
	}

	// don't count overhealing
	amount -= MAX( 0, pMarine->GetMaxHealth() - ( pMarine->m_iSlowHealAmount + pMarine->GetHealth() ) );
	if ( amount <= 0 )
	{
		return;
	}

	if ( pHealingWeapon->Classify() == CLASS_ASW_HEALGRENADE_PROJECTILE )
	{
		CASW_HealGrenade_Projectile *pHealGrenade = assert_cast<CASW_HealGrenade_Projectile *>( pHealingWeapon );
		CASW_Marine *pMedic = CASW_Marine::AsMarine( pHealGrenade->GetOwnerEntity() );
		if ( !pMedic )
		{
			return;
		}
		CASW_Marine_Resource *pMR = pMedic->GetMarineResource();
		if ( !pMR )
		{
			return;
		}
		if ( pMarine == pMedic )
		{
			ADD_STAT( m_iHealBeaconHeals_Self, amount );
		}
		else
		{
			ADD_STAT( m_iHealBeaconHeals, amount );
		}
	}
	else if ( pHealingWeapon->Classify() == CLASS_ASW_HEAL_GUN )
	{
		CASW_Weapon_Heal_Gun *pHealGun = assert_cast<CASW_Weapon_Heal_Gun *>( pHealingWeapon );
		CASW_Marine *pMedic = pHealGun->GetMarine();
		if ( !pMedic )
		{
			return;
		}
		CASW_Marine_Resource *pMR = pMedic->GetMarineResource();
		if ( !pMR )
		{
			return;
		}
		if ( pMarine == pMedic )
		{
			ADD_STAT( m_iHealGunHeals_Self, amount );
		}
		else
		{
			ADD_STAT( m_iHealGunHeals, amount );
		}
	}
	else if ( pHealingWeapon->Classify() == CLASS_ASW_HEALAMP_GUN )
	{
		CASW_Weapon_HealAmp_Gun *pHealGun = assert_cast<CASW_Weapon_HealAmp_Gun *>( pHealingWeapon );
		CASW_Marine *pMedic = pHealGun->GetMarine();
		if ( !pMedic )
		{
			return;
		}
		CASW_Marine_Resource *pMR = pMedic->GetMarineResource();
		if ( !pMR )
		{
			return;
		}
		ADD_STAT( m_iHealAmpGunHeals, amount );
	}
	else if ( pHealingWeapon->Classify() == CLASS_ASW_MEDRIFLE )
	{
		CASW_Weapon_MedRifle *pHealGun = assert_cast< CASW_Weapon_MedRifle *>( pHealingWeapon );
		CASW_Marine *pMedic = pHealGun->GetMarine();
		if ( !pMedic )
		{
			return;
		}
		CASW_Marine_Resource *pMR = pMedic->GetMarineResource();
		if ( !pMR )
		{
			return;
		}
		ADD_STAT( m_iMedRifleHeals, amount );
	}
	else if ( pHealingWeapon->Classify() == CLASS_ASW_MEDKIT )
	{
		CASW_Weapon_Medkit *pMedkit = assert_cast<CASW_Weapon_Medkit *>( pHealingWeapon );
		CASW_Marine *pMedic = pMedkit->GetMarine();
		if ( !pMedic )
		{
			return;
		}
		CASW_Marine_Resource *pMR = pMedic->GetMarineResource();
		if ( !pMR )
		{
			return;
		}
		ADD_STAT( m_iMedkitHeals_Self, amount );
	}
}

void CASWGameStats::Event_MarineWeaponFired( const CBaseEntity *pWeapon, const CASW_Marine *pMarine, int nShotsFired, bool bIsSecondary )
{
	Class_T weaponClass = const_cast<CBaseEntity *>( pWeapon )->Classify();

	CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
	if ( !pMR )
	{
		return;
	}

	if ( weaponClass == CLASS_ASW_AMMO_SATCHEL )
	{
		ADD_STAT( m_iAmmoDeployed, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_SENTRY_GUN_CASE )
	{
		ADD_STAT( m_iSentryGunsDeployed, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_SENTRY_FLAMER_CASE )
	{
		ADD_STAT( m_iSentryFlamerDeployed, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_SENTRY_FREEZE_CASE )
	{
		ADD_STAT( m_iSentryFreezeDeployed, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_SENTRY_CANNON_CASE )
	{
		ADD_STAT( m_iSentryCannonDeployed, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_MEDKIT )
	{
		ADD_STAT( m_iMedkitsUsed, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_FLARES )
	{
		ADD_STAT( m_iFlaresUsed, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_STIM )
	{
		ADD_STAT( m_iAdrenalineUsed, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_TESLA_TRAP )
	{
		ADD_STAT( m_iTeslaTrapsDeployed, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_TESLA_TRAP_PROJECTILE )
	{
		weaponClass = (Class_T)CLASS_ASW_TESLA_TRAP;
	}
	else if ( weaponClass == CLASS_ASW_FREEZE_GRENADES )
	{
		ADD_STAT( m_iFreezeGrenadesThrown, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_ELECTRIFIED_ARMOR )
	{
		ADD_STAT( m_iElectricArmorUsed, 1 );
		return;
	}
	else if ( weaponClass == CLASS_ASW_BUFF_GRENADE )
	{
		ADD_STAT( m_iDamageAmpsUsed, 1 );
		return;
	}
	else if (weaponClass == CLASS_ASW_HEALGRENADE)
	{
		ADD_STAT( m_iHealBeaconsDeployed, 1 );
		return;
	}

	if ( bIsSecondary )
	{
		if ( weaponClass == CLASS_ASW_RIFLE )
		{
			weaponClass = (Class_T)CLASS_ASW_RIFLE_GRENADE;
		}
		else if ( weaponClass == CLASS_ASW_PRIFLE )
		{
			weaponClass = (Class_T)CLASS_ASW_GRENADE_PRIFLE;
		}
		else if ( weaponClass == CLASS_ASW_ASSAULT_SHOTGUN )
		{
			weaponClass = (Class_T)CLASS_ASW_GRENADE_VINDICATOR;
		}
		else if ( weaponClass == CLASS_ASW_COMBAT_RIFLE )
		{
			weaponClass = (Class_T)CLASS_ASW_COMBAT_RIFLE_SHOTGUN;
		}
		else if ( weaponClass == CLASS_ASW_AR2 )
		{
			weaponClass = (Class_T)CLASS_ASW_COMBINE_BALL;
		}
		else
		{
			Assert( 0 );
			weaponClass = (Class_T)(1024 - weaponClass);
		}
	}

	pMR->IncrementWeaponStats( weaponClass, 0, 0, 1, 0, 0 );
}

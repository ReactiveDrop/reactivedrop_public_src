#include "cbase.h"
#include "asw_ammo.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "asw_player.h"
#include "asw_marine.h"
#include "asw_ammo_drop_shared.h"
#include "particle_parse.h"
#include "cvisibilitymonitor.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-------------
// Generic give ammo function
//-------------

int ASW_GiveAmmo( CASW_Inhabitable_NPC *pNPC, float flCount, const char *pszAmmoName, CBaseEntity *pAmmoEntity, bool bSuppressSound = false )
{
	int iAmmoType = GetAmmoDef()->Index( pszAmmoName );
	if ( iAmmoType == -1 )
	{
		Msg( "ERROR: Attempting to give unknown ammo type (%s)\n", pszAmmoName );
		return 0;
	}

	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( !pMarine )
	{
		return 0;
	}

	int amount = pMarine->GiveAmmo( flCount, iAmmoType, bSuppressSound );
	if ( amount == 0 )
		amount = pMarine->GiveAmmoToAmmoBag( flCount, iAmmoType, bSuppressSound );

	if ( amount > 0 )
	{
		pMarine->TookAmmoPickup( pAmmoEntity );

		// Check the ammo type... for some doing a spilling bullet effect isn't fictionally appropriate
		if ( iAmmoType != GetAmmoDef()->Index( "ASW_F" ) && iAmmoType != GetAmmoDef()->Index( "ASW_ML" ) && iAmmoType != GetAmmoDef()->Index( "ASW_TG" ) && iAmmoType != GetAmmoDef()->Index( "ASW_GL" ) )
		{
			// Do effects
			int iAmmoCost = CASW_Ammo_Drop_Shared::GetAmmoUnitCost( iAmmoType );

			if ( iAmmoCost < 20 )
			{
				pAmmoEntity->EmitSound( "ASW_Ammobag.Pickup_sml" );
				DispatchParticleEffect( "ammo_satchel_take_sml", pAmmoEntity->GetAbsOrigin() + Vector( 0, 0, 4 ), vec3_angle );
			}
			else if ( iAmmoCost < 75 )
			{
				pAmmoEntity->EmitSound( "ASW_Ammobag.Pickup_med" );
				DispatchParticleEffect( "ammo_satchel_take_med", pAmmoEntity->GetAbsOrigin() + Vector( 0, 0, 4 ), vec3_angle );
			}
			else
			{
				pAmmoEntity->EmitSound( "ASW_Ammobag.Pickup_lrg" );
				DispatchParticleEffect( "ammo_satchel_take_lrg", pAmmoEntity->GetAbsOrigin() + Vector( 0, 0, 4 ), vec3_angle );
			}
		}

		IGameEvent *event = gameeventmanager->CreateEvent( "ammo_pickup" );
		if ( event )
		{
			CASW_Player *pPlayer = pMarine->GetCommander();
			event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
			event->SetInt( "entindex", pMarine->entindex() );

			gameeventmanager->FireEvent( event );
		}
	}

	return amount;
}

IMPLEMENT_AUTO_LIST( IAmmoPickupAutoList );

void CASW_Ammo::Spawn( void )
{
	BaseClass::Spawn();

	VisibilityMonitor_AddEntity( this, asw_visrange_generic.GetFloat() * 0.9f, &CASW_Ammo::VismonCallback, NULL );
}


//---------------------------------------------------------
// Callback for the visibility monitor.
//---------------------------------------------------------
bool CASW_Ammo::VismonCallback( CBaseEntity *pPickup, CBasePlayer *pViewingPlayer )
{
	CASW_Ammo *pPickupPtr = dynamic_cast < CASW_Ammo * >( pPickup );

	if ( !pPickupPtr )
		return true;

	IGameEvent *event = gameeventmanager->CreateEvent( "entity_visible" );
	if ( event )
	{
		event->SetInt( "userid", pViewingPlayer->GetUserID() );
		event->SetInt( "subject", pPickupPtr->entindex() );
		event->SetString( "classname", pPickupPtr->GetClassname() );
		event->SetString( "entityname", STRING( pPickupPtr->GetEntityName() ) );
		gameeventmanager->FireEvent( event );
	}

	return false;
}


//-------------
// Rifle Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Rifle, DT_ASW_Ammo_Rifle )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Rifle )
DEFINE_KEYFIELD( m_bAddSecondary, FIELD_BOOLEAN, "AddSecondary" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_rifle, CASW_Ammo_Rifle );

void CASW_Ammo_Rifle::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/ammo/ammoassaultrifle.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_R" );
}


void CASW_Ammo_Rifle::Precache( void )
{
	PrecacheModel( "models/swarm/ammo/ammoassaultrifle.mdl" );
}

void CASW_Ammo_Rifle::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	// player has used this item	
	if ( ASW_GiveAmmo( pNPC, 98, "ASW_R", this ) )
	{
		if ( m_bAddSecondary )//add rifle grenade
		{
			bool bFilledActive = false;
			CBaseCombatWeapon *pActive = pNPC->GetActiveWeapon();
			if ( pActive && ( pActive->Classify() == CLASS_ASW_RIFLE || pActive->Classify() == CLASS_ASW_PRIFLE || pActive->Classify() == CLASS_ASW_COMBAT_RIFLE ) )
			{
				if ( pActive->Clip2() < pActive->GetMaxClip2() )
				{
					pActive->m_iClip2++;
					bFilledActive = true;
				}
			}

			if ( !bFilledActive )
			{
				CBaseCombatWeapon *pWeapon;
				for ( int i = 0; i < ASW_MAX_MARINE_WEAPONS; i++ )
				{
					pWeapon = pNPC->GetWeapon( i );
					if ( pWeapon == pActive )
						continue;
					if ( pWeapon && ( pWeapon->Classify() == CLASS_ASW_RIFLE || pWeapon->Classify() == CLASS_ASW_PRIFLE || pWeapon->Classify() == CLASS_ASW_COMBAT_RIFLE ) )
						if ( pWeapon->Clip2() < pWeapon->GetMaxClip2() )
							pWeapon->m_iClip2++;
				}
			}
		}
		UTIL_Remove( this );
	}
}

//-------------
// Autogun Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Autogun, DT_ASW_Ammo_Autogun )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Autogun )
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_autogun, CASW_Ammo_Autogun );

void CASW_Ammo_Autogun::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/ammo/ammoautogun.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_AG" );
}


void CASW_Ammo_Autogun::Precache( void )
{
	PrecacheModel( "models/swarm/ammo/ammoautogun.mdl" );
}

void CASW_Ammo_Autogun::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	// player has used this item	
	if ( ASW_GiveAmmo( pNPC, 250, "ASW_AG", this ) )
	{
		UTIL_Remove( this );
	}
}

//-------------
// Shotgun Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Shotgun, DT_ASW_Ammo_Shotgun )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Shotgun )
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_shotgun, CASW_Ammo_Shotgun );

void CASW_Ammo_Shotgun::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/Ammo/ammoshotgun.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_SG" );
}


void CASW_Ammo_Shotgun::Precache( void )
{
	PrecacheModel( "models/swarm/Ammo/ammoshotgun.mdl" );
}

void CASW_Ammo_Shotgun::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( ASW_GiveAmmo( pNPC, 8, "ASW_SG", this ) )		// two clips per pack
	{
		UTIL_Remove( this );
	}
}

//-------------
// Assault Shotgun Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Assault_Shotgun, DT_ASW_Ammo_Assault_Shotgun )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Assault_Shotgun )
DEFINE_KEYFIELD( m_bAddSecondary, FIELD_BOOLEAN, "AddSecondary" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_vindicator, CASW_Ammo_Assault_Shotgun );

void CASW_Ammo_Assault_Shotgun::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/Ammo/ammovindicator.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_ASG" );
}


void CASW_Ammo_Assault_Shotgun::Precache( void )
{
	PrecacheModel( "models/swarm/Ammo/ammovindicator.mdl" );
}

void CASW_Ammo_Assault_Shotgun::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( ASW_GiveAmmo( pNPC, 14, "ASW_ASG", this ) )
	{
		if ( m_bAddSecondary )
		{
			bool bFilledActive = false;
			CBaseCombatWeapon *pActive = pNPC->GetActiveWeapon();
			if ( pActive && pActive->Classify() == CLASS_ASW_ASSAULT_SHOTGUN )
			{
				if ( pActive->Clip2() < pActive->GetMaxClip2() )
				{
					pActive->m_iClip2++;
					bFilledActive = true;
				}
			}

			if ( !bFilledActive )
			{
				CBaseCombatWeapon *pWeapon;
				for ( int i = 0; i < ASW_MAX_MARINE_WEAPONS; i++ )
				{
					pWeapon = pNPC->GetWeapon( i );
					if ( pWeapon == pActive )
						continue;
					if ( pWeapon && pWeapon->Classify() == CLASS_ASW_ASSAULT_SHOTGUN )
						if ( pWeapon->Clip2() < pWeapon->GetMaxClip2() )
							pWeapon->m_iClip2++;
				}
			}
		}
		UTIL_Remove( this );
	}
}

//-------------
// Flamer Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Flamer, DT_ASW_Ammo_Flamer )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Flamer )
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_flamer, CASW_Ammo_Flamer );

void CASW_Ammo_Flamer::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/Ammo/ammoflamer.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_F" );
}


void CASW_Ammo_Flamer::Precache( void )
{
	PrecacheModel( "models/swarm/Ammo/ammoflamer.mdl" );
}

void CASW_Ammo_Flamer::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( ASW_GiveAmmo( pNPC, 80, "ASW_F", this ) )
	{
		UTIL_Remove( this );
	}
}

//-------------
// Pistol Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Pistol, DT_ASW_Ammo_Pistol )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Pistol )
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_pistol, CASW_Ammo_Pistol );

void CASW_Ammo_Pistol::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/Ammo/ammopistol.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_P" );
}


void CASW_Ammo_Pistol::Precache( void )
{
	PrecacheModel( "models/swarm/Ammo/ammopistol.mdl" );
}

void CASW_Ammo_Pistol::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( ASW_GiveAmmo( pNPC, 16, "ASW_P", this ) )
	{
		UTIL_Remove( this );
	}
}

//-------------
// Mining Laser Battery Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Mining_Laser, DT_ASW_Ammo_Mining_Laser )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Mining_Laser )
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_mining_laser, CASW_Ammo_Mining_Laser );

void CASW_Ammo_Mining_Laser::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/Ammo/ammobattery.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_ML" );
}


void CASW_Ammo_Mining_Laser::Precache( void )
{
	PrecacheModel( "models/swarm/Ammo/ammobattery.mdl" );
}

void CASW_Ammo_Mining_Laser::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( ASW_GiveAmmo( pNPC, 50, "ASW_ML", this ) )
	{
		UTIL_Remove( this );
	}
}

//-------------
// Railgun Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Railgun, DT_ASW_Ammo_Railgun )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Railgun )
END_DATADESC()

//LINK_ENTITY_TO_CLASS(asw_ammo_railgun, CASW_Ammo_Railgun);

void CASW_Ammo_Railgun::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/Ammo/ammorailgun.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_RG" );
}


void CASW_Ammo_Railgun::Precache( void )
{
	PrecacheModel( "models/swarm/Ammo/ammorailgun.mdl" );
}

void CASW_Ammo_Railgun::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( ASW_GiveAmmo( pNPC, 12, "ASW_RG", this ) )
	{
		UTIL_Remove( this );
	}
}

//-------------
// Chainsaw Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Chainsaw, DT_ASW_Ammo_Chainsaw )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Chainsaw )
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_chainsaw, CASW_Ammo_Chainsaw );

void CASW_Ammo_Chainsaw::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/Ammo/ammobattery.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_CS" );
}


void CASW_Ammo_Chainsaw::Precache( void )
{
	PrecacheModel( "models/swarm/Ammo/ammobattery.mdl" );
}

void CASW_Ammo_Chainsaw::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( ASW_GiveAmmo( pNPC, 150, "ASW_CS", this ) )
	{
		UTIL_Remove( this );
	}
}

//-------------
// PDW Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_PDW, DT_ASW_Ammo_PDW )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_PDW )
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_pdw, CASW_Ammo_PDW );

void CASW_Ammo_PDW::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/Ammo/ammopdw.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_PDW" );
}


void CASW_Ammo_PDW::Precache( void )
{
	PrecacheModel( "models/swarm/Ammo/ammopdw.mdl" );
}

void CASW_Ammo_PDW::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( ASW_GiveAmmo( pNPC, 80, "ASW_PDW", this ) )
	{
		UTIL_Remove( this );
	}
}

//-------------
// AR2 Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_AR2, DT_ASW_Ammo_AR2 )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_AR2 )
DEFINE_KEYFIELD( m_bAddSecondary, FIELD_BOOLEAN, "AddSecondary" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_ar2, CASW_Ammo_AR2 );

void CASW_Ammo_AR2::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/ammo/ammoar2.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "AR2" );
}


void CASW_Ammo_AR2::Precache( void )
{
	PrecacheModel( "models/swarm/ammo/ammoar2.mdl" );
}

void CASW_Ammo_AR2::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	// give two reloads rather than just one like other ammo types
	if ( ASW_GiveAmmo( pNPC, 60, "AR2", this ) )
	{
		if ( m_bAddSecondary )
		{
			bool bFilledActive = false;
			CBaseCombatWeapon *pActive = pNPC->GetActiveWeapon();
			if ( pActive && pActive->Classify() == CLASS_ASW_AR2 )
			{
				if ( pActive->Clip2() < pActive->GetMaxClip2() )
				{
					pActive->m_iClip2++;
					bFilledActive = true;
				}
			}

			if ( !bFilledActive )
			{
				CBaseCombatWeapon *pWeapon;
				for ( int i = 0; i < ASW_MAX_MARINE_WEAPONS; i++ )
				{
					pWeapon = pNPC->GetWeapon( i );
					if ( pWeapon == pActive )
						continue;
					if ( pWeapon && pWeapon->Classify() == CLASS_ASW_AR2 )
						if ( pWeapon->Clip2() < pWeapon->GetMaxClip2() )
							pWeapon->m_iClip2++;
				}
			}
		}
		UTIL_Remove( this );
	}
}

//-------------
// Grenade Launcher Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Grenade_Launcher, DT_ASW_Ammo_Grenade_Launcher )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Grenade_Launcher )
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_grenade_launcher, CASW_Ammo_Grenade_Launcher );

void CASW_Ammo_Grenade_Launcher::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/ammo/ammogrenadelauncher.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_GL" );
}


void CASW_Ammo_Grenade_Launcher::Precache( void )
{
	PrecacheModel( "models/swarm/ammo/ammogrenadelauncher.mdl" );
}

void CASW_Ammo_Grenade_Launcher::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	// give three reloads to match the ammo stash interaction
	if ( ASW_GiveAmmo( pNPC, 18, "ASW_GL", this ) )
	{
		UTIL_Remove( this );
	}
}

//-------------
// Sniper Rifle Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Sniper_Rifle, DT_ASW_Ammo_Sniper_Rifle )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Sniper_Rifle )
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_sniper_rifle, CASW_Ammo_Sniper_Rifle );

void CASW_Ammo_Sniper_Rifle::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/ammo/ammosniperrifle.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_SNIPER" );
}


void CASW_Ammo_Sniper_Rifle::Precache( void )
{
	PrecacheModel( "models/swarm/ammo/ammosniperrifle.mdl" );
}

void CASW_Ammo_Sniper_Rifle::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( ASW_GiveAmmo( pNPC, 12, "ASW_SNIPER", this ) )
	{
		UTIL_Remove( this );
	}
}

//-------------
// Heavy Rifle Ammo
//-------------

IMPLEMENT_SERVERCLASS_ST( CASW_Ammo_Heavy_Rifle, DT_ASW_Ammo_Heavy_Rifle )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Ammo_Heavy_Rifle )
DEFINE_KEYFIELD( m_bAddSecondary, FIELD_BOOLEAN, "AddSecondary" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( asw_ammo_heavy_rifle, CASW_Ammo_Heavy_Rifle );

void CASW_Ammo_Heavy_Rifle::Spawn( void )
{
	Precache();
	SetModel( "models/swarm/ammo/ammohvyrifle.mdl" );
	BaseClass::Spawn();
	m_iAmmoIndex = GetAmmoDef()->Index( "ASW_HR" );
}


void CASW_Ammo_Heavy_Rifle::Precache( void )
{
	PrecacheModel( "models/swarm/ammo/ammohvyrifle.mdl" );
}

void CASW_Ammo_Heavy_Rifle::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	if ( ASW_GiveAmmo( pNPC, 98, "ASW_HR", this ) )
	{
		if ( m_bAddSecondary )
		{
			bool bFilledActive = false;
			CBaseCombatWeapon *pActive = pNPC->GetActiveWeapon();
			if ( pActive && pActive->Classify() == CLASS_ASW_HEAVY_RIFLE )
			{
				if ( pActive->Clip2() < pActive->GetMaxClip2() )
				{
					pActive->m_iClip2++;
					bFilledActive = true;
				}
			}

			if ( !bFilledActive )
			{
				CBaseCombatWeapon *pWeapon;
				for ( int i = 0; i < ASW_MAX_MARINE_WEAPONS; i++ )
				{
					pWeapon = pNPC->GetWeapon( i );
					if ( pWeapon == pActive )
						continue;
					if ( pWeapon && pWeapon->Classify() == CLASS_ASW_HEAVY_RIFLE )
						if ( pWeapon->Clip2() < pWeapon->GetMaxClip2() )
							pWeapon->m_iClip2++;
				}
			}
		}
		UTIL_Remove( this );
	}
}

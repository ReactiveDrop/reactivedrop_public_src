#include "cbase.h"
#include "asw_extinguisher_projectile.h"
#include "Sprite.h"
#include "soundent.h"
#include "te_effect_dispatch.h"
#include "IEffects.h"
#include "EntityFlame.h"
#include "asw_fire.h"
#include "asw_marine.h"
#include "asw_weapon_flamer_shared.h"
#include "asw_sentry_top_icer.h"
#include "asw_sentry_base.h"
#include "particle_parse.h"
#include "asw_marine_resource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


extern ConVar sk_plr_dmg_asw_f;
extern ConVar sk_npc_dmg_asw_f;
extern ConVar asw_flamer_debug;
ConVar rd_extinguisher_freeze_amount( "rd_extinguisher_freeze_amount", "0.0", FCVAR_REPLICATED | FCVAR_CHEAT, "The amount of freezing to apply to the extinguisher" );
ConVar rd_extinguisher_dmg_amount( "rd_extinguisher_dmg_amount", "0.0", FCVAR_REPLICATED | FCVAR_CHEAT, "The amount of damage the extinguisher does to entities" );
ConVar rd_extinguisher_dmg_force( "rd_extinguisher_dmg_force", "1.0", FCVAR_REPLICATED | FCVAR_CHEAT, "The amount of force being hit by a damaging extinguisher particle applies" );
ConVar rd_extinguisher_hull_scale[NUM_HULLS]
{
	{ "rd_extinguisher_hull_scale_human", "1", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for y'know ;)" },
	{ "rd_extinguisher_hull_scale_small_centered", "5", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for (unused, was scanner in HL2)" },
	{ "rd_extinguisher_hull_scale_wide_human", "1", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for Vortigaunt (unused)" },
	{ "rd_extinguisher_hull_scale_tiny", "5", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for Headcrab, Zombie Torso, Birds, Parasite, Grub, Xenomite" },
	{ "rd_extinguisher_hull_scale_wide_short", "0.4", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for Harvester, Mortarbug, Shieldbug" },
	{ "rd_extinguisher_hull_scale_medium", "1", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for Antlion, Mender" },
	{ "rd_extinguisher_hull_scale_tiny_centered", "5", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for Buzzer, Scanner" },
	{ "rd_extinguisher_hull_scale_large", "0.4", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for Boomer, Antlion Guard" },
	{ "rd_extinguisher_hull_scale_large_centered", "0", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for Strider / Dropship / Gunship / Helicopter" },
	{ "rd_extinguisher_hull_scale_medium_tall", "0.4", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for Hunter" },
	{ "rd_extinguisher_hull_scale_tiny_fluid", "0", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for EP3 Blob (unused)" },
	{ "rd_extinguisher_hull_scale_mediumbig", "1", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for Drone, Ranger" },
	{ "rd_extinguisher_hull_scale_huge", "0.2", FCVAR_CHEAT, "Cryo Cannon freeze rate scale for Queen" },
};

#define PELLET_MODEL "models/swarm/Shotgun/ShotgunPellet.mdl"

LINK_ENTITY_TO_CLASS( asw_extinguisher_projectile, CASW_Extinguisher_Projectile );

IMPLEMENT_SERVERCLASS_ST( CASW_Extinguisher_Projectile, DT_ASW_Extinguisher_Projectile )
	SendPropDataTable( SENDINFO_DT( m_ProjectileData ), &REFERENCE_SEND_TABLE( DT_RD_ProjectileData ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Extinguisher_Projectile )
	DEFINE_FUNCTION( ProjectileTouch ),
	DEFINE_FIELD( m_hFirer, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hFirerWeapon, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flDamage, FIELD_FLOAT ),
	DEFINE_FIELD( m_flFreezeAmount, FIELD_FLOAT ),
	DEFINE_FIELD( m_flExplosionRadius, FIELD_FLOAT ),
	DEFINE_FIELD( m_bAllowFriendlyFire, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bUseHullFreezeScale, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecSpawnOrigin, FIELD_POSITION_VECTOR ),
END_DATADESC()


CASW_Extinguisher_Projectile::CASW_Extinguisher_Projectile()
{
	m_flDamage = rd_extinguisher_dmg_amount.GetFloat();
	m_flFreezeAmount = rd_extinguisher_freeze_amount.GetFloat();
	m_flExplosionRadius = 0.0f;
	m_bAllowFriendlyFire = false;
	m_bUseHullFreezeScale = false;
}

CASW_Extinguisher_Projectile::~CASW_Extinguisher_Projectile()
{
}

void CASW_Extinguisher_Projectile::Spawn()
{
	Precache();

	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM );

	m_takedamage = DAMAGE_NO;

	SetSize( -Vector( 4, 4, 4 ), Vector( 4, 4, 4 ) );
	SetSolid( SOLID_BBOX );
	SetGravity( 0.05f );
	SetCollisionGroup( ASW_COLLISION_GROUP_EXTINGUISHER_PELLETS );

	SetTouch( &CASW_Extinguisher_Projectile::ProjectileTouch );

	// extinguisher projectile only lasts 1 second
	SetThink( &CASW_Extinguisher_Projectile::SUB_Remove );
	SetNextThink( gpGlobals->curtime + 1.0f );
}

bool CASW_Extinguisher_Projectile::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, FSOLID_NOT_STANDABLE, false );
	return true;
}

unsigned int CASW_Extinguisher_Projectile::PhysicsSolidMaskForEntity() const
{
	return ( BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX ) & ~CONTENTS_GRATE;
}

void CASW_Extinguisher_Projectile::ProjectileTouch( CBaseEntity *pOther )
{
	if ( !g_pGameRules || !g_pGameRules->ShouldCollide( GetCollisionGroup(), pOther->GetCollisionGroup() ) )
		return;

	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet( FSOLID_VOLUME_CONTENTS ) )
		return;

	OnProjectileTouch( pOther );

	if ( m_flExplosionRadius > 0.0f )
	{
		if ( m_hFirer.Get() && m_hFirer->IRelationType( pOther ) != D_LI )
		{
			// Don't splash friendly fire if we hit an enemy.
			m_bAllowFriendlyFire = false;
		}

		Explode( pOther );
	}

	SetAbsVelocity( vec3_origin );

	SetTouch( NULL );
	SetThink( NULL );

	UTIL_Remove( this );
}

void CASW_Extinguisher_Projectile::Explode( CBaseEntity *pExcept )
{
	bool bAtLeastOne = false;

	CEntitySphereQuery sphere( GetAbsOrigin(), m_flExplosionRadius );
	while ( CBaseEntity *pEnt = sphere.GetCurrentEntity() )
	{
		sphere.NextEntity();

		if ( !g_pGameRules->ShouldCollide( GetCollisionGroup(), pEnt->GetCollisionGroup() ) )
			continue;

		if ( !pEnt->IsSolid() || pEnt->IsSolidFlagSet( FSOLID_VOLUME_CONTENTS ) )
			continue;

		if ( pEnt->m_takedamage != DAMAGE_NO )
			bAtLeastOne = true;

		if ( pEnt == pExcept )
			continue;

		OnProjectileTouch( pEnt );
	}

	if ( bAtLeastOne )
	{
		// don't play explode animation if we only hit world or props
		CBaseEntity *pHelpHelpImBeingSupressed = ( CBaseEntity * )te->GetSuppressHost();
		te->SetSuppressHost( NULL );
		DispatchParticleEffect( "rd_cryocannon_explode", GetAbsOrigin(), Vector( m_flExplosionRadius, 0, 0 ), vec3_angle );
		te->SetSuppressHost( pHelpHelpImBeingSupressed );
	}
}

void CASW_Extinguisher_Projectile::OnProjectileTouch( CBaseEntity *pOther )
{
	if ( pOther->m_takedamage == DAMAGE_NO )
		return;

	CASW_Marine *pMarine = CASW_Marine::AsMarine( pOther );
	if ( pMarine && pMarine->m_bOnFire )
	{
		pMarine->Extinguish( m_hFirer, this );
		m_bAllowFriendlyFire = false; // never do friendly fire when also extinguishing
	}
	else
	{
		CBaseAnimating *pAnim = pOther->GetBaseAnimating();
		if ( pAnim && pAnim->IsOnFire() )
		{
			CEntityFlame *pFireChild = dynamic_cast< CEntityFlame * >( pAnim->GetEffectEntity() );
			if ( pFireChild )
			{
				pAnim->SetEffectEntity( NULL );
				UTIL_Remove( pFireChild );
			}

			pAnim->Extinguish();
		}
	}

	bool bFriendly = m_hFirer.Get() && m_hFirer->IRelationType( pOther ) == D_LI;

	// half radius for friendly fire
	if ( m_flExplosionRadius > 0.0f && bFriendly )
	{
		// actually less than half because we're measuring to the center rather than the edge
		if ( GetAbsOrigin().DistTo( pOther->WorldSpaceCenter() ) > m_flExplosionRadius / 2.0f )
		{
			return;
		}
	}

	if ( m_flDamage > 0.0f && ( !bFriendly || m_bAllowFriendlyFire ) )
	{
		Vector vecDirection = pOther->WorldSpaceCenter() - m_vecSpawnOrigin;
		vecDirection.NormalizeInPlace();
		CTakeDamageInfo	dmgInfo( this, m_hFirer, m_hFirerWeapon, vecDirection * rd_extinguisher_dmg_force.GetFloat(), GetAbsOrigin(), m_flDamage, DMG_COLD );
		pOther->TakeDamage( dmgInfo );
	}

	CAI_BaseNPC *pNPC = pOther->MyNPCPointer();
	if ( pNPC )
	{
		float flFreezeAmount = m_flFreezeAmount;
		if ( m_bUseHullFreezeScale )
		{
			flFreezeAmount *= rd_extinguisher_hull_scale[clamp( pNPC->GetHullType(), 0, NUM_HULLS - 1 )].GetFloat();
		}

		if ( flFreezeAmount > 0 && ( !bFriendly || m_bAllowFriendlyFire ) )
		{
			bool bWasFrozen = pNPC->m_bWasEverFrozen;
			pNPC->Freeze( flFreezeAmount, m_hFirer ? m_hFirer : this );
			if ( !bWasFrozen && pNPC->IsFrozen() )
			{
				CASW_Marine *pMarineOwner = CASW_Marine::AsMarine( m_hFirer );
				if ( !pMarine && pMarineOwner && m_hFirerWeapon.Get() && m_hFirerWeapon->Classify() == CLASS_ASW_CRYO_CANNON )
				{
					CASW_Marine_Resource *pMR = pMarineOwner->GetMarineResource();
					if ( pMR )
					{
						pMR->m_iCryoCannonFreezeAlien++;
					}
				}

				IGameEvent *event = gameeventmanager->CreateEvent( "entity_frozen" );
				if ( event )
				{
					event->SetInt( "entindex", pNPC->entindex() );
					event->SetInt( "attacker", m_hFirer ? m_hFirer->entindex() : -1 );
					event->SetInt( "weapon", m_hFirerWeapon ? m_hFirerWeapon->entindex() : -1 );

					gameeventmanager->FireEvent( event );
				}
			}
		}
	}
}

CASW_Extinguisher_Projectile *CASW_Extinguisher_Projectile::Extinguisher_Projectile_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseCombatCharacter *pOwner, CBaseEntity *pWeapon )
{
	CASW_Extinguisher_Projectile *pPellet = ( CASW_Extinguisher_Projectile * )CreateEntityByName( "asw_extinguisher_projectile" );
	pPellet->SetAbsAngles( angles );
	pPellet->Spawn();
	pPellet->SetOwnerEntity( pOwner );
	pPellet->m_hFirer = pOwner;
	pPellet->m_hFirerWeapon = pWeapon;
	pPellet->m_ProjectileData.GetForModify().SetFromWeapon( pWeapon );
	UTIL_SetOrigin( pPellet, position );
	pPellet->SetAbsVelocity( velocity );
	pPellet->m_vecSpawnOrigin = position;

	if ( asw_flamer_debug.GetBool() )
		pPellet->m_debugOverlays |= OVERLAY_BBOX_BIT;

	return pPellet;
}

#define ASW_EXTINGUISHER_PROJECTILE_ACCN 250.0f
void CASW_Extinguisher_Projectile::PhysicsSimulate()
{
	// Make sure not to simulate this guy twice per frame
	if ( m_nSimulationTick == gpGlobals->tickcount )
		return;

	// slow down the projectile's velocity	
	SetAbsVelocity( GetAbsVelocity() * ( 1 - gpGlobals->frametime * ASW_EXTINGUISHER_PROJECTILE_ACCN / CASW_Weapon_Flamer::EXTINGUISHER_PROJECTILE_AIR_VELOCITY ) );

	if ( asw_flamer_debug.GetBool() )
	{
		NDebugOverlay::Box( GetAbsOrigin(), -Vector( 4, 4, 4 ), Vector( 4, 4, 4 ), 255, 0, 0, 255, 0.2f );

		NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity(), 255, 255, 0, true, 0.3f );
		NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + GetAbsVelocity() * gpGlobals->interval_per_tick, 128, 255, 0, true, 0.3f );
	}

	BaseClass::PhysicsSimulate();
}

// the client doesn't need this entity
int CASW_Extinguisher_Projectile::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_DONTSEND;
}

void CASW_Extinguisher_Projectile::TouchedEnvFire()
{
	if ( m_flExplosionRadius > 0.0f )
	{
		// make sure we still do AOE damage even if there's a fire in the way when we stop
		m_bAllowFriendlyFire = false;
		Explode( NULL );
	}

	SetThink( &CASW_Extinguisher_Projectile::SUB_Remove );
	SetNextThink( gpGlobals->curtime );
}

CBaseEntity *CASW_Extinguisher_Projectile::GetFirer()
{
	return m_hFirer.Get();
}

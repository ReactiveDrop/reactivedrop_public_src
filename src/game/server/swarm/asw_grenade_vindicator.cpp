#include "cbase.h"
#include "asw_grenade_vindicator.h"
#include "asw_gamerules.h"
#include "world.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "asw_util_shared.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "te_effect_dispatch.h"
#include "particle_parse.h"
#include "asw_player.h"
#include "asw_achievements.h"
#include "asw_boomer_blob.h"
#include "asw_fx_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sk_plr_dmg_asw_r_g;
ConVar asw_vindicator_grenade_friction("asw_vindicator_grenade_friction", "-1.0f", FCVAR_CHEAT, "Time before grenade can ");
ConVar asw_vindicator_grenade_gravity("asw_vindicator_grenade_gravity", "0.8f", FCVAR_CHEAT, "Gravity of vindicator grenade");
ConVar asw_vindicator_grenade_elasticity("asw_vindicator_grenade_elasticity", "1.8f", FCVAR_CHEAT, "elasticity of vindicator grenade");
ConVar asw_vindicator_grenade_min_detonation_time("asw_vindicator_grenade_min_detonation_time", "0.05f", FCVAR_CHEAT, "Minimum before this grenade can detonate");
ConVar asw_vindicator_grenade_mass("asw_vindicator_grenade_mass", "10", FCVAR_CHEAT, "Mass of indendiary/cluster grenade");
ConVar asw_vindicator_grenade_fuse("asw_vindicator_grenade_fuse", "3", FCVAR_CHEAT, "Fuse time on incendiary grenades");
ConVar rd_grenade_collision_fix("rd_grenade_collision_fix", "1", FCVAR_CHEAT, "Set to 1 to not impact on dropped weapons & items");	//DRAVEN ~FIXGLITEMCOLLISION~ Added check to exclude item drops
ConVar rd_grenade_launcher_arm_time( "rd_grenade_launcher_arm_time", "0", FCVAR_CHEAT, "Time in seconds until grenade launcher grenade is armed and can explode" );
ConVar rd_grenade_launcher_projectile_direct_dmg( "rd_grenade_launcher_projectile_direct_dmg", "256", FCVAR_CHEAT, "The direct damage caused by non exploded grenade from GL" );

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

#define VIND_GRENADE_MODEL "models/swarm/grenades/VindicatorGrenadeProjectile.mdl"

LINK_ENTITY_TO_CLASS( asw_grenade_vindicator, CASW_Grenade_Vindicator );

PRECACHE_REGISTER( asw_grenade_vindicator );

BEGIN_DATADESC( CASW_Grenade_Vindicator )
	DEFINE_ENTITYFUNC( VGrenadeTouch ),
	DEFINE_THINKFUNC( Detonate ),
	DEFINE_FIELD( m_pMainGlow, FIELD_EHANDLE ),
	DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),
	DEFINE_FIELD( m_iClusters, FIELD_INTEGER ),
	DEFINE_FIELD( m_fEarliestTouchDetonationTime, FIELD_TIME ),
	DEFINE_FIELD( m_bKicked, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bExplodeOnWorldContact, FIELD_BOOLEAN ),

	DEFINE_OUTPUT(m_OnDamaged, "OnDamaged"),
END_DATADESC()

extern int	g_sModelIndexFireball;			// (in combatweapon.cpp) holds the index for the smoke cloud

CASW_Grenade_Vindicator::CASW_Grenade_Vindicator()
{
	g_aExplosiveProjectiles.AddToTail( this );
	SetBloodColor( DONT_BLEED ); 
	m_flDirectHitDamageMultiplier = 1.0f;
}

CASW_Grenade_Vindicator::~CASW_Grenade_Vindicator()
{
	g_aExplosiveProjectiles.FindAndRemove( this );
}

void CASW_Grenade_Vindicator::Spawn( void )
{
	Precache( );

	SetModel( VIND_GRENADE_MODEL );
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	m_flDamage		= sk_plr_dmg_asw_r_g.GetFloat();
	m_DmgRadius		= 220.0f;

	Ignite(3.0, false, 0, false);

	m_takedamage	= DAMAGE_YES;

	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	SetSolid( SOLID_BBOX );
	SetGravity( asw_vindicator_grenade_gravity.GetFloat() );
	SetFriction( asw_vindicator_grenade_friction.GetFloat() );
	SetElasticity( asw_vindicator_grenade_elasticity.GetFloat() );
	SetCollisionGroup( ASW_COLLISION_GROUP_GRENADES );
	//CreateVPhysics();

	SetTouch( &CASW_Grenade_Vindicator::VGrenadeTouch );

	CreateEffects();
	m_hFirer = NULL;

	//AddSolidFlags( FSOLID_NOT_STANDABLE );

	// Tumble in air
	QAngle vecAngVelocity( random->RandomFloat ( -100, -500 ), 0, 0 );
	SetLocalAngularVelocity( vecAngVelocity );

	if ( !m_bSilent )
	{
		EmitSound( "ASWGrenade.Alarm" );  // 3 second warning sound
	}
	SetFuseLength(asw_vindicator_grenade_fuse.GetFloat());

	m_fEarliestTouchDetonationTime = GetEarliestTouchDetonationTime();

	m_iClusters = 0;
	m_bMaster = true;

	//AddSolidFlags(FSOLID_TRIGGER);
	//m_pLastHit = NULL;
}

void CASW_Grenade_Vindicator::SetFuseLength(float fSeconds)
{
	SetThink( &CASW_Grenade_Vindicator::Detonate );
	SetNextThink( gpGlobals->curtime + fSeconds );
}

void CASW_Grenade_Vindicator::VGrenadeTouch( CBaseEntity *pOther )
{
	if ( !pOther || pOther == GetOwnerEntity() )
		return;

	if ( !pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS) )
		return;

	// reactivedrop: added ASW_COLLISION_GROUP_PASSABLE because 
	// grenades from Grenade Launcher detonate on fire mines while
	// all other grenades pass it
	if ( pOther->GetCollisionGroup() == ASW_COLLISION_GROUP_PASSABLE )
		return;

	if ( rd_grenade_collision_fix.GetBool() && pOther->GetCollisionGroup() == COLLISION_GROUP_WEAPON )	//DRAVEN ~FIXGLITEMCOLLISION~ Added check to exclude item drops
		return;																						//DRAVEN ~FIXGLITEMCOLLISION~ Added check to exclude item drops

	// make sure we don't die on things we shouldn't
	if ( !ASWGameRules() || !ASWGameRules()->ShouldCollide( GetCollisionGroup(), pOther->GetCollisionGroup() ) )
		return;

	if ( m_bExplodeOnWorldContact && rd_grenade_launcher_arm_time.GetFloat() > 0 &&
		 gpGlobals->curtime < m_fEarliestTouchDetonationTime )
	{
		if ( pOther->IsNPC() || pOther->Classify() == CLASS_ASW_EGG )
		{
			if ( pOther->m_takedamage != DAMAGE_NO )
			{
				CTakeDamageInfo info( this, m_hFirer.Get(), rd_grenade_launcher_projectile_direct_dmg.GetFloat(), DMG_CRUSH );
				info.SetWeapon( m_hCreatorWeapon );
				pOther->TakeDamage( info );
				UTIL_ASW_DroneBleed( GetAbsOrigin(), Vector( 0, 0, 1 ), 4 );
			}
		}
		if ( !m_bSilent )
		{
			EmitSound( "Grenade.ImpactHard" );
		}

		//UTIL_Remove( this );			// just removing the grenade is not looking good, let it bounce back and live for a few seconds
		//StopParticleEffects( this );	// this removes smoke and glow, but for some reason it works only when marine is far from spawn point, tested on ocs2 map
		SetTouch( NULL );

		SetThink( &CASW_Grenade_Vindicator::SUB_Remove );
		SetNextThink( gpGlobals->curtime + 5.0f );
		return;
	}

	if ( m_bExplodeOnWorldContact )
	{
		if ( pOther->m_takedamage != DAMAGE_NO && pOther->IsNPC() )
		{
			m_flDamage *= m_flDirectHitDamageMultiplier;
			m_flDirectHitDamageMultiplier = 1.0f;
		}

		Detonate();
		return;
	}

	if ( pOther->m_takedamage == DAMAGE_NO )
	{
		if ( GetAbsVelocity().Length2D() > 60 )
		{
			if ( !m_bSilent )
			{
				EmitSound( "Grenade.ImpactHard" );
			}
		}
	}

	// can't detonate yet?
	if ( gpGlobals->curtime < m_fEarliestTouchDetonationTime )
		return;

	if ( pOther->m_takedamage != DAMAGE_NO )
	{
		if (pOther->IsNPC() && pOther->Classify() != CLASS_ASW_MARINE)
			Detonate();
	}
}

CASW_Grenade_Vindicator* CASW_Grenade_Vindicator::Vindicator_Grenade_Create( float flDamage, float fRadius, const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, CBaseEntity *pCreatorWeapon  )
{
	CASW_Grenade_Vindicator *pGrenade = (CASW_Grenade_Vindicator *)CreateEntityByName( "asw_grenade_vindicator" );
	pGrenade->SetAbsAngles( angles );
	UTIL_SetOrigin( pGrenade, position );
	pGrenade->Spawn();
	pGrenade->m_flDamage = flDamage;
	pGrenade->m_DmgRadius = fRadius;
	pGrenade->m_hFirer = pOwner;
	pGrenade->SetOwnerEntity( pOwner );
	UTIL_SetOrigin( pGrenade, position );
	pGrenade->SetAbsVelocity( velocity );
	pGrenade->m_hCreatorWeapon = pCreatorWeapon;

	if ( pCreatorWeapon )
	{
		pGrenade->m_ProjectileData.GetForModify().SetFromWeapon( pCreatorWeapon );
	}

	return pGrenade;
}

extern ConVar asw_medal_explosive_kills;

void CASW_Grenade_Vindicator::Detonate()
{
	m_takedamage	= DAMAGE_NO;

	StopSound( "ASWGrenade.Alarm" );
	CPASFilter filter( GetAbsOrigin() );

	/*
	te->Explosion( filter, 0.0,
		&GetAbsOrigin(), 
		g_sModelIndexFireball,
		2.0, 
		15,
		TE_EXPLFLAG_NONE,
		m_DmgRadius,
		m_flDamage );
	*/

	if ( !m_bSilent )
	{
		EmitSound( "ASWGrenade.Incendiary" );
	}
	// throw out some flames
	CEffectData	data;			
	data.m_vOrigin = GetAbsOrigin();
	DispatchEffect( "ASWFireBurst", data );

	Vector vecForward = GetAbsVelocity();
	VectorNormalize(vecForward);
	trace_t		tr;
	UTIL_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + 60*vecForward, MASK_SHOT, 
		this, COLLISION_GROUP_NONE, &tr);


	if (m_bMaster)
	{
		if ((tr.m_pEnt != GetWorldEntity()) || (tr.hitbox != 0))
		{
			// non-world needs smaller decals
			if( tr.m_pEnt && !tr.m_pEnt->IsNPC() )
			{
				UTIL_DecalTrace( &tr, "SmallScorch" );
			}
		}
		else
		{
			UTIL_DecalTrace( &tr, "Scorch" );
		}

		UTIL_ASW_ScreenShake( GetAbsOrigin(), 10.0, 150.0, 1.0, 750, SHAKE_START );
	}

	int iPreExplosionKills = 0;

	CASW_Marine* pMarine = NULL;
	CASW_Marine_Resource* pPMR = NULL;
	CBaseEntity* pFirer = m_hFirer.Get();

	if ( pFirer && pFirer->Classify() == CLASS_ASW_MARINE)
	{ 
		pMarine = assert_cast<CASW_Marine*>( pFirer );
		pPMR = pMarine->GetMarineResource();
	}

	if (pMarine && pPMR)
		iPreExplosionKills = pPMR->m_iAliensKilled;

	CTakeDamageInfo info( this, pFirer, m_flDamage, DMG_BURN );
	info.SetWeapon( m_hCreatorWeapon );
	RadiusDamage ( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );
	//RadiusDamage ( CTakeDamageInfo( this, GetOwnerEntity(), m_flDamage, DMG_BLAST ), GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

	if (pMarine && pPMR)
	{
		int iKilledByExplosion = pPMR->m_iAliensKilled - iPreExplosionKills;
		if (iKilledByExplosion > pPMR->m_iSingleGrenadeKills)
		{
			pPMR->m_iSingleGrenadeKills = iKilledByExplosion;
			if ( iKilledByExplosion > asw_medal_explosive_kills.GetInt() && pMarine->GetCommander() && pMarine->IsInhabited() )
			{
				pMarine->GetCommander()->AwardAchievement( ACHIEVEMENT_ASW_GRENADE_MULTI_KILL );
			}
		}
		//if (m_bKicked)
			//pMarine->GetMarineResource()->m_iKickedGrenadeKills += iKilledByExplosion;

		// count as a shot fired
		pPMR->UsedWeapon(NULL, 1);
	}

	while (m_iClusters > 0)
	{
		float fYaw = random->RandomFloat(0, 360);
		QAngle ang(0, fYaw, 0);
		Vector newVel;
		AngleVectors(ang, &newVel);
		newVel *= random->RandomFloat(150, 200);
		newVel.z = random->RandomFloat(200, 400);
		CASW_Grenade_Vindicator *pGrenade = CASW_Grenade_Vindicator::Vindicator_Grenade_Create( 
			m_flDamage,
			m_DmgRadius,
			GetAbsOrigin(), ang, newVel, AngularImpulse(0,0,0), pFirer, m_hCreatorWeapon );
		if (pGrenade)
		{
			pGrenade->m_takedamage = DAMAGE_YES;
			//pGrenade->SetClusters(0);
			//pGrenade->SetFuseLength(random->RandomFloat(0.8f, 1.6f));
			//if (bMaster)
				//pGrenade->SetClusters(1, false);
			//else
			pGrenade->SetFuseLength(random->RandomFloat(0.8f, 1.6f));
				pGrenade->SetClusters(0, false);
			//float f = m_iClusters / 5.0f;
			
		}
		m_iClusters--;
	}

	UTIL_Remove( this );
}

void CASW_Grenade_Vindicator::Precache()
{
	BaseClass::Precache( );

	PrecacheModel(VIND_GRENADE_MODEL);
	PrecacheModel("sprites/redglow1.vmt");
	PrecacheModel("sprites/blueglow1.vmt");
	PrecacheModel("sprites/bluelaser1.vmt");
	PrecacheScriptSound("ASWGrenade.Incendiary");
	PrecacheScriptSound("ASWGrenade.Alarm");
	PrecacheScriptSound("Grenade.ImpactHard");
	PrecacheParticleSystem( "VindGrenade" );
	PrecacheParticleSystem( "grenade_main_trail" );
}

void CASW_Grenade_Vindicator::KillEffects()
{
	
}

void CASW_Grenade_Vindicator::CreateEffects()
{
	// Start up the eye glow
	/*
	m_pMainGlow = CSprite::SpriteCreate( "sprites/redglow1.vmt", GetLocalOrigin(), false );

	int	nAttachment = LookupAttachment( "fuse" );

	if ( m_pMainGlow != NULL )
	{
		m_pMainGlow->FollowEntity( this );
		m_pMainGlow->SetAttachment( this, nAttachment );
		m_pMainGlow->SetTransparency( kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation );
		m_pMainGlow->SetScale( 0.2f );
		m_pMainGlow->SetGlowProxySize( 4.0f );
	}

	// Start up the eye trail
	m_pGlowTrail	= CSpriteTrail::SpriteTrailCreate( "sprites/bluelaser1.vmt", GetLocalOrigin(), false );

	if ( m_pGlowTrail != NULL )
	{
		m_pGlowTrail->FollowEntity( this );
		m_pGlowTrail->SetAttachment( this, nAttachment );
		m_pGlowTrail->SetTransparency( kRenderTransAdd, 255, 0, 0, 255, kRenderFxNone );
		m_pGlowTrail->SetStartWidth( 8.0f );
		m_pGlowTrail->SetEndWidth( 1.0f );
		m_pGlowTrail->SetLifeTime( 0.5f );
	}
	*/

	CEffectData	data;
	data.m_vOrigin = GetAbsOrigin();
	//data.m_vNormal = dir;
	//data.m_flScale = (float)amount;
	CPASFilter filter( data.m_vOrigin );
	filter.SetIgnorePredictionCull(true);
	DispatchParticleEffect( "rocket_trail_small", PATTACH_ABSORIGIN_FOLLOW, this, "fuse", false, -1, &filter );
}

int	CASW_Grenade_Vindicator::OnTakeDamage_Dying( const CTakeDamageInfo &info )
{	
	return BaseClass::OnTakeDamage_Dying(info);
}

int	CASW_Grenade_Vindicator::OnTakeDamage( const CTakeDamageInfo &info )
{
	// don't allow grenades to be damaged by other grenade explosions
	if (info.GetDamageType() & DMG_BLAST)
	{
		return 0;
	}

	// let NPCS kick us around
	if (info.GetAttacker() && info.GetAttacker()->IsNPC())
	{
		Vector vecForce = info.GetDamageForce() / asw_vindicator_grenade_mass.GetFloat();
		vecForce.z = random->RandomFloat(100, 150);
		ApplyAbsVelocityImpulse(vecForce);
		if (info.GetDamageType() & DMG_CLUB)
		{
			m_bKicked = true;
		}

		//Mad Orange. Fire OnDamaged Output
		m_OnDamaged.FireOutput(info.GetAttacker(), this);
	}

	return BaseClass::OnTakeDamage(info);
}

float CASW_Grenade_Vindicator::GetEarliestTouchDetonationTime()
{
	return gpGlobals->curtime + asw_vindicator_grenade_min_detonation_time.GetFloat();
}

void CASW_Grenade_Vindicator::BurntAlien(CBaseEntity *pAlien)
{
	//Msg("grenade is burning %s %d\n", pAlien->GetClassname(), pAlien->entindex());
	if (m_bKicked && pAlien && pAlien->IsNPC())
	{
		// check the alien is actually an alien
		if (pAlien->Classify() == CLASS_ASW_MARINE)
			return;

		CBaseEntity* pFirer = m_hFirer.Get();
		if ( pFirer && pFirer->Classify() == CLASS_ASW_MARINE )
		{
			CASW_Marine* pMarine = assert_cast<CASW_Marine*>( pFirer );
			if ( pMarine->GetMarineResource() )
			{
				//Msg("kicked grenade is burning %s %d\n", pAlien->GetClassname(), pAlien->entindex());
				pMarine->GetMarineResource()->m_iKickedGrenadeKills++;
			}
		}
	}
}

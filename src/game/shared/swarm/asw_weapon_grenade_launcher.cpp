#include "cbase.h"
#include "asw_weapon_grenade_launcher.h"
#include "in_buttons.h"

#ifdef CLIENT_DLL
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "c_asw_marine.h"
#include "prediction.h"
#include "c_te_effect_dispatch.h"
#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
#else
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_weapon.h"
#include "npcevent.h"
#include "shot_manipulator.h"
#include "asw_grenade_cluster.h"
#include "asw_marine_speech.h"
#include "asw_gamerules.h"
#endif
#include "asw_marine_skills.h"
#include "particle_parse.h"
#include "asw_util_shared.h"
#include "asw_deathmatch_mode_light.h"
#include "asw_weapon_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ASW_FLARES_FASTEST_REFIRE_TIME		0.1f

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Grenade_Launcher, DT_ASW_Weapon_Grenade_Launcher )

BEGIN_NETWORK_TABLE( CASW_Weapon_Grenade_Launcher, DT_ASW_Weapon_Grenade_Launcher )
#ifdef CLIENT_DLL
	// recvprops
#else
	// sendprops
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Grenade_Launcher )
	
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_grenade_launcher, CASW_Weapon_Grenade_Launcher );
PRECACHE_WEAPON_REGISTER( asw_weapon_grenade_launcher );

#ifndef CLIENT_DLL
extern ConVar asw_debug_marine_damage;
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon_Grenade_Launcher )
	DEFINE_FIELD( m_flSoonestPrimaryAttack, FIELD_TIME ),	
END_DATADESC()

#endif /* not client */

CASW_Weapon_Grenade_Launcher::CASW_Weapon_Grenade_Launcher()
{
	m_flSoonestPrimaryAttack = gpGlobals->curtime;
}


CASW_Weapon_Grenade_Launcher::~CASW_Weapon_Grenade_Launcher()
{

}

void CASW_Weapon_Grenade_Launcher::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "ASW_GrenadeLauncher.ReloadA" );
	PrecacheScriptSound( "ASW_GrenadeLauncher.ReloadB" );
	PrecacheScriptSound( "ASW_GrenadeLauncher.ReloadC" );
}

#ifdef GAME_DLL
ConVar rd_grenade_launcher_explode_on_contact( "rd_grenade_launcher_explode_on_contact", "1", FCVAR_CHEAT, "If set to 0 grenade will not explode on contact with rigid world" );
ConVar rda_grenade_launcher_grenade_ricochet("rda_grenade_launcher_grenade_ricochet", "0", FCVAR_CHEAT, "If set to 1 GL grenades ricochet after world collision");
#endif
ConVar rd_grenade_launcher_direct_hit_damage_mult( "rd_grenade_launcher_direct_hit_damage_mult", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "Damage multiplier for direct hits with grenade launcher grenades." );
ConVar rd_grenade_launcher_num_clusters( "rd_grenade_launcher_num_clusters", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "Number of clusters to spawn on grenade explosion", true, 0, true, 15 );
ConVar rd_grenade_launcher_grenade_preview( "rd_grenade_launcher_grenade_preview", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "Draw a predictive arc for a grenade launcher" );
ConVar asw_grenade_launcher_gravity( "asw_grenade_launcher_gravity", "2.4", FCVAR_CHEAT | FCVAR_REPLICATED, "Gravity of grenade launcher grenades" );
ConVar asw_grenade_launcher_velocity( "asw_grenade_launcher_velocity", "28.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Velocity multiplier for grenade launcher grenades" );
ConVar rd_grenade_launcher_far_below_require_hit_world( "rd_grenade_launcher_far_below_require_hit_world", "128", FCVAR_CHEAT | FCVAR_REPLICATED, "Only use the \"far below\" logic if the grenade would hit a static part of the world more than this many units away from the target." );
ConVar rd_grenade_launcher_far_below_distance( "rd_grenade_launcher_far_below_distance", "512", FCVAR_CHEAT | FCVAR_REPLICATED, "How far below the grenade's starting position the target has to be to use the arc modifier." );
ConVar rd_grenade_launcher_far_below_arc_modifier( "rd_grenade_launcher_far_below_arc_modifier", "1.5", FCVAR_CHEAT | FCVAR_REPLICATED, "A time value to force grenades launched \"far below\" to use while calculating the arc." );

void CASW_Weapon_Grenade_Launcher::PrimaryAttack( void )
{
	// NOTE: this class is now a grenade launcher, do we want to rename it at some point?
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine || pMarine->GetWaterLevel() == 3 )
		return;

#ifndef CLIENT_DLL
	pMarine->GetMarineSpeech()->Chatter(CHATTER_GRENADE);

	Vector vecSrc;
	Vector vecLaunch = IdealLaunchVelocity( vecSrc );

	float fGrenadeDamage = GetWeaponDamage();
	float fGrenadeRadius = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_RADIUS );
	int iClusters = rd_grenade_launcher_num_clusters.GetInt();
	if ( asw_debug_marine_damage.GetBool() )
	{
		Msg( "Grenade launcher damage = %f radius = %f clusters = %d\n", fGrenadeDamage, fGrenadeRadius, iClusters );
	}

	if (ASWGameRules())
		ASWGameRules()->m_fLastFireTime = gpGlobals->curtime;

	CASW_Grenade_Cluster *pGrenade =
		CASW_Grenade_Cluster::Cluster_Grenade_Create(
			fGrenadeDamage,
			fGrenadeRadius,
			iClusters,
			vecSrc, pMarine->EyeAngles(), vecLaunch, AngularImpulse( 0, 0, 0 ), pMarine, this );
	if ( pGrenade )
	{
		pGrenade->SetGravity( asw_grenade_launcher_gravity.GetFloat() );

		pGrenade->SetDirectHitDamageMultiplier( rd_grenade_launcher_direct_hit_damage_mult.GetFloat() );

		if ( rda_grenade_launcher_grenade_ricochet.GetBool() )
			pGrenade->SetAdvancedRicochet(true);
		else
			pGrenade->SetExplodeOnWorldContact( rd_grenade_launcher_explode_on_contact.GetBool() );

		pMarine->OnWeaponFired( this, 1 );
	}

#else
	DispatchParticleEffect( "muzzleflash_grenadelauncher_main", PATTACH_POINT_FOLLOW, this, "muzzle" );
#endif

	WeaponSound(SINGLE);

	//pMarine->DoMuzzleFlash();
	// sets the animation on the weapon model iteself
	SendWeaponAnim( GetPrimaryAttackActivity() );

	pMarine->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_PRIMARY );

	// decrement ammo
	m_iClip1 -= 1;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + GetFireRate();
}

float CASW_Weapon_Grenade_Launcher::GetWeaponBaseDamageOverride()
{
	extern ConVar rd_grenade_launcher_dmg_base;
	return rd_grenade_launcher_dmg_base.GetFloat();
}

int CASW_Weapon_Grenade_Launcher::GetWeaponSkillId()
{
	return ASW_MARINE_SKILL_GRENADES;
}

int CASW_Weapon_Grenade_Launcher::GetWeaponSubSkillId()
{
	return ASW_MARINE_SUBSKILL_GRENADE_CLUSTER_DMG;
}

void CASW_Weapon_Grenade_Launcher::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();

	Preview();
}

#ifdef CLIENT_DLL
bool CASW_Weapon_Grenade_Launcher::Simulate()
{
	bool bContinue = BaseClass::Simulate();

	C_ASW_Marine *pMarine = GetMarine();
	C_ASW_Marine *pViewMarine = C_ASW_Marine::GetViewMarine();
	if ( pMarine && pMarine == pViewMarine && pMarine->GetActiveASWWeapon() == this )
	{
		// We don't care if the grenade launcher is pointing at a marine if the grenade isn't going to land anywhere near them.
		SetLaserTargetEntity( NULL );

		Vector vecSrc;
		Vector vecLaunch = IdealLaunchVelocity( vecSrc );
		Vector vecDest = UTIL_Check_Throw( vecSrc, vecLaunch, asw_grenade_launcher_gravity.GetFloat(), Vector( -4, -4, -4 ), Vector( 4, 4, 4 ), MASK_SOLID, ASW_COLLISION_GROUP_GRENADES, pMarine );

		// We don't have access to the value of asw_marine_explosion_protection on the client.
		const float flMarineExplosionProtection = 0.5f;
		// ...or rd_radial_damage_no_falloff_distance
		const float flNoFalloffDistance = 16;
		float flDamage = GetWeaponDamage();
		float flRadius = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_RADIUS );
		float flFriendlyFireRadius = flRadius * flMarineExplosionProtection;
		float flOneHitpointRadius = ( flFriendlyFireRadius - flNoFalloffDistance ) / flDamage;
		float flRadiusSquared = Square( flFriendlyFireRadius - flOneHitpointRadius );
		C_BaseEntity *pEnt = NULL;
		for ( CEntitySphereQuery sphere( vecDest, flFriendlyFireRadius - flOneHitpointRadius ); ( pEnt = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if ( pEnt->WorldSpaceCenter().DistToSqr( vecDest ) < flRadiusSquared && IsFriendlyFireTarget( pEnt ) )
			{
				SetLaserTargetEntity( pEnt );
				break;
			}
		}
	}

	return bContinue;
}
#endif

Vector CASW_Weapon_Grenade_Launcher::IdealLaunchVelocity( Vector &vecSrc )
{
	CBaseCombatCharacter *pOwner = GetOwner();
	Assert( pOwner && pOwner->IsInhabitableNPC() );
	if ( !pOwner || !pOwner->IsInhabitableNPC() )
		return vec3_origin;

	CASW_Inhabitable_NPC *pNPC = assert_cast< CASW_Inhabitable_NPC * >( pOwner );
	CASW_Player *pPlayer = pNPC->GetCommander();

	vecSrc = pNPC->Weapon_ShootPosition();
	// check it fits where we want to spawn it
	Ray_t ray;
	trace_t pm;
	ray.Init( pNPC->WorldSpaceCenter(), vecSrc, -Vector( 4, 4, 4 ), Vector( 4, 4, 4 ) );
	UTIL_TraceRay( ray, MASK_SOLID, pNPC, ASW_COLLISION_GROUP_GRENADES, &pm );
	if ( pm.fraction < 1.0f )
		vecSrc = pm.endpos;

	Vector vecDest;
#ifdef CLIENT_DLL
	Assert( pNPC->IsInhabited() && pPlayer );
	if ( !pNPC->IsInhabited() || !pPlayer )
		return vec3_origin;
#else
	vecDest = pNPC->GetEnemyLKP();
#endif

	if ( pNPC->IsInhabited() )
	{
		Assert( pPlayer );
		if ( pPlayer )
			vecDest = pPlayer->GetCrosshairTracePos();
	}

	Vector vecLaunch = UTIL_LaunchVector( vecSrc, vecDest, asw_grenade_launcher_gravity.GetFloat() ) * asw_grenade_launcher_velocity.GetFloat();
	if ( vecSrc.z > vecDest.z + rd_grenade_launcher_far_below_distance.GetFloat() )
	{
		Vector vecEndPos = UTIL_Check_Throw( vecSrc, vecLaunch, asw_grenade_launcher_gravity.GetFloat(), -Vector( 4, 4, 4 ), Vector( 4, 4, 4 ), MASK_SOLID, ASW_COLLISION_GROUP_GRENADES, pNPC );
		if ( vecDest.DistToSqr( vecEndPos ) > Square( rd_grenade_launcher_far_below_require_hit_world.GetFloat() ) )
		{
			vecLaunch = UTIL_LaunchVector( vecSrc, vecDest, asw_grenade_launcher_gravity.GetFloat(), rd_grenade_launcher_far_below_arc_modifier.GetFloat() * asw_grenade_launcher_velocity.GetFloat() ) * asw_grenade_launcher_velocity.GetFloat();
		}
	}

	return vecLaunch;
}

void CASW_Weapon_Grenade_Launcher::Preview()
{
	if ( !rd_grenade_launcher_grenade_preview.GetBool() )
		return;

#ifdef CLIENT_DLL
	CASW_Marine *pMarine = GetMarine();
	CASW_Marine *pLocalMarine = CASW_Marine::GetLocalMarine();

	if ( pMarine && pMarine == pLocalMarine && pMarine->IsInhabited() )
	{
		Vector vecSrc;
		Vector vecLaunch = IdealLaunchVelocity( vecSrc );
		Vector vecEndPos = UTIL_Check_Throw( vecSrc, vecLaunch, asw_grenade_launcher_gravity.GetFloat(), -Vector( 4, 4, 4 ), Vector( 4, 4, 4 ), MASK_SOLID, ASW_COLLISION_GROUP_GRENADES, pMarine, true );
		debugoverlay->AddBoxOverlay( vecEndPos, Vector( -1, -1, -1 ), Vector( 1, 1, 1 ), QAngle( 0, 0, 0 ), 255, 0, 0, 127, NDEBUG_PERSIST_TILL_NEXT_SERVER );
	}
#endif
}

#ifdef CLIENT_DLL
const char *CASW_Weapon_Grenade_Launcher::GetPartialReloadSound( int iPart )
{
	if ( iPart == 1 )
		return "ASW_GrenadeLauncher.ReloadB";
	if ( iPart == 2 )
		return "ASW_GrenadeLauncher.ReloadC";
	return "ASW_GrenadeLauncher.ReloadA";
}
#endif

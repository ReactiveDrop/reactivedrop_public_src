#include "cbase.h"
#include "props.h"
#include "asw_sentry_base.h"
#include "asw_sentry_top_cannon.h"
#include "asw_player.h"
#include "asw_marine.h"
#include "ammodef.h"
#include "asw_gamerules.h"
#include "beam_shared.h"
#include "asw_rifle_grenade.h"
#include "asw_weapon_grenades.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define SENTRY_TOP_MODEL "models/sentry_gun/grenade_top.mdl"

ConVar asw_sentry_top_cannon_dmg_override( "asw_sentry_top_cannon_dmg_override", "0", FCVAR_CHEAT, "Overrides sentry cannon base damage. 0 means no override is done", true, 0.0f, true, 99999.0f );
ConVar asw_sentry_top_cannon_fire_rate( "asw_sentry_top_cannon_fire_rate", "1.75", FCVAR_CHEAT, "Time in seconds between each shot of sentry cannon", true, 0.001f, true, 999.0f );
extern ConVar asw_sentry_friendly_target;


LINK_ENTITY_TO_CLASS( asw_sentry_top_cannon, CASW_Sentry_Top_Cannon );
PRECACHE_REGISTER( asw_sentry_top_cannon );

/*
IMPLEMENT_SERVERCLASS_ST(CASW_Sentry_Top_Cannon, DT_ASW_Sentry_Top_cannon )
END_SEND_TABLE()
*/

BEGIN_DATADESC( CASW_Sentry_Top_Cannon )
DEFINE_KEYFIELD( m_flFireRate, FIELD_FLOAT, "FireRate" ),
END_DATADESC()

void CASW_Sentry_Top_Cannon::SetTopModel()
{
	SetModel(SENTRY_TOP_MODEL);
}

CASW_Sentry_Top_Cannon::CASW_Sentry_Top_Cannon() 
{
	m_flShootRange = 1000;
	m_flFireRate = asw_sentry_top_cannon_fire_rate.GetFloat();
}

/// @TODO: lead target
void CASW_Sentry_Top_Cannon::Fire()
{
	if ( !m_hEnemy.IsValid() || !m_hEnemy.Get() || !HasAmmo() )
		return;

	BaseClass::Fire();

	Vector diff = m_hEnemy->WorldSpaceCenter() - GetFiringPosition();
	diff.NormalizeInPlace();
	//FireBulletsInfo_t( int nShots, const Vector &vecSrc, const Vector &vecDir, const Vector &vecSpread, float flDistance, int nAmmoType, bool bPrimaryAttack = true )

	Vector launchVector = diff * 1000.0f;

	CASW_Marine *RESTRICT pMarineDeployer = GetSentryBase() ? GetSentryBase()->m_hDeployer.Get() : NULL;

	float fGrenadeDamage;
	float fGrenadeRadius;

	float fBaseGrenadeDamage = 60.0f;
	if ( asw_sentry_top_cannon_dmg_override.GetFloat() > 0 )
	{
		fBaseGrenadeDamage = asw_sentry_top_cannon_dmg_override.GetFloat();
	}
	else
	{
		CASW_WeaponInfo *pWeaponInfo = g_ASWEquipmentList.GetWeaponDataFor( "asw_weapon_sentry_cannon" );
		if ( pWeaponInfo )
			fBaseGrenadeDamage = pWeaponInfo->m_flBaseDamage;
	}

	if ( pMarineDeployer )
	{
		fGrenadeDamage = fBaseGrenadeDamage + MarineSkills()->GetSkillBasedValueByMarine( pMarineDeployer, ASW_MARINE_SKILL_GRENADES, ASW_MARINE_SUBSKILL_GRENADE_CLUSTER_DMG ) * 0.5f;
		fGrenadeRadius = CASW_Weapon_Grenades::GetBoomRadius( pMarineDeployer ) * 0.5f;
	}
	else
	{
		extern ConVar asw_skill_grenades_cluster_dmg_base;
		extern ConVar asw_skill_grenades_radius_base;
		fGrenadeDamage = fBaseGrenadeDamage;	// reactivedrop: was asw_skill_grenades_cluster_dmg_base.GetFloat() * 0.5f, but we made asw_skill_grenades_cluster_dmg_base 0(zero) to fix the UI 80(+80) bug
		fGrenadeRadius = asw_skill_grenades_radius_base.GetFloat() * 0.5f;
	}

	CBaseEntity *owner;
	if ( pMarineDeployer )
		owner = pMarineDeployer;
	else
		owner = this;

	CASW_Rifle_Grenade::Rifle_Grenade_Create(
		fGrenadeDamage, fGrenadeRadius,
		GetFiringPosition() + ( diff * ( WorldAlignSize().Length() * 0.5f ) ),
		GetAbsAngles(), launchVector, AngularImpulse( 0, 0, 0 ),
		owner, this );

	if ( pMarineDeployer )
		pMarineDeployer->OnWeaponFired( this, 1 );

	EmitSound( "ASW_Sentry.CannonFire" );

	m_fNextFireTime = gpGlobals->curtime + m_flFireRate;

	// use ammo
	if ( GetSentryBase() )
	{
		GetSentryBase()->OnFiredShots();
	}
}

CAI_BaseNPC *CASW_Sentry_Top_Cannon::SelectOptimalEnemy()
{
	// prioritize unfrozen aliens who are going to leave the cone soon.
	// prioritize aliens less the more frozen they get.
	CUtlVectorFixedGrowable< CAI_BaseNPC *,16 > candidates;
	CUtlVectorFixedGrowable< float, 16 > candidatescores;

	// search through all npcs, any that are in LOS and have health
	CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
	for ( int i = 0; i < g_AI_Manager.NumAIs(); i++ )
	{
		if (ppAIs[i]->GetHealth() > 0 && CanSee(ppAIs[i]))
		{
			// don't shoot marines
			if ( !asw_sentry_friendly_target.GetBool() && ( ppAIs[i]->Classify() == CLASS_ASW_MARINE || ppAIs[i]->Classify() == CLASS_ASW_COLONIST ) )
				continue;

			if ( ppAIs[i]->Classify() == CLASS_SCANNER )
				continue;

			if ( !IsValidEnemy( ppAIs[i] ) )
				continue;

			candidates.AddToTail( ppAIs[i] );
		}
	}

	// bail out if we don't have anyone
	if ( candidates.Count() < 1 )
		return NULL;

	else if ( candidates.Count() == 1 ) // just one candidate is an obvious result
		return candidates[0];

	// score each of the candidates
	candidatescores.EnsureCount( candidates.Count() );
	for ( int i = candidates.Count() - 1; i >= 0 ; --i )
	{
		CAI_BaseNPC * RESTRICT pCandidate = candidates[i];

		// is the candidate moving into or out of the cone?
		Vector vCandVel = GetEnemyVelocity(pCandidate);
		Vector vMeToTarget = pCandidate->GetAbsOrigin() - GetFiringPosition();
		Vector vBaseForward = UTIL_YawToVector( m_fDeployYaw );

		// crush everything to 2d for simplicity
		vMeToTarget.z = 0.0f;
		vCandVel.z = 0.0f;
		vBaseForward.z = 0.0f;

		Vector velCross = vBaseForward.Cross(vCandVel); // this encodes also some info on perpendicularity
		Vector vAimCross = vBaseForward.Cross(vMeToTarget);
		bool bTargetHeadedOutOfCone = !vCandVel.IsZero() && velCross.z * vAimCross.z >= 0; // true if same sign
		float flConeLeavingUrgency;

		if ( bTargetHeadedOutOfCone )
		{
			flConeLeavingUrgency = fabs( velCross.z / vCandVel.Length2D() ); 
			// just the sin; varies 0..1 where 1 means moving perpendicular to my aim
		}
		else
		{
			flConeLeavingUrgency = 0; // not at threat of leaving just yet
		}

		// the angle between my current yaw and what's needed to hit the target
		float flSwivelNeeded = fabs( UTIL_AngleDiff(  // i wish we weren't storing euler angles
			UTIL_VecToYaw( vMeToTarget ), m_fDeployYaw ) );
		flSwivelNeeded /= ASW_SENTRY_ANGLE; // normalize to 0..2

		float fBigness = 0.0f;

		int nClassify = pCandidate->Classify();
		switch( nClassify )
		{
		case CLASS_ASW_SHIELDBUG:
		case CLASS_ASW_MORTAR_BUG:
			fBigness = 4.0f;
			break;

		case CLASS_ASW_HARVESTER:
		case CLASS_ASW_RANGER:
			fBigness = 2.0f;
			break;
		}

		candidatescores[i] = Vector( 3.0f, -1.5f, 4.0f ).Dot( Vector( flConeLeavingUrgency, flSwivelNeeded, fBigness ) );
	}
	// find the highest scoring candidate
	int best = 0;
	for ( int i = 1 ; i < candidatescores.Count() ; ++i )
	{
		if ( candidatescores[i] > candidatescores[best] )
			best = i;
	}

	// NDebugOverlay::EntityBounds(candidates[best], 255, 255, 0, 255, 0.2f );

	return candidates[best];
}

#include "cbase.h"
#include "asw_grenade_cluster.h"
#include "asw_gamerules.h"
#include "world.h"
#include "asw_util_shared.h"
#include "iasw_spawnable_npc.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_fx_shared.h"
#include "particle_parse.h"
#include "te_effect_dispatch.h"
#include "asw_achievements.h"
#include "asw_player.h"
#include "asw_weapon_grenade_launcher.h"
#include "asw_drone_advanced.h"
#include "asw_buzzer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define CLUSTER_GRENADE_MODEL "models/swarm/grenades/HandGrenadeProjectile.mdl"

extern ConVar sk_plr_dmg_asw_r_g;
extern ConVar asw_grenade_vindicator_radius;
extern ConVar asw_vindicator_grenade_friction;
extern ConVar asw_vindicator_grenade_gravity;
extern ConVar asw_vindicator_grenade_elasticity;
extern ConVar rd_grenade_collision_fix;

extern ConVar asw_grenade_launcher_gravity;
extern ConVar rd_grenade_launcher_explode_on_contact;
extern ConVar rda_grenade_launcher_grenade_ricochet;

ConVar asw_cluster_grenade_min_detonation_time("asw_cluster_grenade_min_detonation_time", "0.9f", FCVAR_CHEAT, "Min. time before cluster grenade can detonate");
ConVar asw_cluster_grenade_fuse("asw_cluster_grenade_fuse", "2.0f", FCVAR_CHEAT, "Fuse length of cluster grenade");
ConVar asw_cluster_grenade_radius_check_interval("asw_cluster_grenade_radius_check_interval", "0.5f", FCVAR_CHEAT, "How often the cluster grenade checks for nearby drones to explode against");
ConVar asw_cluster_grenade_radius_check_scale("asw_cluster_grenade_radius_check_scale", "0.6f", FCVAR_CHEAT, "What fraction of the grenade's damage radius is used for the early detonation check");
ConVar asw_cluster_grenade_child_fuse_min("asw_cluster_grenade_child_fuse_min", "0.5", FCVAR_CHEAT, "Cluster grenade child cluster's minimum fuse length");
ConVar asw_cluster_grenade_child_fuse_max("asw_cluster_grenade_child_fuse_max", "1.0", FCVAR_CHEAT, "Cluster grenade child cluster's maximum fuse length");

ConVar rda_grenade_post_ricochet_velocity_multiplier("rda_grenade_post_ricochet_velocity_multiplier", "2", FCVAR_CHEAT, "Set to change GL grenade post ricochet velocity. Try values within [1..4] range");	//Orange. Advanced grenade behaviour.
ConVar rda_grenade_max_ricochets("rda_grenade_max_ricochets", "1", FCVAR_CHEAT, "Set to change how many times GL grenade can bouncy, 0 = unlim till timer");
ConVar rda_grenade_allow_electro_amped("rda_grenade_allow_electro_amped", "0", FCVAR_CHEAT, "Make grenades benefit from tesla additions");
ConVar rda_grenade_electrostun_duration("rda_grenade_electrostun_duration", "0.5", FCVAR_CHEAT, "Electrostun duration time of amped grenades. Valid range 0.0 - 5.0 s");
ConVar rda_grenade_electrostun_range_multiplier("rda_grenade_electrostun_range_multiplier", "1", FCVAR_CHEAT, "Electrostun range of amped grenades. Valid range 0.2 - 1.5 of current damage range");
ConVar rda_grenade_reflector_radius("rda_grenade_reflector_radius", "72", FCVAR_CHEAT, "If grenade hit something while reflector alien is this nearby grenade will be reflected");
ConVar rda_grenade_search_reflectors("rda_grenade_search_reflectors", "0", FCVAR_CHEAT, "Set to 1 to allow grenades to be reflected from reflector aliens");

LINK_ENTITY_TO_CLASS( asw_grenade_cluster, CASW_Grenade_Cluster );

PRECACHE_REGISTER( asw_grenade_cluster );

BEGIN_DATADESC( CASW_Grenade_Cluster )	
	DEFINE_THINKFUNC( CheckNearbyDrones ),
	DEFINE_THINKFUNC( Detonate ),
	DEFINE_FIELD( m_fDetonateTime, FIELD_TIME ),
	DEFINE_FIELD( m_fEarliestAOEDetonationTime, FIELD_TIME ),
	DEFINE_FIELD( m_bAdvancedRicochet, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iMaxRicochets, FIELD_INTEGER ),

	DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
	DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableWithReset", InputEnableWithReset),
	DEFINE_INPUTFUNC(FIELD_VOID, "ReflectBack", InputReflectBack),
	DEFINE_INPUTFUNC(FIELD_VOID, "ReflectBackSimple", InputReflectBack),
	DEFINE_INPUTFUNC(FIELD_VOID, "ReflectRandomly", InputReflectRandomly),
END_DATADESC()

BEGIN_ENT_SCRIPTDESC(CASW_Grenade_Cluster, CBaseCombatCharacter, "Cluster grenade")
	DEFINE_SCRIPTFUNC(Disable, "Disable the grenade")
	DEFINE_SCRIPTFUNC(Enable, "Enable the grenade, different time setting like explosion time correctly adjusted after disable state")
	DEFINE_SCRIPTFUNC(EnableWithReset, "Enable the grenade, different time setting like explosion time reset like grenade is newly created")
	DEFINE_SCRIPTFUNC(ReflectBack, "Reflect grenade preciselly to firer marine position if marine is alive, ReflectBackSimple used otherwise")
	DEFINE_SCRIPTFUNC(ReflectBackSimple, "Reflect grenade near to initial fire position")
	DEFINE_SCRIPTFUNC(ReflectRandomly, "Reflect grenade to random porition")
END_SCRIPTDESC();

void CASW_Grenade_Cluster::Spawn( void )
{
	Precache();
	SetModel( CLUSTER_GRENADE_MODEL );
	m_nSkin = 0;
	
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );

	m_flDamage		= sk_plr_dmg_asw_r_g.GetFloat();
	m_DmgRadius		= 220.0f;

	m_takedamage	= DAMAGE_YES;
	m_iHealth = 1;

	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	SetSolid( SOLID_BBOX );
	SetGravity( asw_vindicator_grenade_gravity.GetFloat() );
	SetFriction( asw_vindicator_grenade_friction.GetFloat() );
	SetElasticity( asw_vindicator_grenade_elasticity.GetFloat() );
	SetCollisionGroup( ASW_COLLISION_GROUP_GRENADES );

	SetTouch( &CASW_Grenade_Cluster::VGrenadeTouch );

	CreateEffects();
	m_hFirer = NULL;

	// Tumble in air
	QAngle vecAngVelocity( random->RandomFloat ( -100, -500 ), 0, 0 );
	SetLocalAngularVelocity( vecAngVelocity );

	m_fEarliestAOEDetonationTime = GetEarliestAOEDetonationTime();
	m_fEarliestTouchDetonationTime = GetEarliestTouchDetonationTime();

	m_iClusters = 0;
	m_bMaster = true;	

	//EmitSound( "ASWGrenade.Alarm" );
	SetFuseLength(asw_cluster_grenade_fuse.GetFloat());	

	m_iMaxRicochets = 0;

	if (m_fDetonateTime <= gpGlobals->curtime + asw_cluster_grenade_radius_check_interval.GetFloat())
	{
		SetThink( &CASW_Grenade_Cluster::Detonate );
		SetNextThink( m_fDetonateTime );
	}
	else
	{
		SetThink( &CASW_Grenade_Cluster::CheckNearbyDrones );
		SetNextThink( gpGlobals->curtime + asw_cluster_grenade_radius_check_interval.GetFloat() );
	}
	m_CreatorWeaponClass = (Class_T)CLASS_ASW_UNKNOWN;

	m_bTeslaAmped = false;

	m_bDisabled = false;
}

void CASW_Grenade_Cluster::Precache()
{
	BaseClass::Precache();

	PrecacheModel(CLUSTER_GRENADE_MODEL);
	PrecacheScriptSound( "ASWGrenade.Alarm" );
	PrecacheParticleSystem( "explosion_grenade" );
}

void CASW_Grenade_Cluster::SetFuseLength(float fSeconds)
{
	m_fDetonateTime = gpGlobals->curtime + fSeconds;	
}

void CASW_Grenade_Cluster::CheckNearbyDrones()
{	
	// see if an alien is nearby
	if (gpGlobals->curtime >= m_fEarliestAOEDetonationTime)
	{
		float flRadius = asw_cluster_grenade_radius_check_scale.GetFloat() * m_DmgRadius;
		Vector vecSrc = GetAbsOrigin();
		CBaseEntity *pEntity = NULL;
		for ( CEntitySphereQuery sphere( vecSrc, flRadius ); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
		{
			if (!pEntity->IsAlien())
				continue;

			Detonate();
			return;
		}

		if (gpGlobals->curtime >= m_fDetonateTime)
		{
			Detonate();
			return;
		}
	}
	
	if (m_fDetonateTime <= gpGlobals->curtime + asw_cluster_grenade_radius_check_interval.GetFloat())
	{
		SetThink( &CASW_Grenade_Cluster::Detonate );
		SetNextThink( m_fDetonateTime );
	}
	else
	{
		SetThink( &CASW_Grenade_Cluster::CheckNearbyDrones );
		SetNextThink( gpGlobals->curtime + asw_cluster_grenade_radius_check_interval.GetFloat() );
	}
}

static const char* s_pReflectContext = "ReflectContext";
void CASW_Grenade_Cluster::ReflectBackThink()
{
	ReflectBack();
	SetContextThink(NULL, gpGlobals->curtime, s_pReflectContext);
}

bool CASW_Grenade_Cluster::IsReflectorNearby()
{
	float flRadius = rda_grenade_reflector_radius.GetFloat();
	flRadius = clamp(flRadius, 1, 500);
	Vector vecSrc = GetAbsOrigin();
	CBaseEntity* pEntity = NULL;
	for (CEntitySphereQuery sphere(vecSrc, flRadius); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
	{
		if (!pEntity->IsAlien())
			continue;
		//Note: since this called from inside touch function, we have to delay our reflect a little bit
		if (pEntity->Classify() == CLASS_ASW_BUZZER)
		{
			CASW_Buzzer* pBuzzer = static_cast<CASW_Buzzer*>(pEntity);
			if (pBuzzer->m_bGrenadeReflector && pBuzzer->GetHealth() > 0)
			{
				SetContextThink(&CASW_Grenade_Cluster::ReflectBack, gpGlobals->curtime + 0.01f, s_pReflectContext);
				return true;
			}
		}
		else
		{
			CASW_Alien* pAlien = static_cast<CASW_Alien*>(pEntity);
			if (pAlien->m_bGrenadeReflector && pAlien->GetHealth() > 0)
			{
				SetContextThink(&CASW_Grenade_Cluster::ReflectBack, gpGlobals->curtime + 0.01f, s_pReflectContext);
				return true;
			}
		}
	}
	return false;
}

CASW_Grenade_Cluster* CASW_Grenade_Cluster::Cluster_Grenade_Create( float flDamage, float fRadius, int iClusters, const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, CBaseEntity *pCreatorWeapon )
{
	CASW_Grenade_Cluster *pGrenade = (CASW_Grenade_Cluster *)CreateEntityByName( "asw_grenade_cluster" );
	pGrenade->SetAbsAngles( angles );
	UTIL_SetOrigin( pGrenade, position );
	pGrenade->m_vecInitPos = position;
	pGrenade->Spawn();
	pGrenade->m_flDamage = flDamage;
	pGrenade->m_DmgRadius = fRadius;
	pGrenade->m_hFirer = pOwner;
	pGrenade->SetOwnerEntity( pOwner );	
	pGrenade->SetAbsVelocity( velocity );
	pGrenade->SetClusters(iClusters, true);
	pGrenade->m_hCreatorWeapon = pCreatorWeapon;
	if( pCreatorWeapon )
		pGrenade->m_CreatorWeaponClass = pCreatorWeapon->Classify();

	IGameEvent* event = gameeventmanager->CreateEvent("cluster_grenade_create");
	if (event)
	{
		event->SetInt("entindex", pGrenade->entindex());
		event->SetInt("marine", pOwner ? pOwner->entindex() : 0);
		event->SetInt("weapon", pCreatorWeapon ? pCreatorWeapon->entindex() : 0);
		gameeventmanager->FireEvent(event);
	}

	// hack attack!  for some reason, grenades refuse to be affect by damage forces until they're actually dead
	//  so we kill it immediately.
	//pGrenade->TakeDamage(CTakeDamageInfo(pGrenade, pGrenade, Vector(0, 0, 1), position, 10, DMG_SLASH));

	return pGrenade;
}

void CASW_Grenade_Cluster::DoExplosion()
{
	// scorch the ground
	trace_t		tr;
	UTIL_TraceLine ( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, -80 ), MASK_SHOT, 
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

		UTIL_ASW_ScreenShake( GetAbsOrigin(), 25.0, 150.0, 1.0, 750, SHAKE_START );
	}

	UTIL_ASW_GrenadeExplosion( GetAbsOrigin(), m_DmgRadius );

	if ( !m_bSilent )
	{
		EmitSound( "ASWGrenade.Explode" );
	}

	// damage to nearby things
	CTakeDamageInfo info( this, m_hFirer.Get(), m_flDamage, DMG_BLAST );
	info.SetWeapon( m_hCreatorWeapon );
	ASWGameRules()->RadiusDamage( info, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL );

	if (m_bTeslaAmped)
	{
		DispatchParticleEffect("Blink", GetAbsOrigin(), vec3_angle);

		int nNumPicked = 3;
		CUtlVector<CASW_Alien*> candidates;
		CUtlVector<CASW_Alien*> picked;
		
		float range = rda_grenade_electrostun_range_multiplier.GetFloat();
		range = clamp(range, 0.2f, 1.5f);
		range *= m_DmgRadius;

		CBaseEntity* pEntity = NULL;
		for (CEntitySphereQuery sphere(GetAbsOrigin(), range); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
		{
			if (!pEntity->IsAlienClassType()) //ignore buzzers
				continue;

			if (pEntity->m_iHealth < 0)
				continue;

			CASW_Alien* pAlien = static_cast<CASW_Alien*>(pEntity);
			if (!pAlien->m_bTeslable)
				continue;

			//ToDo. Perhaps better to mod RadiusDamage() with corresponding code than do more traces.
			if (FVisible(pEntity, MASK_SOLID_BRUSHONLY))
				candidates.AddToTail(pAlien);
		}

		for (int i = 0; ( (i < nNumPicked) && (candidates.Count() > 0) ); i++)
		{
			int nChosen = RandomInt(0, candidates.Count() - 1);
			picked.AddToTail(candidates[nChosen]);
			candidates.Remove(nChosen);
		}

		float duration = rda_grenade_electrostun_duration.GetFloat();
		duration = clamp(duration, 0.0f, 5.0f);
		for (int i = 0; i < picked.Count(); i++)
		{
			picked[i]->ElectroStun(duration);
		}
	}
}

extern ConVar asw_medal_explosive_kills;

void CASW_Grenade_Cluster::Detonate()
{
	m_takedamage = DAMAGE_NO;	

	int iPreExplosionKills = 0;
	CASW_Marine* pMarine = NULL;
	CASW_Marine_Resource* pPMR = NULL;
	CBaseEntity* pFirer = m_hFirer.Get();
	if ( pFirer && pFirer->Classify() == CLASS_ASW_MARINE )
	{ 
		pMarine = assert_cast<CASW_Marine*>( pFirer );
		pPMR = pMarine->GetMarineResource();
	}

	if ( pMarine && pPMR )
		iPreExplosionKills = pPMR->m_iAliensKilled;
	
	DoExplosion();

	if ( pMarine && pPMR )
	{
		int iKilledByExplosion = pPMR->m_iAliensKilled - iPreExplosionKills;
		if ( iKilledByExplosion > pPMR->m_iSingleGrenadeKills )
		{
			pPMR->m_iSingleGrenadeKills = iKilledByExplosion;
			if ( iKilledByExplosion > asw_medal_explosive_kills.GetInt() && pMarine->GetCommander() && pMarine->IsInhabited() )
			{
				pMarine->GetCommander()->AwardAchievement( ACHIEVEMENT_ASW_GRENADE_MULTI_KILL );
			}
		}
		if (m_bKicked)
			pPMR->m_iKickedGrenadeKills += iKilledByExplosion;

		// primary cluster counts as a shot fired
		if (m_iClusters > 0)
		{
			pPMR->UsedWeapon(NULL, 1);
		}
	}

	while (m_iClusters > 0)
	{
		float fYaw = random->RandomFloat(0, 360);
		QAngle ang(0, fYaw, 0);
		Vector newVel;
		AngleVectors(ang, &newVel);
		newVel *= random->RandomFloat(150, 200);
		newVel.z = random->RandomFloat(200, 400);
		CASW_Grenade_Cluster *pGrenade = CASW_Grenade_Cluster::Cluster_Grenade_Create( 
			m_flDamage,
			m_DmgRadius,
			0,
			GetAbsOrigin(), ang, newVel, AngularImpulse(0,0,0), pFirer, m_hCreatorWeapon);
		if (pGrenade)
		{
			pGrenade->m_fEarliestAOEDetonationTime = 0;	// children can go whenever they want
			pGrenade->m_fEarliestTouchDetonationTime = 0;
			pGrenade->m_takedamage = DAMAGE_NO;
			pGrenade->SetFuseLength(random->RandomFloat(asw_cluster_grenade_child_fuse_min.GetFloat(), asw_cluster_grenade_child_fuse_max.GetFloat()));
			pGrenade->SetClusters(0, false);	
			pGrenade->m_bKicked = m_bKicked;
		}
		m_iClusters--;
	}

	UTIL_Remove( this );
}

float CASW_Grenade_Cluster::GetEarliestAOEDetonationTime()
{
	return gpGlobals->curtime + asw_cluster_grenade_min_detonation_time.GetFloat();
}

void CASW_Grenade_Cluster::SetClusters(int iClusters, bool bMaster)
{
	BaseClass::SetClusters(iClusters, bMaster);
	if (!bMaster)
	{
		SetThink( &CASW_Grenade_Cluster::Detonate );
		SetNextThink( m_fDetonateTime );
	}
}

float CASW_Grenade_Cluster::GetEarliestTouchDetonationTime()
{
	extern ConVar rd_grenade_launcher_arm_time;
	return gpGlobals->curtime + rd_grenade_launcher_arm_time.GetFloat();
}

void CASW_Grenade_Cluster::VGrenadeTouch(CBaseEntity* pOther)
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
		return;

	// make sure we don't die on things we shouldn't
	if ( !ASWGameRules() || !ASWGameRules()->ShouldCollide( GetCollisionGroup(), pOther->GetCollisionGroup() ) )
		return;

	if ( rda_grenade_search_reflectors.GetBool() && IsReflectorNearby() )
		return;

	//fix for situation when drone attacks through marine from behind and touches grenade launcher's grenade firing opposite direction
	if ( pOther->IsNPC() && pOther->Classify() == CLASS_ASW_DRONE )
	{
		if ( m_hCreatorWeapon.Get() && m_hCreatorWeapon.Get()->Classify() == CLASS_ASW_GRENADE_LAUNCHER ) //filter out grenades not related to grenade laucher
		{
			if ( m_hFirer.Get() && m_hFirer.Get()->Classify() == CLASS_ASW_MARINE )
			{
				CASW_Marine* pMarine = assert_cast<CASW_Marine*>(m_hFirer.Get());
				Vector vecDist = pOther->GetAbsOrigin() - pMarine->GetAbsOrigin();
				if (vecDist.LengthSqr() <= 3200) //some small enough delta squared. may have to increase for drone long ranged attack //around 41^2 in test when drone stays close
				{
					Vector vecNadeDir = UTIL_YawToVector(GetAbsAngles().y); //direction of nade, normalized
					vecDist.z = 0;
					vecDist.NormalizeInPlace();
					float dotpr = DotProduct(vecDist, vecNadeDir); //cos of angle between our vectors, -1 gives us drone right opposite direction
					if (dotpr < -0.05) //around 90 degree and more, so back-sided and back drones but not front
					{
						CASW_Drone_Advanced* pDrone = assert_cast<CASW_Drone_Advanced*>(pOther);
						if ((m_vecInitPos - GetAbsOrigin()).LengthSqr() < 36)//check if we dont fire through marine head, some small delta squared //nade through head gives us around 60^2
						{
							pDrone->CollisionSaveAndRestore(0.2f); // calls think func //TODO: calculate time based on grenade speed through drone model?
							pDrone->SetCollisionGroup(ASW_COLLISION_GROUP_PASSABLE); //make this drone notsolid to grenade

							//for some reason we unable to change current grenade behaviour, so we create new instead with same data
							//copy from void CASW_Weapon_Grenade_Launcher::PrimaryAttack( void )
							CASW_Grenade_Cluster* pGrenade = CASW_Grenade_Cluster::Cluster_Grenade_Create(
								m_flDamage,
								m_DmgRadius,
								m_iClusters,
								GetAbsOrigin(), GetAbsAngles(), GetAbsVelocity(), AngularImpulse(0, 0, 0), m_hFirer.Get(), m_hCreatorWeapon.Get());

							UTIL_Remove(this);

							if (pGrenade)
							{
								pGrenade->SetGravity(asw_grenade_launcher_gravity.GetFloat());

								if (rda_grenade_launcher_grenade_ricochet.GetBool())
									pGrenade->SetAdvancedRicochet(true);
								else
									pGrenade->SetExplodeOnWorldContact(rd_grenade_launcher_explode_on_contact.GetBool());

								//pMarine->OnWeaponFired(this, 1); Do not fire stats since replacement
							}
							return;
						}
					}
				}
			}
		}
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

	//Orange. Ricochet stuff
	if ( m_bAdvancedRicochet )
	{
		float multip = clamp( rda_grenade_post_ricochet_velocity_multiplier.GetFloat(), 1, 4 );
		if ( rda_grenade_max_ricochets.GetInt() > 0 )
		{
			if ( rda_grenade_max_ricochets.GetInt() >= ++m_iMaxRicochets )
			{
				SetAbsVelocity( multip * GetAbsVelocity() );
			}
			else
			{
				Detonate();
				return;
			}
		}
		else
		{
			SetAbsVelocity( multip * GetAbsVelocity() );
		}
	}

	// can't detonate yet?
	if ( gpGlobals->curtime < m_fEarliestTouchDetonationTime )
		return;

	if ( pOther->m_takedamage != DAMAGE_NO )
	{
		if ( pOther->IsNPC() && pOther->Classify() != CLASS_ASW_MARINE )
			Detonate();
	}
}

int	CASW_Grenade_Cluster::OnTakeDamage(const CTakeDamageInfo& info)
{
	if (rda_grenade_allow_electro_amped.GetBool() && info.GetDamageType() & DMG_SHOCK)
	{
		if (info.GetWeapon() && info.GetWeapon()->Classify() == CLASS_ASW_TESLA_GUN)
		{
			m_bTeslaAmped = true;
		}
	}

	return BaseClass::OnTakeDamage(info);
}

void CASW_Grenade_Cluster::Disable()
{
	if (m_bDisabled)
		return;
	m_bDisabled = true;

	SetMoveType( MOVETYPE_NONE );
	m_takedamage = DAMAGE_NO;
	SetCollisionGroup(COLLISION_GROUP_DEBRIS_TRIGGER);
	SetThink(NULL);
	SetTouch(NULL);

	m_fSavedDisableTime = gpGlobals->curtime;
}

void CASW_Grenade_Cluster::Enable()
{
	if (!m_bDisabled)
		return;

	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	m_takedamage = DAMAGE_YES;
	SetCollisionGroup(ASW_COLLISION_GROUP_GRENADES);

	SetTouch(&CASW_Grenade_Cluster::VGrenadeTouch);

	//shift our times by our disable delay
	m_fDetonateTime += gpGlobals->curtime - m_fSavedDisableTime;
	m_fEarliestAOEDetonationTime += gpGlobals->curtime - m_fSavedDisableTime;
	m_fEarliestTouchDetonationTime += gpGlobals->curtime - m_fSavedDisableTime;

	if (m_fDetonateTime <= gpGlobals->curtime + asw_cluster_grenade_radius_check_interval.GetFloat())
	{
		SetThink(&CASW_Grenade_Cluster::Detonate);
		SetNextThink(m_fDetonateTime);
	}
	else
	{
		SetThink(&CASW_Grenade_Cluster::CheckNearbyDrones);
		SetNextThink(gpGlobals->curtime + asw_cluster_grenade_radius_check_interval.GetFloat());
	}
}

void CASW_Grenade_Cluster::EnableWithReset()
{
	if (!m_bDisabled)
		return;

	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	m_takedamage = DAMAGE_YES;
	SetCollisionGroup(ASW_COLLISION_GROUP_GRENADES);

	SetTouch(&CASW_Grenade_Cluster::VGrenadeTouch);

	//reset times like if grenade is newly created
	SetFuseLength(asw_cluster_grenade_fuse.GetFloat());
	m_fEarliestAOEDetonationTime = GetEarliestAOEDetonationTime();
	m_fEarliestTouchDetonationTime = GetEarliestTouchDetonationTime();

	if (m_fDetonateTime <= gpGlobals->curtime + asw_cluster_grenade_radius_check_interval.GetFloat())
	{
		SetThink(&CASW_Grenade_Cluster::Detonate);
		SetNextThink(m_fDetonateTime);
	}
	else
	{
		SetThink(&CASW_Grenade_Cluster::CheckNearbyDrones);
		SetNextThink(gpGlobals->curtime + asw_cluster_grenade_radius_check_interval.GetFloat());
	}
}

void CASW_Grenade_Cluster::ReflectBack()
{
	CBaseEntity* pFirer = m_hFirer.Get();
	if (pFirer && pFirer->Classify() == CLASS_ASW_MARINE)
	{
		//NDebugOverlay::Box(GetAbsOrigin(), Vector(2, 2, 2), Vector(-2, -2, -2), 0, 0, 0, 255, 6);
		//NDebugOverlay::Box(pFirer->GetAbsOrigin(), Vector(2, 2, 2), Vector(-2, -2, -2), 255, 255, 255, 255, 6);
		Vector newVel = UTIL_LaunchVector(GetAbsOrigin(), pFirer->GetAbsOrigin(), GetGravity()) * 28.0f;
		SetAbsVelocity(newVel);
	}
	else
	{
		ReflectBackSimple();
	}
}

void CASW_Grenade_Cluster::ReflectBackSimple()
{
	Vector vec = GetAbsVelocity();
	SetAbsVelocity(Vector(-vec.x, -vec.y, vec.z));
}

void CASW_Grenade_Cluster::ReflectRandomly()
{
	Vector vec = GetAbsVelocity();
	float len = vec.Length2D();
	//construct random vector with same 2d length
	float rad = RandomFloat(0, 2 * M_PI);
	float sin, cos;
	FastSinCos(rad, &sin, &cos);
	SetAbsVelocity(Vector(cos * len, sin * len, vec.z));
}

void CASW_Grenade_Cluster::InputDisable(inputdata_t& inputdata)
{
	Disable();
}

void CASW_Grenade_Cluster::InputEnable(inputdata_t& inputdata)
{
	Enable();
}

void CASW_Grenade_Cluster::InputEnableWithReset(inputdata_t& inputdata)
{
	EnableWithReset();
}

void CASW_Grenade_Cluster::InputReflectBack(inputdata_t& inputdata)
{
	ReflectBack();
}

void CASW_Grenade_Cluster::InputReflectBackSimple(inputdata_t& inputdata)
{
	ReflectBackSimple();
}

void CASW_Grenade_Cluster::InputReflectRandomly(inputdata_t& inputdata)
{
	ReflectRandomly();
}

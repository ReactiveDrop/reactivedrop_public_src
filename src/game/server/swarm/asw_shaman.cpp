#include "cbase.h"
#include "asw_shaman.h"
#include "npcevent.h"
#include "asw_gamerules.h"
#include "asw_shareddefs.h"
#include "asw_fx_shared.h"
#include "asw_grenade_cluster.h"
#include "world.h"
#include "particle_parse.h"
#include "asw_util_shared.h"
#include "ai_squad.h"
#include "asw_marine.h"
#include "asw_ai_behavior_fear.h"
#include "gib.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_shaman, CASW_Shaman );

IMPLEMENT_SERVERCLASS_ST(CASW_Shaman, DT_ASW_Shaman)
	SendPropEHandle		( SENDINFO( m_hHealingTarget ) ),
END_SEND_TABLE()


BEGIN_DATADESC( CASW_Shaman )
DEFINE_EMBEDDEDBYREF( m_pExpresser ),
END_DATADESC()
 
int AE_SHAMAN_SPRAY_START;
int AE_SHAMAN_SPRAY_END;

ConVar asw_shaman_health( "asw_shaman_health", "59", FCVAR_CHEAT );
ConVar rd_shaman_healing_speed( "rd_shaman_healing_speed", "0", FCVAR_CHEAT, "Number of health shaman gives per second. 0 means heal ~40% of maxhealth in 1 second" );
extern ConVar asw_debug_alien_damage;

#define RD_SHAMAN_HEAL_INTERVAL 1.0f

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
CASW_Shaman::CASW_Shaman()
{
	m_pszAlienModelName = "models/aliens/shaman/shaman.mdl";
	// reactivedrop: this is a must or burrowed aliens spawned from spawner 
	// have incorrect collision group and block other aliens
	m_nAlienCollisionGroup = ASW_COLLISION_GROUP_ALIEN;
	m_flHealTime = 0.0f;
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Shaman::Spawn( void )
{
	SetHullType( HULL_MEDIUM );

	BaseClass::Spawn();

	SetHullType( HULL_MEDIUM );
	SetBloodColor( BLOOD_COLOR_GREEN );
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_AUTO_DOORS );

	AddFactionRelationship( FACTION_MARINES, D_FEAR, 10 );

	SetIdealState( NPC_STATE_ALERT );
	m_bNeverRagdoll = true;
}

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Shaman::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "Shaman.Pain" );
	PrecacheScriptSound( "Shaman.Die" );
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
int CASW_Shaman::GetBaseHealth()
{
	return asw_shaman_health.GetInt();
}


#if 0
//-----------------------------------------------------------------------------
// Purpose: A scalar to apply to base (walk/run) speeds.
//-----------------------------------------------------------------------------
float CASW_Shaman::GetMovementSpeedModifier()
{
	// don't like the way this is done, but haven't thought of a better approach yet
	if ( IsRunningBehavior() && static_cast< CAI_ASW_Behavior * >(  GetPrimaryBehavior() )->Classify() == BEHAVIOR_CLASS_FEAR )
	{
		return ASW_CONCAT_SPEED_ADD( 0.55f );
	}

	return BaseClass::GetMovementSpeedModifier();
}
#endif

//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
float CASW_Shaman::MaxYawSpeed( void )
{
	return 32.0f;// * GetMovementSpeedModifier();
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Shaman::PainSound( const CTakeDamageInfo &info )
{
	// reactivedrop: apparently npc_tier_tables don't work, changed to EmitSound()
	//	// sounds for pain and death are defined in the npc_tier_tables excel sheet
	//	// they are called from the asw_alien base class (m_fNextPainSound is handled there)
	//	BaseClass::PainSound(info);
	if ( gpGlobals->curtime > m_fNextPainSound )
	{
		EmitSound( "Shaman.Pain" );
		m_fNextPainSound = gpGlobals->curtime + RandomFloat( 0.75f, 1.25f );
	}
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Shaman::DeathSound( const CTakeDamageInfo &info )
{
	// reactivedrop: apparently npc_tier_tables don't work, changed to EmitSound()
	//	// sounds for pain and death are defined in the npc_tier_tables excel sheet
	//	// they are called from the asw_alien base class
	//	BaseClass::DeathSound(info);
	EmitSound( "Shaman.Die" );
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
void CASW_Shaman::HandleAnimEvent( animevent_t *pEvent )
{
	int nEvent = pEvent->Event();

	if ( nEvent == AE_SHAMAN_SPRAY_START )
	{
		//m_HealOtherBehavior.HandleBehaviorEvent( this, BEHAVIOR_EVENT_START_HEAL, 0 );
		return;
	}
	if ( nEvent == AE_SHAMAN_SPRAY_END )
	{
		//m_HealOtherBehavior.HandleBehaviorEvent( this, BEHAVIOR_EVENT_FINISH_HEAL, 0 );
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}


//-----------------------------------------------------------------------------
// Purpose:	
// Input:	
// Output:	
//-----------------------------------------------------------------------------
bool CASW_Shaman::CreateBehaviors()
{
	/*
	AddBehavior( &m_CombatStunBehavior );
	m_CombatStunBehavior.Init();
	*/

	//self.AddBehavior( "behavior_protect", BehaviorParms );


	m_HealOtherBehavior.KeyValue( "heal_distance", "300" );
	m_HealOtherBehavior.KeyValue( "approach_distance", "120" );
	m_HealOtherBehavior.KeyValue( "heal_amount", "0.04" );	// percentage per tick healed
	m_HealOtherBehavior.KeyValue( "consideration_distance", "800" );
	AddBehavior( &m_HealOtherBehavior );
	m_HealOtherBehavior.Init();


	m_ScuttleBehavior.KeyValue( "pack_range", "800" );
	m_ScuttleBehavior.KeyValue( "min_backoff", "150" );
	m_ScuttleBehavior.KeyValue( "max_backoff", "300" );
	m_ScuttleBehavior.KeyValue( "min_yaw", "10" );
	m_ScuttleBehavior.KeyValue( "max_yaw", "25" );
	m_ScuttleBehavior.KeyValue( "min_wait", "1.25" );
	m_ScuttleBehavior.KeyValue( "max_wait", "2.0" );
	AddBehavior( &m_ScuttleBehavior );
	m_ScuttleBehavior.Init();

	/*
	AddBehavior( &m_FearBehavior );
	m_FearBehavior.Init();
	*/

	AddBehavior( &m_IdleBehavior );
	m_IdleBehavior.Init();

	return BaseClass::CreateBehaviors();
}

void CASW_Shaman::SetCurrentHealingTarget( CBaseEntity *pTarget )
{
	if ( pTarget != m_hHealingTarget.Get() )
	{
		m_hHealingTarget = pTarget;
	}
}

void CASW_Shaman::NPCThink( void )
{
	BaseClass::NPCThink();

	CBaseEntity *pHealTarget = NULL;
	if ( GetPrimaryBehavior() == &m_HealOtherBehavior )
	{
		pHealTarget = m_HealOtherBehavior.GetCurrentHealTarget();
		if ( pHealTarget )
		{
			if ( rd_shaman_healing_speed.GetInt() > 0 )
			{
				if ( m_flHealTime < gpGlobals->curtime )
				{
					pHealTarget->TakeHealth( rd_shaman_healing_speed.GetInt(), DMG_GENERIC );

					while ( m_flHealTime + RD_SHAMAN_HEAL_INTERVAL < gpGlobals->curtime )
						m_flHealTime += RD_SHAMAN_HEAL_INTERVAL;

					m_flHealTime = m_flHealTime + RD_SHAMAN_HEAL_INTERVAL;
				}
			}
			else
			{
				pHealTarget->TakeHealth( m_HealOtherBehavior.m_flHealAmount * pHealTarget->GetMaxHealth(), DMG_GENERIC );
			}
		}
	}
	SetCurrentHealingTarget( pHealTarget );
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( asw_shaman, CASW_Shaman )
	DECLARE_ANIMEVENT( AE_SHAMAN_SPRAY_START );
	DECLARE_ANIMEVENT( AE_SHAMAN_SPRAY_END );
AI_END_CUSTOM_NPC()


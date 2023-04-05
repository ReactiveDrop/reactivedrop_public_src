//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: This is the soldier version of the combine, analogous to the HL1 grunt.
//
//=============================================================================//

#include "cbase.h"
#include "ai_hull.h"
#include "ai_motor.h"
#include "npc_combines.h"
#include "bitstring.h"
#include "engine/IEngineSound.h"
#include "soundent.h"
#include "ndebugoverlay.h"
#include "npcevent.h"
#include "game.h"
#include "ammodef.h"
#include "explode.h"
#include "ai_memory.h"
#include "Sprite.h"
#include "soundenvelope.h"
#include "weapon_physcannon.h"
#include "gameweaponmanager.h"
#include "vehicle_base.h"
#include "ai_network.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_combine_s_health( "sk_combine_s_health", "50", FCVAR_CHEAT );
ConVar sk_combine_s_kick( "sk_combine_s_kick", "10", FCVAR_CHEAT );

ConVar sk_combine_guard_health( "sk_combine_guard_health", "70", FCVAR_CHEAT );
ConVar sk_combine_guard_kick( "sk_combine_guard_kick", "15", FCVAR_CHEAT );

#ifndef INFESTED_DLL
// Whether or not the combine guard should spawn health on death
ConVar combine_guard_spawn_health( "combine_guard_spawn_health", "1" );

extern ConVar sk_plr_dmg_buckshot;
extern ConVar sk_plr_num_shotgun_pellets;

//Whether or not the combine should spawn health on death
ConVar	combine_spawn_health( "combine_spawn_health", "1" );
#endif

LINK_ENTITY_TO_CLASS( npc_combine_s, CNPC_CombineS );
LINK_ENTITY_TO_CLASS( npc_combine_shotgun, CNPC_CombineS );
LINK_ENTITY_TO_CLASS( npc_combine_elite, CNPC_CombineS );


#define AE_SOLDIER_BLOCK_PHYSICS		20 // trying to block an incoming physics object

extern Activity ACT_WALK_EASY;
extern Activity ACT_WALK_MARCH;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CombineS::Spawn( void )
{
	Precache();
	SetModel( STRING( GetModelName() ) );

	if ( IsElite() )
	{
		// Stronger, tougher.
		SetKickDamage( sk_combine_guard_kick.GetFloat() );
	}
	else
	{
		SetKickDamage( sk_combine_s_kick.GetFloat() );
	}

#ifndef INFESTED_DLL
	CapabilitiesAdd( bits_CAP_ANIMATEDFACE );
#endif
	CapabilitiesAdd( bits_CAP_MOVE_SHOOT );
	CapabilitiesAdd( bits_CAP_DOORS_GROUP );

	BaseClass::Spawn();

	if ( m_iUseMarch && !HasSpawnFlags( SF_NPC_START_EFFICIENT ) )
	{
		Msg( "Soldier %s is set to use march anim, but is not an efficient AI. The blended march anim can only be used for dead-ahead walks!\n", GetDebugName() );
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_CombineS::Precache()
{
	if ( !GetModelName() )
	{
		if ( ClassMatches( "npc_combine_elite" ) )
		{
			SetModelName( AllocPooledString( "models/combine_super_soldier.mdl" ) );
			m_nSkin = 0;

			if ( !m_spawnEquipment )
				m_spawnEquipment = AllocPooledString( "asw_weapon_ar2" );
		}
		else if ( ClassMatches( "npc_combine_shotgun" ) )
		{
			SetModelName( AllocPooledString( "models/combine_soldier.mdl" ) );
			m_nSkin = 1;

			if ( !m_spawnEquipment )
				m_spawnEquipment = AllocPooledString( "asw_weapon_shotgun" );
		}
		else
		{
			Assert( ClassMatches( "npc_combine_s" ) );
			SetModelName( AllocPooledString( "models/combine_soldier.mdl" ) );
			m_nSkin = 0;

			if ( !m_spawnEquipment )
				m_spawnEquipment = AllocPooledString( "asw_weapon_pdw" );
		}

		m_iNumGrenades = 1;
		AddSpawnFlags( SF_NPC_FADE_CORPSE );

		if ( !m_SquadName )
		{
			Assert( g_pBigAINet );
			if ( g_pBigAINet )
			{
				// Put us in a squad if there's no squad specified, just to make sure we get to do our cool AI stuff.
				char szSquadName[256];
				V_snprintf( szSquadName, sizeof( szSquadName ), "CombineSquadAuto%d", g_pBigAINet->NearestNodeToPoint( GetAbsOrigin(), false ) );
				m_SquadName = AllocPooledString( szSquadName );
			}
		}
	}

	const char *pModelName = STRING( GetModelName() );
	if ( !Q_stricmp( pModelName, "models/combine_super_soldier.mdl" ) )
	{
		m_fIsElite = true;
	}
	else
	{
		m_fIsElite = false;
	}

	PrecacheModel( STRING( GetModelName() ) );

	BaseClass::Precache();
}

int CNPC_CombineS::GetBaseHealth()
{
	if ( IsElite() )
	{
		return sk_combine_guard_health.GetInt();
	}

	return sk_combine_s_health.GetInt();
}

void CNPC_CombineS::DeathSound( const CTakeDamageInfo &info )
{
	// NOTE: The response system deals with this at the moment
	if ( GetFlags() & FL_DISSOLVING )
		return;

	GetSentences()->Speak( "COMBINE_DIE", SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS );
}


//-----------------------------------------------------------------------------
// Purpose: Soldiers use CAN_RANGE_ATTACK2 to indicate whether they can throw
//			a grenade. Because they check only every half-second or so, this
//			condition must persist until it is updated again by the code
//			that determines whether a grenade can be thrown, so prevent the 
//			base class from clearing it out. (sjb)
//-----------------------------------------------------------------------------
void CNPC_CombineS::ClearAttackConditions()
{
	bool fCanRangeAttack2 = HasCondition( COND_CAN_RANGE_ATTACK2 );

	// Call the base class.
	BaseClass::ClearAttackConditions();

	if ( fCanRangeAttack2 )
	{
		// We don't allow the base class to clear this condition because we
		// don't sense for it every frame.
		SetCondition( COND_CAN_RANGE_ATTACK2 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_CombineS::BuildScheduleTestBits( void )
{
	//Interrupt any schedule with physics danger (as long as I'm not moving or already trying to block)
	if ( m_flGroundSpeed == 0.0 && !IsCurSchedule( SCHED_FLINCH_PHYSICS ) )
	{
		SetCustomInterruptCondition( COND_HEAR_PHYSICS_DANGER );
	}

	BaseClass::BuildScheduleTestBits();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_CombineS::SelectSchedule( void )
{
	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CNPC_CombineS::GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info )
{
	switch ( iHitGroup )
	{
	case HITGROUP_HEAD:
	{
		// Soldiers take double headshot damage
		return 2.0f;
	}
	}

	return BaseClass::GetHitgroupDamageMultiplier( iHitGroup, info );
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_CombineS::HandleAnimEvent( animevent_t *pEvent )
{
	switch ( pEvent->Event() )
	{
	case AE_SOLDIER_BLOCK_PHYSICS:
		DevMsg( "BLOCKING!\n" );
		m_fIsBlocking = true;
		break;

	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

void CNPC_CombineS::OnChangeActivity( Activity eNewActivity )
{
	// Any new sequence stops us blocking.
	m_fIsBlocking = false;

	BaseClass::OnChangeActivity( eNewActivity );

	// Give each trooper a varied look for his march. Done here because if you do it earlier (eg Spawn, StartTask), the
	// pose param gets overwritten.
	if ( m_iUseMarch )
	{
		SetPoseParameter( "casual", RandomFloat() );
	}
}

void CNPC_CombineS::OnListened()
{
	BaseClass::OnListened();

	if ( HasCondition( COND_HEAR_DANGER ) && HasCondition( COND_HEAR_PHYSICS_DANGER ) )
	{
		if ( HasInterruptCondition( COND_HEAR_DANGER ) )
		{
			ClearCondition( COND_HEAR_PHYSICS_DANGER );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Translate base class activities into combot activites
//-----------------------------------------------------------------------------
Activity CNPC_CombineS::NPC_TranslateActivity( Activity eNewActivity )
{
	// If the special ep2_outland_05 "use march" flag is set, use the more casual marching anim.
	if ( m_iUseMarch && eNewActivity == ACT_WALK )
	{
		eNewActivity = ACT_WALK_MARCH;
	}

	return BaseClass::NPC_TranslateActivity( eNewActivity );
}


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_CombineS )
	DEFINE_KEYFIELD( m_iUseMarch, FIELD_INTEGER, "usemarch" ),
END_DATADESC()

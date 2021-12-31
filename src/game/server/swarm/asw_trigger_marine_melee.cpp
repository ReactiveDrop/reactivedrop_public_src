#include "cbase.h"
#include "asw_trigger_marine_melee.h"
#include "asw_marine.h"
#include "asw_movedata.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


BEGIN_DATADESC( CASW_Trigger_Marine_Melee )
	DEFINE_KEYFIELD( m_fAngleTolerance, FIELD_FLOAT, "AngleTolerance" ),

	// Outputs
	DEFINE_OUTPUT( m_OnTrigger, "OnTrigger" )
END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_asw_marine_melee, CASW_Trigger_Marine_Melee );

void CASW_Trigger_Marine_Melee::Spawn(void)
{
	BaseClass::Spawn();

	InitTrigger();

	ASSERTSZ(m_iHealth == 0, "trigger_multiple with health");
}

void CASW_Trigger_Marine_Melee::StartTouch( CBaseEntity *pOther )
{
	BaseClass::StartTouch( pOther );

	CASW_Marine *pMarine = CASW_Marine::AsMarine( pOther );
	if ( pMarine )
	{
		pMarine->m_hTouchingMeleeTriggers.AddToHead( this );
	}
}

void CASW_Trigger_Marine_Melee::EndTouch( CBaseEntity *pOther )
{
	BaseClass::EndTouch( pOther );

	CASW_Marine *pMarine = CASW_Marine::AsMarine( pOther );
	if ( pMarine )
	{
		pMarine->m_hTouchingMeleeTriggers.FindAndRemove( this );
	}
}

void CASW_Trigger_Marine_Melee::OnMeleeAttack( CASW_Melee_Attack *pAttack, CASW_Marine *pMarine, CMoveData *pMoveData )
{
	if ( m_bDisabled )
	{
		return;
	}

	CASW_MoveData *pASWMove = static_cast<CASW_MoveData *>( pMoveData );
	if ( pASWMove->m_iForcedAction != FORCED_ACTION_NONE )
	{
		return;
	}

	Vector vecMarineOffset = GetAbsOrigin() - pMarine->GetAbsOrigin();
	float flTargetAngle = UTIL_VecToYaw( vecMarineOffset );
	float flAngleOffset = abs( AngleDiff( flTargetAngle, pMarine->m_flMeleeYaw ) );
	if ( flAngleOffset <= m_fAngleTolerance / 2 )
	{
		m_OnTrigger.FireOutput( pMarine, this );
	}
}

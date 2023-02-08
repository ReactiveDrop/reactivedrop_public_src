#include "cbase.h"
#include "asw_trace_filter_doors.h"
#include "util.h"
#include "asw_door.h"
#include "ai_navtype.h"
#include "ai_waypoint.h"
#include "doors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pHandleEntity - 
//			contentsMask - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CASW_Trace_Filter_Doors::ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
{
	if ( !StandardFilterRules( pHandleEntity, contentsMask ) )
		return false;

	// Don't test if the game code tells us we should ignore this collision...
	CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );
	const CBaseEntity *pEntPass = EntityFromEntityHandle( m_pPassEnt );

	// don't hurt ourself
	if ( pEntPass == pEntity )
		return false;

	if ( !pEntity || pEntity->Classify() != CLASS_ASW_DOOR )
		return false;

	CASW_Door *pDoor = assert_cast<CASW_Door*>( pEntity );

	if ( m_bRequireLockedOrSealed )
	{
		if ( pDoor->GetSealAmount() > 0 || !pDoor->IsAutoOpen() )
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

extern ConVar asw_blink_debug;

// traces along an AI network route to see if a door is blocking the way
CASW_Door* UTIL_ASW_DoorBlockingRoute( AI_Waypoint_t *pRoute, bool bRequireLockedOrSealed )
{
	if ( !pRoute )
		return NULL;

	AI_Waypoint_t *pLastPoint = pRoute;
	pRoute = pRoute->GetNext();
	trace_t tr;
	Vector vecShiftUp = Vector( 0, 0, 20 );

	while ( pRoute )
	{
		CASW_Trace_Filter_Doors traceFilter( NULL, COLLISION_GROUP_NONE, bRequireLockedOrSealed );
		UTIL_TraceLine( pLastPoint->GetPos() + vecShiftUp, pRoute->GetPos() + vecShiftUp,
			MASK_NPCSOLID, &traceFilter, &tr );
		
		if ( asw_blink_debug.GetBool() )
		{
			DebugDrawLine( pLastPoint->GetPos() + vecShiftUp, pRoute->GetPos() + vecShiftUp, 0, 0, 255, true, 30.0f );
		}

		if ( tr.DidHit() )
		{
			// reactivedrop: added check for door
			// if not door continue checking route
			if ( tr.m_pEnt && tr.m_pEnt->Classify() == CLASS_ASW_DOOR )
				return assert_cast<CASW_Door*>(tr.m_pEnt);
		}

		pLastPoint = pRoute;
		pRoute = pRoute->GetNext();
	}

	return NULL;
}

bool UTIL_ASW_BrushBlockingRoute( AI_Waypoint_t *pRoute, const int nCollisionMask, const int nCollisionGroup )
{
	if ( !pRoute )
		return false;

	AI_Waypoint_t *pLastPoint = pRoute;
	pRoute = pRoute->GetNext();
	trace_t tr;
	Vector vecShiftUp = Vector( 0, 0, 20 );

	while ( pRoute )
	{
		CTraceFilterSimple traceFilter( NULL, nCollisionGroup );
		UTIL_TraceLine( pLastPoint->GetPos() + vecShiftUp, pRoute->GetPos() + vecShiftUp, nCollisionMask, &traceFilter, &tr );

		if ( asw_blink_debug.GetBool() )
		{
			DebugDrawLine( pLastPoint->GetPos() + vecShiftUp, pRoute->GetPos() + vecShiftUp, 255, 0, 0, true, 30.0f );
		}

		if ( tr.DidHit() && tr.m_pEnt )
		{
			Class_T iClass = tr.m_pEnt->Classify();

			if ( CLASS_ASW_ALIEN_GOO	== iClass ||			// asw_alien_goo biomass
				 CLASS_FUNC_MOVELINEAR	== iClass ||			// func_movelinear doors
				 dynamic_cast< CFuncBrush* >( tr.m_pEnt ) ||	// func_brush airlocks
				 dynamic_cast< CBaseDoor*  >( tr.m_pEnt )		// func_door doors
				 )
			{
				return true;
			}
		}

		pLastPoint = pRoute;
		pRoute = pRoute->GetNext();
	}

	return false;
}

CDynamicProp* UTIL_ASW_AirlockBlockingRoute( AI_Waypoint_t* pRoute, const int nCollisionMask, const int nCollisionGroup )
{
	if ( !pRoute )
		return NULL;

	AI_Waypoint_t* pLastPoint = pRoute;
	pRoute = pRoute->GetNext();
	trace_t tr;
	Vector vecShiftUp = Vector( 0, 0, 20 );

	while ( pRoute )
	{
		CTraceFilterSimple traceFilter( NULL, nCollisionGroup );
		UTIL_TraceLine( pLastPoint->GetPos() + vecShiftUp, pRoute->GetPos() + vecShiftUp, nCollisionMask, &traceFilter, &tr );

		if ( asw_blink_debug.GetBool() )
		{
			DebugDrawLine( pLastPoint->GetPos() + vecShiftUp, pRoute->GetPos() + vecShiftUp, 0, 0, 255, true, 30.0f );
		}

		if ( tr.DidHit() )
		{
			CDynamicProp* pAirlock = dynamic_cast< CDynamicProp* >( tr.m_pEnt );
			if ( pAirlock && FStrEq( STRING( tr.m_pEnt->GetModelName() ), "models/props/doors/slow_heavy_door/slow_heavy_door.mdl" ) )
			{
				return pAirlock;
			}
			if ( pAirlock && FStrEq( STRING( tr.m_pEnt->GetModelName() ), "models/props/doors/slow_heavy_door/slow_heavy_door_v2.mdl" ) )
			{
				return pAirlock;
			}
		}

		pLastPoint = pRoute;
		pRoute = pRoute->GetNext();
	}

	return NULL;
}

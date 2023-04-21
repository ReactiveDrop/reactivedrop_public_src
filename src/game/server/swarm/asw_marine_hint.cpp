//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "asw_marine_hint.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// ========= Temporary hint entity ============

LINK_ENTITY_TO_CLASS( info_marine_hint, CASW_Marine_Hint_Ent );

BEGIN_DATADESC( CASW_Marine_Hint_Ent )

END_DATADESC();

void CASW_Marine_Hint_Ent::Spawn()
{
	Assert( MarineHintManager() );
	MarineHintManager()->AddHint( this );

	BaseClass::Spawn();

	UTIL_RemoveImmediate( this );
}

// ========= Movable hint entity ============

LINK_ENTITY_TO_CLASS( info_marine_hint_dynamic, CASW_Marine_Hint_Dynamic );

BEGIN_DATADESC( CASW_Marine_Hint_Dynamic )

END_DATADESC();

void CASW_Marine_Hint_Dynamic::Spawn()
{
	Assert( MarineHintManager() );
	m_pHintData = MarineHintManager()->AddHint( this );

	Assert( m_pHintData );
	if ( m_pHintData )
		m_pHintData->m_Flags |= HintData_t::HINT_DYNAMIC;

	BaseClass::Spawn();
}

void CASW_Marine_Hint_Dynamic::PhysicsSimulate()
{
	BaseClass::PhysicsSimulate();

	Assert( m_pHintData );
	if ( !m_pHintData )
		return;

	m_pHintData->m_vecPosition = GetAbsOrigin();
	m_pHintData->m_flYaw = GetAbsAngles()[YAW];
}

void CASW_Marine_Hint_Dynamic::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();

	// we can't change the indices at runtime, so we just mark the hint data as unusable
	if ( m_pHintData )
		m_pHintData->m_Flags |= HintData_t::HINT_DELETED;
}

// ========= Temporary hint entity that's also an info_node ============

LINK_ENTITY_TO_CLASS( info_node_marine_hint, CASW_Marine_Hint_Node_Ent );

BEGIN_DATADESC( CASW_Marine_Hint_Node_Ent )

END_DATADESC();

// info_nodes are automatically removed when the map loads, store position/direction in our hint list
void CASW_Marine_Hint_Node_Ent::UpdateOnRemove()
{
	Assert( MarineHintManager() );
	HintData_t *pHintData = MarineHintManager()->AddHint( this );

	Assert( pHintData );
	if ( pHintData )
		pHintData->m_Flags |= HintData_t::HINT_NODE;

	BaseClass::UpdateOnRemove();
}

// =============== Hint Manager ===============

CASW_Marine_Hint_Manager g_Marine_Hint_Manager;
CASW_Marine_Hint_Manager *MarineHintManager() { return &g_Marine_Hint_Manager; }

CASW_Marine_Hint_Manager::CASW_Marine_Hint_Manager()
{
}

CASW_Marine_Hint_Manager::~CASW_Marine_Hint_Manager()
{
}

void CASW_Marine_Hint_Manager::LevelInitPreEntity()
{
	Reset();
}

void CASW_Marine_Hint_Manager::LevelShutdownPostEntity()
{
	Reset();
}

void CASW_Marine_Hint_Manager::Reset()
{
	m_Hints.PurgeAndDeleteElements();
}

int CASW_Marine_Hint_Manager::FindHints( const Vector &position, const float flMinDistance, const float flMaxDistance, CUtlVector<HintData_t *> *pResult )
{
	const float flMinDistSqr = flMinDistance * flMinDistance;
	const float flMaxDistSqr = flMaxDistance * flMaxDistance;
	int nCount = m_Hints.Count();
	for ( int i = 0; i < nCount; i++ )
	{
		if ( m_Hints[i]->m_Flags & HintData_t::HINT_DELETED )
			continue;

		if ( m_Hints[i]->m_flIgnoreUntil > gpGlobals->curtime )
			continue;

		float flDistSqr = position.DistToSqr( m_Hints[i]->m_vecPosition );
		if ( flDistSqr < flMinDistSqr || flDistSqr > flMaxDistSqr )
			continue;

		pResult->AddToTail( m_Hints[i] );
	}

	return pResult->Count();
}

int CASW_Marine_Hint_Manager::FindHints( const CBaseTrigger &volume, CUtlVector<HintData_t *> *pResult )
{
	int nCount = m_Hints.Count();
	for ( int i = 0; i < nCount; i++ )
	{
		if ( m_Hints[i]->m_Flags & HintData_t::HINT_DELETED )
			continue;

		if ( m_Hints[i]->m_flIgnoreUntil > gpGlobals->curtime )
			continue;

		if ( volume.CollisionProp()->IsPointInBounds( m_Hints[i]->GetAbsOrigin() ) )
			pResult->AddToTail( m_Hints[i] );
	}
	return pResult->Count();
}

HintData_t *CASW_Marine_Hint_Manager::AddHint( CBaseEntity *pEnt )
{
	HintData_t *pHintData = new HintData_t;
	pHintData->m_Flags = 0;
	pHintData->m_vecPosition = pEnt->GetAbsOrigin();
	pHintData->m_flYaw = pEnt->GetAbsAngles()[YAW];
	pHintData->m_nHintIndex = m_Hints.AddToTail( pHintData );
	pHintData->m_flIgnoreUntil = -1;
	pHintData->m_flPathValidUntil = -1;
	return pHintData;
}

HintData_t *CASW_Marine_Hint_Manager::AddInfoNode( CAI_Node *pNode )
{
	if ( !pNode )
		return NULL;

	HintData_t *pHintData = new HintData_t;
	pHintData->m_Flags = HintData_t::HINT_NODE;
	pHintData->m_vecPosition = pNode->GetPosition( HULL_HUMAN );
	pHintData->m_flYaw = pNode->GetYaw();
	pHintData->m_nHintIndex = m_Hints.AddToTail( pHintData );
	pHintData->m_flIgnoreUntil = -1;
	pHintData->m_flPathValidUntil = -1;
	return pHintData;
}

CON_COMMAND( asw_show_marine_hints, "Show hint manager spots" )
{
	int nCount = MarineHintManager()->GetHintCount();
	for ( int i = 0; i < nCount; i++ )
	{
		int iFlags = MarineHintManager()->GetHintFlags( i );
		if ( iFlags & HintData_t::HINT_DELETED )
			continue;

		Vector vecPos = MarineHintManager()->GetHintPosition( i );
		float flYaw = MarineHintManager()->GetHintYaw( i );

		NDebugOverlay::YawArrow( vecPos, flYaw, 64, 16, 255, ( iFlags & HintData_t::HINT_DYNAMIC ) ? 127 : 255, ( iFlags & HintData_t::HINT_NODE ) ? 127 : 255, 0, true, 3.0f );
	}
}

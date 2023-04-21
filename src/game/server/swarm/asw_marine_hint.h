//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef _INCLUDED_ASW_MARINE_HINT_H
#define _INCLUDED_ASW_MARINE_HINT_H
#ifdef _WIN32
#pragma once
#endif

#include "igamesystem.h"
#include "ai_initutils.h"
#include "triggers.h"

class HintData_t
{
public:
	const Vector &GetAbsOrigin() { return m_vecPosition; }

	enum Flag_t
	{
		HINT_DELETED = 0x00000001, // hint is unusable
		HINT_NODE    = 0x00000002, // hint was created by the nodegraph
		HINT_DYNAMIC = 0x00000004, // hint is (or was) tied to an active entity
	};
	int m_Flags;
	Vector m_vecPosition;
	float m_flYaw;
	int m_nHintIndex;
	float m_flIgnoreUntil;
	float m_flPathValidUntil;
};

class CASW_Marine_Hint_Ent : public CServerOnlyPointEntity
{
	DECLARE_CLASS( CASW_Marine_Hint_Ent, CServerOnlyPointEntity );
public:
	DECLARE_DATADESC();

	virtual void Spawn();
};

class CASW_Marine_Hint_Dynamic : public CServerOnlyPointEntity
{
	DECLARE_CLASS( CASW_Marine_Hint_Dynamic, CServerOnlyPointEntity );
public:
	DECLARE_DATADESC();

	virtual void Spawn();
	virtual void PhysicsSimulate();
	virtual void UpdateOnRemove();

	HintData_t *m_pHintData{};
};

class CASW_Marine_Hint_Node_Ent : public CNodeEnt
{
	DECLARE_CLASS( CASW_Marine_Hint_Node_Ent, CNodeEnt );
public:
	DECLARE_DATADESC();

	virtual void UpdateOnRemove();
};

//-----------------------------------------------------------------------------
// Purpose: Stores positions for the marines to use while following
//-----------------------------------------------------------------------------
class CASW_Marine_Hint_Manager : protected CAutoGameSystem
{
	typedef CAutoGameSystem BaseClass;
public:
	CASW_Marine_Hint_Manager();
	~CASW_Marine_Hint_Manager();

	void Reset();
	virtual void LevelInitPreEntity();
	virtual void LevelShutdownPostEntity();

	HintData_t *AddHint( CBaseEntity *pEnt );
	HintData_t *AddInfoNode( CAI_Node *pNode );
	int FindHints( const Vector &position, const float flMinDistance, const float flMaxDistance, CUtlVector<HintData_t *> *pResult );
	// finds hints inside a volume
	int FindHints( const CBaseTrigger &volume, CUtlVector<HintData_t *> *pResult );

	int GetHintCount() { return m_Hints.Count(); }
	int GetHintFlags( int nHint ) { return m_Hints[nHint]->m_Flags; }
	const Vector &GetHintPosition( int nHint ) { return m_Hints[nHint]->m_vecPosition; }
	float GetHintYaw( int nHint ) { return m_Hints[nHint]->m_flYaw; }

protected:
	CUtlVector<HintData_t *> m_Hints;
};
CASW_Marine_Hint_Manager *MarineHintManager();

#define INVALID_HINT_INDEX -1

#endif // _INCLUDED_ASW_MARINE_HINT_H

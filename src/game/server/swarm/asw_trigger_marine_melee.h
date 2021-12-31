#pragma once

#include "triggers.h"

class CASW_Melee_Attack;
class CASW_Marine;
class CMoveData;

class CASW_Trigger_Marine_Melee : public CBaseTrigger
{
	DECLARE_CLASS( CASW_Trigger_Marine_Melee, CBaseTrigger );
public:
	void Spawn( void );
	void StartTouch( CBaseEntity *pOther );
	void EndTouch( CBaseEntity *pOther );
	void OnMeleeAttack( CASW_Melee_Attack *pAttack, CASW_Marine *pMarine, CMoveData *pMoveData );

	DECLARE_DATADESC();

	float m_fAngleTolerance;

	// Outputs
	COutputEvent m_OnTrigger;
};

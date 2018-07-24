#ifndef _INCLUDED_ASW_TRIGGERS_H
#define _INCLUDED_ASW_TRIGGERS_H

#include <vector>
#include "triggers.h"

class CASW_Marine;
class CASW_Weapon;

// a trigger just like the trigger_multiple, but has a mapper set (0 to 1) chance of actually firing
class CASW_Chance_Trigger_Multiple : public CBaseTrigger
{
	DECLARE_CLASS( CASW_Chance_Trigger_Multiple, CBaseTrigger );
public:	
	void ChanceMultiTouch(CBaseEntity *pOther);
	void ActivateChanceMultiTrigger(CBaseEntity *pActivator);
	void Spawn( void );	
	void MultiWaitOver( void );

	DECLARE_DATADESC();

	float m_fTriggerChance;

	// Outputs
	COutputEvent m_OnTrigger;
};

// a trigger with multiple outputs, a randomly chosen one is fired
//  has up to 6 random targets
class CASW_Random_Target_Trigger_Multiple : public CBaseTrigger
{
	DECLARE_CLASS( CASW_Random_Target_Trigger_Multiple, CBaseTrigger );
public:	
	void RandomMultiTouch(CBaseEntity *pOther);
	void ActivateRandomMultiTrigger(CBaseEntity *pActivator);
	void Spawn( void );	
	void MultiWaitOver( void );

	DECLARE_DATADESC();

	int m_iNumOutputs;

	// Outputs
	COutputEvent m_OnTrigger1, m_OnTrigger2, m_OnTrigger3, m_OnTrigger4, m_OnTrigger5, m_OnTrigger6;
};

// same as a trigger multiple, but will fire the 'supplies!' speech when triggered
class CASW_Supplies_Chatter_Trigger : public CBaseTrigger
{
	DECLARE_CLASS( CASW_Supplies_Chatter_Trigger, CBaseTrigger );
public:	
	void SuppliesChatterTouch(CBaseEntity *pOther);
	void ActivateSuppliesChatterTrigger(CBaseEntity *pActivator);
	void Spawn( void );	
	void MultiWaitOver( void );

	DECLARE_DATADESC();

	int m_iNumOutputs;
	bool m_bNoAmmo;

	// Outputs
	COutputEvent m_OnTrigger;
};

// same as a trigger multiple, but will fire the 'supplies!' speech when triggered
class CASW_SynUp_Chatter_Trigger : public CBaseTrigger
{
	DECLARE_CLASS( CASW_SynUp_Chatter_Trigger, CBaseTrigger );
public:	
	void ChatterTouch(CBaseEntity *pOther);
	void ActivateChatterTrigger(CBaseEntity *pActivator);
	void Spawn( void );	
	void MultiWaitOver( void );

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_OnTrigger;
};

// same as a trigger multiple, but will fire the 'supplies!' speech when triggered
class CASW_Marine_Position_Trigger : public CBaseTrigger
{
	DECLARE_CLASS( CASW_Marine_Position_Trigger, CBaseTrigger );
public:		
	void PositionTouch(CBaseEntity *pOther);
	void ActivatePositionTrigger(CBaseEntity *pActivator);
	void Spawn( void );	
	void MultiWaitOver( void );

	DECLARE_DATADESC();
	
	float m_fDesiredFacing;
	float m_fTolerance;

	// Outputs
	COutputEvent m_OnTrigger;
	COutputEvent m_OnMarineInPosition;
	COutputEvent m_OnMarineOutOfPosition;

	EHANDLE m_hMarine;
};


// entity that fires outputs when a marine is healed

class CASW_Info_Heal : public CLogicalEntity
{
public:
	DECLARE_CLASS( CASW_Info_Heal, CLogicalEntity );
	DECLARE_DATADESC();

	virtual void Spawn();
	void OnMarineHealed( CASW_Marine *pHealer, CASW_Marine *pHealed, CASW_Weapon *pHealEquip );

	COutputEvent m_MarineHealed;
};


class CASW_Hurt_Nearest_Marine : public CServerOnlyPointEntity
{
public:
	DECLARE_CLASS( CASW_Hurt_Nearest_Marine, CServerOnlyPointEntity );
	DECLARE_DATADESC();

	virtual void InputHurtMarine( inputdata_t &inputdata );
};


DECLARE_AUTO_LIST( IASW_Marines_Past_Area_List );

class CASW_Marines_Past_Area : public CServerOnlyPointEntity, public IASW_Marines_Past_Area_List
{
public:
	DECLARE_CLASS( CASW_Marines_Past_Area, CServerOnlyPointEntity );
	DECLARE_DATADESC();

	CASW_Marines_Past_Area();

	void OnMarineKilled( CASW_Marine *pKilledMarine );

	virtual void InputMarineInFront( inputdata_t &inputdata );
	virtual void InputMarineBehind( inputdata_t &inputdata );

	IMPLEMENT_AUTO_LIST_GET();

	COutputEvent m_OutputMarinesPast;

	CUtlVector<EHANDLE> m_MarinesPast;

	int m_iNumMarines;
};


class CASW_Marine_Knockback_Trigger : public CBaseTrigger
{
	DECLARE_CLASS( CASW_Marine_Knockback_Trigger, CBaseTrigger );
public:	
	void KnockbackTriggerTouch(CBaseEntity *pOther);
	void Spawn( void );	

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_OnKnockedBack;

	Vector m_vecKnockbackDir;
};

class CASW_Marine_JumpJet_Trigger : public CTriggerMultiple
{
	DECLARE_CLASS( CASW_Marine_JumpJet_Trigger, CTriggerMultiple );
public:	
	//void JumpJetTriggerTouch(CBaseEntity *pOther);
	void Spawn( void );	

	virtual bool PassesTriggerFilters( CBaseEntity *pOther );
	virtual void ActivateMultiTrigger( CBaseEntity *pOther );

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_OnJumpJetDone;

	string_t m_sDestination1;	// the entity name where marine will jump to 
	string_t m_sDestination2;	// for the next jump next entity will be used
	string_t m_sDestination3;	// if it is valid 
	string_t m_sDestination4;
	string_t m_sDestination5;
	string_t m_sDestination6;
	string_t m_sDestination7;
	string_t m_sDestination8;

	int  m_iJumpType;		// 0 = jump jet, 1 = blink 
	bool m_bRequireOffhand; // if true, marine must have a jump jet or blink 
							// offhand to perform jump

	float m_fJumpTimeOverride;		// overrides the time taken to perform a 
									// jump jet 
	float m_fAnimationTimeOverride;	// overrides the time taken to perform 
									// jump jet animation 

	CUtlVector<string_t> m_Destinations;
	int m_iCurEntToJumpTo;			// the index in m_Nodes, used to iterate
									// 
};

DECLARE_AUTO_LIST(IASW_StickTogether_Area_List);

// A trigger that forces bots(AI marines) to get inside this trigger 
// when a leader is inside it 
class CASW_StickTogether_Area : public CTriggerMultiple, public IASW_StickTogether_Area_List
{
	DECLARE_CLASS(CASW_StickTogether_Area, CTriggerMultiple);
public:
	CASW_StickTogether_Area();
	void Spawn(void);

	//DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	IMPLEMENT_AUTO_LIST_GET();
};

//-----------------------------------------------------------------------------
// Purpose: Used at the bottom of maps where objects should fall away to infinity
//-----------------------------------------------------------------------------
class CASW_Trigger_Fall : public CBaseTrigger
{
	DECLARE_CLASS( CASW_Trigger_Fall, CBaseTrigger );
public:
	void Spawn( void );
	void FallTouch( CBaseEntity *pOther );

	DECLARE_DATADESC();

	// Outputs
	COutputEvent m_OnFallingObject;
};

#endif // _INCLUDED_ASW_TRIGGERS_H

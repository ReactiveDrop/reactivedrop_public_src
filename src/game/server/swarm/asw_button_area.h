#ifndef _DEFINED_ASW_BUTTON_AREA_H
#define _DEFINED_ASW_BUTTON_AREA_H

#include "asw_use_area.h"
#include "asw_shareddefs.h"

class CASW_Player;
class CASW_Door;
class CASW_Hack;
class CASW_Marine;

class CASW_Button_Area : public CASW_Use_Area
{
	DECLARE_CLASS( CASW_Button_Area, CASW_Use_Area );
public:
	CASW_Button_Area();
	virtual ~CASW_Button_Area();
	virtual void Spawn( void );
	virtual void Precache();
	void ActivateUnlockedButton( CASW_Inhabitable_NPC *pNPC );
	CASW_Door *GetDoor();
	CASW_Hack *GetCurrentHack();
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
	virtual bool IsLocked() { return m_bIsLocked.Get(); }
	virtual bool HasPower() { return !m_bNoPower.Get(); }
	float m_fStartedHackTime;

	Class_T		Classify( void ) { return (Class_T) CLASS_ASW_BUTTON_PANEL; }

	static bool WaitingForInputVismonEvaluator( CBaseEntity *pVisibleEntity, CBasePlayer *pViewingPlayer );
	static bool WaitingForInputVismonCallback( CBaseEntity *pVisibleEntity, CBasePlayer *pViewingPlayer );
	
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CNetworkVar( int, m_iHackLevel );	
	CNetworkVar( bool, m_bIsLocked );
	CNetworkVar( bool, m_bIsDoorButton );
	CNetworkVar( bool, m_bNoPower );
	CNetworkVar( bool, m_bWaitingForInput );
	CNetworkVar( bool, m_bNeedsTech );
	CNetworkString( m_NoPowerMessage, 255 );

	// settings for the wire puzzle
	int m_iWireColumns;
	int m_iWireRows;
	int m_iNumWires;
	
	EHANDLE m_hDoorHack;

	COutputEvent m_OnFastHackFailed;
	COutputEvent m_OnButtonHackStarted;
	COutputEvent m_OnButtonHackAt25Percent;
	COutputEvent m_OnButtonHackAt50Percent;
	COutputEvent m_OnButtonHackAt75Percent;
	COutputEvent m_OnButtonHackCompleted;
	COutputEvent m_OnButtonActivated;

	void InputPowerOff( inputdata_t &inputdata );
	void InputPowerOn( inputdata_t &inputdata );
	void InputResetHack( inputdata_t &inputdata );
	void InputUnlock( inputdata_t &inputdata );
	
	virtual void ActivateUseIcon( CASW_Inhabitable_NPC *pMarine, int nHoldType );
	virtual void NPCUsing( CASW_Inhabitable_NPC *pNPC, float deltatime );
	virtual void NPCStartedUsing( CASW_Inhabitable_NPC *pNPC );
	virtual void NPCStoppedUsing( CASW_Inhabitable_NPC *pNPC );
	virtual bool IsActive( void );
	virtual bool IsWaitingForInput( void ) const { return m_bWaitingForInput; }

	virtual void UpdateWaitingForInput();
	virtual void UpdatePanelSkin();

	virtual void SetHackProgress(float f, CASW_Marine *pMarine);
	float GetHackProgress() { return m_fHackProgress; }

	CNetworkVar(bool, m_bIsInUse);
	CNetworkVar(float, m_fHackProgress);
	CNetworkVar(float, m_flHoldTime);
	bool m_bChangePanelSkin;
	bool m_bWasLocked;
	bool m_bUseAfterHack;
	bool m_bDisableAfterUse;
	int m_iAliensKilledBeforeHack;

	float m_fLastButtonUseTime;
	bool m_bDestroyHeldObject;
};

#endif /* _DEFINED_ASW_BUTTON_AREA_H */
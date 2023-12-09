#pragma once

#include "rd_hud_vscript_shared.h"

#ifdef CLIENT_DLL
#define CRD_VGui_VScript C_RD_VGui_VScript
#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
class CRD_VGui_VScript_Button;
class CRD_VGui_VScript_Button_Panel;

#include <vgui_controls/PHandle.h>
#endif
class CASW_Inhabitable_NPC;

class CRD_VGui_VScript : public CRD_HUD_VScript
{
	DECLARE_CLASS( CRD_VGui_VScript, CRD_HUD_VScript );
public:
	DECLARE_PREDICTABLE();
	DECLARE_NETWORKCLASS();
	DECLARE_ENT_SCRIPTDESC();

	CRD_VGui_VScript();
	~CRD_VGui_VScript();

#ifdef CLIENT_DLL
	static CUtlVector<CRD_VGui_VScript *> s_InteractiveHUDEntities;

	void SendInput( int value );
	void SetEntity( int i, HSCRIPT entity );
	void SetInt( int i, int value );
	void SetFloat( int i, float value );
	void SetString( int i, const char *string );

	ScriptVariant_t m_hControlTable;
	HSCRIPT m_hControlFunc{ INVALID_HSCRIPT };

	void OnDataChanged( DataUpdateType_t type ) override;
	bool ShouldPredict() override;
	C_BasePlayer *GetPredictionOwner() override;
	void PhysicsSimulate() override;
	virtual bool AllowedToInteract();
	virtual bool InterceptButtonPress( ButtonCode_t iButton );
	virtual void RunControlFunction( ButtonCode_t iButton = BUTTON_CODE_NONE );
	virtual void UpdateControlTable( ButtonCode_t iButton );

	struct QueuedInput_t
	{
		int sequence;
		int value;
	};
	CUtlVector<QueuedInput_t> m_QueuedInputsForPrediction;
	int m_iNextSequence;

	CUtlVectorAutoPurge<CRD_VGui_VScript_Button *> m_ButtonDefs;
	CUtlVector<vgui::DHANDLE<CRD_VGui_VScript_Button_Panel>> m_ButtonPanels;
	vgui::PHandle m_hButtonPanelParent;
	int m_iControllerFocusIndex{ -1 };
	void InitButtonPanels( vgui::Panel *pParent );
	void ClearButtonPanels();
	void CursorThink( int x, int y, bool bOverParent );
	HSCRIPT CreateButton();

	bool m_bIsControlling{ false };
	bool m_bIsPredicting{ false };
#else
	DECLARE_DATADESC();

	virtual void OnInput( int sequence, int value );
	void SetInteracter( HSCRIPT interacter );
	virtual bool AllowSetInteracter() { return true; }

	void RunVScripts() override;
#endif

	HSCRIPT m_hInputFunc{ INVALID_HSCRIPT };
	HSCRIPT GetInteracter();

	CNetworkVar( int, m_iRandomCheck );
	CNetworkHandle( CASW_Inhabitable_NPC, m_hInteracter );
	CNetworkVar( int, m_iSequenceAck );

	bool m_bIsInput{ false };

	const char *GetDebugClassname() const override { return "rd_vgui_vscript"; }
};

#ifdef CLIENT_DLL
class CRD_VGui_VScript_Button
{
public:
	CRD_VGui_VScript_Button( CRD_VGui_VScript *pOwner );
	~CRD_VGui_VScript_Button();

	// script functions
	int GetX() const { return m_x; }
	int GetY() const { return m_y; }
	int GetWide() const { return m_wide; }
	int GetTall() const { return m_tall; }
	void SetX( int x);
	void SetY(int y);
	void SetWide( int wide );
	void SetTall( int tall );
	void SetPos( int x, int y ) { SetX( x ); SetY( y ); }
	void SetSize( int wide, int tall ) { SetWide( wide ); SetTall( tall ); }
	void SetBounds( int x, int y, int wide, int tall ) { SetPos( x, y ); SetSize( wide, tall ); }
	void SetOnCursorMoved( HSCRIPT callback );
	void SetOnCursorEntered( HSCRIPT callback );
	void SetOnCursorExited( HSCRIPT callback );
	void SetOnMousePressed( HSCRIPT callback );

	// callbacks
	void OnCursorMoved( int x, int y );
	void OnCursorEntered();
	void OnCursorExited();
	void OnMousePressed( bool right );

	HSCRIPT m_hThis;
	CRD_VGui_VScript *m_pOwner;
	int m_x, m_y, m_wide, m_tall;
	vgui::DHANDLE<CRD_VGui_VScript_Button_Panel> m_hPanel;
	HSCRIPT m_hCursorMovedCallback{ INVALID_HSCRIPT };
	HSCRIPT m_hCursorEnteredCallback{ INVALID_HSCRIPT };
	HSCRIPT m_hCursorExitedCallback{ INVALID_HSCRIPT };
	HSCRIPT m_hMousePressedCallback{ INVALID_HSCRIPT };
};
#endif

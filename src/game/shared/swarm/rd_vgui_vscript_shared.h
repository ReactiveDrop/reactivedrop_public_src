#pragma once

#include "rd_hud_vscript_shared.h"

#ifdef CLIENT_DLL
#define CRD_VGui_VScript C_RD_VGui_VScript
#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
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
	virtual bool AllowedToInteract() { return ShouldPredict(); }
	virtual bool InterceptButtonPress( ButtonCode_t iButton );
	virtual void RunControlFunction( ButtonCode_t iButton = BUTTON_CODE_NONE );
	virtual void UpdateControlTable( ButtonCode_t iButton );

	CUtlVector<int> m_QueuedInputsForPrediction;
	int m_iControllerFocusIndex{ -1 };
	bool m_bIsControlling{ false };
	bool m_bIsPredicting{ false };
#else
	DECLARE_DATADESC();

	virtual void OnInput( int value );
	void SetInteracter( HSCRIPT interacter );
	virtual bool AllowSetInteracter() { return true; }

	void RunVScripts() override;

	bool m_bIsInput{ false };
#endif

	HSCRIPT m_hInputFunc{ INVALID_HSCRIPT };
	HSCRIPT GetInteracter();

	CNetworkVar( int, m_iRandomCheck );
	CNetworkHandle( CASW_Inhabitable_NPC, m_hInteracter );

	const char *GetDebugClassname() const override { return "rd_vgui_vscript"; }
};

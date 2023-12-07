#pragma once

#include "rd_vgui_vscript_shared.h"

#ifdef CLIENT_DLL
#define CRD_Computer_VScript C_RD_Computer_VScript
#endif

class CRD_Computer_VScript : public CRD_VGui_VScript
{
	DECLARE_CLASS( CRD_Computer_VScript, CRD_VGui_VScript );
public:
	DECLARE_NETWORKCLASS();
	DECLARE_ENT_SCRIPTDESC();

	CRD_Computer_VScript();
	~CRD_Computer_VScript();

#ifdef CLIENT_DLL
	void OnDataChanged( DataUpdateType_t type ) override;
#else
	DECLARE_DATADESC();

	bool KeyValue( const char *szKeyName, const char *szValue ) override;
	bool AllowSetInteracter() override { return false; }
	void RunVScripts() override;

	void HackCompleted();
#endif

	CNetworkString( m_szLabel, 255 );
	CNetworkString( m_szIconName, 255 );

	void OnOpened( CASW_Inhabitable_NPC *pInteracter );
	void OnClosed();
	HSCRIPT m_hOnOpenedFunc{ INVALID_HSCRIPT };
	HSCRIPT m_hOnClosedFunc{ INVALID_HSCRIPT };
	bool m_bInOnOpened{ false };
	bool m_bInOnClosed{ false };

	void Back();

	const char *GetDebugClassname() const override { return "rd_computer_vscript"; }
};

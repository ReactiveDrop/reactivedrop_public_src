#include "cbase.h"
#include "rd_computer_vscript_shared.h"
#include <vgui_controls/Controls.h>
#include <vgui/ISurface.h>
#ifdef CLIENT_DLL
#include "c_asw_inhabitable_npc.h"
#include "c_asw_hack_computer.h"
#include "c_asw_computer_area.h"
#include "asw_vgui_computer_frame.h"
#include "asw_vgui_computer_menu.h"
#include "asw_vgui_computer_custom.h"
#include "asw_input.h"
#include "clientmode.h"
#define CASW_Computer_Area C_ASW_Computer_Area
#else
#include "asw_marine.h"
#include "asw_hack_computer.h"
#include "asw_computer_area.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CRD_Computer_VScript )
	DEFINE_PRED_FIELD( m_flHackProgress, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA();
#else
LINK_ENTITY_TO_CLASS( rd_computer_vscript, CRD_Computer_VScript );

BEGIN_DATADESC( CRD_Computer_VScript )
END_DATADESC();
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( RD_Computer_VScript, DT_RD_Computer_VScript );

BEGIN_NETWORK_TABLE( CRD_Computer_VScript, DT_RD_Computer_VScript )
#ifdef CLIENT_DLL
	RecvPropString( RECVINFO( m_szLabel ) ),
	RecvPropString( RECVINFO( m_szIconName ) ),
	RecvPropFloat( RECVINFO( m_flHackProgress ) ),
	RecvPropEHandle( RECVINFO( m_hHack ) ),
#else
	SendPropString( SENDINFO( m_szLabel ) ),
	SendPropString( SENDINFO( m_szIconName ) ),
	SendPropFloat( SENDINFO( m_flHackProgress ), 10, 0, 0.0f, 1.0f ),
	SendPropEHandle( SENDINFO( m_hHack ) ),
#endif
END_NETWORK_TABLE();

BEGIN_ENT_SCRIPTDESC( CRD_Computer_VScript, CRD_VGui_VScript, "Alien Swarm: Reactive Drop scriptable computer" )
#ifdef CLIENT_DLL
#else
	DEFINE_SCRIPTFUNC( HackCompleted, "Call when the player has completed the hack." )
	DEFINE_SCRIPTFUNC( SetFastHackEndTime, "Set the time limit (absolute) for the fast hack. The timer is initially set using the logic for the default computer hack." )
#endif
	DEFINE_SCRIPTFUNC( Back, "Call to close the computer screen and return to the menu." )
	DEFINE_SCRIPTFUNC( SetHackProgress, "Set the hack progress between 0 (just started) and 1 (ready to call HackCompleted). Can only be called from Input." )
	DEFINE_SCRIPTFUNC( GetHackStartTime, "Get the value of Time() when the hack started." )
	DEFINE_SCRIPTFUNC( GetFastHackEndTime, "Get the time limit (absolute) for the fast hack. Call Time() for the current time." )
END_SCRIPTDESC();

CRD_Computer_VScript::CRD_Computer_VScript()
{
	m_szLabel.GetForModify()[0] = '\0';
	m_szIconName.GetForModify()[0] = '\0';
	m_flHackProgress = 0.0f;

#ifdef CLIENT_DLL
	// remove us from the "always draw" list that our parent parent constructor added us to
	s_HUDEntities.FindAndRemove( this );
	// we don't need to be in the global interactive list either
	s_InteractiveHUDEntities.FindAndRemove( this );
#endif
}

CRD_Computer_VScript::~CRD_Computer_VScript()
{
	if ( m_ScriptScope.IsInitialized() )
	{
		m_ScriptScope.ReleaseFunction( m_hOnOpenedFunc );
		m_ScriptScope.ReleaseFunction( m_hOnClosedFunc );
	}
}

#ifdef CLIENT_DLL
void CRD_Computer_VScript::OnDataChanged( DataUpdateType_t type )
{
	bool bWantScriptInit = !m_ScriptScope.IsInitialized() && m_szClientVScript.Get()[0] != '\0';

	BaseClass::OnDataChanged( type );

	if ( bWantScriptInit && m_ScriptScope.IsInitialized() )
	{
		m_hOnOpenedFunc = m_ScriptScope.IsInitialized() ? m_ScriptScope.LookupFunction( "OnOpened" ) : INVALID_HSCRIPT;
		m_hOnClosedFunc = m_ScriptScope.IsInitialized() ? m_ScriptScope.LookupFunction( "OnClosed" ) : INVALID_HSCRIPT;
	}
}

void CRD_Computer_VScript::UpdateControlTable( ButtonCode_t iButton )
{
	BaseClass::UpdateControlTable( iButton );

	CASWInput *pInput = ASWInput();
	Assert( pInput );
	if ( !pInput )
		return;

	// move mouse position so it's relative to the inner computer screen
	int x, y;
	pInput->GetSimulatedFullscreenMousePos( &x, &y );
	CASW_VGUI_Computer_Frame *pComputerFrame = assert_cast< CASW_VGUI_Computer_Frame * >( GetClientMode()->GetPanelFromViewport( "ComputerContainer/VGUIComputerFrame" ) );
	if ( pComputerFrame && pComputerFrame->m_pMenuPanel )
	{
		pComputerFrame->m_pMenuPanel->ScreenToLocal( x, y );
	}
	g_pScriptVM->SetValue( m_hControlTable, "mouse_x", x );
	g_pScriptVM->SetValue( m_hControlTable, "mouse_y", y );
}
#else
bool CRD_Computer_VScript::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( FStrEq( szKeyName, "Label" ) )
	{
		V_strncpy( m_szLabel.GetForModify(), szValue, sizeof( m_szLabel ) );
		return true;
	}
	if ( FStrEq( szKeyName, "IconName" ) )
	{
		V_strncpy( m_szIconName.GetForModify(), szValue, sizeof( m_szIconName ) );
		return true;
	}
	return BaseClass::KeyValue( szKeyName, szValue );
}

void CRD_Computer_VScript::RunVScripts()
{
	BaseClass::RunVScripts();

	m_hOnOpenedFunc = m_ScriptScope.IsInitialized() ? m_ScriptScope.LookupFunction( "OnOpened" ) : INVALID_HSCRIPT;
	m_hOnClosedFunc = m_ScriptScope.IsInitialized() ? m_ScriptScope.LookupFunction( "OnClosed" ) : INVALID_HSCRIPT;
}

void CRD_Computer_VScript::HackCompleted()
{
	Assert( m_hHack );
	if ( !m_hHack )
		return;

	CASW_Computer_Area *pArea = m_hHack->GetComputerArea();
	Assert( pArea );
	if ( !pArea )
		return;

	Assert( pArea->m_hCustomHack.Get() == this );
	if ( pArea->m_hCustomHack.Get() != this )
		return;

	m_hHack->OnHackUnlocked( CASW_Marine::AsMarine( m_hInteracter ) );
}

void CRD_Computer_VScript::SetFastHackEndTime( float flTime )
{
	Assert( m_hHack );
	if ( !m_hHack )
		return;

	CASW_Computer_Area *pArea = m_hHack->GetComputerArea();
	Assert( pArea );
	if ( !pArea )
		return;

	Assert( pArea->m_hCustomHack.Get() == this );
	if ( pArea->m_hCustomHack.Get() != this )
		return;

	Assert( m_hHack->m_fStartedHackTime != 0 );
	if ( m_hHack->m_fStartedHackTime == 0 )
	{
		Warning( "%s (%s): Cannot call SetFastHackEndTime when hack has not yet launched.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	if ( m_hHack->m_fStartedHackTime < flTime )
	{
		Warning( "%s (%s): SetFastHackEndTime would set hack duration to %f seconds. Clamping to 0.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ), flTime - m_hHack->m_fStartedHackTime );
		flTime = m_hHack->m_fStartedHackTime;
	}

	m_hHack->m_fFastFinishTime = flTime;
}
#endif

void CRD_Computer_VScript::OnOpened( CASW_Inhabitable_NPC *pInteracter )
{
	Assert( pInteracter );
	Assert( !m_hInteracter || m_hInteracter.Get() == pInteracter );

	m_hInteracter.Set( pInteracter );

	Assert( !m_bInOnOpened );
	if ( m_hOnOpenedFunc && m_hOnOpenedFunc != INVALID_HSCRIPT )
	{
		m_bInOnOpened = true;
		g_pScriptVM->Call( m_hOnOpenedFunc, m_ScriptScope );
		m_bInOnOpened = false;
	}
}

void CRD_Computer_VScript::OnClosed()
{
	Assert( m_hInteracter );

	Assert( !m_bInOnClosed );
	if ( m_hOnClosedFunc && m_hOnClosedFunc != INVALID_HSCRIPT )
	{
		m_bInOnClosed = true;
		g_pScriptVM->Call( m_hOnClosedFunc, m_ScriptScope );
		m_bInOnClosed = false;
	}

	m_hInteracter.Set( NULL );
}

void CRD_Computer_VScript::Back()
{
	Assert( m_hHack.Get() );
	if ( !m_hHack.Get() )
		return;

	CASW_Computer_Area *pArea = m_hHack->GetComputerArea();
	Assert( pArea );
	if ( !pArea )
		return;

#ifdef CLIENT_DLL
	CASW_VGUI_Computer_Frame *pFrame = m_hHack->m_hComputerFrame;
	Assert( pFrame );
	if ( !pFrame )
		return;

	Assert( pFrame->m_pMenuPanel );
	if ( !pFrame->m_pMenuPanel )
		return;

	CASW_VGUI_Computer_Custom *pCustom = dynamic_cast< CASW_VGUI_Computer_Custom * >( pFrame->m_pMenuPanel->m_hCurrentPage.Get() );
	Assert( pCustom );
	if ( !pCustom )
		return;

	Assert( pCustom->m_hCustom.Get() == this );
	if ( pCustom->m_hCustom.Get() != this )
		return;

	if ( pArea->IsLocked() )
	{
		C_ASW_Player::GetLocalASWPlayer()->StopUsing();
	}
	else
	{
		pFrame->m_pMenuPanel->FadeCurrentPage();
	}
#else
	if ( pArea->m_bIsLocked )
	{
		Assert( pArea->m_hCustomHack.Get() == this );
		if ( pArea->m_hCustomHack.Get() != this )
			return;

		pArea->m_flStopUsingTime = gpGlobals->curtime;
		return;
	}

	switch ( m_hHack->GetOptionTypeForEntry( m_hHack->m_iShowOption ) )
	{
	case ASW_COMPUTER_OPTION_TYPE_CUSTOM_1:
		Assert( pArea->m_hCustomScreen1.Get() == this );
		if ( pArea->m_hCustomScreen1.Get() != this )
			return;
		break;
	case ASW_COMPUTER_OPTION_TYPE_CUSTOM_2:
		Assert( pArea->m_hCustomScreen2.Get() == this );
		if ( pArea->m_hCustomScreen2.Get() != this )
			return;
		break;
	case ASW_COMPUTER_OPTION_TYPE_CUSTOM_3:
		Assert( pArea->m_hCustomScreen3.Get() == this );
		if ( pArea->m_hCustomScreen3.Get() != this )
			return;
		break;
	case ASW_COMPUTER_OPTION_TYPE_CUSTOM_4:
		Assert( pArea->m_hCustomScreen4.Get() == this );
		if ( pArea->m_hCustomScreen4.Get() != this )
			return;
		break;
	case ASW_COMPUTER_OPTION_TYPE_CUSTOM_5:
		Assert( pArea->m_hCustomScreen5.Get() == this );
		if ( pArea->m_hCustomScreen5.Get() != this )
			return;
		break;
	case ASW_COMPUTER_OPTION_TYPE_CUSTOM_6:
		Assert( pArea->m_hCustomScreen6.Get() == this );
		if ( pArea->m_hCustomScreen6.Get() != this )
			return;
		break;
	default:
		return;
	}

	m_hHack->SelectHackOption( 0 );
#endif
}

void CRD_Computer_VScript::SetHackProgress( float flProgress )
{
	Assert( m_bIsInput );
	if ( !m_bIsInput )
	{
		Warning( "%s (%s): Cannot call SetHackProgress outside of Input function.\n", GetDebugClassname(), STRING( m_szClientVScript.Get() ) );
		return;
	}

	m_flHackProgress.Set( clamp( flProgress, 0.0f, 1.0f ) );
}

float CRD_Computer_VScript::GetHackStartTime()
{
	Assert( m_hHack );
	if ( !m_hHack )
		return 0;

	CASW_Computer_Area *pArea = m_hHack->GetComputerArea();
	Assert( pArea );
	if ( !pArea )
		return 0;

	Assert( pArea->m_hCustomHack.Get() == this );
	if ( pArea->m_hCustomHack.Get() != this )
		return 0;

	return m_hHack->m_fStartedHackTime;
}

float CRD_Computer_VScript::GetFastHackEndTime()
{
	Assert( m_hHack );
	if ( !m_hHack )
		return 0;

	CASW_Computer_Area *pArea = m_hHack->GetComputerArea();
	Assert( pArea );
	if ( !pArea )
		return 0;

	Assert( pArea->m_hCustomHack.Get() == this );
	if ( pArea->m_hCustomHack.Get() != this )
		return 0;

	return m_hHack->m_fFastFinishTime;
}

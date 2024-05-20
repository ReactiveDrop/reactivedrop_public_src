#include "cbase.h"
#include "asw_vgui_computer_custom.h"
#include "asw_vgui_computer_frame.h"
#include "clientmode.h"
#include "rd_computer_vscript_shared.h"
#include "c_asw_marine.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_VGUI_Computer_Custom::CASW_VGUI_Computer_Custom( vgui::Panel *pParent, const char *pElementName, C_ASW_Hack_Computer *pHackComputer, C_RD_Computer_VScript *pCustom ) :
	BaseClass( pParent, pElementName ),
	CASW_VGUI_Ingame_Panel(),
	m_hHackComputer( pHackComputer ),
	m_hCustom( pCustom )
{
	m_bInitialized = false;

	CASW_VGUI_Computer_Frame *pComputerFrame = dynamic_cast< CASW_VGUI_Computer_Frame * >( GetClientMode()->GetPanelFromViewport( "ComputerContainer/VGUIComputerFrame" ) );
	if ( pComputerFrame )
	{
		pComputerFrame->m_bHideLogoffButton = true;
	}
}

CASW_VGUI_Computer_Custom::~CASW_VGUI_Computer_Custom()
{
	CASW_VGUI_Computer_Frame *pComputerFrame = dynamic_cast< CASW_VGUI_Computer_Frame * >( GetClientMode()->GetPanelFromViewport( "ComputerContainer/VGUIComputerFrame" ) );
	if ( pComputerFrame )
	{
		pComputerFrame->m_bHideLogoffButton = false;
	}

	if ( m_bInitialized )
	{
		ASWClose();
	}
}

void CASW_VGUI_Computer_Custom::ASWInit()
{
	Assert( !m_bInitialized );
	m_bInitialized = true;

	if ( C_RD_Computer_VScript *pCustom = m_hCustom )
	{
		pCustom->OnOpened( C_ASW_Marine::GetViewMarine() );
		pCustom->InitButtonPanels( this );
	}
}

void CASW_VGUI_Computer_Custom::ASWClose()
{
	Assert( m_bInitialized );
	m_bInitialized = false;

	if ( C_RD_Computer_VScript *pCustom = m_hCustom )
	{
		pCustom->OnClosed();
		pCustom->ClearButtonPanels();
	}
}

void CASW_VGUI_Computer_Custom::PerformLayout()
{
	BaseClass::PerformLayout();

	int w, t;
	GetParent()->GetSize( w, t );
	SetBounds( 0, 0, w, t );
}

void CASW_VGUI_Computer_Custom::OnThink()
{
	BaseClass::OnThink();

	if ( C_RD_Computer_VScript *pCustom = m_hCustom )
	{
		pCustom->RunControlFunction();
	}
}

void CASW_VGUI_Computer_Custom::Paint()
{
	BaseClass::Paint();

	if ( C_RD_Computer_VScript *pCustom = m_hCustom )
	{
		pCustom->Paint();
	}
}

bool CASW_VGUI_Computer_Custom::MouseClick( int x, int y, bool bRightClick, bool bDown )
{
	if ( C_RD_Computer_VScript *pCustom = m_hCustom )
	{
		if ( bDown )
		{
			return pCustom->InterceptButtonPress( bRightClick ? MOUSE_RIGHT : MOUSE_LEFT );
		}
	}

	return CASW_VGUI_Ingame_Panel::MouseClick( x, y, bRightClick, bDown );
}

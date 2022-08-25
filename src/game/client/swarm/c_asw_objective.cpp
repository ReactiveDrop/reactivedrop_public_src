#include "cbase.h"
#include "c_asw_objective.h"
#include "hud_element_helper.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include "asw_hud_objective.h"
#include <vgui/ILocalize.h>
#include "c_asw_marker.h"
#include "asw_hud_minimap.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Objective, DT_ASW_Objective, CASW_Objective )
	RecvPropString( RECVINFO( m_ObjectiveTitle ) ),
	RecvPropString( RECVINFO( m_ObjectiveDescription1 ) ),
	RecvPropString( RECVINFO( m_ObjectiveDescription2 ) ),
	RecvPropString( RECVINFO( m_ObjectiveDescription3 ) ),
	RecvPropString( RECVINFO( m_ObjectiveDescription4 ) ),
	RecvPropString( RECVINFO( m_ObjectiveImage ) ),
	RecvPropString( RECVINFO( m_ObjectiveInfoIcon1 ) ),
	RecvPropString( RECVINFO( m_ObjectiveInfoIcon2 ) ),
	RecvPropString( RECVINFO( m_ObjectiveInfoIcon3 ) ),
	RecvPropString( RECVINFO( m_ObjectiveInfoIcon4 ) ),
	RecvPropString( RECVINFO( m_ObjectiveInfoIcon5 ) ),
	RecvPropString( RECVINFO( m_ObjectiveIcon ) ),
	RecvPropString( RECVINFO( m_LegacyMapMarkings ) ),
	RecvPropInt( RECVINFO( m_Priority ) ),
	RecvPropBool( RECVINFO( m_bComplete ) ),
	RecvPropBool( RECVINFO( m_bFailed ) ),
	RecvPropBool( RECVINFO( m_bOptional ) ),
	RecvPropBool( RECVINFO( m_bDummy ) ),
	RecvPropBool( RECVINFO( m_bVisible ) ),
END_RECV_TABLE()

C_ASW_Objective::C_ASW_Objective()
{
	m_ObjectiveTitle[0] = '\0';
	m_ObjectiveDescription1[0] = '\0';
	m_ObjectiveDescription2[0] = '\0';
	m_ObjectiveDescription3[0] = '\0';
	m_ObjectiveDescription4[0] = '\0';
	m_ObjectiveImage[0] = '\0';
	m_ObjectiveInfoIcon1[0] = '\0';
	m_ObjectiveInfoIcon2[0] = '\0';
	m_ObjectiveInfoIcon3[0] = '\0';
	m_ObjectiveInfoIcon4[0] = '\0';
	m_ObjectiveInfoIcon5[0] = '\0';
	m_ObjectiveIcon[0] = '\0';
	m_LegacyMapMarkings[0] = '\0';

	m_Priority = 0;
	m_bComplete = false;
	m_bFailed = false;
	m_bOptional = false;
	m_bDummy = false;
	m_bVisible = true;

	m_ObjectiveIconTextureID = -1;
}

C_ASW_Objective::~C_ASW_Objective()
{
}

const wchar_t *C_ASW_Objective::GetObjectiveTitle()
{
	static wchar_t wbuffer[256];
	if ( m_ObjectiveTitle[0] == '#' )
	{
		wchar_t *pTitle = g_pVGuiLocalize->Find( m_ObjectiveTitle );
		if ( pTitle )
			return pTitle;
	}

	g_pVGuiLocalize->ConvertANSIToUnicode( m_ObjectiveTitle, wbuffer, sizeof( wbuffer ) );
	return wbuffer;
}

const char *C_ASW_Objective::GetDescription( int i )
{
	switch ( i )
	{
	case 0:
		return m_ObjectiveDescription1;
	case 1:
		return m_ObjectiveDescription2;
	case 2:
		return m_ObjectiveDescription3;
	case 3:
		return m_ObjectiveDescription4;
	}
	return NULL;
}

const char *C_ASW_Objective::GetObjectiveIconName()
{
	return m_ObjectiveIcon;
}

int C_ASW_Objective::GetObjectiveIconTextureID()
{
	if ( m_ObjectiveIconTextureID == -1 )
	{
		if ( m_ObjectiveIcon[0] != '\0' )
		{
			m_ObjectiveIconTextureID = vgui::surface()->CreateNewTextureID();
			vgui::surface()->DrawSetTextureFile( m_ObjectiveIconTextureID, m_ObjectiveIcon, true, false );
		}
		else
		{
			return -1;
		}
	}
	return m_ObjectiveIconTextureID;
}

const char *C_ASW_Objective::GetInfoIcon( int i )
{
	switch ( i )
	{
	case 0:
		return m_ObjectiveInfoIcon1;
	case 1:
		return m_ObjectiveInfoIcon2;
	case 2:
		return m_ObjectiveInfoIcon3;
	case 3:
		return m_ObjectiveInfoIcon4;
	case 4:
		return m_ObjectiveInfoIcon5;
	}
	return NULL;
}

// allows this objective to paint its status on the HUD
void C_ASW_Objective::PaintObjective( float &current_y )
{
}

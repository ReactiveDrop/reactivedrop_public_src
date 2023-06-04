#include "cbase.h"
#include "rd_vgui_settings.h"
#include <vgui_controls/Label.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Controls );

CRD_VGUI_Bind::CRD_VGUI_Bind( vgui::Panel *parent, const char *panelName, const char *szLabel, const char *szBind, bool bUseRowLayout ) :
	BaseClass( parent, panelName )
{
	V_strncpy( m_szBind, szBind, sizeof( m_szBind ) );
	m_bUseRowLayout = bUseRowLayout;

	m_pLblDescription = new vgui::Label( this, "LblDescription", szLabel );
}

CRD_VGUI_Settings_Controls::CRD_VGUI_Settings_Controls( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
}

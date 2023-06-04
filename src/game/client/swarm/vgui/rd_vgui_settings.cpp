#include "cbase.h"
#include "rd_vgui_settings.h"
#include "gameui/swarm/vhybridbutton.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings );

CRD_VGUI_Settings::CRD_VGUI_Settings( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	m_pBtnControls = new BaseModUI::BaseModHybridButton( this, "BtnControls", "#rd_settings_controls", this, "Controls" );
	m_pBtnOptions = new BaseModUI::BaseModHybridButton( this, "BtnOptions", "#rd_settings_options", this, "Options" );
	m_pBtnAudio = new BaseModUI::BaseModHybridButton( this, "BtnAudio", "#rd_settings_audio", this, "Audio" );
	m_pBtnVideo = new BaseModUI::BaseModHybridButton( this, "BtnVideo", "#rd_settings_video", this, "Video" );
	m_pBtnAbout = new BaseModUI::BaseModHybridButton( this, "BtnAbout", "#rd_settings_about", this, "About" );

	m_pPnlControls = new CRD_VGUI_Settings_Controls( this, "PnlControls" );
	m_pPnlOptions = new CRD_VGUI_Settings_Options( this, "PnlOptions" );
	m_pPnlAudio = new CRD_VGUI_Settings_Audio( this, "PnlAudio" );
	m_pPnlVideo = new CRD_VGUI_Settings_Video( this, "PnlVideo" );
	m_pPnlAbout = new CRD_VGUI_Settings_About( this, "PnlAbout" );
}

CRD_VGUI_Settings_Panel_Base::CRD_VGUI_Settings_Panel_Base( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
}

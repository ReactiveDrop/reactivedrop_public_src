#include "cbase.h"
#include "vloadouts.h"
#include "rd_loadout.h"
#include "nb_header_footer.h"
#include "rd_vgui_main_menu_top_bar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

Loadouts::Loadouts( Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	SetProportional( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetBackgroundStyle( NB_BACKGROUND_NONE );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( false );
	m_pTopBar = new CRD_VGUI_Main_Menu_Top_Bar( this, "TopBar" );
	m_pTopBar->m_hActiveButton = m_pTopBar->m_pTopButton[0];

	// loadout name list
	// - load
	// - delete
	// save as
	// medal slot x3
	// for each marine (x8):
	// - marine/suit slot
	// - primary weapon slot
	// - secondary weapon slot
	// - equipment slot
	// options:
	// - load medals from config
	// - auto update current loadout

	SetTitle( "", false );
	SetDeleteSelfOnClose( true );
	SetLowerGarnishEnabled( false );
	SetMoveable( false );
}

Loadouts::~Loadouts()
{
}

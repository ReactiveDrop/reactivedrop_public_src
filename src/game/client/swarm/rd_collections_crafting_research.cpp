#include "cbase.h"
#include "rd_collections_crafting_research.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


CRD_Collection_Tab_Crafting_Research::CRD_Collection_Tab_Crafting_Research( TabbedGridDetails *parent, const char *szLabel )
	: BaseClass( parent, szLabel )
{
}

vgui::Panel *CRD_Collection_Tab_Crafting_Research::CreatePanel()
{
	return new CRD_Crafting_Research_Panel( this );
}

CRD_Crafting_Research_Panel::CRD_Crafting_Research_Panel( CRD_Collection_Tab_Crafting_Research *pTab )
	: BaseClass( pTab->m_pParent, "CraftingResearchPanel" )
{
	SetConsoleStylePanel( true );

	m_pParent = pTab;
}

CRD_Crafting_Research_Panel::~CRD_Crafting_Research_Panel()
{
	m_pParent->m_pPanel = nullptr;
}

void CRD_Crafting_Research_Panel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	bool bWasVisible = IsVisible();

	LoadControlSettings( "Resource/UI/CraftingResearchPanel.res" );

	SetVisible( bWasVisible );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Crafting_Research_Panel::OnThink()
{
	BaseClass::OnThink();

	// TODO
}

void CRD_Crafting_Research_Panel::OnCommand( const char *szCommand )
{
	if ( false )
	{
		// TODO
	}
	else
	{
		BaseClass::OnCommand( szCommand );
	}
}

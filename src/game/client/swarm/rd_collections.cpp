#include "cbase.h"
#include "rd_collections.h"
#include "asw_util_shared.h"
#include <vgui_controls/Label.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


vgui::DHANDLE<TabbedGridDetails> g_hCollectionFrame;
void LaunchCollectionsFrame()
{
	TabbedGridDetails *pFrame = g_hCollectionFrame;
	if ( pFrame )
	{
		pFrame->SetVisible( false );
		pFrame->MarkForDeletion();
		g_hCollectionFrame = NULL;
	}

	pFrame = new TabbedGridDetails();
	pFrame->SetTitle( "#rd_collection_title", true );
	pFrame->AddTab( new CRD_Collection_Tab_Inventory( pFrame, "#rd_collection_inventory_medals", "medal" ) );
#ifdef RD_COLLECTIONS_WEAPONS_ENABLED
	pFrame->AddTab( new CRD_Collection_Tab_Equipment( pFrame, "#rd_collection_weapons", NULL, false ) );
	pFrame->AddTab( new CRD_Collection_Tab_Equipment( pFrame, "#rd_collection_equipment", NULL, true ) );
#endif
#ifdef RD_COLLECTIONS_SWARMOPEDIA_ENABLED
	pFrame->AddTab( new CRD_Collection_Tab_Swarmopedia( pFrame, "#rd_collection_swarmopedia" ) );
#endif
	pFrame->ShowFullScreen();

	g_hCollectionFrame = pFrame;
}

static int rd_collections_completion( const char *partial, char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH] )
{
	return 0;
}

CON_COMMAND_F_COMPLETION( rd_collections, "open collections view", FCVAR_CLIENTCMD_CAN_EXECUTE, rd_collections_completion )
{
	if ( args.ArgC() > 1 )
	{
		ConMsg( "Usage: rd_collections\n" );
		return;
	}

	LaunchCollectionsFrame();
}

DECLARE_BUILD_FACTORY( CRD_Collection_StatLine );

CRD_Collection_StatLine::CRD_Collection_StatLine( vgui::Panel *parent, const char *panelName )
	: BaseClass( parent, panelName )
{
	m_pLblTitle = new vgui::Label( this, "LblTitle", L"" );
	m_pLblStat = new vgui::Label( this, "LblStat", L"" );
}

void CRD_Collection_StatLine::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	LoadControlSettings( "resource/UI/CollectionStatLine.res" );

	BaseClass::ApplySchemeSettings( pScheme );
}

void CRD_Collection_StatLine::SetLabel( const char *szLabel )
{
	m_pLblTitle->SetText( szLabel );
}

void CRD_Collection_StatLine::SetLabel( const wchar_t *wszLabel )
{
	m_pLblTitle->SetText( wszLabel );
}

void CRD_Collection_StatLine::SetValue( int64_t nValue )
{
	m_pLblStat->SetText( UTIL_RD_CommaNumber( nValue ) );
}

#include "cbase.h"
#include "rd_collections.h"

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
	pFrame->AddTab( new CRD_Collection_Tab_Equipment( pFrame, "#rd_collection_weapons", false ) );
	pFrame->AddTab( new CRD_Collection_Tab_Equipment( pFrame, "#rd_collection_equipment", true ) );
	pFrame->AddTab( new CRD_Collection_Tab_Swarmopedia( pFrame, "#rd_collection_swarmopedia" ) );
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

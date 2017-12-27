#include "cbase.h"
#include "nb_lobby_row_small.h"
#include "asw_briefing.h"
#include "vgui_controls/ImagePanel.h"

#include "vgui_controls/Panel.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CNB_Lobby_Row_Small::CNB_Lobby_Row_Small( vgui::Panel *parent, const char *name ) : BaseClass( parent, name )
{
	// == MANAGED_MEMBER_CREATION_START: Do not edit by hand ==
	m_pReadyCheckImage = new vgui::ImagePanel( this, "ReadyCheckImage" );
	m_pBackroundPlain = new vgui::Panel( this, "BackroundPlain" );
	// == MANAGED_MEMBER_CREATION_END ==
	m_pReadyCheckImage->SetShouldScaleImage( true );
}

CNB_Lobby_Row_Small::~CNB_Lobby_Row_Small()
{

}

void CNB_Lobby_Row_Small::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::BaseClass::ApplySchemeSettings( pScheme );
	
	LoadControlSettings( "resource/ui/nb_lobby_row_small.res" );

	for ( int i = 0;i < ASW_NUM_INVENTORY_SLOTS; i++ )
	{
		m_szLastWeaponImage[i][0] = 0;
	}
	m_szLastPortraitImage[ 0 ] = 0;
	m_lastSteamID.Set( 0, k_EUniverseInvalid, k_EAccountTypeInvalid );

	SetTall( m_pBackroundPlain->GetTall() );
}

void CNB_Lobby_Row_Small::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CNB_Lobby_Row_Small::UpdateDetails()
{
	if ( !Briefing()->IsLobbySlotOccupied( m_nLobbySlot ) || Briefing()->IsOfflineGame() )
	{
		m_pReadyCheckImage->SetVisible( false );
	}
	else
	{
		m_pReadyCheckImage->SetVisible( true );
		if ( Briefing()->GetCommanderReady( m_nLobbySlot ) )
		{
			m_pReadyCheckImage->SetImage( "swarm/HUD/TickBoxTicked" );
		}
		else if ( Briefing()->IsLeader( m_nLobbySlot ) )
		{
			m_pReadyCheckImage->SetImage( "swarm/PlayerList/LeaderIcon" );
		}
		else
		{
			m_pReadyCheckImage->SetImage( "swarm/HUD/TickBoxEmpty" );
		}
	}

	BaseClass::UpdateDetails();
}

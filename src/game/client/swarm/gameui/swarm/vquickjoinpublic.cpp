//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "VQuickJoinPublic.h"
#include "filesystem.h"
#include "vgui/ILocalize.h"
#include "asw_util_shared.h"
#include "rd_lobby_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;
using namespace vgui;

extern ConVar cl_quick_join_scroll_max;
extern ConVar cl_quick_join_scroll_start;
extern ConVar cl_quick_join_panel_fakecount;

DECLARE_BUILD_FACTORY( QuickJoinPublicPanel );

//=============================================================================
// Quick Join Panel
//=============================================================================
QuickJoinPublicPanel::QuickJoinPublicPanel( vgui::Panel *parent , const char* panelName ):
	BaseClass( parent, panelName )
{
	RefreshQuery();
}

QuickJoinPublicPanel::~QuickJoinPublicPanel()
{
}

void QuickJoinPublicPanel::OnCommand(const char *command)
{
	if ( StringHasPrefix( command, "CustomMatch_" ) )
	{
		// relay the command up to our parent
		if ( Panel *pParent = GetParent() )
		{
			pParent->OnCommand( command );
		}
	}
}

void QuickJoinPublicPanel::OnMousePressed( vgui::MouseCode code )
{
	switch ( code )
	{
	case MOUSE_LEFT:
		if ( GetParent() )
		{
			GetParent()->OnCommand( "CustomMatch_" );
		}
		break;
	}
}

void QuickJoinPublicPanel::AddServersToList( void )
{
	if ( cl_quick_join_panel_fakecount.GetInt() >= 0 )
	{
		BaseClass::AddServersToList();
		return;
	}

	int nServers = g_ReactiveDropServerList.m_PublicServers.Count();
	for ( int iServer = 0; iServer < nServers; iServer++ )
	{
		if ( g_ReactiveDropServerList.m_PublicServers.HasPassword( iServer ) || !g_ReactiveDropServerList.m_PublicServers.IsVACSecure( iServer ) )
			continue;

		int iInfo = m_FriendInfo.AddToTail();
		m_FriendInfo[iInfo].m_eType = g_ReactiveDropServerList.m_PublicServers.IsHoIAFServer( iServer ) ? QuickInfo::TYPE_SERVER_RANKED : QuickInfo::TYPE_SERVER;
		m_FriendInfo[iInfo].m_xuid = g_ReactiveDropServerList.m_PublicServers.GetSteamID( iServer ).ConvertToUint64();
		V_UTF8ToUnicode( g_ReactiveDropServerList.m_PublicServers.GetName( iServer ), m_FriendInfo[iInfo].m_wszName, sizeof( m_FriendInfo[iInfo].m_wszName ) );
	}

	int nLobbies = g_ReactiveDropServerList.m_PublicLobbies.m_MatchingLobbies.Count();
	if ( nLobbies > 0 )
	{
		int iInfo = m_FriendInfo.AddToTail();
		m_FriendInfo[iInfo].m_eType = QuickInfo::TYPE_PUBLIC_LOBBY_COUNT_PLACEHOLDER;
		m_FriendInfo[iInfo].m_xuid = nLobbies;
		g_pVGuiLocalize->ConstructString( m_FriendInfo[iInfo].m_wszName, sizeof( m_FriendInfo[iInfo].m_wszName ), g_pVGuiLocalize->Find( "#L4D360UI_MainMenu_PublicLobbies_More" ), 1, UTIL_RD_CommaNumber( nLobbies ) );
	}

	RefreshQuery();
}

void QuickJoinPublicPanel::RefreshQuery()
{
	g_ReactiveDropServerList.m_PublicLobbies.WantUpdatedLobbyList();
	g_ReactiveDropServerList.m_PublicServers.WantUpdatedServerList();
}

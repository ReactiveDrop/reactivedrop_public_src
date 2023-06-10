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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;
using namespace vgui;

extern ConVar cl_quick_join_scroll_max;
extern ConVar cl_quick_join_scroll_start;
extern ConVar cl_quick_join_panel_fakecount;

DECLARE_BUILD_FACTORY( QuickJoinPublicPanel );

#ifdef DBGFLAG_ASSERT
// on debug builds, make sure that the participating servers file is up-to-date.
// we don't do this on release builds because it'd just create a bunch of network traffic for no gain.
static class CCheckHoIAFParticipatingServersFile : public CAutoGameSystem
{
public:
	CCheckHoIAFParticipatingServersFile() : CAutoGameSystem( "CCheckHoIAFParticipatingServersFile" )
	{
	}

	void PostInit() override
	{
		ISteamHTTP *pHTTP = SteamHTTP();
		Assert( pHTTP );
		if ( !pHTTP )
			return;

		HTTPRequestHandle hRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodGET, "https://stats.reactivedrop.com/hoiaf_participating_servers.bin" );
		Assert( hRequest != INVALID_HTTPREQUEST_HANDLE );
		SteamAPICall_t hCall{ k_uAPICallInvalid };
		bool bOK = pHTTP->SendHTTPRequest( hRequest, &hCall );
		Assert( bOK );

		m_HTTPRequestCompleted.Set( hCall, this, &CCheckHoIAFParticipatingServersFile::OnHTTPRequestCompleted );
	}

	void OnHTTPRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
	{
		Assert( !bIOFailure && pParam->m_bRequestSuccessful );
		if ( bIOFailure || !pParam->m_bRequestSuccessful )
			return;

		Assert( pParam->m_eStatusCode == k_EHTTPStatusCode200OK );

		CUtlBuffer localBuf;
		bool bOK = g_pFullFileSystem->ReadFile( "resource/hoiaf_participating_servers.bin", "MOD", localBuf );
		Assert( bOK );

		Assert( localBuf.TellMaxPut() == int( pParam->m_unBodySize ) );

		CUtlBuffer remoteBuf{ 0, int( pParam->m_unBodySize ), 0 };
		bOK = SteamHTTP()->GetHTTPResponseBodyData( pParam->m_hRequest, ( uint8_t * )remoteBuf.Base(), pParam->m_unBodySize );
		Assert( bOK );

		Assert( !V_memcmp( localBuf.Base(), remoteBuf.Base(), MIN( localBuf.TellMaxPut(), int( pParam->m_unBodySize ) ) ) );
		SteamHTTP()->ReleaseHTTPRequest( pParam->m_hRequest );
	}

	CCallResult<CCheckHoIAFParticipatingServersFile, HTTPRequestCompleted_t> m_HTTPRequestCompleted;
} s_CheckHoIAFParticipatingServersFile;
#endif

//=============================================================================
// Quick Join Panel
//=============================================================================
QuickJoinPublicPanel::QuickJoinPublicPanel( vgui::Panel *parent , const char* panelName ):
	BaseClass( parent, panelName )
{
	CUtlBuffer buf;
	if ( g_pFullFileSystem->ReadFile( "resource/hoiaf_participating_servers.bin", "MOD", buf ) )
	{
		m_ParticipatingServers.SetCount( buf.Size() / 4 );
		V_memcpy( m_ParticipatingServers.Base(), buf.Base(), m_ParticipatingServers.Count() * 4 );
	}

	RefreshQuery();
}

QuickJoinPublicPanel::~QuickJoinPublicPanel()
{
	ISteamMatchmakingServers *pMatchmakingServers = SteamMatchmakingServers();
	if ( pMatchmakingServers && m_hServerListRequest )
	{
		pMatchmakingServers->ReleaseRequest( m_hServerListRequest );
		m_hServerListRequest = NULL;
	}
	if ( pMatchmakingServers && m_hServerListRequestNext )
	{
		pMatchmakingServers->ReleaseRequest( m_hServerListRequestNext );
		m_hServerListRequestNext = NULL;
	}
}

void QuickJoinPublicPanel::OnCommand(const char *command)
{
	if ( StringHasPrefix( command, "GroupServer_" ) )
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

	ISteamMatchmakingServers *pMatchmakingServers = SteamMatchmakingServers();
	if ( pMatchmakingServers && m_hServerListRequest )
	{
		int nServers = pMatchmakingServers->GetServerCount( m_hServerListRequest );
		for ( int iServer = 0; iServer < nServers; iServer++ )
		{
			gameserveritem_t *pServer = pMatchmakingServers->GetServerDetails( m_hServerListRequest, iServer );
			if ( !pServer || pServer->m_bPassword || !pServer->m_bSecure )
				continue;

			CSplitString tags( pServer->m_szGameTags, "," );

			bool bIsHoIAF = false;
			FOR_EACH_VEC( tags, i )
			{
				if ( !V_strcmp( tags[i], "HoIAF" ) )
				{
					bIsHoIAF = true;
					break;
				}
			}

			if ( bIsHoIAF && !m_ParticipatingServers.IsValidIndex( m_ParticipatingServers.Find( pServer->m_NetAdr.GetIP() ) ) )
			{
				bIsHoIAF = false;

				// it's not an HoIAF server, so make sure it's still valid to show in the list
				if ( pServer->m_nPlayers <= 0 || pServer->m_nPlayers >= pServer->m_nMaxPlayers )
					continue;
			}

			int iInfo = m_FriendInfo.AddToTail();
			m_FriendInfo[iInfo].m_eType = bIsHoIAF ? QuickInfo::TYPE_SERVER_RANKED : QuickInfo::TYPE_SERVER;
			m_FriendInfo[iInfo].m_xuid = pServer->m_steamID.ConvertToUint64();
			V_UTF8ToUnicode( pServer->GetName(), m_FriendInfo[iInfo].m_wszName, sizeof( m_FriendInfo[iInfo].m_wszName ) );
		}
	}

	if ( m_nPublicLobbies > 0 )
	{
		int iInfo = m_FriendInfo.AddToTail();
		m_FriendInfo[iInfo].m_eType = QuickInfo::TYPE_PUBLIC_LOBBY_COUNT_PLACEHOLDER;
		m_FriendInfo[iInfo].m_xuid = m_nPublicLobbies;
		g_pVGuiLocalize->ConstructString( m_FriendInfo[iInfo].m_wszName, sizeof( m_FriendInfo[iInfo].m_wszName ), g_pVGuiLocalize->Find( "#L4D360UI_MainMenu_PublicLobbies_More" ), 1, UTIL_RD_CommaNumber( m_nPublicLobbies ) );
	}

	RefreshQuery();
}

void QuickJoinPublicPanel::RefreshQuery()
{
	ISteamMatchmaking *pMatchmaking = SteamMatchmaking();
	ISteamMatchmakingServers *pMatchmakingServers = SteamMatchmakingServers();
	Assert( pMatchmaking && pMatchmakingServers );
	if ( !pMatchmaking || !pMatchmakingServers )
	{
		m_nPublicLobbies = 0;
		m_hServerListRequest = NULL;

		return;
	}

	if ( !m_LobbyListCallback.IsActive() )
	{
		pMatchmaking->AddRequestLobbyListFilterSlotsAvailable( 1 );
		pMatchmaking->AddRequestLobbyListDistanceFilter( k_ELobbyDistanceFilterWorldwide );
		SteamAPICall_t hCall = pMatchmaking->RequestLobbyList();
		m_LobbyListCallback.Set( hCall, this, &QuickJoinPublicPanel::OnLobbyList );
	}

	if ( !m_hServerListRequestNext )
	{
		MatchMakingKeyValuePair_t ServerQuery[]
		{
			{ "and", "6" },
			{ "secure", "" },
			{ "or", "4" },
			{ "and", "2" },
			{ "notfull", "" },
			{ "hasplayers", "" },
			{ "gametagsand", "HoIAF" },
		};
		MatchMakingKeyValuePair_t *pServerQuery[NELEMS( ServerQuery )];
		for ( int i = 0; i < NELEMS( ServerQuery ); i++ )
		{
			pServerQuery[i] = &ServerQuery[i];
		}

		m_hServerListRequestNext = pMatchmakingServers->RequestInternetServerList( SteamUtils()->GetAppID(), pServerQuery, NELEMS( pServerQuery ), this );
	}
}

void QuickJoinPublicPanel::OnLobbyList( LobbyMatchList_t *pParam, bool bIOFailure )
{
	if ( bIOFailure )
	{
		m_nPublicLobbies = 0;
		return;
	}

	m_nPublicLobbies = pParam->m_nLobbiesMatching;
}

void QuickJoinPublicPanel::ServerResponded( HServerListRequest hRequest, int iServer )
{
	Assert( m_hServerListRequestNext == hRequest );
}
void QuickJoinPublicPanel::ServerFailedToRespond( HServerListRequest hRequest, int iServer )
{
	Assert( m_hServerListRequestNext == hRequest );
}
void QuickJoinPublicPanel::RefreshComplete( HServerListRequest hRequest, EMatchMakingServerResponse response )
{
	Assert( m_hServerListRequestNext == hRequest );

	if ( m_hServerListRequest )
		SteamMatchmakingServers()->ReleaseRequest( m_hServerListRequest );

	m_hServerListRequest = m_hServerListRequestNext;
	m_hServerListRequestNext = NULL;
}

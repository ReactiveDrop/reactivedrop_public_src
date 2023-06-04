#ifndef __QUICKJOIN_PUBLIC_H__
#define __QUICKJOIN_PUBLIC_H__

#include "VQuickJoin.h"

namespace BaseModUI
{
	class QuickJoinPublicPanel : public QuickJoinPanel, public ISteamMatchmakingServerListResponse
	{
		DECLARE_CLASS_SIMPLE( QuickJoinPublicPanel, QuickJoinPanel );

	public:
		QuickJoinPublicPanel( vgui::Panel *parent, const char *panelName );
		virtual ~QuickJoinPublicPanel();

		virtual void OnMousePressed( vgui::MouseCode code );
		virtual void OnCommand(const char *command);

	protected:
		virtual void AddServersToList( void );
		virtual const char *GetTitle( void ) { return "#L4D360UI_MainMenu_PublicLobbies"; }

		// HoIAF server list
		CUtlVector<uint32_t> m_ParticipatingServers;

		void RefreshQuery();

		// lobbies
		void OnLobbyList( LobbyMatchList_t *pParam, bool bIOFailure );
		CCallResult<QuickJoinPublicPanel, LobbyMatchList_t> m_LobbyListCallback;
		int m_nPublicLobbies{};

		// ISteamMatchmakingServerListResponse implementation
		void ServerResponded( HServerListRequest hRequest, int iServer );
		void ServerFailedToRespond( HServerListRequest hRequest, int iServer );
		void RefreshComplete( HServerListRequest hRequest, EMatchMakingServerResponse response );
		HServerListRequest m_hServerListRequest{};
		HServerListRequest m_hServerListRequestNext{};
	};
};

#endif	// __QUICKJOIN_PUBLIC_H__

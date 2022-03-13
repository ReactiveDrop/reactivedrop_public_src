#pragma once

#include "VFoundGames.h"

namespace BaseModUI
{
	class GenericPanelList;
	class BaseModHybridButton;

	class FoundGroupGamesIAFRanks : public FoundGames, public ISteamMatchmakingServerListResponse
	{
		DECLARE_CLASS_SIMPLE( FoundGroupGamesIAFRanks, FoundGames );

	public:
		FoundGroupGamesIAFRanks( vgui::Panel *parent, const char *panelName );
		virtual ~FoundGroupGamesIAFRanks();

		virtual void PaintBackground();
		virtual void OnEvent( KeyValues *pEvent );

		virtual bool CanCreateGame() { return false; }

	protected:
		virtual void StartSearching( void );
		virtual void AddServersToList( void );

		virtual void ServerResponded( HServerListRequest hRequest, int iServer );
		virtual void ServerFailedToRespond( HServerListRequest hRequest, int iServer );
		virtual void RefreshComplete( HServerListRequest hRequest, EMatchMakingServerResponse response );

		HServerListRequest m_hServerListRequest;
		bool m_bRefreshFinished;
		CUtlVector<KeyValuesAD> m_KeyValuesCleanup;
	};
}

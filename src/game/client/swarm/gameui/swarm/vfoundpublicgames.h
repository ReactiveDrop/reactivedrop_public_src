//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#ifndef __VFOUND_PUBLIC_GAMES_H__
#define __VFOUND_PUBLIC_GAMES_H__

#include "basemodui.h"
#include "VFoundGames.h"
#include "rd_lobby_utils.h"

class CRD_VGUI_Option;
class CNB_Button;

namespace BaseModUI {

	class GenericPanelList;
	class FoundGames;
	class BaseModHybridButton;


	//=============================================================================

	class FoundPublicGames : public FoundGames
	{
		DECLARE_CLASS_SIMPLE( FoundPublicGames, FoundGames );

	public:
		FoundPublicGames( vgui::Panel *parent, const char *panelName );
		~FoundPublicGames();

		virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
		virtual void OnCommand( const char *command );
		virtual void Activate();
		virtual void OnKeyCodePressed( vgui::KeyCode code );

		MESSAGE_FUNC_CHARPTR( OnItemSelected, "OnItemSelected", panelName );
		MESSAGE_FUNC_PTR( OnCurrentOptionChanged, "CurrentOptionChanged", panel );

		CRD_VGUI_Option *m_pOptionNameMode;
		CRD_VGUI_Option *m_pOptionDifficultyMin;
		CRD_VGUI_Option *m_pOptionDifficultyMax;
		CRD_VGUI_Option *m_pOptionOnslaught;
		CRD_VGUI_Option *m_pOptionHardcoreFF;
		CRD_VGUI_Option *m_pOptionDedicated;
		CRD_VGUI_Option *m_pOptionInstalled;
		CRD_VGUI_Option *m_pOptionChallenge;
		CRD_VGUI_Option *m_pOptionAlwaysFriends;
		CRD_VGUI_Option *m_pOptionDistance;
		CNB_Button *m_pBtnFilters;
		bool m_bFiltersVisible;

	protected:

		virtual void AddServersToList( void );
		virtual void AddPublicGamesToList();
		virtual void AddDedicatedServersToList();
		virtual char const * GetListHeaderText();

		virtual void UpdateTitle();

	private:
		void UpdateDifficultyMinMaxLock();
		bool CanCreateGame();

#if !defined( _X360 ) && !defined( NO_STEAM )
		CCallResult<FoundPublicGames, NumberOfCurrentPlayers_t> m_callbackNumberOfCurrentPlayers;
		void Steam_OnNumberOfCurrentPlayers( NumberOfCurrentPlayers_t *pResult, bool bError );
#endif

		int m_numCurrentPlayers;
	};

};

#endif

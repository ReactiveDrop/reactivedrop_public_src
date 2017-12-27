#ifndef _INCLUDED_NB_LOBBY_ROW_SMALL_H
#define _INCLUDED_NB_LOBBY_ROW_SMALL_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include "vgui_controls/ImagePanel.h"
#include "swarm/gameui/swarm/vgenericpanellist.h"
#include "nb_lobby_row.h"

// == MANAGED_CLASS_DECLARATIONS_START: Do not edit by hand ==
class vgui::ImagePanel;
class vgui::Panel;
// == MANAGED_CLASS_DECLARATIONS_END ==

class CNB_Lobby_Row_Small : public CNB_Lobby_Row, BaseModUI::IGenericPanelListItem
{
	DECLARE_CLASS_SIMPLE( CNB_Lobby_Row_Small, CNB_Lobby_Row );
public:
	CNB_Lobby_Row_Small( vgui::Panel *parent, const char *name );
	virtual ~CNB_Lobby_Row_Small();
	
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();

    virtual bool IsLabel() { return false; }

	virtual void UpdateDetails();
	
	// == MANAGED_MEMBER_POINTERS_START: Do not edit by hand ==
	vgui::ImagePanel	*m_pReadyCheckImage;
	vgui::Panel	*m_pBackroundPlain;
	// == MANAGED_MEMBER_POINTERS_END ==
};

#endif // _INCLUDED_NB_LOBBY_ROW_SMALL_H



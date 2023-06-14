#pragma once

#include "basemodui.h"
#include "vgenericpanellist.h"

class CNB_Header_Footer;
class CRD_VGUI_Main_Menu_Top_Bar;

namespace ReactiveDropLoadout
{
	struct LoadoutData_t;
}

namespace BaseModUI
{

class Loadouts : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( Loadouts, CBaseModFrame );

public:
	Loadouts( vgui::Panel *parent, const char *panelName );
	~Loadouts();

	CNB_Header_Footer *m_pHeaderFooter;
	CRD_VGUI_Main_Menu_Top_Bar *m_pTopBar;
	GenericPanelList *m_pGplSavedLoadouts;

	void DisplayPublishingError( const char *szMessage, int nArgs = 0, const wchar_t *wszArg1 = NULL, const wchar_t *wszArg2 = NULL, const wchar_t *wszArg3 = NULL, const wchar_t *wszArg4 = NULL );
	void StartPublishingLoadouts( const char *szTitle, const char *szDescription, const char *szPreviewFile, const CUtlDict<ReactiveDropLoadout::LoadoutData_t> &loadouts );
	void OnRemoteStoragePublishFileResult( RemoteStoragePublishFileResult_t *pParam, bool bIOFailure );
	CCallResult<Loadouts, RemoteStoragePublishFileResult_t> m_RemoteStoragePublishFileResult;
};


class CRD_VGUI_Loadout_List_Item : public vgui::EditablePanel, public IGenericPanelListItem
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_List_Item, vgui::EditablePanel );
public:
	CRD_VGUI_Loadout_List_Item( vgui::Panel *parent, const char *panelName, const char *szName, const ReactiveDropLoadout::LoadoutData_t &loadout, PublishedFileId_t id = k_PublishedFileIdInvalid );

	bool IsLabel() override { return false; }
};

class CRD_VGUI_Loadout_List_Addon_Header : public vgui::EditablePanel, public IGenericPanelListItem
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_List_Addon_Header, vgui::EditablePanel );
public:
	CRD_VGUI_Loadout_List_Addon_Header( vgui::Panel *parent, const char *panelName, PublishedFileId_t id );

	bool IsLabel() override { return true; }
};

}

#ifndef _INCLUDED_IASW_MISSION_CHOOSER_LIST_H
#define _INCLUDED_IASW_MISSION_CHOOSER_LIST_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_shareddefs.h"
#include "missionchooser/iasw_mission_chooser_source.h"
#include <vgui/VGUI.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/PropertyPage.h>

class CASW_Mission_Chooser_Entry;
class ServerOptionsPanel;

enum class ASW_CHOOSER_TYPE;
enum class ASW_HOST_TYPE;

class CASW_Mission_Chooser_List : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_List, vgui::Panel );
public:
	CASW_Mission_Chooser_List( vgui::Panel *pParent, const char *pElementName, ASW_CHOOSER_TYPE iChooserType, ASW_HOST_TYPE iHostType, IASW_Mission_Chooser_Source *pMissionSource );
	virtual ~CASW_Mission_Chooser_List();

	virtual void PerformLayout();
	virtual void OnThink();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	void UpdateDetails();
	virtual void OnCommand( const char *command );
	void OnEntryClicked( CASW_Mission_Chooser_Entry *pClicked );
	void OnSaveDeleted();
	const char *GenerateNewSaveGameName();
	void CloseSelf();
	void UpdateNumPages();
	void ChangeToShowingMissionsWithinCampaign( int nCampaignIndex );

	vgui::Button *m_pCancelButton;
	vgui::Button *m_pNextPageButton;
	vgui::Button *m_pPrevPageButton;
	vgui::Label *m_pPageLabel;
	vgui::Label *m_pTitleLabel;
	vgui::CheckButton *m_pShowAllCheck;
	CASW_Mission_Chooser_Entry *m_pEntry[ASW_SAVES_PER_PAGE]; // this array needs to be large enough for either ASW_SAVES_PER_SCREEN (for when showing save games), or ASW_MISSIONS_PER_SCREEN (for when showing missions) or ASW_CAMPAIGNS_PER_SCREEN (for when showing campaigns)

	ASW_CHOOSER_TYPE m_ChooserType;
	ASW_HOST_TYPE m_HostType;
	int m_iPage;
	int m_iNumSlots;
	int m_iMaxPages;
	int m_nCampaignIndex;
	IASW_Mission_Chooser_Source *m_pMissionSource;

	ServerOptionsPanel *m_pServerOptions;

	MESSAGE_FUNC( OnButtonChecked, "CheckButtonChecked" );
};

#endif // _INCLUDED_IASW_MISSION_CHOOSER_LIST_H

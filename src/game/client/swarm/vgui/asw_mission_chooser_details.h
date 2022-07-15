#ifndef _INCLUDED_IASW_MISSION_CHOOSER_DETAILS_H
#define _INCLUDED_IASW_MISSION_CHOOSER_DETAILS_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>

class CASW_Mission_Chooser_Entry;
class CampaignMapSearchLights;
class vgui::ImagePanel;
class vgui::Label;

class CASW_Mission_Chooser_Details : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_Details, vgui::EditablePanel );
public:
	CASW_Mission_Chooser_Details( vgui::Panel *pParent, const char *pElementName );
	virtual ~CASW_Mission_Chooser_Details();

	virtual void OnThink();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();

	void HighlightEntry( CASW_Mission_Chooser_Entry *pEntry );

	int m_nDataResets;
	int m_nForceReLayout;
	vgui::ImagePanel *m_pImage;
	vgui::Panel *m_pBackdrop;
	vgui::Label *m_pTitle;
	vgui::Label *m_pDescription;
	vgui::ImagePanel *m_pMapBase;
	vgui::ImagePanel *m_pMapLayer[3];
	CampaignMapSearchLights *m_pSearchLights;
	vgui::DHANDLE<CASW_Mission_Chooser_Entry> m_pLastEntry;
};

#endif // _INCLUDED_IASW_MISSION_CHOOSER_DETAILS_H

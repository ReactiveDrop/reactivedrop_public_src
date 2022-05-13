#ifndef _INCLUDED_IASW_MISSION_CHOOSER_LIST_H
#define _INCLUDED_IASW_MISSION_CHOOSER_LIST_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>

enum class ASW_CHOOSER_TYPE;
enum class ASW_HOST_TYPE;

class CASW_Mission_Chooser_Frame;
class CASW_Mission_Chooser_Entry;

class CASW_Mission_Chooser_List : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_List, vgui::EditablePanel );
public:
	CASW_Mission_Chooser_List( vgui::Panel *pParent, const char *pElementName, ASW_CHOOSER_TYPE iChooserType, CASW_Mission_Chooser_Frame *pFrame );
	virtual ~CASW_Mission_Chooser_List();

	virtual void OnThink();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void PerformLayout();

	void ClearList();
	void AddEntry( CASW_Mission_Chooser_Entry *pEntry );
	void BuildCampaignList( const char *szRequiredTag );
	void BuildMissionList( const char *szRequiredTag );

	int m_nDataResets;
	int m_nLastX, m_nLastY;
	ASW_CHOOSER_TYPE m_ChooserType;
	CASW_Mission_Chooser_Frame *m_pFrame;
	CUtlVector<vgui::DHANDLE<CASW_Mission_Chooser_Entry>> m_Entries;
};

#endif // _INCLUDED_IASW_MISSION_CHOOSER_LIST_H

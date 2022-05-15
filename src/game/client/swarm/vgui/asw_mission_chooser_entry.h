#ifndef _INCLUDED_IASW_MISSION_CHOOSER_ENTRY_H
#define _INCLUDED_IASW_MISSION_CHOOSER_ENTRY_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui_controls/EditablePanel.h>

enum class ASW_CHOOSER_TYPE;
enum class ASW_HOST_TYPE;

struct RD_Campaign_t;
struct RD_Mission_t;

namespace vgui
{
	class ImagePanel;
	class Label;
}

class CASW_Mission_Chooser_List;

class CASW_Mission_Chooser_Entry : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_Entry, vgui::EditablePanel );
public:
	CASW_Mission_Chooser_Entry( vgui::Panel *pParent, const char *pElementName, CASW_Mission_Chooser_List *pList, const RD_Campaign_t *pCampaign, const RD_Mission_t *pMission );
	CASW_Mission_Chooser_Entry( vgui::Panel *pParent, const char *pElementName, CASW_Mission_Chooser_List *pList, ASW_CHOOSER_TYPE iChooserType );
	virtual ~CASW_Mission_Chooser_Entry();

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	vgui::EditablePanel *m_pFocusHolder;
	CASW_Mission_Chooser_List *m_pList;
	const RD_Campaign_t *m_pCampaign;
	const RD_Mission_t *m_pMission;
	ASW_CHOOSER_TYPE m_WorkshopChooserType;

	vgui::Panel *m_pHighlight;
	vgui::ImagePanel *m_pImage;
	vgui::Label *m_pTitle;
};

#endif // _INCLUDED_IASW_MISSION_CHOOSER_ENTRY_H

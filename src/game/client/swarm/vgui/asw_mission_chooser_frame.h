#ifndef _INCLUDED_IASW_MISSION_CHOOSER_FRAME_H
#define _INCLUDED_IASW_MISSION_CHOOSER_FRAME_H
#ifdef _WIN32
#pragma once
#endif

#include "missionchooser/iasw_mission_chooser_source.h"
#include <vgui_controls/Frame.h>

// chooser types - what we're going to launch
enum class ASW_CHOOSER_TYPE
{
	CAMPAIGN,
	SAVED_CAMPAIGN,
	SINGLE_MISSION,
	BONUS_MISSION,
	DEATHMATCH,

	NUM_TYPES,
};

extern const char *g_ASW_ChooserTypeName[int( ASW_CHOOSER_TYPE::NUM_TYPES )];

// host types - how we're going to launch it
enum class ASW_HOST_TYPE
{
	SINGLEPLAYER,
	CREATESERVER,
	CALLVOTE,

	NUM_TYPES,
};

extern const char *g_ASW_HostTypeName[int( ASW_HOST_TYPE::NUM_TYPES )];

class CASW_Mission_Chooser_List;
class ServerOptionsPanel;
namespace vgui
{
	class PropertySheet;
};

class CASW_Mission_Chooser_Frame : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_Frame, vgui::Frame );
public:
	CASW_Mission_Chooser_Frame( vgui::Panel *pParent, const char *pElementName, ASW_HOST_TYPE iHostType, IASW_Mission_Chooser_Source *pMissionSource );
	virtual ~CASW_Mission_Chooser_Frame();

	virtual void PerformLayout();
	virtual void OnThink();
	virtual void OnClose();
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	CUtlVector<CASW_Mission_Chooser_List *> m_ChooserLists;
	ServerOptionsPanel *m_pOptionsPanel;
	vgui::PropertySheet *m_pSheet;

	IASW_Mission_Chooser_Source *m_pMissionSource;
};

#endif // _INCLUDED_IASW_MISSION_CHOOSER_FRAME_H
#ifndef _INCLUDED_IASW_MISSION_CHOOSER_FRAME_H
#define _INCLUDED_IASW_MISSION_CHOOSER_FRAME_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

// chooser types - what we're going to launch
enum class ASW_CHOOSER_TYPE
{
	CAMPAIGN,
	SAVED_CAMPAIGN,
	SINGLE_MISSION,
	BONUS_MISSION,
	DEATHMATCH,
	ENDLESS,

	NUM_TYPES,
};

extern const char *const g_ASW_ChooserTypeName[int( ASW_CHOOSER_TYPE::NUM_TYPES )];

// host types - how we're going to launch it
enum class ASW_HOST_TYPE
{
	SINGLEPLAYER,
	CREATESERVER,
	CALLVOTE,

	NUM_TYPES,
};

extern const char *const g_ASW_HostTypeName[int( ASW_HOST_TYPE::NUM_TYPES )];

class CNB_Header_Footer;
class CASW_Mission_Chooser_List;
class CASW_Mission_Chooser_Entry;
class CASW_Mission_Chooser_Details;
class vgui::PropertySheet;

class CASW_Mission_Chooser_Frame : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_Frame, vgui::Frame );
public:
	CASW_Mission_Chooser_Frame( vgui::Panel *pParent, const char *pElementName, ASW_HOST_TYPE iHostType );
	virtual ~CASW_Mission_Chooser_Frame();

	virtual void OnCommand( const char *command );
	virtual void OnKeyCodeTyped( vgui::KeyCode keycode );
	virtual void OnKeyCodePressed( vgui::KeyCode keycode );
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	void ApplyEntry( CASW_Mission_Chooser_Entry *pEntry );
	void ApplyCampaign( ASW_CHOOSER_TYPE iChooserType, const char *szCampaignName );
	bool SelectTab( ASW_CHOOSER_TYPE iChooserType );

	ASW_HOST_TYPE m_HostType;
	CNB_Header_Footer *m_pHeaderFooter;
	vgui::PropertySheet *m_pSheet;
	vgui::PropertySheet *m_pCampaignMissionList;
	CASW_Mission_Chooser_Details *m_pDetails;
	bool m_bViewingCampaign;
};

#endif // _INCLUDED_IASW_MISSION_CHOOSER_FRAME_H
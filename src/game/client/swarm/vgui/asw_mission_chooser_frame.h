#ifndef _INCLUDED_IASW_MISSION_CHOOSER_FRAME_H
#define _INCLUDED_IASW_MISSION_CHOOSER_FRAME_H
#ifdef _WIN32
#pragma once
#endif

#include "tabbedgriddetails.h"

struct RD_Campaign_t;
struct RD_Mission_t;
class CASW_Mission_Chooser_Tab;
class vgui::ImagePanel;
class vgui::Label;
class CNB_Header_Footer;
class CampaignMapSearchLights;

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

class CASW_Mission_Chooser_Frame : public TabbedGridDetails
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_Frame, TabbedGridDetails );
public:
	CASW_Mission_Chooser_Frame( ASW_HOST_TYPE iHostType );

	virtual void OnCommand( const char *command ) override;

	void ApplyCampaign( ASW_CHOOSER_TYPE iChooserType, const char *szCampaignName );
	bool SelectTab( ASW_CHOOSER_TYPE iChooserType );

	ASW_HOST_TYPE m_HostType;
	CUtlVector<CASW_Mission_Chooser_Tab *> m_MainTabs;
	bool m_bViewingCampaign;
};

class CASW_Mission_Chooser_Tab : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_Tab, TGD_Tab );
public:
	CASW_Mission_Chooser_Tab( CASW_Mission_Chooser_Frame *pFrame, ASW_CHOOSER_TYPE iChooserType, const char *szCampaignName = NULL );

	virtual TGD_Details *CreateDetails() override;
	virtual void OnThink() override;

	void BuildCampaignList( const char *szRequiredTag );
	void BuildMissionList( const char *szRequiredTag );
	void BuildCampaignMissionList();

	int m_nDataResets;
	int m_nLastX, m_nLastY;
	ASW_CHOOSER_TYPE m_ChooserType;
	char m_szCampaignName[64];
};

class CASW_Mission_Chooser_Details : public TGD_Details
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_Details, TGD_Details );
public:
	explicit CASW_Mission_Chooser_Details( TGD_Tab *pTab );
	virtual ~CASW_Mission_Chooser_Details();

	virtual void OnThink() override;
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void PerformLayout() override;
	virtual void DisplayEntry( TGD_Entry *pEntry ) override;

	int m_nDataResets;
	int m_nForceReLayout;
	vgui::ImagePanel *m_pImage;
	vgui::Panel *m_pBackdrop;
	vgui::Label *m_pTitle;
	vgui::Label *m_pDescription;
	vgui::ImagePanel *m_pMapBase;
	vgui::ImagePanel *m_pMapLayer[3];
	CampaignMapSearchLights *m_pSearchLights;
};

class CASW_Mission_Chooser_Entry : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CASW_Mission_Chooser_Entry, TGD_Entry );
public:
	CASW_Mission_Chooser_Entry( TGD_Grid *parent, const char *panelName, const RD_Campaign_t *pCampaign, const RD_Mission_t *pMission );
	CASW_Mission_Chooser_Entry( TGD_Grid *parent, const char *panelName, ASW_CHOOSER_TYPE iChooserType );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void ApplyEntry() override;

	char m_szCampaign[64];
	char m_szMission[64];
	ASW_CHOOSER_TYPE m_WorkshopChooserType;

	vgui::ImagePanel *m_pImage;
	vgui::Label *m_pTitle;
};

#endif // _INCLUDED_IASW_MISSION_CHOOSER_FRAME_H

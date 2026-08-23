#pragma once

#include "tabbedgriddetails.h"

#include "steam/isteamhttp.h"
#include "steam/isteaminventory.h"
#include "steam/isteamuser.h"
#include "asw_util_shared.h"
#include "gameui/swarm/vgenericpanellist.h"
#include "statsbar.h"

class CRD_Collection_Tab_Crafting_Research : public TGD_Tab_Panel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Crafting_Research, TGD_Tab_Panel );
public:
	CRD_Collection_Tab_Crafting_Research( TabbedGridDetails *parent, const char *szLabel );

	vgui::Panel *CreatePanel() override;
};

class CRD_Crafting_Research_Slot;
class CRD_Crafting_Research_Component;
class CRD_Crafting_Research_Donate_Modal;
class CRD_VGUI_Option;

class CRD_Crafting_Research_Panel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Research_Panel, vgui::EditablePanel );
public:
	explicit CRD_Crafting_Research_Panel( CRD_Collection_Tab_Crafting_Research *pTab );
	~CRD_Crafting_Research_Panel() override;

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;
	void OnCommand( const char *szCommand ) override;
	void OnThink() override;

	void ShowError( const char *szError );
	void RequestResearchState();
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	STEAM_CALLBACK( CRD_Crafting_Research_Panel, OnGetTicketForWebApiResponse, GetTicketForWebApiResponse_t );
#else
	STEAM_CALLBACK( CRD_Crafting_Research_Panel, OnGetTicketForWebApiResponse, GetTicketForWebApiResponse_t );
#endif
	void OnGetStateRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure );

	CRD_Collection_Tab_Crafting_Research *m_pParent = nullptr;
	HAuthTicket m_hGetStateAuth = k_HAuthTicketInvalid;
	HTTPRequestHandle m_hGetStateRequest = INVALID_HTTPREQUEST_HANDLE;
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	CCallResult<CRD_Crafting_Research_Panel, HTTPRequestCompleted_t> m_OnGetStateRequestCompleted;
#else
	CCallResult<CRD_Crafting_Research_Panel, HTTPRequestCompleted_t> m_OnGetStateRequestCompleted;
#endif

	vgui::DHANDLE<CRD_Crafting_Research_Donate_Modal> m_hDonateModal;
	vgui::Label *m_pLblUpdateTimer;
	vgui::Panel *m_pBackdrop;
	vgui::Panel *m_pBackdropError;
	vgui::Label *m_pLblErrorMessage;
	CNB_Button *m_pBtnRetryAfterError;
	double m_flNextUpdateTime;
	bool m_bErrored;

	struct ResearchComponent_t
	{
		int ID = 0;
		SteamItemDef_t ItemDef = 0;
		int TotalRequirement = 0;
		int MaxPersonal = 0;
		int TotalContributed = 0;
		int PersonalContributed = 0;
		int MaxDonation() const { return MIN( TotalRequirement - TotalContributed, MaxPersonal - PersonalContributed ); }
	};
	struct ResearchSlot_t
	{
		int ID = 0;
		CUtlString Title;
		CUtlString WaitDescription;
		RTime32 WaitUntil = 0;

		int ProjectID = 0;
		SteamItemDef_t ItemForIcon = 0;
		CUtlWString ProjectTitle;
		CUtlWString ProjectFlavorText;

		CUtlVectorAutoPurge<ResearchComponent_t *> Components;
	};
	CUtlVectorAutoPurge<ResearchSlot_t *> m_Slots;
	CUtlVector<CRD_Crafting_Research_Slot *> m_VisualSlots;
};

class CRD_Crafting_Research_Slot : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Research_Slot, vgui::EditablePanel );
public:
	CRD_Crafting_Research_Slot( vgui::Panel *parent, const char *panelName );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnThink() override;

	void UpdateState();
	void UpdateTimer();

	CRD_Crafting_Research_Panel *m_pParent = nullptr;
	int m_iSlotIndex = -1;

	vgui::Label *m_pLblTitle;
	vgui::Label *m_pLblWaitDescription;
	vgui::Label *m_pLblWaitTimer;
	vgui::ImagePanel *m_pImgItemIcon;
	vgui::Label *m_pLblProjectTitle;
	vgui::Label *m_pLblProjectFlavorText;
	CNB_Button *m_pBtnFetchProjectComponents;
	BaseModUI::GenericPanelList *m_pGplComponents;
};

class CRD_Crafting_Research_Component : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Research_Component, vgui::EditablePanel );
public:
	CRD_Crafting_Research_Component( vgui::Panel *parent, const char *panelName );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnCommand( const char *szCommand ) override;
	void PostChildPaint() override;

	void UpdateState();

	CRD_Crafting_Research_Slot *m_pParent = nullptr;
	int m_iComponentIndex = -1;

	vgui::ImagePanel *m_pImgItemIcon;
	vgui::Label *m_pLblItemName;
	StatsBar *m_pCommunityProgressBar;
	vgui::Label *m_pLblPersonalProgress;
	CNB_Button *m_pBtnContribute;

	CPanelAnimationVarAliasType( int, m_nDonateTextureId, "donate_texture", "vgui/swarm/swarmopedia/inbox_2d", "textureid" );
};

class CRD_Crafting_Research_Donate_Modal : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Crafting_Research_Donate_Modal, vgui::EditablePanel );
public:
	CRD_Crafting_Research_Donate_Modal( vgui::Panel *parent, const char *panelName );
	~CRD_Crafting_Research_Donate_Modal() override;

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnCommand( const char *szCommand ) override;

#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	STEAM_CALLBACK( CRD_Crafting_Research_Donate_Modal, OnGetTicketForWebApiResponse, GetTicketForWebApiResponse_t );
#else
	STEAM_CALLBACK( CRD_Crafting_Research_Donate_Modal, OnGetTicketForWebApiResponse, GetTicketForWebApiResponse_t );
#endif
	void OnDonateRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure );
	MESSAGE_FUNC_PTR( OnCurrentOptionChanged, "CurrentOptionChanged", panel );

	CRD_Crafting_Research_Panel *m_pParent = nullptr;
	HAuthTicket m_hDonateAuth = k_HAuthTicketInvalid;
	HTTPRequestHandle m_hDonateRequest = INVALID_HTTPREQUEST_HANDLE;
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	CCallResult<CRD_Crafting_Research_Donate_Modal, HTTPRequestCompleted_t> m_OnDonateRequestCompleted;
#else
	CCallResult<CRD_Crafting_Research_Donate_Modal, HTTPRequestCompleted_t> m_OnDonateRequestCompleted;
#endif

	int m_iSelectedProjectID = 0;
	int m_iDonateComponentID = 0;
	SteamItemDef_t m_iDonateItemDef = 0;
	SteamItemInstanceID_t m_iDonateItemInstance = k_SteamItemInstanceIDInvalid;
	int m_iDonatedQuantity = 1;
	int m_iDonateMaxQuantity = 0;

	vgui::ImagePanel *m_pImgItemIcon;
	vgui::Label *m_pLblItemName;
	vgui::Label *m_pLblDisplayType;
	CRD_VGUI_Option *m_pQuantitySlider;
	CNB_Button *m_pBtnConfirm;
	CNB_Button *m_pBtnCancel;
};

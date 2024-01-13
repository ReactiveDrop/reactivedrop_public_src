#pragma once

#include <vgui_controls/Button.h>
#include <vgui_controls/EditablePanel.h>
#include "rd_hoiaf_utils.h"

namespace BaseModUI
{
	class GenericPanelList;
}

namespace vgui
{
	class MultiFontRichText;
}

class CNB_Button;
class CRD_VGUI_Notifications_Button;
class CRD_VGUI_Notifications_List;
class CRD_VGUI_Notifications_List_Item;
class CRD_VGUI_Notifications_Details;
class CRD_VGUI_Notifications_Filters;

extern CUtlVector<CRD_VGUI_Notifications_Button *> g_NotificationsButtons;
extern CUtlVector<CRD_VGUI_Notifications_List *> g_NotificationsLists;

class CRD_VGUI_Notifications_Button : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_Button, vgui::Button );
public:
	CRD_VGUI_Notifications_Button( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Notifications_Button();

	void UpdateNotifications();
	void OnCommand( const char *command ) override;
	void PaintBackground() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;

	vgui::Label *m_pLblCounter;
	CRD_VGUI_Notifications_List *m_pListPopOut;

	CPanelAnimationVar( Color, m_BoldCounterColor, "bold_counter_color", "224 64 32 255" );
	CPanelAnimationVar( Color, m_MutedCounterColor, "muted_counter_color", "64 68 72 255" );
	CPanelAnimationVar( Color, m_CounterFgColor, "counter_fg_color", "255 255 255 255" );
	CPanelAnimationVarAliasType( int, m_iButtonIcon, "button_icon", "vgui/swarm/notification_button", "textureid" );
};

class CRD_VGUI_Notifications_List : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_List, vgui::EditablePanel );
public:
	CRD_VGUI_Notifications_List( vgui::Panel *parent, const char *panelName );
	~CRD_VGUI_Notifications_List();

	void UpdateNotifications();
	CRD_VGUI_Notifications_List_Item *CreateNotificationListItem( HoIAFNotification_t *pNotification );
	void OnThink() override;
	void OnCommand( const char *command ) override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;

	BaseModUI::GenericPanelList *m_pList;
	vgui::Label *m_pLblNone;
	CNB_Button *m_pFiltersButton;
	CRD_VGUI_Notifications_Filters *m_pFiltersPopOut;
	int64_t m_iLastTimerUpdate;
};

class CRD_VGUI_Notifications_List_Item : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_List_Item, vgui::EditablePanel );
public:
	CRD_VGUI_Notifications_List_Item( vgui::Panel *parent, const char *panelName, HoIAFNotification_t *pNotification );

	virtual void InitFromNotification();
	virtual void UpdateTimers( int64_t iNow );
	void SetSeenAtLeast( int iSeen );
	virtual void SetSeen( int iSeen ) = 0;
	virtual bool MatchesNotification( const HoIAFNotification_t *pNotification ) const = 0;
	virtual void OnClicked();
	void OnCursorEntered() override;
	void OnCursorExited() override;
	void NavigateTo() override;
	void NavigateFrom() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;

	vgui::MultiFontRichText *m_pNotificationText;
	vgui::Label *m_pLblAge;
	vgui::Label *m_pLblExpires;
	CRD_VGUI_Notifications_Details *m_pDetailsPopOut;
	HoIAFNotification_t m_Notification;
	int m_iExpectedOrder;
	bool m_bOverMouse;
	bool m_bOverKeyboard;
	vgui::HFont m_hFontBold;
	vgui::HFont m_hFontNormal;

	CPanelAnimationVar( Color, m_TitleColor, "title_color", "224 224 224 255" );
	CPanelAnimationVar( Color, m_DescriptionColor, "description_color", "160 160 160 255" );
};

class CRD_VGUI_Notifications_List_Item_Inventory : public CRD_VGUI_Notifications_List_Item
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_List_Item_Inventory, CRD_VGUI_Notifications_List_Item );
public:
	CRD_VGUI_Notifications_List_Item_Inventory( vgui::Panel *parent, const char *panelName, HoIAFNotification_t *pNotification );

	void InitFromNotification() override;
	bool MatchesNotification( const HoIAFNotification_t *pNotification ) const override;
	void SetSeen( int iSeen ) override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
};

class CRD_VGUI_Notifications_List_Item_HoIAF_Bounty : public CRD_VGUI_Notifications_List_Item
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_List_Item_HoIAF_Bounty, CRD_VGUI_Notifications_List_Item );
public:
	CRD_VGUI_Notifications_List_Item_HoIAF_Bounty( vgui::Panel *parent, const char *panelName, HoIAFNotification_t *pNotification );

	enum { MAX_MISSIONS_PER_BOUNTY = 9 };

	void InitFromNotification() override;
	bool MatchesNotification( const HoIAFNotification_t *pNotification ) const override;
	void SetSeen( int iSeen ) override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;

	vgui::ImagePanel *m_pImgCompleted[MAX_MISSIONS_PER_BOUNTY];
	vgui::ImagePanel *m_pImgMissionIcon[MAX_MISSIONS_PER_BOUNTY];
	vgui::Label *m_pLblMissionPoints[MAX_MISSIONS_PER_BOUNTY];
};

class CRD_VGUI_Notifications_Details : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_Details, vgui::EditablePanel );
public:
	CRD_VGUI_Notifications_Details( vgui::Panel *parent, const char *panelName );
};

class CRD_VGUI_Notifications_Details_HoIAF_Bounty : public CRD_VGUI_Notifications_Details
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_Details_HoIAF_Bounty, CRD_VGUI_Notifications_Details );
public:
	CRD_VGUI_Notifications_Details_HoIAF_Bounty( vgui::Panel *parent, const char *panelName );

	void SetBountyMission( int index, const HoIAFNotification_t::BountyMission_t *pMission );
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
};

class CRD_VGUI_Notifications_Filters : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_Filters, vgui::EditablePanel );
public:
	CRD_VGUI_Notifications_Filters( vgui::Panel *parent, const char *panelName );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
};

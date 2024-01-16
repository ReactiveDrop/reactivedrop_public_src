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
class CRD_VGUI_Option;

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
	void OnThink() override;
	void NavigateTo() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;

	vgui::Label *m_pLblCounter;
	vgui::DHANDLE<CRD_VGUI_Notifications_List> m_hListPopOut;

	CPanelAnimationVar( Color, m_BoldCounterColor, "bold_counter_color", "224 64 32 255" );
	CPanelAnimationVar( Color, m_MutedCounterColor, "muted_counter_color", "64 68 72 255" );
	CPanelAnimationVar( Color, m_CounterFgColor, "counter_fg_color", "255 255 255 255" );
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

	vgui::PHandle m_hButton;
	BaseModUI::GenericPanelList *m_pList;
	vgui::Label *m_pLblNone;
	CNB_Button *m_pFiltersButton;
	vgui::DHANDLE<CRD_VGUI_Notifications_Filters> m_hFiltersPopOut;
	int64_t m_iLastTimerUpdate;
};

class CRD_VGUI_Notifications_List_Item : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_List_Item, vgui::EditablePanel );
public:
	CRD_VGUI_Notifications_List_Item( vgui::Panel *parent, const char *panelName, HoIAFNotification_t *pNotification );
	~CRD_VGUI_Notifications_List_Item();

	virtual void InitFromNotification();
	virtual void UpdateTimers( int64_t iNow );
	virtual void UpdateBackgroundColor( int isFocused = -1 );
	void SetSeenAtLeast( int iSeen );
	virtual void SetSeen( int iSeen ) = 0;
	virtual bool MatchesNotification( const HoIAFNotification_t *pNotification ) const = 0;
	virtual void OnClicked();
	void OnCursorEntered() override;
	void NavigateTo() override;
	void OnSetFocus() override;
	void OnKillFocus() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;

	vgui::MultiFontRichText *m_pNotificationText;
	vgui::Label *m_pLblAge;
	vgui::Label *m_pLblExpires;
	vgui::DHANDLE<CRD_VGUI_Notifications_Details> m_hDetailsPopOut;
	HoIAFNotification_t m_Notification;
	int m_iExpectedOrder;

	CPanelAnimationVar( vgui::HFont, m_hFontBold, "font_title", "DefaultTextBold" );
	CPanelAnimationVar( vgui::HFont, m_hFontNormal, "font_desc", "Default" );
	CPanelAnimationVar( Color, m_TitleColor, "title_color", "224 224 224 255" );
	CPanelAnimationVar( Color, m_DescriptionColor, "description_color", "160 160 160 255" );
	CPanelAnimationVar( Color, m_BackgroundColorHover, "bgcolor_hover", "20 59 96 255" );
	CPanelAnimationVar( Color, m_BackgroundColorFresh, "bgcolor_fresh", "24 43 66 255" );
	CPanelAnimationVar( Color, m_BackgroundColorViewed, "bgcolor_viewed", "40 48 56 255" );
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
	CRD_VGUI_Notifications_Details( CRD_VGUI_Notifications_List_Item *listItem, const char *panelName );

	virtual void InitFromNotification() = 0;
	void OnThink() override;

	vgui::DHANDLE<CRD_VGUI_Notifications_List_Item> m_hListItem;
};

class CRD_VGUI_Notifications_Details_HoIAF_Bounty : public CRD_VGUI_Notifications_Details
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_Details_HoIAF_Bounty, CRD_VGUI_Notifications_Details );
public:
	CRD_VGUI_Notifications_Details_HoIAF_Bounty( CRD_VGUI_Notifications_List_Item *listItem, const char *panelName );

	void InitFromNotification() override;
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
};

class CRD_VGUI_Notifications_Filters : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Notifications_Filters, vgui::EditablePanel );
public:
	CRD_VGUI_Notifications_Filters( vgui::Panel *parent, const char *panelName );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;

	CRD_VGUI_Option *m_pSettingCrafting;
	CRD_VGUI_Option *m_pSettingHoIAF;
	CRD_VGUI_Option *m_pSettingReports;
};

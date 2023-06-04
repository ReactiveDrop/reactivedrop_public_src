#pragma once

#include "tabbedgriddetails.h"
#include "asw_model_panel.h"
#include "asw_marine_skills.h"
#include "rd_inventory_shared.h"

class CASW_WeaponInfo;
class CASW_Marine_Profile;
class CBitmapButton;
class IBriefing;
namespace RD_Swarmopedia
{
	struct Collection;
	struct Alien;
	struct Display;
	struct Weapon;
	struct WeaponFact;
}
namespace BaseModUI
{
	class GenericPanelList;
}

void LaunchCollectionsFrame();

class CRD_Collection_StatLine : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_StatLine, vgui::EditablePanel );
public:
	CRD_Collection_StatLine( vgui::Panel *parent, const char *panelName );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;

	void SetLabel( const char *szLabel );
	void SetLabel( const wchar_t *wszLabel );
	void SetValue( int64_t nValue );

	vgui::Label *m_pLblTitle;
	vgui::Label *m_pLblStat;
};

class CRD_Swarmopedia_Model_Panel : public CASW_Model_Panel
{
	DECLARE_CLASS_SIMPLE( CRD_Swarmopedia_Model_Panel, CASW_Model_Panel );
public:
	CRD_Swarmopedia_Model_Panel( vgui::Panel *parent, const char *panelName );

	void SetDisplay( const RD_Swarmopedia::Display *pDisplay );

private:
	void OnPaint3D() override;
	void OnMouseDoublePressed( vgui::MouseCode code ) override;

	CUtlVector<MDLData_t> m_Models;
	bool m_bDisplayChanged;
};

class CRD_Collection_Tab_Equipment : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Equipment, TGD_Tab );
public:
	CRD_Collection_Tab_Equipment( TabbedGridDetails *parent, const char *szLabel, CASW_Marine_Profile *pProfile, int nInventorySlot );
	virtual ~CRD_Collection_Tab_Equipment();

	virtual TGD_Grid *CreateGrid() override;
	virtual TGD_Details *CreateDetails() override;

	void SetBriefing( IBriefing *pBriefing, int nLobbySlot );

	RD_Swarmopedia::Collection *m_pCollection;
	CASW_Marine_Profile *m_pProfile;
	int m_nSkillValue[ASW_NUM_MARINE_SKILLS];
	int m_nInventorySlot;
	IBriefing *m_pBriefing;
	int m_nLobbySlot;
};

class CRD_Collection_Details_Equipment : public TGD_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Equipment, TGD_Details );
public:
	CRD_Collection_Details_Equipment( CRD_Collection_Tab_Equipment *parent );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void DisplayEntry( TGD_Entry *pEntry ) override;
	virtual void OnThink() override;

	void OnGlobalStatsReceived( GlobalStatsReceived_t *pParam, bool bIOError );

	int m_nStatsDays;
	bool m_bStatsReady;
	CCallResult<CRD_Collection_Details_Equipment, GlobalStatsReceived_t> m_OnGlobalStatsReceived;
	int m_nDisplayedFrames;

	CRD_Swarmopedia_Model_Panel *m_pModelPanel;
	vgui::Label *m_pWeaponNameLabel;
	vgui::Label *m_pWeaponAttrLabel;
	vgui::Label *m_pWeaponDescLabel;
	BaseModUI::GenericPanelList *m_pGplStats;
};

class CRD_Collection_Entry_Equipment : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Equipment, TGD_Entry );
public:
	CRD_Collection_Entry_Equipment( TGD_Grid *parent, const char *panelName, const RD_Swarmopedia::Weapon *pWeapon );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void OnKeyCodePressed( vgui::KeyCode keycode ) override;
	virtual void OnCommand( const char *command ) override;
	virtual void OnThink() override;
	virtual void PostChildPaint() override;
	virtual void ApplyEntry() override;

	const RD_Swarmopedia::Weapon *m_pWeapon;
	bool m_bNoDirectEquip;

	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pLockedIcon;
	vgui::Label *m_pLockedOverlay;
	vgui::Label *m_pLockedLabel;
	vgui::Label *m_pCantEquipLabel;
	vgui::Label *m_pNewLabel;
	vgui::ImagePanel *m_pClassIcon;
	vgui::Label *m_pClassLabel;
	vgui::HFont m_hButtonFont;
	CBitmapButton *m_pInfoButton;
};

class CRD_Collection_Panel_Equipment : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Panel_Equipment, vgui::EditablePanel );
public:
	CRD_Collection_Panel_Equipment( vgui::Panel *parent, const char *panelName, CRD_Collection_Tab_Equipment *pTab, const RD_Swarmopedia::Weapon *pWeapon );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void OnCommand( const char *command ) override;
	virtual void OnKeyCodePressed( vgui::KeyCode code ) override;

	MESSAGE_FUNC_CHARPTR( OnItemSelected, "OnItemSelected", panelName );

	void AddWeaponFact( const RD_Swarmopedia::WeaponFact *pFact );

	BaseModUI::GenericPanelList *m_pGplFacts;
	CNB_Button *m_pBtnEquip;

	CRD_Collection_Tab_Equipment *m_pTab;
	const RD_Swarmopedia::Weapon *m_pWeapon;
};

class CRD_Collection_Tab_Inventory : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Inventory, TGD_Tab );
public:
	CRD_Collection_Tab_Inventory( TabbedGridDetails *parent, const char *szLabel, const char *szSlot );
	virtual ~CRD_Collection_Tab_Inventory();

	virtual TGD_Details *CreateDetails() override;
	virtual void OnThink() override;

	void UpdateErrorMessage( TGD_Grid *pGrid );
	void LoadCachedInventory();

	const char *m_szSlot;
	SteamInventoryResult_t m_hResult;
	bool m_bUnavailable;
	bool m_bForceUpdateMessage;
};

class CRD_Collection_Details_Inventory : public TGD_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Inventory, TGD_Details );
public:
	CRD_Collection_Details_Inventory( CRD_Collection_Tab_Inventory *parent );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void OnThink() override;
	virtual void DisplayEntry( TGD_Entry *pEntry ) override;

	vgui::Panel *m_pIconBackground;
	vgui::ImagePanel *m_pIcon;
	vgui::RichText *m_pTitle;
	vgui::RichText *m_pDescription;
};

class CRD_Collection_Entry_Inventory : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Inventory, TGD_Entry );
public:
	CRD_Collection_Entry_Inventory( TGD_Grid *parent, const char *panelName, SteamInventoryResult_t hResult, int index );
	CRD_Collection_Entry_Inventory( TGD_Grid *parent, const char *panelName, KeyValues *pCached, int index );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void ApplyEntry() override;

	CRD_Collection_Tab_Inventory *GetTab();

	vgui::Panel *m_pIconBackground;
	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pEquippedMarker;

	int m_Index;
	ReactiveDropInventory::ItemInstance_t m_Details;
};

class CRD_Collection_Tab_Swarmopedia : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Swarmopedia, TGD_Tab );
public:
	CRD_Collection_Tab_Swarmopedia( TabbedGridDetails *parent, const char *szLabel );
	virtual ~CRD_Collection_Tab_Swarmopedia();

	virtual TGD_Grid *CreateGrid() override;
	virtual TGD_Details *CreateDetails() override;

	RD_Swarmopedia::Collection *m_pCollection;
};

class CRD_Collection_Details_Swarmopedia : public TGD_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Swarmopedia, TGD_Details );
public:
	CRD_Collection_Details_Swarmopedia( CRD_Collection_Tab_Swarmopedia *parent );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void PerformLayout() override;
	virtual void DisplayEntry( TGD_Entry *pEntry ) override;
	virtual void OnThink() override;

	void DisplayEntryLocked( const RD_Swarmopedia::Alien *pAlien );
	void OnGlobalStatsReceived( GlobalStatsReceived_t *pParam, bool bIOError );

	int m_nStatsDays;
	bool m_bStatsReady;
	CCallResult<CRD_Collection_Details_Swarmopedia, GlobalStatsReceived_t> m_OnGlobalStatsReceived;
	int m_nDisplayedFrames;

	vgui::Label *m_pLblHeader;
	vgui::Label *m_pLblAbilities;
	vgui::Label *m_pLblError;
	BaseModUI::GenericPanelList *m_pGplStats;
};

class CRD_Collection_Entry_Swarmopedia : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Swarmopedia, TGD_Entry );
public:
	CRD_Collection_Entry_Swarmopedia( TGD_Grid *parent, const char *panelName, const RD_Swarmopedia::Alien *pAlien );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void ApplyEntry() override;

	vgui::ImagePanel *m_pIcon;
	vgui::Panel *m_pUnlockProgress;

	const RD_Swarmopedia::Alien *m_pAlien;
};

class CRD_Collection_Panel_Swarmopedia : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Panel_Swarmopedia, vgui::EditablePanel );
public:
	CRD_Collection_Panel_Swarmopedia( vgui::Panel *parent, const char *panelName, const RD_Swarmopedia::Alien *pAlien );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void PerformLayout() override;
	virtual void OnCommand( const char *command ) override;
	virtual void OnKeyCodePressed( vgui::KeyCode keycode ) override;

	CRD_Swarmopedia_Model_Panel *m_pModelPanel;
	CNB_Button *m_pModelButton;
	vgui::Label *m_pLblNoModel;
	vgui::RichText *m_pContent;

	const RD_Swarmopedia::Alien *m_pAlien;
	int m_iCurrentDisplay;
};

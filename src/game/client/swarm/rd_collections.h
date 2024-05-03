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
namespace vgui
{
	class MultiFontRichText;
}

void LaunchCollectionsFrame();

class CRD_Collection_StatLine : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_StatLine, vgui::EditablePanel );
public:
	CRD_Collection_StatLine( vgui::Panel *parent, const char *panelName );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;

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

	enum Mode_t
	{
		MODE_SPINNER,
		MODE_FULLSCREEN_MOUSE,
	} m_eMode{ MODE_SPINNER };

	float m_flPitchIntensity{ -45.0f };
	float m_flYawIntensity{ 120.0f };
	float m_flPanSpeed{ 2.5f };
	QAngle m_angPanOrigin{ 15.0f, 0.0f, 0.0f };

	bool m_bUseTimeScale{ true };
	bool m_bAutoPosition{ true };
	Vector m_vecCenter;
	float m_flRadius;

private:
	void OnPaint3D() override;
	void OnMouseDoublePressed( vgui::MouseCode code ) override;

	struct RD_MDLData_t : public MDLData_t
	{
		float m_flPoseParameters[MAXSTUDIOPOSEPARAM];
		CUtlVector<MDLSquenceLayer_t> m_SequenceOverlay;
		int m_iBoneMerge{ -1 };
		int m_iRenderFlags{ STUDIORENDER_DRAW_ENTIRE_MODEL };
		bool m_bIsWeapon;
	};

	CUtlVector<RD_MDLData_t> m_Models;
	bool m_bDisplayChanged;
	float m_flSmoothXPan{};
	float m_flSmoothYPan{};
};

class CRD_Collection_Tab_Equipment : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Equipment, TGD_Tab );
public:
	CRD_Collection_Tab_Equipment( TabbedGridDetails *parent, const char *szLabel, CASW_Marine_Profile *pProfile, int nInventorySlot );
	~CRD_Collection_Tab_Equipment();

	TGD_Grid *CreateGrid() override;
	TGD_Details *CreateDetails() override;

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

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void DisplayEntry( TGD_Entry *pEntry ) override;
	void OnThink() override;

	CRD_Swarmopedia_Model_Panel *m_pModelPanel;
	vgui::Label *m_pWeaponNameLabel;
	vgui::Label *m_pWeaponAttrLabel;
	vgui::MultiFontRichText *m_pWeaponDescLabel;
	BaseModUI::GenericPanelList *m_pGplStats;
	vgui::ImagePanel *m_pImgStatsUpdating;
	int m_iStatsUpdateCount{};
};

class CRD_Collection_Entry_Equipment : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Equipment, TGD_Entry );
public:
	CRD_Collection_Entry_Equipment( TGD_Grid *parent, const char *panelName, const RD_Swarmopedia::Weapon *pWeapon, const ReactiveDropInventory::ItemInstance_t &itemInstance );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnKeyCodePressed( vgui::KeyCode keycode ) override;
	void OnCommand( const char *command ) override;
	void OnThink() override;
	void PostChildPaint() override;
	void ApplyEntry() override;

	const RD_Swarmopedia::Weapon *m_pWeapon;
	ReactiveDropInventory::ItemInstance_t m_ItemInstance;
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
	CRD_Collection_Panel_Equipment( vgui::Panel *parent, const char *panelName, CRD_Collection_Tab_Equipment *pTab, const RD_Swarmopedia::Weapon *pWeapon, const ReactiveDropInventory::ItemInstance_t &itemInstance );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnCommand( const char *command ) override;
	void OnKeyCodePressed( vgui::KeyCode code ) override;

	MESSAGE_FUNC_CHARPTR( OnItemSelected, "OnItemSelected", panelName );

	void AddWeaponFact( const RD_Swarmopedia::WeaponFact *pFact );

	BaseModUI::GenericPanelList *m_pGplFacts;
	CNB_Button *m_pBtnEquip;

	CRD_Collection_Tab_Equipment *m_pTab;
	const RD_Swarmopedia::Weapon *m_pWeapon;
	ReactiveDropInventory::ItemInstance_t m_ItemInstance;
};

class CRD_Collection_Tab_Inventory : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Inventory, TGD_Tab );
public:
	CRD_Collection_Tab_Inventory( TabbedGridDetails *parent, const char *szLabel, const char *szSlot );
	~CRD_Collection_Tab_Inventory();

	TGD_Details *CreateDetails() override;
	void OnThink() override;

	bool ShowsItemsForSlot( const char *szSlot );
	void UpdateErrorMessage( TGD_Grid *pGrid );

	CUtlStringList m_Slots;
	int m_nLastFullUpdateCount;
	bool m_bInvertSlotFilter;
	bool m_bUnavailable;
	bool m_bForceUpdateMessage;
};

class CRD_Collection_Details_Inventory : public TGD_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Inventory, TGD_Details );
public:
	CRD_Collection_Details_Inventory( CRD_Collection_Tab_Inventory *parent );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void DisplayEntry( TGD_Entry *pEntry ) override;
	void SetItemStyleOverride( TGD_Entry *pEntry, int iStyle );

	vgui::Panel *m_pIconBackground;
	vgui::ImagePanel *m_pIcon;
	vgui::RichText *m_pTitle;
	vgui::MultiFontRichText *m_pDescription;
	int m_iStyleOverride;
};

class CRD_Collection_Entry_Inventory : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Inventory, TGD_Entry );
public:
	CRD_Collection_Entry_Inventory( TGD_Grid *parent, const char *panelName, int index, const ReactiveDropInventory::ItemInstance_t &details );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void ApplyEntry() override;
	void OnCommand( const char *command ) override;

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
	~CRD_Collection_Tab_Swarmopedia();

	TGD_Grid *CreateGrid() override;
	TGD_Details *CreateDetails() override;

	RD_Swarmopedia::Collection *m_pCollection;
};

class CRD_Collection_Details_Swarmopedia : public TGD_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Swarmopedia, TGD_Details );
public:
	CRD_Collection_Details_Swarmopedia( CRD_Collection_Tab_Swarmopedia *parent );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;
	void DisplayEntry( TGD_Entry *pEntry ) override;
	void OnThink() override;

	void DisplayEntryLocked( const RD_Swarmopedia::Alien *pAlien );

	vgui::Label *m_pLblHeader;
	vgui::Label *m_pLblAbilities;
	vgui::Label *m_pLblError;
	BaseModUI::GenericPanelList *m_pGplStats;
	vgui::ImagePanel *m_pImgStatsUpdating;
	int m_iStatsUpdateCount{};
};

class CRD_Collection_Entry_Swarmopedia : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Swarmopedia, TGD_Entry );
public:
	CRD_Collection_Entry_Swarmopedia( TGD_Grid *parent, const char *panelName, const RD_Swarmopedia::Alien *pAlien );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void ApplyEntry() override;

	vgui::ImagePanel *m_pIcon;
	vgui::Panel *m_pUnlockProgress;

	const RD_Swarmopedia::Alien *m_pAlien;
};

class CRD_Collection_Panel_Swarmopedia : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Panel_Swarmopedia, vgui::EditablePanel );
public:
	CRD_Collection_Panel_Swarmopedia( vgui::Panel *parent, const char *panelName, const RD_Swarmopedia::Alien *pAlien );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void PerformLayout() override;
	void OnCommand( const char *command ) override;
	void OnKeyCodePressed( vgui::KeyCode keycode ) override;

	CRD_Swarmopedia_Model_Panel *m_pModelPanel;
	CNB_Button *m_pModelButton;
	vgui::Label *m_pLblNoModel;
	vgui::RichText *m_pContent;

	const RD_Swarmopedia::Alien *m_pAlien;
	int m_iCurrentDisplay;
};

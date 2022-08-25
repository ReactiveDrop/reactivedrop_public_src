#pragma once

#include "tabbedgriddetails.h"
#include "steam/steam_api.h"

class CASW_WeaponInfo;
class CASW_Model_Panel;
class CRD_Swarmopedia_Model_Panel;
namespace RD_Swarmopedia
{
	struct Collection;
	struct Alien;
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

#ifdef RD_COLLECTIONS_WEAPONS_ENABLED

class CRD_Collection_Tab_Equipment : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Equipment, TGD_Tab );
public:
	CRD_Collection_Tab_Equipment( TabbedGridDetails *parent, const char *szLabel, bool bExtra );

	virtual TGD_Grid *CreateGrid() override;
	virtual TGD_Details *CreateDetails() override;

	bool m_bExtra;
};

class CRD_Collection_Details_Equipment : public TGD_Details
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Details_Equipment, TGD_Details );
public:
	CRD_Collection_Details_Equipment( CRD_Collection_Tab_Equipment *parent );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void OnThink() override;
	virtual void DisplayEntry( TGD_Entry *pEntry ) override;

	float m_flZOffset;
	CASW_Model_Panel *m_pModelPanel;
	vgui::Label *m_pWeaponNameLabel;
	vgui::Label *m_pWeaponDescLabel;
};

class CRD_Collection_Entry_Equipment : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Equipment, TGD_Entry );
public:
	CRD_Collection_Entry_Equipment( TGD_Grid *parent, const char *panelName, int iEquipIndex, const char *szEquipClass );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void ApplyEntry() override;

	int m_iEquipIndex;
	CASW_WeaponInfo *m_pWeaponInfo;
	int m_iRequiredLevel;

	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pLockedIcon;
	vgui::Label *m_pLockedOverlay;
	vgui::Label *m_pLockedLabel;
	vgui::ImagePanel *m_pClassIcon;
	vgui::Label *m_pClassLabel;
};

class CRD_Collection_Panel_Equipment : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Panel_Equipment, vgui::EditablePanel );
public:
	CRD_Collection_Panel_Equipment( vgui::Panel *parent, const char *panelName, CASW_WeaponInfo *pWeaponInfo );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;

	CASW_WeaponInfo *m_pWeaponInfo;
};

#endif

class CRD_Collection_Tab_Inventory : public TGD_Tab
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Tab_Inventory, TGD_Tab );
public:
	CRD_Collection_Tab_Inventory( TabbedGridDetails *parent, const char *szLabel, const char *szSlot );
	virtual ~CRD_Collection_Tab_Inventory();

	virtual TGD_Details *CreateDetails() override;
	virtual void OnThink() override;

	void UpdateErrorMessage( TGD_Grid *pGrid );

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

	vgui::ImagePanel *m_pIcon;
	vgui::RichText *m_pTitle;
	vgui::RichText *m_pDescription;
};

class CRD_Collection_Entry_Inventory : public TGD_Entry
{
	DECLARE_CLASS_SIMPLE( CRD_Collection_Entry_Inventory, TGD_Entry );
public:
	CRD_Collection_Entry_Inventory( TGD_Grid *parent, const char *panelName, int index, SteamItemDetails_t details );

	virtual void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	virtual void ApplyEntry() override;

	CRD_Collection_Tab_Inventory *GetTab();

	vgui::ImagePanel *m_pIcon;
	vgui::ImagePanel *m_pEquippedMarker;

	int m_Index;
	SteamItemDetails_t m_Details;
};

#ifdef RD_COLLECTIONS_SWARMOPEDIA_ENABLED

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

#endif

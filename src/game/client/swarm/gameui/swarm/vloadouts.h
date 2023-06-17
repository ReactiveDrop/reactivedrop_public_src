#pragma once

#include "basemodui.h"
#include "vgenericpanellist.h"
#include "asw_shareddefs.h"
#include "rd_loadout.h"

class CBitmapButton;
class CNB_Header_Footer;
class CNB_Skill_Panel;
class CRD_Swarmopedia_Model_Panel;
class CRD_VGUI_Main_Menu_Top_Bar;

namespace BaseModUI
{

class BaseModHybridButton;
class CRD_VGUI_Loadout_List_Item;
class CRD_VGUI_Loadout_List_Addon_Header;
class CRD_VGUI_Loadout_Marine;
class CRD_VGUI_Loadout_Slot_Inventory;
class CRD_VGUI_Loadout_Slot_Marine;
class CRD_VGUI_Loadout_Slot_Weapon;

class Loadouts : public CBaseModFrame
{
	DECLARE_CLASS_SIMPLE( Loadouts, CBaseModFrame );

public:
	Loadouts( vgui::Panel *parent, const char *panelName );
	~Loadouts();

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnCommand( const char *command ) override;

	CNB_Header_Footer *m_pHeaderFooter;
	CRD_VGUI_Main_Menu_Top_Bar *m_pTopBar;
	GenericPanelList *m_pGplSavedLoadouts;
	CRD_VGUI_Loadout_Slot_Inventory *m_pMedalSlot[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];
	vgui::Label *m_pLblMedal[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];
	vgui::ImagePanel *m_pImgPromotionIcon;
	vgui::Label *m_pLblPromotion;
	CBitmapButton *m_pBtnMarineLoadout[ASW_NUM_MARINE_PROFILES];
	vgui::PHandle m_hSubScreen;

	void DisplayPublishingError( const char *szMessage, int nArgs = 0, const wchar_t *wszArg1 = NULL, const wchar_t *wszArg2 = NULL, const wchar_t *wszArg3 = NULL, const wchar_t *wszArg4 = NULL );
	void StartPublishingLoadouts( const char *szTitle, const char *szDescription, const char *szPreviewFile, const CUtlDict<ReactiveDropLoadout::LoadoutData_t> &loadouts );
	void OnRemoteStoragePublishFileResult( RemoteStoragePublishFileResult_t *pParam, bool bIOFailure );
	CCallResult<Loadouts, RemoteStoragePublishFileResult_t> m_RemoteStoragePublishFileResult;
};

class CRD_VGUI_Loadout_List_Item : public vgui::EditablePanel, public IGenericPanelListItem
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_List_Item, vgui::EditablePanel );
public:
	CRD_VGUI_Loadout_List_Item( vgui::Panel *parent, const char *panelName, const char *szName, const ReactiveDropLoadout::LoadoutData_t &loadout, PublishedFileId_t id = k_PublishedFileIdInvalid );

	bool IsLabel() override { return false; }

	char m_szName[MAX_VALUE];
	ReactiveDropLoadout::LoadoutData_t m_Loadout;
	PublishedFileId_t m_iAddonID;
};

class CRD_VGUI_Loadout_List_Addon_Header : public vgui::EditablePanel, public IGenericPanelListItem
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_List_Addon_Header, vgui::EditablePanel );
public:
	CRD_VGUI_Loadout_List_Addon_Header( vgui::Panel *parent, const char *panelName, PublishedFileId_t id );

	bool IsLabel() override { return true; }

	PublishedFileId_t m_iAddonID;
};

class CRD_VGUI_Loadout_Marine : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_Marine, vgui::EditablePanel );
public:
	CRD_VGUI_Loadout_Marine( vgui::Panel *parent, const char *panelName, ASW_Marine_Profile iProfile );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void SetupDisplay();

	CRD_Swarmopedia_Model_Panel *m_pModelPanel;
	vgui::RichText *m_pLblBiography;
	CNB_Skill_Panel *m_pSkillPanel[ASW_NUM_SKILL_SLOTS - 1];
	CRD_VGUI_Loadout_Slot_Marine *m_pMarine;
	CRD_VGUI_Loadout_Slot_Weapon *m_pWeapon[ASW_NUM_INVENTORY_SLOTS];
};

class CRD_VGUI_Loadout_Slot : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_Slot, vgui::EditablePanel );
public:
	CRD_VGUI_Loadout_Slot( vgui::Panel *parent, const char *panelName, ConVar *pInventoryVar );

	void Paint() override;
	virtual bool PaintItemFullSize() { return false; }
	const ReactiveDropInventory::ItemInstance_t *GetItem();

	ConVar *m_pInventoryVar;
};

class CRD_VGUI_Loadout_Slot_Inventory : public CRD_VGUI_Loadout_Slot
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_Slot_Inventory, CRD_VGUI_Loadout_Slot );
public:
	CRD_VGUI_Loadout_Slot_Inventory( vgui::Panel *parent, const char *panelName, ConVar *pInventoryVar, const char *szSlot );

	bool PaintItemFullSize() override { return true; }

	char m_szSlot[64];
};

class CRD_VGUI_Loadout_Slot_Marine : public CRD_VGUI_Loadout_Slot
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_Slot_Marine, CRD_VGUI_Loadout_Slot );
public:
	CRD_VGUI_Loadout_Slot_Marine( vgui::Panel *parent, const char *panelName, ASW_Marine_Profile iProfile, ConVar *pInventoryVar );

	void Paint() override;

	ASW_Marine_Profile m_iProfile;
	char m_szSlot[64];
};

class CRD_VGUI_Loadout_Slot_Weapon : public CRD_VGUI_Loadout_Slot
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_Slot_Weapon, CRD_VGUI_Loadout_Slot );
public:
	CRD_VGUI_Loadout_Slot_Weapon( vgui::Panel *parent, const char *panelName, ASW_Marine_Profile iProfile, ConVar *pWeaponVar, ConVar *pInventoryVar, ASW_Inventory_slot_t iSlot );

	void Paint() override;

	ASW_Marine_Profile m_iProfile;
	ASW_Inventory_slot_t m_iSlot;
	ConVar *m_pWeaponVar;
	ConVar *m_pInventoryVar;
	char m_szSlot[64];
};

}

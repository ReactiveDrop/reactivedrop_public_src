#pragma once

#include "basemodui.h"
#include "vgenericpanellist.h"
#include "asw_shareddefs.h"
#include "rd_loadout.h"
#include "ibriefing.h"
#include "asw_marine_profile.h"

class CAvatarImagePanel;
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

class Loadouts : public CBaseModFrame, public IBriefing
{
	DECLARE_CLASS_SIMPLE( Loadouts, CBaseModFrame );

public:
	Loadouts( vgui::Panel *parent, const char *panelName );
	~Loadouts();

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnCommand( const char *command ) override;
	void OnKeyCodeTyped( vgui::KeyCode code ) override;

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

	// IBriefing implementation
	const char *GetLeaderName() override { return ""; }
	const char *GetTeamName() override { return ""; }
	Color GetTeamColor() override { return Color{}; }
	bool IsLocalPlayerLeader() override { return true; }
	void ToggleLocalPlayerReady() override {}
	bool CheckMissionRequirements() override { return true; }
	bool AreOtherPlayersReady() override { return true; }
	void StartMission() override {}
	bool IsLobbySlotOccupied( int nLobbySlot ) override { return false; }
	bool IsLobbySlotLocal( int nLobbySlot ) override { return false; }
	bool IsLobbySlotBot( int nLobbySlot ) override { return false; }
	const wchar_t *GetMarineOrPlayerName( int nLobbySlot ) override { return L""; }
	const wchar_t *GetMarineName( int nLobbySlot ) override { return L""; }
	const char *GetPlayerNameForMarineProfile( int nProfileIndex ) override { return ""; }
	int GetCommanderLevel( int nLobbySlot ) override { return 0; }
	int GetCommanderXP( int nLobbySlot ) override { return 0; }
	int GetCommanderPromotion( int nLobbySlot ) override { return 0; }
	bool IsFullyConnected( int nLobbySlot ) override { return false; }
	CSteamID GetCommanderSteamID( int nLobbySlot ) override { return k_steamIDNil; }
	ASW_Marine_Class GetMarineClass( int nLobbySlot ) override { return GetMarineProfile( nLobbySlot )->GetMarineClass(); }
	CASW_Marine_Profile *GetMarineProfile( int nLobbySlot ) override;
	CASW_Marine_Profile *GetMarineProfileByProfileIndex( int nProfileIndex ) override;
	int GetProfileSelectedWeapon( int nProfileIndex, int nWeaponSlot ) override;
	int GetMarineSelectedWeapon( int nLobbySlot, int nWeaponSlot ) override { return GetProfileSelectedWeapon( nLobbySlot, nWeaponSlot ); }
	const char *GetMarineWeaponClass( int nLobbySlot, int nWeaponSlot ) override;
	int GetCommanderReady( int nLobbySlot ) override { return false; }
	bool IsLeader( int nLobbySlot ) override { return false; }
	int GetMarineSkillPoints( int nLobbySlot, int nSkillSlot ) override;
	int GetProfileSkillPoints( int nProfileIndex, int nSkillSlot ) override;
	bool IsWeaponUnlocked( const char *szWeaponClass ) override;
	bool IsProfileSelectedBySomeoneElse( int nProfileIndex ) override { return false; }
	bool IsProfileSelected( int nProfileIndex ) override { return true; }
	int GetMaxPlayers() override { return ASW_NUM_MARINE_PROFILES; }
	bool IsOfflineGame() override { return true; }
	bool IsCampaignGame() override { return false; }
	bool UsingFixedSkillPoints() override { return true; }
	void SetChangingWeaponSlot( int nLobbySlot, int nWeaponSlot ) override {}
	int GetChangingWeaponSlot( int nLobbySlot ) override { return -1; }
	bool IsCommanderSpeaking( int nLobbySlot ) override { return false; }
	void SelectMarine( int nOrder, int nProfileIndex, int nPreferredLobbySlot ) override {}
	void SelectBot( int nOrder, int nProfileIndex ) override {}
	void SelectWeapon( int nProfileIndex, int nInventorySlot, int nEquipIndex, SteamItemInstanceID_t iItemInstance ) override;
	void AutoSelectFullSquadForSingleplayer( int nFirstSelectedProfileIndex ) override {}
	void ResetLastChatterTime() override {}
	const static IBriefing_ItemInstance s_EmptyItemInstance;
	const IBriefing_ItemInstance &GetEquippedMedal( int nLobbySlot, int nMedalIndex ) override { return s_EmptyItemInstance; }
	const IBriefing_ItemInstance &GetEquippedSuit( int nLobbySlot ) override { return s_EmptyItemInstance; }
	const IBriefing_ItemInstance &GetEquippedWeapon( int nLobbySlot, int nWeaponSlot ) override { return s_EmptyItemInstance; }
};

class CRD_VGUI_Loadout_List_Item : public vgui::EditablePanel, public IGenericPanelListItem
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_List_Item, vgui::EditablePanel );
public:
	CRD_VGUI_Loadout_List_Item( vgui::Panel *parent, const char *panelName, const char *szName, const ReactiveDropLoadout::LoadoutData_t &loadout, PublishedFileId_t id = k_PublishedFileIdInvalid );

	bool IsLabel() override { return false; }
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void ApplySettings( KeyValues *pSettings ) override;
	void Paint() override;

	char m_szName[MAX_VALUE];
	ReactiveDropLoadout::LoadoutData_t m_Loadout;
	PublishedFileId_t m_iAddonID;

	vgui::Label *m_pLblTitle;

	CPanelAnimationVar( int, m_iNumColumns, "num_columns", "2" );
	CPanelAnimationVarAliasType( int, m_iRowHeight, "row_height", "10", "proportional_int" );
};

class CRD_VGUI_Loadout_List_Addon_Header : public vgui::EditablePanel, public IGenericPanelListItem
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_List_Addon_Header, vgui::EditablePanel );
public:
	CRD_VGUI_Loadout_List_Addon_Header( vgui::Panel *parent, const char *panelName, PublishedFileId_t id );

	bool IsLabel() override { return true; }
	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnThink() override;

	PublishedFileId_t m_iAddonID;

	vgui::ImagePanel *m_pImgWorkshopIcon;
	vgui::Label *m_pLblTitle;
	CAvatarImagePanel *m_pImgAuthorAvatar;
	vgui::Label *m_pLblAuthorName;
	bool m_bLoadedAddonDetails;

	STEAM_CALLBACK( CRD_VGUI_Loadout_List_Addon_Header, OnPersonaStateChange, PersonaStateChange_t );
};

class CRD_VGUI_Loadout_Marine : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_Marine, vgui::EditablePanel );
public:
	CRD_VGUI_Loadout_Marine( vgui::Panel *parent, const char *panelName, ASW_Marine_Profile iProfile );

	void ApplySchemeSettings( vgui::IScheme *pScheme ) override;
	void OnCommand( const char *command ) override;
	void SetupDisplay();
	void OnThink() override;
	void NavigateTo() override;

	CRD_Swarmopedia_Model_Panel *m_pModelPanel;
	vgui::RichText *m_pLblBiography;
	CNB_Skill_Panel *m_pSkillPanel[ASW_NUM_SKILL_SLOTS - 1];
	vgui::ImagePanel *m_pImgClass;
	CRD_VGUI_Loadout_Slot_Marine *m_pMarine;
	CRD_VGUI_Loadout_Slot_Weapon *m_pWeapon[ASW_NUM_INVENTORY_SLOTS];
	vgui::Label *m_pLblClass;
	vgui::Label *m_pLblMarine;
	vgui::Label *m_pLblWeapon[ASW_NUM_INVENTORY_SLOTS];
	bool m_bSecondaryWeapon{ false };
};

class CRD_VGUI_Loadout_Slot : public vgui::Button
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_Slot, vgui::Button );
public:
	CRD_VGUI_Loadout_Slot( vgui::Panel *parent, const char *panelName, ConVar *pInventoryVar );

	void Paint() override;
	void OnCursorEntered() override;
	void NavigateTo() override;
	virtual bool PaintItemFullSize() { return false; }
	const ReactiveDropInventory::ItemInstance_t *GetItem();

	ConVar *m_pInventoryVar;
};

class CRD_VGUI_Loadout_Slot_Inventory : public CRD_VGUI_Loadout_Slot
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_Slot_Inventory, CRD_VGUI_Loadout_Slot );
public:
	CRD_VGUI_Loadout_Slot_Inventory( vgui::Panel *parent, const char *panelName, ConVar *pInventoryVar, const char *szSlot );

	void OnCommand( const char *command ) override;
	bool PaintItemFullSize() override { return true; }

	char m_szSlot[64];
};

class CRD_VGUI_Loadout_Slot_Marine : public CRD_VGUI_Loadout_Slot
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_Slot_Marine, CRD_VGUI_Loadout_Slot );
public:
	CRD_VGUI_Loadout_Slot_Marine( vgui::Panel *parent, const char *panelName, ASW_Marine_Profile iProfile, ConVar *pInventoryVar );

	void Paint() override;
	void OnCursorEntered() override;

	ASW_Marine_Profile m_iProfile;
	char m_szSlot[64];
};

class CRD_VGUI_Loadout_Slot_Weapon : public CRD_VGUI_Loadout_Slot
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Loadout_Slot_Weapon, CRD_VGUI_Loadout_Slot );
public:
	CRD_VGUI_Loadout_Slot_Weapon( vgui::Panel *parent, const char *panelName, ASW_Marine_Profile iProfile, ConVar *pWeaponVar, ConVar *pInventoryVar, ASW_Inventory_slot_t iSlot );

	void Paint() override;
	void OnCommand( const char *command ) override;

	ASW_Marine_Profile m_iProfile;
	ASW_Inventory_slot_t m_iSlot;
	ConVar *m_pWeaponVar;
	ConVar *m_pInventoryVar;
	char m_szSlot[64];
};

}

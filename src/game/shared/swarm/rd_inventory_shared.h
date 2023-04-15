#pragma once

#include "steam/steam_api.h"

#ifdef CLIENT_DLL
namespace vgui
{
	class IImage;
	class RichText;
}

#define CASW_Player C_ASW_Player
#define CRD_ItemInstance C_RD_ItemInstance
#define CRD_ProjectileData C_RD_ProjectileData
#endif

class CASW_Player;

namespace ReactiveDropInventory
{
	constexpr const char *const g_InventorySlotNames[] =
	{
		"medal",
		"marine0", "marine1", "marine2", "marine3",
		"marine4", "marine5", "marine6", "marine7",
		"weapon0", "weapon1", "weapon2", "weapon3", "weapon4",
		"weapon5", "weapon6", "weapon7", "weapon8", "weapon9",
		"weapon10", "weapon11", "weapon12", "weapon13", "weapon14",
		"weapon15", "weapon16", "weapon17", "weapon18", "weapon19",
		"weapon20", "weapon21", "weapon22", "weapon23", "weapon24",
		"weapon25", "weapon26", "weapon27", "weapon28", "weapon29",
		"weapon30", "weapon31", "weapon32", "weapon33", "weapon34",
#ifdef RD_7A_WEAPONS
		"weapon35", "weapon36", "weapon37", "weapon38", "weapon39",
		"weapon40",
#endif
		"extra0", "extra1", "extra2", "extra3", "extra4",
		"extra5", "extra6", "extra7", "extra8", "extra9",
		"extra10", "extra11", "extra12", "extra13", "extra14",
		"extra15", "extra16", "extra17", "extra18", "extra19",
		"extra20", "extra21",
#ifdef RD_7A_WEAPONS
		"extra22", "extra23", "extra24",
		"extra25",
#endif
	};
#define RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS NELEMS( ReactiveDropInventory::g_InventorySlotNames )
#define RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL 0
#define RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MARINE 1
#define RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_WEAPON 9
#ifdef RD_7A_WEAPONS
#define RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_EXTRA 50
#else
#define RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_EXTRA 44
#endif
#define RD_STEAM_INVENTORY_ITEM_MAX_STYLES 64
	constexpr const char *const g_InventorySlotAliases[][2] =
	{
		// first value is the name from g_InventorySlotNames, second value is the value to also accept from item definitions.
		{ "", "" }, // placeholder until we have any of these
	};

	// Data extracted from the Steam Inventory Service Schema.
	struct ItemDef_t
	{
		SteamItemDef_t ID;
		CUtlString ItemSlot;
		CUtlStringMap<CUtlStringList> Tags;
		CUtlStringMap<CUtlStringList> AllowedTagsFromTools;
		CUtlString AccessoryTag;
		int AccessoryLimit{ 4 };
		CUtlStringList CompressedDynamicProps;
		CUtlString DisplayType;
		CUtlString Name;
		CUtlString BriefingName;
		CUtlStringList StyleNames;
		CUtlString Description;
		CUtlString BeforeDescription;
		CUtlString AfterDescription;
		CUtlString AccessoryDescription;
		Color BackgroundColor;
		Color NameColor;
		bool AfterDescriptionOnlyMultiStack : 1;
		bool HasInGameDescription : 1;
		bool HasBorder : 1;
#ifdef CLIENT_DLL
		vgui::IImage *Icon{};
		CUtlVector<vgui::IImage *> StyleIcons{};
		ITexture *AccessoryIcon{};
#endif

		bool ItemSlotMatches( const char *szRequiredSlot ) const;
	};

	// Data extracted from SteamInventoryResult_t; it is safe to destroy the result after constructing this data type.
	struct ItemInstance_t
	{
		CSteamID AccountID{};
		SteamItemInstanceID_t ItemID{ k_SteamItemInstanceIDInvalid };
		SteamItemInstanceID_t OriginalItemID{ k_SteamItemInstanceIDInvalid };
		SteamItemDef_t ItemDefID{};
		int32 Quantity{};
		uint32 Acquired{};
		uint32 StateChangedTimestamp{};
		CUtlString State{};
		CUtlString Origin{};
		CUtlStringMap<CUtlString> DynamicProps{};
		CUtlStringMap<CUtlStringList> Tags{};

		explicit ItemInstance_t( SteamInventoryResult_t hResult, uint32 index );
		explicit ItemInstance_t( KeyValues *pKV );
		void FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc ) const;
#ifdef CLIENT_DLL
		void FormatDescription( vgui::RichText *pRichText ) const;
		vgui::IImage *GetIcon() const;
#endif
		int GetStyle() const;
		KeyValues *ToKeyValues() const;
		void FromKeyValues( KeyValues *pKV );
	};

	const ItemDef_t *GetItemDef( SteamItemDef_t id );
	bool DecodeItemData( SteamInventoryResult_t &hResult, const char *szEncodedData );
	bool ValidateItemData( bool &bValid, SteamInventoryResult_t hResult, const char *szRequiredSlot = NULL, CSteamID requiredSteamID = k_steamIDNil, bool bRequireFresh = false );
	bool ValidateEquipItemData( bool &bValid, SteamInventoryResult_t hResult, byte( &nIndex )[RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS], CSteamID requiredSteamID );
#ifdef CLIENT_DLL
	void AddPromoItem( SteamItemDef_t id );
	void RequestGenericPromoItems();
	void CheckPlaytimeItemGenerators( int iMarineClass );

	void CommitDynamicProperties();
#endif
	void OnHitConfirm( CBaseEntity *pAttacker, CBaseEntity *pTarget, Vector vecDamagePosition, bool bKilled, bool bDamageOverTime, bool bBlastDamage, int iDisposition, float flDamage, CBaseEntity *pWeapon );
}

#define RD_ITEM_MAX_ACCESSORIES 4
#define RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS 10
#define RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS_PER_ACCESSORY 2

#define RD_EQUIPPED_ITEMS_NOTIFICATION_WORST_CASE_SIZE ( RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS * 2048 )
#define RD_EQUIPPED_ITEMS_NOTIFICATION_PAYLOAD_SIZE_PER_PACKET ( MAX_VALUE / 2 - 1 )

#ifdef CLIENT_DLL
EXTERN_RECV_TABLE( DT_RD_ItemInstance );
EXTERN_RECV_TABLE( DT_RD_ProjectileData );

namespace vgui
{
	class RichText;
}
#else
EXTERN_SEND_TABLE( DT_RD_ItemInstance );
EXTERN_SEND_TABLE( DT_RD_ProjectileData );
#endif

// A reduced network-friendly version of the ItemInstance_t that can be transmitted from server to client.
// It does not include a signature or time information, so the data must be validated before adding it to this structure.
class CRD_ItemInstance
{
public:
	DECLARE_CLASS_NOBASE( CRD_ItemInstance );
	DECLARE_EMBEDDED_NETWORKVAR();

	CRD_ItemInstance();
	explicit CRD_ItemInstance( const ReactiveDropInventory::ItemInstance_t &instance );
	explicit CRD_ItemInstance( SteamInventoryResult_t hResult, uint32 index );

	void Reset();
	bool IsSet() const;
	void SetFromInstance( const ReactiveDropInventory::ItemInstance_t &instance );
	void FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc ) const;
#ifdef CLIENT_DLL
	static void AppendBBCode( vgui::RichText *pRichText, const wchar_t *wszBuf, Color defaultColor );
	void FormatDescription( vgui::RichText *pRichText ) const;
	vgui::IImage *GetIcon() const;
#endif
	int GetStyle() const;

	CNetworkVar( SteamItemInstanceID_t, m_iItemInstanceID );
	CNetworkVar( SteamItemDef_t, m_iItemDefID );
	CNetworkArray( SteamItemDef_t, m_iAccessory, RD_ITEM_MAX_ACCESSORIES );
	CNetworkArray( int64, m_nCounter, RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS );
};

class CRD_ProjectileData
{
public:
	DECLARE_CLASS_NOBASE( CRD_ProjectileData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CRD_ProjectileData();

	CNetworkVar( AccountID_t, m_iOriginalOwnerSteamAccount );
#ifdef CLIENT_DLL
	CNetworkVarEmbedded( CRD_ItemInstance, m_InventoryItemData );
#else
	CHandle<CASW_Player> m_hOriginalOwnerPlayer;
	int m_iInventoryEquipSlotIndex;
#endif
	CNetworkVar( bool, m_bFiredByOwner );

#ifdef GAME_DLL
	void SetFromWeapon( CBaseEntity *pCreator );
#endif
};

abstract_class IRD_Has_Projectile_Data
{
public:
	virtual const CRD_ProjectileData *GetProjectileData() const = 0;
};

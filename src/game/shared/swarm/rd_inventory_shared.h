#pragma once

#include "asw_shareddefs.h"
#include "steam/steam_api.h"

#ifdef CLIENT_DLL
namespace vgui
{
	class IImage;
	class MultiFontRichText;
}

#define CASW_Player C_ASW_Player
#define CASW_Marine_Resource C_ASW_Marine_Resource
#define CRD_ItemInstance C_RD_ItemInstance
#define CRD_ItemInstances_Player C_RD_ItemInstances_Player
#define CRD_ItemInstances_Marine_Resource C_RD_ItemInstances_Marine_Resource
#define CRD_ProjectileData C_RD_ProjectileData

extern ConVar rd_briefing_item_details_color1;
extern ConVar rd_briefing_item_details_color2;

class CSteamItemIcon;
vgui::IImage *GetSteamItemIcon( const char *szURL, bool bForceLoadRemote = false );
enum RD_Crafting_Material_t;
#endif

class CASW_Player;
class CASW_Marine_Resource;

#define RD_ITEM_MAX_ACCESSORIES 4
#define RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS 10
#define RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS_PER_ACCESSORY 2

namespace ReactiveDropInventory
{
#ifdef CLIENT_DLL
	extern int g_nFullInventoryUpdates;
#endif

	constexpr const char *const g_PlayerInventorySlotNames[] =
	{
		"medal", "medal1", "medal2",
	};
	constexpr const char *const g_MarineResourceInventorySlotNames[] =
	{
		"marine_any", "weapon", "weapon", "extra",
	};
#define RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_PLAYER NELEMS( ReactiveDropInventory::g_PlayerInventorySlotNames )
#define RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL 0
#define RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_MARINE_RESOURCE NELEMS( ReactiveDropInventory::g_MarineResourceInventorySlotNames )
#define RD_STEAM_INVENTORY_ITEM_MAX_STYLES 64
	constexpr const char *const g_InventorySlotAliases[][2] =
	{
		// first value is the name from g_InventorySlotNames, second value is the value to also accept from item definitions.

		// all three medal slots share an allowed item list
		{ "medal1", "medal" },
		{ "medal2", "medal" },

		// slots used for generating lists of marine suits
		{ "marine_any", "marine0" },
		{ "marine_any", "marine1" },
		{ "marine_any", "marine2" },
		{ "marine_any", "marine3" },
		{ "marine_any", "marine4" },
		{ "marine_any", "marine5" },
		{ "marine_any", "marine6" },
		{ "marine_any", "marine7" },
		{ "marine_any", "marine_nco" },
		{ "marine_any", "marine_sw" },
		{ "marine_any", "marine_medic" },
		{ "marine_any", "marine_tech" },

		// pseudo-slots for marine classes
		{ "marine_nco", "marine0" },
		{ "marine_sw", "marine1" },
		{ "marine_medic", "marine2" },
		{ "marine_tech", "marine3" },
		{ "marine_nco", "marine4" },
		{ "marine_sw", "marine5" },
		{ "marine_medic", "marine6" },
		{ "marine_tech", "marine7" },
	};

	fieldtype_t DynamicPropertyDataType( const char *szPropertyName );

	// Data extracted from the Steam Inventory Service Schema.
	struct ItemDef_t
	{
		SteamItemDef_t ID;
		CUtlString ItemSlot;
		int EquipIndex{ -1 };
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
		CUtlString NotificationName[RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS_PER_ACCESSORY];
		Color BackgroundColor;
		Color NameColor;
		int64_t StrangeNotifyEvery{ 0 };
		CUtlVector<int64_t> StrangeNotify;
		CUtlStringList NotificationTags; // for notification items
		bool AfterDescriptionOnlyMultiStack : 1;
		bool HasInGameDescription : 1;
		bool HasBorder : 1;
		bool IsBasic : 1;
		bool GameOnly : 1;
		bool AutoStack : 1;
		bool IsTagTool : 1;
#ifdef CLIENT_DLL
		vgui::IImage *Icon{};
		CUtlVector<vgui::IImage *> StyleIcons{};
		vgui::IImage *AccessoryImage{};
		mutable ITexture *AccessoryIcon{};
		ITexture *GetAccessoryIcon() const;
#endif

		bool ItemSlotMatches( const char *szRequiredSlot ) const;
		int64_t CountForStrangeTier( int iTier ) const;
		int GetStrangeTier( int64_t x ) const;
		bool HasNotificationTag( const char *szTag ) const;
	};

	// Data extracted from SteamInventoryResult_t; it is safe to destroy the result after constructing this data type.
	struct ItemInstance_t
	{
		CSteamID AccountID{};
		SteamItemInstanceID_t ItemID{ k_SteamItemInstanceIDInvalid };
		SteamItemInstanceID_t OriginalItemID{ k_SteamItemInstanceIDInvalid };
		SteamItemDef_t ItemDefID{};
		int32 Quantity{};
		RTime32 Acquired{};
		RTime32 StateChangedTimestamp{};
		// "", "consumed"
		CUtlString State{};
		// "external", "promo", "playtime", "purchase", "exchange"
		CUtlString Origin{};
		CUtlStringMap<CUtlString> DynamicProps{};
		CUtlStringMap<CUtlStringList> Tags{};

		ItemInstance_t() {}
		ItemInstance_t( const ItemInstance_t &other )
		{
			*this = other;
		}
		explicit ItemInstance_t( SteamInventoryResult_t hResult, uint32 index );
		explicit ItemInstance_t( KeyValues *pKV );
#ifdef CLIENT_DLL
		void FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc, bool bIsSteamCommunityDesc ) const;
		void FormatDescription( vgui::MultiFontRichText *pRichText, bool bIncludeAccessories = true, Color descriptionColor = rd_briefing_item_details_color1.GetColor(), Color beforeAfterColor = rd_briefing_item_details_color2.GetColor() ) const;
		vgui::IImage *GetIcon() const;
#endif
		int GetStyle() const;
		KeyValues *ToKeyValues() const;
		void FromKeyValues( KeyValues *pKV );

		ItemInstance_t &operator=( const ItemInstance_t &other );
	};

	const ItemDef_t *GetItemDef( SteamItemDef_t id );
	bool DecodeItemData( SteamInventoryResult_t &hResult, const char *szEncodedData );
	bool ValidateItemData( bool &bValid, SteamInventoryResult_t hResult, const char *szRequiredSlot = NULL, CSteamID requiredSteamID = k_steamIDNil, bool bRequireFresh = false );
#ifdef CLIENT_DLL
	void AddPromoItem( SteamItemDef_t id );
	void RequestGenericPromoItems();
	void CheckPlaytimeItemGenerators();
	void IncrementStrangePropertyOnStartingItems( SteamItemDef_t iAccessoryID, int64_t iAmount, int iPropertyIndex = 0, bool bRelative = true, bool bAllowCheating = false );
	void CommitDynamicProperties();
	const ItemInstance_t *GetLocalItemCache( SteamItemInstanceID_t id );
	void GetLocalItemCache( CUtlVector<ItemInstance_t> &items );
	ItemInstance_t *GetLocalItemCacheForModify( SteamItemInstanceID_t id );
	void GetItemsForSlot( CUtlVector<ItemInstance_t> &instances, const char *szRequiredSlot );
	void GetItemsForSlotAndEquipIndex( CUtlVector<ItemInstance_t> &instances, const char *szRequiredSlot, int iEquipIndex );
	void GetItemsForDef( CUtlVector<ItemInstance_t> &instances, SteamItemDef_t iDefID );
	void GetItemsForTagTool( CUtlVector<ItemInstance_t> &instances, const char *szCategory, const char *szID );
	bool CheckMedalEquipCache();
	void ChangeItemStyle( SteamItemInstanceID_t id, int iStyle );
	void QueueSetNotificationSeen( SteamItemInstanceID_t id, int iSeen );
	void CommitNotificationSeen();
	void DeleteNotificationItem( SteamItemInstanceID_t id );
#ifdef RD_7A_DROPS
	void PickUpCraftingMaterialAtLocation( int iLocation, RD_Crafting_Material_t eMaterial );
	int GetCraftingMaterialsFound();
	int GetCraftingMaterialsMissed();
#endif
	enum CraftItemType_t
	{
		// craft an item via a predefined recipe. notifies user when complete. modal while in progress.
		CRAFT_RECIPE,
		// attach an accessory to an item. modal while in progress. updates equip slots to use new ID. forces a dynamic property update for the accessory.
		CRAFT_ACCESSORY,
		// convert currency to items or items to currency. modal while in progress.
		CRAFT_SHOP,
		// claiming a currency reward. modal while in progress.
		CRAFT_CLAIM_MINOR,
		// claiming an item or bundle reward. modal while in progress. notifies user when complete.
		CRAFT_CLAIM_MAJOR,
		// picking up a crafting material. silent while in progress. in-game notification when complete.
		CRAFT_PICKUP_MATERIAL,
		// behind-the-scenes item exchange. no notification.
		CRAFT_BTS,
		// set dynamic properties on newly created item to 0. modal while in progress. notifies user when complete (replaces notification from craft task that queued this).
		CRAFT_DYNAMIC_PROPERTY_INIT,
		// checking for item drop. notifies user based on preferences if successful.
		CRAFT_DROP,
		// checking for promo item. notifies user and server if successful.
		CRAFT_PROMO,
		// retrieving item data. may not be ours. notification when complete.
		CRAFT_INSPECT,
		// updating dynamic properties for an item (eg. at the end of a mission). no notifications.
		CRAFT_DYNAMIC_PROPERTY_UPDATE,
		// updating user-modifiable dynamic properties for an item (eg. style). modal while in progress.
		CRAFT_USER_DYNAMIC_PROPERTY_UPDATE,
		// deleting an item. modal while in progress.
		CRAFT_DELETE,
		// deleting an item silently. no notifications. (funnily enough, this is used to expire a different kind of notifications.)
		CRAFT_DELETE_SILENT,
		// updating the "seen" status of notifications. only one of these exist at a time. no notifications. (heh)
		CRAFT_NOTIFICATION_DYNAMIC_PROPERTY_UPDATE,
	};
	void PerformCraftingAction( CraftItemType_t eCraftType, SteamItemDef_t recipe, std::initializer_list<SteamItemInstanceID_t> ingredient, std::initializer_list<uint32> quantity, SteamItemDef_t iAccessoryDef = 0, SteamItemInstanceID_t iReplaceItemInstance = k_SteamItemInstanceIDInvalid );
	void RequestFullInventoryRefresh();
#endif
	void OnHitConfirm( CBaseEntity *pAttacker, CBaseEntity *pTarget, Vector vecDamagePosition, bool bKilled, bool bDamageOverTime, bool bBlastDamage, int iDisposition, float flDamage, CBaseEntity *pWeapon );
}

#define RD_ITEM_ID_BITS 30
#define RD_ITEM_ACCESSORY_BITS 13

#ifdef CLIENT_DLL
EXTERN_RECV_TABLE( DT_RD_ItemInstance );
EXTERN_RECV_TABLE( DT_RD_ItemInstances_Player );
EXTERN_RECV_TABLE( DT_RD_ItemInstances_Marine_Resource );
EXTERN_RECV_TABLE( DT_RD_ProjectileData );
#else
EXTERN_SEND_TABLE( DT_RD_ItemInstance );
EXTERN_SEND_TABLE( DT_RD_ItemInstances_Player );
EXTERN_SEND_TABLE( DT_RD_ItemInstances_Marine_Resource );
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
	void FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc, bool bIsSteamCommunityDesc ) const;
#ifdef CLIENT_DLL
	void FormatDescription( vgui::MultiFontRichText *pRichText, bool bIncludeAccessories = true, Color descriptionAfterColor = rd_briefing_item_details_color1.GetColor(), Color beforeAfterColor = rd_briefing_item_details_color2.GetColor() ) const;
	vgui::IImage *GetIcon() const;
#endif
	int GetStyle() const;

	CNetworkVar( SteamItemInstanceID_t, m_iItemInstanceID );
	CNetworkVar( SteamItemDef_t, m_iItemDefID );
	CNetworkArray( SteamItemDef_t, m_iAccessory, RD_ITEM_MAX_ACCESSORIES );
	CNetworkArray( int64, m_nCounter, RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS );
#ifndef CLIENT_DLL
	CNetworkArray( int64, m_nCounterCommitted, RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS );
	void CommitCounters();
#endif
};

class CRD_ItemInstances_Player
{
public:
	DECLARE_CLASS_NOBASE( CRD_ItemInstances_Player );
	DECLARE_EMBEDDED_NETWORKVAR();

	CNetworkVarEmbedded( CRD_ItemInstance, m_Medal0 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Medal1 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Medal2 );

	CRD_ItemInstance &operator []( int index );
};

class CRD_ItemInstances_Marine_Resource
{
public:
	DECLARE_CLASS_NOBASE( CRD_ItemInstances_Marine_Resource );
	DECLARE_EMBEDDED_NETWORKVAR();

	CNetworkVarEmbedded( CRD_ItemInstance, m_Suit );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Weapon1 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Weapon2 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Extra );

	CRD_ItemInstance &operator []( int index );
};

class CRD_ProjectileData
{
public:
	DECLARE_CLASS_NOBASE( CRD_ProjectileData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CRD_ProjectileData();

	CNetworkHandle( CASW_Marine_Resource, m_hOriginalOwnerMR );
	CNetworkVar( int, m_iInventoryEquipSlot );
	CNetworkVar( bool, m_bFiredByOwner );

	bool IsInventoryEquipSlotValid() const { return !!m_hOriginalOwnerMR && m_iInventoryEquipSlot != 0; }

#ifdef GAME_DLL
	void SetFromWeapon( CBaseEntity *pCreator );
#endif
};

abstract_class IRD_Has_Projectile_Data
{
public:
	virtual const CRD_ProjectileData *GetProjectileData() const = 0;
};

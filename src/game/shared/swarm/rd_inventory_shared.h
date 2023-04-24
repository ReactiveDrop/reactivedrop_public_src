#pragma once

#include "asw_shareddefs.h"
#include "steam/steam_api.h"

#ifdef CLIENT_DLL
namespace vgui
{
	class IImage;
	class RichText;
}

#define CASW_Player C_ASW_Player
#define CRD_ItemInstance C_RD_ItemInstance
#define CRD_ItemInstances_Static C_RD_ItemInstances_Static
#define CRD_ItemInstances_Dynamic C_RD_ItemInstances_Dynamic
#define CRD_ProjectileData C_RD_ProjectileData
#endif

class CASW_Player;

namespace ReactiveDropInventory
{
	constexpr const char *const g_InventorySlotNames[] =
	{
		"medal", "medal1", "medal2",
		"marine0", "marine1", "marine2", "marine3",
		"marine4", "marine5", "marine6", "marine7",
	};
#define RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_STATIC NELEMS( ReactiveDropInventory::g_InventorySlotNames )
#define RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_DYNAMIC ( ASW_NUM_MARINE_PROFILES * ASW_NUM_INVENTORY_SLOTS )
#define RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS 3
#define RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL 0
#define RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MARINE 3
#define RD_STEAM_INVENTORY_ITEM_MAX_STYLES 64
	constexpr const char *const g_InventorySlotAliases[][2] =
	{
		// first value is the name from g_InventorySlotNames, second value is the value to also accept from item definitions.
		{ "medal1", "medal" },
		{ "medal2", "medal" },
	};

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
		Color BackgroundColor;
		Color NameColor;
		bool AfterDescriptionOnlyMultiStack : 1;
		bool HasInGameDescription : 1;
		bool HasBorder : 1;
#ifdef CLIENT_DLL
		vgui::IImage *Icon{};
		CUtlVector<vgui::IImage *> StyleIcons{};
		mutable ITexture *AccessoryIcon{};
		ITexture *GetAccessoryIcon() const;
#endif

		bool ItemSlotMatches( const char *szRequiredSlot ) const;
		bool ItemSlotMatchesAnyDynamic() const;
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

		ItemInstance_t() {}
		ItemInstance_t( const ItemInstance_t &other )
		{
			*this = other;
		}
		explicit ItemInstance_t( SteamInventoryResult_t hResult, uint32 index );
		explicit ItemInstance_t( KeyValues *pKV );
		void FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc, bool bIsSteamCommunityDesc ) const;
#ifdef CLIENT_DLL
		void FormatDescription( vgui::RichText *pRichText ) const;
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
	bool ValidateEquipItemDataStatic( bool &bValid, SteamInventoryResult_t hResult, byte( &nIndex )[RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_STATIC], CSteamID requiredSteamID );
	bool ValidateEquipItemDataDynamic( bool &bValid, SteamInventoryResult_t hResult, byte( &nIndex )[RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_DYNAMIC], CSteamID requiredSteamID, CASW_Player *pPlayer );
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

#define RD_EQUIPPED_ITEMS_NOTIFICATION_WORST_CASE_SIZE ( MAX( RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_STATIC, RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_DYNAMIC ) * 2048 )
#define RD_EQUIPPED_ITEMS_NOTIFICATION_PAYLOAD_SIZE_PER_PACKET ( MAX_VALUE / 2 - 1 )

#ifdef CLIENT_DLL
EXTERN_RECV_TABLE( DT_RD_ItemInstance );
EXTERN_RECV_TABLE( DT_RD_ItemInstances_Static );
EXTERN_RECV_TABLE( DT_RD_ItemInstances_Dynamic );
EXTERN_RECV_TABLE( DT_RD_ProjectileData );

namespace vgui
{
	class RichText;
}
#else
EXTERN_SEND_TABLE( DT_RD_ItemInstance );
EXTERN_SEND_TABLE( DT_RD_ItemInstances_Static );
EXTERN_SEND_TABLE( DT_RD_ItemInstances_Dynamic );
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
	static void AppendBBCode( vgui::RichText *pRichText, const wchar_t *wszBuf, Color defaultColor );
	void FormatDescription( vgui::RichText *pRichText ) const;
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

class CRD_ItemInstances_Static
{
public:
	DECLARE_CLASS_NOBASE( CRD_ItemInstances_Static );
	DECLARE_EMBEDDED_NETWORKVAR();

	CNetworkVarEmbedded( CRD_ItemInstance, m_Medal0 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Medal1 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Medal2 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Marine0 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Marine1 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Marine2 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Marine3 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Marine4 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Marine5 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Marine6 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Marine7 );

	CRD_ItemInstance &operator []( int index );
};

class CRD_ItemInstances_Dynamic
{
public:
	DECLARE_CLASS_NOBASE( CRD_ItemInstances_Dynamic );
	DECLARE_EMBEDDED_NETWORKVAR();

	CNetworkVarEmbedded( CRD_ItemInstance, m_Item0 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item1 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item2 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item3 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item4 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item5 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item6 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item7 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item8 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item9 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item10 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item11 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item12 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item13 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item14 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item15 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item16 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item17 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item18 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item19 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item20 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item21 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item22 );
	CNetworkVarEmbedded( CRD_ItemInstance, m_Item23 );

	CRD_ItemInstance &operator []( int index );
};

class CRD_ProjectileData
{
public:
	DECLARE_CLASS_NOBASE( CRD_ProjectileData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CRD_ProjectileData();

	CNetworkHandle( CASW_Player, m_hOriginalOwnerPlayer );
	CNetworkVar( int, m_iInventoryEquipSlot );
	CNetworkVar( bool, m_bFiredByOwner );

	bool IsInventoryEquipSlotValid() const { return !!m_hOriginalOwnerPlayer && m_iInventoryEquipSlot != -1; }

#ifdef GAME_DLL
	void SetFromWeapon( CBaseEntity *pCreator );
#endif
};

abstract_class IRD_Has_Projectile_Data
{
public:
	virtual const CRD_ProjectileData *GetProjectileData() const = 0;
};

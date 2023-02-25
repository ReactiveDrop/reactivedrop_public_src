#pragma once

#include "steam/steam_api.h"

#ifdef CLIENT_DLL
namespace vgui
{
	class IImage;
	class RichText;
}

#define CRD_ItemInstance C_RD_ItemInstance
#endif

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
		"weapon25", "weapon26",
		"extra0", "extra1", "extra2", "extra3", "extra4",
		"extra5", "extra6", "extra7", "extra8", "extra9",
		"extra10", "extra11", "extra12", "extra13", "extra14",
		"extra15", "extra16", "extra17",
	};
#define RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS NELEMS( ReactiveDropInventory::g_InventorySlotNames )

	// Data extracted from the Steam Inventory Service Schema.
	struct ItemDef_t
	{
		SteamItemDef_t ID;
		CUtlString ItemSlot;
		CUtlStringMap<CUtlStringList> Tags;
		CUtlStringMap<CUtlStringList> AllowedTagsFromTools;
		CUtlString AccessoryTag;
		CUtlStringList CompressedDynamicProps;
		CUtlString DisplayType;
		CUtlString Name;
		CUtlString BriefingName;
		CUtlString Description;
		CUtlString BeforeDescription;
		CUtlString AfterDescription;
		CUtlString AccessoryDescription;
		Color BackgroundColor;
		Color NameColor;
		bool AfterDescriptionOnlyMultiStack : 1;
		bool HasIngameDescription : 1;
		bool HasBorder : 1;
#ifdef CLIENT_DLL
		vgui::IImage *Icon{};
		vgui::IImage *IconSmall{};
#endif
	};

	// Data extracted from SteamInventoryResult_t; it is safe to destroy the result after constructing this data type.
	struct ItemInstance_t
	{
		CSteamID AccountID{};
		SteamItemInstanceID_t ItemID{};
		SteamItemInstanceID_t OriginalItemID{};
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
#endif
		KeyValues *ToKeyValues() const;
		void FromKeyValues( KeyValues *pKV );
	};

	const ItemDef_t *GetItemDef( SteamItemDef_t id );
	bool DecodeItemData( SteamInventoryResult_t &hResult, const char *szEncodedData );
	bool ValidateItemData( bool &bValid, SteamInventoryResult_t hResult, const char *szRequiredSlot = NULL, CSteamID requiredSteamID = k_steamIDNil, bool bRequireFresh = false );
#ifdef CLIENT_DLL
	void AddPromoItem( SteamItemDef_t id );
	void RequestGenericPromoItems();
	void CheckPlaytimeItemGenerators( int iMarineClass );
#endif
}

#define RD_ITEM_MAX_ACCESSORIES 4
#define RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS 8
#define RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS_PER_ACCESSORY 2

#ifdef CLIENT_DLL
EXTERN_RECV_TABLE( DT_RD_ItemInstance );

namespace vgui
{
	class RichText;
}
#else
EXTERN_SEND_TABLE( DT_RD_ItemInstance );
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
#endif

	CNetworkVar( SteamItemDef_t, m_iItemDefID );
	CNetworkArray( SteamItemDef_t, m_iAccessory, RD_ITEM_MAX_ACCESSORIES );
	CNetworkArray( int64, m_nCounter, RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS );
};

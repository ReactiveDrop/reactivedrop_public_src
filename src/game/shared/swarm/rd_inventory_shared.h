#pragma once

#include "steam/steam_api.h"

#ifdef CLIENT_DLL
namespace vgui
{
	class IImage;
}

#define CRD_ItemInstance C_RD_ItemInstance
#endif

namespace ReactiveDropInventory
{
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
		bool AfterDescriptionOnlyMultiStack : 1;
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
		void FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc );
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

	void Reset();
	bool IsSet() const;
	void SetFromInstance( const ReactiveDropInventory::ItemInstance_t &instance );
	void FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc ) const;
#ifdef CLIENT_DLL
	void FormatDescription( vgui::RichText *pRichText ) const;
#endif

	CNetworkVar( SteamItemDef_t, m_iItemDefID );
	CNetworkVar( int32, m_nQuantity );
	CNetworkArray( SteamItemDef_t, m_iAccessory, RD_ITEM_MAX_ACCESSORIES );
	CNetworkArray( int64, m_nCounter, RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS );
};

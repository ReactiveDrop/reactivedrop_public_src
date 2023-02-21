#pragma once

#include "steam/steam_api.h"

#ifdef CLIENT_DLL
namespace vgui
{
	class IImage;
}
#endif

namespace ReactiveDropInventory
{
	struct ItemDef_t
	{
		SteamItemDef_t ID;
		CUtlString ItemSlot;
		CUtlStringMap<CUtlStringList> Tags;
		CUtlString DisplayType;
		CUtlString Name;
		CUtlString BriefingName;
		CUtlString Description;
		CUtlString BeforeDescription;
		CUtlString AfterDescription;
		CUtlString AccessoryDescription;
		bool AfterDescriptionOnlyMultiStack : 1;
#ifdef CLIENT_DLL
		vgui::IImage *Icon;
		vgui::IImage *IconSmall;
#endif
	};

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
	SteamItemDetails_t GetItemDetails( SteamInventoryResult_t hResult, uint32_t index );
#ifdef CLIENT_DLL
	void AddPromoItem( SteamItemDef_t id );
	void RequestGenericPromoItems();
	void CheckPlaytimeItemGenerators( int iMarineClass );
#endif
}

#pragma once

#include "steam/steam_api.h"

namespace ReactiveDropInventory
{
	struct ItemDef_t
	{
		SteamItemDef_t ID;
		CUtlString ItemSlot;
		CUtlString Tags;
		CUtlString DisplayType;
		CUtlString Name;
		CUtlString Description;
		CUtlString BeforeDescription;
		CUtlString AfterDescription;
	};

	const ItemDef_t *GetItemDef( SteamItemDef_t id );
	void FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc, SteamInventoryResult_t hResult, uint32_t index );
	bool DecodeItemData( SteamInventoryResult_t &hResult, const char *szEncodedData, const char *szRequiredSlot = NULL, CSteamID requiredSteamID = k_steamIDNil, bool bRequireFresh = false );
	SteamItemDef_t GetItemID( SteamInventoryResult_t hResult, uint32_t index );
}

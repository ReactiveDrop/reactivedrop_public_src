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
		CUtlString Tags;
		CUtlString DisplayType;
		CUtlString Name;
		CUtlString BriefingName;
		CUtlString Description;
		CUtlString BeforeDescription;
		CUtlString AfterDescription;
#ifdef CLIENT_DLL
		vgui::IImage *Icon;
#endif
	};

	const ItemDef_t *GetItemDef( SteamItemDef_t id );
	void FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc, SteamInventoryResult_t hResult, uint32_t index );
	bool DecodeItemData( SteamInventoryResult_t &hResult, const char *szEncodedData );
	bool ValidateItemData( bool &bValid, SteamInventoryResult_t hResult, const char *szRequiredSlot = NULL, CSteamID requiredSteamID = k_steamIDNil, bool bRequireFresh = false );
	SteamItemDetails_t GetItemDetails( SteamInventoryResult_t hResult, uint32_t index );
}

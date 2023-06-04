#pragma once

#include "asw_equipment_list.h"
#include "rd_inventory_shared.h"

namespace ReactiveDropLoadout
{
	struct LoadoutMarineData_t
	{
		ASW_Equip_Regular Primary{ ASW_EQUIP_RIFLE };
		ASW_Equip_Regular Secondary{ ASW_EQUIP_RIFLE };
		ASW_Equip_Extra Extra{ ASW_EQUIP_MEDKIT };
		SteamItemInstanceID_t Suit{ k_SteamItemInstanceIDInvalid };
		SteamItemInstanceID_t PrimaryItem{ k_SteamItemInstanceIDInvalid };
		SteamItemInstanceID_t SecondaryItem{ k_SteamItemInstanceIDInvalid };
		SteamItemInstanceID_t ExtraItem{ k_SteamItemInstanceIDInvalid };

		void ToKeyValues( KeyValues *pKV, bool bBinary ) const;
		void FromKeyValues( KeyValues *pKV, bool bBinary );
#ifdef CLIENT_DLL
		void CopyToLive( ASW_Marine_Profile id ) const;
		void CopyFromLive( ASW_Marine_Profile id );
		bool ReplaceItemID( SteamItemInstanceID_t oldID, SteamItemInstanceID_t newID, const char *szDebugLoadoutName, const char *szDebugMarineName );
#endif
	};
	struct LoadoutData_t
	{
		SteamItemInstanceID_t Medals[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS]{ k_SteamItemInstanceIDInvalid, k_SteamItemInstanceIDInvalid, k_SteamItemInstanceIDInvalid };
		LoadoutMarineData_t Marines[ASW_NUM_MARINE_PROFILES]{};
		bool MarineIncluded[ASW_NUM_MARINE_PROFILES]{};
		uint32 LastModified{};
		uint32 LastUsed{};

		void ToKeyValues( KeyValues *pKV, bool bBinary ) const;
		void FromKeyValues( KeyValues *pKV, bool bBinary );
#ifdef CLIENT_DLL
		void CopyToLive();
		void CopyFromLive();
		bool ReplaceItemID( SteamItemInstanceID_t oldID, SteamItemInstanceID_t newID, const char *szDebugLoadoutName );
#endif
	};
	extern const LoadoutData_t DefaultLoadout;

#ifdef CLIENT_DLL
	// saved loadout names will be appended to the list
	void List( CUtlStringList &names );
	// returns true if the named loadout already existed and was replaced
	bool Save( const char *szName );
	// returns true if the loadout was found and loaded
	bool Load( const char *szName );
	// returns true if the loadout was found and deleted
	bool Delete( const char *szName );
	// replaces all instances of the item ID in loadouts (saved and active)
	void ReplaceItemID( SteamItemInstanceID_t oldID, SteamItemInstanceID_t newID );
#endif
}

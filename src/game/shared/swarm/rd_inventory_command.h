#pragma once

#include "steam/isteaminventory.h"

enum EInventoryCommand
{
	INVCMD_PLAYER_EQUIPS,
	INVCMD_MARINE_RESOURCE_EQUIPS,
	INVCMD_MATERIAL_SPAWN,
	INVCMD_MATERIAL_PICKUP,
	INVCMD_PROMO_DROP,
};

// this can be any odd number up to 511; the trade-off is
// lower value = more round trips 
// higher value = higher chance of overflowing reliable buffer and disconnecting
#define RD_INVCMD_PAYLOAD_CHUNK_STRSIZE 257

#ifdef CLIENT_DLL
int UTIL_RD_SendInventoryCommand( EInventoryCommand eCmd, const CUtlVector<int> &args, SteamInventoryResult_t hResult );
void UTIL_RD_SendInventoryCommandByIDs( EInventoryCommand eCmd, const CUtlVector<SteamItemInstanceID_t> &ids, const CUtlVector<int> &args = CUtlVector<int>{} );
void UTIL_RD_SendInventoryCommandOffline( EInventoryCommand eCmd, const CUtlVector<int> &args, const CUtlVector<SteamItemInstanceID_t> &items );
void UTIL_RD_AbortInventoryCommand( int iCommandID );
#endif

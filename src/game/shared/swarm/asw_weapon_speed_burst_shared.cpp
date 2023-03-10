#include "cbase.h"
#include "asw_weapon_speed_burst_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Speed_Burst, DT_ASW_Weapon_Speed_Burst );

BEGIN_NETWORK_TABLE( CASW_Weapon_Speed_Burst, DT_ASW_Weapon_Speed_Burst )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Speed_Burst )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_speed_burst, CASW_Weapon_Speed_Burst );
PRECACHE_WEAPON_REGISTER( asw_weapon_speed_burst );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Speed_Burst )
END_DATADESC()
#endif
#endif

#include "cbase.h"
#include "asw_weapon_plasma_thrower_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Plasma_Thrower, DT_ASW_Weapon_Plasma_Thrower );

BEGIN_NETWORK_TABLE( CASW_Weapon_Plasma_Thrower, DT_ASW_Weapon_Plasma_Thrower )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Plasma_Thrower )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_plasma_thrower, CASW_Weapon_Plasma_Thrower );
PRECACHE_WEAPON_REGISTER( asw_weapon_plasma_thrower );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Plasma_Thrower )
END_DATADESC()
#endif
#endif

#include "cbase.h"
#include "asw_weapon_incendiary_grenades_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Incendiary_Grenades, DT_ASW_Weapon_Incendiary_Grenades );

BEGIN_NETWORK_TABLE( CASW_Weapon_Incendiary_Grenades, DT_ASW_Weapon_Incendiary_Grenades )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Incendiary_Grenades )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_incendiary_grenades, CASW_Weapon_Incendiary_Grenades );
PRECACHE_WEAPON_REGISTER( asw_weapon_incendiary_grenades );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Incendiary_Grenades )
END_DATADESC()
#endif
#endif

#include "cbase.h"
#include "asw_weapon_cryo_cannon_shared.h"
#ifdef CLIENT_DLL
#include "c_asw_extinguisher_projectile.h"
#else
#include "asw_extinguisher_projectile.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Cryo_Cannon, DT_ASW_Weapon_Cryo_Cannon );

BEGIN_NETWORK_TABLE( CASW_Weapon_Cryo_Cannon, DT_ASW_Weapon_Cryo_Cannon )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Cryo_Cannon )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_cryo_cannon, CASW_Weapon_Cryo_Cannon );
PRECACHE_WEAPON_REGISTER( asw_weapon_cryo_cannon );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Cryo_Cannon )
END_DATADESC()
#endif

void CASW_Weapon_Cryo_Cannon::Precache()
{
#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "asw_extinguisher_projectile" );
#endif

	BaseClass::Precache();
}

// TODO: m_iCryoCannonFreezeAlien
#endif

#include "cbase.h"
#include "asw_weapon_hack_tool_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Hack_Tool, DT_ASW_Weapon_Hack_Tool );

BEGIN_NETWORK_TABLE( CASW_Weapon_Hack_Tool, DT_ASW_Weapon_Hack_Tool )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Hack_Tool )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_hack_tool, CASW_Weapon_Hack_Tool );
PRECACHE_WEAPON_REGISTER( asw_weapon_hack_tool );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Hack_Tool )
END_DATADESC()
#endif

// TODO: m_iHackToolWireHacksTech
// TODO: m_iHackToolWireHacksOther
// TODO: m_iHackToolComputerHacksTech
// TODO: m_iHackToolComputerHacksOther
#endif

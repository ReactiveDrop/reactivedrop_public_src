#include "cbase.h"
#include "asw_weapon_shield_bubble_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Shield_Bubble, DT_ASW_Weapon_Shield_Bubble );

BEGIN_NETWORK_TABLE( CASW_Weapon_Shield_Bubble, DT_ASW_Weapon_Shield_Bubble )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Shield_Bubble )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_shield_bubble, CASW_Weapon_Shield_Bubble );
PRECACHE_WEAPON_REGISTER( asw_weapon_shield_bubble );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Shield_Bubble )
END_DATADESC()
#endif

// TODO: m_iShieldBubblePushedEnemy
// TODO: m_iShieldBubbleDamageAbsorbed
#endif

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

void CASW_Weapon_Shield_Bubble::Precache()
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "asw_shieldgrenade_projectile" );
#endif
}

bool CASW_Weapon_Shield_Bubble::OffhandActivate()
{
#ifdef GAME_DLL
	CBaseEntity *pProp = CreateEntityByName( "prop_dynamic" );
	pProp->SetModelName( AllocPooledString( "models/items/shield_generator/shield_generator.mdl" ) );
	pProp->Precache();
	pProp->SetAbsOrigin( GetMoveParent()->GetAbsOrigin() );
	pProp->Spawn();
	pProp->SetMoveType( MOVETYPE_NONE );
	pProp->AddEffects( EF_NOSHADOW );
	CBaseEntity *pProp2 = CreateEntityByName( "prop_dynamic" );
	pProp2->SetModelName( AllocPooledString( "models/items/shield_bubble/shield_bubble_arena.mdl" ) );
	pProp2->Precache();
	pProp2->SetAbsOrigin( GetMoveParent()->GetAbsOrigin() );
	pProp2->Spawn();
	pProp2->SetMoveType( MOVETYPE_NONE );
	pProp2->AddEffects( EF_NOSHADOW );
	UTIL_Remove( this );
#endif
	return true;
}

// TODO: m_iShieldBubblePushedEnemy
// TODO: m_iShieldBubbleDamageAbsorbed
#endif

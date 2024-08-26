#include "cbase.h"
#include "asw_shieldgrenade_projectile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define SHIELD_GRENADE_MODEL "models/items/shield_generator/shield_generator.mdl"

LINK_ENTITY_TO_CLASS( asw_shieldgrenade_projectile, CASW_ShieldGrenade_Projectile );

IMPLEMENT_SERVERCLASS_ST( CASW_ShieldGrenade_Projectile, DT_ASW_ShieldGrenade_Projectile )
END_SEND_TABLE()

BEGIN_DATADESC( CASW_ShieldGrenade_Projectile )
END_DATADESC()

CASW_ShieldGrenade_Projectile::CASW_ShieldGrenade_Projectile()
{
	SetModelName( MAKE_STRING( SHIELD_GRENADE_MODEL ) );
}

void CASW_ShieldGrenade_Projectile::Precache()
{
	BaseClass::Precache();

	PrecacheModel( "models/items/shield_bubble/shield_bubble_arena.mdl" );
	PrecacheModel( "models/items/shield_bubble/shield_bubble_arena_low.mdl" );
}

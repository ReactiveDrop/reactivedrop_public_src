#include "cbase.h"
#include "c_asw_buffgrenade_projectile.h"
#include "c_asw_marine.h"
#include "c_asw_weapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//Precahce the effects
PRECACHE_REGISTER_BEGIN( GLOBAL, ASWPrecacheEffectBuffGrenades )
	PRECACHE( MATERIAL, "swarm/effects/blueflare" )
	PRECACHE( MATERIAL, "effects/yellowflare" )
	PRECACHE( MATERIAL, "effects/yellowflare_noz" )
PRECACHE_REGISTER_END()

IMPLEMENT_CLIENTCLASS_DT( C_ASW_BuffGrenade_Projectile, DT_ASW_BuffGrenade_Projectile, CASW_BuffGrenade_Projectile )
END_RECV_TABLE()


ConVar asw_buffgrenade( "asw_buffgrenade", "98 34 16", 0, "Color of grenades" );


Color C_ASW_BuffGrenade_Projectile::GetGrenadeColor( void )
{
	return asw_buffgrenade.GetColor();
}

int C_ASW_BuffGrenade_Projectile::GetArcEffectIndex( C_BaseEntity *pEnt )
{
	C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pEnt );
	if ( !pMarine )
		return 0;

	C_ASW_Weapon *pWeapon = pMarine->GetActiveASWWeapon();
	return pWeapon && !pWeapon->SupportsDamageModifiers() ? 1 : 0;
}

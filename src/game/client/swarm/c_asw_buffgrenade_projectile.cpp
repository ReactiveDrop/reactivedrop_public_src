#include "cbase.h"
#include "c_asw_buffgrenade_projectile.h"
#include "c_asw_marine.h"
#include "c_asw_weapon.h"
#include "asw_marine_profile.h"

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
extern ConVar rd_buff_grenade_attach_sw;


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

bool C_ASW_BuffGrenade_Projectile::IsUsable( C_BaseEntity *pUser )
{
	if ( GetMoveParent() && GetMoveParent()->IsInhabitableNPC() )
		return false;

	C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pUser );
	if ( !pMarine )
		return false;

	if ( pMarine->GetAbsOrigin().DistTo( GetAbsOrigin() ) >= ASW_MARINE_USE_RADIUS )
		return false;

	for ( C_BaseEntity *pEnt = pMarine->FirstMoveChild(); pEnt; pEnt = pEnt->NextMovePeer() )
	{
		if ( dynamic_cast< C_ASW_BuffGrenade_Projectile * >( pEnt ) )
		{
			// only one amp grenade attached to a marine (they don't stack anyway)
			return false;
		}
	}

	return pMarine->GetMarineProfile() && pMarine->GetMarineProfile()->GetMarineClass() == MARINE_CLASS_SPECIAL_WEAPONS && rd_buff_grenade_attach_sw.GetBool();
}

bool C_ASW_BuffGrenade_Projectile::GetUseAction( ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser )
{
	action.bShowUseKey = true;
	action.iUseIconTexture = g_ASWEquipmentList.GetEquipIconTexture( false, ASW_EQUIP_BUFF_GRENADE );
	action.fProgress = -1;
	action.bWideIcon = false;
	action.UseIconRed = 66;
	action.UseIconGreen = 142;
	action.UseIconBlue = 192;
	action.bShowUseKey = true;

	return true;
}

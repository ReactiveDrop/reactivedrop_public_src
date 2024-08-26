#include "cbase.h"
#include "c_asw_shieldgrenade_projectile.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define BUBBLE_MODEL "models/items/shield_bubble/shield_bubble_arena.mdl"
#define BUBBLE_MODEL_LOW "models/items/shield_bubble/shield_bubble_arena_low.mdl"

extern ConVar rd_simple_beacons;

IMPLEMENT_CLIENTCLASS_DT( C_ASW_ShieldGrenade_Projectile, DT_ASW_ShieldGrenade_Projectile, CASW_ShieldGrenade_Projectile )
END_RECV_TABLE()

static bool UseLowBubbleModel()
{
	return rd_simple_beacons.GetBool() || ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 92 );
}

void C_ASW_ShieldGrenade_Projectile::Precache()
{
	BaseClass::Precache();

	PrecacheModel( BUBBLE_MODEL );
	PrecacheModel( BUBBLE_MODEL_LOW );
}

void C_ASW_ShieldGrenade_Projectile::UpdatePingEffects()
{
	BaseClass::UpdatePingEffects();

	// Spawn a sphere that is different from the default aoegrenade sphere.
	if ( m_bSettled && m_hSphereModel.Get() == NULL )
	{
		C_BaseAnimating *pEnt = new C_BaseAnimating;
		if ( !pEnt )
		{
			Warning( "Error, couldn't create new C_BaseAnimating\n" );
			return;
		}
		if ( !pEnt->InitializeAsClientEntity( UseLowBubbleModel() ? BUBBLE_MODEL_LOW : BUBBLE_MODEL, false ) )
		{
			Warning( "Error, couldn't InitializeAsClientEntity\n" );
			pEnt->Release();
			return;
		}

		pEnt->SetParent( this );
		pEnt->SetLocalOrigin( Vector( 0, 0, 0 ) );
		pEnt->SetLocalAngles( QAngle( 0, 0, 0 ) );
		pEnt->SetSolid( SOLID_NONE );
		pEnt->SetSkin( GetSphereSkin() );
		pEnt->RemoveEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );

		m_hSphereModel = pEnt;
		m_flTimeCreated = gpGlobals->curtime;
	}
}

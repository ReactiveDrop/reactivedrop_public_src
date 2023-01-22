#include "cbase.h"
#ifdef CLIENT_DLL
#include "c_asw_inhabitable_npc.h"
#include "c_asw_weapon.h"
#include "c_asw_player.h"
#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
#else
#include "asw_inhabitable_npc.h"
#include "asw_weapon.h"
#include "asw_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar asw_alien_hurt_speed( "asw_alien_hurt_speed", "0.5", FCVAR_CHEAT | FCVAR_REPLICATED, "Fraction of speed to use when the alien is hurt after being shot" );
ConVar asw_alien_stunned_speed( "asw_alien_stunned_speed", "0.3", FCVAR_CHEAT | FCVAR_REPLICATED, "Fraction of speed to use when the alien is electrostunned" );
extern ConVar asw_controls;

bool CASW_Inhabitable_NPC::IsInhabited()
{
	return m_bInhabited;
}

CASW_Player *CASW_Inhabitable_NPC::GetCommander() const
{
	return m_Commander.Get();
}

CASW_Weapon *CASW_Inhabitable_NPC::GetActiveASWWeapon( void ) const
{
	return assert_cast< CASW_Weapon * >( GetActiveWeapon() );
}

CASW_Weapon *CASW_Inhabitable_NPC::GetASWWeapon( int i ) const
{
	return assert_cast< CASW_Weapon * >( GetWeapon( i ) );
}

bool CASW_Inhabitable_NPC::IsMovementFrozen()
{
	return GetFrozenAmount() > 0.5f;
}

bool CASW_Inhabitable_NPC::ShouldMoveSlow() const
{
	return ( gpGlobals->curtime < m_fHurtSlowMoveTime );
}

float CASW_Inhabitable_NPC::MaxSpeed()
{
	float flBaseSpeed = 300 * m_fSpeedScale;

	if ( ShouldMoveSlow() )
	{
		if ( m_bElectroStunned.Get() )
		{
			return flBaseSpeed * asw_alien_stunned_speed.GetFloat();
		}
		else
		{
			return flBaseSpeed * asw_alien_hurt_speed.GetFloat();
		}
	}

	return flBaseSpeed;
}

ASW_Controls_t CASW_Inhabitable_NPC::GetASWControls()
{
	if ( m_iControlsOverride >= 0 )
	{
		return ( ASW_Controls_t )m_iControlsOverride.Get();
	}

	return ( ASW_Controls_t )asw_controls.GetInt();
}

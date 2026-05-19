#include "cbase.h"
#include "env_sprite_asw_fade.h"
#include "asw_player.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_game_resource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( env_sprite_asw_fade, CEnv_Sprite_ASW_Fade );

BEGIN_DATADESC( CEnv_Sprite_ASW_Fade )
	DEFINE_KEYFIELD( m_nFadeOpacity, FIELD_CHARACTER, "fade_opacity" ),
	DEFINE_INPUT( m_bAllowFade, FIELD_BOOLEAN, "AllowFade" ),
	DEFINE_KEYFIELD( m_vecFadeOrigin, FIELD_VECTOR, "fade_origin" ),
	DEFINE_FIELD( m_fVisibleToMarine, FIELD_INTEGER ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "ShowToMarine", InputShowToMarine ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "HideFromMarine", InputHideFromMarine ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetVisibleForAllMarines", InputSetVisibleForAllMarines ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CEnv_Sprite_ASW_Fade, DT_Env_Sprite_ASW_Fade )
	SendPropInt( SENDINFO( m_nFadeOpacity ), 8, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bAllowFade ) ),
	SendPropVector( SENDINFO( m_vecFadeOrigin ) ),
	SendPropInt( SENDINFO( m_fVisibleToMarine ), ASW_MAX_MARINE_RESOURCES, SPROP_UNSIGNED ),
END_SEND_TABLE()

BEGIN_ENT_SCRIPTDESC( CEnv_Sprite_ASW_Fade, CBaseEntity, "" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetVisibleForMarine, "SetVisibleForMarine", "Make the sprite visible or invisible for a marine entity" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptIsVisibleForMarine, "IsVisibleForMarine", "Check whether a sprite is visible for a marine entity" )
END_SCRIPTDESC()

CEnv_Sprite_ASW_Fade::CEnv_Sprite_ASW_Fade()
{
	m_nFadeOpacity = 0;
	m_bAllowFade = true;
	m_vecFadeOrigin = vec3_origin;
	m_fVisibleToMarine = UINT32_MAX;
}

bool CEnv_Sprite_ASW_Fade::ShouldFade( CASW_Inhabitable_NPC *pNPC )
{
	if ( !m_bAllowFade || !pNPC || !pNPC->IsInhabited() || !pNPC->GetCommander() )
	{
		return false;
	}

	if ( pNPC->GetCommander()->GetASWControls() == ASWC_TOPDOWN )
	{
		return pNPC->GetAbsOrigin().z < m_vecFadeOrigin.GetZ();
	}

	return false;
}

int CEnv_Sprite_ASW_Fade::GetMarineIndex( CBaseEntity *pEnt, const char *szContext )
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pEnt );
	Assert( !szContext || pMarine );
	if ( !pMarine )
	{
		if ( this && szContext )
		{
			DevWarning( "%s: Entity given to %s is not a marine!\n", GetDebugName(), szContext );
		}
		return -1;
	}

	CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
	Assert( !szContext || pMR );
	if ( !pMR )
	{
		if ( this && szContext )
		{
			DevWarning( "%s: Marine given to %s does not have a resource (not a player marine?)\n", GetDebugName(), szContext );
		}
		return -1;
	}

	CASW_Game_Resource *pGameResource = ASWGameResource();
	Assert( !szContext || pGameResource );
	if ( !pGameResource )
	{
		if ( this && szContext )
		{
			DevWarning( "%s: %s called before game resource was created!\n", GetDebugName(), szContext );
		}
		return -1;
	}

	return pGameResource->GetMarineResourceIndex( pMR );
}

void CEnv_Sprite_ASW_Fade::InputShowToMarine( inputdata_t &inputdata )
{
	int iMR = GetMarineIndex( inputdata.value.Entity(), "input ShowToMarine" );
	if ( iMR == -1 )
	{
		return;
	}

	m_fVisibleToMarine.GetForModify() |= 1u << iMR;
}

void CEnv_Sprite_ASW_Fade::InputHideFromMarine( inputdata_t &inputdata )
{
	int iMR = GetMarineIndex( inputdata.value.Entity(), "input HideFromMarine" );
	if ( iMR == -1 )
	{
		return;
	}

	m_fVisibleToMarine.GetForModify() &= ~( 1u << iMR );
}

void CEnv_Sprite_ASW_Fade::InputSetVisibleForAllMarines( inputdata_t &inputdata )
{
	m_fVisibleToMarine = inputdata.value.Bool() ? UINT32_MAX : 0;
}

void CEnv_Sprite_ASW_Fade::ScriptSetVisibleForMarine( HSCRIPT hMarine, bool bVisible )
{
	CBaseEntity *pEnt = ToEnt( hMarine );

	int iMR = GetMarineIndex( pEnt, "vscript function SetVisibleForMarine" );
	if ( iMR == -1 )
	{
		return;
	}

	if ( bVisible )
	{
		m_fVisibleToMarine.GetForModify() |= 1u << iMR;
	}
	else
	{
		m_fVisibleToMarine.GetForModify() &= ~( 1u << iMR );
	}
}

bool CEnv_Sprite_ASW_Fade::ScriptIsVisibleForMarine( HSCRIPT hMarine, bool bConsiderPositionalFade )
{
	CBaseEntity *pEnt = ToEnt( hMarine );

	int iMR = GetMarineIndex( pEnt, nullptr );
	if ( iMR != -1 && ( m_fVisibleToMarine.Get() & ( 1u << iMR ) ) == 0 )
	{
		return false;
	}

	return !bConsiderPositionalFade || !pEnt || !pEnt->IsInhabitableNPC() || !ShouldFade( assert_cast<CASW_Inhabitable_NPC *>( pEnt ) );
}

#include "cbase.h"
#include "func_asw_fade.h"
#include "asw_shareddefs.h"
#include "asw_fade_proxy_shared.h"
#include "asw_inhabitable_npc.h"
#include "asw_player.h"
#include "prop_asw_fade.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( func_asw_fade, CFunc_ASW_Fade );

BEGIN_DATADESC( CFunc_ASW_Fade )
	DEFINE_FIELD( m_bHasProxies, FIELD_BOOLEAN ),
	DEFINE_KEYFIELD( m_nFadeOpacity, FIELD_CHARACTER, "fade_opacity" ),
	DEFINE_KEYFIELD( m_iCollideWithGrenades, FIELD_CHARACTER, "CollideWithGrenades" ),
	DEFINE_KEYFIELD( m_bCollideWithMarines, FIELD_BOOLEAN, "CollideWithMarines" ),
	DEFINE_INPUT( m_bAllowFade, FIELD_BOOLEAN, "AllowFade" ),
	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetCollideWithGrenades", SetGrenadeCollisionRules ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetCollideWithMarines", SetMarineCollisionRules ),
	DEFINE_FIELD( m_fVisibleToMarine, FIELD_INTEGER ),
	DEFINE_KEYFIELD( m_bSolidWhenInvisible, FIELD_BOOLEAN, "SolidWhenInvisible" ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "ShowToMarine", InputShowToMarine ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "HideFromMarine", InputHideFromMarine ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "SetVisibleForAllMarines", InputSetVisibleForAllMarines ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CFunc_ASW_Fade, DT_Func_ASW_Fade )
	SendPropInt( SENDINFO( m_nFadeOpacity ), 8, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bAllowFade ) ),
	SendPropBool( SENDINFO( m_bHasProxies ) ),
	SendPropBool( SENDINFO( m_bCollideWithMarines ) ),
	SendPropInt( SENDINFO( m_fVisibleToMarine ), ASW_MAX_MARINE_RESOURCES, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bSolidWhenInvisible ) ),
END_SEND_TABLE()

BEGIN_ENT_SCRIPTDESC( CFunc_ASW_Fade, CBaseAnimating, "" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptSetVisibleForMarine, "SetVisibleForMarine", "Make the brush visible or invisible for a marine entity" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptIsVisibleForMarine, "IsVisibleForMarine", "Check whether a brush is visible for a marine entity" )
END_SCRIPTDESC()

CFunc_ASW_Fade::CFunc_ASW_Fade()
{
	m_bHasProxies = false;
	m_iCollideWithGrenades = 0;
	m_bCollideWithMarines = true;
	m_nFadeOpacity = 0;
	m_bAllowFade = true;
	m_fVisibleToMarine = UINT32_MAX;
	m_bSolidWhenInvisible = false;
}

void CFunc_ASW_Fade::Spawn()
{
	BaseClass::Spawn();

	SetCollisionGroup( COLLISION_GROUP_NONE );
	Assert( m_iCollideWithGrenades <= 2 );
}

bool CFunc_ASW_Fade::ShouldFade( CASW_Inhabitable_NPC *pNPC )
{
	if ( !m_bAllowFade || !pNPC || !pNPC->IsInhabited() || !pNPC->GetCommander() )
	{
		return false;
	}

	if ( pNPC->GetCommander()->GetASWControls() == ASWC_TOPDOWN )
	{
		if ( m_bHasProxies )
		{
			Vector vecEyePosition = pNPC->EyePosition();

#ifdef DBGFLAG_ASSERT
			bool bAtLeastOneProxy = false;
#endif
			for ( CBaseEntity *pEnt = FirstMoveChild(); pEnt; pEnt = pEnt->NextMovePeer() )
			{
				CPoint_ASW_Fade_Proxy *pProxy = dynamic_cast<CPoint_ASW_Fade_Proxy *>( pEnt );
				if ( pProxy )
				{
					if ( pProxy->ShouldFade( vecEyePosition ) )
					{
						return true;
					}

#ifdef DBGFLAG_ASSERT
					bAtLeastOneProxy = true;
#endif
				}
			}

			Assert( bAtLeastOneProxy );

			return false;
		}

		return pNPC->GetAbsOrigin().z < GetAbsOrigin().z;
	}

	return false;
}

void CFunc_ASW_Fade::ApplyAllGrenadeCollisionRules( CBaseEntity *pGrenade )
{
	const string_t iszClassName = AllocPooledString( "func_asw_fade" );

	CFunc_ASW_Fade *pCeiling = nullptr;
	while ( ( pCeiling = assert_cast<CFunc_ASW_Fade *>( gEntList.FindEntityByClassnameFast( pCeiling, iszClassName ) ) ) != nullptr )
	{
		pCeiling->ApplyGrenadeCollisionRules( pGrenade );
	}

	const string_t iszPropClassName = AllocPooledString( "prop_asw_fade" );

	CProp_ASW_Fade *pProp = nullptr;
	while ( ( pProp = assert_cast<CProp_ASW_Fade *>( gEntList.FindEntityByClassnameFast( pProp, iszPropClassName ) ) ) != nullptr )
	{
		pProp->ApplyGrenadeCollisionRules( pGrenade );
	}
}

void CFunc_ASW_Fade::ApplyAllMarineCollisionRules( CBaseEntity *pMarine )
{
	int iMR = ( ( CFunc_ASW_Fade * )nullptr )->GetMarineIndex( pMarine, nullptr );

	const string_t iszClassName = AllocPooledString( "func_asw_fade" );

	CFunc_ASW_Fade *pCeiling = nullptr;
	while ( ( pCeiling = assert_cast<CFunc_ASW_Fade *>( gEntList.FindEntityByClassnameFast( pCeiling, iszClassName ) ) ) != nullptr )
	{
		pCeiling->ApplyMarineCollisionRules( pMarine, iMR );
	}

	const string_t iszPropClassName = AllocPooledString( "prop_asw_fade" );

	CProp_ASW_Fade *pProp = nullptr;
	while ( ( pProp = assert_cast<CProp_ASW_Fade *>( gEntList.FindEntityByClassnameFast( pProp, iszPropClassName ) ) ) != nullptr )
	{
		pProp->ApplyMarineCollisionRules( pMarine, iMR );
	}
}

void CFunc_ASW_Fade::ApplyGrenadeCollisionRules( CBaseEntity *pGrenade )
{
	const bool bShouldDisable =
		( m_iCollideWithGrenades == 0 && GetAbsOrigin().z >= pGrenade->GetAbsOrigin().z ) ||
		( m_iCollideWithGrenades == 2 );

	const bool bCurrentlyDisabled = PhysEntityCollisionsAreDisabled( this, pGrenade );

	if ( bShouldDisable != bCurrentlyDisabled )
	{
		if ( bShouldDisable )
		{
			PhysDisableEntityCollisions( this, pGrenade );
		}
		else
		{
			PhysEnableEntityCollisions( this, pGrenade );
		}
	}
}

void CFunc_ASW_Fade::ApplyMarineCollisionRules( CBaseEntity *pMarine, int iMR )
{
	const bool bShouldDisable = !m_bCollideWithMarines || ( iMR != -1 && !m_bSolidWhenInvisible && ( m_fVisibleToMarine & ( 1u << iMR ) ) == 0 );
	const bool bCurrentlyDisabled = PhysEntityCollisionsAreDisabled( this, pMarine );

	if ( bShouldDisable != bCurrentlyDisabled )
	{
		if ( bShouldDisable )
		{
			PhysDisableEntityCollisions( this, pMarine );
		}
		else
		{
			PhysEnableEntityCollisions( this, pMarine );
		}
	}
}

void CFunc_ASW_Fade::SetGrenadeCollisionRules( inputdata_t &inputdata )
{
	m_iCollideWithGrenades = clamp( inputdata.value.Int(), 0, 2 );

	for ( const char *const *pszClass = s_pszExplosiveClasses; *pszClass; ++pszClass )
	{
		CBaseEntity *pGrenade = NULL;
		while ( ( pGrenade = gEntList.FindEntityByClassname( pGrenade, *pszClass ) ) != NULL )
		{
			ApplyGrenadeCollisionRules( pGrenade );
		}
	}
}

void CFunc_ASW_Fade::SetMarineCollisionRules( inputdata_t &inputdata )
{
	m_bCollideWithMarines = !!inputdata.value.Int();

	const string_t iszClassName = AllocPooledString( "asw_marine" );

	CBaseEntity *pMarine = nullptr;
	while ( ( pMarine = gEntList.FindEntityByClassnameFast( pMarine, iszClassName ) ) != nullptr )
	{
		ApplyMarineCollisionRules( pMarine, GetMarineIndex( pMarine, nullptr ) );
	}
}

int CFunc_ASW_Fade::GetMarineIndex( CBaseEntity *pEnt, const char *szContext )
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

void CFunc_ASW_Fade::InputShowToMarine( inputdata_t &inputdata )
{
	int iMR = GetMarineIndex( inputdata.value.Entity(), "input ShowToMarine" );
	if ( iMR == -1 )
	{
		return;
	}

	m_fVisibleToMarine.GetForModify() |= 1u << iMR;
	ApplyMarineCollisionRules( inputdata.value.Entity(), iMR );
}

void CFunc_ASW_Fade::InputHideFromMarine( inputdata_t &inputdata )
{
	int iMR = GetMarineIndex( inputdata.value.Entity(), "input HideFromMarine" );
	if ( iMR == -1 )
	{
		return;
	}

	m_fVisibleToMarine.GetForModify() &= ~( 1u << iMR );
	ApplyMarineCollisionRules( inputdata.value.Entity(), iMR );
}

void CFunc_ASW_Fade::InputSetVisibleForAllMarines( inputdata_t &inputdata )
{
	m_fVisibleToMarine = inputdata.value.Bool() ? UINT32_MAX : 0;

	CASW_Game_Resource *pGameResource = ASWGameResource();
	Assert( pGameResource );
	if ( !pGameResource )
	{
		return;
	}

	for ( int iMR = 0; iMR < pGameResource->GetMaxMarineResources(); iMR++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( iMR );
		if ( !pMR )
		{
			continue;
		}

		CASW_Marine *pMarine = pMR->GetMarineEntity();
		if ( pMarine )
		{
			ApplyMarineCollisionRules( pMarine, iMR );
		}
	}
}

void CFunc_ASW_Fade::ScriptSetVisibleForMarine( HSCRIPT hMarine, bool bVisible )
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

	ApplyMarineCollisionRules( pEnt, iMR );
}

bool CFunc_ASW_Fade::ScriptIsVisibleForMarine( HSCRIPT hMarine, bool bConsiderPositionalFade )
{
	CBaseEntity *pEnt = ToEnt( hMarine );

	int iMR = GetMarineIndex( pEnt, nullptr );
	if ( iMR != -1 && ( m_fVisibleToMarine.Get() & ( 1u << iMR ) ) == 0 )
	{
		return false;
	}

	return !bConsiderPositionalFade || !pEnt || !pEnt->IsInhabitableNPC() || !ShouldFade( assert_cast<CASW_Inhabitable_NPC *>( pEnt ) );
}

#include "cbase.h"
#include "rd_weapon_generic_object_shared.h"

#ifdef CLIENT_DLL
#include <vgui_controls/Controls.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "c_asw_marine.h"
#include "asw_util_shared.h"
#else
#include "asw_marine.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( RD_Weapon_Generic_Object, DT_RD_Weapon_Generic_Object );

BEGIN_NETWORK_TABLE( CRD_Weapon_Generic_Object, DT_RD_Weapon_Generic_Object )
#ifdef CLIENT_DLL
	RecvPropFloat( RECVINFO( m_flMoveSpeedMultiplier ) ),
	RecvPropBool( RECVINFO( m_bLargeObject ) ),
	RecvPropBool( RECVINFO( m_bUseBoneMerge ) ),
	RecvPropQAngles( RECVINFO( m_angCarriedAngle ) ),
	RecvPropVector( RECVINFO( m_vecCarriedOffset ) ),
	RecvPropString( RECVINFO( m_szWorldModel ) ),
	RecvPropString( RECVINFO( m_szCarriedName ) ),
	RecvPropString( RECVINFO( m_iOriginalName ) ),
	RecvPropString( RECVINFO( m_szEquipIcon ) ),
#else
	SendPropFloat( SENDINFO( m_flMoveSpeedMultiplier ) ),
	SendPropBool( SENDINFO( m_bLargeObject ) ),
	SendPropBool( SENDINFO( m_bUseBoneMerge ) ),
	SendPropQAngles( SENDINFO( m_angCarriedAngle ) ),
	SendPropVector( SENDINFO( m_vecCarriedOffset ) ),
	SendPropString( SENDINFO( m_szWorldModel ) ),
	SendPropString( SENDINFO( m_szCarriedName ) ),
	SendPropStringT( SENDINFO( m_iOriginalName ) ),
	SendPropString( SENDINFO( m_szEquipIcon ) ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CRD_Weapon_Generic_Object )
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( rd_weapon_generic_object, CRD_Weapon_Generic_Object );
PRECACHE_WEAPON_REGISTER( rd_weapon_generic_object );

#ifdef GAME_DLL
BEGIN_DATADESC( CRD_Weapon_Generic_Object )
	DEFINE_KEYFIELD( m_flMoveSpeedMultiplier, FIELD_FLOAT, "MoveSpeedMultiplier" ),
	DEFINE_KEYFIELD( m_bLargeObject, FIELD_BOOLEAN, "LargeObject" ),
	DEFINE_KEYFIELD( m_bUseBoneMerge, FIELD_BOOLEAN, "UseBoneMerge" ),
	DEFINE_KEYFIELD( m_angCarriedAngle, FIELD_VECTOR, "CarriedAngle" ),
	DEFINE_KEYFIELD( m_vecCarriedOffset, FIELD_VECTOR, "CarriedOffset" ),
	DEFINE_INPUTFUNC( FIELD_EHANDLE, "ForcePickUp", InputForcePickUp ),
	DEFINE_INPUTFUNC( FIELD_VOID, "ForceDrop", InputForceDrop ),
	DEFINE_OUTPUT( m_OnPrimaryAttack, "OnPrimaryAttack" ),
	DEFINE_OUTPUT( m_OnSecondaryAttack, "OnSecondaryAttack" ),
	DEFINE_OUTPUT( m_OnReload, "OnReload" ),
	DEFINE_OUTPUT( m_OnPickedUp, "OnPickedUp" ),
	DEFINE_OUTPUT( m_OnDropped, "OnDropped" ),
END_DATADESC()
#endif

CRD_Weapon_Generic_Object::CRD_Weapon_Generic_Object()
{
	m_flMoveSpeedMultiplier = 1.0f;
	m_bLargeObject = false;
	m_bUseBoneMerge = false;
	m_angCarriedAngle.Init();
	m_vecCarriedOffset.Init();
	V_strncpy( m_szWorldModel.GetForModify(), "models/error.mdl", sizeof( m_szWorldModel ) );
	m_szCarriedName.GetForModify()[0] = '\0';
	m_szEquipIcon.GetForModify()[0] = '\0';
#ifdef CLIENT_DLL
	m_wszCarriedName[0] = L'\0';
	m_iEquipIcon = -1;
#endif
}

CRD_Weapon_Generic_Object::~CRD_Weapon_Generic_Object()
{
#ifdef CLIENT_DLL
	if ( vgui::surface() && m_iEquipIcon != -1 )
	{
		vgui::surface()->DestroyTextureID( m_iEquipIcon );
		m_iEquipIcon = -1;
	}
#endif
}

#ifdef GAME_DLL
bool CRD_Weapon_Generic_Object::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( FStrEq( szKeyName, "model" ) )
	{
		V_strncpy( m_szWorldModel.GetForModify(), szValue, 256 );
		return true;
	}
	else if ( FStrEq( szKeyName, "CarriedName" ) )
	{
		V_strncpy( m_szCarriedName.GetForModify(), szValue, 256 );
		return true;
	}
	else if ( FStrEq( szKeyName, "EquipIcon" ) )
	{
		V_strncpy( m_szEquipIcon.GetForModify(), szValue, 256 );
		return true;
	}
	
	return BaseClass::KeyValue( szKeyName, szValue );
}

void CRD_Weapon_Generic_Object::Spawn()
{
	BaseClass::Spawn();

	m_iOriginalName = m_iName;
}

void CRD_Weapon_Generic_Object::MarineDropped( CASW_Marine *pMarine )
{
	BaseClass::MarineDropped( pMarine );

	SetName( m_iOriginalName );
}

void CRD_Weapon_Generic_Object::InputForcePickUp( inputdata_t &data )
{
	if ( GetOwner() )
	{
		DevWarning( "Mapper error: %s cannot be picked up as it is already being held by %s\n", GetDebugName(), GetOwner()->GetDebugName() );
		return;
	}

	CASW_Marine *pMarine = CASW_Marine::AsMarine( data.value.Entity() );
	Assert( pMarine );
	if ( !pMarine )
	{
		DevWarning( "Mapper error: %s cannot be picked up by non-marine entity %s\n", GetDebugName(), data.value.Entity() ? data.value.Entity()->GetDebugName() : "<<NULL>>" );
		return;
	}

	ActivateUseIcon( pMarine, ASW_USE_RELEASE_QUICK );
}

void CRD_Weapon_Generic_Object::InputForceDrop( inputdata_t &data )
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( GetOwner() );
	Assert( pMarine );
	if ( !pMarine )
	{
		DevWarning( "Mapper error: %s cannot be picked up as it is not being held\n", GetDebugName() );
		return;
	}

	pMarine->DropWeapon( this, false );
}
#endif

void CRD_Weapon_Generic_Object::Precache()
{
	BaseClass::Precache();

	PrecacheModel( m_szWorldModel );
}

void CRD_Weapon_Generic_Object::PrimaryAttack()
{
#ifdef GAME_DLL
	m_OnPrimaryAttack.FireOutput( GetOwner(), this );
#endif
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f;
}

void CRD_Weapon_Generic_Object::SecondaryAttack()
{
#ifdef GAME_DLL
	m_OnSecondaryAttack.FireOutput( GetOwner(), this );
#endif
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.1f;
}

bool CRD_Weapon_Generic_Object::Reload()
{
#ifdef GAME_DLL
	m_OnReload.FireOutput( GetOwner(), this );
#endif
	return false;
}

void CRD_Weapon_Generic_Object::Equip( CBaseCombatCharacter *pOwner )
{
	BaseClass::Equip( pOwner );

	if ( !m_bUseBoneMerge )
	{
		SetParent( pOwner, pOwner->LookupAttachment( "UnderGun" ) );
		RemoveEffects( EF_BONEMERGE );
		SetLocalOrigin( m_vecCarriedOffset );
		SetLocalAngles( m_angCarriedAngle );
	}

#ifdef GAME_DLL
	m_OnPickedUp.FireOutput( GetOwner(), this );
#endif
}

void CRD_Weapon_Generic_Object::Drop( const Vector & vecVelocity )
{
#ifdef GAME_DLL
	m_OnDropped.FireOutput( GetOwner(), this );
#endif

	if ( !m_bUseBoneMerge )
	{
		Vector origin = GetAbsOrigin();
		QAngle angles = GetAbsAngles();

		FollowEntity( GetMoveParent() );
		BaseClass::Drop( vecVelocity );

		SetAbsOrigin( origin );
		SetAbsAngles( angles );
	}
	else
	{
		BaseClass::Drop( vecVelocity );
	}
}

bool CRD_Weapon_Generic_Object::ShouldMarineMoveSlow()
{
	return m_flMoveSpeedMultiplier < 1.0f;
}

float CRD_Weapon_Generic_Object::GetMovementScale()
{
	return m_flMoveSpeedMultiplier;
}

bool CRD_Weapon_Generic_Object::IsOffensiveWeapon()
{
	return false;
}

int CRD_Weapon_Generic_Object::ASW_SelectWeaponActivity( int idealActivity )
{
	switch ( idealActivity )
	{
	case ACT_WALK:
		idealActivity = m_bLargeObject ? ACT_MP_WALK_ITEM1 : ACT_WALK_AIM_PISTOL;
		break;
	case ACT_RUN:
		idealActivity = m_bLargeObject ? ACT_MP_RUN_ITEM1 : ACT_RUN_AIM_PISTOL;
		break;
	case ACT_IDLE:
		idealActivity = m_bLargeObject ? ACT_MP_STAND_ITEM1 : ACT_IDLE_PISTOL;
		break;
	case ACT_CROUCHIDLE:
		idealActivity = m_bLargeObject ? ACT_MP_CROUCHWALK_ITEM1 : idealActivity;
		break;
	case ACT_JUMP:
		idealActivity = m_bLargeObject ? ACT_MP_JUMP_ITEM1 : idealActivity;
		break;
	}

	return idealActivity;
}

#ifdef CLIENT_DLL
void CRD_Weapon_Generic_Object::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );

	TryLocalize( m_szCarriedName, m_wszCarriedName, sizeof( m_wszCarriedName ) );
}

bool CRD_Weapon_Generic_Object::GetUseAction( ASWUseAction & action, C_ASW_Inhabitable_NPC *pUser )
{
	if ( !pUser )
		return false;

	CASW_Marine *pMarine = CASW_Marine::AsMarine( pUser );

	action.iUseIconTexture = GetUseIconTextureID();
	action.UseTarget = this;
	action.fProgress = -1;
	action.iInventorySlot = pMarine ? pMarine->GetWeaponPositionForPickup( GetClassname(), m_bIsTemporaryPickup ) : -1;
	action.bWideIcon = ( action.iInventorySlot != ASW_INVENTORY_SLOT_EXTRA );

	// build the appropriate take string
	const CASW_WeaponInfo *pInfo = GetWeaponInfo();
	if ( pInfo )
	{
		if ( m_bSwappingWeapon )
		{
			g_pVGuiLocalize->ConstructString( action.wszText, sizeof( action.wszText ), g_pVGuiLocalize->Find("#asw_swap_weapon_format"), 1, m_wszCarriedName );
		}
		else
		{
			g_pVGuiLocalize->ConstructString( action.wszText, sizeof( action.wszText ), g_pVGuiLocalize->Find("#asw_take_weapon_format"), 1, m_wszCarriedName );
		}
	}

	action.UseIconRed = 66;
	action.UseIconGreen = 142;
	action.UseIconBlue = 192;
	action.bShowUseKey = true;

	return true;
}

int CRD_Weapon_Generic_Object::GetUseIconTextureID()
{
	if ( m_iEquipIcon == -1 && m_szEquipIcon.Get()[0] != '\0' && vgui::surface() )
	{
		m_iEquipIcon = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_iEquipIcon, m_szEquipIcon, 1, false );
	}

	return m_iEquipIcon;
}
#endif

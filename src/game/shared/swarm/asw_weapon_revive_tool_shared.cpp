#include "cbase.h"
#include "asw_weapon_revive_tool_shared.h"
#include "asw_marine_profile.h"
#ifdef CLIENT_DLL
#include "c_asw_marine.h"
#include "c_asw_marine_resource.h"
#include "dlight.h"
#include "iefx.h"
#else
#include "ai_pathfinder.h"
#include "asw_path_utils.h"
#include "asw_marine.h"
#include "phys_controller.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
IMPLEMENT_AUTO_LIST( IASW_Revive_Tool_Auto_List );
IMPLEMENT_AUTO_LIST( IASW_Revive_Tombstone_Auto_List );

#ifdef CLIENT_DLL
static void TombstoneLightSettingsChanged( IConVar *var, const char *pOldValue, float flOldValue );
ConVar rd_revive_tombstone_hologram_always_visible( "rd_revive_tombstone_hologram_always_visible", "0", FCVAR_ARCHIVE, "1 = show hologram when it would normally be inactive; 3 = even when falling" );
ConVar rd_revive_tombstone_hologram_force_visible_distance( "rd_revive_tombstone_hologram_force_visible_distance", "100", FCVAR_NONE, "Tombstone holograms show as if rd_revive_tombstone_hologram_always_visible was 1 when the marine is within this distance" );
ConVar rd_revive_tombstone_dlight( "rd_revive_tombstone_dlight", "1", FCVAR_NONE, "Should tombstone holograms emit light?", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_z_offset( "rd_revive_tombstone_dlight_z_offset", "64", FCVAR_NONE, "Where's that light?", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_radius( "rd_revive_tombstone_dlight_radius", "128", FCVAR_NONE, "How big's that light?", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_color_officer( "rd_revive_tombstone_dlight_color_officer", "255 255 255 128", FCVAR_NONE, "Light glow color for officer", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_color_special_weapons( "rd_revive_tombstone_dlight_color_special_weapons", "255 255 255 128", FCVAR_NONE, "Light glow color for special weapons", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_color_medic( "rd_revive_tombstone_dlight_color_medic", "255 255 255 128", FCVAR_NONE, "Light glow color for medic", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_color_tech( "rd_revive_tombstone_dlight_color_tech", "255 255 255 128", FCVAR_NONE, "Light glow color for tech", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_active_z_offset( "rd_revive_tombstone_dlight_active_z_offset", "64", FCVAR_NONE, "Where's that light? (during revive)", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_active_radius( "rd_revive_tombstone_dlight_active_radius", "128", FCVAR_NONE, "How big's that light? (during revive)", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_active_color_officer( "rd_revive_tombstone_dlight_active_color_officer", "255 255 255 128", FCVAR_NONE, "Light glow color for officer being revived", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_active_color_special_weapons( "rd_revive_tombstone_dlight_active_color_special_weapons", "255 255 255 128", FCVAR_NONE, "Light glow color for special weapons being revived", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_active_color_medic( "rd_revive_tombstone_dlight_active_color_medic", "255 255 255 128", FCVAR_NONE, "Light glow color for medic being revived", TombstoneLightSettingsChanged );
ConVar rd_revive_tombstone_dlight_active_color_tech( "rd_revive_tombstone_dlight_active_color_tech", "255 255 255 128", FCVAR_NONE, "Light glow color for tech being revived", TombstoneLightSettingsChanged );
#else
ConVar rd_revive_tombstone_tick( "rd_revive_tombstone_tick", "0.25", FCVAR_CHEAT, "Tick interval for tombstone maintenance" );
ConVar rd_revive_tombstone_path_fails_for_teleport( "rd_revive_tombstone_path_fails_for_teleport", "16", FCVAR_CHEAT, "Teleport if the tombstone is grounded and the medic cannot reach the tombstone for this many consecutive ticks" );
ConVar rd_revive_tombstone_path_success_cooldown( "rd_revive_tombstone_path_success_cooldown", "8", FCVAR_CHEAT, "Tombstone doesn't check again for this many ticks if it finds a path" );
ConVar rd_revive_tombstone_angular_limit( "rd_revive_tombstone_angular_limit", "2", FCVAR_CHEAT, "Maximum tilt of the tombstone" );
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Revive_Tool, DT_ASW_Weapon_Revive_Tool );

BEGIN_NETWORK_TABLE( CASW_Weapon_Revive_Tool, DT_ASW_Weapon_Revive_Tool )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CASW_Weapon_Revive_Tool )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( asw_weapon_revive_tool, CASW_Weapon_Revive_Tool );
PRECACHE_WEAPON_REGISTER( asw_weapon_revive_tool );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Revive_Tool )
END_DATADESC()
#endif

void CASW_Weapon_Revive_Tool::Precache()
{
	BaseClass::Precache();

#ifdef GAME_DLL
	UTIL_PrecacheOther( "asw_revive_tombstone" );
#endif
}

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Revive_Tombstone, DT_ASW_Revive_Tombstone );

BEGIN_NETWORK_TABLE( CASW_Revive_Tombstone, DT_ASW_Revive_Tombstone )
#ifdef CLIENT_DLL
	RecvPropEHandle( RECVINFO( m_hMarineResource ) ),
	RecvPropEHandle( RECVINFO( m_hGroundEntity ) ),
	RecvPropBool( RECVINFO( m_bGibbed ) ),
	RecvPropTime( RECVINFO( m_flReviveTime ) ),
#else
	SendPropEHandle( SENDINFO( m_hMarineResource ) ),
	SendPropEHandle( SENDINFO( m_hGroundEntity ) ),
	SendPropBool( SENDINFO( m_bGibbed ) ),
	SendPropTime( SENDINFO( m_flReviveTime ) ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( asw_revive_tombstone, CASW_Revive_Tombstone );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Revive_Tombstone )
	DEFINE_THINKFUNC( TombstoneThink ),
	DEFINE_FIELD( m_iPathFails, FIELD_INTEGER ),
	DEFINE_FIELD( m_hKeepUpright, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bKeepUpright, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_hMarineResource, FIELD_EHANDLE ),
	DEFINE_FIELD( m_bGibbed, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flReviveTime, FIELD_TIME ),
END_DATADESC()
#endif

CASW_Revive_Tombstone::CASW_Revive_Tombstone()
{
#ifdef CLIENT_DLL
	m_bHologramActive = false;
	m_bIsReviving = false;

	m_pLightGlow = NULL;
#else
	SetModelName( AllocPooledString( "models/weapons/revivetool/blackbox.mdl" ) );

	m_iPathFails = 0;
	m_hKeepUpright = NULL;
	m_bKeepUpright = false;
#endif

	m_hMarineResource = NULL;
	m_bGibbed = false;
	m_flReviveTime = -1.0f;
}

CASW_Revive_Tombstone::~CASW_Revive_Tombstone()
{
#ifdef CLIENT_DLL
	if ( m_pLightGlow )
	{
		m_pLightGlow->die = gpGlobals->curtime;
		m_pLightGlow = NULL;
	}
#endif
}

constexpr const char *s_pTombstoneThink = "TombstoneThink";

void CASW_Revive_Tombstone::Precache()
{
	BaseClass::Precache();

	PrecacheModel( STRING( GetModelName() ) );
	PrecacheParticleSystem( "marine_resurrection" );
}

#ifdef CLIENT_DLL
void CASW_Revive_Tombstone::OnDataChanged( DataUpdateType_t updateType )
{
	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

void CASW_Revive_Tombstone::ClientThink()
{
	bool bShouldHologramsBeActive = true;
	if ( m_flReviveTime < 0.0f )
	{
		bool bAnyReviveTool = false;
		bool bReviveToolIsOut = false;
		CASW_Marine *pViewMarine = CASW_Marine::GetViewMarine();
		FOR_EACH_VEC( IASW_Revive_Tool_Auto_List::AutoList(), i )
		{
			CASW_Weapon_Revive_Tool *pReviveTool = assert_cast< CASW_Weapon_Revive_Tool * >( IASW_Revive_Tool_Auto_List::AutoList()[i] );
			bAnyReviveTool = true;

			CASW_Marine *pMarine = pReviveTool->GetMarine();
			if ( !pMarine )
				continue;

			if ( pMarine == pViewMarine || pMarine->GetActiveASWWeapon() == pReviveTool )
			{
				bReviveToolIsOut = true;
				break;
			}
		}

		if ( !bAnyReviveTool )
		{
			// we will die soon :(

			return;
		}

		bShouldHologramsBeActive = GetGroundEntity() != NULL || ( rd_revive_tombstone_hologram_always_visible.GetInt() & 2 ) != 0;
		if ( bShouldHologramsBeActive )
			bShouldHologramsBeActive = ( rd_revive_tombstone_hologram_always_visible.GetInt() & 1 ) != 0 || bReviveToolIsOut || ( pViewMarine && pViewMarine->GetAbsOrigin().DistToSqr( GetAbsOrigin() ) <= Square( rd_revive_tombstone_hologram_force_visible_distance.GetFloat() ) );
	}

	bool bIsRevivingChanged = ( m_flReviveTime >= 0.0f ) != m_bIsReviving;
	int iMarineProfile = m_hMarineResource ? m_hMarineResource->GetProfileIndex() : ASW_MARINE_PROFILE_BASTILLE;
	int iProfileSkinMinusOne = m_hMarineResource && m_hMarineResource->GetProfile() ? m_hMarineResource->GetProfile()->GetSkinNum() - 1 : 1;
	SetSkin( iProfileSkinMinusOne + 1 );
	if ( bShouldHologramsBeActive != m_bHologramActive || bIsRevivingChanged )
	{
		m_bIsReviving = m_flReviveTime >= 0.0f;

		// 1:
		// - 0: is reviving currently
		// - 1: is not reviving currently
		SetBodygroup( 1, m_bIsReviving ? 0 : 1 );

		// 2:
		// - 0: any hologram is visible (light on)
		// - 1: no hologram is visible (light off)
		SetBodygroup( 2, bShouldHologramsBeActive ? 0 : 1 );

		// 3 (variant A):
		// - 0: hologram for generic marine
		// - 1: no hologram
		// 3 (variant B):
		// - 0: hologram for generic marine
		// - 1: hologram for generic gibbed marine
		// - 2: no hologram
		// 3 (variant C):
		// - 0: hologram for officer
		// - 1: hologram for medic
		// - 2: hologram for special weapons
		// - 3: hologram for tech
		// - 4: no hologram
		// 3 (variant D):
		// - 0: hologram for officer
		// - 1: hologram for medic
		// - 2: hologram for special weapons
		// - 3: hologram for tech
		// - 4: hologram for gibbed officer
		// - 5: hologram for gibbed medic
		// - 6: hologram for gibbed special weapons
		// - 7: hologram for gibbed tech
		// - 8: no hologram
		// 3 (variant E):
		// - 0: hologram for sarge
		// - 1: hologram for wildcat (! order is different !)
		// - 2: hologram for faith
		// - 3: hologram for crash
		// - 4: hologram for jaeger
		// - 5: hologram for wolfe
		// - 6: hologram for bastille
		// - 7: hologram for vegas
		// - 8: hologram for gibbed sarge
		// - 9: hologram for gibbed wildcat
		// - 10: hologram for gibbed faith
		// - 11: hologram for gibbed crash
		// - 12: hologram for gibbed jaeger
		// - 13: hologram for gibbed wolfe
		// - 14: hologram for gibbed bastille
		// - 15: hologram for gibbed vegas
		// - 16: no hologram
		switch ( GetBodygroupCount( 3 ) )
		{
		default:
			Assert( GetBodygroupCount( 3 ) <= 1 );
			break;
		case 2: // variant A
			SetBodygroup( 3, bShouldHologramsBeActive ? 0 : 1 );
			break;
		case 3: // variant B
			if ( !bShouldHologramsBeActive )
			{
				SetBodygroup( 3, 2 );
			}
			else
			{
				SetBodygroup( 3, m_bGibbed ? 1 : 0 );
			}
			break;
		case 5: // variant C
			SetBodygroup( 3, bShouldHologramsBeActive ? iProfileSkinMinusOne : 4 );
			break;
		case 9: // variant D
			SetBodygroup( 3, bShouldHologramsBeActive ? iProfileSkinMinusOne + ( m_bGibbed ? 4 : 0 ) : 8 );
			break;
		case 17: // variant E
			SetBodygroup( 3, bShouldHologramsBeActive ? iMarineProfile + ( m_bGibbed ? 8 : 0 ) : 16 );
			break;
		}
	}

	if ( ( m_pLightGlow != NULL ) != ( bShouldHologramsBeActive && rd_revive_tombstone_dlight.GetBool() ) || bIsRevivingChanged )
	{
		if ( m_pLightGlow )
		{
			m_pLightGlow->die = gpGlobals->curtime;
			m_pLightGlow = NULL;
		}

		if ( bShouldHologramsBeActive && rd_revive_tombstone_dlight.GetBool() )
		{
			m_pLightGlow = effects->CL_AllocDlight( index );
			m_pLightGlow->origin = GetAbsOrigin() + Vector( 0, 0, m_bIsReviving ? rd_revive_tombstone_dlight_active_z_offset.GetFloat() : rd_revive_tombstone_dlight_z_offset.GetFloat() );
			Color lightColor;
			switch ( iProfileSkinMinusOne )
			{
			case 0:
				lightColor = m_bIsReviving ? rd_revive_tombstone_dlight_active_color_officer.GetColor() : rd_revive_tombstone_dlight_color_officer.GetColor();
				break;
			default:
			case 1:
				lightColor = m_bIsReviving ? rd_revive_tombstone_dlight_active_color_medic.GetColor() : rd_revive_tombstone_dlight_color_medic.GetColor();
				break;
			case 2:
				lightColor = m_bIsReviving ? rd_revive_tombstone_dlight_active_color_special_weapons.GetColor() : rd_revive_tombstone_dlight_color_special_weapons.GetColor();
				break;
			case 3:
				lightColor = m_bIsReviving ? rd_revive_tombstone_dlight_active_color_tech.GetColor() : rd_revive_tombstone_dlight_color_tech.GetColor();
				break;
			}
			m_pLightGlow->color.r = lightColor.r();
			m_pLightGlow->color.g = lightColor.g();
			m_pLightGlow->color.b = lightColor.b();
			m_pLightGlow->color.exponent = lightColor.a() - 128;
			m_pLightGlow->radius = m_bIsReviving ? rd_revive_tombstone_dlight_active_radius.GetFloat() : rd_revive_tombstone_dlight_radius.GetFloat();
		}
	}

	Assert( 0 ); // TODO: globally visible minimap skull
}

static void TombstoneLightSettingsChanged( IConVar *var, const char *pOldValue, float flOldValue )
{
	// kill the lights if the settings changed; the tombstones will re-create the lights if they need to.
	FOR_EACH_VEC( IASW_Revive_Tombstone_Auto_List::AutoList(), i )
	{
		CASW_Revive_Tombstone *pTombstone = assert_cast< CASW_Revive_Tombstone * >( IASW_Revive_Tombstone_Auto_List::AutoList()[i] );
		if ( pTombstone->m_pLightGlow )
		{
			pTombstone->m_pLightGlow->die = gpGlobals->curtime;
			pTombstone->m_pLightGlow = NULL;
		}
	}
}
#else
void CASW_Revive_Tombstone::Spawn()
{
	Precache();

	SetModel( STRING( GetModelName() ) );

	BaseClass::Spawn();

	m_hKeepUpright = CreateKeepUpright( GetAbsOrigin(), vec3_angle, this, rd_revive_tombstone_angular_limit.GetFloat(), false );
	SetContextThink( &TombstoneThink, gpGlobals->curtime, s_pTombstoneThink );
}

void CASW_Revive_Tombstone::TombstoneThink()
{
	Assert( m_flReviveTime < 0.0f ); // we shouldn't be doing the TombstoneThink if we're reviving.

	bool bOnGround = GetGroundEntity() != NULL;
	if ( !bOnGround )
	{
		m_iPathFails = 0;
	}
	else if ( m_iPathFails < 0 )
	{
		m_iPathFails++;
	}
	else
	{
		bool bAnyReviveTool = false;
		bool bAnyMedicHoldingReviveTool = false;
		bool bAnyMedicCanReach = false;

		CBaseEntity *pEnt = NULL;
		FOR_EACH_VEC( IASW_Revive_Tool_Auto_List::AutoList(), i )
		{
			CASW_Weapon_Revive_Tool *pReviveTool = assert_cast< CASW_Weapon_Revive_Tool * >( IASW_Revive_Tool_Auto_List::AutoList()[i] );
			bAnyReviveTool = true;

			CASW_Marine *pMarine = pReviveTool->GetMarine();
			if ( !pMarine )
				continue;

			bAnyMedicHoldingReviveTool = true;

			AI_Waypoint_t *pPath = pMarine->GetPathfinder()->BuildRoute( pMarine->GetAbsOrigin(), GetAbsOrigin(), this, ASW_MARINE_USE_RADIUS );
			if ( pPath )
			{
				ASWPathUtils()->DeleteRoute( pPath );

				bAnyMedicCanReach = true;

				break;
			}
		}

		if ( !bAnyReviveTool )
		{
			Assert( 0 ); // TODO: the box dies :(
		}
		else if ( bAnyMedicHoldingReviveTool && !bAnyMedicCanReach )
		{
			m_iPathFails++;

			if ( m_iPathFails >= rd_revive_tombstone_path_fails_for_teleport.GetInt() )
			{
				Assert( 0 ); // TODO: teleport somewhere closer
			}
		}
		else
		{
			m_iPathFails = -rd_revive_tombstone_path_success_cooldown.GetInt();
		}
	}

	if ( m_hKeepUpright && bOnGround != m_bKeepUpright )
	{
		m_hKeepUpright->AcceptInput( bOnGround ? "TurnOn" : "TurnOff", this, this, variant_t{}, -1 );
		m_bKeepUpright = bOnGround;
	}

	SetNextThink( gpGlobals->curtime + rd_revive_tombstone_tick.GetFloat(), s_pTombstoneThink);
}
#endif
#endif

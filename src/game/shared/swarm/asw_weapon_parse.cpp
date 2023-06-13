#include "cbase.h"
#include <KeyValues.h>
#include "asw_weapon_parse.h"
#include "rd_inventory_shared.h"

FileWeaponInfo_t* CreateWeaponInfo()
{
	return new CASW_WeaponInfo;
}



CASW_WeaponInfo::CASW_WeaponInfo()
{
	flReloadTime = 0;	// asw
	m_flBaseDamage = 0;
	m_flFireRate = 0;
	m_iNumPellets = 0;
	m_iPlayerModelSkin = 0;
	m_iDisplayModelSkin = 0;
	m_iDisplayClipSize = -1;
	m_flDisplayReloadTime = -1.0f;
	szDisplayModel[0] = 0;
	szDisplayModel2[0] = 0;
	m_flModelPanelZOffset = 0.0f;
	m_bShowCharges = false;
	m_bShowBatteryCharge = false;
	m_bShowMultiplayerHotkey = false;
	m_bShowLocalPlayerHotkey = true;
	m_bHUDPlayerActivate = false;
	m_nOffhandOrderType = ASW_OFFHAND_USE_IMMEDIATELY;
	m_bOrientToLaser = true;
	m_bShowClipsInWeaponDetail = true;
	m_bShowClipsDoubled = false;

	m_fFlinchChance = 0;
	m_fStoppingPowerFlinchBonus = 0;

	m_iHUDIconOffsetX = m_iHUDIconOffsetY = 0;
	m_iHUDNumberOffsetX = m_iHUDNumberOffsetY = 0;

	m_bUseStrangeDeviceWorldOffsets = false;

	COMPILE_TIME_ASSERT( NELEMS( m_vecStrangeDeviceOffset ) == NELEMS( m_angStrangeDeviceAngle ) );
	COMPILE_TIME_ASSERT( NELEMS( m_vecStrangeDeviceOffset ) == NELEMS( m_szStrangeDeviceBone ) );
	COMPILE_TIME_ASSERT( NELEMS( m_vecStrangeDeviceOffset ) == NELEMS( m_vecStrangeDeviceOffsetWorld ) );
	COMPILE_TIME_ASSERT( NELEMS( m_vecStrangeDeviceOffset ) == NELEMS( m_angStrangeDeviceAngleWorld ) );
	COMPILE_TIME_ASSERT( NELEMS( m_vecStrangeDeviceOffset ) == NELEMS( m_szStrangeDeviceBoneWorld ) );
	COMPILE_TIME_ASSERT( NELEMS( m_vecStrangeDeviceOffset ) == RD_ITEM_MAX_ACCESSORIES + 1 );
	for ( int i = 0; i < NELEMS( m_vecStrangeDeviceOffset ); i++ )
	{
		m_vecStrangeDeviceOffset[i].Init();
		m_angStrangeDeviceAngle[i].Init();
		m_szStrangeDeviceBone[i][0] = 0;
		m_vecStrangeDeviceOffsetWorld[i].Init();
		m_angStrangeDeviceAngleWorld[i].Init();
		m_szStrangeDeviceBoneWorld[i][0] = 0;
	}
}

static const char* g_OffhandOrderNames[]={
	"ASW_OFFHAND_USE_IMMEDIATELY",
	"ASW_OFFHAND_THROW",
	"ASW_OFFHAND_DEPLOY",
};

ASW_Offhand_Order_t OffhandOrderTypeFromString( const char *szText )
{
	for ( int i = 0; i < NELEMS( g_OffhandOrderNames ); i++ )
	{
		if ( !Q_stricmp( szText, g_OffhandOrderNames[ i ] ) )
		{
			return (ASW_Offhand_Order_t) i;
		}
	}
	return ASW_OFFHAND_USE_IMMEDIATELY;
}

void CASW_WeaponInfo::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	BaseClass::Parse( pKeyValuesData, szWeaponName );

	// asw: alien swarm specific weapon properties
	flReloadTime = pKeyValuesData->GetFloat( "reload_time", 2.2f );
	m_flBaseDamage = pKeyValuesData->GetFloat( "base_damage", 0.0f );
	m_flFireRate = pKeyValuesData->GetFloat( "fire_rate", 0.0f );
	m_flSecondaryFireRate = pKeyValuesData->GetFloat( "secondary_fire_rate", 0.0f );
	m_iNumPellets = pKeyValuesData->GetInt("num_pellets", 1 );

	m_iDisplayClipSize = pKeyValuesData->GetFloat( "display_clip_size", -1 );
	m_flDisplayReloadTime = pKeyValuesData->GetFloat( "display_reload_time", -1.0f );

	m_iPlayerModelSkin = pKeyValuesData->GetInt("playermodel_skin", 0 );
	m_iDisplayModelSkin = pKeyValuesData->GetInt("displaymodel_skin", 0 );

	V_strncpy( szDisplayModel, pKeyValuesData->GetString( "displaymodel" ), 64 );
	V_strncpy( szDisplayModel2, pKeyValuesData->GetString( "displaymodel2" ), 64 );

	m_flModelPanelZOffset = pKeyValuesData->GetFloat( "ModelPanelZOffset", 0.0f );
	m_bOffhandActivate = !!pKeyValuesData->GetInt("OffhandActivate", 0);
	m_bHUDPlayerActivate = !!pKeyValuesData->GetInt("HUDPlayerActivate", 0);
	m_bOffhandSwitch = !!pKeyValuesData->GetInt("OffhandSwitch", 0);
	m_nOffhandOrderType = OffhandOrderTypeFromString( pKeyValuesData->GetString( "OffhandOrderType", "" ) );
	m_bZoomHotbarIcon = pKeyValuesData->GetBool( "ZoomHotbarIcon", false );

	m_fFlinchChance = pKeyValuesData->GetFloat("FlinchChance");
	m_fStoppingPowerFlinchBonus = pKeyValuesData->GetFloat("StoppingPowerFlinchBonus");

	m_bShowCharges = !!pKeyValuesData->GetInt("ShowCharges", 0);
	m_bShowBatteryCharge = pKeyValuesData->GetBool("ShowBatteryCharge", 0);
	m_bShowMultiplayerHotkey = !!pKeyValuesData->GetInt("ShowMultiplayerHotkey", 0);
	m_bShowLocalPlayerHotkey = pKeyValuesData->GetBool( "ShowLocalPlayerHotkey", 1 );

	m_iHUDIconOffsetX = pKeyValuesData->GetInt("HUDIconOffsetX", 0);
	m_iHUDIconOffsetY = pKeyValuesData->GetInt("HUDIconOffsetY", 0);
	m_iHUDNumberOffsetX = pKeyValuesData->GetInt("HUDNumberOffsetX", 0);
	m_iHUDNumberOffsetY = pKeyValuesData->GetInt("HUDNumberOffsetY", 0);

	m_iShowBulletsOnHUD = pKeyValuesData->GetInt("ShowBulletsOnHUD", 0);
	m_iShowClipsOnHUD = pKeyValuesData->GetInt("ShowClipsOnHUD", 0);
	m_iShowGrenadesOnHUD = pKeyValuesData->GetInt("ShowGrenadesOnHUD", 0);
	m_iShowSecondaryBulletsOnHUD = pKeyValuesData->GetInt("ShowSecondaryBulletsOnHUD", 0);
	m_bOrientToLaser = pKeyValuesData->GetBool( "OrientToLaser", 1 );
	m_bShowClipsInWeaponDetail = pKeyValuesData->GetBool( "ShowClipsInWeaponDetail", 1 );
	m_bShowClipsDoubled = pKeyValuesData->GetBool( "ShowClipsDoubled", 0 );

	m_iSquadEmote = pKeyValuesData->GetInt("SquadEmote", -1);

	m_bUseStrangeDeviceWorldOffsets = pKeyValuesData->GetBool( "StrangeDeviceUseWorldOffsets", false );
	for ( int i = 0; i < NELEMS( m_vecStrangeDeviceOffset ); i++ )
	{
		m_vecStrangeDeviceOffset[i].x = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dX", i ), 0.0f );
		m_vecStrangeDeviceOffset[i].y = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dY", i ), 0.0f );
		m_vecStrangeDeviceOffset[i].z = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dZ", i ), 0.0f );
		m_angStrangeDeviceAngle[i][PITCH] = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dPitch", i ), 0.0f );
		m_angStrangeDeviceAngle[i][YAW] = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dYaw", i ), 0.0f );
		m_angStrangeDeviceAngle[i][ROLL] = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dRoll", i ), 0.0f );
		V_strncpy( m_szStrangeDeviceBone[i], pKeyValuesData->GetString( "StrangeDevice%dBone", "ValveBiped.Bip01_R_Hand" ), sizeof( m_szStrangeDeviceBone[i] ) );
		m_vecStrangeDeviceOffsetWorld[i].x = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dWorldX", i ), 0.0f );
		m_vecStrangeDeviceOffsetWorld[i].y = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dWorldY", i ), 0.0f );
		m_vecStrangeDeviceOffsetWorld[i].z = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dWorldZ", i ), 0.0f );
		m_angStrangeDeviceAngleWorld[i][PITCH] = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dWorldPitch", i ), 0.0f );
		m_angStrangeDeviceAngleWorld[i][YAW] = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dWorldYaw", i ), 0.0f );
		m_angStrangeDeviceAngleWorld[i][ROLL] = pKeyValuesData->GetFloat( UTIL_VarArgs( "StrangeDevice%dWorldRoll", i ), 0.0f );
		V_strncpy( m_szStrangeDeviceBoneWorld[i], pKeyValuesData->GetString( "StrangeDevice%dWorldBone", "ValveBiped.Bip01_R_Hand" ), sizeof( m_szStrangeDeviceBone[i] ) );
	}
}

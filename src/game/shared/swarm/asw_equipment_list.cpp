#include "cbase.h"
#include "asw_equipment_list.h"
#include "filesystem.h"
#include "weapon_parse.h"
#include <KeyValues.h>
#ifdef CLIENT_DLL
	#include <vgui/ISurface.h>
	#include <vgui/ISystem.h>
	#include <vgui_controls/Panel.h>
#endif
#include "asw_weapon_parse.h"
#include "asw_gamerules.h"
#include "asw_util_shared.h"
#include "gamestringpool.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_EquipItem::CASW_EquipItem( int iItemIndex, const char *szEquipClass, bool bSelectableInBriefing )
	: m_iItemIndex{ iItemIndex },
	m_szEquipClass{ szEquipClass },
	m_bSelectableInBriefing{ bSelectableInBriefing },
	m_EquipClass{ NULL_STRING }
{
}

static CASW_EquipItem s_RegularEquips[ASW_NUM_EQUIP_REGULAR] =
{
	{ ASW_EQUIP_RIFLE, "asw_weapon_rifle", true },
	{ ASW_EQUIP_PRIFLE, "asw_weapon_prifle", true },
	{ ASW_EQUIP_AUTOGUN, "asw_weapon_autogun", true },
	{ ASW_EQUIP_VINDICATOR, "asw_weapon_vindicator", true },
	{ ASW_EQUIP_PISTOL, "asw_weapon_pistol", true },
	{ ASW_EQUIP_SENTRY, "asw_weapon_sentry", true },
	{ ASW_EQUIP_HEAL_GRENADE, "asw_weapon_heal_grenade", true },
	{ ASW_EQUIP_AMMO_SATCHEL, "asw_weapon_ammo_satchel", true },

	{ ASW_EQUIP_SHOTGUN, "asw_weapon_shotgun", true },
	{ ASW_EQUIP_TESLA_GUN, "asw_weapon_tesla_gun", true },
	{ ASW_EQUIP_RAILGUN, "asw_weapon_railgun", true },
	{ ASW_EQUIP_HEAL_GUN, "asw_weapon_heal_gun", true },
	{ ASW_EQUIP_PDW, "asw_weapon_pdw", true },
	{ ASW_EQUIP_FLAMER, "asw_weapon_flamer", true },
	{ ASW_EQUIP_SENTRY_FREEZE, "asw_weapon_sentry_freeze", true },
	{ ASW_EQUIP_MINIGUN, "asw_weapon_minigun", true },
	{ ASW_EQUIP_SNIPER_RIFLE, "asw_weapon_sniper_rifle", true },
	{ ASW_EQUIP_SENTRY_FLAMER, "asw_weapon_sentry_flamer", true },
	{ ASW_EQUIP_CHAINSAW, "asw_weapon_chainsaw", true },
	{ ASW_EQUIP_SENTRY_CANNON, "asw_weapon_sentry_cannon", true },
	{ ASW_EQUIP_GRENADE_LAUNCHER, "asw_weapon_grenade_launcher", true },
	{ ASW_EQUIP_DEAGLE, "asw_weapon_deagle", true },
	{ ASW_EQUIP_DEVASTATOR, "asw_weapon_devastator", true },
	{ ASW_EQUIP_COMBAT_RIFLE, "asw_weapon_combat_rifle", true },
	{ ASW_EQUIP_HEALAMP_GUN, "asw_weapon_healamp_gun", true },
	{ ASW_EQUIP_HEAVY_RIFLE, "asw_weapon_heavy_rifle", true },
	{ ASW_EQUIP_MEDRIFLE, "asw_weapon_medrifle", true },

	{ ASW_EQUIP_FIRE_EXTINGUISHER, "asw_weapon_fire_extinguisher", false },
	{ ASW_EQUIP_MINING_LASER, "asw_weapon_mining_laser", false },
	{ ASW_EQUIP_50CALMG, "asw_weapon_50calmg", false },
	{ ASW_EQUIP_FLECHETTE, "asw_weapon_flechette", false },
	{ ASW_EQUIP_RICOCHET, "asw_weapon_ricochet", false },
	{ ASW_EQUIP_AMMO_BAG, "asw_weapon_ammo_bag", false },
	{ ASW_EQUIP_MEDICAL_SATCHEL, "asw_weapon_medical_satchel", false },
	{ ASW_EQUIP_AR2, "asw_weapon_ar2", false },
	//{ ASW_EQUIP_CRYOCANNON, "asw_weapon_cryocannon", false },
};

static CASW_EquipItem s_ExtraEquips[ASW_NUM_EQUIP_EXTRA] =
{
	{ ASW_EQUIP_MEDKIT, "asw_weapon_medkit", true },
	{ ASW_EQUIP_WELDER, "asw_weapon_welder", true },
	{ ASW_EQUIP_FLARES, "asw_weapon_flares", true },
	{ ASW_EQUIP_LASER_MINES, "asw_weapon_laser_mines", true },

	{ ASW_EQUIP_NORMAL_ARMOR, "asw_weapon_normal_armor", true },
	{ ASW_EQUIP_BUFF_GRENADE, "asw_weapon_buff_grenade", true },
	{ ASW_EQUIP_HORNET_BARRAGE, "asw_weapon_hornet_barrage", true },
	{ ASW_EQUIP_FREEZE_GRENADES, "asw_weapon_freeze_grenades", true },
	{ ASW_EQUIP_STIM, "asw_weapon_stim", true },
	{ ASW_EQUIP_TESLA_TRAP, "asw_weapon_tesla_trap", true },
	{ ASW_EQUIP_ELECTRIFIED_ARMOR, "asw_weapon_electrified_armor", true },
	{ ASW_EQUIP_MINES, "asw_weapon_mines", true },
	{ ASW_EQUIP_FLASHLIGHT, "asw_weapon_flashlight", true },
	{ ASW_EQUIP_FIST, "asw_weapon_fist", true },
	{ ASW_EQUIP_GRENADES, "asw_weapon_grenades", true },
	{ ASW_EQUIP_NIGHT_VISION, "asw_weapon_night_vision", true },
	{ ASW_EQUIP_SMART_BOMB, "asw_weapon_smart_bomb", true },
	{ ASW_EQUIP_GAS_GRENADES, "asw_weapon_gas_grenades", true },

	{ ASW_EQUIP_T75, "asw_weapon_t75", false },
	{ ASW_EQUIP_BLINK, "asw_weapon_blink", false },
	{ ASW_EQUIP_JUMP_JET, "asw_weapon_jump_jet", false },
	{ ASW_EQUIP_BAIT, "asw_weapon_bait", false },
};

CASW_EquipmentList::CASW_EquipmentList()
	: CAutoGameSystem{ "CASW_EquipmentList" }
{
#ifdef CLIENT_DLL
	m_bLoadedTextures = false;
#endif
}

void CASW_EquipmentList::LevelInitPreEntity()
{
	for ( int i = 0; i < NELEMS( s_RegularEquips ); i++ )
	{
		s_RegularEquips[i].m_EquipClass = AllocPooledString( s_RegularEquips[i].m_szEquipClass );
	}
	for ( int i = 0; i < NELEMS( s_ExtraEquips ); i++ )
	{
		s_ExtraEquips[i].m_EquipClass = AllocPooledString( s_ExtraEquips[i].m_szEquipClass );
	}
}

// gets the weapondata for a particular class of weapon (assumes the weapon has been precached already)
CASW_WeaponInfo *CASW_EquipmentList::GetWeaponDataFor( const char *szWeaponClass )
{
	WEAPON_FILE_INFO_HANDLE hWeaponFileInfo;
	ReadWeaponDataFromFileForSlot( filesystem, szWeaponClass, &hWeaponFileInfo, ASWGameRules() ? ASWGameRules()->GetEncryptionKey() : NULL );
	return assert_cast< CASW_WeaponInfo * >( GetFileWeaponInfoFromHandle( hWeaponFileInfo ) );
}

CASW_EquipItem *CASW_EquipmentList::GetRegular( int index )
{
	Assert( index >= 0 && index < NELEMS( s_RegularEquips ) );
	if ( index < 0 || index >= NELEMS( s_RegularEquips ) )
		return NULL;

	return &s_RegularEquips[index];
}

int CASW_EquipmentList::GetNumRegular( bool bIncludeHidden )
{
	return bIncludeHidden ? ASW_NUM_EQUIP_REGULAR : ASW_FIRST_HIDDEN_EQUIP_REGULAR;
}

CASW_EquipItem *CASW_EquipmentList::GetExtra( int index )
{
	Assert( index >= 0 && index < NELEMS( s_ExtraEquips ) );
	if ( index < 0 || index >= NELEMS( s_ExtraEquips ) )
		return NULL;

	return &s_ExtraEquips[index];
}

int CASW_EquipmentList::GetNumExtra( bool bIncludeHidden )
{
	return bIncludeHidden ? ASW_NUM_EQUIP_EXTRA : ASW_FIRST_HIDDEN_EQUIP_EXTRA;
}

CASW_EquipItem *CASW_EquipmentList::GetItemForSlot( int iWpnSlot, int index )
{
	return ( iWpnSlot <= 1 ) ? GetRegular( index ) : GetExtra( index );
}

int CASW_EquipmentList::GetRegularIndex( const char *szWeaponClass )
{
	for ( int i = 0; i < NELEMS( s_RegularEquips ); i++ )
	{
		if ( !V_stricmp( s_RegularEquips[i].m_szEquipClass, szWeaponClass ) )
			return i;
	}

	return -1;
}

int CASW_EquipmentList::GetExtraIndex( const char *szWeaponClass )
{
	for ( int i = 0; i < NELEMS( s_ExtraEquips ); i++ )
	{
		if ( !V_stricmp( s_ExtraEquips[i].m_szEquipClass, szWeaponClass ) )
			return i;
	}

	return -1;
}

int CASW_EquipmentList::GetIndexForSlot( int iWpnSlot, const char *szWeaponClass )
{
	return ( iWpnSlot <= 1 ) ? GetRegularIndex( szWeaponClass ) : GetExtraIndex( szWeaponClass );
}

// loads the weapon icon textures for all our equipment
#ifdef CLIENT_DLL
void CASW_EquipmentList::LoadTextures()
{
	if ( m_bLoadedTextures )
		return;

	LoadTextures( m_iRegularTexture, s_RegularEquips, NELEMS( s_RegularEquips ) );
	LoadTextures( m_iExtraTexture, s_ExtraEquips, NELEMS( s_ExtraEquips ) );
	m_bLoadedTextures = true;
}

void CASW_EquipmentList::LoadTextures( CUtlVector<int> &TextureIDs, CASW_EquipItem *pEquipItems, int nEquipItems )
{
	char buffer[256];

	TextureIDs.SetCount( nEquipItems );

	for ( int i = 0; i < nEquipItems; i++ )
	{
		int t = vgui::surface()->CreateNewTextureID();
		CASW_EquipItem *pItem = &pEquipItems[i];
		CASW_WeaponInfo *pWeaponData = GetWeaponDataFor( pItem->m_szEquipClass );
		gWR.LoadWeaponSprites( LookupWeaponInfoSlot( pItem->m_szEquipClass ) ); // make sure we load in its .txt + sprites now too
		V_snprintf( buffer, sizeof( buffer ), "vgui/%s", pWeaponData->szEquipIcon );
		vgui::surface()->DrawSetTextureFile( t, buffer, true, false );
		TextureIDs[i] = t;
	}
}

int CASW_EquipmentList::GetEquipIconTexture( bool bRegular, int iIndex )
{
	if ( !m_bLoadedTextures )
		LoadTextures();

	if ( iIndex < 0 )
		return -1;

	if ( bRegular )
	{
		if ( iIndex > m_iRegularTexture.Count() )
			return -1;
		return m_iRegularTexture[iIndex];
	}

	if ( iIndex > m_iExtraTexture.Count() )
		return -1;
	return m_iExtraTexture[iIndex];
}
#endif

CASW_EquipmentList g_ASWEquipmentList;

#ifdef CLIENT_DLL
CON_COMMAND( asw_list_equipment, "list weapons and their IDs" )
{
	Msg( "Regular\n");
	int nRegular = g_ASWEquipmentList.GetNumRegular( true );
	for ( int i = 0; i < nRegular; i++ )
	{
		CASW_EquipItem *pRegular = g_ASWEquipmentList.GetRegular( i );
		Assert( pRegular );
		Msg( "%d: %s%s\n", i, pRegular->m_szEquipClass, pRegular->m_bSelectableInBriefing ? "" : " (hidden)" );
	}
	Msg( "\n" );

	Msg( "Extra\n" );
	int nExtra = g_ASWEquipmentList.GetNumExtra( true );
	for ( int i = 0; i < nExtra; i++ )
	{
		CASW_EquipItem *pExtra = g_ASWEquipmentList.GetExtra( i );
		Assert( pExtra );
		Msg( "%d: %s%s\n", i, pExtra->m_szEquipClass, pExtra->m_bSelectableInBriefing ? "" : " (hidden)" );
	}
}
#endif

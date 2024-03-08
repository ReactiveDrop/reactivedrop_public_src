#include "cbase.h"
#include "rd_inventory_shared.h"
#include "rd_lobby_utils.h"
#include "asw_util_shared.h"
#include "asw_equipment_list.h"
#include "asw_deathmatch_mode_light.h"
#include "asw_gamerules.h"
#include "GameEventListener.h"
#include "fmtstr.h"
#include "jsmn.h"
#include <ctime>

#ifdef CLIENT_DLL
#include <vgui/IImage.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include "MultiFontRichText.h"
#include "rd_png_texture.h"
#include "c_asw_player.h"
#include "c_asw_marine_resource.h"
#include "c_asw_marine.h"
#include "c_asw_weapon.h"
#include "c_asw_sentry_base.h"
#include "c_asw_sentry_top.h"
#include "c_asw_game_resource.h"
#include "asw_equipment_list.h"
#include "rd_workshop.h"
#include "rd_missions_shared.h"
#include "rd_loadout.h"
#include "asw_deathmatch_mode_light.h"
#include "gameui/swarm/vgenericconfirmation.h"
#include "gameui/swarm/vitemshowcase.h"
#include "filesystem.h"
#include "c_user_message_register.h"
#include "asw_hud_3dmarinenames.h"
#include "rd_collections.h"
#include "rd_hoiaf_utils.h"
#include "rd_inventory_command.h"
#define CASW_Sentry_Base C_ASW_Sentry_Base
#define CASW_Sentry_Top C_ASW_Sentry_Top
#else
#include "asw_player.h"
#include "asw_marine_resource.h"
#include "asw_marine.h"
#include "asw_weapon.h"
#include "asw_sentry_base.h"
#include "asw_sentry_top.h"
#include "asw_game_resource.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


COMPILE_TIME_ASSERT( RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL + RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS == RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_PLAYER );
#pragma warning(push)
#pragma warning(disable: 4130) // we're comparing string literals, but if the comparison fails due to memory weirdness, it'll fail at compile time, so it's fine
COMPILE_TIME_ASSERT( ReactiveDropInventory::g_PlayerInventorySlotNames[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL] == "medal" );
#pragma warning(pop)

#ifdef CLIENT_DLL
ConVar rd_debug_inventory( "cl_debug_inventory", "0", FCVAR_NONE, "print debugging messages about inventory service calls" );
#define GET_INVENTORY_OR_BAIL \
	ISteamInventory *pInventory = SteamInventory(); \
	Assert( pInventory ); \
	if ( !pInventory ) \
		return

extern ConVar rd_strange_device_tier_notifications;
extern ConVar rd_equipped_medal[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS];
#else
ConVar rd_debug_inventory( "sv_debug_inventory", "0", FCVAR_NONE, "print debugging messages about inventory service calls" );
#define GET_INVENTORY_OR_BAIL \
	ISteamInventory *pInventory = engine->IsDedicatedServer() ? SteamGameServerInventory() : SteamInventory(); \
	Assert( pInventory ); \
	if ( !pInventory ) \
		return

extern ConVar rd_dedicated_server_language;
#endif

static CUtlMap<SteamItemDef_t, ReactiveDropInventory::ItemDef_t *> s_ItemDefs( DefLessFunc( SteamItemDef_t ) );
static KeyValues *s_pItemDefCache = NULL;
static bool s_bLoadedItemDefs = false;

#ifdef CLIENT_DLL
static void ResetAccessoryIconsOnMaterialsReleased( int nChangeFlags )
{
	if ( nChangeFlags & MATERIAL_RESTORE_VERTEX_FORMAT_CHANGED )
	{
		FOR_EACH_MAP_FAST( s_ItemDefs, i )
		{
			// we will re-fetch the texture the next time it is to be displayed
			s_ItemDefs[i]->AccessoryIcon = NULL;
		}
	}
}
#endif

static const char s_szHexDigits[] = "0123456789abcdef";

static class CRD_Inventory_Manager final : public CAutoGameSystem, public CGameEventListener
{
public:
	CRD_Inventory_Manager() : CAutoGameSystem( "CRD_Inventory_Manager" )
	{
	}

	void PostInit() override
	{
#ifdef CLIENT_DLL
		if ( SteamUser() )
		{
			CFmtStr szCacheFileName{ "cfg/clienti_%llu.dat", SteamUser()->GetSteamID().ConvertToUint64() };
			CUtlBuffer buf;
			if ( g_pFullFileSystem->ReadFile( szCacheFileName, "MOD", buf ) )
			{
				KeyValues::AutoDelete pCache{ "IC" };
				bool bOK = pCache->ReadAsBinary( buf );
				Assert( bOK );
				if ( bOK )
				{
					FOR_EACH_SUBKEY( pCache, pItem )
					{
						m_LocalInventoryCache.AddToTail( ReactiveDropInventory::ItemInstance_t{ pItem } );
					}

					if ( rd_debug_inventory.GetBool() )
					{
						Msg( "Loaded %d items from inventory cache.\n", m_LocalInventoryCache.Count() );
					}
				}
			}
		}
#endif

		ISteamInventory *pInventory = SteamInventory();
#ifdef GAME_DLL
		if ( engine->IsDedicatedServer() )
		{
			pInventory = SteamGameServerInventory();
		}
#endif
		if ( !pInventory )
		{
			Warning( "Cannot access ISteamInventory!\n" );
			return;
		}

		if ( !pInventory->LoadItemDefinitions() )
		{
			Warning( "Failed to load inventory item definitions!\n" );
		}

		ListenForGameEvent( "mission_success" );
		ListenForGameEvent( "mission_failed" );
		ListenForGameEvent( "asw_mission_restart" );
		ListenForGameEvent( "fast_hack_success" );
		ListenForGameEvent( "entity_frozen" );
		ListenForGameEvent( "marine_infested_cured" );
		ListenForGameEvent( "marine_extinguished" );
		ListenForGameEvent( "marine_healed" );

#ifdef CLIENT_DLL
		pInventory->GetAllItems( &m_GetFullInventoryForCacheResult );

		materials->AddReleaseFunc( &ResetAccessoryIconsOnMaterialsReleased );

		CRD_PNG_Texture::CleanLocalCachedTextures( "vgui/inventory/cache" );
#endif
	}

	void LevelInitPreEntity() override
	{
		ISteamInventory *pInventory = SteamInventory();
#ifdef GAME_DLL
		if ( engine->IsDedicatedServer() )
		{
			pInventory = SteamGameServerInventory();
		}
#endif
		if ( !pInventory )
		{
			Warning( "Cannot access ISteamInventory!\n" );
			return;
		}

		if ( m_flDefsUpdateTime < Plat_FloatTime() - 3600.0f )
		{
			m_flDefsUpdateTime = Plat_FloatTime();

			if ( !pInventory->LoadItemDefinitions() )
			{
				Warning( "Failed to load inventory item definitions!\n" );
			}
		}

#ifdef CLIENT_DLL
		m_iPlayerEquipmentCommand = 0;
#endif
	}

	void LevelInitPostEntity() override
	{
#ifdef CLIENT_DLL
		if ( ValidatePlayerEquipmentResult() )
		{
			SendPlayerEquipmentToServer();
		}
#endif
	}

	void LevelShutdownPreEntity() override
	{
		ISteamInventory *pInventory = SteamInventory();
#ifdef GAME_DLL
		if ( engine->IsDedicatedServer() )
		{
			pInventory = SteamGameServerInventory();
		}
#endif
		if ( !pInventory )
		{
			Warning( "Cannot access ISteamInventory!\n" );
			return;
		}

#ifdef CLIENT_DLL
		CommitDynamicProperties();
#endif
	}

#ifdef CLIENT_DLL
	void CacheUserInventory( SteamInventoryResult_t hResult )
	{
		ISteamInventory *pInventory = SteamInventory();
		if ( !pInventory )
		{
			Warning( "Failed to cache user inventory for offline play: no ISteamInventory\n" );
			return;
		}
		ISteamUser *pUser = SteamUser();
		if ( !pUser )
		{
			Warning( "Failed to cache user inventory for offline play: no ISteamUser\n" );
			return;
		}

		uint32 nItems{};
		if ( !pInventory->GetResultItems( hResult, NULL, &nItems ) )
		{
			Warning( "Failed to retrieve item count from inventory result for cache\n" );
			return;
		}

		m_LocalInventoryCache.Purge();
		m_LocalInventoryCache.EnsureCapacity( nItems );

		m_HighOwnedInventoryDefIDs.Purge();

		KeyValues::AutoDelete pCache{ "IC" };

		for ( uint32 i = 0; i < nItems; i++ )
		{
			ReactiveDropInventory::ItemInstance_t instance{ hResult, i };
			m_LocalInventoryCache.AddToTail( instance );
			pCache->AddSubKey( instance.ToKeyValues() );

			// precache item def + icon
			( void )ReactiveDropInventory::GetItemDef( instance.ItemDefID );

			// The Steam inventory API doesn't list item def IDs that are between 1 million and 1 billion unless we ask about them specifically.
			// If we own any items with def IDs in this range, remember that and re-write the schema cache to include them.
			// These IDs are used for donation receipt medals as well as unique medals.
			if ( instance.ItemDefID >= 1000000 )
			{
				if ( !m_HighOwnedInventoryDefIDs.IsValidIndex( m_HighOwnedInventoryDefIDs.Find( instance.ItemDefID ) ) )
					m_HighOwnedInventoryDefIDs.AddToTail( instance.ItemDefID );
			}
		}

		CUtlBuffer buf;
		if ( !pCache->WriteAsBinary( buf ) )
		{
			Warning( "Failed to serialize inventory cache\n" );
			return;
		}

		CFmtStr szCacheFileName{ "cfg/clienti_%llu.dat", pUser->GetSteamID().ConvertToUint64() };
		if ( !g_pFullFileSystem->WriteFile( szCacheFileName, "MOD", buf ) )
		{
			Warning( "Failed to write inventory cache\n" );
			return;
		}

		if ( rd_debug_inventory.GetBool() )
		{
			Msg( "Successfully wrote inventory cache with %d items\n", nItems );
		}

		if ( m_HighOwnedInventoryDefIDs.Count() )
		{
			CacheItemSchema();
		}

		HoIAF()->RebuildNotificationList();
	}

	void CacheItemSchema()
	{
		ISteamInventory *pInventory = SteamInventory();
		if ( !pInventory )
		{
			Warning( "Failed to cache item schema for offline play: no ISteamInventory\n" );
			return;
		}

		KeyValues::AutoDelete pCache{ "IS" };

		uint32 nItemDefs{};
		pInventory->GetItemDefinitionIDs( NULL, &nItemDefs );
		CUtlVector<SteamItemDef_t> ItemDefIDs;
		ItemDefIDs.AddMultipleToTail( nItemDefs );
		pInventory->GetItemDefinitionIDs( ItemDefIDs.Base(), &nItemDefs );

		ItemDefIDs.AddVectorToTail( m_HighOwnedInventoryDefIDs );

		CUtlMemory<char> szStringBuf( 0, 1024 );
		uint32 size{};

		uint32 nSkippedDefs = 0;
		const char *szUserLanguage = SteamApps()->GetCurrentGameLanguage();

		FOR_EACH_VEC( ItemDefIDs, i )
		{
			size = szStringBuf.Count();
			pInventory->GetItemDefinitionProperty( ItemDefIDs[i], "type", szStringBuf.Base(), &size );
			if ( !V_strcmp( szStringBuf.Base(), "bundle" ) ||
				!V_strcmp( szStringBuf.Base(), "generator" ) ||
				!V_strcmp( szStringBuf.Base(), "playtimegenerator" ) )
			{
				// don't need these item types offline
				nSkippedDefs++;
				continue;
			}

			KeyValues *pDef = new KeyValues{ "d" };

			pInventory->GetItemDefinitionProperty( ItemDefIDs[i], NULL, NULL, &size );
			szStringBuf.EnsureCapacity( size );
			size = szStringBuf.Count();
			pInventory->GetItemDefinitionProperty( ItemDefIDs[i], NULL, szStringBuf.Base(), &size );

			CSplitString PropertyNames{ szStringBuf.Base(), "," };
			FOR_EACH_VEC( PropertyNames, j )
			{
				if ( !V_strcmp( PropertyNames[j], "appid" ) ||
					!V_strcmp( PropertyNames[j], "Timestamp" ) ||
					!V_strcmp( PropertyNames[j], "modified" ) ||
					!V_strcmp( PropertyNames[j], "date_created" ) ||
					!V_strcmp( PropertyNames[j], "quantity" ) ||
					!V_strcmp( PropertyNames[j], "exchange" ) ||
					!V_strcmp( PropertyNames[j], "allowed_tags_from_tools" ) ||
					!V_strcmp( PropertyNames[j], "tradable" ) ||
					!V_strcmp( PropertyNames[j], "marketable" ) ||
					!V_strcmp( PropertyNames[j], "commodity" ) )
				{
					// don't need these properties offline
					continue;
				}

				// only cache english (fallback) and the current language preference so we don't waste space on strings we probably won't use
				const char *szAfterPrefix;
#define CHECK_LANGUAGE_PREFIX( szPrefix ) \
				szAfterPrefix = StringAfterPrefixCaseSensitive( PropertyNames[j], szPrefix ); \
				if ( szAfterPrefix && V_strcmp( szAfterPrefix, "english" ) && V_strcmp( szAfterPrefix, szUserLanguage ) ) \
					continue

				CHECK_LANGUAGE_PREFIX( "name_" );
				CHECK_LANGUAGE_PREFIX( "briefing_name_" );
				CHECK_LANGUAGE_PREFIX( "description_" );
				CHECK_LANGUAGE_PREFIX( "ingame_description_" );
				CHECK_LANGUAGE_PREFIX( "before_description_" );
				CHECK_LANGUAGE_PREFIX( "after_description_" );
				CHECK_LANGUAGE_PREFIX( "accessory_description_" );
				CHECK_LANGUAGE_PREFIX( "display_type_" );

				for ( int k = 0; k < RD_STEAM_INVENTORY_ITEM_MAX_STYLES; k++ )
				{
					CHECK_LANGUAGE_PREFIX( CFmtStr( "style_%d_name_", k ) );
				}

				for ( int k = 0; k < RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS_PER_ACCESSORY; k++ )
				{
					CHECK_LANGUAGE_PREFIX( CFmtStr( "notification_name_%d_", k ) );
				}

#undef CHECK_LANGUAGE_PREFIX

				pInventory->GetItemDefinitionProperty( ItemDefIDs[i], PropertyNames[j], NULL, &size );
				szStringBuf.EnsureCapacity( size );
				size = szStringBuf.Count();
				pInventory->GetItemDefinitionProperty( ItemDefIDs[i], PropertyNames[j], szStringBuf.Base(), &size );

				if ( szStringBuf.Base()[0] == '\0' )
				{
					// don't waste space storing empty string
					continue;
				}

				pDef->SetString( PropertyNames[j], szStringBuf.Base() );
			}

			pCache->AddSubKey( pDef );
		}

		CUtlBuffer buf;
		if ( !pCache->WriteAsBinary( buf ) )
		{
			Warning( "Failed to serialize item schema cache\n" );
			return;
		}

		if ( !g_pFullFileSystem->WriteFile( "cfg/item_schema_cache.dat", "MOD", buf ) )
		{
			Warning( "Failed to write item schema cache\n" );
			return;
		}

		if ( rd_debug_inventory.GetBool() )
		{
			Msg( "Successfully wrote item schema cache with %d items (skipped %d)\n", nItemDefs - nSkippedDefs, nSkippedDefs );
		}
	}

	CUtlVector<SteamItemDef_t> m_HighOwnedInventoryDefIDs;

	struct PendingDynamicPropertyUpdate_t
	{
		SteamItemInstanceID_t ItemInstanceID{ k_SteamItemInstanceIDInvalid };
		SteamItemDef_t ItemDefID{};
		int PropertyIndex{};
		int64_t NewValue{};
	};
	CUtlVector<PendingDynamicPropertyUpdate_t> m_PendingDynamicPropertyUpdates{};
	SteamInventoryResult_t m_DynamicPropertyUpdateResult{ k_SteamInventoryResultInvalid };
	bool m_bWantExtraDynamicPropertyCommit{};

	void CommitDynamicProperties()
	{
		if ( m_PendingDynamicPropertyUpdates.Count() == 0 )
		{
			if ( rd_debug_inventory.GetBool() )
			{
				Msg( "[C] Not committing dynamic property update (no properties changed)\n" );
			}

			return;
		}

		if ( m_DynamicPropertyUpdateResult != k_SteamInventoryResultInvalid )
		{
			if ( rd_debug_inventory.GetBool() )
			{
				Msg( "[C] Not committing dynamic property update (%s)\n", m_bWantExtraDynamicPropertyCommit ? "request in flight and repeat already queued" : "request in flight, queueing repeat" );
			}

			m_bWantExtraDynamicPropertyCommit = true;

			return;
		}

		ISteamInventory *pInventory = SteamInventory();
		Assert( pInventory );
		if ( !pInventory )
		{
			Warning( "Cannot commit dynamic property updates: no inventory API\n" );
			return;
		}

		SteamInventoryUpdateHandle_t hUpdate = pInventory->StartUpdateProperties();

		if ( rd_debug_inventory.GetBool() )
		{
			Msg( "[C] Committing dynamic property update with %d changed properties (handle %016llx)\n", m_PendingDynamicPropertyUpdates.Count(), hUpdate );
		}

		FOR_EACH_VEC( m_PendingDynamicPropertyUpdates, i )
		{
			const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_PendingDynamicPropertyUpdates[i].ItemDefID );
			const char *szProperty = pDef->CompressedDynamicProps[m_PendingDynamicPropertyUpdates[i].PropertyIndex];
			if ( rd_debug_inventory.GetBool() )
			{
				Msg( "[C] Setting item %llu (#%d:%s) property %d:%s value to %lld\n", m_PendingDynamicPropertyUpdates[i].ItemInstanceID, m_PendingDynamicPropertyUpdates[i].ItemDefID, pDef->Name.Get(),
					m_PendingDynamicPropertyUpdates[i].PropertyIndex, szProperty, m_PendingDynamicPropertyUpdates[i].NewValue );
			}

			bool ok = pInventory->SetProperty( hUpdate, m_PendingDynamicPropertyUpdates[i].ItemInstanceID, szProperty, m_PendingDynamicPropertyUpdates[i].NewValue );
			Assert( ok );
			if ( !ok )
			{
				Warning( "ISteamInventory::SetProperty returned false\n" );
			}
		}

		bool ok = pInventory->SubmitUpdateProperties( hUpdate, &m_DynamicPropertyUpdateResult );
		Assert( ok );
		if ( !ok )
		{
			Warning( "ISteamInventory::SubmitUpdateProperties returned false\n" );
		}

		if ( rd_debug_inventory.GetBool() )
		{
			Msg( "[C] Item property update handle is %08x\n", m_DynamicPropertyUpdateResult );
		}

		m_PendingDynamicPropertyUpdates.Purge();
	}
#endif

	void IncrementStrangePropertyOnStartingItems( SteamItemDef_t iAccessoryID, int64_t iAmount, int iPropertyIndex = 0, bool bRelative = true, bool bAllowCheating = false )
	{
		Assert( iPropertyIndex >= 0 );

		CASW_Game_Resource *pGameResource = ASWGameResource();
		if ( !pGameResource )
		{
			if ( rd_debug_inventory.GetBool() )
			{
				Msg( "[%c] Cannot increment property on starting items: no game resource\n", IsClientDll() ? 'C' : 'S' );
			}

			return;
		}

		for ( int i = 1; i <= ( IsClientDll() ? 1 : gpGlobals->maxClients ); i++ )
		{
#ifdef CLIENT_DLL
			C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
#else
			CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
#endif
			if ( !pPlayer )
				continue;

			for ( int j = 0; j < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; j++ )
			{
				CRD_ItemInstance &medalInstance = pPlayer->m_EquippedItemData[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL + j];

				ModifyAccessoryDynamicPropValue( pPlayer->GetNPC(), pPlayer, medalInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
			}

			CASW_Marine_Resource *pMR = pGameResource->GetFirstMarineResourceForPlayer( pPlayer );
			if ( !pMR || pMR->m_OriginalCommander.Get() != pPlayer || ( !pMR->IsInhabited() && pMR->GetHealthPercent() > 0 ) )
				continue;

			pMR->ClearInvalidEquipData();

			for ( int j = 0; j < RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_MARINE_RESOURCE; j++ )
			{
				CRD_ItemInstance &equipInstance = pMR->m_EquippedItemData[j];

				ModifyAccessoryDynamicPropValue( pMR->GetMarineEntity(), pPlayer, equipInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
			}
		}
	}

	void IncrementStrangePropertyOnEquippedItems( CASW_Inhabitable_NPC *pNPC, SteamItemDef_t iAccessoryID, int64_t iAmount, int iPropertyIndex = 0, bool bRelative = true, bool bAllowCheating = false )
	{
		Assert( pNPC );
		if ( !pNPC || !pNPC->IsInhabited() || !pNPC->GetCommander() )
			return;

#ifdef CLIENT_DLL
		if ( !pNPC->GetCommander()->IsLocalPlayer() )
			return;
#endif

		for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
		{
			CRD_ItemInstance &medalInstance = pNPC->GetCommander()->m_EquippedItemData[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL + i];

			ModifyAccessoryDynamicPropValue( pNPC, pNPC->GetCommander(), medalInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
		}

		if ( CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC ) )
		{
			CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
			if ( pMR && pMR->m_OriginalCommander.Get() == pMR->m_Commander.Get() )
			{
				pMR->ClearInvalidEquipData();

				for ( int i = 0; i < RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_MARINE_RESOURCE; i++ )
				{
					CRD_ItemInstance &equipInstance = pMR->m_EquippedItemData[i];

					ModifyAccessoryDynamicPropValue( pMR->GetMarineEntity(), pMR->m_Commander.Get(), equipInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
				}
			}
		}
	}

	template<typename Weapon_t>
	void IncrementStrangePropertyOnWeaponAndGlobals( CASW_Inhabitable_NPC *pNPC, Weapon_t *pWeapon, SteamItemDef_t iAccessoryID, int64_t iAmount, int iPropertyIndex = 0, bool bRelative = true, bool bAllowCheating = false, bool bAllowBorrowed = false )
	{
		Assert( pNPC );
		if ( !pNPC || !pNPC->IsInhabited() || !pNPC->GetCommander() )
			return;

#ifdef CLIENT_DLL
		if ( !bAllowBorrowed && !pNPC->GetCommander()->IsLocalPlayer() )
			return;
#endif

		for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
		{
			CRD_ItemInstance &medalInstance = pNPC->GetCommander()->m_EquippedItemData[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL + i];

			ModifyAccessoryDynamicPropValue( pNPC, pNPC->GetCommander(), medalInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
		}

		if ( CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC ) )
		{
			CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
			if ( pMR && pMR->m_OriginalCommander.Get() == pMR->m_Commander.Get()
#ifdef CLIENT_DLL
				&& pMR->m_OriginalCommander->IsLocalPlayer()
#endif
				)
			{
				pMR->ClearInvalidEquipData();

				ModifyAccessoryDynamicPropValue( pMR->GetMarineEntity(), pMR->m_Commander.Get(), pMR->m_EquippedItemData.m_Suit, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
			}
		}

		if ( pWeapon && pWeapon->IsInventoryEquipSlotValid() && ( bAllowBorrowed || ( pWeapon->m_hOriginalOwnerMR && pWeapon->m_hOriginalOwnerMR->m_OriginalCommander.Get() == pNPC->GetCommander() ) ) )
		{
			CRD_ItemInstance &weaponInstance = pWeapon->m_hOriginalOwnerMR->m_EquippedItemData[pWeapon->m_iInventoryEquipSlot];
			Assert( weaponInstance.IsSet() );

			ModifyAccessoryDynamicPropValue( pNPC, pWeapon->m_hOriginalOwnerMR->m_OriginalCommander, weaponInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
		}
	}

	void IncrementStrangePropertiesForWeapon( CASW_Inhabitable_NPC *pNPC, CBaseEntity *pWeapon, SteamItemDef_t iAccessoryID, int64_t iAmount, int iPropertyIndex = 0, bool bRelative = true )
	{
		if ( CASW_Weapon *pASWWeapon = dynamic_cast< CASW_Weapon * >( pWeapon ) )
		{
			s_RD_Inventory_Manager.IncrementStrangePropertyOnWeaponAndGlobals( pNPC, pASWWeapon, iAccessoryID, iAmount, iPropertyIndex, bRelative );
		}
		else if ( CASW_Sentry_Base *pSentry = dynamic_cast< CASW_Sentry_Base * >( pWeapon ) )
		{
			s_RD_Inventory_Manager.IncrementStrangePropertyOnWeaponAndGlobals( pNPC, pSentry, iAccessoryID, iAmount, iPropertyIndex, bRelative );
		}
		else if ( CASW_Sentry_Top *pTop = dynamic_cast< CASW_Sentry_Top * >( pWeapon ) )
		{
			if ( CASW_Sentry_Base *pBase = pTop->GetSentryBase() )
			{
				s_RD_Inventory_Manager.IncrementStrangePropertyOnWeaponAndGlobals( pNPC, pBase, iAccessoryID, iAmount, iPropertyIndex, bRelative );
			}
		}
		else if ( IRD_Has_Projectile_Data *pProjectile = dynamic_cast< IRD_Has_Projectile_Data * >( pWeapon ) )
		{
			CRD_ProjectileData *pData = const_cast< CRD_ProjectileData * >( pProjectile->GetProjectileData() );
			if ( pData->m_bFiredByOwner )
			{
				s_RD_Inventory_Manager.IncrementStrangePropertyOnWeaponAndGlobals( pNPC, pData, iAccessoryID, iAmount, iPropertyIndex, bRelative );
			}
		}
		else if ( pWeapon && pWeapon->IsInhabitableNPC() )
		{
			// unarmed melee
			s_RD_Inventory_Manager.IncrementStrangePropertyOnWeaponAndGlobals< CASW_Weapon >( pNPC, NULL, iAccessoryID, iAmount, iPropertyIndex, bRelative );
		}
#ifdef DBGFLAG_ASSERT
		else if ( pWeapon )
		{
			Assert( !"Unhandled weapon type" );
		}
#endif
	}

	bool ModifyAccessoryDynamicPropValue( CASW_Inhabitable_NPC *pNPC, CASW_Player *pOwner, CRD_ItemInstance &instance, SteamItemDef_t iAccessoryID, int iPropertyIndex, int64_t iAmount, bool bRelative = true, bool bAllowCheating = false )
	{
		if ( !instance.IsSet() )
		{
			// no item in slot
			return false;
		}

		if ( !bAllowCheating && ASWGameRules() && ASWGameRules()->m_bCheated )
		{
			// can't affect strange stats while cheating
			return false;
		}

		if ( !s_bLoadedItemDefs )
		{
			if ( rd_debug_inventory.GetBool() )
			{
				static int s_nOfflineWarnings = 0;
				if ( s_nOfflineWarnings < 10 )
				{
					s_nOfflineWarnings++;
					Msg( "[%c] Cannot modify dynamic properties in offline mode.\n", IsClientDll() ? 'C' : 'S' );
				}
			}

			return false;
		}

		const ReactiveDropInventory::ItemDef_t *pItemDef = ReactiveDropInventory::GetItemDef( instance.m_iItemDefID );
		const ReactiveDropInventory::ItemDef_t *pAccessoryDef = ReactiveDropInventory::GetItemDef( iAccessoryID );
		Assert( pItemDef );
		Assert( pAccessoryDef );
		if ( !pItemDef || !pAccessoryDef )
			return false;

		Assert( iPropertyIndex >= 0 && iPropertyIndex < pAccessoryDef->CompressedDynamicProps.Count() );
		const char *szPropertyName = pAccessoryDef->CompressedDynamicProps[iPropertyIndex];
		int iCombinedIndex = iPropertyIndex;

		ReactiveDropInventory::ItemInstance_t *pLocalInstance = NULL;
#ifdef CLIENT_DLL
		pLocalInstance = ReactiveDropInventory::GetLocalItemCacheForModify( instance.m_iItemInstanceID );
		Assert( pLocalInstance );
		if ( !pLocalInstance )
		{
			if ( rd_debug_inventory.GetBool() )
			{
				Msg( "[C] Could not find local instance of item %llu '%s' for property update.\n", instance.m_iItemInstanceID, pItemDef->Name.Get() );
			}

			return false;
		}
#endif

		if ( instance.m_iItemDefID == iAccessoryID )
		{
			ModifyDynamicPropValueHelper( pNPC, pOwner, instance, iAccessoryID, iPropertyIndex, iCombinedIndex, szPropertyName, pItemDef, pAccessoryDef, pLocalInstance, iAmount, bRelative );
			return true;
		}

		iCombinedIndex += pItemDef->CompressedDynamicProps.Count();

		FOR_EACH_VEC( instance.m_iAccessory, i )
		{
			if ( instance.m_iAccessory[i] == iAccessoryID )
			{
				ModifyDynamicPropValueHelper( pNPC, pOwner, instance, iAccessoryID, iPropertyIndex, iCombinedIndex, szPropertyName, pItemDef, pAccessoryDef, pLocalInstance, iAmount, bRelative );
				return true;
			}

			if ( instance.m_iAccessory[i] )
			{
				const ReactiveDropInventory::ItemDef_t *pOtherAccessory = ReactiveDropInventory::GetItemDef( instance.m_iAccessory[i] );
				Assert( pOtherAccessory );
				if ( !pOtherAccessory )
					return false;

				iCombinedIndex += pOtherAccessory->CompressedDynamicProps.Count();
			}
		}

		// no such accessory on item
		return false;
	}

	static void UpdateCounterForAllInstancesOfThisItem( CRD_ItemInstance &instance, int iCombinedIndex, int64_t iCounterBefore, int64_t iCounterAfter )
	{
		Assert( instance.IsSet() );
		if ( !instance.IsSet() )
			return;

		Assert( instance.m_nCounter[iCombinedIndex] == iCounterBefore );

		int iFound = 0;
#define CHECK( inst ) \
		if ( inst.m_iItemInstanceID == instance.m_iItemInstanceID ) \
		{ \
			if ( &inst == &instance ) \
			{ \
				iFound++; \
			} \
			Assert( inst.m_nCounter[iCombinedIndex] == iCounterBefore ); \
			inst.m_nCounter.Set( iCombinedIndex, iCounterAfter ); \
		}

		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
			if ( !pPlayer )
				continue;

			COMPILE_TIME_ASSERT( RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_PLAYER == 3 );
			CHECK( pPlayer->m_EquippedItemData.m_Medal0 );
			CHECK( pPlayer->m_EquippedItemData.m_Medal1 );
			CHECK( pPlayer->m_EquippedItemData.m_Medal2 );
		}

		if ( CASW_Game_Resource *pGameResource = ASWGameResource() )
		{
			for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
				if ( !pMR )
					continue;

				COMPILE_TIME_ASSERT( RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_MARINE_RESOURCE == 4 );
				CHECK( pMR->m_EquippedItemData.m_Suit );
				CHECK( pMR->m_EquippedItemData.m_Weapon1 );
				CHECK( pMR->m_EquippedItemData.m_Weapon2 );
				CHECK( pMR->m_EquippedItemData.m_Extra );
			}
		}

#undef CHECK

		Assert( iFound == 1 );
		if ( !iFound )
		{
			instance.m_nCounter.Set( iCombinedIndex, iCounterAfter );
		}
	}

	void ModifyDynamicPropValueHelper( CASW_Inhabitable_NPC *pNPC, CASW_Player *pOwner, CRD_ItemInstance &instance, SteamItemDef_t iAccessoryID, int iPropertyIndex, int iCombinedIndex, const char *szPropertyName,
		const ReactiveDropInventory::ItemDef_t *pItemDef, const ReactiveDropInventory::ItemDef_t *pAccessoryDef, ReactiveDropInventory::ItemInstance_t *pLocalInstance, int64_t iAmount, bool bRelative )
	{
		int64_t iCounterBefore = instance.m_nCounter[iCombinedIndex];
#ifdef CLIENT_DLL
		if ( engine->IsPlayingDemo() )
		{
			// watching a recording
			return;
		}

		PendingDynamicPropertyUpdate_t *pUpdate = NULL;
		FOR_EACH_VEC( m_PendingDynamicPropertyUpdates, j )
		{
			if ( m_PendingDynamicPropertyUpdates[j].ItemInstanceID == instance.m_iItemInstanceID && m_PendingDynamicPropertyUpdates[j].ItemDefID == iAccessoryID && m_PendingDynamicPropertyUpdates[j].PropertyIndex == iPropertyIndex )
			{
				pUpdate = &m_PendingDynamicPropertyUpdates[j];
				iCounterBefore = pUpdate->NewValue;
				break;
			}
		}
		if ( !pUpdate )
		{
			pUpdate = &m_PendingDynamicPropertyUpdates[m_PendingDynamicPropertyUpdates.AddToTail()];
			pUpdate->ItemInstanceID = instance.m_iItemInstanceID;
			pUpdate->ItemDefID = iAccessoryID;
			pUpdate->PropertyIndex = iPropertyIndex;
			iCounterBefore = strtoll( pLocalInstance->DynamicProps[szPropertyName], NULL, 10 );
		}
#endif
		int64_t iCounterAfter = iCounterBefore;
		if ( !bRelative )
		{
			iCounterAfter = iAmount;
		}
		else if ( iCounterBefore > 0 )
		{
			if ( iAmount > INT64_MAX - iCounterBefore )
				iCounterAfter = INT64_MAX;
			else
				iCounterAfter += iAmount;
		}
		else
		{
			if ( iAmount < INT64_MIN - iCounterBefore )
				iCounterAfter = INT64_MIN;
			else
				iCounterAfter += iAmount;
		}

		int iTierBefore = iPropertyIndex == 0 ? pAccessoryDef->GetStrangeTier( iCounterBefore ) : -1;
		int iTierAfter = iPropertyIndex == 0 ? pAccessoryDef->GetStrangeTier( iCounterAfter ) : -1;

		if ( iAccessoryID == 5007 && iPropertyIndex == 0 ) // 5007 = Alien Kill Streak
		{
			int64_t iBestStreak = instance.m_nCounter[iCombinedIndex + 1];
#ifdef CLIENT_DLL
			bool bFoundBestStreak = false;
			FOR_EACH_VEC( m_PendingDynamicPropertyUpdates, j )
			{
				if ( m_PendingDynamicPropertyUpdates[j].ItemInstanceID == instance.m_iItemInstanceID && m_PendingDynamicPropertyUpdates[j].ItemDefID == iAccessoryID && m_PendingDynamicPropertyUpdates[j].PropertyIndex == iPropertyIndex + 1 )
				{
					iBestStreak = pUpdate->NewValue;
					bFoundBestStreak = true;
					break;
				}
			}
			if ( !bFoundBestStreak )
			{
				iBestStreak = strtoll( pLocalInstance->DynamicProps[pAccessoryDef->CompressedDynamicProps[1]], NULL, 10 );
			}
#endif

			if ( iBestStreak < iCounterAfter )
			{
				ModifyAccessoryDynamicPropValue( pNPC, pOwner, instance, iAccessoryID, 1, iCounterAfter, false );
			}
			else
			{
				iTierAfter = -1;
			}
		}

#ifdef CLIENT_DLL
		pLocalInstance->DynamicProps[szPropertyName] = CFmtStr( "%lld", iCounterAfter );
		pUpdate->NewValue = iCounterAfter;
#endif
		UpdateCounterForAllInstancesOfThisItem( instance, iCombinedIndex, iCounterBefore, iCounterAfter );

		if ( iTierBefore < iTierAfter && pNPC && pOwner )
		{
#ifdef CLIENT_DLL
			if ( CASWHud3DMarineNames *pMarineNames = assert_cast< CASWHud3DMarineNames * >( GetHud().FindElement( "ASWHud3DMarineNames" ) ) )
			{
				pMarineNames->OnStrangeDeviceTierNotification( pOwner, pNPC, instance.m_iItemDefID, iAccessoryID, iPropertyIndex, iCounterAfter );
			}
#else
			CReliableBroadcastRecipientFilter filter;
			filter.UsePredictionRules();
			UserMessageBegin( filter, "RDStrangeDeviceTier" );
				WRITE_UBITLONG( pOwner->GetClientIndex(), 6 );
				WRITE_ENTITY( pNPC->entindex() );
				WRITE_UBITLONG( instance.m_iItemDefID, RD_ITEM_ID_BITS );
				WRITE_UBITLONG( iAccessoryID, RD_ITEM_ACCESSORY_BITS );
				WRITE_UBITLONG( iPropertyIndex, 2 ); // 1 extra bit for possible future use
				WRITE_BITS( &iCounterAfter, 64 );
			MessageEnd();
#endif
		}

		if ( rd_debug_inventory.GetBool() )
		{
			Msg( "[%c] Item %llu #%d '%s' dynamic property '%s' '%s' changed (%s) %+lld from %lld (tier %d) to %lld (tier %d).\n", IsClientDll() ? 'C' : 'S', instance.m_iItemInstanceID, instance.m_iItemDefID, pItemDef->Name.Get(), pAccessoryDef->Name.Get(), szPropertyName, bRelative ? "relative" : "absolute", iAmount, iCounterBefore, iTierBefore, iCounterAfter, iTierAfter );
		}
	}

	static CBaseEntity *Ent( int index )
	{
		if ( index == -1 )
			return NULL;

		return CBaseEntity::Instance( index );
	}

	void FireGameEvent( IGameEvent *event ) override
	{
		if ( !ASWDeathmatchMode() )
		{
			if ( FStrEq( event->GetName(), "mission_success" ) )
			{
				IncrementStrangePropertyOnStartingItems( 5000, 1 ); // Missions
				IncrementStrangePropertyOnStartingItems( 5001, 1 ); // Successful Missions
				return;
			}

			if ( FStrEq( event->GetName(), "mission_failed" ) )
			{
				IncrementStrangePropertyOnStartingItems( 5000, 1 ); // Missions
				return;
			}

			if ( FStrEq( event->GetName(), "fast_hack_success" ) )
			{
				CASW_Marine *pMarine = CASW_Marine::AsMarine( Ent( event->GetInt( "marine" ) ) );
				if ( pMarine )
				{
					IncrementStrangePropertyOnEquippedItems( pMarine, 5004, 1 ); // Fast Hacks
				}
				return;
			}

			if ( FStrEq( event->GetName(), "entity_frozen" ) )
			{
				CBaseEntity *pTarget = Ent( event->GetInt( "entindex" ) );
				CBaseEntity *pAttacker = Ent( event->GetInt( "attacker" ) );
				CBaseEntity *pWeapon = Ent( event->GetInt( "weapon" ) );
				if ( pTarget && pAttacker && pAttacker->IsInhabitableNPC() && pWeapon )
				{
					s_RD_Inventory_Manager.IncrementStrangePropertiesForWeapon( assert_cast< CASW_Inhabitable_NPC * >( pAttacker ), pWeapon, 5005, 1 ); // Enemies Frozen
				}
				return;
			}

			if ( FStrEq( event->GetName(), "marine_infested_cured" ) )
			{
				CASW_Marine *pTarget = CASW_Marine::AsMarine( Ent( event->GetInt( "entindex" ) ) );
				CASW_Marine *pHealer = CASW_Marine::AsMarine( Ent( event->GetInt( "marine" ) ) );
				CBaseEntity *pWeapon = Ent( event->GetInt( "weapon" ) );

				if ( !pWeapon && pTarget && pTarget->IsElectrifiedArmorActive() )
				{
					s_RD_Inventory_Manager.IncrementStrangePropertyOnWeaponAndGlobals( pTarget, &pTarget->m_ElectrifiedArmorProjectileData, 5008, 1, 0, true, false, true ); // Infestations Cured
				}
				else if ( pTarget && pHealer && pWeapon )
				{
					s_RD_Inventory_Manager.IncrementStrangePropertiesForWeapon( pHealer, pWeapon, 5008, 1 ); // Infestations Cured
				}

				return;
			}

			if ( FStrEq( event->GetName(), "marine_extinguished" ) )
			{
				CASW_Marine *pTarget = CASW_Marine::AsMarine( Ent( event->GetInt( "entindex" ) ) );
				CASW_Marine *pHealer = CASW_Marine::AsMarine( Ent( event->GetInt( "healer" ) ) );
				CBaseEntity *pWeapon = Ent( event->GetInt( "weapon" ) );

				if ( pTarget && pHealer && pWeapon )
				{
					s_RD_Inventory_Manager.IncrementStrangePropertiesForWeapon( pHealer, pWeapon, 5006, 1 ); // Allies Extinguished
				}

				return;
			}

			if ( FStrEq( event->GetName(), "marine_healed" ) )
			{
				CASW_Marine *pTarget = CASW_Marine::AsMarine( Ent( event->GetInt( "patient_entindex" ) ) );
				CASW_Marine *pHealer = CASW_Marine::AsMarine( Ent( event->GetInt( "medic_entindex" ) ) );
				CBaseEntity *pWeapon = Ent( event->GetInt( "weapon" ) );

				if ( pTarget && pHealer && pWeapon )
				{
					s_RD_Inventory_Manager.IncrementStrangePropertiesForWeapon( pHealer, pWeapon, 5003, event->GetInt( "amount" ) ); // Healing
				}

				return;
			}
		}

		if ( FStrEq( event->GetName(), "asw_mission_restart" ) )
		{
#ifdef CLIENT_DLL
			if ( m_PendingDynamicPropertyUpdates.Count() )
			{
				CommitDynamicProperties();
			}
			else if ( m_DynamicPropertyUpdateResult == k_SteamInventoryResultInvalid )
			{
				if ( ValidatePlayerEquipmentResult() )
				{
					SendPlayerEquipmentToServer();
				}
			}
#endif
			return;
		}
	}

#ifdef CLIENT_DLL
	enum CraftItemType_t
	{
		// craft an item via a predefined recipe. notifies user when complete. modal while in progress.
		CRAFT_RECIPE,
		// attach an accessory to an item. modal while in progress. updates equip slots to use new ID. forces a dynamic property update for the accessory.
		CRAFT_ACCESSORY,
		// convert currency to items or items to currency. modal while in progress.
		CRAFT_SHOP,
		// claiming a currency reward. modal while in progress.
		CRAFT_CLAIM_MINOR,
		// claiming an item or bundle reward. modal while in progress. notifies user when complete.
		CRAFT_CLAIM_MAJOR,
		// behind-the-scenes item exchange. no notification.
		CRAFT_BTS,
		// set dynamic properties on newly created item to 0. modal while in progress. notifies user when complete (replaces notification from craft task that queued this).
		CRAFT_DYNAMIC_PROPERTY_INIT,
		// checking for item drop. notifies user based on preferences if successful.
		CRAFT_DROP,
		// checking for promo item. notifies user if successful.
		CRAFT_PROMO,
		// retrieving item data. may not be ours. notification when complete.
		CRAFT_INSPECT,
		// updating dynamic properties for an item (eg. at the end of a mission). no notifications.
		CRAFT_DYNAMIC_PROPERTY_UPDATE,
		// updating user-modifiable dynamic properties for an item (eg. style). modal while in progress.
		CRAFT_USER_DYNAMIC_PROPERTY_UPDATE,
		// deleting an item. modal while in progress.
		CRAFT_DELETE,
		// deleting an item silently. no notifications. (funnily enough, this is used to expire a different kind of notifications.)
		CRAFT_DELETE_SILENT,
		// updating the "seen" status of notifications. only one of these exist at a time. no notifications. (heh)
		CRAFT_NOTIFICATION_DYNAMIC_PROPERTY_UPDATE,
	};
	struct CraftItemTask_t
	{
		~CraftItemTask_t()
		{
			ISteamInventory *pInventory = SteamInventory();
			Assert( pInventory );
			if ( pInventory )
			{
				pInventory->DestroyResult( m_hResult );
			}
		}

		CraftItemType_t m_Type;
		SteamItemDef_t m_iAccessoryDef{ 0 }; // used for BaseModUI::ItemShowcase::Mode_t if m_Type is CRAFT_DYNAMIC_PROPERTY_INIT.
		SteamItemInstanceID_t m_iReplaceItemInstance{ k_SteamItemInstanceIDInvalid };
		SteamInventoryResult_t m_hResult{ k_SteamInventoryResultInvalid };
	};
	CUtlVectorAutoPurge<CraftItemTask_t *> m_CraftingQueue;
	bool m_bModalCraftingWaitScreenActive{ false };

	SteamInventoryResult_t *AddCraftItemTask( CraftItemType_t iMode, SteamItemDef_t iAccessoryDef = 0, SteamItemInstanceID_t iReplaceItemInstance = k_SteamItemInstanceIDInvalid )
	{
		CraftItemTask_t *pTask = new CraftItemTask_t
		{
			iMode,
			iAccessoryDef,
			iReplaceItemInstance,
			k_SteamInventoryResultInvalid,
		};

		bool bHadModal = HasModalCraftingTask();

		m_CraftingQueue.AddToTail( pTask );

		if ( !bHadModal && HasModalCraftingTask() )
			ShowModalCraftingWaitScreen();

		return &pTask->m_hResult;
	}
	bool HasModalCraftingTask()
	{
		FOR_EACH_VEC( m_CraftingQueue, i )
		{
			switch ( m_CraftingQueue[i]->m_Type )
			{
			case CRAFT_RECIPE:
			case CRAFT_ACCESSORY:
			case CRAFT_SHOP:
			case CRAFT_CLAIM_MINOR:
			case CRAFT_CLAIM_MAJOR:
			case CRAFT_DYNAMIC_PROPERTY_INIT:
			case CRAFT_USER_DYNAMIC_PROPERTY_UPDATE:
			case CRAFT_DELETE:
				return true;
			case CRAFT_BTS:
			case CRAFT_DROP:
			case CRAFT_PROMO:
			case CRAFT_INSPECT:
			case CRAFT_DYNAMIC_PROPERTY_UPDATE:
			case CRAFT_DELETE_SILENT:
				break;
			default:
				Assert( !"unhandled crafting task type" );
				break;
			}
		}

		return false;
	}
	bool IsUserInitiatedTask( CraftItemTask_t *pTask )
	{
		switch ( pTask->m_Type )
		{
		case CRAFT_RECIPE:
		case CRAFT_ACCESSORY:
		case CRAFT_SHOP:
		case CRAFT_CLAIM_MINOR:
		case CRAFT_CLAIM_MAJOR:
		case CRAFT_INSPECT:
		case CRAFT_USER_DYNAMIC_PROPERTY_UPDATE:
		case CRAFT_DELETE:
			return true;
		case CRAFT_DYNAMIC_PROPERTY_INIT:
			return pTask->m_iAccessoryDef != BaseModUI::ItemShowcase::MODE_ITEM_DROP;
		case CRAFT_BTS:
		case CRAFT_DROP:
		case CRAFT_PROMO:
		case CRAFT_DYNAMIC_PROPERTY_UPDATE:
		case CRAFT_DELETE_SILENT:
			break;
		default:
			Assert( !"unhandled crafting task type" );
			break;
		}

		return false;
	}
	void OnCraftingTaskCompleted( CraftItemTask_t *pTask )
	{
		GET_INVENTORY_OR_BAIL;

		if ( rd_debug_inventory.GetBool() )
		{
			Msg( "Crafting task (type %d):\n", pTask->m_Type );
			DebugPrintResult( pTask->m_hResult );
		}

		EResult eResult = pInventory->GetResultStatus( pTask->m_hResult );
		if ( eResult != k_EResultOK )
		{
			if ( !HasModalCraftingTask() )
			{
				HideModalCraftingWaitScreen();
			}

			Warning( "Crafting task (type %d) failed with EResult %d %s\n", pTask->m_Type, eResult, UTIL_RD_EResultToString( eResult ) );
			DebugPrintResult( pTask->m_hResult );

			if ( IsUserInitiatedTask( pTask ) )
			{
				BaseModUI::GenericConfirmation *pConfirm = assert_cast< BaseModUI::GenericConfirmation * >( BaseModUI::CBaseModPanel::GetSingleton().OpenWindow( BaseModUI::WT_GENERICCONFIRMATION, NULL, false ) );
				BaseModUI::GenericConfirmation::Data_t data{};
				CFmtStr szSpecificError{ "#rd_inventory_item_action_failed_%d_desc", eResult };
				data.pWindowTitle = "#rd_inventory_item_action_failed_title";
				if ( g_pVGuiLocalize->Find( szSpecificError ) )
				{
					data.pMessageText = szSpecificError;
				}
				else
				{
					wchar_t wszErrorCode[16];
					V_snwprintf( wszErrorCode, NELEMS( wszErrorCode ), L"%d", eResult );
					wchar_t wszMessage[1024];
					g_pVGuiLocalize->ConstructString( wszMessage, sizeof( wszMessage ), g_pVGuiLocalize->Find( "#rd_inventory_item_action_failed_generic_desc" ), 1, wszErrorCode );
					data.pMessageTextW = wszMessage;
				}
				data.bOkButtonEnabled = true;
				pConfirm->SetUsageData( data );
			}

			if ( pTask->m_Type == CRAFT_NOTIFICATION_DYNAMIC_PROPERTY_UPDATE )
			{
				DebuggerBreakIfDebugging(); // TODO
			}

			return;
		}

		switch ( pTask->m_Type )
		{
		case CRAFT_RECIPE:
			InitDynamicPropertiesAndShowcase( pTask, BaseModUI::ItemShowcase::MODE_ITEM_CRAFTED );
			break;
		case CRAFT_ACCESSORY:
			ReplaceEquippedItemInstance( pTask->m_iReplaceItemInstance, pTask->m_hResult, 0 );
			InitDynamicPropertiesAndShowcase( pTask, BaseModUI::ItemShowcase::MODE_ITEM_UPGRADED );
			break;
		case CRAFT_SHOP:
			InitDynamicPropertiesAndShowcase( pTask, BaseModUI::ItemShowcase::MODE_ITEM_CLAIMED, false, true );
			break;
		case CRAFT_CLAIM_MINOR:
			InitDynamicPropertiesAndShowcase( pTask, BaseModUI::ItemShowcase::MODE_ITEM_CLAIMED, false, true );
			break;
		case CRAFT_CLAIM_MAJOR:
			InitDynamicPropertiesAndShowcase( pTask, BaseModUI::ItemShowcase::MODE_ITEM_CLAIMED );
			break;
		case CRAFT_BTS:
			break;
		case CRAFT_DYNAMIC_PROPERTY_INIT:
			BaseModUI::ItemShowcase::ShowItems( pTask->m_hResult, 0, -1, ( BaseModUI::ItemShowcase::Mode_t )pTask->m_iAccessoryDef );
			break;
		case CRAFT_DROP:
			InitDynamicPropertiesAndShowcase( pTask, BaseModUI::ItemShowcase::MODE_ITEM_DROP, true );
			break;
		case CRAFT_PROMO:
			InitDynamicPropertiesAndShowcase( pTask, BaseModUI::ItemShowcase::MODE_ITEM_DROP );
			break;
		case CRAFT_INSPECT:
			BaseModUI::ItemShowcase::ShowItems( pTask->m_hResult, 0, -1, BaseModUI::ItemShowcase::MODE_INSPECT );
			break;
		case CRAFT_DYNAMIC_PROPERTY_UPDATE:
			if ( m_bWantExtraDynamicPropertyCommit )
			{
				CommitDynamicProperties();
				m_bWantExtraDynamicPropertyCommit = false;
			}
			break;
		case CRAFT_USER_DYNAMIC_PROPERTY_UPDATE:
			break;
		case CRAFT_DELETE:
			break;
		case CRAFT_DELETE_SILENT:
			break;
		case CRAFT_NOTIFICATION_DYNAMIC_PROPERTY_UPDATE:
			m_InFlightNotificationSeen.Purge();
			break;
		default:
			Assert( !"unhandled crafting task type" );
			break;
		}

		ISteamUser *pUser = SteamUser();
		if ( pUser && pInventory->CheckResultSteamID( pTask->m_hResult, pUser->GetSteamID() ) )
		{
			uint32 nCount{ 0 };
			pInventory->GetResultItems( pTask->m_hResult, NULL, &nCount );
			if ( nCount > 0 )
			{
				m_bWantFullInventoryRefresh = true;
			}
		}

		if ( !HasModalCraftingTask() )
		{
			// the just-completed task is already removed from the list before this function is called, but we may have added an additional modal task
			HideModalCraftingWaitScreen();
		}

		if ( m_bWantFullInventoryRefresh && m_CraftingQueue.Count() == 0 )
		{
			pInventory->DestroyResult( m_GetFullInventoryForCacheResult );
			m_GetFullInventoryForCacheResult = k_SteamInventoryResultInvalid;

			pInventory->GetAllItems( &m_GetFullInventoryForCacheResult );

			if ( ValidatePlayerEquipmentResult() )
			{
				SendPlayerEquipmentToServer();
			}

			m_bWantFullInventoryRefresh = false;
		}
	}
	void InitDynamicPropertiesAndShowcase( CraftItemTask_t *pTask, BaseModUI::ItemShowcase::Mode_t iMode, bool bCheckPreferences = false, bool bSilenceNotification = false )
	{
		GET_INVENTORY_OR_BAIL;

		SteamInventoryUpdateHandle_t hUpdate = k_SteamInventoryUpdateHandleInvalid;

		uint32 nCount{};
		pInventory->GetResultItems( pTask->m_hResult, NULL, &nCount );
		CUtlVector<SteamItemDetails_t> items;
		items.AddMultipleToTail( nCount );
		pInventory->GetResultItems( pTask->m_hResult, items.Base(), &nCount );

		FOR_EACH_VEC( items, i )
		{
			if ( ( items[i].m_unFlags & k_ESteamItemRemoved ) || items[i].m_unQuantity == 0 || items[i].m_iDefinition == 0 )
				continue;

			ReactiveDropInventory::ItemInstance_t instance{ pTask->m_hResult, uint32( i ) };
			const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( instance.ItemDefID );
			Assert( pDef );
			if ( !pDef )
				continue;

			bool bHeldBack = false;

			FOR_EACH_VEC( pDef->CompressedDynamicProps, j )
			{
				if ( V_stricmp( pDef->CompressedDynamicProps[j], "m_unQuantity" ) && !instance.DynamicProps.Defined(pDef->CompressedDynamicProps[j]) )
				{
					Assert( ReactiveDropInventory::DynamicPropertyDataType( pDef->CompressedDynamicProps[j] ) == FIELD_INTEGER64 );
					if ( hUpdate == k_SteamInventoryUpdateHandleInvalid )
					{
						hUpdate = pInventory->StartUpdateProperties();
						Assert( hUpdate != k_SteamInventoryUpdateHandleInvalid );
					}

					if ( rd_debug_inventory.GetBool() )
						Msg( "[C] Initializing missing dynamic property '%s' on item %llu (%d %s) to 0.\n", pDef->CompressedDynamicProps[j], instance.ItemID, instance.ItemDefID, pDef->Name.Get() );

					bHeldBack = true;
					pInventory->SetProperty( hUpdate, instance.ItemID, pDef->CompressedDynamicProps[j], 0ll );
				}
			}

			if ( !pDef->AccessoryTag.IsEmpty() && instance.Tags.Defined( pDef->AccessoryTag ) )
			{
				const CUtlStringList &accessories = instance.Tags[pDef->AccessoryTag];
				FOR_EACH_VEC( accessories, j )
				{
					SteamItemDef_t accessoryID = V_atoi( accessories[j] );
					Assert( accessoryID > 0 && accessoryID < 1000000000 );
					const ReactiveDropInventory::ItemDef_t *pAccessoryDef = ReactiveDropInventory::GetItemDef( accessoryID );
					Assert( pAccessoryDef );
					if ( !pAccessoryDef )
						continue;

					FOR_EACH_VEC( pAccessoryDef->CompressedDynamicProps, k )
					{
						if ( V_stricmp( pAccessoryDef->CompressedDynamicProps[k], "m_unQuantity" ) && !instance.DynamicProps.Defined( pAccessoryDef->CompressedDynamicProps[k] ) )
						{
							Assert( ReactiveDropInventory::DynamicPropertyDataType( pAccessoryDef->CompressedDynamicProps[k] ) == FIELD_INTEGER64 );
							if ( hUpdate == k_SteamInventoryUpdateHandleInvalid )
							{
								hUpdate = pInventory->StartUpdateProperties();
								Assert( hUpdate != k_SteamInventoryUpdateHandleInvalid );
							}

							if ( rd_debug_inventory.GetBool() )
								Msg( "[C] Initializing missing dynamic property '%s' on item %llu (%d %s) (from accessory %d %s) to 0.\n", pAccessoryDef->CompressedDynamicProps[k], instance.ItemID, instance.ItemDefID, pDef->Name.Get(), accessoryID, pAccessoryDef->Name.Get() );

							bHeldBack = true;
							pInventory->SetProperty( hUpdate, instance.ItemID, pAccessoryDef->CompressedDynamicProps[k], 0ll );
						}
					}
				}
			}

			if ( !bHeldBack )
			{
				if ( rd_debug_inventory.GetBool() )
					Msg( "[C] No dynamic property init needed for item %llu (%d %s).\n", instance.ItemID, instance.ItemDefID, pDef->Name.Get() );

				if ( !bSilenceNotification )
					BaseModUI::ItemShowcase::ShowItems( pTask->m_hResult, i, 1, iMode );
			}
		}

		if ( hUpdate != k_SteamInventoryUpdateHandleInvalid )
		{
			pInventory->SubmitUpdateProperties( hUpdate, AddCraftItemTask( bSilenceNotification ? CRAFT_DYNAMIC_PROPERTY_UPDATE : CRAFT_DYNAMIC_PROPERTY_INIT, SteamItemDef_t( iMode ) ) );
		}
	}
	void ReplaceEquippedItemInstance( SteamItemInstanceID_t iOldInstance, SteamInventoryResult_t hResult, uint32 index )
	{
		ReactiveDropInventory::ItemInstance_t instance{ hResult, index };
		ReactiveDropLoadout::ReplaceItemID( iOldInstance, instance.ItemID );
	}
	void ShowModalCraftingWaitScreen()
	{
		if ( m_bModalCraftingWaitScreenActive )
			return;

		BaseModUI::CUIGameData::Get()->OpenWaitScreen( "#rd_inventory_item_action_wait", 0.5f );
		m_bModalCraftingWaitScreenActive = true;
	}
	void HideModalCraftingWaitScreen()
	{
		if ( !m_bModalCraftingWaitScreenActive )
			return;

		BaseModUI::CUIGameData::Get()->CloseWaitScreen( NULL, NULL );
		m_bModalCraftingWaitScreenActive = false;
	}
#endif

	void DebugPrintResult( SteamInventoryResult_t hResult )
	{
		ISteamInventory *pInventory = SteamInventory();
		Assert( pInventory );
		if ( !pInventory )
		{
			Warning( "Cannot access ISteamInventory in callback!\n" );
			return;
		}

		uint32_t count{};
		if ( pInventory->GetResultItems( hResult, NULL, &count ) )
			Msg( "Result %08x (%s, age %d sec) has %d items:\n", hResult, UTIL_RD_EResultToString( pInventory->GetResultStatus( hResult ) ), SteamUtils()->GetServerRealTime() - pInventory->GetResultTimestamp( hResult ), count );
		else
			Msg( "Result %08x (%s, age %d sec) has ERR items:\n", hResult, UTIL_RD_EResultToString( pInventory->GetResultStatus( hResult ) ), SteamUtils()->GetServerRealTime() - pInventory->GetResultTimestamp( hResult ) );

		uint32 nSerializedBufferSize{};
		if ( pInventory->SerializeResult( hResult, NULL, &nSerializedBufferSize ) )
		{
			byte *pSerialized = ( byte * )stackalloc( nSerializedBufferSize );
			pInventory->SerializeResult( hResult, pSerialized, &nSerializedBufferSize );
			char *szSerialized = ( char * )stackalloc( nSerializedBufferSize * 2 + 1 );

			// V_binarytohex is O(n^2); use an O(n) algorithm instead.
			for ( uint32 i = 0; i < nSerializedBufferSize; i++ )
			{
				szSerialized[i * 2] = s_szHexDigits[( pSerialized[i] >> 4 ) & 0xf];
				szSerialized[i * 2 + 1] = s_szHexDigits[pSerialized[i] & 0xf];
			}
			szSerialized[nSerializedBufferSize * 2] = '\0';

			Msg( "Serialized (%d bytes): %s\n", nSerializedBufferSize, szSerialized );
		}

		CUtlMemory<SteamItemDetails_t> itemDetails( 0, count );
		if ( !pInventory->GetResultItems( hResult, itemDetails.Base(), &count ) )
		{
			Warning( "Failed to get item details for result.\n" );
			count = 0;
		}

		CUtlMemory<char> szStringBuf( 0, 1024 );
		uint32_t size{};

		FOR_EACH_VEC( itemDetails, i )
		{
			Msg( "Item %llu (def %d qty %d flags %x)\n", itemDetails[i].m_itemId, itemDetails[i].m_iDefinition, itemDetails[i].m_unQuantity, itemDetails[i].m_unFlags );

			szStringBuf[0] = '\0';

			{
				pInventory->GetResultItemProperty( hResult, i, NULL, NULL, &size );
				szStringBuf.EnsureCapacity( size + 1 );
				size = szStringBuf.Count();
				pInventory->GetResultItemProperty( hResult, i, NULL, szStringBuf.Base(), &size );
				Msg( "ResultProperties: %s\n", szStringBuf.Base() );
				CSplitString propertyNames( szStringBuf.Base(), "," );
				FOR_EACH_VEC( propertyNames, j )
				{
					pInventory->GetResultItemProperty( hResult, i, propertyNames[j], NULL, &size );
					szStringBuf.EnsureCapacity( size + 1 );
					size = szStringBuf.Count();
					pInventory->GetResultItemProperty( hResult, i, propertyNames[j], szStringBuf.Base(), &size );
					Msg( "ResultProperties[%s] = %s\n", propertyNames[j], szStringBuf.Base() );
				}
			}

			Msg( "\n" );
		}
	}

#ifndef CLIENT_DLL
	STEAM_GAMESERVER_CALLBACK( CRD_Inventory_Manager, OnSteamInventoryResultReadyDedicated, SteamInventoryResultReady_t )
	{
		OnSteamInventoryResultReady( pParam );
	}
#endif

	STEAM_CALLBACK( CRD_Inventory_Manager, OnSteamInventoryResultReady, SteamInventoryResultReady_t )
	{
		DevMsg( 2, "[%c] Steam Inventory result for %08x received: EResult %d (%s)\n", IsClientDll() ? 'C' : 'S', pParam->m_handle, pParam->m_result, UTIL_RD_EResultToString( pParam->m_result ) );

#ifdef CLIENT_DLL
		ISteamInventory *pInventory = SteamInventory();
#else
		ISteamInventory *pInventory = engine->IsDedicatedServer() ? SteamGameServerInventory() : SteamInventory();
#endif
		Assert( pInventory );

		if ( pParam->m_handle == m_DebugPrintInventoryResult )
		{
			DebugPrintResult( m_DebugPrintInventoryResult );
			pInventory->DestroyResult( m_DebugPrintInventoryResult );
			m_DebugPrintInventoryResult = k_SteamInventoryResultInvalid;

			return;
		}

		if ( pParam->m_result != k_EResultOK && rd_debug_inventory.GetBool() )
		{
			DebugPrintResult( pParam->m_handle );
		}

#ifdef CLIENT_DLL
		if ( pParam->m_handle == m_GetFullInventoryForCacheResult )
		{
			CacheUserInventory( m_GetFullInventoryForCacheResult );

			TabbedGridDetails *pCollections = assert_cast< TabbedGridDetails * >( BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::WT_COLLECTIONS ) );
			if ( pCollections && pCollections->m_hCurrentTab )
			{
				CRD_Collection_Tab_Inventory *pInventoryTab = dynamic_cast< CRD_Collection_Tab_Inventory * >( pCollections->m_hCurrentTab.Get() );
				if ( pInventoryTab )
				{
					pInventoryTab->ForceRefreshItems( m_GetFullInventoryForCacheResult );
				}
			}

			pInventory->DestroyResult( m_GetFullInventoryForCacheResult );
			m_GetFullInventoryForCacheResult = k_SteamInventoryResultInvalid;

			return;
		}

		if ( pParam->m_handle == m_DynamicPropertyUpdateResult )
		{
			bool bAnyPlayerEquipChanged = false;

			uint32_t nItems = 0;
			pInventory->GetResultItems( pParam->m_handle, NULL, &nItems );
			SteamItemDetails_t *pDetails = ( SteamItemDetails_t * )stackalloc( sizeof( SteamItemDetails_t ) * nItems );
			pInventory->GetResultItems( pParam->m_handle, pDetails, &nItems );

			for ( int i = 0; i < NELEMS( ReactiveDropInventory::g_PlayerInventorySlotNames ); i++ )
			{
				ConVarRef var( VarArgs( "rd_equipped_%s", ReactiveDropInventory::g_PlayerInventorySlotNames[i] ) );
				SteamItemInstanceID_t id = strtoull( var.GetString(), NULL, 10 );
				if ( id == 0 || id == k_SteamItemInstanceIDInvalid )
					continue;

				for ( uint32_t j = 0; j < nItems; j++ )
				{
					if ( pDetails[j].m_itemId == id )
					{
						bAnyPlayerEquipChanged = true;
						break;
					}
				}

				if ( bAnyPlayerEquipChanged )
					break;
			}

			pInventory->DestroyResult( m_DynamicPropertyUpdateResult );
			m_DynamicPropertyUpdateResult = k_SteamInventoryResultInvalid;

			if ( m_bWantExtraDynamicPropertyCommit )
			{
				m_bWantExtraDynamicPropertyCommit = false;
				CommitDynamicProperties();
			}
			else
			{
				// TODO: can we just slot in the updated items?
				if ( m_GetFullInventoryForCacheResult != k_SteamInventoryResultInvalid )
					pInventory->DestroyResult( m_GetFullInventoryForCacheResult );
				pInventory->GetAllItems( &m_GetFullInventoryForCacheResult );

				ValidatePlayerEquipmentResult( bAnyPlayerEquipChanged );
			}

			return;
		}

		if ( pParam->m_handle == m_PlayerEquipmentResult )
		{
			SendPlayerEquipmentToServer();
		}

		FOR_EACH_VEC( m_CraftingQueue, i )
		{
			CraftItemTask_t *pTask = m_CraftingQueue[i];
			if ( pParam->m_handle == pTask->m_hResult )
			{
				m_CraftingQueue.Remove( i );
				OnCraftingTaskCompleted( pTask );
				delete pTask;
				return;
			}
		}
#else
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
			if ( !pPlayer )
				continue;

			if ( pPlayer->OnSteamInventoryResultReady( pParam ) )
			{
				return;
			}
		}
#endif

		if ( rd_debug_inventory.GetBool() )
		{
			DevMsg( "[%c] Inventory result %08x unhandled on %s.\n", IsClientDll() ? 'C' : 'S', pParam->m_handle, IsClientDll() ? "client" : "server" );
		}
	}

	float m_flDefsUpdateTime{ 0.0f };
	SteamInventoryResult_t m_DebugPrintInventoryResult{ k_SteamInventoryResultInvalid };
#ifdef CLIENT_DLL
	CUtlVector<ReactiveDropInventory::ItemInstance_t> m_LocalInventoryCache;
	SteamInventoryResult_t m_GetFullInventoryForCacheResult{ k_SteamInventoryResultInvalid };
	bool m_bWantFullInventoryRefresh{ false };

	// we keep this result alive so we can quickly send our medals to game servers without having to do two network round trips.
	// the result gets reset when an equipped medal changes due to dynamic property updates or crafting,
	// the set of equipped medals is modified, or the result is within 5 minutes of expiring.
	SteamInventoryResult_t m_PlayerEquipmentResult{ k_SteamInventoryResultInvalid };
	// checks to see if m_PlayerEquipmentResult is ready for sending to a server; if not, an inventory request MAY be started.
	bool ValidatePlayerEquipmentResult( bool bForceReset = false )
	{
		// if we can't get the inventory API, we can't send the data anyway.
		GET_INVENTORY_OR_BAIL( false );

		// we need SteamUtils so we can check the server time to see if our result is near the 1 hour expiration time.
		ISteamUtils *pUtils = SteamUtils();
		Assert( pUtils );
		if ( !pUtils )
			return false;

		if ( gpGlobals->maxClients == 1 && engine->IsInGame() && engine->IsClientLocalToActiveServer() && !s_bLoadedItemDefs )
		{
			// If we're offline and in singleplayer, send our medals the offline way.
			CUtlVector<int> args;
			CUtlVector<SteamItemInstanceID_t> items;

			for ( int i = 0; i < NELEMS( ReactiveDropInventory::g_PlayerInventorySlotNames ); i++ )
			{
				ConVarRef var( VarArgs( "rd_equipped_%s", ReactiveDropInventory::g_PlayerInventorySlotNames[i] ) );
				SteamItemInstanceID_t id = strtoull( var.GetString(), NULL, 10 );
				if ( id != 0 && id != k_SteamItemInstanceIDInvalid )
				{
					args.AddToTail( i );
					items.AddToTail( id );
				}
			}

			UTIL_RD_SendInventoryCommandOffline( INVCMD_PLAYER_EQUIPS, args, items );

			// keep going just in case we are actually online now.
		}

		if ( m_PlayerEquipmentResult != k_SteamInventoryResultInvalid && !bForceReset )
		{
			EResult eResult = pInventory->GetResultStatus( m_PlayerEquipmentResult );

			// if the result is pending, don't touch it.
			if ( eResult == k_EResultPending )
				return false;

			// result is valid; check timestamp
			if ( eResult == k_EResultOK )
			{
				// result expires 60 minutes after being created; catch it 5 minutes early and fetch a new one.
				int iAge = pUtils->GetServerRealTime() - pInventory->GetResultTimestamp( m_PlayerEquipmentResult );
				if ( iAge < 55 * 60 )
					return true;
			}
		}

		// otherwise, grab a new one
		CUtlVector<SteamItemInstanceID_t> items;
		items.EnsureCapacity( RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_PLAYER );

#define CHECK_PLAYER_EQUIP_CVAR( var ) \
		{ \
			SteamItemInstanceID_t iItemID = strtoull( var.GetString(), NULL, 10 ); \
			if ( items.IsValidIndex( items.Find( iItemID ) ) ) \
			{ \
				Warning( "Item in %s is already equipped in another slot; unequipping %llu.\n", var.GetName(), iItemID ); \
				var.SetValue( VarArgs( "%llu", k_SteamItemInstanceIDInvalid ) ); \
			} \
			else if ( iItemID != 0 && iItemID != k_SteamItemInstanceIDInvalid ) \
			{ \
				items.AddToTail( iItemID ); \
			} \
		}

		CHECK_PLAYER_EQUIP_CVAR( rd_equipped_medal[0] );
		CHECK_PLAYER_EQUIP_CVAR( rd_equipped_medal[1] );
		CHECK_PLAYER_EQUIP_CVAR( rd_equipped_medal[2] );

#undef CHECK_PLAYER_EQUIP_CVAR

		if ( m_PlayerEquipmentResult != k_SteamInventoryResultInvalid )
		{
			pInventory->DestroyResult( m_PlayerEquipmentResult );
			m_PlayerEquipmentResult = k_SteamInventoryResultInvalid;
		}

		if ( items.Count() == 0 )
		{
			// nothing equipped means we're ready to send now.
			return true;
		}

		pInventory->GetItemsByID( &m_PlayerEquipmentResult, items.Base(), items.Count() );
		Assert( m_PlayerEquipmentResult != k_SteamInventoryResultInvalid );

		return false;
	}
	int m_iPlayerEquipmentCommand{ 0 };
	void SendPlayerEquipmentToServer()
	{
		Assert( ValidatePlayerEquipmentResult() );
		if ( !engine->IsInGame() )
			return;

		UTIL_RD_AbortInventoryCommand( m_iPlayerEquipmentCommand );

		if ( m_PlayerEquipmentResult == k_SteamInventoryResultInvalid )
		{
			m_iPlayerEquipmentCommand = UTIL_RD_SendInventoryCommand( INVCMD_PLAYER_EQUIPS, CUtlVector<int>{}, m_PlayerEquipmentResult );
			return;
		}

		GET_INVENTORY_OR_BAIL;

		uint32_t nItems = 0;
		pInventory->GetResultItems( m_PlayerEquipmentResult, NULL, &nItems );
		SteamItemDetails_t *pDetails = ( SteamItemDetails_t * )stackalloc( sizeof( SteamItemDetails_t ) * nItems );
		pInventory->GetResultItems( m_PlayerEquipmentResult, pDetails, &nItems );

		CUtlVector<int> order;
		order.SetCount( nItems );
		order.FillWithValue( -1 );

		for ( int i = 0; i < RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_PLAYER; i++ )
		{
			ConVarRef var( VarArgs( "rd_equipped_%s", ReactiveDropInventory::g_PlayerInventorySlotNames[i] ) );
			SteamItemInstanceID_t id = strtoull( var.GetString(), NULL, 10 );
			if ( id == 0 || id == k_SteamItemInstanceIDInvalid )
				continue;

			for ( uint32_t j = 0; j < nItems; j++ )
			{
				if ( pDetails[j].m_itemId == id )
				{
					Assert( order[j] == -1 );
					order[j] = i;
					break;
				}
			}
		}

		m_iPlayerEquipmentCommand = UTIL_RD_SendInventoryCommand( INVCMD_PLAYER_EQUIPS, order, m_PlayerEquipmentResult );
	}

	struct NotificationSeenUpdate_t
	{
		SteamItemInstanceID_t NotificationID;
		int Seen;
	};
	CUtlVector<NotificationSeenUpdate_t> m_QueuedNotificationSeen;
	CUtlVector<NotificationSeenUpdate_t> m_InFlightNotificationSeen;

	STEAM_CALLBACK( CRD_Inventory_Manager, OnSteamInventoryFullUpdate, SteamInventoryFullUpdate_t )
	{
		// don't write the same cache twice in a row
		if ( pParam->m_handle == m_GetFullInventoryForCacheResult )
			return;

		CacheUserInventory( pParam->m_handle );
	}
#else
	STEAM_GAMESERVER_CALLBACK( CRD_Inventory_Manager, OnSteamInventoryDefinitionUpdateDedicated, SteamInventoryDefinitionUpdate_t )
	{
		OnSteamInventoryDefinitionUpdate( pParam );
	}
#endif

	STEAM_CALLBACK( CRD_Inventory_Manager, OnSteamInventoryDefinitionUpdate, SteamInventoryDefinitionUpdate_t )
	{
		s_bLoadedItemDefs = true;
		if ( s_pItemDefCache )
		{
			s_pItemDefCache->deleteThis();
			s_pItemDefCache = NULL;
		}

		// this leaks memory, but it's a small amount and only happens when an update is manually pushed.
		s_ItemDefs.RemoveAll();

#ifdef CLIENT_DLL
		CacheItemSchema();
#endif
	}
} s_RD_Inventory_Manager;

#ifdef CLIENT_DLL
static void RD_Equipped_Item_Changed( IConVar *var, const char *pOldValue, float flOldValue )
{
	s_RD_Inventory_Manager.ValidatePlayerEquipmentResult( true );

	if ( ReactiveDropInventory::CheckMedalEquipCache() )
	{
		ISteamUserStats *pUserStats = SteamUserStats();
		Assert( pUserStats );
		if ( pUserStats )
		{
			bool bOK = pUserStats->StoreStats();
			Assert( bOK );
			( void )bOK;
		}
	}
}
ConVar rd_equipped_medal[RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS]
{
	{ "rd_equipped_medal", "0", FCVAR_ARCHIVE, "Steam inventory item ID of equipped medal.", RD_Equipped_Item_Changed },
	{ "rd_equipped_medal1", "0", FCVAR_ARCHIVE, "Steam inventory item ID of equipped medal.", RD_Equipped_Item_Changed },
	{ "rd_equipped_medal2", "0", FCVAR_ARCHIVE, "Steam inventory item ID of equipped medal.", RD_Equipped_Item_Changed },
};

CON_COMMAND( rd_debug_print_inventory, "" )
{
	SteamInventory()->GetAllItems( &s_RD_Inventory_Manager.m_DebugPrintInventoryResult );
}

CON_COMMAND( rd_debug_inspect_entire_inventory, "inspect every item in your inventory" )
{
	SteamInventory()->GetAllItems( s_RD_Inventory_Manager.AddCraftItemTask( CRD_Inventory_Manager::CRAFT_INSPECT ) );
}

CON_COMMAND( rd_debug_inspect_own_item, "inspect an item you own by ID" )
{
	SteamItemInstanceID_t id = strtoull( args.Arg( 1 ), NULL, 10 );
	if ( !id )
	{
		Msg( "Usage: rd_debug_inspect_own_item [item instance ID]\n" );
		return;
	}

	SteamInventory()->GetItemsByID( s_RD_Inventory_Manager.AddCraftItemTask( CRD_Inventory_Manager::CRAFT_INSPECT ), &id, 1 );
}

CON_COMMAND( rd_econ_item_preview, "inspect an item using a code from the Steam Community backpack" )
{
	if ( args.ArgC() != 2 )
	{
		Msg( "missing or invalid inspect item code\n" );
		return;
	}

	SteamInventory()->InspectItem( s_RD_Inventory_Manager.AddCraftItemTask( CRD_Inventory_Manager::CRAFT_INSPECT ), args.Arg( 1 ) );
}
#endif

namespace ReactiveDropInventory
{
	fieldtype_t DynamicPropertyDataType( const char *szPropertyName )
	{
		// can be FIELD_INTEGER64, FIELD_STRING, FIELD_FLOAT, or FIELD_BOOLEAN.
		return FIELD_INTEGER64;
	}

	bool ItemDef_t::ItemSlotMatches( const char *szRequiredSlot ) const
	{
		Assert( szRequiredSlot );
		if ( !szRequiredSlot )
			return false;

		if ( ItemSlot == szRequiredSlot )
			return true;

		for ( int i = 0; i < NELEMS( g_InventorySlotAliases ); i++ )
		{
			if ( !V_strcmp( g_InventorySlotAliases[i][0], szRequiredSlot ) && ItemSlot == g_InventorySlotAliases[i][1] )
			{
				return true;
			}
		}

		return false;
	}

	int64_t ItemDef_t::CountForStrangeTier( int iTier ) const
	{
		Assert( iTier >= 0 );
		if ( iTier <= 0 )
			return 0;

		if ( iTier <= StrangeNotify.Count() )
		{
			return StrangeNotify[iTier - 1];
		}

		iTier -= StrangeNotify.Count();

		return StrangeNotifyEvery * iTier;
	}

	int ItemDef_t::GetStrangeTier( int64_t x ) const
	{
		int iTier = 0;
		Assert( x >= 0 );

		FOR_EACH_VEC( StrangeNotify, i )
		{
			if ( StrangeNotify[i] > x )
				break;

			iTier++;
		}

		if ( StrangeNotifyEvery > 0 && x > 0 )
		{
			iTier += x / StrangeNotifyEvery;
		}

		return iTier;
	}

	bool ItemDef_t::HasNotificationTag( const char *szTag ) const
	{
		FOR_EACH_VEC( NotificationTags, i )
		{
			if ( !V_strcmp( szTag, NotificationTags[i] ) )
			{
				return true;
			}
		}

		return false;
	}

	static bool ParseDynamicProps( CUtlStringMap<CUtlString> & props, const char *szDynamicProps )
	{
		jsmn_parser parser;
		jsmntok_t tokens[256];

		jsmn_init( &parser );

		int count = jsmn_parse( &parser, szDynamicProps, V_strlen( szDynamicProps ), tokens, NELEMS( tokens ) );
		if ( count <= 0 )
		{
			Warning( "Parsing item dynamic property data: corrupt data type %d\n", -count );
			return false;
		}

		Assert( tokens[0].type & JSMN_OBJECT );
		Assert( count & 1 );

		char szKey[1024];
		char szValue[1024];

		for ( int i = 1; i + 1 < count; i += 2 )
		{
			Assert( tokens[i].type & JSMN_STRING );
			Assert( tokens[i + 1].type & ( JSMN_STRING | JSMN_PRIMITIVE ) );

			V_strncpy( szKey, &szDynamicProps[tokens[i].start], MIN( tokens[i].end - tokens[i].start + 1, sizeof( szKey ) ) );
			V_strncpy( szValue, &szDynamicProps[tokens[i + 1].start], MIN( tokens[i + 1].end - tokens[i + 1].start + 1, sizeof( szValue ) ) );

			props[szKey] = szValue;
		}

		return true;
	}

	static void ParseTags( CUtlStringMap<CUtlStringList> & tags, const char *szTags )
	{
		if ( *szTags == '\0' )
		{
			return;
		}

		CSplitString tagsSplit( szTags, ";" );
		FOR_EACH_VEC( tagsSplit, i )
		{
			char *szKey = tagsSplit[i];
			char *szValue = strchr( tagsSplit[i], ':' );
			*szValue++ = '\0';

			tags[szKey].CopyAndAddToTail( szValue );
		}
	}

	static uint32 ParseTimestamp( const char *szTimestamp )
	{
		// This is an attrocious way of parsing an ISO 8601 timestamp, but it's fast and we trust the input data.
		Assert( V_strlen( szTimestamp ) == 16 && szTimestamp[8] == 'T' && szTimestamp[15] == 'Z' );

		std::tm timestamp{};
		Assert( V_isdigit( szTimestamp[0] ) && V_isdigit( szTimestamp[1] ) && V_isdigit( szTimestamp[2] ) && V_isdigit( szTimestamp[3] ) );
		timestamp.tm_year = ( szTimestamp[0] - '0' ) * 1000 + ( szTimestamp[1] - '0' ) * 100 + ( szTimestamp[2] - '0' ) * 10 + ( szTimestamp[3] - '0' );
		Assert( V_isdigit( szTimestamp[4] ) && V_isdigit( szTimestamp[5] ) );
		timestamp.tm_mon = ( szTimestamp[4] - '0' ) * 10 + ( szTimestamp[5] - '0' );
		Assert( V_isdigit( szTimestamp[6] ) && V_isdigit( szTimestamp[7] ) );
		timestamp.tm_mday = ( szTimestamp[6] - '0' ) * 10 + ( szTimestamp[7] - '0' );
		Assert( V_isdigit( szTimestamp[9] ) && V_isdigit( szTimestamp[10] ) );
		timestamp.tm_hour = ( szTimestamp[9] - '0' ) * 10 + ( szTimestamp[10] - '0' );
		Assert( V_isdigit( szTimestamp[11] ) && V_isdigit( szTimestamp[12] ) );
		timestamp.tm_min = ( szTimestamp[11] - '0' ) * 10 + ( szTimestamp[12] - '0' );
		Assert( V_isdigit( szTimestamp[13] ) && V_isdigit( szTimestamp[14] ) );
		timestamp.tm_sec = ( szTimestamp[13] - '0' ) * 10 + ( szTimestamp[14] - '0' );

		return std::mktime( &timestamp );
	}

	ItemInstance_t::ItemInstance_t( SteamInventoryResult_t hResult, uint32 index )
	{
		GET_INVENTORY_OR_BAIL;

		char buf[1536]{};
		uint32 len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "accountid", buf, &len ) )
		{
			AccountID.SetFromUint64( strtoull( buf, NULL, 10 ) );
		}
		len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "itemid", buf, &len ) )
		{
			ItemID = strtoull( buf, NULL, 10 );
		}
		len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "originalitemid", buf, &len ) )
		{
			OriginalItemID = strtoull( buf, NULL, 10 );
		}
		len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "itemdefid", buf, &len ) )
		{
			ItemDefID = strtol( buf, NULL, 10 );
		}
		len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "quantity", buf, &len ) )
		{
			Quantity = strtol( buf, NULL, 10 );
		}
		len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "acquired", buf, &len ) )
		{
			Acquired = ParseTimestamp( buf );
		}
		len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "state_changed_timestamp", buf, &len ) )
		{
			StateChangedTimestamp = ParseTimestamp( buf );
		}
		len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "state", buf, &len ) )
		{
			State = buf;
		}
		len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "origin", buf, &len ) )
		{
			Origin = buf;
		}
		len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "dynamic_props", buf, &len ) )
		{
			ParseDynamicProps( DynamicProps, buf );
		}
		len = sizeof( buf );
		if ( pInventory->GetResultItemProperty( hResult, index, "tags", buf, &len ) )
		{
			ParseTags( Tags, buf );
		}
	}

	ItemInstance_t::ItemInstance_t( KeyValues *pKV )
	{
		FromKeyValues( pKV );
	}

	int ItemInstance_t::GetStyle() const
	{
		UtlSymId_t i = DynamicProps.Find( "style" );
		if ( i != UTL_INVAL_SYMBOL )
		{
			return atoi( DynamicProps[i] );
		}

		return 0;
	}

	KeyValues *ItemInstance_t::ToKeyValues() const
	{
		KeyValues *pKV = new KeyValues( "i" );
		pKV->SetUint64( "a", AccountID.ConvertToUint64() );
		pKV->SetUint64( "i", ItemID );
		pKV->SetUint64( "o", OriginalItemID );
		pKV->SetInt( "d", ItemDefID );
		pKV->SetInt( "q", Quantity );
		pKV->SetInt( "c", Acquired );
		pKV->SetInt( "t", StateChangedTimestamp );
		pKV->SetString( "s", State );
		pKV->SetString( "b", Origin );
		for ( int i = 0; i < DynamicProps.GetNumStrings(); i++ )
		{
			switch ( DynamicPropertyDataType( DynamicProps.String( i ) ) )
			{
			case FIELD_STRING:
				pKV->SetString( CFmtStr( "x/%s", DynamicProps.String( i ) ), DynamicProps[i] );
				break;
			case FIELD_INTEGER64:
				pKV->SetUint64( CFmtStr( "x/%s", DynamicProps.String( i ) ), strtoll( DynamicProps[i], NULL, 10 ) );
				break;
			case FIELD_FLOAT:
				pKV->SetFloat( CFmtStr( "x/%s", DynamicProps.String( i ) ), strtof( DynamicProps[i], NULL ) );
				break;
			case FIELD_BOOLEAN:
				pKV->SetBool( CFmtStr( "x/%s", DynamicProps.String( i ) ), !V_strcmp( DynamicProps[i], "true" ) );
				break;
			}
		}
		if ( Tags.GetNumStrings() )
		{
			KeyValues *pTags = new KeyValues( "y" );
			for ( int i = 0; i < Tags.GetNumStrings(); i++ )
			{
				FOR_EACH_VEC( Tags[i], j )
				{
					KeyValues *pTag = new KeyValues( Tags.String( i ) );
					pTag->SetStringValue( Tags[i][j] );
					pTags->AddSubKey( pTag );
				}
			}
			pKV->AddSubKey( pTags );
		}

		return pKV;
	}

	void ItemInstance_t::FromKeyValues( KeyValues *pKV )
	{
		AccountID = pKV->GetUint64( "a" );
		ItemID = pKV->GetUint64( "i" );
		OriginalItemID = pKV->GetUint64( "o" );
		ItemDefID = pKV->GetInt( "d" );
		Quantity = pKV->GetInt( "q" );
		Acquired = pKV->GetInt( "c" );
		StateChangedTimestamp = pKV->GetInt( "t" );
		State = pKV->GetString( "s" );
		Origin = pKV->GetString( "b" );
		DynamicProps.Purge();
		Tags.Purge();
		if ( KeyValues *pDynamicProps = pKV->FindKey( "x" ) )
		{
			FOR_EACH_VALUE( pDynamicProps, pProp )
			{
				if ( pProp->GetDataType() == KeyValues::TYPE_UINT64 )
					DynamicProps[pProp->GetName()] = CFmtStr( "%lld", pProp->GetUint64() );
				else if ( pProp->GetDataType() == KeyValues::TYPE_INT )
					DynamicProps[pProp->GetName()] = pProp->GetBool() ? "true" : "false";
				else
					DynamicProps[pProp->GetName()] = pProp->GetString();
			}
		}
		if ( KeyValues *pTags = pKV->FindKey( "y" ) )
		{
			FOR_EACH_VALUE( pTags, pTag )
			{
				Tags[pTag->GetName()].CopyAndAddToTail( pTag->GetString() );
			}
		}
	}

	ItemInstance_t &ItemInstance_t::operator=( const ItemInstance_t &other )
	{
		if ( this == &other )
			return *this;

		AccountID = other.AccountID;
		ItemID = other.ItemID;
		OriginalItemID = other.OriginalItemID;
		ItemDefID = other.ItemDefID;
		Quantity = other.Quantity;
		Acquired = other.Acquired;
		StateChangedTimestamp = other.StateChangedTimestamp;
		State = other.State.Get();
		Origin = other.Origin.Get();

		DynamicProps.Purge();
		for ( int i = 0; i < other.DynamicProps.GetNumStrings(); i++ )
		{
			DynamicProps[other.DynamicProps.String( i )] = other.DynamicProps[i].Get();
		}

		Tags.Purge();
		for ( int i = 0; i < other.Tags.GetNumStrings(); i++ )
		{
			Tags[other.Tags.String( i )].EnsureCapacity( other.Tags[i].Count() );
			Assert( Tags.Find( other.Tags.String( i ) ) == i );
			for ( int j = 0; j < other.Tags[i].Count(); j++ )
			{
				Tags[i].CopyAndAddToTail( other.Tags[i][j] );
			}
		}

		return *this;
	}

	const ItemDef_t *GetItemDef( SteamItemDef_t id )
	{
		Assert( id >= 1 && id <= 999999999 );
		if ( id <= 0 )
		{
			return NULL;
		}

		unsigned short index = s_ItemDefs.Find( id );
		if ( s_ItemDefs.IsValidIndex( index ) )
		{
			return s_ItemDefs[index];
		}

		ISteamInventory *pInventory = SteamInventory();
#ifdef GAME_DLL
		if ( engine->IsDedicatedServer() )
		{
			pInventory = SteamGameServerInventory();
		}
#endif
		KeyValues *pCachedDef = NULL;
		if ( !s_bLoadedItemDefs )
		{
#ifdef GAME_DLL
			if ( engine->IsDedicatedServer() )
				return NULL;
#endif

			if ( !s_pItemDefCache )
			{
				s_pItemDefCache = new KeyValues{ "IS" };
				CUtlBuffer buf;
				if ( !g_pFullFileSystem->ReadFile( "cfg/item_schema_cache.dat", "MOD", buf ) )
				{
					return NULL;
				}

				if ( !s_pItemDefCache->ReadAsBinary( buf ) )
				{
					s_pItemDefCache->Clear();
					return NULL;
				}
			}

			FOR_EACH_SUBKEY( s_pItemDefCache, pCache )
			{
				if ( pCache->GetInt( "itemdefid" ) == id )
				{
					pCachedDef = pCache;
					break;
				}
			}

			Assert( pCachedDef );
			if ( !pCachedDef )
			{
				return NULL;
			}
		}
		else
		{
			Assert( pInventory );

			if ( !pInventory )
			{
				return NULL;
			}
		}

		const char *szLang = SteamApps() ? SteamApps()->GetCurrentGameLanguage() : "english";
#ifdef GAME_DLL
		if ( engine->IsDedicatedServer() )
		{
			szLang = rd_dedicated_server_language.GetString();
		}
#endif

		uint32_t count{};
		if ( s_bLoadedItemDefs )
		{
			pInventory->GetItemDefinitionProperty( id, NULL, NULL, &count );
			Assert( count );
			if ( !count )
			{
				// no such item in schema
				return NULL;
			}
		}

		ItemDef_t *pItemDef = new ItemDef_t;
		pItemDef->ID = id;

		CUtlMemory<char> szBuf( 0, 1024 );
		const char *szValue;

#define FETCH_PROPERTY( szPropertyName ) \
		if ( pCachedDef ) \
		{ \
			szValue = pCachedDef->GetString( szPropertyName ); \
		} \
		else \
		{ \
			pInventory->GetItemDefinitionProperty( id, szPropertyName, NULL, &count ); \
			szBuf.EnsureCapacity( count + 1 ); \
			count = szBuf.Count(); \
			pInventory->GetItemDefinitionProperty( id, szPropertyName, szBuf.Base(), &count ); \
			szValue = szBuf.Base(); \
		}

		char szKey[256];

		FETCH_PROPERTY( "type" );
		CUtlString szType = szValue;
		FETCH_PROPERTY( "item_slot" );
		pItemDef->ItemSlot = szValue;
		FETCH_PROPERTY( "equip_index" );
		pItemDef->EquipIndex = *szValue ? V_atoi( szValue ) : -1;
		FETCH_PROPERTY( "tags" );
		ParseTags( pItemDef->Tags, szValue );
		FETCH_PROPERTY( "allowed_tags_from_tools" );
		ParseTags( pItemDef->AllowedTagsFromTools, szValue );
		FETCH_PROPERTY( "accessory_tag" );
		pItemDef->AccessoryTag = szValue;
		FETCH_PROPERTY( "accessory_limit" );
		if ( *szValue )
			pItemDef->AccessoryLimit = strtol( szValue, NULL, 10 );
		Assert( pItemDef->AccessoryLimit >= 0 && pItemDef->AccessoryLimit <= RD_ITEM_MAX_ACCESSORIES );
		FETCH_PROPERTY( "compressed_dynamic_props" );
		if ( *szValue )
		{
			CSplitString CompressedDynamicProps{ szValue, ";" };
			Assert( CompressedDynamicProps.Count() <= RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS - ( pItemDef->AccessoryTag.IsEmpty() ? 0 : pItemDef->AccessoryLimit * RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS_PER_ACCESSORY ) );
			Assert( szType != "tag_tool" || CompressedDynamicProps.Count() <= RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS_PER_ACCESSORY );
			FOR_EACH_VEC( CompressedDynamicProps, i )
			{
				pItemDef->CompressedDynamicProps.CopyAndAddToTail( CompressedDynamicProps[i] );
			}
		}

		V_snprintf( szKey, sizeof( szKey ), "display_type_%s", szLang );
		FETCH_PROPERTY( "display_type" );
		pItemDef->DisplayType = szValue;
		FETCH_PROPERTY( szKey );
		if ( *szValue )
			pItemDef->DisplayType = szValue;

		V_snprintf( szKey, sizeof( szKey ), "name_%s", szLang );
		FETCH_PROPERTY( "name" );
		pItemDef->Name = szValue;
		FETCH_PROPERTY( szKey );
		if ( *szValue )
			pItemDef->Name = szValue;

		for ( int i = 0; i < RD_STEAM_INVENTORY_ITEM_MAX_STYLES; i++ )
		{
			V_snprintf( szKey, sizeof( szKey ), "style_%d_name_english", i );
			FETCH_PROPERTY( szKey );
			if ( !*szValue )
				break;

			CUtlString szFallback( szValue );
			V_snprintf( szKey, sizeof( szKey ), "style_%d_name_%s", i, szLang );
			FETCH_PROPERTY( szKey );
			if ( *szValue )
				pItemDef->StyleNames.CopyAndAddToTail( szValue );
			else
				pItemDef->StyleNames.CopyAndAddToTail( szFallback );
		}

		FETCH_PROPERTY( "background_color" );
		if ( *szValue )
		{
			Assert( V_strlen( szValue ) == 6 );
			unsigned iColor = strtoul( szValue, NULL, 16 );
			pItemDef->BackgroundColor = Color( iColor >> 16, ( iColor >> 8 ) & 255, iColor & 255, 200 );
		}
		else
		{
			pItemDef->BackgroundColor = Color( 41, 41, 41, 0 );
		}
		FETCH_PROPERTY( "name_color" );
		if ( *szValue )
		{
			Assert( V_strlen( szValue ) == 6 );
			unsigned iColor = strtoul( szValue, NULL, 16 );
			pItemDef->NameColor = Color( iColor >> 16, ( iColor >> 8 ) & 255, iColor & 255, 255 );
			pItemDef->HasBorder = true;
		}
		else
		{
			pItemDef->NameColor = Color( 178, 178, 178, 255 );
			pItemDef->HasBorder = false;
		}
		
		V_snprintf( szKey, sizeof( szKey ), "description_%s", szLang );
		pItemDef->HasInGameDescription = false;
		FETCH_PROPERTY( "description" );
		pItemDef->Description = szValue;
		FETCH_PROPERTY( szKey );
		if ( *szValue )
			pItemDef->Description = szValue;

		FETCH_PROPERTY( "ingame_description_english" );
		if ( *szValue )
		{
			pItemDef->Description = szValue;
			pItemDef->HasInGameDescription = true;
			V_snprintf( szKey, sizeof( szKey ), "ingame_description_%s", szLang );
			FETCH_PROPERTY( szKey );
			if ( *szValue )
				pItemDef->Description = szValue;
		}

		V_snprintf( szKey, sizeof( szKey ), "briefing_name_%s", szLang );
		pItemDef->BriefingName = pItemDef->Name;
		FETCH_PROPERTY( "briefing_name_english" );
		if ( *szValue )
			pItemDef->BriefingName = szValue;
		FETCH_PROPERTY( szKey );
		if ( *szValue )
			pItemDef->BriefingName = szValue;

		V_snprintf( szKey, sizeof( szKey ), "before_description_%s", szLang );
		FETCH_PROPERTY( "before_description_english" );
		pItemDef->BeforeDescription = szValue;
		FETCH_PROPERTY( szKey );
		if ( *szValue )
			pItemDef->BeforeDescription = szValue;

		V_snprintf( szKey, sizeof( szKey ), "after_description_%s", szLang );
		FETCH_PROPERTY( "after_description_english" );
		pItemDef->AfterDescription = szValue;
		FETCH_PROPERTY( szKey );
		if ( *szValue )
			pItemDef->AfterDescription = szValue;

		V_snprintf( szKey, sizeof( szKey ), "accessory_description_%s", szLang );
		FETCH_PROPERTY( "accessory_description_english" );
		pItemDef->AccessoryDescription = szValue;
		FETCH_PROPERTY( szKey );
		if ( *szValue )
			pItemDef->AccessoryDescription = szValue;

		for ( int i = 0; i < RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS_PER_ACCESSORY; i++ )
		{
			pItemDef->NotificationName[i] = pItemDef->Name;
			V_snprintf( szKey, sizeof( szKey ), "notification_name_%d_english", i );
			FETCH_PROPERTY( szKey );
			if ( *szValue )
				pItemDef->NotificationName[i] = szValue;
			V_snprintf( szKey, sizeof( szKey ), "notification_name_%d_%s", i, szLang );
			FETCH_PROPERTY( szKey );
			if ( *szValue )
				pItemDef->NotificationName[i] = szValue;
		}

		FETCH_PROPERTY( "strange_notify_every" );
		if ( *szValue )
		{
			pItemDef->StrangeNotifyEvery = strtoll( szValue, NULL, 10 );
			Assert( pItemDef->StrangeNotifyEvery > 0 );
		}
		for ( int i = 0; ; i++ )
		{
			FETCH_PROPERTY( CFmtStr( "strange_notify_%d", i ) );
			if ( !*szValue )
				break;

			pItemDef->StrangeNotify.AddToTail( strtoll( szValue, NULL, 10 ) );
			Assert( pItemDef->StrangeNotify.Tail() > 0 );
			Assert( i == 0 || pItemDef->StrangeNotify[i - 1] < pItemDef->StrangeNotify[i] );
			Assert( pItemDef->StrangeNotifyEvery == 0 || pItemDef->StrangeNotifyEvery > pItemDef->StrangeNotify[i] );
		}

		FETCH_PROPERTY( "notification_tags" );
		if ( *szValue )
		{
			CSplitString NotificationTags{ szValue, ";" };
			FOR_EACH_VEC( NotificationTags, i )
			{
				pItemDef->NotificationTags.CopyAndAddToTail( NotificationTags[i] );
			}
		}

		FETCH_PROPERTY( "after_description_only_multi_stack" );
		Assert( !V_strcmp( szValue, "" ) || !V_strcmp( szValue, "1" ) || !V_strcmp( szValue, "0" ) );
		pItemDef->AfterDescriptionOnlyMultiStack = !V_strcmp( szValue, "1" );

		FETCH_PROPERTY( "is_basic" );
		Assert( !V_strcmp( szValue, "" ) || !V_strcmp( szValue, "1" ) || !V_strcmp( szValue, "0" ) );
		pItemDef->IsBasic = !V_strcmp( szValue, "1" );

		FETCH_PROPERTY( "game_only" );
		Assert( !V_strcmp( szValue, "" ) || !V_strcmp( szValue, "1" ) || !V_strcmp( szValue, "0" ) );
		pItemDef->GameOnly = !V_strcmp( szValue, "1" );

#ifdef CLIENT_DLL
		pItemDef->Icon = NULL;
		FETCH_PROPERTY( "icon_url" );
		if ( *szValue )
		{
			pItemDef->Icon = GetSteamItemIcon( szValue );
		}

		pItemDef->StyleIcons.SetCount( pItemDef->StyleNames.Count() );
		for ( int i = 0; i < pItemDef->StyleNames.Count(); i++ )
		{
			V_snprintf( szKey, sizeof( szKey ), "icon_url_style_%d", i );
			pItemDef->StyleIcons[i] = pItemDef->Icon;
			FETCH_PROPERTY( szKey );
			if ( *szValue )
			{
				pItemDef->StyleIcons[i] = GetSteamItemIcon( szValue );

				// first style should always match default icon
				Assert( i || pItemDef->StyleIcons[i] == pItemDef->Icon );
			}
		}
#endif
#undef FETCH_PROPERTY

		s_ItemDefs.Insert( id, pItemDef );

		return pItemDef;
	}

	bool DecodeItemData( SteamInventoryResult_t &hResult, const char *szEncodedData )
	{
		GET_INVENTORY_OR_BAIL( false );

		pInventory->DestroyResult( hResult );
		hResult = k_SteamInventoryResultInvalid;

		size_t nEncodedChars = V_strlen( szEncodedData );
		if ( nEncodedChars == 0 )
		{
			return false;
		}

		CUtlMemory<byte> decodedData{ 0, int( nEncodedChars / 2 ) };
		V_hextobinary( szEncodedData, nEncodedChars, decodedData.Base(), decodedData.Count() );

		if ( !pInventory->DeserializeResult( &hResult, decodedData.Base(), decodedData.Count() ) )
		{
			Warning( "ISteamInventory::DeserializeResult failed to create a result.\n" );
			return false;
		}

		return true;
	}

	bool ValidateItemData( bool &bValid, SteamInventoryResult_t hResult, const char *szRequiredSlot, CSteamID requiredSteamID, bool bRequireFresh )
	{
		GET_INVENTORY_OR_BAIL( false );

		EResult eResultStatus = pInventory->GetResultStatus( hResult );
		if ( eResultStatus == k_EResultPending )
		{
			return false;
		}

		if ( eResultStatus != k_EResultOK && ( bRequireFresh || eResultStatus != k_EResultExpired ) )
		{
			Warning( "ReactiveDropInventory::ValidateItemData: EResult %d (%s)\n", eResultStatus, UTIL_RD_EResultToString( eResultStatus ) );
			
			bValid = false;
			return true;
		}

		if ( requiredSteamID.IsValid() && !pInventory->CheckResultSteamID( hResult, requiredSteamID ) )
		{
			Warning( "ReactiveDropInventory::ValidateItemData: not from SteamID %llu\n", requiredSteamID.ConvertToUint64() );

			bValid = false;
			return true;
		}

		if ( szRequiredSlot )
		{
			ReactiveDropInventory::ItemInstance_t instance{ hResult, 0 };
			const ReactiveDropInventory::ItemDef_t *pItemDef = GetItemDef( instance.ItemDefID );
			if ( !pItemDef || !pItemDef->ItemSlotMatches( szRequiredSlot ) )
			{
				Warning( "ReactiveDropInventory::ValidateItemData: item fits in slot '%s', not '%s'\n", pItemDef ? pItemDef->ItemSlot.Get() : "<NO DEF>", szRequiredSlot );

				bValid = false;
				return true;
			}
		}

		bValid = true;
		return true;
	}

#ifdef CLIENT_DLL
	void AddPromoItem( SteamItemDef_t id )
	{
		GET_INVENTORY_OR_BAIL;

		pInventory->AddPromoItem( s_RD_Inventory_Manager.AddCraftItemTask( CRD_Inventory_Manager::CRAFT_PROMO ), id );
	}

	void RequestGenericPromoItems()
	{
		GET_INVENTORY_OR_BAIL;

		pInventory->GrantPromoItems( s_RD_Inventory_Manager.AddCraftItemTask( CRD_Inventory_Manager::CRAFT_PROMO ) );
	}

	void CheckPlaytimeItemGenerators()
	{
#if defined( RD_7A_DROPS ) || defined( RD_7A_DROPS_PRE )
		GET_INVENTORY_OR_BAIL;

		pInventory->TriggerItemDrop( s_RD_Inventory_Manager.AddCraftItemTask( CRD_Inventory_Manager::CRAFT_DROP ), 7000 ); // Playtime Random Material Tokens Weekly
		pInventory->TriggerItemDrop( s_RD_Inventory_Manager.AddCraftItemTask( CRD_Inventory_Manager::CRAFT_DROP ), 7002 ); // Playtime Random Material Tokens Extended
#endif
	}

	void IncrementStrangePropertyOnStartingItems( SteamItemDef_t iAccessoryID, int64_t iAmount, int iPropertyIndex, bool bRelative, bool bAllowCheating )
	{
		s_RD_Inventory_Manager.IncrementStrangePropertyOnStartingItems( iAccessoryID, iAmount, iPropertyIndex, bRelative, bAllowCheating );
	}

	void CommitDynamicProperties()
	{
		s_RD_Inventory_Manager.CommitDynamicProperties();
	}

	template<typename F>
	static void GetLocalInventoryWhere( CUtlVector<ItemInstance_t> &instances, F condition )
	{
		FOR_EACH_VEC( s_RD_Inventory_Manager.m_LocalInventoryCache, i )
		{
			if ( condition( s_RD_Inventory_Manager.m_LocalInventoryCache[i] ) )
			{
				instances.AddToTail( s_RD_Inventory_Manager.m_LocalInventoryCache[i] );
			}
		}
	}

	const ItemInstance_t *GetLocalItemCache( SteamItemInstanceID_t id )
	{
		FOR_EACH_VEC( s_RD_Inventory_Manager.m_LocalInventoryCache, i )
		{
			ItemInstance_t *pInstance = &s_RD_Inventory_Manager.m_LocalInventoryCache[i];
			if ( pInstance->ItemID == id )
			{
				return pInstance;
			}
		}

		return NULL;
	}

	ItemInstance_t *GetLocalItemCacheForModify( SteamItemInstanceID_t id )
	{
		FOR_EACH_VEC( s_RD_Inventory_Manager.m_LocalInventoryCache, i )
		{
			ItemInstance_t *pInstance = &s_RD_Inventory_Manager.m_LocalInventoryCache[i];
			if ( pInstance->ItemID == id )
			{
				return pInstance;
			}
		}

		return NULL;
	}

	void GetItemsForSlot( CUtlVector<ItemInstance_t> &instances, const char *szRequiredSlot )
	{
		GetLocalInventoryWhere( instances, [&]( const ItemInstance_t &instance ) -> bool
			{
				const ItemDef_t *pDef = GetItemDef( instance.ItemDefID );
				return pDef && pDef->ItemSlotMatches( szRequiredSlot );
			} );
	}

	void GetItemsForSlotAndEquipIndex( CUtlVector<ItemInstance_t> &instances, const char *szRequiredSlot, int iEquipIndex )
	{
		GetLocalInventoryWhere( instances, [&]( const ItemInstance_t &instance ) -> bool
			{
				const ItemDef_t *pDef = GetItemDef( instance.ItemDefID );
				return pDef && pDef->ItemSlotMatches( szRequiredSlot ) && pDef->EquipIndex == iEquipIndex;
			} );
	}

	bool CheckMedalEquipCache()
	{
		ISteamUserStats *pUserStats = SteamUserStats();
		Assert( pUserStats );
		if ( !pUserStats )
			return false;

		bool bAnyChanged = false;

		union
		{
			SteamItemInstanceID_t iItemInstance;
			struct
			{
				int32 iLowBits;
				int32 iHighBits;
			} Bits;
		} MedalID;

		for ( int i = 0; i < NELEMS( rd_equipped_medal ); i++ )
		{
			CFmtStr szLowBitsStatName{ "equipped_medal_%da", i + 1 };
			CFmtStr szHighBitsStatName{ "equipped_medal_%db", i + 1 };
			bool bOK = pUserStats->GetStat( szLowBitsStatName, &MedalID.Bits.iLowBits ) && pUserStats->GetStat( szHighBitsStatName, &MedalID.Bits.iHighBits );
			Assert( bOK );
			if ( bOK )
			{
				SteamItemInstanceID_t iCurrentInstance = strtoull( rd_equipped_medal[i].GetString(), NULL, 10 );
				if ( iCurrentInstance == 0 )
				{
					iCurrentInstance = k_SteamItemInstanceIDInvalid;
				}

				if ( iCurrentInstance != MedalID.iItemInstance )
				{
					bAnyChanged = true;
					MedalID.iItemInstance = iCurrentInstance;
					bOK = pUserStats->SetStat( szLowBitsStatName, MedalID.Bits.iLowBits );
					Assert( bOK ); ( void )bOK;
					bOK = pUserStats->SetStat( szHighBitsStatName, MedalID.Bits.iHighBits );
					Assert( bOK ); ( void )bOK;
				}
			}
		}

		return bAnyChanged;
	}

	void ChangeItemStyle( SteamItemInstanceID_t id, int iStyle )
	{
		GET_INVENTORY_OR_BAIL;

		SteamInventoryUpdateHandle_t hUpdate = pInventory->StartUpdateProperties();
		Assert( hUpdate != k_SteamInventoryUpdateHandleInvalid );

		bool bOK = pInventory->SetProperty( hUpdate, id, "style", int64( iStyle ) );
		Assert( bOK );
		if ( !bOK )
			Warning( "Inventory item style property update failed!\n" );

		bOK = pInventory->SubmitUpdateProperties( hUpdate, s_RD_Inventory_Manager.AddCraftItemTask( CRD_Inventory_Manager::CRAFT_USER_DYNAMIC_PROPERTY_UPDATE ) );
		Assert( bOK );
		if ( !bOK )
			Warning( "Inventory item style update submit failed!\n" );
	}

	void QueueSetNotificationSeen( SteamItemInstanceID_t id, int iSeen )
	{
		FOR_EACH_VEC( s_RD_Inventory_Manager.m_CraftingQueue, i )
		{
			if ( s_RD_Inventory_Manager.m_CraftingQueue[i]->m_Type == CRD_Inventory_Manager::CRAFT_DELETE_SILENT && s_RD_Inventory_Manager.m_CraftingQueue[i]->m_iReplaceItemInstance == id )
			{
				// queued for deletion
				return;
			}
		}

		FOR_EACH_VEC( s_RD_Inventory_Manager.m_LocalInventoryCache, i )
		{
			if ( s_RD_Inventory_Manager.m_LocalInventoryCache[i].ItemID == id )
			{
				// update the local inventory cache (prediction!)
				UtlSymId_t iProp = s_RD_Inventory_Manager.m_LocalInventoryCache[i].DynamicProps.AddString( "notification_seen" );
				s_RD_Inventory_Manager.m_LocalInventoryCache[i].DynamicProps[iProp] = CFmtStr{ "%d", iSeen };
				break;
			}
		}

		FOR_EACH_VEC( s_RD_Inventory_Manager.m_QueuedNotificationSeen, i )
		{
			if ( s_RD_Inventory_Manager.m_QueuedNotificationSeen[i].NotificationID == id )
			{
				// already have a queued request; update it
				s_RD_Inventory_Manager.m_QueuedNotificationSeen[i].Seen = iSeen;
			}
		}

		// add to the queue
		s_RD_Inventory_Manager.m_QueuedNotificationSeen.AddToTail( CRD_Inventory_Manager::NotificationSeenUpdate_t{ id, iSeen } );
	}

	void CommitNotificationSeen()
	{
		GET_INVENTORY_OR_BAIL;

		if ( s_RD_Inventory_Manager.m_QueuedNotificationSeen.Count() == 0 )
		{
			// no queued notification seen updates to commit!
			return;
		}

		SteamInventoryResult_t *hResult = NULL;
		FOR_EACH_VEC( s_RD_Inventory_Manager.m_CraftingQueue, i )
		{
			if ( s_RD_Inventory_Manager.m_CraftingQueue[i]->m_Type == CRD_Inventory_Manager::CRAFT_NOTIFICATION_DYNAMIC_PROPERTY_UPDATE )
			{
				hResult = &s_RD_Inventory_Manager.m_CraftingQueue[i]->m_hResult;
			}
		}

		if ( hResult )
		{
			// we're likely going to fail due to an update conflict, but we need to move the queue
			pInventory->DestroyResult( *hResult );
			*hResult = k_SteamInventoryResultInvalid;
		}

		if ( !hResult )
		{
			hResult = s_RD_Inventory_Manager.AddCraftItemTask( CRD_Inventory_Manager::CRAFT_NOTIFICATION_DYNAMIC_PROPERTY_UPDATE );
		}

		// collapse the queue into the in-flight list
		FOR_EACH_VEC( s_RD_Inventory_Manager.m_QueuedNotificationSeen, i )
		{
			bool bFound = false;
			FOR_EACH_VEC( s_RD_Inventory_Manager.m_InFlightNotificationSeen, j )
			{
				if ( s_RD_Inventory_Manager.m_InFlightNotificationSeen[j].NotificationID == s_RD_Inventory_Manager.m_QueuedNotificationSeen[i].NotificationID )
				{
					s_RD_Inventory_Manager.m_InFlightNotificationSeen[j].Seen = s_RD_Inventory_Manager.m_QueuedNotificationSeen[i].Seen;
					bFound = true;
					break;
				}
			}

			if ( !bFound )
			{
				s_RD_Inventory_Manager.m_InFlightNotificationSeen.AddToTail( s_RD_Inventory_Manager.m_QueuedNotificationSeen[i] );
			}
		}

		s_RD_Inventory_Manager.m_QueuedNotificationSeen.Purge();

		Assert( s_RD_Inventory_Manager.m_InFlightNotificationSeen.Count() <= 100 );
		// if we have more than 100 updates, the commit will fail. just throw out some until we're under the limit.
		if ( s_RD_Inventory_Manager.m_InFlightNotificationSeen.Count() > 100 )
		{
			s_RD_Inventory_Manager.m_InFlightNotificationSeen.RemoveMultipleFromHead( s_RD_Inventory_Manager.m_InFlightNotificationSeen.Count() - 100 );
		}

		SteamInventoryUpdateHandle_t hUpdate = pInventory->StartUpdateProperties();
		FOR_EACH_VEC( s_RD_Inventory_Manager.m_InFlightNotificationSeen, i )
		{
			pInventory->SetProperty( hUpdate, s_RD_Inventory_Manager.m_InFlightNotificationSeen[i].NotificationID, "notification_seen", int64_t( s_RD_Inventory_Manager.m_InFlightNotificationSeen[i].Seen ) );
		}
		pInventory->SubmitUpdateProperties( hUpdate, hResult );
	}

	void DeleteNotificationItem( SteamItemInstanceID_t id )
	{
		GET_INVENTORY_OR_BAIL;

		const ItemInstance_t *pCached = GetLocalItemCache( id );
		Assert( pCached );
		if ( !pCached )
		{
			Warning( "Tried to delete notification item %llu which I have no memory of!\n", id );
			return;
		}

		const ItemDef_t *pDef = GetItemDef( pCached->ItemDefID );
		Assert( pDef );
		if ( !pDef )
		{
			Warning( "Tried to delete notification item %llu which is item def id %d which I cannot find!\n", id, pCached->ItemDefID );
			return;
		}

		Assert( pDef->ItemSlotMatches( "notification" ) );
		if ( !pDef->ItemSlotMatches( "notification" ) )
		{
			Warning( "Tried to delete notification item %llu but item def id %d fits in slot %s!\n", id, pCached->ItemDefID, pDef->ItemSlot.Get() );
			return;
		}

		FOR_EACH_VEC( s_RD_Inventory_Manager.m_CraftingQueue, i )
		{
			if ( s_RD_Inventory_Manager.m_CraftingQueue[i]->m_Type == CRD_Inventory_Manager::CRAFT_DELETE_SILENT && s_RD_Inventory_Manager.m_CraftingQueue[i]->m_iReplaceItemInstance == id )
			{
				// request already in-flight
				return;
			}
		}

		FOR_EACH_VEC( s_RD_Inventory_Manager.m_QueuedNotificationSeen, i )
		{
			if ( s_RD_Inventory_Manager.m_QueuedNotificationSeen[i].NotificationID == id )
			{
				// remove from the queue so we don't try to set a property on a deleted item in a batch
				s_RD_Inventory_Manager.m_QueuedNotificationSeen.Remove( i );
				break;
			}
		}

		FOR_EACH_VEC( s_RD_Inventory_Manager.m_InFlightNotificationSeen, i )
		{
			if ( s_RD_Inventory_Manager.m_InFlightNotificationSeen[i].NotificationID == id )
			{
				// remove from the in-flight cache so we don't retry on a deleted item
				s_RD_Inventory_Manager.m_InFlightNotificationSeen.Remove( i );
				break;
			}
		}

		pInventory->ConsumeItem( s_RD_Inventory_Manager.AddCraftItemTask( CRD_Inventory_Manager::CRAFT_DELETE_SILENT, 0, id ), id, 1 );
	}
#endif

	void OnHitConfirm( CBaseEntity *pAttacker, CBaseEntity *pTarget, Vector vecDamagePosition, bool bKilled, bool bDamageOverTime, bool bBlastDamage, int iDisposition, float flDamage, CBaseEntity *pWeapon )
	{
#ifdef CLIENT_DLL
		if ( engine->IsPlayingDemo() )
		{
			return;
		}
#endif

		CASW_Game_Resource *pGameResource = ASWGameResource();
		if ( !pGameResource )
			return;

		if ( !ASWDeathmatchMode() )
		{
			if ( pTarget && pTarget->IsInhabitableNPC() && bKilled )
			{
				s_RD_Inventory_Manager.IncrementStrangePropertyOnEquippedItems( assert_cast< CASW_Inhabitable_NPC * >( pTarget ), 5007, 0, 0, false ); // Alien Kill Streak
			}

			if ( !pAttacker || !pAttacker->IsInhabitableNPC() )
			{
				if ( pAttacker && pTarget && pTarget->IsInhabitableNPC() && !V_stricmp( IGameSystem::MapName(), "rd-reduction2" ) && !V_strcmp( STRING( pAttacker->GetEntityName() ), "trigger_pitworm_hitbox" ) )
				{
					CASW_Inhabitable_NPC *pTargetNPC = assert_cast< CASW_Inhabitable_NPC * >( pTarget );

#ifdef CLIENT_DLL
					static bool s_bRequestedWormToucherMedal = false;
					if ( !s_bRequestedWormToucherMedal && pTargetNPC->IsInhabited() && pTargetNPC->GetCommander() && pTargetNPC->GetCommander()->IsLocalPlayer() )
					{
						AddPromoItem( 42 );
						s_bRequestedWormToucherMedal = true;
					}
#endif

					s_RD_Inventory_Manager.IncrementStrangePropertyOnEquippedItems( pTargetNPC, 42, 1 );
				}

				return;
			}

			CASW_Inhabitable_NPC *pInhabitableAttacker = assert_cast< CASW_Inhabitable_NPC * >( pAttacker );
			if ( pTarget && pTarget->IsAlien() && bKilled )
			{
				s_RD_Inventory_Manager.IncrementStrangePropertiesForWeapon( pInhabitableAttacker, pWeapon, 5002, 1 ); // Aliens Killed
				s_RD_Inventory_Manager.IncrementStrangePropertiesForWeapon( pInhabitableAttacker, pWeapon, 5007, 1 ); // Alien Kill Streak
			}
		}
	}
}

BEGIN_NETWORK_TABLE_NOBASE( CRD_ItemInstance, DT_RD_ItemInstance )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iItemInstanceID ) ),
	RecvPropInt( RECVINFO( m_iItemDefID ) ),
	RecvPropArray( RecvPropInt( RECVINFO( m_iAccessory[0] ) ), m_iAccessory ),
	RecvPropArray( RecvPropInt( RECVINFO( m_nCounter[0] ) ), m_nCounter ),
#else
	SendPropInt( SENDINFO( m_iItemInstanceID ), 64, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iItemDefID ), RD_ITEM_ID_BITS, SPROP_UNSIGNED ),
	SendPropArray( SendPropInt( SENDINFO_ARRAY( m_iAccessory ), RD_ITEM_ACCESSORY_BITS, SPROP_UNSIGNED ), m_iAccessory ),
	// This should be a SendPropArray3, but if we do that we create way too many netprops.
	// Problem is, as a result of this, updating a counter sends a minimum of 630 bits of data.
	// We limit player inventory counter updates to once every few seconds to avoid flooding the network.
	SendPropArray( SendPropInt( SENDINFO_ARRAY( m_nCounterCommitted ), 63, SPROP_UNSIGNED ), m_nCounter ),
#endif
END_NETWORK_TABLE()

CRD_ItemInstance::CRD_ItemInstance()
{
	// clear values without marking network state as updated
	m_iItemInstanceID.m_Value = k_SteamItemInstanceIDInvalid;

	// except for this one
	m_iItemDefID = 0;

	for ( int i = 0; i < m_iAccessory.Count(); i++ )
	{
		m_iAccessory.m_Value[i] = 0;
	}
	for ( int i = 0; i < m_nCounter.Count(); i++ )
	{
		m_nCounter.m_Value[i] = 0;
#ifndef CLIENT_DLL
		m_nCounterCommitted.m_Value[i] = 0;
#endif
	}
}

CRD_ItemInstance::CRD_ItemInstance( const ReactiveDropInventory::ItemInstance_t &instance )
{
	SetFromInstance( instance );
}

CRD_ItemInstance::CRD_ItemInstance( SteamInventoryResult_t hResult, uint32 index )
	: CRD_ItemInstance{ ReactiveDropInventory::ItemInstance_t{ hResult, index } }
{
}

void CRD_ItemInstance::Reset()
{
	// avoid calling NetworkStateChanged if it didn't change
	m_iItemInstanceID.Set( k_SteamItemInstanceIDInvalid );
	m_iItemDefID.Set( 0 );
	FOR_EACH_VEC( m_iAccessory, i )
	{
		m_iAccessory.Set( i, 0 );
	}
	FOR_EACH_VEC( m_nCounter, i )
	{
		m_nCounter.Set( i, 0 );
#ifndef CLIENT_DLL
		m_nCounterCommitted.Set( i, 0 );
#endif
	}
}

bool CRD_ItemInstance::IsSet() const
{
	return m_iItemDefID != 0;
}

void CRD_ItemInstance::SetFromInstance( const ReactiveDropInventory::ItemInstance_t &instance )
{
	Reset();

	m_iItemInstanceID = instance.ItemID;
	m_iItemDefID = instance.ItemDefID;
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_iItemDefID );
	Assert( pDef );
	if ( !pDef )
		return;

	Assert( pDef->CompressedDynamicProps.Count() + ( pDef->AccessoryTag.IsEmpty() ? 0 : RD_ITEM_MAX_ACCESSORIES * RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS_PER_ACCESSORY ) <= RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS );

	FOR_EACH_VEC( pDef->CompressedDynamicProps, i )
	{
		Assert( ReactiveDropInventory::DynamicPropertyDataType( pDef->CompressedDynamicProps[i] ) == FIELD_INTEGER64 );

		if ( !V_stricmp( pDef->CompressedDynamicProps[i], "m_unQuantity" ) )
		{
			m_nCounter.Set( i, instance.Quantity );
		}
		else if ( instance.DynamicProps.Defined( pDef->CompressedDynamicProps[i] ) )
		{
			const char *szPropValue = instance.DynamicProps[instance.DynamicProps.Find( pDef->CompressedDynamicProps[i] )];
			m_nCounter.Set( i, strtoll( szPropValue, NULL, 10 ) );
		}
	}

	if ( !pDef->AccessoryTag.IsEmpty() && instance.Tags.Defined( pDef->AccessoryTag ) )
	{
		const CUtlStringList &accessories = instance.Tags[instance.Tags.Find( pDef->AccessoryTag )];
		Assert( accessories.Count() <= RD_ITEM_MAX_ACCESSORIES );

		int iCounterIndex = pDef->CompressedDynamicProps.Count();
		FOR_EACH_VEC( accessories, i )
		{
			if ( i >= RD_ITEM_MAX_ACCESSORIES )
				break;

			SteamItemDef_t iAccessoryID = strtol( accessories[i], NULL, 10 );
			Assert( iAccessoryID < ( 1 << RD_ITEM_ACCESSORY_BITS ) );
			m_iAccessory.Set( i, iAccessoryID );

			const ReactiveDropInventory::ItemDef_t *pAccessoryDef = ReactiveDropInventory::GetItemDef( iAccessoryID );
			FOR_EACH_VEC( pAccessoryDef->CompressedDynamicProps, j )
			{
				Assert( ReactiveDropInventory::DynamicPropertyDataType( pAccessoryDef->CompressedDynamicProps[j] ) == FIELD_INTEGER64 );
				if ( instance.DynamicProps.Defined( pAccessoryDef->CompressedDynamicProps[j] ) )
				{
					const char *szPropValue = instance.DynamicProps[instance.DynamicProps.Find( pAccessoryDef->CompressedDynamicProps[j] )];
					m_nCounter.Set( iCounterIndex, strtoll( szPropValue, NULL, 10 ) );
				}
				iCounterIndex++;
			}
		}
	}

#ifndef CLIENT_DLL
	CommitCounters();
#endif
}

#ifdef CLIENT_DLL
vgui::IImage *CRD_ItemInstance::GetIcon() const
{
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_iItemDefID );
	Assert( pDef );
	if ( !pDef )
		return NULL;

	int iStyle = GetStyle();
	if ( pDef->StyleIcons.Count() )
	{
		Assert( pDef->StyleIcons.IsValidIndex( iStyle ) );
		if ( pDef->StyleIcons.IsValidIndex( iStyle ) )
		{
			return pDef->StyleIcons[iStyle];
		}
	}

	return pDef->Icon;
}
#endif

int CRD_ItemInstance::GetStyle() const
{
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_iItemDefID );
	Assert( pDef );
	if ( !pDef )
		return 0;

	FOR_EACH_VEC( pDef->CompressedDynamicProps, i )
	{
		if ( !V_stricmp( pDef->CompressedDynamicProps[i], "style" ) )
		{
			return m_nCounter.Get( i );
		}
	}

	return 0;
}

#ifndef CLIENT_DLL
void CRD_ItemInstance::CommitCounters()
{
	FOR_EACH_VEC( m_nCounter, i )
	{
		m_nCounterCommitted.Set( i, m_nCounter.Get( i ) );
	}
}
#endif

BEGIN_NETWORK_TABLE_NOBASE( CRD_ItemInstances_Player, DT_RD_ItemInstances_Player )
#ifdef CLIENT_DLL
	RecvPropDataTable( RECVINFO_DT( m_Medal0 ), 0, &REFERENCE_RECV_TABLE( DT_RD_ItemInstance ) ),
	RecvPropDataTable( RECVINFO_DT( m_Medal1 ), 0, &REFERENCE_RECV_TABLE( DT_RD_ItemInstance ) ),
	RecvPropDataTable( RECVINFO_DT( m_Medal2 ), 0, &REFERENCE_RECV_TABLE( DT_RD_ItemInstance ) ),
#else
	SendPropDataTable( SENDINFO_DT( m_Medal0 ), &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ) ),
	SendPropDataTable( SENDINFO_DT( m_Medal1 ), &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ) ),
	SendPropDataTable( SENDINFO_DT( m_Medal2 ), &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ) ),
#endif
END_NETWORK_TABLE()

CRD_ItemInstance &CRD_ItemInstances_Player::operator []( int index )
{
	COMPILE_TIME_ASSERT( sizeof( ThisClass ) - offsetof( ThisClass, m_Medal0 ) == sizeof( CRD_ItemInstance[RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_PLAYER] ) );
	Assert( index >= 0 && index < RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_PLAYER );

	return ( &m_Medal0 )[index];
}

BEGIN_NETWORK_TABLE_NOBASE( CRD_ItemInstances_Marine_Resource, DT_RD_ItemInstances_Marine_Resource )
#ifdef CLIENT_DLL
	RecvPropDataTable( RECVINFO_DT( m_Suit ), 0, &REFERENCE_RECV_TABLE( DT_RD_ItemInstance ) ),
	RecvPropDataTable( RECVINFO_DT( m_Weapon1 ), 0, &REFERENCE_RECV_TABLE( DT_RD_ItemInstance ) ),
	RecvPropDataTable( RECVINFO_DT( m_Weapon2 ), 0, &REFERENCE_RECV_TABLE( DT_RD_ItemInstance ) ),
	RecvPropDataTable( RECVINFO_DT( m_Extra ), 0, &REFERENCE_RECV_TABLE( DT_RD_ItemInstance ) ),
#else
	SendPropDataTable( SENDINFO_DT( m_Suit ), &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ) ),
	SendPropDataTable( SENDINFO_DT( m_Weapon1 ), &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ) ),
	SendPropDataTable( SENDINFO_DT( m_Weapon2 ), &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ) ),
	SendPropDataTable( SENDINFO_DT( m_Extra ), &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ) ),
#endif
END_NETWORK_TABLE()

CRD_ItemInstance &CRD_ItemInstances_Marine_Resource::operator []( int index )
{
	COMPILE_TIME_ASSERT( sizeof( ThisClass ) - offsetof( ThisClass, m_Suit ) == sizeof( CRD_ItemInstance[RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_MARINE_RESOURCE] ) );
	Assert( index >= 0 && index < RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS_MARINE_RESOURCE );

	return ( &m_Suit )[index];
}

BEGIN_NETWORK_TABLE_NOBASE( CRD_ProjectileData, DT_RD_ProjectileData )
#ifdef CLIENT_DLL
	RecvPropEHandle( RECVINFO( m_hOriginalOwnerMR ) ),
	RecvPropInt( RECVINFO( m_iInventoryEquipSlot ) ),
	RecvPropBool( RECVINFO( m_bFiredByOwner ) ),
#else
	SendPropEHandle( SENDINFO( m_hOriginalOwnerMR ) ),
	SendPropInt( SENDINFO( m_iInventoryEquipSlot ), 2, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bFiredByOwner ) ),
#endif
END_NETWORK_TABLE()

CRD_ProjectileData::CRD_ProjectileData()
{
	m_hOriginalOwnerMR = NULL;
	m_iInventoryEquipSlot = 0;
	m_bFiredByOwner = false;
}

#ifdef GAME_DLL
void CRD_ProjectileData::SetFromWeapon( CBaseEntity *pCreator )
{
	if ( CASW_Weapon *pWeapon = dynamic_cast< CASW_Weapon * >( pCreator ) )
	{
		if ( pWeapon->IsInventoryEquipSlotValid() )
		{
			m_hOriginalOwnerMR = pWeapon->m_hOriginalOwnerMR;
			m_iInventoryEquipSlot = pWeapon->m_iInventoryEquipSlot;

			CASW_Marine_Resource *pMR = pWeapon->m_hOriginalOwnerMR;
			CASW_Marine *pMarine = pWeapon->GetMarine();
			m_bFiredByOwner = pMR && pMarine && pMarine->IsInhabited() && pMarine->GetCommander() == pMR->m_OriginalCommander;
		}
		return;
	}

	if ( CASW_Sentry_Top *pSentry = dynamic_cast< CASW_Sentry_Top * >( pCreator ) )
	{
		pCreator = pSentry->GetSentryBase();
	}

	if ( CASW_Sentry_Base *pSentry = dynamic_cast< CASW_Sentry_Base * >( pCreator ) )
	{
		m_hOriginalOwnerMR = pSentry->m_hOriginalOwnerMR;
		m_iInventoryEquipSlot = pSentry->m_iInventoryEquipSlot;
		m_bFiredByOwner = true;
		return;
	}
}
#endif

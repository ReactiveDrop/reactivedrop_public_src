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
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/RichText.h>
#include "lodepng.h"
#include "c_asw_player.h"
#include "c_asw_marine_resource.h"
#include "c_asw_marine.h"
#include "c_asw_weapon.h"
#include "c_asw_sentry_base.h"
#include "c_asw_game_resource.h"
#include "asw_equipment_list.h"
#include "rd_workshop.h"
#include "rd_missions_shared.h"
#include "asw_deathmatch_mode_light.h"
#include "gameui/swarm/vitemshowcase.h"
#include "filesystem.h"
#include "c_user_message_register.h"
#define CASW_Sentry_Base C_ASW_Sentry_Base
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


COMPILE_TIME_ASSERT( RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MARINE + ASW_NUM_MARINE_PROFILES == RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_WEAPON );
COMPILE_TIME_ASSERT( RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_WEAPON + ASW_NUM_EQUIP_REGULAR == RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_EXTRA );
COMPILE_TIME_ASSERT( RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_EXTRA + ASW_NUM_EQUIP_EXTRA == RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS );
#pragma warning(push)
#pragma warning(disable: 4130) // we're comparing string literals, but if the comparison fails due to memory weirdness, it'll fail at compile time, so it's fine
COMPILE_TIME_ASSERT( ReactiveDropInventory::g_InventorySlotNames[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL] == "medal" );
COMPILE_TIME_ASSERT( ReactiveDropInventory::g_InventorySlotNames[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MARINE] == "marine0" );
COMPILE_TIME_ASSERT( ReactiveDropInventory::g_InventorySlotNames[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_WEAPON] == "weapon0" );
COMPILE_TIME_ASSERT( ReactiveDropInventory::g_InventorySlotNames[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_EXTRA] == "extra0" );
#pragma warning(pop)

#ifdef CLIENT_DLL
ConVar rd_debug_inventory_dynamic_props( "cl_debug_inventory_dynamic_props", "0", FCVAR_NONE, "print debugging messages about dynamic property updates" );
#else
extern ConVar rd_dedicated_server_language;
ConVar rd_debug_inventory_dynamic_props( "sv_debug_inventory_dynamic_props", "0", FCVAR_NONE, "print debugging messages about dynamic property updates" );
#endif

class CSteamItemIcon;

static CUtlMap<SteamItemDef_t, ReactiveDropInventory::ItemDef_t *> s_ItemDefs( DefLessFunc( SteamItemDef_t ) );
static CUtlStringMap<CSteamItemIcon *> s_ItemIcons( false );
static KeyValues *s_pItemDefCache = NULL;
static bool s_bLoadedItemDefs = false;

static bool DynamicPropertyAllowsArbitraryValues( const char *szPropertyName )
{
	return false;
}

static class CRD_Inventory_Manager final : public CAutoGameSystem, public CGameEventListener
{
public:
	CRD_Inventory_Manager() : CAutoGameSystem( "CRD_Inventory_Manager" )
	{
	}

	~CRD_Inventory_Manager()
	{
		ISteamInventory *pInventory = SteamInventory();
#ifdef GAME_DLL
		if ( engine->IsDedicatedServer() )
		{
			pInventory = SteamGameServerInventory();
		}
#endif
		if ( pInventory )
		{
			pInventory->DestroyResult( m_DebugPrintInventoryResult );
#ifdef CLIENT_DLL
			pInventory->DestroyResult( m_PromotionalItemsResult );
			for ( int i = 0; i < NELEMS( m_PlaytimeItemGeneratorResult ); i++ )
			{
				pInventory->DestroyResult( m_PlaytimeItemGeneratorResult[i] );
			}
			pInventory->DestroyResult( m_InspectItemResult );
			pInventory->DestroyResult( m_ExchangeItemsResult );
			pInventory->DestroyResult( m_DynamicPropertyUpdateResult );
			pInventory->DestroyResult( m_PreparingEquipNotification );
			FOR_EACH_VEC( m_PendingEquipSend, i )
			{
				m_PendingEquipSend[i]->deleteThis();

			}
			m_PendingEquipSend.Purge();
#endif
		}
	}

	void PostInit() override
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
			DevWarning( "Cannot access ISteamInventory!\n" );
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
			DevWarning( "Cannot access ISteamInventory!\n" );
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
		QueueSendEquipNotification( true );
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
			DevWarning( "Cannot access ISteamInventory!\n" );
			return;
		}

#ifdef CLIENT_DLL
		pInventory->DestroyResult( m_PreparingEquipNotification );
		m_PreparingEquipNotification = k_SteamInventoryResultInvalid;
		FOR_EACH_VEC( m_PendingEquipSend, i )
		{
			m_PendingEquipSend[i]->deleteThis();

		}
		m_PendingEquipSend.Purge();

		CommitDynamicProperties();

		m_LocalEquipCache.PurgeAndDeleteElements();
#endif
	}

#ifdef CLIENT_DLL
	void QueueSendEquipNotification( bool bForce = false )
	{
		if ( !bForce && ( m_PreparingEquipNotification != k_SteamInventoryResultInvalid || !engine->IsInGame() ) )
		{
			return;
		}

		if ( ASWGameRules() && ASWGameRules()->GetGameState() == ASW_GS_INGAME )
		{
			// we'll try again on map load or instant restart
			return;
		}

		ISteamInventory *pInventory = SteamInventory();
		Assert( pInventory );
		if ( !pInventory )
		{
			return;
		}

		pInventory->DestroyResult( m_PreparingEquipNotification );
		m_PreparingEquipNotification = k_SteamInventoryResultInvalid;
		FOR_EACH_VEC( m_PendingEquipSend, i )
		{
			m_PendingEquipSend[i]->deleteThis();
		}
		m_PendingEquipSend.Purge();

		CUtlVector<SteamItemInstanceID_t> ItemIDs;
		for ( int i = 0; i < RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS; i++ )
		{
			const char *szSlot = ReactiveDropInventory::g_InventorySlotNames[i];
			ConVarRef cv{ CFmtStr{ "rd_equipped_%s", szSlot } };
			Assert( cv.IsValid() );

			SteamItemInstanceID_t id = strtoull( cv.GetString(), NULL, 10 );
			if ( id != 0 && id != k_SteamItemInstanceIDInvalid )
				ItemIDs.AddToTail( id );
		}

		if ( ItemIDs.Count() == 0 )
			SendEquipNotification();
		else
			pInventory->GetItemsByID( &m_PreparingEquipNotification, ItemIDs.Base(), ItemIDs.Count() );
	}

	void SendEquipNotification()
	{
		COMPILE_TIME_ASSERT( RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS < 255 );

		if ( m_PreparingEquipNotification == k_SteamInventoryResultInvalid )
		{
			byte allZeroes[4 + RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS];
			V_memset( allZeroes, 0, sizeof( allZeroes ) );

			*reinterpret_cast< CRC32_t * >( &allZeroes[0] ) = CRC32_ProcessSingleBuffer( &allZeroes[4], RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS );

			SendEquipNotification( allZeroes, sizeof( allZeroes ) );

			return;
		}

		ISteamInventory *pInventory = SteamInventory();
		Assert( pInventory );
		if ( !pInventory )
		{
			return;
		}

		EResult eResult = pInventory->GetResultStatus( m_PreparingEquipNotification );
		if ( eResult != k_EResultOK )
		{
			Warning( "Getting snapshot of equipped items to send to server failed: %d %s\n", eResult, UTIL_RD_EResultToString( eResult ) );

			pInventory->DestroyResult( m_PreparingEquipNotification );
			m_PreparingEquipNotification = k_SteamInventoryResultInvalid;

			return;
		}

		uint32_t nSize{};
		pInventory->GetResultItems( m_PreparingEquipNotification, NULL, &nSize );
		m_LocalEquipCache.PurgeAndDeleteElements();
		m_LocalEquipCache.EnsureCapacity( int( nSize ) );

		for ( uint32_t i = 0; i < nSize; i++ )
		{
			m_LocalEquipCache.AddToTail( new ReactiveDropInventory::ItemInstance_t{ m_PreparingEquipNotification, i } );
		}

		nSize = 0;
		pInventory->SerializeResult( m_PreparingEquipNotification, NULL, &nSize );
		CUtlMemory<byte> buf{ 0, int( 4 + RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS + nSize ) };

		byte *pIndex = buf.Base() + 4;

		for ( int i = 0; i < RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS; i++ )
		{
			const char *szSlot = ReactiveDropInventory::g_InventorySlotNames[i];
			ConVarRef cv{ CFmtStr{ "rd_equipped_%s", szSlot } };
			Assert( cv.IsValid() );

			*pIndex = 0;

			SteamItemInstanceID_t id = strtoull( cv.GetString(), NULL, 10 );
			if ( id != 0 && id != k_SteamItemInstanceIDInvalid )
			{
				FOR_EACH_VEC( m_LocalEquipCache, j )
				{
					if ( m_LocalEquipCache[j]->ItemID == id )
					{
						*pIndex = j + 1;
						break;
					}
				}
			}

			pIndex++;
		}

		pInventory->SerializeResult( m_PreparingEquipNotification, pIndex, &nSize );

		*reinterpret_cast< CRC32_t * >( buf.Base() ) = CRC32_ProcessSingleBuffer( buf.Base() + 4, buf.Count() - 4 );

		pInventory->DestroyResult( m_PreparingEquipNotification );
		m_PreparingEquipNotification = k_SteamInventoryResultInvalid;

		SendEquipNotification( buf.Base(), buf.Count() );
	}

	void SendEquipNotification( const byte *pData, int nLength )
	{
		Assert( m_PendingEquipSend.Count() == 0 );
		Assert( nLength <= RD_EQUIPPED_ITEMS_NOTIFICATION_WORST_CASE_SIZE );
		m_EquipSendParity++;
		Assert( m_EquipSendParity > 0 );
		char szHex[RD_EQUIPPED_ITEMS_NOTIFICATION_PAYLOAD_SIZE_PER_PACKET * 2 + 1]{};
		for ( int i = 0; i < nLength; i += RD_EQUIPPED_ITEMS_NOTIFICATION_PAYLOAD_SIZE_PER_PACKET )
		{
			V_binarytohex( pData + i, MIN( nLength - i, RD_EQUIPPED_ITEMS_NOTIFICATION_PAYLOAD_SIZE_PER_PACKET ), szHex, sizeof( szHex ) );

			KeyValues *pKV = new KeyValues( "EquippedItems" );
			pKV->SetInt( "i", i * 2 );
			pKV->SetInt( "t", nLength * 2 );
			pKV->SetInt( "e", m_EquipSendParity );
			pKV->SetString( "m", szHex );

			if ( i == 0 )
				engine->ServerCmdKeyValues( pKV );
			else
				m_PendingEquipSend.Insert( pKV );
		}

		DevMsg( 3, "Split equipped items notification into %d chunks (%d bytes total payload)\n", m_PendingEquipSend.Count() + 1, nLength );
	}

	void HandleItemDropResult( SteamInventoryResult_t &hResult )
	{
		BaseModUI::ItemShowcase::ShowItems( hResult, 0, -1, BaseModUI::ItemShowcase::MODE_ITEM_DROP );

		SteamInventory()->DestroyResult( hResult );
		hResult = k_SteamInventoryResultInvalid;
	}

	void CacheUserInventory( SteamInventoryResult_t hResult )
	{
		ISteamInventory *pInventory = SteamInventory();
		if ( !pInventory )
		{
			DevWarning( "Failed to cache user inventory for offline play: no ISteamInventory\n" );
			return;
		}

		uint32 nItems{};
		if ( !pInventory->GetResultItems( hResult, NULL, &nItems ) )
		{
			DevWarning( "Failed to retrieve item count from inventory result for cache\n" );
			return;
		}

		m_HighOwnedInventoryDefIDs.Purge();

		KeyValues::AutoDelete pCache{ "IC" };

		for ( uint32 i = 0; i < nItems; i++ )
		{
			ReactiveDropInventory::ItemInstance_t instance{ hResult, i };
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
			DevWarning( "Failed to serialize inventory cache\n" );
			return;
		}

		CFmtStr szCacheFileName{ "cfg/clienti_%llu.dat", SteamUser()->GetSteamID().ConvertToUint64() };
		if ( !g_pFullFileSystem->WriteFile( szCacheFileName, "MOD", buf ) )
		{
			DevWarning( "Failed to write inventory cache\n" );
			return;
		}

		DevMsg( 3, "Successfully wrote inventory cache with %d items\n", nItems );

		if ( m_HighOwnedInventoryDefIDs.Count() )
		{
			CacheItemSchema();
		}
	}

	void CacheItemSchema()
	{
		ISteamInventory *pInventory = SteamInventory();
		if ( !pInventory )
		{
			DevWarning( "Failed to cache item schema for offline play: no ISteamInventory\n" );
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
			DevWarning( "Failed to serialize item schema cache\n" );
			return;
		}

		if ( !g_pFullFileSystem->WriteFile( "cfg/item_schema_cache.dat", "MOD", buf ) )
		{
			DevWarning( "Failed to write item schema cache\n" );
			return;
		}

		DevMsg( 3, "Successfully wrote item schema cache with %d items (skipped %d)\n", nItemDefs - nSkippedDefs, nSkippedDefs );
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
			if ( rd_debug_inventory_dynamic_props.GetBool() )
			{
				Msg( "[C] Not committing dynamic property update (no properties changed)\n" );
			}

			return;
		}

		if ( m_DynamicPropertyUpdateResult != k_SteamInventoryResultInvalid )
		{
			if ( rd_debug_inventory_dynamic_props.GetBool() )
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

		if ( rd_debug_inventory_dynamic_props.GetBool() )
		{
			Msg( "[C] Committing dynamic property update with %d changed properties (handle %016llx)\n", m_PendingDynamicPropertyUpdates.Count(), hUpdate );
		}

		FOR_EACH_VEC( m_PendingDynamicPropertyUpdates, i )
		{
			const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_PendingDynamicPropertyUpdates[i].ItemDefID );
			const char *szProperty = pDef->CompressedDynamicProps[m_PendingDynamicPropertyUpdates[i].PropertyIndex];
			if ( rd_debug_inventory_dynamic_props.GetBool() )
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

		m_PendingDynamicPropertyUpdates.Purge();
	}
#endif

	void IncrementStrangePropertyOnStartingItems( SteamItemDef_t iAccessoryID, int64_t iAmount, int iPropertyIndex = 0, bool bRelative = true, bool bAllowCheating = false )
	{
		Assert( iPropertyIndex >= 0 );

		CASW_Game_Resource *pGameResource = ASWGameResource();
		if ( !pGameResource )
		{
			if ( rd_debug_inventory_dynamic_props.GetBool() )
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

			CASW_Marine_Resource *pMR = pGameResource->GetFirstMarineResourceForPlayer( pPlayer );
			if ( !pMR || pMR->m_OriginalCommander.Get() != pPlayer || ( !pMR->IsInhabited() && pMR->GetHealthPercent() > 0 ) )
				continue;

#ifdef CLIENT_DLL
			CRD_ItemInstance &suitInstance = pMR->m_EquippedSuit;
#else
			CRD_ItemInstance &suitInstance = pPlayer->m_EquippedItemData[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MARINE + pMR->GetProfileIndex()];
#endif

			ModifyAccessoryDynamicPropValue( suitInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );

			for ( int j = 0; j < ASW_NUM_INVENTORY_SLOTS; j++ )
			{
#ifdef CLIENT_DLL
				CRD_ItemInstance &weaponInstance = pMR->m_StartingEquipWeapons[j];
#else
				CASW_EquipItem *pEquip = g_ASWEquipmentList.GetItemForSlot( j, pMR->m_iInitialWeaponsInSlots[j] );
				if ( !pEquip || pEquip->m_iInventoryEquipIndex == -1 )
					continue;
				CRD_ItemInstance &weaponInstance = pPlayer->m_EquippedItemData[pEquip->m_iInventoryEquipIndex];
#endif

				ModifyAccessoryDynamicPropValue( weaponInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
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

		if ( CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC ) )
		{
			CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
			if ( pMR && pMR->m_OriginalCommander.Get() == pMR->m_Commander.Get() )
			{
#ifdef CLIENT_DLL
				CRD_ItemInstance &suitInstance = pMR->m_EquippedSuit;
#else
				CRD_ItemInstance &suitInstance = pMR->m_OriginalCommander->m_EquippedItemData[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MARINE + pMR->GetProfileIndex()];
#endif

				ModifyAccessoryDynamicPropValue( suitInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
			}
		}

		AccountID_t iRequiredAccount = pNPC->GetCommander()->GetSteamID().GetAccountID();

		for ( int i = 0; i < MAX_WEAPONS; i++ )
		{
			CASW_Weapon *pWeapon = pNPC->GetASWWeapon( i );
			if ( pWeapon && pWeapon->m_iOriginalOwnerSteamAccount == iRequiredAccount )
			{
#ifdef CLIENT_DLL
				CRD_ItemInstance &weaponInstance = pWeapon->m_InventoryItemData;
#else
				if ( pWeapon->m_iInventoryEquipSlotIndex == -1 )
					continue;
				CRD_ItemInstance &weaponInstance = pWeapon->m_hOriginalOwnerPlayer->m_EquippedItemData[pWeapon->m_iInventoryEquipSlotIndex];
#endif

				ModifyAccessoryDynamicPropValue( weaponInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
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

		if ( CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC ) )
		{
			CASW_Marine_Resource *pMR = pMarine->GetMarineResource();
			if ( pMR && pMR->m_OriginalCommander.Get() == pMR->m_Commander.Get()
#ifdef CLIENT_DLL
				&& pMR->m_OriginalCommander->IsLocalPlayer()
#endif
				)
			{
#ifdef CLIENT_DLL
				CRD_ItemInstance &suitInstance = pMR->m_EquippedSuit;
#else
				CRD_ItemInstance &suitInstance = pMR->m_OriginalCommander->m_EquippedItemData[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MARINE + pMR->GetProfileIndex()];
#endif

				ModifyAccessoryDynamicPropValue( suitInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
			}
		}

		AccountID_t iRequiredAccount = pNPC->GetCommander()->GetSteamID().GetAccountID();
		if ( pWeapon && ( bAllowBorrowed || pWeapon->m_iOriginalOwnerSteamAccount == iRequiredAccount )
#ifdef CLIENT_DLL
			&& SteamUser() && pWeapon->m_iOriginalOwnerSteamAccount == SteamUser()->GetSteamID().GetAccountID()
#endif
			)
		{
#ifdef CLIENT_DLL
			CRD_ItemInstance &weaponInstance = pWeapon->m_InventoryItemData;
#else
			if ( pWeapon->m_iInventoryEquipSlotIndex == -1 || !pWeapon->m_hOriginalOwnerPlayer )
				return;
			CRD_ItemInstance &weaponInstance = pWeapon->m_hOriginalOwnerPlayer->m_EquippedItemData[pWeapon->m_iInventoryEquipSlotIndex];
#endif

			ModifyAccessoryDynamicPropValue( weaponInstance, iAccessoryID, iPropertyIndex, iAmount, bRelative, bAllowCheating );
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

	bool ModifyAccessoryDynamicPropValue( CRD_ItemInstance &instance, SteamItemDef_t iAccessoryID, int iPropertyIndex, int64_t iAmount, bool bRelative = true, bool bAllowCheating = false )
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
			if ( rd_debug_inventory_dynamic_props.GetBool() )
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
		FOR_EACH_VEC( m_LocalEquipCache, i )
		{
			if ( m_LocalEquipCache[i]->ItemID == instance.m_iItemInstanceID )
			{
				pLocalInstance = m_LocalEquipCache[i];
				break;
			}
		}
		Assert( pLocalInstance );
		if ( !pLocalInstance )
		{
			if ( rd_debug_inventory_dynamic_props.GetBool() )
			{
				Msg( "[C] Could not find local instance of item %llu '%s' for property update.\n", instance.m_iItemInstanceID, pItemDef->Name.Get() );
			}

			return false;
		}
#endif

		if ( instance.m_iItemDefID == iAccessoryID )
		{
			ModifyDynamicPropValueHelper( instance, iAccessoryID, iPropertyIndex, iCombinedIndex, szPropertyName, pItemDef, pAccessoryDef, pLocalInstance, iAmount, bRelative );
			return true;
		}

		iCombinedIndex += pItemDef->CompressedDynamicProps.Count();

		FOR_EACH_VEC( instance.m_iAccessory, i )
		{
			if ( instance.m_iAccessory[i] == iAccessoryID )
			{
				ModifyDynamicPropValueHelper( instance, iAccessoryID, iPropertyIndex, iCombinedIndex, szPropertyName, pItemDef, pAccessoryDef, pLocalInstance, iAmount, bRelative );
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

	void ModifyDynamicPropValueHelper( CRD_ItemInstance &instance, SteamItemDef_t iAccessoryID, int iPropertyIndex, int iCombinedIndex, const char *szPropertyName,
		const ReactiveDropInventory::ItemDef_t *pItemDef, const ReactiveDropInventory::ItemDef_t *pAccessoryDef, ReactiveDropInventory::ItemInstance_t *pLocalInstance, int64_t iAmount, bool bRelative )
	{
		int64_t iCounterBefore = instance.m_nCounter[iCombinedIndex];
#ifdef CLIENT_DLL
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
				ModifyAccessoryDynamicPropValue( instance, iAccessoryID, 1, iCounterAfter, false );
			}
		}

#ifdef CLIENT_DLL
		pLocalInstance->DynamicProps[szPropertyName] = CFmtStr( "%lld", iCounterAfter );
		pUpdate->NewValue = iCounterAfter;
#endif
		instance.m_nCounter.GetForModify( iCombinedIndex ) = iCounterAfter;

		if ( rd_debug_inventory_dynamic_props.GetBool() )
		{
			DevMsg( "[%c] Item %llu #%d '%s' dynamic property '%s' '%s' changed (%s) %+lld from %lld to %lld.\n", IsClientDll() ? 'C' : 'S', instance.m_iItemInstanceID, instance.m_iItemDefID, pItemDef->Name.Get(), pAccessoryDef->Name.Get(), szPropertyName, bRelative ? "relative" : "absolute", iAmount, iCounterBefore, iCounterAfter );
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

			if ( FStrEq( event->GetName(), "mission_failure" ) )
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
				CommitDynamicProperties();
			else if ( m_DynamicPropertyUpdateResult == k_SteamInventoryResultInvalid )
				QueueSendEquipNotification();
#endif
			return;
		}

	}

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
		{
			Msg( "Result %08x (%s, age %d sec) has %d items:\n", hResult, UTIL_RD_EResultToString( pInventory->GetResultStatus( hResult ) ), SteamUtils()->GetServerRealTime() - pInventory->GetResultTimestamp( hResult ), count );

			uint32 nSerializedBufferSize{};
			if ( pInventory->SerializeResult( hResult, NULL, &nSerializedBufferSize ) )
			{
				byte *pSerialized = ( byte * )stackalloc( nSerializedBufferSize );
				pInventory->SerializeResult( hResult, pSerialized, &nSerializedBufferSize );
				char *szSerialized = ( char * )stackalloc( nSerializedBufferSize * 2 + 1 );
				V_binarytohex( pSerialized, nSerializedBufferSize, szSerialized, nSerializedBufferSize * 2 + 1 );
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
	}

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

#ifdef CLIENT_DLL
		if ( pParam->m_handle == m_PromotionalItemsResult )
		{
			HandleItemDropResult( m_PromotionalItemsResult );
			if ( m_PromotionalItemsNext.Count() )
			{
				pInventory->AddPromoItems( &m_PromotionalItemsResult, m_PromotionalItemsNext.Base(), m_PromotionalItemsNext.Count() );
				m_PromotionalItemsNext.Purge();
			}
			else if ( m_bRequestGenericPromotionalItemsAgain )
			{
				pInventory->GrantPromoItems( &m_PromotionalItemsResult );
				m_bRequestGenericPromotionalItemsAgain = false;
			}

			return;
		}

		for ( int i = 0; i < NELEMS( m_PlaytimeItemGeneratorResult ); i++ )
		{
			if ( pParam->m_handle == m_PlaytimeItemGeneratorResult[i] )
			{
				HandleItemDropResult( m_PlaytimeItemGeneratorResult[i] );
			}
		}

		if ( pParam->m_handle == m_ExchangeItemsResult )
		{
			DebugPrintResult( m_ExchangeItemsResult );
			BaseModUI::ItemShowcase::ShowItems( m_ExchangeItemsResult, 0, -1, BaseModUI::ItemShowcase::MODE_ITEM_DROP );

			pInventory->DestroyResult( m_ExchangeItemsResult );
			m_ExchangeItemsResult = k_SteamInventoryResultInvalid;

			return;
		}

		if ( pParam->m_handle == m_InspectItemResult )
		{
			BaseModUI::ItemShowcase::ShowItems( pParam->m_handle, 0, -1, BaseModUI::ItemShowcase::MODE_INSPECT );

			pInventory->DestroyResult( m_InspectItemResult );
			m_InspectItemResult = k_SteamInventoryResultInvalid;

			return;
		}

		if ( pParam->m_handle == m_GetFullInventoryForCacheResult )
		{
			CacheUserInventory( m_GetFullInventoryForCacheResult );

			pInventory->DestroyResult( m_GetFullInventoryForCacheResult );
			m_GetFullInventoryForCacheResult = k_SteamInventoryResultInvalid;

			return;
		}

		if ( pParam->m_handle == m_DynamicPropertyUpdateResult )
		{
			if ( rd_debug_inventory_dynamic_props.GetBool() )
			{
				DebugPrintResult( pParam->m_handle );
			}

			EResult eResult = pInventory->GetResultStatus( pParam->m_handle );

			pInventory->DestroyResult( m_DynamicPropertyUpdateResult );
			m_DynamicPropertyUpdateResult = k_SteamInventoryResultInvalid;

			if ( m_bWantExtraDynamicPropertyCommit )
			{
				CommitDynamicProperties();
				m_bWantExtraDynamicPropertyCommit = false;
			}
			else if ( eResult == k_EResultOK )
			{
				pInventory->DestroyResult( m_GetFullInventoryForCacheResult );
				m_GetFullInventoryForCacheResult = k_SteamInventoryResultInvalid;

				pInventory->GetAllItems( &m_GetFullInventoryForCacheResult );

				QueueSendEquipNotification();
			}
		}

		if ( pParam->m_handle == m_PreparingEquipNotification )
		{
			SendEquipNotification();

			return;
		}
#else
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
			if ( !pPlayer )
				continue;

			if ( pParam->m_handle == pPlayer->m_EquippedItemsResult )
			{
				byte nIndex[RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS]{};
				V_hextobinary( static_cast< const char * >( pPlayer->m_EquippedItemsReceiving.Base() ) + 8, RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS * 2, nIndex, sizeof( nIndex ) );

				bool bValid = false;
				if ( !ReactiveDropInventory::ValidateEquipItemData( bValid, pParam->m_handle, nIndex, pPlayer->GetSteamIDAsUInt64() ) )
				{
					Assert( !"ValidateEquipItemData failed as 'not-ready' but we are in the ready callback!" );
				}

				for ( int j = 0; j < RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS; j++ )
				{
					if ( nIndex[j] == 0 || !bValid )
					{
						pPlayer->m_EquippedItemData[j].Reset();
					}
					else
					{
						pPlayer->m_EquippedItemData[j].SetFromInstance( ReactiveDropInventory::ItemInstance_t{ pParam->m_handle, nIndex[j] - 1u } );
					}
				}

				return;
			}
		}
#endif
	}

	float m_flDefsUpdateTime{ 0.0f };
	SteamInventoryResult_t m_DebugPrintInventoryResult{ k_SteamInventoryResultInvalid };
#ifdef CLIENT_DLL
	SteamInventoryResult_t m_PromotionalItemsResult{ k_SteamInventoryResultInvalid };
	CUtlVector<SteamItemDef_t> m_PromotionalItemsNext{};
	bool m_bRequestGenericPromotionalItemsAgain{ false };
	SteamInventoryResult_t m_PlaytimeItemGeneratorResult[3]{ k_SteamInventoryResultInvalid, k_SteamInventoryResultInvalid, k_SteamInventoryResultInvalid };
	SteamInventoryResult_t m_InspectItemResult{ k_SteamInventoryResultInvalid };
	SteamInventoryResult_t m_ExchangeItemsResult{ k_SteamInventoryResultInvalid };
	SteamInventoryResult_t m_GetFullInventoryForCacheResult{ k_SteamInventoryResultInvalid };
	SteamInventoryResult_t m_PreparingEquipNotification{ k_SteamInventoryResultInvalid };
	CUtlQueue<KeyValues *> m_PendingEquipSend{};
	int m_EquipSendParity{};
	CUtlVectorAutoPurge<ReactiveDropInventory::ItemInstance_t *> m_LocalEquipCache;

	STEAM_CALLBACK( CRD_Inventory_Manager, OnSteamInventoryFullUpdate, SteamInventoryFullUpdate_t )
	{
		// don't write the same cache twice in a row
		if ( pParam->m_handle == m_GetFullInventoryForCacheResult )
			return;

		CacheUserInventory( pParam->m_handle );
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
void __MsgFunc_RDEquippedItemsACK( bf_read &msg )
{
	int iParity = msg.ReadLong();
	Assert( s_RD_Inventory_Manager.m_EquipSendParity == iParity );
	if ( s_RD_Inventory_Manager.m_EquipSendParity != iParity )
		return;

	Assert( s_RD_Inventory_Manager.m_PendingEquipSend.Count() != 0 );
	if ( s_RD_Inventory_Manager.m_PendingEquipSend.Count() == 0 )
		return;

	engine->ServerCmdKeyValues( s_RD_Inventory_Manager.m_PendingEquipSend.RemoveAtHead() );

	DevMsg( 3, "Sending next equipped items notification chunk (%d remain)\n", s_RD_Inventory_Manager.m_PendingEquipSend.Count() );
}
USER_MESSAGE_REGISTER( RDEquippedItemsACK );

static void RD_Equipped_Item_Changed( IConVar *var, const char *pOldValue, float flOldValue )
{
	s_RD_Inventory_Manager.QueueSendEquipNotification();
}
ConVar rd_equipped_medal( "rd_equipped_medal", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped medal.", RD_Equipped_Item_Changed );
ConVar rd_equipped_marine[ASW_NUM_MARINE_PROFILES]
{
	{ "rd_equipped_marine0", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Sarge's suit.", RD_Equipped_Item_Changed },
	{ "rd_equipped_marine1", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Wildcat's suit.", RD_Equipped_Item_Changed },
	{ "rd_equipped_marine2", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Faith's suit.", RD_Equipped_Item_Changed },
	{ "rd_equipped_marine3", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Crash's suit.", RD_Equipped_Item_Changed },
	{ "rd_equipped_marine4", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Jaeger's suit.", RD_Equipped_Item_Changed },
	{ "rd_equipped_marine5", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Wolfe's suit.", RD_Equipped_Item_Changed },
	{ "rd_equipped_marine6", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Bastille's suit.", RD_Equipped_Item_Changed },
	{ "rd_equipped_marine7", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Vegas's suit.", RD_Equipped_Item_Changed },
};
ConVar rd_equipped_weapon[ASW_NUM_EQUIP_REGULAR]
{
	{ "rd_equipped_weapon0", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for 22A3-1 Assault Rifle.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon1", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for 22A7-Z Prototype Assault Rifle.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon2", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for S23A SynTek Autogun.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon3", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for M42 Vindicator.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon4", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for M73 Twin Pistols.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon5", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Advanced Sentry Gun.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon6", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Heal Beacon.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon7", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Ammo Satchel.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon8", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Model 35 Pump-action Shotgun.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon9", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Tesla Cannon.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon10", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Precision Rail Rifle.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon11", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Medical Gun.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon12", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for K80 Personal Defense Weapon.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon13", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for M868 Flamer Unit.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon14", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Freeze Sentry Gun.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon15", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Minigun.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon16", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for AVK-36 Marksman Rifle.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon17", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Incendiary Sentry Gun.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon18", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Chainsaw.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon19", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF High Velocity Sentry Cannon.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon20", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Grenade Launcher.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon21", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for PS50 Bulldog.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon22", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF HAS42 Devastator.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon23", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for 22A4-2 Combat Rifle.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon24", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Medical Amplifier Gun.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon25", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for 22A5 Heavy Assault Rifle.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon26", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Medical SMG.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon27", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Fire Extinguisher.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon28", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Mining Laser.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon29", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for 50CMG4-1.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon30", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Flechette Launcher.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon31", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Ricochet Rifle.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon32", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Ammo Bag.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon33", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Medical Satchel.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon34", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Overwatch Standard Issue Pulse Rifle.", RD_Equipped_Item_Changed },
#ifdef RD_7A_WEAPONS
	{ "rd_equipped_weapon35", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for <<Cryo Cannon>>.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon36", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for <<Plasma Thrower>>.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon37", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for <<Hack Tool>>.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon38", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for <<Railgun Sentry>>.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon39", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for <<Energy Shield>>.", RD_Equipped_Item_Changed },
	{ "rd_equipped_weapon40", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for <<Revive Tool>>.", RD_Equipped_Item_Changed },
#endif
};
ConVar rd_equipped_extra[ASW_NUM_EQUIP_EXTRA]
{
	{ "rd_equipped_extra0", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Personal Healing Kit.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra1", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Hand Welder.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra2", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for SM75 Combat Flares.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra3", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for ML30 Laser Trip Mine.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra4", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for l3a Tactical Heavy Armor.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra5", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for X33 Damage Amplifier.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra6", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Hornet Barrage.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra7", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for CR18 Freeze Grenades.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra8", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Adrenaline.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra9", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Tesla Sentry Coil.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra10", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for v45 Electric Charged Armor.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra11", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for M478 Proximity Incendiary Mines.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra12", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Flashlight Attachment.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra13", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for IAF Power Fist Attachment.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra14", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for FG01 Hand Grenades.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra15", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for MNV34 Nightvision Goggles.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra16", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for MTD6 Smart Bomb.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra17", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for TG-05 Gas Grenades.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra18", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for T75 Explosives.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra19", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Displacement 'Blink' Pack.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra20", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Short Range Assault Jets.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra21", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for Swarm Bait.", RD_Equipped_Item_Changed },
#ifdef RD_7A_WEAPONS
	{ "rd_equipped_extra22", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for <<Stun Grenades>>.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra23", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for <<Incendiary Grenades>>.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra24", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for <<Speed Burst>>.", RD_Equipped_Item_Changed },
	{ "rd_equipped_extra25", "0", FCVAR_ARCHIVE | FCVAR_HIDDEN, "Steam inventory item ID of equipped replacement for <<Shield Bubble>>.", RD_Equipped_Item_Changed },
#endif
};

CON_COMMAND( rd_debug_print_inventory, "" )
{
	SteamInventory()->GetAllItems( &s_RD_Inventory_Manager.m_DebugPrintInventoryResult );
}

CON_COMMAND( rd_debug_inspect_entire_inventory, "inspect every item in your inventory" )
{
	SteamInventory()->GetAllItems( &s_RD_Inventory_Manager.m_InspectItemResult );
}

CON_COMMAND( rd_debug_inspect_own_item, "inspect an item you own by ID" )
{
	SteamItemInstanceID_t id = strtoull( args.Arg( 1 ), NULL, 10 );
	if ( !id )
	{
		Msg( "Usage: rd_debug_inspect_own_item [item instance ID]\n" );
		return;
	}

	SteamInventory()->GetItemsByID( &s_RD_Inventory_Manager.m_InspectItemResult, &id, 1 );
}

CON_COMMAND( rd_econ_item_preview, "inspect an item using a code from the Steam Community backpack" )
{
	if ( args.ArgC() != 2 )
	{
		Msg( "missing or invalid inspect item code\n" );
		return;
	}

	SteamInventory()->InspectItem( &s_RD_Inventory_Manager.m_InspectItemResult, args.Arg( 1 ) );
}

class CSteamItemIcon : public vgui::IImage
{
public:
	CSteamItemIcon( const char *szURL )
	{
		m_iTextureID = 0;
		m_Color.SetColor( 255, 255, 255, 255 );
		m_nX = 0;
		m_nY = 0;
		m_nWide = 512;
		m_nTall = 512;

		ISteamHTTP *pHTTP = SteamHTTP();
		Assert( pHTTP );
		if ( pHTTP )
		{
			// The medal images send a Cache-Control header of "public, max-age=315569520" (1 decade).
			// The Steam API will automatically cache stuff for us, so we don't have to manage cache ourselves.
			HTTPRequestHandle hRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodGET, szURL );
			SteamAPICall_t hAPICall;
			if ( pHTTP->SendHTTPRequest( hRequest, &hAPICall ) )
			{
				m_HTTPRequestCompleted.Set( hAPICall, this, &CSteamItemIcon::OnRequestCompleted );
			}
			else
			{
				Warning( "Sending request for inventory item icon failed!\n" );
			}
		}
		else
		{
			Warning( "No ISteamHTTP access - cannot fetch inventory item icon.\n" );
		}
	}

	virtual ~CSteamItemIcon()
	{
		if ( m_iTextureID )
		{
			vgui::surface()->DestroyTextureID( m_iTextureID );
		}
	}

	virtual void Paint()
	{
		if ( !m_iTextureID )
		{
			return;
		}

		vgui::surface()->DrawSetColor( m_Color );
		vgui::surface()->DrawSetTexture( m_iTextureID );
		vgui::surface()->DrawTexturedRect( m_nX, m_nY, m_nX + m_nWide, m_nY + m_nTall );
	}

	virtual void SetPos( int x, int y )
	{
		m_nX = x;
		m_nY = y;
	}

	virtual void GetContentSize( int &wide, int &tall )
	{
		wide = m_nWide;
		tall = m_nTall;
	}

	virtual void GetSize( int &wide, int &tall )
	{
		wide = m_nWide;
		tall = m_nTall;
	}

	virtual void SetSize( int wide, int tall )
	{
		m_nWide = wide;
		m_nTall = tall;
	}

	virtual void SetColor( Color col )
	{
		m_Color = col;
	}

	virtual bool Evict() { return false; }
	// Using GetNumFrames to signal whether the HTTP request has finished.
	virtual int GetNumFrames() { return m_HTTPRequestCompleted.IsActive() ? 0 : 1; }
	virtual void SetFrame( int nFrame ) {}
	virtual vgui::HTexture GetID() { return m_iTextureID; }
	virtual void SetRotation( int iRotation ) {}

	static CSteamItemIcon *Get( const char *szURL )
	{
		UtlSymId_t index = s_ItemIcons.Find( szURL );
		if ( index != s_ItemIcons.InvalidIndex() )
		{
			return s_ItemIcons[index];
		}

		return s_ItemIcons[szURL] = new CSteamItemIcon( szURL );
	}

private:
	CRC32_t m_URLHash;
	vgui::HTexture m_iTextureID;
	Color m_Color;
	int m_nX, m_nY;
	unsigned m_nWide, m_nTall;

	void OnRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
	{
		if ( bIOFailure || !pParam->m_bRequestSuccessful )
		{
			Warning( "Failed to fetch inventory item icon: IO Failure\n" );
			return;
		}

		ISteamHTTP *pHTTP = SteamHTTP();
		Assert( pHTTP );
		if ( !pHTTP )
		{
			Warning( "No access to ISteamHTTP inside callback from HTTP request!\n" );
			return;
		}

		if ( pParam->m_eStatusCode != k_EHTTPStatusCode200OK )
		{
			Warning( "Status code %d from inventory item icon request - trying to parse anyway.\n", pParam->m_eStatusCode );
		}

		CUtlMemory<uint8_t> data( 0, pParam->m_unBodySize );
		if ( !pHTTP->GetHTTPResponseBodyData( pParam->m_hRequest, data.Base(), pParam->m_unBodySize ) )
		{
			Warning( "Failed to get inventory item icon from successful request. Programmer error?\n" );
			return;
		}

		pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );

		uint8_t *rgba = NULL;
		unsigned error = lodepng_decode32( &rgba, &m_nWide, &m_nTall, data.Base(), pParam->m_unBodySize );
		if ( error )
		{
			Warning( "Decoding inventory item icon: lodepng error %d: %s\n", error, lodepng_error_text( error ) );
		}

		m_iTextureID = vgui::surface()->CreateNewTextureID( true );
		vgui::surface()->DrawSetTextureRGBA( m_iTextureID, rgba, m_nWide, m_nTall );

		free( rgba );
	}

	CCallResult<CSteamItemIcon, HTTPRequestCompleted_t> m_HTTPRequestCompleted;
};
#endif

namespace ReactiveDropInventory
{
#ifdef CLIENT_DLL
#define GET_INVENTORY_OR_BAIL \
	ISteamInventory *pInventory = SteamInventory(); \
	if ( !pInventory ) \
		return
#else
#define GET_INVENTORY_OR_BAIL \
	ISteamInventory *pInventory = engine->IsDedicatedServer() ? SteamGameServerInventory() : SteamInventory(); \
	if ( !pInventory ) \
		return
#endif

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

	static bool ParseDynamicProps( CUtlStringMap<CUtlString> & props, const char *szDynamicProps )
	{
		jsmn_parser parser;
		jsmntok_t tokens[256];

		jsmn_init( &parser );

		int count = jsmn_parse( &parser, szDynamicProps, V_strlen( szDynamicProps ), tokens, NELEMS( tokens ) );
		if ( count <= 0 )
		{
			DevWarning( "Parsing item dynamic property data: corrupt data type %d\n", -count );
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

	void ItemInstance_t::FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc ) const
	{
		V_UTF8ToUnicode( szDesc, wszBuf, sizeOfBufferInBytes );

		GET_INVENTORY_OR_BAIL;

		char szToken[128]{};
		wchar_t wszReplacement[1024]{};

		for ( size_t i = 0; i < sizeOfBufferInBytes / sizeof( wchar_t ); i++ )
		{
			if ( wszBuf[i] == L'\0' )
			{
				break;
			}

			if ( wszBuf[i] != L'%' )
			{
				continue;
			}

			V_wcsncpy( wszReplacement, L"MISSING", sizeof( wszReplacement ) );

			size_t tokenLength = 1;
			while ( wszBuf[i + tokenLength] != L'%' && wszBuf[i + tokenLength] != L':' )
			{
				if ( wszBuf[i + tokenLength] == L'\0' )
				{
					return;
				}

				Assert( wszBuf[i + tokenLength] < 0x80 ); // assume ASCII
				szToken[tokenLength - 1] = ( char )wszBuf[i + tokenLength];

				tokenLength++;

				Assert( tokenLength < sizeof( szToken ) );
			}

			szToken[tokenLength - 1] = '\0';

			if ( wszBuf[i + tokenLength] == L':' )
			{
				size_t tokenReplacementLength = 0;
				tokenLength++;

				while ( wszBuf[i + tokenLength] != L'%' )
				{
					if ( wszBuf[i + tokenLength] == L'\0' )
					{
						return;
					}

					szToken[tokenReplacementLength] = wszBuf[i + tokenLength];

					tokenReplacementLength++;
					tokenLength++;

					Assert( tokenReplacementLength < NELEMS( wszReplacement ) );
				}

				wszReplacement[tokenReplacementLength] = L'\0';
			}

			tokenLength++;

			if ( tokenLength == 2 )
			{
				// special case: %% is just %
				V_memmove( &wszBuf[i + 1], &wszBuf[i + 2], sizeOfBufferInBytes - ( i + 2 ) * sizeof( wchar_t ) );
				i++;
				continue;
			}

			if ( !V_stricmp( szToken, "m_unQuantity" ) )
			{
				// special case: m_unQuantity is not stored in dynamic_props
				V_wcsncpy( wszReplacement, UTIL_RD_CommaNumber( Quantity ), sizeof( wszReplacement ) );
			}

			if ( !DynamicProps.Defined( szToken ) )
			{
				// use replacement value as-is
			}
			else if ( DynamicPropertyAllowsArbitraryValues( szToken ) )
			{
				V_UTF8ToUnicode( DynamicProps[DynamicProps.Find( szToken )], wszReplacement, sizeof( wszReplacement ) );
			}
			else
			{
				int64_t nValue = strtoll( DynamicProps[DynamicProps.Find( szToken )], NULL, 10 );
				V_wcsncpy( wszReplacement, UTIL_RD_CommaNumber( nValue ), sizeof( wszReplacement ) );
			}

			size_t replacementLength = 0;
			while ( wszReplacement[replacementLength] )
			{
				replacementLength++;
			}

			if ( i + replacementLength >= sizeOfBufferInBytes / sizeof( wchar_t ) )
			{
				replacementLength = ( sizeOfBufferInBytes - i - 1 ) / sizeof( wchar_t );
			}

			V_memmove( &wszBuf[i + replacementLength], &wszBuf[i + tokenLength], sizeOfBufferInBytes - ( i + MAX( replacementLength, tokenLength ) ) * sizeof( wchar_t ) );
			V_memmove( &wszBuf[i], wszReplacement, replacementLength * sizeof( wchar_t ) );
			if ( replacementLength > tokenLength )
			{
				wszBuf[sizeOfBufferInBytes / sizeof( wchar_t ) - 1] = L'\0';
			}
		}

#ifdef DBGFLAG_ASSERT
		wchar_t *wszTemp = new wchar_t[sizeOfBufferInBytes / sizeof( wchar_t )];
		CRD_ItemInstance reduced{ *this };
		reduced.FormatDescription( wszTemp, sizeOfBufferInBytes, szDesc );
		Assert( !V_wcscmp( wszBuf, wszTemp ) );
		delete[] wszTemp;
#endif
	}

#ifdef CLIENT_DLL
	void ItemInstance_t::FormatDescription( vgui::RichText *pRichText ) const
	{
		CRD_ItemInstance reduced{ *this };
		reduced.FormatDescription( pRichText );
	}
#endif

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
			if ( DynamicPropertyAllowsArbitraryValues( DynamicProps.String( i ) ) )
				pKV->SetString( CFmtStr( "x/%s", DynamicProps.String( i ) ), DynamicProps[i] );
			else
				pKV->SetUint64( CFmtStr( "x/%s", DynamicProps.String( i ) ), strtoll( DynamicProps[i], NULL, 10 ) );
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

		FETCH_PROPERTY( "item_slot" );
		pItemDef->ItemSlot = szValue;
		FETCH_PROPERTY( "tags" );
		ParseTags( pItemDef->Tags, szValue );
		FETCH_PROPERTY( "allowed_tags_from_tools" );
		ParseTags( pItemDef->AllowedTagsFromTools, szValue );
		FETCH_PROPERTY( "accessory_tag" );
		pItemDef->AccessoryTag = szValue;
		FETCH_PROPERTY( "accessory_limit" );
		if ( *szValue )
			pItemDef->AccessoryLimit = strtol( szValue, NULL, 10 );
		Assert( pItemDef->AccessoryLimit <= RD_ITEM_MAX_ACCESSORIES );
		FETCH_PROPERTY( "compressed_dynamic_props" );
		if ( *szValue )
		{
			CSplitString CompressedDynamicProps{ szValue, ";" };
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

		for ( int i = 0; ; i++ )
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

		FETCH_PROPERTY( "after_description_only_multi_stack" );
		Assert( !V_strcmp( szValue, "" ) || !V_strcmp( szValue, "1" ) || !V_strcmp( szValue, "0" ) );
		pItemDef->AfterDescriptionOnlyMultiStack = !V_strcmp( szValue, "1" );

#ifdef CLIENT_DLL
		pItemDef->Icon = NULL;
		FETCH_PROPERTY( "icon_url" );
		if ( *szValue )
		{
			pItemDef->Icon = CSteamItemIcon::Get( szValue );
		}

		pItemDef->IconSmall = pItemDef->Icon;
		FETCH_PROPERTY( "icon_url_small" );
		if ( *szValue )
		{
			pItemDef->IconSmall = CSteamItemIcon::Get( szValue );
		}

		pItemDef->StyleIcons.SetCount( pItemDef->StyleNames.Count() );
		for ( int i = 0; i < pItemDef->StyleNames.Count(); i++ )
		{
			V_snprintf( szKey, sizeof( szKey ), "icon_url_style_%d", i );
			pItemDef->StyleIcons[i] = pItemDef->Icon;
			FETCH_PROPERTY( szKey );
			if ( *szValue )
			{
				pItemDef->StyleIcons[i] = CSteamItemIcon::Get( szValue );
			}
		}

		pItemDef->AccessoryIcon = NULL;
		FETCH_PROPERTY( "accessory_icon" );
		if ( *szValue )
		{
			pItemDef->AccessoryIcon = materials->FindTexture( szValue, TEXTURE_GROUP_CLIENT_EFFECTS );
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
			DevWarning( "ISteamInventory::DeserializeResult failed to create a result.\n" );
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
			DevWarning( "ReactiveDropInventory::ValidateItemData: EResult %d (%s)\n", eResultStatus, UTIL_RD_EResultToString( eResultStatus ) );
			
			bValid = false;
			return true;
		}

		if ( requiredSteamID.IsValid() && !pInventory->CheckResultSteamID( hResult, requiredSteamID ) )
		{
			DevWarning( "ReactiveDropInventory::ValidateItemData: not from SteamID %llu\n", requiredSteamID.ConvertToUint64() );

			bValid = false;
			return true;
		}

		if ( szRequiredSlot )
		{
			ReactiveDropInventory::ItemInstance_t instance{ hResult, 0 };
			const ReactiveDropInventory::ItemDef_t *pItemDef = GetItemDef( instance.ItemDefID );
			if ( !pItemDef || !pItemDef->ItemSlotMatches( szRequiredSlot ) )
			{
				DevWarning( "ReactiveDropInventory::ValidateItemData: item fits in slot '%s', not '%s'\n", pItemDef ? pItemDef->ItemSlot.Get() : "<NO DEF>", szRequiredSlot );

				bValid = false;
				return true;
			}
		}

		bValid = true;
		return true;
	}

	bool ValidateEquipItemData( bool &bValid, SteamInventoryResult_t hResult, byte( &nIndex )[RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS], CSteamID requiredSteamID )
	{
		GET_INVENTORY_OR_BAIL( false );

		EResult eResultStatus = pInventory->GetResultStatus( hResult );
		if ( eResultStatus == k_EResultPending )
		{
			return false;
		}

		if ( eResultStatus != k_EResultOK )
		{
			DevWarning( "ReactiveDropInventory::ValidateEquipItemData: EResult %d (%s)\n", eResultStatus, UTIL_RD_EResultToString( eResultStatus ) );

			bValid = false;
			return true;
		}

		Assert( requiredSteamID.IsValid() );
		if ( !pInventory->CheckResultSteamID( hResult, requiredSteamID ) )
		{
			DevWarning( "ReactiveDropInventory::ValidateEquipItemData: not from SteamID %llu\n", requiredSteamID.ConvertToUint64() );

			bValid = false;
			return true;
		}

		CUtlVector<SteamItemInstanceID_t> seenIDs;
		seenIDs.EnsureCapacity( RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS );

		for ( int i = 0; i < RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS; i++ )
		{
			if ( nIndex[i] == 0 )
				continue;

			ReactiveDropInventory::ItemInstance_t instance{ hResult, nIndex[i] - 1u };
			const ReactiveDropInventory::ItemDef_t *pItemDef = GetItemDef( instance.ItemDefID );

			if ( !pItemDef || !pItemDef->ItemSlotMatches( ReactiveDropInventory::g_InventorySlotNames[i] ) )
			{
				DevWarning( "ReactiveDropInventory::ValidateEquipItemData: item %llu '%s' fits in slot '%s', not '%s'\n", instance.ItemID, pItemDef ? pItemDef->Name.Get() : "<NO DEF>", pItemDef ? pItemDef->ItemSlot.Get() : "<NO DEF>", ReactiveDropInventory::g_InventorySlotNames[i] );

				bValid = false;
				return true;
			}

			if ( seenIDs.IsValidIndex( seenIDs.Find( instance.ItemID ) ) )
			{
				DevWarning( "ReactiveDropInventory::ValidateEquipItemData: item %llu '%s' is in multiple slots (latest '%s')\n", instance.ItemID, pItemDef ? pItemDef->Name.Get() : "<NO DEF>", ReactiveDropInventory::g_InventorySlotNames[i] );

				bValid = false;
				return true;
			}
			seenIDs.AddToTail( instance.ItemID );
		}

		bValid = true;
		return true;
	}

#ifdef CLIENT_DLL
	void AddPromoItem( SteamItemDef_t id )
	{
		if ( s_RD_Inventory_Manager.m_PromotionalItemsResult != k_SteamInventoryResultInvalid )
		{
			DevMsg( "Not requesting promo item %d: request already in-flight.\n", id );
			s_RD_Inventory_Manager.m_PromotionalItemsNext.AddToTail( id );
			return;
		}

		GET_INVENTORY_OR_BAIL;

		pInventory->AddPromoItem( &s_RD_Inventory_Manager.m_PromotionalItemsResult, id );
	}

	void RequestGenericPromoItems()
	{
		if ( s_RD_Inventory_Manager.m_PromotionalItemsResult != k_SteamInventoryResultInvalid )
		{
			DevMsg( "Not requesting generic promo items: request already in-flight.\n" );
			s_RD_Inventory_Manager.m_bRequestGenericPromotionalItemsAgain = true;
			return;
		}

		GET_INVENTORY_OR_BAIL;

		pInventory->GrantPromoItems( &s_RD_Inventory_Manager.m_PromotionalItemsResult );
	}

#ifdef RD_7A_DROPS
	static const char *const s_RDWorkshopCompetitionTags[] =
	{
		"BossFight2023",
		// (future themes subject to change)
		//"SpaceStation2024",
		//"Endless2025",
		//"Biomes2026",
		//"Deathmatch2027",
		//"Factory2028",
		//"Backtracking2029",
		//"Transportation2030",
		//"Defense2031",
		//"VirtualReality2032",
		//"AlternatePath2033",
		//"Coast2034",
		//"Scavenger2035",
		//"Mining2036",
		//"Infiltration2037",
		//"City2038",
	};

	static const struct
	{
		const char *szMissionName;
		SteamItemDef_t iGenerator;
	} s_RDOfficialMissionGenerators[] =
	{
		{ "rd-bonus_mission1", 7006 },
		{ "rd-bonus_mission2", 7006 },
		{ "rd-bonus_mission3", 7006 },
		{ "asi-jac1-landingbay_01", 7009 },
		{ "asi-jac1-landingbay_02", 7009 },
		{ "asi-jac1-landingbay_pract", 7009 },
		{ "asi-jac2-deima", 7009 },
		{ "asi-jac3-rydberg", 7009 },
		{ "asi-jac4-residential", 7009 },
		{ "asi-jac6-sewerjunction", 7009 },
		{ "asi-jac7-timorstation", 7009 },
		{ "rd-bonus_mission4", 7009 },
		{ "rd-ocs1storagefacility", 7011 },
		{ "rd-ocs2landingbay7", 7011 },
		{ "rd-ocs3uscmedusa", 7011 },
		{ "rd-res1forestentrance", 7012 },
		{ "rd-res2research7", 7012 },
		{ "rd-res3miningcamp", 7012 },
		{ "rd-res4mines", 7012 },
		{ "rd-area9800lz", 7010 },
		{ "rd-area9800pp1", 7010 },
		{ "rd-area9800pp2", 7010 },
		{ "rd-area9800wl", 7010 },
		{ "rd-bonus_mission7", 7010 },
		{ "rd-tft1desertoutpost", 7013 },
		{ "rd-tft2abandonedmaintenance", 7013 },
		{ "rd-tft3spaceport", 7013 },
		{ "rd-til1midnightport", 7014 },
		{ "rd-til2roadtodawn", 7014 },
		{ "rd-til3arcticinfiltration", 7014 },
		{ "rd-til4area9800", 7014 },
		{ "rd-til5coldcatwalks", 7014 },
		{ "rd-til6yanaurusmine", 7014 },
		{ "rd-til7factory", 7014 },
		{ "rd-til8comcenter", 7014 },
		{ "rd-til9syntekhospital", 7014 },
		{ "rd-lan1_bridge", 7015 },
		{ "rd-lan2_sewer", 7015 },
		{ "rd-lan3_maintenance", 7015 },
		{ "rd-lan4_vent", 7015 },
		{ "rd-lan5_complex", 7015 },
		{ "rd-par1unexpected_encounter", 7016 },
		{ "rd-par2hostile_places", 7016 },
		{ "rd-par3close_contact", 7016 },
		{ "rd-par4high_tension", 7016 },
		{ "rd-par5crucial_point", 7016 },
		{ "rd-bonus_mission5", 7016 },
		{ "rd-bonus_mission6", 7016 },
		{ "rd-nh01_logisticsarea", 7017 },
		{ "rd-nh02_platformxvii", 7017 },
		{ "rd-nh03_groundworklabs", 7017 },
		{ "rd-bio1operationx5", 7018 },
		{ "rd-bio2invisiblethreat", 7018 },
		{ "rd-bio3biogenlabs", 7018 },
		{ "rd-acc1_infodep", 7019 },
		{ "rd-acc2_powerhood", 7019 },
		{ "rd-acc3_rescenter", 7019 },
		{ "rd-acc4_confacility", 7019 },
		{ "rd-acc5_j5connector", 7019 },
		{ "rd-acc6_labruins", 7019 },
		{ "rd-acc_complex", 7019 },
		{ "rd-ada_sector_a9", 7020 },
		{ "rd-ada_nexus_subnode", 7020 },
		{ "rd-ada_neon_carnage", 7020 },
		{ "rd-ada_fuel_junction", 7020 },
		{ "rd-ada_dark_path", 7020 },
		{ "rd-ada_forbidden_outpost", 7020 },
		{ "rd-ada_new_beginning", 7020 },
		{ "rd-ada_anomaly", 7020 },
	};
#endif

	void CheckPlaytimeItemGenerators( int iMarineClass )
	{
#ifdef RD_7A_DROPS
		for ( int i = 0; i < NELEMS( s_RD_Inventory_Manager.m_PlaytimeItemGeneratorResult ); i++ )
		{
			if ( s_RD_Inventory_Manager.m_PlaytimeItemGeneratorResult[i] != k_SteamInventoryResultInvalid )
			{
				DevMsg( "Not checking playtime item generators: request already in-flight.\n" );
				return;
			}
		}

		GET_INVENTORY_OR_BAIL;

		SteamItemDef_t iItemGenerator = 7000;

		const RD_Mission_t *pMission = ReactiveDropMissions::GetMission( engine->GetLevelNameShort() );
		COMPILE_TIME_ASSERT( NUM_MARINE_CLASSES == 4 );
		if ( iMarineClass != MARINE_CLASS_UNDEFINED && iMarineClass < NUM_MARINE_CLASSES && RandomInt( 0, 1 ) )
		{
			iItemGenerator = 7025 + iMarineClass;
		}
		else if ( ASWDeathmatchMode() )
		{
			iItemGenerator = 7008;
		}
		else if ( pMission && pMission->HasTag( "endless" ) )
		{
			iItemGenerator = 7007;
		}
		else if ( CAlienSwarm *pAlienSwarm = ASWGameRules() )
		{
			if ( pAlienSwarm->m_iMissionWorkshopID != k_PublishedFileIdInvalid )
			{
				const CReactiveDropWorkshop::WorkshopItem_t &item = g_ReactiveDropWorkshop.TryQueryAddon( pAlienSwarm->m_iMissionWorkshopID );
				CSplitString tags( item.details.m_rgchTags, "," );
				bool bIsCompetitionAddon = false;
				FOR_EACH_VEC( tags, i )
				{
					for ( int j = 0; j < NELEMS( s_RDWorkshopCompetitionTags ); j++ )
					{
						if ( FStrEq( tags[i], s_RDWorkshopCompetitionTags[j] ) )
						{
							bIsCompetitionAddon = true;
							break;
						}
					}

					if ( bIsCompetitionAddon )
					{
						break;
					}
				}

				if ( bIsCompetitionAddon )
				{
					iItemGenerator = 7001;
				}
				else if ( pAlienSwarm->IsCampaignGame() )
				{
					iItemGenerator = 7002 + ( pAlienSwarm->m_iMissionWorkshopID & 1 );
				}
				else
				{
					iItemGenerator = 7004 + ( pAlienSwarm->m_iMissionWorkshopID & 1 );
				}
			}
			else if ( pMission )
			{
				for ( int i = 0; i < NELEMS( s_RDOfficialMissionGenerators ); i++ )
				{
					if ( FStrEq( pMission->BaseName, s_RDOfficialMissionGenerators[i].szMissionName ) )
					{
						iItemGenerator = s_RDOfficialMissionGenerators[i].iGenerator;
						break;
					}
				}
			}
		}

		pInventory->TriggerItemDrop( &s_RD_Inventory_Manager.m_PlaytimeItemGeneratorResult[0], iItemGenerator );
		pInventory->TriggerItemDrop( &s_RD_Inventory_Manager.m_PlaytimeItemGeneratorResult[1], 7021 );
		pInventory->TriggerItemDrop( &s_RD_Inventory_Manager.m_PlaytimeItemGeneratorResult[2], 7029 );
#endif
	}

	void CommitDynamicProperties()
	{
		s_RD_Inventory_Manager.CommitDynamicProperties();
	}
#endif

	void OnHitConfirm( CBaseEntity *pAttacker, CBaseEntity *pTarget, Vector vecDamagePosition, bool bKilled, bool bDamageOverTime, bool bBlastDamage, int iDisposition, float flDamage, CBaseEntity *pWeapon )
	{
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
				return;

			CASW_Inhabitable_NPC *pInhabitableAttacker = assert_cast< CASW_Inhabitable_NPC * >( pAttacker );
			if ( pTarget && pTarget->IsAlien() && bKilled )
			{
				s_RD_Inventory_Manager.IncrementStrangePropertiesForWeapon( pInhabitableAttacker, pWeapon, 5002, 1 ); // Aliens Killed
				s_RD_Inventory_Manager.IncrementStrangePropertiesForWeapon( pInhabitableAttacker, pWeapon, 5007, 1 ); // Alien Kill Streak
			}
		}
	}

#undef GET_INVENTORY_OR_BAIL
}

#define RD_ITEM_ID_BITS 30
#define RD_ITEM_ACCESSORY_BITS 13

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
	SendPropArray( SendPropInt( SENDINFO_ARRAY( m_nCounter ), 63, SPROP_UNSIGNED ), m_nCounter ),
#endif
END_NETWORK_TABLE()

CRD_ItemInstance::CRD_ItemInstance()
{
	Reset();
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
	m_iItemInstanceID = k_SteamItemInstanceIDInvalid;
	m_iItemDefID = 0;
	for ( int i = 0; i < m_iAccessory.Count(); i++ )
	{
		m_iAccessory.Set( i, 0 );
	}
	for ( int i = 0; i < m_nCounter.Count(); i++ )
	{
		m_nCounter.Set( i, 0 );
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
		Assert( !DynamicPropertyAllowsArbitraryValues( pDef->CompressedDynamicProps[i] ) );

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
				Assert( !DynamicPropertyAllowsArbitraryValues( pAccessoryDef->CompressedDynamicProps[j] ) );
				if ( instance.DynamicProps.Defined( pAccessoryDef->CompressedDynamicProps[j] ) )
				{
					const char *szPropValue = instance.DynamicProps[instance.DynamicProps.Find( pAccessoryDef->CompressedDynamicProps[j] )];
					m_nCounter.Set( i, strtoll( szPropValue, NULL, 10 ) );
				}
				iCounterIndex++;
			}
		}
	}
}

void CRD_ItemInstance::FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc ) const
{
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_iItemDefID );

	V_UTF8ToUnicode( szDesc, wszBuf, sizeOfBufferInBytes );

	char szToken[128]{};
	wchar_t wszReplacement[1024]{};

	for ( size_t i = 0; i < sizeOfBufferInBytes / sizeof( wchar_t ); i++ )
	{
		if ( wszBuf[i] == L'\0' )
		{
			return;
		}

		if ( wszBuf[i] != L'%' )
		{
			continue;
		}

		V_wcsncpy( wszReplacement, L"MISSING", sizeof( wszReplacement ) );

		size_t tokenLength = 1;
		while ( wszBuf[i + tokenLength] != L'%' && wszBuf[i + tokenLength] != L':' )
		{
			if ( wszBuf[i + tokenLength] == L'\0' )
			{
				return;
			}

			Assert( wszBuf[i + tokenLength] < 0x80 ); // assume ASCII
			szToken[tokenLength - 1] = ( char )wszBuf[i + tokenLength];

			tokenLength++;

			Assert( tokenLength < sizeof( szToken ) );
		}

		szToken[tokenLength - 1] = '\0';

		Assert( wszBuf[i + tokenLength] != L':' );
		if ( wszBuf[i + tokenLength] == L':' )
		{
			size_t tokenReplacementLength = 0;
			tokenLength++;

			while ( wszBuf[i + tokenLength] != L'%' )
			{
				if ( wszBuf[i + tokenLength] == L'\0' )
				{
					return;
				}

				szToken[tokenReplacementLength] = wszBuf[i + tokenLength];

				tokenReplacementLength++;
				tokenLength++;

				Assert( tokenReplacementLength < NELEMS( wszReplacement ) );
			}

			wszReplacement[tokenReplacementLength] = L'\0';
		}

		tokenLength++;

		if ( tokenLength == 2 )
		{
			// special case: %% is just %
			V_memmove( &wszBuf[i + 1], &wszBuf[i + 2], sizeOfBufferInBytes - ( i + 2 ) * sizeof( wchar_t ) );
			i++;
			continue;
		}

		FOR_EACH_VEC( pDef->CompressedDynamicProps, j )
		{
			if ( !V_strcmp( pDef->CompressedDynamicProps[j], szToken ) )
			{
				if ( !V_strcmp( szToken, "style" ) && m_nCounter[j] >= 0 && m_nCounter[j] < pDef->StyleNames.Count() )
				{
					V_UTF8ToUnicode( pDef->StyleNames[m_nCounter[j]], wszReplacement, sizeof( wszReplacement ) );
				}
				else
				{
					V_wcsncpy( wszReplacement, UTIL_RD_CommaNumber( m_nCounter[j] ), sizeof( wszReplacement ) );
				}
				break;
			}
		}

		if ( !pDef->AccessoryTag.IsEmpty() )
		{
			int iCounterIndex = pDef->CompressedDynamicProps.Count();
			for ( int j = 0; j < RD_ITEM_MAX_ACCESSORIES; j++ )
			{
				if ( m_iAccessory[j] == 0 )
					continue;

				const ReactiveDropInventory::ItemDef_t *pAccessoryDef = ReactiveDropInventory::GetItemDef( m_iAccessory[j] );

				FOR_EACH_VEC( pAccessoryDef->CompressedDynamicProps, k )
				{
					if ( !V_strcmp( pAccessoryDef->CompressedDynamicProps[k], szToken ) )
					{
						V_wcsncpy( wszReplacement, UTIL_RD_CommaNumber( m_nCounter[iCounterIndex] ), sizeof( wszReplacement ) );
						break;
					}

					iCounterIndex++;
				}
			}
		}

		size_t replacementLength = 0;
		while ( wszReplacement[replacementLength] )
		{
			replacementLength++;
		}

		if ( i + replacementLength >= sizeOfBufferInBytes / sizeof( wchar_t ) )
		{
			replacementLength = ( sizeOfBufferInBytes - i - 1 ) / sizeof( wchar_t );
		}

		V_memmove( &wszBuf[i + replacementLength], &wszBuf[i + tokenLength], sizeOfBufferInBytes - ( i + MAX( replacementLength, tokenLength ) ) * sizeof( wchar_t ) );
		V_memmove( &wszBuf[i], wszReplacement, replacementLength * sizeof( wchar_t ) );
		if ( replacementLength > tokenLength )
		{
			wszBuf[sizeOfBufferInBytes / sizeof( wchar_t ) - 1] = L'\0';
		}
	}
}

#ifdef CLIENT_DLL
ConVar rd_briefing_item_details_color1( "rd_briefing_item_details_color1", "169 213 255 255", FCVAR_NONE );
ConVar rd_briefing_item_details_color2( "rd_briefing_item_details_color2", "83 148 192 255", FCVAR_NONE );
ConVar rd_briefing_item_accessory_color( "rd_briefing_item_accessory_color", "191 191 191 255", FCVAR_NONE );

void CRD_ItemInstance::AppendBBCode( vgui::RichText *pRichText, const wchar_t *wszBuf, Color defaultColor )
{
	CUtlVector<Color> colorStack;
	colorStack.AddToTail( defaultColor );
	pRichText->InsertColorChange( defaultColor );

	for ( const wchar_t *pBuf = wszBuf; *pBuf; pBuf++ )
	{
		if ( *pBuf == L'[' )
		{
			if ( pBuf[1] == L'c' && pBuf[2] == L'o' && pBuf[3] == L'l' && pBuf[4] == L'o' && pBuf[5] == L'r' && pBuf[6] == L'=' && pBuf[7] == L'#' &&
				pBuf[8] && pBuf[9] && pBuf[10] && pBuf[11] && pBuf[12] && pBuf[13] && pBuf[14] == L']' )
			{
				char szHex[6]
				{
					( char )pBuf[8],
					( char )pBuf[9],
					( char )pBuf[10],
					( char )pBuf[11],
					( char )pBuf[12],
					( char )pBuf[13],
				};
				byte szBin[3]{};
				V_hextobinary( szHex, 6, szBin, sizeof( szBin ) );

				Color nextColor{ szBin[0], szBin[1], szBin[2], 255 };
				colorStack.AddToTail( nextColor );
				pRichText->InsertColorChange( nextColor );

				pBuf += 15;
				continue;
			}

			if ( pBuf[1] == L'/' && pBuf[2] == L'c' && pBuf[3] == L'o' && pBuf[4] == L'l' && pBuf[5] == L'o' && pBuf[6] == L'r' && pBuf[7] == L']' )
			{
				Assert( colorStack.Count() > 1 );

				colorStack.RemoveMultipleFromTail( 1 );
				pRichText->InsertColorChange( colorStack.Tail() );

				pBuf += 8;

				continue;
			}
		}

		pRichText->InsertChar( *pBuf );
	}

	Assert( colorStack.Count() == 1 );
}

void CRD_ItemInstance::FormatDescription( vgui::RichText *pRichText ) const
{
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_iItemDefID );
	wchar_t wszBuf[2048];

	pRichText->SetText( "" );

	FormatDescription( wszBuf, sizeof( wszBuf ), pDef->BeforeDescription );
	if ( wszBuf[0] )
	{
		AppendBBCode( pRichText, wszBuf, rd_briefing_item_details_color2.GetColor() );
		pRichText->InsertString( L"\n\n" );
	}

	bool bAnyAccessories = false;
	FormatDescription( wszBuf, sizeof( wszBuf ), pDef->AccessoryDescription );
	if ( wszBuf[0] )
	{
		AppendBBCode( pRichText, wszBuf, rd_briefing_item_accessory_color.GetColor() );
		pRichText->InsertString( L"\n" );
		bAnyAccessories = true;
	}
	for ( int i = 0; i < RD_ITEM_MAX_ACCESSORIES; i++ )
	{
		if ( m_iAccessory[i] == 0 )
			continue;

		bAnyAccessories = true;
		FormatDescription( wszBuf, sizeof( wszBuf ), ReactiveDropInventory::GetItemDef( m_iAccessory[i] )->AccessoryDescription );
		if ( wszBuf[0] )
		{
			AppendBBCode( pRichText, wszBuf, rd_briefing_item_accessory_color.GetColor() );
			pRichText->InsertString( L"\n" );
		}
	}

	if ( bAnyAccessories )
		pRichText->InsertString( L"\n" );

	if ( pDef->HasInGameDescription )
		FormatDescription( wszBuf, sizeof( wszBuf ), pDef->Description );
	else
		V_UTF8ToUnicode( pDef->Description, wszBuf, sizeof( wszBuf ) );
	AppendBBCode( pRichText, wszBuf, rd_briefing_item_details_color1.GetColor() );

	bool bShowAfterDescription = !pDef->AfterDescriptionOnlyMultiStack;
	if ( !bShowAfterDescription )
	{
		bool bFound = false;
		FOR_EACH_VEC( pDef->CompressedDynamicProps, i )
		{
			if ( !V_stricmp( pDef->CompressedDynamicProps[i], "m_unQuantity" ) )
			{
				bFound = true;
				bShowAfterDescription = m_nCounter[i] > 1;
				break;
			}
		}
		( void )bFound;
		Assert( bFound );
	}
	if ( bShowAfterDescription )
	{
		FormatDescription( wszBuf, sizeof( wszBuf ), pDef->AfterDescription );
		if ( wszBuf[0] )
		{
			pRichText->InsertString( L"\n\n" );
			AppendBBCode( pRichText, wszBuf, rd_briefing_item_details_color2.GetColor() );
		}
	}
}
#endif

#ifndef CLIENT_DLL
static void *SendProxy_ProjectileItemDataForOwningPlayer( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	static CRD_ItemInstance s_BlankInstance;

	const CRD_ProjectileData *pProjectile = static_cast< const CRD_ProjectileData * >( pStructBase );
	CASW_Player *pPlayer = pProjectile->m_hOriginalOwnerPlayer;
	if ( !pPlayer || pProjectile->m_iInventoryEquipSlotIndex == -1 )
		return &s_BlankInstance;

	return &pPlayer->m_EquippedItemData[pProjectile->m_iInventoryEquipSlotIndex];
}
#endif

BEGIN_NETWORK_TABLE_NOBASE( CRD_ProjectileData, DT_RD_ProjectileData )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iOriginalOwnerSteamAccount ) ),
	RecvPropDataTable( RECVINFO_DT( m_InventoryItemData ), 0, &REFERENCE_RECV_TABLE( DT_RD_ItemInstance ) ),
	RecvPropBool( RECVINFO( m_bFiredByOwner ) ),
#else
	SendPropInt( SENDINFO( m_iOriginalOwnerSteamAccount ), -1, SPROP_UNSIGNED ),
	SendPropDataTable( "m_InventoryItemData", 0, &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ), SendProxy_ProjectileItemDataForOwningPlayer ),
	SendPropBool( SENDINFO( m_bFiredByOwner ) ),
#endif
END_NETWORK_TABLE()

CRD_ProjectileData::CRD_ProjectileData()
{
	m_iOriginalOwnerSteamAccount = 0;
#ifdef CLIENT_DLL
	m_InventoryItemData.Reset();
#else
	m_hOriginalOwnerPlayer = NULL;
	m_iInventoryEquipSlotIndex = -1;
#endif
	m_bFiredByOwner = false;
}

#ifdef GAME_DLL
void CRD_ProjectileData::SetFromWeapon( CBaseEntity *pCreator )
{
	if ( CASW_Weapon *pWeapon = dynamic_cast< CASW_Weapon * >( pCreator ) )
	{
		m_iOriginalOwnerSteamAccount = pWeapon->m_iOriginalOwnerSteamAccount;
		m_hOriginalOwnerPlayer = pWeapon->m_hOriginalOwnerPlayer;
		m_iInventoryEquipSlotIndex = pWeapon->m_iInventoryEquipSlotIndex;
		if ( pWeapon->GetOwner() && pWeapon->GetOwner()->IsInhabitableNPC() )
		{
			CASW_Inhabitable_NPC *pOwnerNPC = assert_cast< CASW_Inhabitable_NPC * >( pWeapon->GetOwner() );
			m_bFiredByOwner = pOwnerNPC->IsInhabited() && pOwnerNPC->GetCommander() == pWeapon->m_hOriginalOwnerPlayer;
		}
		return;
	}

	if ( CASW_Sentry_Top *pSentry = dynamic_cast< CASW_Sentry_Top * >( pCreator ) )
	{
		pCreator = pSentry->GetSentryBase();
	}

	if ( CASW_Sentry_Base *pSentry = dynamic_cast< CASW_Sentry_Base * >( pCreator ) )
	{
		m_iOriginalOwnerSteamAccount = pSentry->m_iOriginalOwnerSteamAccount;
		m_hOriginalOwnerPlayer = pSentry->m_hOriginalOwnerPlayer;
		m_iInventoryEquipSlotIndex = pSentry->m_iInventoryEquipSlotIndex;
		m_bFiredByOwner = true;
		return;
	}
}
#endif

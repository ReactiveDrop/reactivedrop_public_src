#include "cbase.h"
#include "rd_inventory_shared.h"
#include "rd_lobby_utils.h"
#include "asw_util_shared.h"
#include "jsmn.h"
#include <ctime>

#ifdef CLIENT_DLL
#include <vgui/IImage.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include <vgui_controls/RichText.h>
#include "lodepng.h"
#include "asw_gamerules.h"
#include "rd_workshop.h"
#include "rd_missions_shared.h"
#include "asw_deathmatch_mode_light.h"
#include "gameui/swarm/vitemshowcase.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef GAME_DLL
extern ConVar rd_dedicated_server_language;
#endif

static CUtlMap<SteamItemDef_t, ReactiveDropInventory::ItemDef_t *> s_ItemDefs( DefLessFunc( SteamItemDef_t ) );

static class CRD_Inventory_Manager : public CAutoGameSystem
{
public:
	CRD_Inventory_Manager() : CAutoGameSystem( "CRD_Inventory_Manager" )
	{
	}

	virtual ~CRD_Inventory_Manager()
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
#ifdef CLIENT_DLL
			pInventory->DestroyResult( m_EquippedMedalResult );
			pInventory->DestroyResult( m_PromotionalItemsResult );
			for ( int i = 0; i < NELEMS( m_PlaytimeItemGeneratorResult ); i++ )
			{
				pInventory->DestroyResult( m_PlaytimeItemGeneratorResult[i] );
			}
			pInventory->DestroyResult( m_InspectItemResult );
#endif
			pInventory->DestroyResult( m_DebugPrintInventoryResult );
		}
	}

	virtual void PostInit()
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
	}

	virtual void LevelInitPreEntity()
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
	}

#ifdef CLIENT_DLL
	void OnEquippedMedalIDChanged()
	{
		extern ConVar rd_equipped_medal;

		ISteamInventory *pInventory = SteamInventory();

		SteamItemInstanceID_t id = strtoull( rd_equipped_medal.GetString(), NULL, 10 );

		if ( pInventory )
		{
			pInventory->DestroyResult( m_EquippedMedalResult );
		}

		m_EquippedMedalResult = k_SteamInventoryResultInvalid;

		if ( id == 0 || id == k_SteamItemInstanceIDInvalid )
		{
			SendMedalBlob( UTIL_RD_GetCurrentLobbyID() );
			return;
		}

		if ( !pInventory )
		{
			Warning( "ISteamInventory unavailable! Unable to retrieve equipped medal.\n" );
			SendMedalBlob( UTIL_RD_GetCurrentLobbyID() );
			return;
		}

		pInventory->GetItemsByID( &m_EquippedMedalResult, &id, 1 );
	}

	void SendMedalBlob( CSteamID currentLobby )
	{
		ISteamUser *pUser = SteamUser();
		ISteamInventory *pInventory = SteamInventory();
		ISteamMatchmaking *pMatchmaking = SteamMatchmaking();
		if ( !pUser || !pInventory || !pMatchmaking || !currentLobby.IsValid() )
		{
			// no lobby to send to
			return;
		}

		char szEncoded[2048]{};
		if ( m_EquippedMedalResult != k_SteamInventoryResultInvalid )
		{
			if ( pInventory->GetResultStatus( m_EquippedMedalResult ) == k_EResultPending )
			{
				DevMsg( "Not sending equipped medal to lobby - inventory request in pending state!\n" );
				return;
			}

			uint32_t count;
			if ( !pInventory->GetResultItems( m_EquippedMedalResult, NULL, &count ) || !count )
			{
				DevWarning( "No equipped medal for ID (%s)\n", UTIL_RD_EResultToString( pInventory->GetResultStatus( m_EquippedMedalResult ) ) );
			}
			else
			{
				byte rawEncoded[1024];
				count = NELEMS( rawEncoded );

				if ( !pInventory->SerializeResult( m_EquippedMedalResult, rawEncoded, &count ) )
				{
					Warning( "Failed to serialize equipped medal!\n" );
				}
				else
				{
					V_binarytohex( rawEncoded, count, szEncoded, sizeof( szEncoded ) );
				}
			}
		}

		pMatchmaking->SetLobbyMemberData( currentLobby, "rd_equipped_medal", szEncoded );
		const char *sz = pMatchmaking->GetLobbyMemberData( currentLobby, pUser->GetSteamID(), "rd_equipped_medal:updates" );
		int nUpdateCount = sz ? atoi( sz ) : 0;
		pMatchmaking->SetLobbyMemberData( currentLobby, "rd_equipped_medal:updates", VarArgs( "%d", nUpdateCount + 1 ) );
	}

	void HandleItemDropResult( SteamInventoryResult_t &hResult )
	{
		BaseModUI::ItemShowcase::ShowItems( hResult, 0, -1, BaseModUI::ItemShowcase::MODE_ITEM_DROP );

		SteamInventory()->DestroyResult( hResult );
		hResult = k_SteamInventoryResultInvalid;
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
		{
			Msg( "Result %08x (%s, age %d sec) has %d items:\n", hResult, UTIL_RD_EResultToString( pInventory->GetResultStatus( hResult ) ), SteamUtils()->GetServerRealTime() - pInventory->GetResultTimestamp( hResult ), count );

			CUtlMemory<SteamItemDetails_t> itemDetails( 0, count );
			if ( !pInventory->GetResultItems( hResult, itemDetails.Base(), &count ) )
			{
				Warning( "Failed to get item details for result.\n" );
				count = 0;
			}

			FOR_EACH_VEC( itemDetails, i )
			{
				Msg( "Item %llu (def %d qty %d flags %x)\n", itemDetails[i].m_itemId, itemDetails[i].m_iDefinition, itemDetails[i].m_unQuantity, itemDetails[i].m_unFlags );

				uint32_t size{};
				CUtlMemory<char> szStringBuf( 0, 1024 );
				szStringBuf[0] = '\0';

				{
					pInventory->GetItemDefinitionProperty( itemDetails[i].m_iDefinition, NULL, NULL, &size );
					szStringBuf.EnsureCapacity( size + 1 );
					size = szStringBuf.Count();
					pInventory->GetItemDefinitionProperty( itemDetails[i].m_iDefinition, NULL, szStringBuf.Base(), &size );
					Msg( "Properties: %s\n", szStringBuf.Base() );
					CSplitString propertyNames( szStringBuf.Base(), "," );
					FOR_EACH_VEC( propertyNames, j )
					{
						pInventory->GetItemDefinitionProperty( itemDetails[i].m_iDefinition, propertyNames[j], NULL, &size );
						szStringBuf.EnsureCapacity( size + 1 );
						size = szStringBuf.Count();
						pInventory->GetItemDefinitionProperty( itemDetails[i].m_iDefinition, propertyNames[j], szStringBuf.Base(), &size );
						Msg( "Properties[%s] = %s\n", propertyNames[j], szStringBuf.Base() );
					}
				}

				{
					pInventory->GetResultItemProperty( hResult, i, NULL, NULL, &size );
					szStringBuf.EnsureCapacity( size + 1 );
					size = szStringBuf.Count();
					pInventory->GetResultItemProperty( hResult, i, NULL, szStringBuf.Base(), &size );
					Msg( "DynamicProperties: %s\n", szStringBuf.Base() );
					CSplitString propertyNames( szStringBuf.Base(), "," );
					FOR_EACH_VEC( propertyNames, j )
					{
						pInventory->GetResultItemProperty( hResult, i, propertyNames[j], NULL, &size );
						szStringBuf.EnsureCapacity( size + 1 );
						size = szStringBuf.Count();
						pInventory->GetResultItemProperty( hResult, i, propertyNames[j], szStringBuf.Base(), &size );
						Msg( "DynamicProperties[%s] = %s\n", propertyNames[j], szStringBuf.Base() );
					}
				}

				Msg( "\n" );
			}
		}
	}

	STEAM_CALLBACK( CRD_Inventory_Manager, OnSteamInventoryResultReady, SteamInventoryResultReady_t )
	{
		DevMsg( 2, "Steam Inventory result for %08x received: EResult %d (%s)\n", pParam->m_handle, pParam->m_result, UTIL_RD_EResultToString( pParam->m_result ) );

#ifdef CLIENT_DLL
		if ( pParam->m_handle == m_EquippedMedalResult )
		{
			// Pre-load our own equipped medal icon even if we're not in a lobby.
			ReactiveDropInventory::ItemInstance_t MedalInstance{ m_EquippedMedalResult, 0 };
			( void )ReactiveDropInventory::GetItemDef( MedalInstance.ItemDefID );

			SendMedalBlob( UTIL_RD_GetCurrentLobbyID() );

			return;
		}
		
		if ( pParam->m_handle == m_PromotionalItemsResult )
		{
			HandleItemDropResult( m_PromotionalItemsResult );
			if ( m_PromotionalItemsNext.Count() )
			{
				SteamInventory()->AddPromoItems( &m_PromotionalItemsResult, m_PromotionalItemsNext.Base(), m_PromotionalItemsNext.Count() );
				m_PromotionalItemsNext.Purge();
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

		if ( pParam->m_handle == m_InspectItemResult )
		{
			BaseModUI::ItemShowcase::ShowItems( pParam->m_handle, 0, -1, BaseModUI::ItemShowcase::MODE_INSPECT );

			SteamInventory()->DestroyResult( m_InspectItemResult );
			m_InspectItemResult = k_SteamInventoryResultInvalid;

			return;
		}
#endif

		if ( pParam->m_handle == m_DebugPrintInventoryResult )
		{
			DebugPrintResult( m_DebugPrintInventoryResult );
			SteamInventory()->DestroyResult( m_DebugPrintInventoryResult );
			m_DebugPrintInventoryResult = k_SteamInventoryResultInvalid;

			return;
		}
	}

#ifdef CLIENT_DLL
	SteamInventoryResult_t m_EquippedMedalResult{ k_SteamInventoryResultInvalid };
	SteamInventoryResult_t m_PromotionalItemsResult{ k_SteamInventoryResultInvalid };
	CUtlVector<SteamItemDef_t> m_PromotionalItemsNext{};
	SteamInventoryResult_t m_PlaytimeItemGeneratorResult[3]{ k_SteamInventoryResultInvalid, k_SteamInventoryResultInvalid, k_SteamInventoryResultInvalid };
	SteamInventoryResult_t m_InspectItemResult{ k_SteamInventoryResultInvalid };
#endif
	SteamInventoryResult_t m_DebugPrintInventoryResult{ k_SteamInventoryResultInvalid };
	float m_flDefsUpdateTime{ 0.0f };

#ifdef CLIENT_DLL
	STEAM_CALLBACK( CRD_Inventory_Manager, OnLobbyEntered, LobbyEnter_t )
	{
		if ( pParam->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess )
		{
			SendMedalBlob( pParam->m_ulSteamIDLobby );
		}
	}
#endif

	STEAM_CALLBACK( CRD_Inventory_Manager, OnSteamInventoryDefinitionUpdate, SteamInventoryDefinitionUpdate_t )
	{
		// this leaks memory, but it's a small amount and only happens when an update is manually pushed.
		s_ItemDefs.RemoveAll();
	}
} s_RD_Inventory_Manager;

#ifdef CLIENT_DLL
static void RD_Equipped_Medal_Changed( IConVar *var, const char *pOldValue, float flOldValue )
{
	s_RD_Inventory_Manager.OnEquippedMedalIDChanged();
}
ConVar rd_equipped_medal( "rd_equipped_medal", "0", FCVAR_ARCHIVE, "Steam inventory item ID of equipped medal.", RD_Equipped_Medal_Changed );

CON_COMMAND_F( rd_debug_print_inventory, "", FCVAR_HIDDEN )
{
	SteamInventory()->GetAllItems( &s_RD_Inventory_Manager.m_DebugPrintInventoryResult );
}

CON_COMMAND_F( rd_debug_fake_drop_entire_inventory, "", FCVAR_HIDDEN )
{
	SteamInventory()->GetAllItems( &s_RD_Inventory_Manager.m_PromotionalItemsResult );
}

CON_COMMAND_F( rd_econ_item_preview, "", FCVAR_HIDDEN )
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

		struct tm timestamp {};
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

	static bool DynamicPropertyAllowsArbitraryValues( const char *szPropertyName )
	{
		return false;
	}

	void ItemInstance_t::FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc )
	{
		V_UTF8ToUnicode( szDesc, wszBuf, sizeOfBufferInBytes );

		GET_INVENTORY_OR_BAIL;

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
				V_UTF8ToUnicode( DynamicProps[szToken], wszReplacement, sizeof( wszReplacement ) );
			}
			else
			{
				int64_t nValue = strtoll( DynamicProps[szToken], NULL, 10 );
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

	const ItemDef_t *GetItemDef( SteamItemDef_t id )
	{
		unsigned short index = s_ItemDefs.Find( id );
		if ( s_ItemDefs.IsValidIndex( index ) )
		{
			return s_ItemDefs[index];
		}

		GET_INVENTORY_OR_BAIL( NULL );

		const char *szLang = SteamApps() ? SteamApps()->GetCurrentGameLanguage() : "english";
#ifdef GAME_DLL
		if ( engine->IsDedicatedServer() )
		{
			szLang = rd_dedicated_server_language.GetString();
		}
#endif

		ItemDef_t *pItemDef = new ItemDef_t;
		pItemDef->ID = id;

		CUtlMemory<char> szBuf( 0, 1024 );

		uint32_t count{};

#define FETCH_PROPERTY( szPropertyName ) \
		pInventory->GetItemDefinitionProperty( id, szPropertyName, NULL, &count ); \
		szBuf.EnsureCapacity( count + 1 ); \
		count = szBuf.Count(); \
		pInventory->GetItemDefinitionProperty( id, szPropertyName, szBuf.Base(), &count )

		char szKey[256];

#ifdef CLIENT_DLL
		pItemDef->Icon = NULL;
		FETCH_PROPERTY( "icon_url" );
		if ( *szBuf.Base() )
		{
			pItemDef->Icon = new CSteamItemIcon( szBuf.Base() );
		}

		pItemDef->IconSmall = pItemDef->Icon;
		FETCH_PROPERTY( "icon_url_small" );
		if ( *szBuf.Base() )
		{
			pItemDef->IconSmall = new CSteamItemIcon( szBuf.Base() );
		}
#endif

		FETCH_PROPERTY( "item_slot" );
		pItemDef->ItemSlot = szBuf.Base();
		FETCH_PROPERTY( "tags" );
		ParseTags( pItemDef->Tags, szBuf.Base() );
		FETCH_PROPERTY( "allowed_tags_from_tools" );
		ParseTags( pItemDef->AllowedTagsFromTools, szBuf.Base() );
		FETCH_PROPERTY( "accessory_tag" );
		pItemDef->AccessoryTag = szBuf.Base();
		FETCH_PROPERTY( "compressed_dynamic_props" );
		if ( *szBuf.Base() )
		{
			CSplitString CompressedDynamicProps{ szBuf.Base(), ";" };
			FOR_EACH_VEC( CompressedDynamicProps, i )
			{
				pItemDef->CompressedDynamicProps.CopyAndAddToTail( CompressedDynamicProps[i] );
			}
		}

		V_snprintf( szKey, sizeof( szKey ), "display_type_%s", szLang );
		FETCH_PROPERTY( "display_type" );
		pItemDef->DisplayType = szBuf.Base();
		FETCH_PROPERTY( szKey );
		if ( *szBuf.Base() )
			pItemDef->DisplayType = szBuf.Base();

		V_snprintf( szKey, sizeof( szKey ), "name_%s", szLang );
		FETCH_PROPERTY( "name" );
		pItemDef->Name = szBuf.Base();
		FETCH_PROPERTY( szKey );
		if ( *szBuf.Base() )
			pItemDef->Name = szBuf.Base();
		
		V_snprintf( szKey, sizeof( szKey ), "description_%s", szLang );
		FETCH_PROPERTY( "description" );
		pItemDef->Description = szBuf.Base();
		FETCH_PROPERTY( szKey );
		if ( *szBuf.Base() )
			pItemDef->Description = szBuf.Base();
		FETCH_PROPERTY( "ingame_description_english" );
		if ( *szBuf.Base() )
			pItemDef->Description = szBuf.Base();
		V_snprintf( szKey, sizeof( szKey ), "ingame_description_%s", szLang );
		FETCH_PROPERTY( szKey );
		if ( *szBuf.Base() )
			pItemDef->Description = szBuf.Base();

		V_snprintf( szKey, sizeof( szKey ), "briefing_name_%s", szLang );
		pItemDef->BriefingName = pItemDef->Name;
		FETCH_PROPERTY( "briefing_name_english" );
		if ( *szBuf.Base() )
			pItemDef->BriefingName = szBuf.Base();
		FETCH_PROPERTY( szKey );
		if ( *szBuf.Base() )
			pItemDef->BriefingName = szBuf.Base();

		V_snprintf( szKey, sizeof( szKey ), "before_description_%s", szLang );
		FETCH_PROPERTY( "before_description_english" );
		pItemDef->BeforeDescription = szBuf.Base();
		FETCH_PROPERTY( szKey );
		if ( *szBuf.Base() )
			pItemDef->BeforeDescription = szBuf.Base();

		V_snprintf( szKey, sizeof( szKey ), "after_description_%s", szLang );
		FETCH_PROPERTY( "after_description_english" );
		pItemDef->AfterDescription = szBuf.Base();
		FETCH_PROPERTY( szKey );
		if ( *szBuf.Base() )
			pItemDef->AfterDescription = szBuf.Base();

		V_snprintf( szKey, sizeof( szKey ), "accessory_description_%s", szLang );
		FETCH_PROPERTY( "accessory_description_english" );
		pItemDef->AccessoryDescription = szBuf.Base();
		FETCH_PROPERTY( szKey );
		if ( *szBuf.Base() )
			pItemDef->AccessoryDescription = szBuf.Base();

		FETCH_PROPERTY( "after_description_only_multi_stack" );
		Assert( !V_strcmp( szBuf.Base(), "" ) || !V_strcmp( szBuf.Base(), "true" ) || !V_strcmp( szBuf.Base(), "false" ) );
		pItemDef->AfterDescriptionOnlyMultiStack = !V_strcmp( szBuf.Base(), "true" );
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
			if ( !pItemDef || pItemDef->ItemSlot != szRequiredSlot )
			{
				DevWarning( "ReactiveDropInventory::ValidateItemData: item fits in slot '%s', not '%s'\n", pItemDef ? pItemDef->ItemSlot.Get() : "<NO DEF>", szRequiredSlot);

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
			return;
		}

		GET_INVENTORY_OR_BAIL;

		pInventory->GrantPromoItems( &s_RD_Inventory_Manager.m_PromotionalItemsResult );
	}

#ifdef RD_CRAFTING_ENABLED
	static const char *const s_RDWorkshopCompetitionTags[] =
	{
		"BossFight2023",
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
#ifdef RD_CRAFTING_ENABLED
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
#endif

#undef GET_INVENTORY_OR_BAIL
}

#define RD_ITEM_ID_BITS 30
#define RD_ITEM_ACCESSORY_BITS 13

BEGIN_NETWORK_TABLE_NOBASE( CRD_ItemInstance, DT_RD_ItemInstance )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iItemDefID ) ),
	RecvPropInt( RECVINFO( m_nQuantity ) ),
	RecvPropArray( RecvPropInt( RECVINFO( m_iAccessory[0] ) ), m_iAccessory ),
	RecvPropArray( RecvPropInt( RECVINFO( m_nCounter[0] ) ), m_nCounter ),
#else
	SendPropInt( SENDINFO( m_iItemDefID ), RD_ITEM_ID_BITS, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nQuantity ), 31, SPROP_UNSIGNED ),
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

void CRD_ItemInstance::Reset()
{
	m_iItemDefID = 0;
	m_nQuantity = 1;
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

	m_iItemDefID = instance.ItemDefID;
	m_nQuantity = instance.Quantity;
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_iItemDefID );
	Assert( pDef );
	if ( !pDef )
		return;

	Assert( pDef->CompressedDynamicProps.Count() + ( pDef->AccessoryTag.IsEmpty() ? 0 : RD_ITEM_MAX_ACCESSORIES ) <= RD_ITEM_MAX_COMPRESSED_DYNAMIC_PROPS );

	FOR_EACH_VEC( pDef->CompressedDynamicProps, i )
	{
		Assert( !ReactiveDropInventory::DynamicPropertyAllowsArbitraryValues( pDef->CompressedDynamicProps[i] ) );

		if ( !V_stricmp( pDef->CompressedDynamicProps[i], "m_unQuantity" ) )
		{
			m_nCounter.Set( i, instance.Quantity );
		}
		else if ( instance.DynamicProps.Defined( pDef->CompressedDynamicProps[i] ) )
		{
			const char *szPropValue = instance.DynamicProps[instance.DynamicProps.Find( pDef->CompressedDynamicProps[i] )];
			m_nCounter.Set( i + pDef->CompressedDynamicProps.Count(), strtoll( szPropValue, NULL, 10 ) );
		}
	}

	if ( !pDef->AccessoryTag.IsEmpty() && instance.Tags.Defined( pDef->AccessoryTag ) )
	{
		const CUtlStringList &accessories = instance.Tags[instance.Tags.Find( pDef->AccessoryTag )];
		Assert( accessories.Count() <= RD_ITEM_MAX_ACCESSORIES );

		FOR_EACH_VEC( accessories, i )
		{
			if ( i >= RD_ITEM_MAX_ACCESSORIES )
				break;

			SteamItemDef_t iAccessoryID = strtol( accessories[i], NULL, 10 );
			Assert( iAccessoryID < ( 1 << RD_ITEM_ACCESSORY_BITS ) );
			m_iAccessory.Set( i, iAccessoryID );

			char szProperty[255];
			V_snprintf( szProperty, sizeof( szProperty ), "%s_%d", pDef->AccessoryTag.Get(), iAccessoryID );
			Assert( !ReactiveDropInventory::DynamicPropertyAllowsArbitraryValues( szProperty ) );

			if ( instance.DynamicProps.Defined( szProperty ) )
			{
				const char *szPropValue = instance.DynamicProps[instance.DynamicProps.Find( szProperty )];
				m_nCounter.Set( i + pDef->CompressedDynamicProps.Count(), strtoll( szPropValue, NULL, 10 ) );
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
				V_wcsncpy( wszReplacement, UTIL_RD_CommaNumber( m_nCounter[j] ), sizeof( wszReplacement ) );
				break;
			}
		}

		if ( !pDef->AccessoryTag.IsEmpty() )
		{
			for ( int j = 0; j < RD_ITEM_MAX_ACCESSORIES; j++ )
			{
				if ( m_iAccessory[j] == 0 )
					continue;

				char szAccessoryProp[128]{};
				V_snprintf( szAccessoryProp, sizeof( szAccessoryProp ), "%s_%d", pDef->AccessoryTag.Get(), m_iAccessory[j] );

				if ( !V_strcmp( szToken, szAccessoryProp ) )
				{
					V_wcsncpy( wszReplacement, UTIL_RD_CommaNumber( m_nCounter[pDef->CompressedDynamicProps.Count() + j] ), sizeof( wszReplacement ) );
					break;
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
extern ConVar rd_briefing_item_details_color1;
extern ConVar rd_briefing_item_details_color2;
ConVar rd_briefing_item_accessory_color( "rd_briefing_item_accessory_color", "191 191 191 255", FCVAR_NONE );

void CRD_ItemInstance::FormatDescription( vgui::RichText *pRichText ) const
{
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_iItemDefID );
	wchar_t wszBuf[2048];

	pRichText->SetText( "" );

	FormatDescription( wszBuf, sizeof( wszBuf ), pDef->BeforeDescription );
	if ( wszBuf[0] )
	{
		pRichText->InsertColorChange( rd_briefing_item_details_color2.GetColor() );
		pRichText->InsertString( wszBuf );
		pRichText->InsertString( L"\n\n" );
	}

	bool bAnyAccessories = false;
	for ( int i = 0; i < RD_ITEM_MAX_ACCESSORIES; i++ )
	{
		if ( m_iAccessory[i] == 0 )
			continue;

		bAnyAccessories = true;
		FormatDescription( wszBuf, sizeof( wszBuf ), ReactiveDropInventory::GetItemDef( m_iAccessory[i] )->AccessoryDescription );
		if ( wszBuf[0] )
		{
			pRichText->InsertColorChange( rd_briefing_item_accessory_color.GetColor() );
			pRichText->InsertString( wszBuf );
			pRichText->InsertString( L"\n" );
		}
	}

	if ( bAnyAccessories )
		pRichText->InsertString( L"\n" );

	V_UTF8ToUnicode( pDef->Description, wszBuf, sizeof( wszBuf ) );
	pRichText->InsertColorChange( rd_briefing_item_details_color1.GetColor() );
	pRichText->InsertString( wszBuf );

	if ( !pDef->AfterDescriptionOnlyMultiStack || m_nQuantity > 1 )
	{
		FormatDescription( wszBuf, sizeof( wszBuf ), pDef->AfterDescription );
		if ( wszBuf[0] )
		{
			pRichText->InsertColorChange( rd_briefing_item_details_color2.GetColor() );
			pRichText->InsertString( L"\n\n" );
			pRichText->InsertString( wszBuf );
		}
	}
}
#endif

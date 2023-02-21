#include "cbase.h"
#include "rd_inventory_shared.h"
#include "rd_lobby_utils.h"
#include "asw_util_shared.h"
#include "jsmn.h"

#ifdef CLIENT_DLL
#include <vgui/IImage.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Controls.h>
#include "lodepng.h"
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
		if ( pInventory )
		{
#ifdef CLIENT_DLL
			pInventory->DestroyResult( m_EquippedMedalResult );
			pInventory->DestroyResult( m_PromotionalItemsResult );
			pInventory->DestroyResult( m_DebugPrintInventoryResult );
#endif
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
		DebugPrintResult( hResult );
		Assert( !"TODO: UI for item drops" );
		SteamInventory()->DestroyResult( hResult );
		hResult = k_SteamInventoryResultInvalid;
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

		if ( pParam->m_handle == m_EquippedMedalResult )
		{
			// Pre-load our own equipped medal icon even if we're not in a lobby.
			SteamItemDef_t iMedalDef = ReactiveDropInventory::GetItemDetails( m_EquippedMedalResult, 0 ).m_iDefinition;
			( void )ReactiveDropInventory::GetItemDef( iMedalDef );

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

		if ( pParam->m_handle == m_DebugPrintInventoryResult )
		{
			DebugPrintResult( m_DebugPrintInventoryResult );
			SteamInventory()->DestroyResult( m_DebugPrintInventoryResult );
			m_DebugPrintInventoryResult = k_SteamInventoryResultInvalid;

			return;
		}
	}

	SteamInventoryResult_t m_EquippedMedalResult{ k_SteamInventoryResultInvalid };
	SteamInventoryResult_t m_PromotionalItemsResult{ k_SteamInventoryResultInvalid };
	CUtlVector<SteamItemDef_t> m_PromotionalItemsNext{};
	SteamInventoryResult_t m_DebugPrintInventoryResult{ k_SteamInventoryResultInvalid };
	float m_flDefsUpdateTime{ 0.0f };

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

	static bool ParseDynamicProps( KeyValues *pKV, const char *szDynamicProps )
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

			pKV->SetString( szKey, szValue );
		}

		return true;
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
		pItemDef->Tags = szBuf.Base();

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

		FETCH_PROPERTY( "after_description_only_multi_stack" );
		Assert( !V_strcmp( szBuf.Base(), "" ) || !V_strcmp( szBuf.Base(), "true" ) || !V_strcmp( szBuf.Base(), "false" ) );
		pItemDef->AfterDescriptionOnlyMultiStack = !V_strcmp( szBuf.Base(), "true" );
#undef FETCH_PROPERTY

		s_ItemDefs.Insert( id, pItemDef );

		return pItemDef;
	}

	void FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc, SteamInventoryResult_t hResult, uint32_t index )
	{
		V_UTF8ToUnicode( szDesc, wszBuf, sizeOfBufferInBytes );

		GET_INVENTORY_OR_BAIL;

		char szDynamicProps[1500]{};
		uint32_t count = sizeof( szDynamicProps );
		if ( !pInventory->GetResultItemProperty( hResult, index, "dynamic_props", szDynamicProps, &count ) )
		{
			return;
		}

		KeyValues::AutoDelete pKV( "DynamicProps" );
		if ( !ParseDynamicProps( pKV, szDynamicProps ) )
		{
			return;
		}

		char szToken[128]{};
		wchar_t wszTokenReplacement[128]{};

		for ( size_t i = 0; i < sizeOfBufferInBytes; i++ )
		{
			if ( wszBuf[i] == L'\0' )
			{
				return;
			}

			if ( wszBuf[i] != L'%' )
			{
				continue;
			}

			V_wcsncpy( wszTokenReplacement, L"MISSING", sizeof( wszTokenReplacement ) );

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

					Assert( tokenReplacementLength < NELEMS( wszTokenReplacement ) );
				}

				wszTokenReplacement[tokenReplacementLength] = L'\0';
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
				SteamItemDetails_t details = GetItemDetails( hResult, index );
				V_snwprintf( wszTokenReplacement, ARRAYSIZE( wszTokenReplacement ), L"%d", details.m_unQuantity );
			}

			const wchar_t *wszReplacement = pKV->GetWString( szToken, wszTokenReplacement );
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

		byte decodedData[1024]{};
		V_hextobinary( szEncodedData, nEncodedChars, decodedData, sizeof( decodedData ) );

		if ( !pInventory->DeserializeResult( &hResult, decodedData, nEncodedChars / 2 ) )
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
			char szSlot[256]{};
			uint32_t count = sizeof( szSlot );
			if ( !pInventory->GetItemDefinitionProperty( GetItemDetails( hResult, 0 ).m_iDefinition, "item_slot", szSlot, &count ) || V_strcmp( szSlot, szRequiredSlot ) )
			{
				DevWarning( "ReactiveDropInventory::ValidateItemData: item fits in slot '%s', not '%s'\n", szSlot, szRequiredSlot );

				bValid = false;
				return true;
			}
		}

		bValid = true;
		return true;
	}

	SteamItemDetails_t GetItemDetails( SteamInventoryResult_t hResult, uint32_t index )
	{
		GET_INVENTORY_OR_BAIL( SteamItemDetails_t{} );

		uint32_t count{};
		pInventory->GetResultItems( hResult, NULL, &count );
		if ( index >= count )
		{
			return SteamItemDetails_t{};
		}

		CUtlMemory<SteamItemDetails_t> details( 0, count );
		pInventory->GetResultItems( hResult, details.Base(), &count );

		return details[index];
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

#undef GET_INVENTORY_OR_BAIL
}

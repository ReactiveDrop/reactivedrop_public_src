#include "cbase.h"
#include "rd_inventory_shared.h"
#include "fmtstr.h"
#include "rd_png_texture.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CUtlStringMap<CSteamItemIcon *> s_ItemIcons( false );

class CSteamItemIcon : public CRD_PNG_Texture
{
public:
	CSteamItemIcon( const char *szURL, bool bForceLoadRemote ) :
		m_URLHash{ CRC32_ProcessSingleBuffer( szURL, V_strlen( szURL ) ) }
	{
		if ( Init( "vgui/inventory/cache", m_URLHash, bForceLoadRemote ) )
			return;

		char szDebugName[512];
		V_snprintf( szDebugName, sizeof( szDebugName ), "inventory item icon %s", szURL );

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
				m_szDebugName = strdup( szDebugName );
				m_HTTPRequestCompleted.Set( hAPICall, this, &CSteamItemIcon::OnRequestCompleted );
			}
			else
			{
				OnFailedToLoadData( "Sending HTTP request failed!", szDebugName );
			}
		}
		else
		{
			OnFailedToLoadData( "No ISteamHTTP access", szDebugName );
		}
	}

	~CSteamItemIcon()
	{
		if ( m_szDebugName )
		{
			free( m_szDebugName );
			m_szDebugName = NULL;
		}
	}

	const CRC32_t m_URLHash;
private:
	char *m_szDebugName{};
	void OnRequestCompleted( HTTPRequestCompleted_t *pParam, bool bIOFailure )
	{
		if ( bIOFailure || !pParam->m_bRequestSuccessful )
		{
			OnFailedToLoadData( "IO Failure", m_szDebugName );
			free( m_szDebugName );
			m_szDebugName = NULL;
			return;
		}

		ISteamHTTP *pHTTP = SteamHTTP();
		Assert( pHTTP );
		if ( !pHTTP )
		{
			OnFailedToLoadData( "No access to ISteamHTTP inside callback from HTTP request!", m_szDebugName );
			free( m_szDebugName );
			m_szDebugName = NULL;
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
		}

		pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );

		OnPNGDataReady( data.Base(), pParam->m_unBodySize, m_szDebugName );
		free( m_szDebugName );
		m_szDebugName = NULL;
	}

	CCallResult<CSteamItemIcon, HTTPRequestCompleted_t> m_HTTPRequestCompleted;
};

vgui::IImage *GetSteamItemIcon( const char *szURL, bool bForceLoadRemote )
{
#ifdef DBGFLAG_ASSERT
	static CUtlMap<CRC32_t, CUtlString> s_HashToURL( DefLessFunc( CRC32_t ) );
	CRC32_t iHash = CRC32_ProcessSingleBuffer( szURL, V_strlen( szURL ) );
	unsigned short iHashIndex = s_HashToURL.Find( iHash );
	if ( !s_HashToURL.IsValidIndex( iHashIndex ) )
	{
		s_HashToURL.Insert( iHash, szURL );
	}
	else
	{
		// if this fails, it means we have a hash collision!
		// we need to rename one of the icons in this unusual case.
		Assert( s_HashToURL[iHashIndex] == szURL );
	}
#endif

	UtlSymId_t index = s_ItemIcons.Find( szURL );
	if ( index != s_ItemIcons.InvalidIndex() )
	{
		if ( bForceLoadRemote )
		{
			Warning( "Loading icon for URL %s: already loaded; cannot create new icon\n", szURL );
		}
		return s_ItemIcons[index];
	}

	return s_ItemIcons[szURL] = new CSteamItemIcon( szURL, bForceLoadRemote );
}

ITexture *ReactiveDropInventory::ItemDef_t::GetAccessoryIcon() const
{
	if ( !AccessoryIcon && AccessoryImage && AccessoryImage->GetNumFrames() )
	{
		CFmtStr szTextureName{ "vgui/inventory/cache/%08x", assert_cast< CSteamItemIcon * >( AccessoryImage )->m_URLHash };
		AccessoryIcon = materials->FindTexture( szTextureName, TEXTURE_GROUP_CLIENT_EFFECTS );
	}

	return AccessoryIcon;
}

vgui::IImage *ReactiveDropInventory::ItemInstance_t::GetIcon() const
{
	const ItemDef_t *pDef = GetItemDef( ItemDefID );
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

#ifdef _DEBUG
CON_COMMAND_F( rd_load_all_inventory_defs, "load data and icons for all defined AS:RD inventory items", FCVAR_NOT_CONNECTED )
{
	// Item def IDs are split into ranges. The important ones are:
	// id <= 0 - invalid.
	// 1 <= id <= 999999 - "public" items, retrieved from GetItemDefinitionIDs.
	// 1000000 <= id <= 899999999 - "private" items; for now all of these are in the store.
	// 900000000 <= id <= 999999999 - "unique" items; one copy of each exists (this is an AS:RD restriction, not an inventory service one). IDs are consecutive.
	// 1000000000 <= id - Steam internal reserved ID range.

	ISteamInventory *pInventory = SteamInventory();
	Assert( pInventory );
	if ( !pInventory )
	{
		Warning( "No inventory API!\n" );
		return;
	}

	uint32_t nItemDefCount{};
	pInventory->GetItemDefinitionIDs( NULL, &nItemDefCount );
	CUtlMemory<SteamItemDef_t> DefIDs{ 0, int( nItemDefCount ) };
	bool bOK = pInventory->GetItemDefinitionIDs( DefIDs.Base(), &nItemDefCount );
	Assert( bOK ); ( void )bOK;

	Msg( "Checking %d item defs...\n", nItemDefCount );
	int iLoading = 0;
	FOR_EACH_VEC( DefIDs, i )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( DefIDs[i] );
		Assert( pDef );
		if ( pDef )
		{
			if ( pDef->Icon && pDef->Icon->GetNumFrames() == 0 )
			{
				iLoading++;
			}

			FOR_EACH_VEC( pDef->StyleIcons, j )
			{
				if ( pDef->StyleIcons[j]->GetNumFrames() == 0 )
				{
					iLoading++;
				}
			}
		}
	}

	SteamAPICall_t hCall = pInventory->RequestPrices();
	bool bFailed{};
	SteamInventoryRequestPricesResult_t param{};
	while ( !SteamUtils()->GetAPICallResult( hCall, &param, sizeof( param ), param.k_iCallback, &bFailed ) )
	{
		ThreadSleep( 10 );
		SteamAPI_RunCallbacks();
	}

	if ( bFailed )
	{
		Warning( "Failed to load mtx item list\n" );
		return;
	}

	DefIDs.Purge();
	DefIDs.EnsureCapacity( pInventory->GetNumItemsWithPrices() );
	CUtlMemory<uint64_t> CurrentPrices{ 0, int( pInventory->GetNumItemsWithPrices() ) };
	CUtlMemory<uint64_t> BasePrices{ 0, int( pInventory->GetNumItemsWithPrices() ) };
	pInventory->GetItemsWithPrices( DefIDs.Base(), CurrentPrices.Base(), BasePrices.Base(), pInventory->GetNumItemsWithPrices() );

	Msg( "Checking %d store item defs...\n", pInventory->GetNumItemsWithPrices() );
	FOR_EACH_VEC( DefIDs, i )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( DefIDs[i] );
		Assert( pDef );
		if ( pDef )
		{
			if ( pDef->Icon && pDef->Icon->GetNumFrames() == 0 )
			{
				iLoading++;
			}

			FOR_EACH_VEC( pDef->StyleIcons, j )
			{
				if ( pDef->StyleIcons[j]->GetNumFrames() == 0 )
				{
					iLoading++;
				}
			}
		}
	}

	for ( SteamItemDef_t id = 900000000; ; id++ )
	{
		uint32_t count{};
		pInventory->GetItemDefinitionProperty( id, NULL, NULL, &count );
		if ( !count )
		{
			Msg( "Number of unique item defs: %d\n", id - 900000000 );
			break;
		}

		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( id );
		Assert( pDef );
		if ( pDef )
		{
			if ( pDef->Icon && pDef->Icon->GetNumFrames() == 0 )
			{
				iLoading++;
			}

			FOR_EACH_VEC( pDef->StyleIcons, j )
			{
				if ( pDef->StyleIcons[j]->GetNumFrames() == 0 )
				{
					iLoading++;
				}
			}
		}
	}

	Msg( "async loading icons: %d\n", iLoading );
}

CON_COMMAND_F( rd_inventory_create_icon_from_url, "Create a cache file for an inventory item icon. For dev use only. Put URL in quotes or it will interpret // as a comment.", FCVAR_NOT_CONNECTED )
{
	GetSteamItemIcon( args[1], true );
}
#endif

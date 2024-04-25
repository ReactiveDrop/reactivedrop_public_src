#include "cbase.h"
#include "rd_inventory_shared.h"
#include "asw_util_shared.h"
#include "MultiFontRichText.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_briefing_item_details_color1( "rd_briefing_item_details_color1", "221 238 255 255", FCVAR_NONE );
ConVar rd_briefing_item_details_color2( "rd_briefing_item_details_color2", "170 204 238 255", FCVAR_NONE );

void ReactiveDropInventory::ItemInstance_t::FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc, bool bIsSteamCommunityDesc ) const
{
	Assert( !bIsSteamCommunityDesc || !V_stristr( szDesc, "m_unQuantity" ) );

	V_UTF8ToUnicode( szDesc, wszBuf, sizeOfBufferInBytes );

	if ( bIsSteamCommunityDesc && DynamicProps.GetNumStrings() == 0 )
		return;

	ISteamInventory *pInventory = SteamInventory();
	Assert( pInventory );
	if ( !pInventory )
		return;

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

		size_t tokenLength = 1;
		while ( wszBuf[i + tokenLength] != L'%' )
		{
			wchar_t ch = wszBuf[i + tokenLength];
			if ( ch == L'\0' )
			{
				return;
			}

			if ( ( ch < L'a' || ch > L'z' ) && ( ch < L'A' || ch > L'Z' ) && ( ch < L'0' || ch > L'9' ) && ch != L'_' )
			{
				tokenLength = 0;
				break;
			}

			Assert( ch < 0x80 ); // assume ASCII
			szToken[tokenLength - 1] = ( char )ch;

			tokenLength++;

			Assert( tokenLength < sizeof( szToken ) );
		}

		// bail if there's a non-token character after the percent sign
		if ( tokenLength == 0 )
		{
			continue;
		}

		szToken[tokenLength - 1] = '\0';
		tokenLength++;

		if ( !bIsSteamCommunityDesc && !V_stricmp( szToken, "m_unQuantity" ) )
		{
			// special case: m_unQuantity is not stored in dynamic_props
			V_wcsncpy( wszReplacement, UTIL_RD_CommaNumber( Quantity ), sizeof( wszReplacement ) );
		}

		if ( !DynamicProps.Defined( szToken ) )
		{
			if ( DynamicPropertyDataType( szToken ) == FIELD_BOOLEAN )
			{
				V_wcsncpy( wszReplacement, L"false", sizeof( wszReplacement ) );
			}
			else if ( DynamicPropertyDataType( szToken ) != FIELD_STRING )
			{
				V_wcsncpy( wszReplacement, L"0", sizeof( wszReplacement ) );
			}
		}
		else if ( DynamicPropertyDataType( szToken ) == FIELD_STRING )
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
	reduced.FormatDescription( wszTemp, sizeOfBufferInBytes, szDesc, bIsSteamCommunityDesc );
	Assert( !V_wcscmp( wszBuf, wszTemp ) );
	delete[] wszTemp;
#endif
}

void ReactiveDropInventory::ItemInstance_t::FormatDescription( vgui::MultiFontRichText *pRichText, bool bIncludeAccessories, Color descriptionColor, Color beforeAfterColor ) const
{
	CRD_ItemInstance reduced{ *this };
	reduced.FormatDescription( pRichText, bIncludeAccessories, descriptionColor, beforeAfterColor );
}

void CRD_ItemInstance::FormatDescription( wchar_t *wszBuf, size_t sizeOfBufferInBytes, const CUtlString &szDesc, bool bIsSteamCommunityDesc ) const
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

		V_wcsncpy( wszReplacement, L"0", sizeof( wszReplacement ) );

		size_t tokenLength = 1;
		while ( wszBuf[i + tokenLength] != L'%' )
		{
			wchar_t ch = wszBuf[i + tokenLength];
			if ( ch == L'\0' )
			{
				return;
			}

			if ( ( ch < L'a' || ch > L'z' ) && ( ch < L'A' || ch > L'Z' ) && ( ch < L'0' || ch > L'9' ) && ch != L'_' )
			{
				tokenLength = 0;
				break;
			}

			Assert( ch < 0x80 ); // assume ASCII
			szToken[tokenLength - 1] = ( char )ch;

			tokenLength++;

			Assert( tokenLength < sizeof( szToken ) );
		}

		if ( tokenLength == 0 )
			continue;

		szToken[tokenLength - 1] = '\0';
		tokenLength++;

		Assert( !bIsSteamCommunityDesc || V_stricmp( szToken, "m_unQuantity" ) );
		Assert( !bIsSteamCommunityDesc || V_stricmp( szToken, "style" ) );

		FOR_EACH_VEC( pDef->CompressedDynamicProps, j )
		{
			if ( !V_strcmp( pDef->CompressedDynamicProps[j], szToken ) )
			{
				if ( !bIsSteamCommunityDesc && !V_strcmp( szToken, "style" ) && m_nCounter[j] >= 0 && m_nCounter[j] < pDef->StyleNames.Count() )
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

void CRD_ItemInstance::FormatDescription( vgui::MultiFontRichText *pRichText, bool bIncludeAccessories, Color descriptionColor, Color beforeAfterColor ) const
{
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_iItemDefID );
	wchar_t wszBuf[2048];

	FormatDescription( wszBuf, sizeof( wszBuf ), pDef->BeforeDescription, false );
	if ( wszBuf[0] )
	{
		pRichText->InsertColorChange( beforeAfterColor );
		pRichText->AppendBBCode( wszBuf );
		pRichText->InsertString( L"\n\n" );
	}

	FormatDescription( wszBuf, sizeof( wszBuf ), pDef->Description, !pDef->HasInGameDescription );
	pRichText->InsertColorChange( descriptionColor );
	pRichText->AppendBBCode( wszBuf );

	if ( bIncludeAccessories )
	{
		for ( int i = 0; i < RD_ITEM_MAX_ACCESSORIES; i++ )
		{
			if ( m_iAccessory[i] == 0 )
				continue;

			Assert( !pDef->AccessoryTag.IsEmpty() );

			FormatDescription( wszBuf, sizeof( wszBuf ), ReactiveDropInventory::GetItemDef( m_iAccessory[i] )->AccessoryDescription, true );
			if ( wszBuf[0] )
			{
				pRichText->InsertString( L"\n" );
				pRichText->InsertColorChange( descriptionColor );
				pRichText->AppendBBCode( wszBuf );
			}
		}
	}

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
		FormatDescription( wszBuf, sizeof( wszBuf ), pDef->AfterDescription, false );
		if ( wszBuf[0] )
		{
			pRichText->InsertString( L"\n\n" );
			pRichText->InsertColorChange( beforeAfterColor );
			pRichText->AppendBBCode( wszBuf );
		}
	}
}

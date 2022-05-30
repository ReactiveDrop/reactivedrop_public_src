#include "cbase.h"
#include "rd_text_filtering.h"
#include "steam/steam_api.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CRD_Text_Filtering g_RDTextFiltering;

// Uses settings at https://store.steampowered.com/account/preferences#CommunityContentPreferences
ConVar rd_text_filtering( "rd_text_filtering", "1", FCVAR_NONE, "Filter text for content using the Steam API." );

ConVar rd_text_filtering_debug_no_steamid( "rd_text_filtering_debug_no_steamid", "0", FCVAR_NONE, "(for testing) Fail all SteamID lookups for text filtering, causing \"friends\" and \"self\" special cases to not fire." );

CRD_Text_Filtering::CRD_Text_Filtering() : CAutoGameSystem( "CRD_Text_Filtering" )
{
}

void CRD_Text_Filtering::PostInit()
{
	if ( !SteamUtils() )
	{
		Warning( "RD_Text_Filtering: SteamUtils() returned NULL!\n" );
		return;
	}

	if ( !SteamUtils()->InitFilterText() )
	{
		DevMsg( "RD_Text_Filtering: filtering unavailable for this language.\n" );
	}
}

CSteamID CRD_Text_Filtering::GetClientSteamID( int client )
{
	player_info_t playerInfo;
	if ( !engine->GetPlayerInfo( client, &playerInfo ) )
	{
		return k_steamIDNil;
	}

	return CSteamID( playerInfo.friendsID, SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );
}

static void DoFilterText( ETextFilteringContext eContext, CSteamID sourceSteamID, char *szText, size_t bufSizeInBytes )
{
	if ( !rd_text_filtering.GetBool() || !SteamUtils() )
		return;

	if ( rd_text_filtering_debug_no_steamid.GetBool() )
		sourceSteamID = k_steamIDNil;

	char *szDest = (char *)stackalloc( bufSizeInBytes );
	SteamUtils()->FilterText( eContext, sourceSteamID, szText, szDest, bufSizeInBytes );
	V_strncpy( szText, szDest, bufSizeInBytes );
}

static void DoFilterText( ETextFilteringContext eContext, CSteamID sourceSteamID, wchar_t *wszText, size_t bufSizeInBytes )
{
	if ( !rd_text_filtering.GetBool() || !SteamUtils() )
		return;

	// handle worst case utf-8
	size_t utf8BufSize = bufSizeInBytes + ( bufSizeInBytes >> 1 );
	char *szText = (char *)stackalloc( utf8BufSize );

	V_UnicodeToUTF8( wszText, szText, utf8BufSize );
	DoFilterText( eContext, sourceSteamID, szText, utf8BufSize );
	V_UTF8ToUnicode( szText, wszText, bufSizeInBytes );
}

static void DoFilterText( ETextFilteringContext eContext, CSteamID sourceSteamID, CUtlString & szText )
{
	DoFilterText( eContext, sourceSteamID, szText.Get(), szText.Length() + 1 );
}

void CRD_Text_Filtering::FilterTextUnknown( wchar_t *wszText, size_t bufSizeInBytes, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextUnknown, sourceSteamID, wszText, bufSizeInBytes );
}
void CRD_Text_Filtering::FilterTextUnknown( char *szText, size_t bufSizeInBytes, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextUnknown, sourceSteamID, szText, bufSizeInBytes );
}
void CRD_Text_Filtering::FilterTextUnknown( CUtlString & szText, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextUnknown, sourceSteamID, szText );
}
void CRD_Text_Filtering::FilterTextGameContent( wchar_t *wszText, size_t bufSizeInBytes, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextGameContent, sourceSteamID, wszText, bufSizeInBytes );
}
void CRD_Text_Filtering::FilterTextGameContent( char *szText, size_t bufSizeInBytes, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextGameContent, sourceSteamID, szText, bufSizeInBytes );
}
void CRD_Text_Filtering::FilterTextGameContent( CUtlString & szText, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextGameContent, sourceSteamID, szText );
}
void CRD_Text_Filtering::FilterTextChat( wchar_t *wszText, size_t bufSizeInBytes, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextChat, sourceSteamID, wszText, bufSizeInBytes );
}
void CRD_Text_Filtering::FilterTextChat( char *szText, size_t bufSizeInBytes, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextChat, sourceSteamID, szText, bufSizeInBytes );
}
void CRD_Text_Filtering::FilterTextChat( CUtlString & szText, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextChat, sourceSteamID, szText );
}
void CRD_Text_Filtering::FilterTextName( wchar_t *wszText, size_t bufSizeInBytes, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextName, sourceSteamID, wszText, bufSizeInBytes );
}
void CRD_Text_Filtering::FilterTextName( char *szText, size_t bufSizeInBytes, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextName, sourceSteamID, szText, bufSizeInBytes );
}
void CRD_Text_Filtering::FilterTextName( CUtlString & szText, CSteamID sourceSteamID )
{
	DoFilterText( k_ETextFilteringContextName, sourceSteamID, szText );
}

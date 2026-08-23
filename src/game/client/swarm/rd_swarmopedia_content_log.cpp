#include "cbase.h"
#include "rd_swarmopedia_content_log.h"
#include "steam/isteamremotestorage.h"
#include "filesystem.h"
#include <ctime>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


static KeyValues::AutoDelete s_pSwarmopediaSeenContent{ "SeenContent" };
static bool s_bLoadedSeenContent = false;

static void LoadSeenContent()
{
	if ( s_bLoadedSeenContent )
		return;

	#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	ISteamRemoteStorage *pRemoteStorage = SteamRemoteStorage();
	#else
	ISteamRemoteStorage *pRemoteStorage = SteamRemoteStorage();
	#endif
	#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	if ( pRemoteStorage && pRemoteStorage->IsCloudEnabledForApp() && pRemoteStorage->FileExists( "reactivedrop/cfg/swarmopedia_found.txt" ) )
	#else
	if ( pRemoteStorage && pRemoteStorage->IsCloudEnabledForApp() && pRemoteStorage->FileExists( "reactivedrop/cfg/swarmopedia_found.txt" ) )
	#endif
	{
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		int nSize = pRemoteStorage->GetFileSize( "reactivedrop/cfg/swarmopedia_found.txt" );
#else
		int nSize = pRemoteStorage->GetFileSize( "reactivedrop/cfg/swarmopedia_found.txt" );
#endif
		CUtlBuffer buf;
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		pRemoteStorage->FileRead( "reactivedrop/cfg/swarmopedia_found.txt", buf.AccessForDirectRead( nSize ), nSize );
#else
		pRemoteStorage->FileRead( "reactivedrop/cfg/swarmopedia_found.txt", buf.AccessForDirectRead( nSize ), nSize );
#endif
		g_pFullFileSystem->WriteFile( "cfg/swarmopedia_found.txt", "MOD", buf );
	}

	s_pSwarmopediaSeenContent->LoadFromFile( g_pFullFileSystem, "cfg/swarmopedia_found.txt", "MOD" );

	s_bLoadedSeenContent = true;
}

static void SaveSeenContent()
{
	s_pSwarmopediaSeenContent->SaveToFile( g_pFullFileSystem, "cfg/swarmopedia_found.txt", "MOD" );

	#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	ISteamRemoteStorage *pRemoteStorage = SteamRemoteStorage();
	#else
	ISteamRemoteStorage *pRemoteStorage = SteamRemoteStorage();
	#endif
	#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	if ( pRemoteStorage && pRemoteStorage->IsCloudEnabledForApp() )
	#else
	if ( pRemoteStorage && pRemoteStorage->IsCloudEnabledForApp() )
	#endif
	{
		CUtlBuffer buf;
		g_pFullFileSystem->ReadFile( "cfg/swarmopedia_found.txt", "MOD", buf );
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		pRemoteStorage->FileWrite( "reactivedrop/cfg/swarmopedia_found.txt", buf.Base(), buf.TellMaxPut() );
#else
		pRemoteStorage->FileWrite( "reactivedrop/cfg/swarmopedia_found.txt", buf.Base(), buf.TellMaxPut() );
#endif

		DevMsg( "Saved reactivedrop/cfg/swarmopedia_found.txt to cloud.\n" );
	}
	else
	{
		DevMsg( "Saved cfg/swarmopedia_found.txt to locally.\n" );
	}
}

void SwarmopediaRecordSeenContent( const char *szCategory, const char *szName, const Vector &vecLocation, int iMarineProfile, const char *szExtraString, int iLockFlags )
{
	LoadSeenContent();

	FOR_EACH_TRUE_SUBKEY( s_pSwarmopediaSeenContent, pKey )
	{
		if ( V_stricmp( pKey->GetName(), szCategory ) )
			continue;

		if ( V_stricmp( pKey->GetString( "name" ), szName ) )
			continue;

		if ( pKey->GetInt( "locked" ) & ~iLockFlags )
		{
			int iRemainingLocks = pKey->GetInt( "locked" ) & iLockFlags;
			if ( iRemainingLocks )
			{
				pKey->SetInt( "locked", iRemainingLocks );
			}
			else
			{
				KeyValues *pLocked = pKey->FindKey( "locked" );
				pKey->RemoveSubKey( pLocked );
				pLocked->deleteThis();
			}

			pKey->SetString( "map", IGameSystem::MapName() );
			pKey->SetInt( "found", std::time( NULL ) );
			if ( szExtraString )
				pKey->SetString( "extra", szExtraString );
			pKey->SetInt( "profile", iMarineProfile );
			pKey->SetFloat( "x", vecLocation.x );
			pKey->SetFloat( "y", vecLocation.y );
			pKey->SetFloat( "z", vecLocation.z );

			SaveSeenContent();
		}

		return;
	}

	KeyValues *pKey = new KeyValues( szCategory );

	pKey->SetString( "name", szName );
	pKey->SetString( "map", IGameSystem::MapName() );
	pKey->SetInt( "found", std::time( NULL ) );
	if ( szExtraString )
		pKey->SetString( "extra", szExtraString );
	pKey->SetInt( "profile", iMarineProfile );
	pKey->SetFloat( "x", vecLocation.x );
	pKey->SetFloat( "y", vecLocation.y );
	pKey->SetFloat( "z", vecLocation.z );
	if ( iLockFlags )
		pKey->SetInt( "locked", iLockFlags );

	s_pSwarmopediaSeenContent->AddSubKey( pKey );

	SaveSeenContent();
}

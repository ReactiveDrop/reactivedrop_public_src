#pragma once

#include "steam/steam_api.h"

class KeyValues;

#define RD_MAX_CHALLENGES 1024

namespace ReactiveDropChallenges
{
#ifdef GAME_DLL
	void CreateNetworkStringTables();
#else
	void InstallStringTableCallback( const char *tableName );
	void ClearClientCache();
#endif

	// Returns true if the data was successfully read into pKV.
	bool ReadData( KeyValues *pKV, const char *pszChallengeName );
	bool ReadData( KeyValues *pKV, int index );

	// Returns the number of challenges known to the server.
	int Count();
	// Returns the name of the challenge at 0 <= index < Count().
	const char *Name( int index );

	// Returns the display name of the challenge.
	const char *DisplayName( const char *pszChallengeName );
	const char *DisplayName( int index );

	// Returns the ID of the workshop addon containing the challenge txt file.
	PublishedFileId_t WorkshopID( const char *pszChallengeName );
	PublishedFileId_t WorkshopID( int index );

	// Returns true if the challenge filename is the same as an official challenge.
	bool IsOfficial( const char *pszChallengeName );
	bool IsOfficial( int index );
};

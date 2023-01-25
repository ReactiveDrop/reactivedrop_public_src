#pragma once

#include "steam/steam_api.h"

class KeyValues;

#define RD_MAX_CHALLENGES 1024

#pragma pack(push, 1)
struct RD_Challenge_t
{
	PublishedFileId_t WorkshopID{ k_PublishedFileIdInvalid };
	bool ForceOnslaught : 1;
	bool IsOnslaught : 1;
	bool ForceHardcore : 1;
	bool IsHardcore : 1;
	bool AllowCoop : 1;
	bool AllowDeathmatch : 1;
	bool RequiredOnClient : 1;
	bool _Reserved1 : 1;
	// Title must be last in this struct
	char Title[255];
};
#pragma pack(pop)

namespace ReactiveDropChallenges
{
#ifdef GAME_DLL
	void CreateNetworkStringTables();
	void ClearServerCache();
#else
	void InstallStringTableCallback( const char *tableName );
	void ClearClientCache();
#endif

	// Get summary - this is available while connected even if the challenge isn't installed locally.
	// The data may be shorter than the type would imply, but the string at the end will be null-terminated.
	const RD_Challenge_t *GetSummary( const char *pszChallengeName );
	const RD_Challenge_t *GetSummary( int index );

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

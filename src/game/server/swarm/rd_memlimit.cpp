#include "cbase.h"
#if IsPlatformWindows()
#include "winlite.h"
#include <psapi.h>
#endif
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CRD_MemLimit : public CAutoGameSystem
{
public:
	CRD_MemLimit() : CAutoGameSystem( "CRD_MemLimit" )
	{
		sv_memlimit = NULL;
	}

	ConVar *sv_memlimit;

	bool Init() override
	{
		if ( !engine->IsDedicatedServer() )
		{
			return true;
		}

		sv_memlimit = g_pCVar->FindVar( "sv_memlimit" );
		if ( !sv_memlimit )
		{
			Error( "Cannot find ConVar sv_memlimit!\n" );
			return false;
		}

		sv_memlimit->RemoveFlags( FCVAR_CHEAT );

		return true;
	}

	void LevelShutdownPostEntity() override
	{
		int iLimit = sv_memlimit ? sv_memlimit->GetInt() : 0;
		if ( iLimit <= 0 )
		{
			return;
		}

		unsigned nBytes = GetMemoryUsage();
		int nMegaBytes = nBytes / 1024u / 1024u;

		Msg( "Memory usage: %d MiB (%u bytes); limit is %d MiB\n", nMegaBytes, nBytes, iLimit );

		if ( nMegaBytes > iLimit )
		{
			Warning( "Memory limit exceeded. Requesting exit.\n" );
			UTIL_RD_ExitOnLevelChange();
		}
	}

	unsigned GetMemoryUsage()
	{
#if IsPlatformWindows()
		PROCESS_MEMORY_COUNTERS_EX pmc;
		if ( GetProcessMemoryInfo( GetCurrentProcess(), ( PROCESS_MEMORY_COUNTERS * )&pmc, sizeof( pmc ) ) )
			return pmc.PrivateUsage;

		return 0;
#else
#error Need to implement CRD_MemLimit on this platform
#endif
	}
};

CRD_MemLimit g_RD_MemLimit;

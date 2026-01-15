#include "cbase.h"
#include "winlite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern void RDMixerInit();

static class CRD_Convar_Hacks final : public CAutoGameSystem
{
public:
	CRD_Convar_Hacks() : CAutoGameSystem( "CRD_Convar_Hacks" )
	{
	}

	constexpr static const char *s_szNonCheat[] =
	{
		// These were marked as cheats to avoid them being used as a kind
		// of primitive wallhack. As a top-down game, we are not concerned
		// with players seeing behind a wall, and due to the nature of the
		// game's spawning, it is often required to have active players
		// on a map in order to accurately measure its performance. We are
		// removing FCVAR_CHEAT from these commands and variables to allow
		// mappers and programmers to debug performance while spectating
		// or playing.
		"+showbudget",
		"+showbudget_texture",
		"+showbudget_texture_global",
		"-showbudget",
		"-showbudget_texture",
		"-showbudget_texture_global",
		"showbudget_texture",
	};

	constexpr static const char *s_szCheat[] =
	{
		"buildcubemaps",
		"envmap",
		"lightprobe",
		// our convar is getting overridden by the engine parent var
		"mat_motion_blur_enabled",
	};

	constexpr static const char *s_ChangeDefault[][2] =
	{
		{ "mat_motion_blur_enabled", "0" },
		{ "rate", "100000" },
	};

	constexpr static const char *s_szAddArchive[] =
	{
		"rate",
	};

	constexpr static const char *s_szRemoveArchive[] =
	{
		// These are very advanced settings, and if a player changes a setting
		// during slomo, FCVAR_ARCHIVE will cause them to save the wrong value.
		"cl_updaterate",
		"cl_cmdrate",
	};

	virtual bool Init() override
	{
		Assert( g_pCVar );
		if ( !g_pCVar )
			return false;

		for ( int i = 0; i < NELEMS( s_szNonCheat ); i++ )
		{
			ConCommandBase *pCmd = g_pCVar->FindCommandBase( s_szNonCheat[i] );
			Assert( pCmd );
			if ( !pCmd )
				continue;

			pCmd->RemoveFlags( FCVAR_CHEAT );
		}

		ApplyShowBudgetHack();
		ApplySndRestartHack();

		for ( int i = 0; i < NELEMS( s_szCheat ); i++ )
		{
			ConCommandBase *pCmd = g_pCVar->FindCommandBase( s_szCheat[i] );
			Assert( pCmd );
			if ( !pCmd )
				continue;

			pCmd->AddFlags( FCVAR_CHEAT );
		}

		for ( int i = 0; i < NELEMS( s_ChangeDefault ); i++ )
		{
			ApplyDefaultValueHack( s_ChangeDefault[i][0], s_ChangeDefault[i][1] );
		}

		for ( int i = 0; i < NELEMS( s_szAddArchive ); i++ )
		{
			// 2024-07-17: Visual Studio is for some reason making this array have 4 nulls after the one string that's in it.
			if ( s_szAddArchive[i] == NULL )
				continue;

			ConCommandBase *pCmd = g_pCVar->FindCommandBase( s_szAddArchive[i] );
			Assert( pCmd );
			if ( !pCmd )
				continue;

			pCmd->AddFlags( FCVAR_ARCHIVE );
		}

		for ( int i = 0; i < NELEMS( s_szRemoveArchive ); i++ )
		{
			ConCommandBase *pCmd = g_pCVar->FindCommandBase( s_szRemoveArchive[i] );
			Assert( pCmd );
			if ( !pCmd )
				continue;

			pCmd->RemoveFlags( FCVAR_ARCHIVE );
		}

		return true;
	}

	void ApplyShowBudgetHack()
	{
		// In addition to marking the convar as non-cheat, we have to disable the
		// code that closes the panel automatically when cheats are disabled.
		ConCommand *pCmd = g_pCVar->FindCommand( "+showbudget" );
		const byte *pCallback = *reinterpret_cast< const byte *const * >( reinterpret_cast< const byte * >( pCmd ) + sizeof( ConCommandBase ) );
		const byte *pPanel = **reinterpret_cast< const byte *const * const * >( pCallback + 3 );
		Assert( pPanel );
		byte *pThink = ( *reinterpret_cast< byte *const *const * >( pPanel ) )[28];
		Assert( pThink[21] == 0x75 );
		Assert( pThink[22] == 0x33 );

		DWORD oldProtect{};
		VirtualProtect( pThink + 21, 1, PAGE_EXECUTE_READWRITE, &oldProtect );
		pThink[21] = 0xEB;
		VirtualProtect( pThink + 21, 1, oldProtect, &oldProtect );
		FlushInstructionCache( GetCurrentProcess(), pThink, 22 );
	}

	void ApplySndRestartHack()
	{
		// snd_restart undoes the changes made by _rd_mixer_init, so re-run _rd_mixer_init whenever the player runs snd_restart.
		ConCommand *pCmd = g_pCVar->FindCommand( "snd_restart" );
		byte *pCallback = *reinterpret_cast< byte *const * >( reinterpret_cast< const byte * >( pCmd ) + sizeof( ConCommandBase ) );

		// function starts at 10053fc0
		// ret instruction at 100540e6
		// after that, we have 9 bytes of padding to work with
		byte *pRet = pCallback + 0x126;

		Assert( pRet[0] == 0xc3 );
		Assert( pRet[1] == 0xcc );
		Assert( pRet[2] == 0xcc );
		Assert( pRet[3] == 0xcc );
		Assert( pRet[4] == 0xcc );
		Assert( pRet[5] == 0xcc );
		Assert( pRet[6] == 0xcc );
		Assert( pRet[7] == 0xcc );
		Assert( pRet[8] == 0xcc );
		Assert( pRet[9] == 0xcc );

		DWORD oldProtect{};
		VirtualProtect( pRet, 5, PAGE_EXECUTE_READWRITE, &oldProtect );
		pRet[0] = 0xE9;
		*reinterpret_cast< uintptr_t * >( &pRet[1] ) = uintptr_t( &RDMixerInit ) - uintptr_t( pRet + 5 );
		VirtualProtect( pRet, 5, oldProtect, &oldProtect );
		FlushInstructionCache( GetCurrentProcess(), pRet, 5 );
	}

	void ApplyDefaultValueHack( const char *szName, const char *szNewDefault )
	{
		ConVar *pConVar = g_pCVar->FindVar( szName );
		Assert( pConVar );

		static_assert( sizeof( ConCommandBase ) == 24, "unexpected ConCommandBase size" );
		static_assert( sizeof( ConVar ) == 88, "unexpected ConVar size" );

		// ConVar *ConVar::m_pParent
		ConVar *pConVarParent = reinterpret_cast< ConVar ** >( pConVar )[7];
		Assert( pConVarParent == pConVar );

		// const char *ConVar::m_pszDefaultValue
		const char **ppConVarDefault = &reinterpret_cast< const char ** >( pConVarParent )[8];
		Assert( pConVarParent->GetDefault() == *ppConVarDefault );

		*ppConVarDefault = szNewDefault;
		pConVar->SetValue( szNewDefault );
	}
} s_RD_Convar_Hacks;




static bool ConCommandBaseSortFunc( const ConCommandBase* const &hLeftCMD, const ConCommandBase* const &hRightCMD )
{ 
	const char* szLeftCMDName = hLeftCMD->GetName();
	const char* szRightCMDName = hRightCMD->GetName();

	if ( *szLeftCMDName == '-' || *szLeftCMDName == '+' )
		szLeftCMDName++;
	if ( *szRightCMDName == '-' || *szRightCMDName == '+' )
		szRightCMDName++;

	return ( Q_stricmp( szLeftCMDName, szRightCMDName ) < 0 );
}

static char *StripTabsAndReturns( const char *inbuffer, char *outbuffer, int outbufferSize )
{
	char *out = outbuffer;
	const char *i = inbuffer;
	char *o = out;

	out[ 0 ] = 0;

	while ( *i && o - out < outbufferSize - 1 )
	{
		if ( *i == '\n' ||
			*i == '\r' ||
			*i == '\t' )
		{
			*o++ = ' ';
			i++;
			continue;
		}
		if ( *i == '\"' )
		{
			*o++ = '\'';
			i++;
			continue;
		}

		*o++ = *i++;
	}

	*o = '\0';

	return out;
}

CON_COMMAND( cvarlist_rd, "Prints a list of all cvars, with correct values (unlike normal cvarlist). No argument - current values, arg being 1 - default values, arg being 2 - current values in csv format, arg being 3 - default values in csv format" )
{
	bool bDefault = false;
	bool bCSV = false;
	int nArgs = args.ArgC();
	if ( nArgs >= 2 )
	{
		int nArg1 = atoi( args[1] );
		
		bDefault = nArg1 == 1 || nArg1 == 3;
		bCSV = nArg1 >= 2;
	}

	if ( !bCSV )
		ConMsg( "cvar list\n--------------\n" );
	else
		ConMsg( "\"Name\",\"Value\",\"GAMEDLL\",\"CLIENTDLL\",\"PROTECTED\",\"SPONLY\",\"ARCHIVE\",\"NOTIFY\",\"USERINFO\",\"PRINTABLEONLY\",\"UNLOGGED\",\"NEVER_AS_STRING\",\"REPLICATED\",\"CHEAT\",\"SS\",\"DEMO\",\"DONTRECORD\",\"SS_ADDED\",,\"Help Text\"\n" );

	const ConCommandBase* pCMD;
	ICvar::Iterator cvariterator( g_pCVar );
	CUtlRBTree< const ConCommandBase* > cvarsorted( 0, 0, ConCommandBaseSortFunc );

	for ( cvariterator.SetFirst(); cvariterator.IsValid(); cvariterator.Next() )
	{  
		pCMD = cvariterator.Get();

		if ( pCMD->IsFlagSet( FCVAR_HIDDEN ) || pCMD->IsFlagSet( FCVAR_DEVELOPMENTONLY ) )
			continue;

		cvarsorted.Insert( pCMD );
	}

	for ( int i = cvarsorted.FirstInorder(); i != cvarsorted.InvalidIndex(); i = cvarsorted.NextInorder( i ) )
    {
        pCMD = cvarsorted[ i ];
        if ( pCMD->IsCommand() )
        {
			if ( !bCSV )   // unchanged cvarlist console output format
			{
				char tempbuff[512]{};
				ConMsg( "%-40s : %-8s : %-16s : %s\n", pCMD->GetName(), "cmd", "", StripTabsAndReturns( pCMD->GetHelpText(), tempbuff, sizeof( tempbuff ) ) );
			}
			else   // CSV format
			{
				char tempbuff[512]{};
				ConMsg( "\"%s\",\"%s\",%s,\"%s\"\n", pCMD->GetName(), "cmd", ",,,,,,,,,,,,,,,,", StripTabsAndReturns( pCMD->GetHelpText(), tempbuff, sizeof( tempbuff ) ) );
			}
        }
		else
		{
			char szFullFlags[256]{};

			if ( !bCSV )   // unchanged cvarlist console output format
			{
				constexpr static const char* s_szFlagDesc[] =
				{
					"",
					"",
					"sv",
					"cl",
					"",
					"prot",
					"sp",
					"a",
					"nf",
					"user",
					"print",
					"log",
					"numeric",
					"rep",
					"cheat",
					"",
					"demo",
					"norecord",
				};
				
				for ( int c = 2; c < ARRAYSIZE( s_szFlagDesc ); ++c )
				{
					char szFlag[32]{};

					if ( pCMD->IsFlagSet( 1 << c ) && sizeof( s_szFlagDesc[c] ) != sizeof( "" ) )
					{
						Q_snprintf( szFlag, sizeof( szFlag ), ", %s", s_szFlagDesc[c] );
						Q_strncat( szFullFlags, szFlag, sizeof( szFullFlags ), COPY_ALL_CHARACTERS );
					}
				}
				char tempbuff[512]{};
				ConMsg( "%-40s : %-8s : %-16s : %s\n", pCMD->GetName(), bDefault ? static_cast< const ConVar* >( pCMD )->GetDefault() : static_cast< const ConVar* >( pCMD )->GetString(), szFullFlags, StripTabsAndReturns( pCMD->GetHelpText(), tempbuff, sizeof(tempbuff) ) );
			}
			else   // CSV format
			{
				constexpr static const char* s_szFlagDesc[] =
				{
					"",
					"",
					"GAMEDLL",
					"CLIENTDLL",
					"",
					"PROTECTED",
					"SPONLY",
					"ARCHIVE",
					"NOTIFY",
					"USERINFO",
					"PRINTABLEONLY",
					"UNLOGGED",
					"NEVER_AS_STRING",
					"REPLICATED",
					"CHEAT",
					"SS",
					"DEMO",
					"DONTRECORD",
					"SS_ADDED",
				};

				for ( int c = 2; c < ARRAYSIZE( s_szFlagDesc ); ++c )
				{
					if ( c == 4 )	// FCVAR_HIDDEN, we filter those out anyway
						continue;
					
					char szFlag[32]{};

					if ( pCMD->IsFlagSet( 1 << c ) && sizeof( s_szFlagDesc[c] ) != sizeof( "" ) )
					{
						Q_snprintf( szFlag, sizeof( szFlag ), "\"%s\",", s_szFlagDesc[c] );
						Q_strncat( szFullFlags, szFlag, sizeof( szFullFlags ), COPY_ALL_CHARACTERS );
					}
					else
					{
						Q_snprintf( szFlag, sizeof( szFlag ), "," );
						Q_strncat( szFullFlags, szFlag, sizeof( szFullFlags ), COPY_ALL_CHARACTERS );
					}
				}

				char tempbuff[512]{};
				ConMsg( "\"%s\",\"%s\",%s,\"%s\"\n", pCMD->GetName(), bDefault ? static_cast< const ConVar* >( pCMD )->GetDefault() : static_cast< const ConVar* >( pCMD )->GetString(), szFullFlags, StripTabsAndReturns( pCMD->GetHelpText(), tempbuff, sizeof(tempbuff) ) );
			}
        }
    }

	if ( !bCSV )
		ConMsg("--------------\n%3i total convars/concommands\n", cvarsorted.Count() );
}
#include "cbase.h"
#include "rd_challenges_shared.h"
#include "gamestringpool.h"
#include "filesystem.h"
#ifdef GAME_DLL
#include "networkstringtable_gamedll.h"
#else
#include "networkstringtable_clientdll.h"
#endif
#include "rd_workshop.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define RD_CHALLENGES_STRINGTABLE_NAME "ReactiveDropChallenges"

INetworkStringTable *g_StringTableReactiveDropChallenges = NULL;

static bool FillChallengeSummary( RD_Challenge_t &summary, const char *szKVFileName )
{
	V_memset( &summary, 0, sizeof( summary ) );
	summary.AllowCoop = true;

	summary.WorkshopID = g_ReactiveDropWorkshop.FindAddonProvidingFile( szKVFileName );

	KeyValues::AutoDelete pKV( "CHALLENGE" );
	if ( !UTIL_RD_LoadKeyValuesFromFile( pKV, filesystem, szKVFileName, "GAME" ) )
	{
		return false;
	}

	V_strncpy( summary.Title, pKV->GetString( "name" ), sizeof( summary.Title ) );

	bool bAnyAllowedModes = false;
	FOR_EACH_VALUE( pKV, pValue )
	{
		if ( V_stricmp( pValue->GetName(), "allowed_mode" ) )
		{
			continue;
		}

		if ( !bAnyAllowedModes )
		{
			bAnyAllowedModes = true;
			summary.AllowCoop = false;
		}

		if ( !V_stricmp( pValue->GetString(), "coop" ) )
		{
			summary.AllowCoop = true;
		}
		else if ( !V_stricmp( pValue->GetString(), "deathmatch" ) )
		{
			summary.AllowDeathmatch = true;
		}
		else
		{
			DevWarning( "unhandled allowed_mode '%s' in challenge '%s'\n", pValue->GetString(), szKVFileName );
		}
	}

	summary.RequiredOnClient = pKV->GetBool( "required_on_client" );

	if ( KeyValues *pConVars = pKV->FindKey( "convars" ) )
	{
		KeyValues *pFF1 = pConVars->FindKey( "asw_marine_ff_absorption" );
		KeyValues *pFF2 = pConVars->FindKey( "asw_sentry_friendly_fire_scale" );
		summary.ForceHardcore = pFF1 || pFF2;
		summary.IsHardcore = ( pFF1 && pFF1->GetInt() != 1 ) || ( pFF2 && pFF2->GetFloat() != 0.0f );

		KeyValues *pOn1 = pConVars->FindKey( "asw_horde_override" );
		KeyValues *pOn2 = pConVars->FindKey( "asw_wanderer_override" );
		summary.ForceOnslaught = pOn1 || pOn2;
		summary.IsOnslaught = ( pOn1 && pOn1->GetBool() ) || ( pOn2 && pOn2->GetBool() );
	}

	return true;
}

#ifdef GAME_DLL
extern ConVar rd_debug_string_tables;

void ReactiveDropChallenges::CreateNetworkStringTables()
{
	g_StringTableReactiveDropChallenges = networkstringtable->CreateStringTable( RD_CHALLENGES_STRINGTABLE_NAME, RD_MAX_CHALLENGES );
	Assert( g_StringTableReactiveDropChallenges );

	ClearServerCache();
}
void ReactiveDropChallenges::ClearServerCache()
{
	if ( !g_StringTableReactiveDropChallenges )
	{
		// not initialized yet
		return;
	}

	// TODO: purge old data

	RD_Challenge_t summary;
	char szKVFileName[MAX_PATH];
	char szChallengeName[MAX_PATH];
	FileFindHandle_t hFind;
	for ( const char *pszChallenge = filesystem->FindFirstEx( "resource/challenges/*.txt", "GAME", &hFind ); pszChallenge; pszChallenge = filesystem->FindNext( hFind ) )
	{
		Q_FileBase( pszChallenge, szChallengeName, sizeof( szChallengeName ) );
		Q_snprintf( szKVFileName, sizeof( szKVFileName ), "resource/challenges/%s", pszChallenge );

		if ( !FillChallengeSummary( summary, szKVFileName ) )
		{
			Warning( "Could not read challenge keyvalues for %s\n", szChallengeName );
			continue;
		}

		int nDataSize = offsetof( RD_Challenge_t, Title ) + V_strlen( summary.Title ) + 1;

		int index = g_StringTableReactiveDropChallenges->AddString( true, szChallengeName, nDataSize, &summary );
		if ( rd_debug_string_tables.GetBool() )
		{
			Msg( "Adding challenge %d to string table: %s, payload size %d\n", index, szChallengeName, nDataSize );
		}
	}
	filesystem->FindClose( hFind );
}
#else
static void OnReactiveDropChallengesStringChanged(
	void *object,
	INetworkStringTable *stringTable,
	int stringNumber,
	const char *newString,
	void const *newData )
{
}

void ReactiveDropChallenges::InstallStringTableCallback( const char *tableName )
{
	if ( 0 == Q_strcasecmp( tableName, RD_CHALLENGES_STRINGTABLE_NAME ) )
	{
		g_StringTableReactiveDropChallenges = networkstringtable->FindTable( tableName );
		if ( g_StringTableReactiveDropChallenges )
			g_StringTableReactiveDropChallenges->SetStringChangedCallback( NULL, OnReactiveDropChallengesStringChanged );
	}
}

static class CReactiveDropChallengesClientSystem : public CAutoGameSystem
{
public:
	CReactiveDropChallengesClientSystem() : CAutoGameSystem( "CReactiveDropChallengesClientSystem" ), m_LocalChallenges( 0, 16, true )
	{
		Clear();
	}

	CUtlSymbolTable m_LocalChallenges;
	bool m_bInit;

	void Clear()
	{
		m_LocalChallenges.RemoveAll();
		m_bInit = false;
	}

	virtual void LevelInitPreEntity()
	{
		Clear();
	}

	void ReadChallengeList()
	{
		if ( m_bInit )
		{
			return;
		}

		FileFindHandle_t hFind;
		for ( const char *pszChallenge = filesystem->FindFirstEx( "resource/challenges/*.txt", "GAME", &hFind ); pszChallenge; pszChallenge = filesystem->FindNext( hFind ) )
		{
			char szChallengeName[MAX_PATH];
			Q_FileBase( pszChallenge, szChallengeName, sizeof( szChallengeName ) );
			m_LocalChallenges.AddString( szChallengeName );
		}
		filesystem->FindClose( hFind );

		m_bInit = true;
	}
} s_ClientChallenges;

void ReactiveDropChallenges::ClearClientCache()
{
	s_ClientChallenges.Clear();
}
#endif

const RD_Challenge_t *ReactiveDropChallenges::GetSummary( const char *pszChallengeName )
{
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		static RD_Challenge_t s_summary;

		char szPath[MAX_PATH];
		Q_snprintf( szPath, sizeof( szPath ), "resource/challenges/%s.txt", pszChallengeName );

		if ( FillChallengeSummary( s_summary, szPath ) )
		{
			return &s_summary;
		}

		return NULL;
	}
#endif

	if( !g_StringTableReactiveDropChallenges )
		return NULL;

	int index = g_StringTableReactiveDropChallenges->FindStringIndex( pszChallengeName );
	if ( index == INVALID_STRING_INDEX )
	{
		return NULL;
	}
	
	int length;
	const void *pSummary = g_StringTableReactiveDropChallenges->GetStringUserData( index, &length);

	return static_cast< const RD_Challenge_t * >( pSummary );
}

const RD_Challenge_t *ReactiveDropChallenges::GetSummary( int index )
{
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		static RD_Challenge_t s_summary;

		const char *szName = Name( index );
		if ( !szName )
		{
			return NULL;
		}

		char szPath[MAX_PATH];
		Q_snprintf( szPath, sizeof( szPath ), "resource/challenges/%s.txt", szName );

		if ( FillChallengeSummary( s_summary, szPath ) )
		{
			return &s_summary;
		}

		return NULL;
	}
#endif

	Assert( g_StringTableReactiveDropChallenges );

	int length;
	const void *pSummary = g_StringTableReactiveDropChallenges->GetStringUserData( index, &length );

	return static_cast< const RD_Challenge_t * >( pSummary );
}

bool ReactiveDropChallenges::ReadData( KeyValues *pKV, int index )
{
	Assert( pKV );
	if ( index < 0 || index >= Count() )
	{
		Assert( index == -1 );
		return false;
	}

	return ReadData( pKV, Name( index ) );
}

bool ReactiveDropChallenges::ReadData( KeyValues *pKV, const char *pszChallengeName )
{
	Assert( pKV );
	Assert( pszChallengeName );

	char szPath[MAX_PATH];
	Q_snprintf( szPath, sizeof( szPath ), "resource/challenges/%s.txt", pszChallengeName );

	if ( UTIL_RD_LoadKeyValuesFromFile( pKV, filesystem, szPath, "GAME" ) )
	{
		pKV->SetUint64( "workshop", g_ReactiveDropWorkshop.FindAddonProvidingFile( szPath ) );
		return true;
	}
	return false;
}

int ReactiveDropChallenges::Count()
{
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		s_ClientChallenges.ReadChallengeList();
		return s_ClientChallenges.m_LocalChallenges.GetNumStrings();
	}
#endif

	Assert( g_StringTableReactiveDropChallenges );
	if ( !g_StringTableReactiveDropChallenges )
	{
		return 0;
	}

	return g_StringTableReactiveDropChallenges->GetNumStrings();
}

const char *ReactiveDropChallenges::Name( int index )
{
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		s_ClientChallenges.ReadChallengeList();
		return s_ClientChallenges.m_LocalChallenges.String( index );
	}
#endif

	Assert( g_StringTableReactiveDropChallenges );
	if ( !g_StringTableReactiveDropChallenges )
	{
		return NULL;
	}

	if ( index < 0 || index >= Count() )
	{
		Assert( index == -1 );
		return NULL;
	}

	return g_StringTableReactiveDropChallenges->GetString( index );
}

const char *ReactiveDropChallenges::DisplayName( const char *pszChallengeName )
{
	if ( const RD_Challenge_t *pChallenge = GetSummary( pszChallengeName ) )
	{
		return STRING( AllocPooledString( pChallenge->Title ) );
	}
	return "#rd_challenge_name_0";
}

const char *ReactiveDropChallenges::DisplayName( int index )
{
	if ( const RD_Challenge_t *pChallenge = GetSummary( index ) )
	{
		return STRING( AllocPooledString( pChallenge->Title ) );
	}
	return "#rd_challenge_name_0";
}

PublishedFileId_t ReactiveDropChallenges::WorkshopID( const char *pszChallengeName )
{
	if ( const RD_Challenge_t *pChallenge = GetSummary( pszChallengeName ) )
	{
		return pChallenge->WorkshopID;
	}
	return k_PublishedFileIdInvalid;
}

PublishedFileId_t ReactiveDropChallenges::WorkshopID( int index )
{
	if ( const RD_Challenge_t *pChallenge = GetSummary( index ) )
	{
		return pChallenge->WorkshopID;
	}
	return k_PublishedFileIdInvalid;
}

static const char *s_szOfficialChallenges[] =
{
	"asbi",
	"difficulty_tier1",
	"difficulty_tier2",
	"energy_weapons",
	"level_one",
	"one_hit",
	"riflemod_classic",
	"rd_first_person",
	"rd_third_person",
};

bool ReactiveDropChallenges::IsOfficial( const char *pszChallengeName )
{
	for ( int i = 0; i < NELEMS( s_szOfficialChallenges ); i++ )
	{
		if ( !Q_stricmp( s_szOfficialChallenges[i], pszChallengeName ) )
		{
			return true;
		}
	}
	return false;
}

bool ReactiveDropChallenges::IsOfficial( int index )
{
	return IsOfficial( Name( index ) );
}

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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define RD_CHALLENGES_STRINGTABLE_NAME "ReactiveDropChallenges"

INetworkStringTable *g_StringTableReactiveDropChallenges = NULL;

#ifdef GAME_DLL
void ReactiveDropChallenges::CreateNetworkStringTables()
{
	g_StringTableReactiveDropChallenges = networkstringtable->CreateStringTable( RD_CHALLENGES_STRINGTABLE_NAME, RD_MAX_CHALLENGES );

	Assert( g_StringTableReactiveDropChallenges );

	char szKVFileName[MAX_PATH];
	char szChallengeName[MAX_PATH];
	CUtlBuffer buf;
	FileFindHandle_t hFind;
	for ( const char *pszChallenge = filesystem->FindFirstEx( "resource/challenges/*.txt", "GAME", &hFind ); pszChallenge; pszChallenge = filesystem->FindNext( hFind ) )
	{
		Q_FileBase( pszChallenge, szChallengeName, sizeof( szChallengeName ) );
		Q_snprintf( szKVFileName, sizeof( szKVFileName ), "resource/challenges/%s", pszChallenge );
		KeyValues::AutoDelete pKV( "CHALLENGE" );
		if ( !pKV->LoadFromFile( filesystem, szKVFileName, "GAME" ) )
		{
			Warning( "Could not read challenge keyvalues for %s\n", szChallengeName );
			continue;
		}

		pKV->SetUint64( "workshop", g_ReactiveDropWorkshop.FindAddonProvidingFile( szKVFileName ) );

		buf.Clear();
		if ( !pKV->WriteAsBinary( buf ) )
		{
			Warning( "Could not write challenge keyvalues for %s\n", szChallengeName );
			continue;
		}

		int index = g_StringTableReactiveDropChallenges->AddString( true, szChallengeName, buf.TellPut(), buf.Base() );
		DevMsg( 2, "Adding challenge %d to string table: %s, payload size %d\n", index, szChallengeName, buf.TellPut() );
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
	CReactiveDropChallengesClientSystem() : CAutoGameSystem( "CReactiveDropChallengesClientSystem" )
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

		m_bInit = true;
	}
} s_ClientChallenges;

void ReactiveDropChallenges::ClearClientCache()
{
	s_ClientChallenges.Clear();
}

static bool ReadDataClient( KeyValues *pKV, const char *pszChallengeName )
{
	Assert( pKV );
	Assert( !engine->IsInGame() );

	s_ClientChallenges.ReadChallengeList();
	if ( !s_ClientChallenges.m_LocalChallenges.Find( pszChallengeName ).IsValid() )
	{
		return false;
	}

	char szPath[MAX_PATH];
	Q_snprintf( szPath, sizeof( szPath ), "resource/challenges/%s.txt", pszChallengeName );

	if ( pKV->LoadFromFile( filesystem, szPath, "GAME" ) )
	{
		pKV->SetUint64( "workshop", g_ReactiveDropWorkshop.FindAddonProvidingFile( szPath ) );
		return true;
	}
	return false;
}
#endif

bool ReactiveDropChallenges::ReadData( KeyValues *pKV, int index )
{
	Assert( pKV );
	if ( index < 0 || index >= Count() )
	{
		Assert( index == INVALID_STRING_INDEX );
		return false;
	}

#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		return ReadDataClient( pKV, Name( index ) );
	}
#endif

	int length;
	const void *pData = g_StringTableReactiveDropChallenges->GetStringUserData( index, &length );

	CUtlBuffer buf( pData, length, CUtlBuffer::READ_ONLY );
	return pKV->ReadAsBinary( buf );
}

bool ReactiveDropChallenges::ReadData( KeyValues *pKV, const char *pszChallengeName )
{
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		s_ClientChallenges.ReadChallengeList();
		return ReadDataClient( pKV, pszChallengeName );
	}
#endif

	Assert( g_StringTableReactiveDropChallenges );
	if ( !g_StringTableReactiveDropChallenges )
	{
		return false;
	}

	return ReadData( pKV, g_StringTableReactiveDropChallenges->FindStringIndex( pszChallengeName ) );
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
		Assert( index == INVALID_STRING_INDEX );
		return NULL;
	}

	return g_StringTableReactiveDropChallenges->GetString( index );
}

const char *ReactiveDropChallenges::DisplayName( const char *pszChallengeName )
{
	KeyValues::AutoDelete pKV( "CHALLENGE" );
	if ( ReadData( pKV, pszChallengeName ) )
	{
		return STRING( AllocPooledString( pKV->GetString( "name", pszChallengeName ) ) );
	}
	return "#rd_challenge_name_0";
}

const char *ReactiveDropChallenges::DisplayName( int index )
{
	KeyValues::AutoDelete pKV( "CHALLENGE" );
	if ( ReadData( pKV, index ) )
	{
		return STRING( AllocPooledString( pKV->GetString( "name", Name( index ) ) ) );
	}
	return "#rd_challenge_name_0";
}

PublishedFileId_t ReactiveDropChallenges::WorkshopID( const char *pszChallengeName )
{
	KeyValues::AutoDelete pKV( "CHALLENGE" );
	if ( ReadData( pKV, pszChallengeName ) )
	{
		return pKV->GetUint64( "workshop" );
	}
	return k_PublishedFileIdInvalid;
}

PublishedFileId_t ReactiveDropChallenges::WorkshopID( int index )
{
	KeyValues::AutoDelete pKV( "CHALLENGE" );
	if ( ReadData( pKV, index ) )
	{
		return pKV->GetUint64( "workshop" );
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

#include "cbase.h"
#include "rd_missions_shared.h"
#include "filesystem.h"
#ifdef GAME_DLL
#include "networkstringtable_gamedll.h"
#else
#include "networkstringtable_clientdll.h"
#endif
#include "fmtstr.h"
#include "rd_workshop.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define RD_CAMPAIGNS_STRINGTABLE_NAME "ReactiveDropCampaigns"
#define RD_MISSIONS_STRINGTABLE_NAME "ReactiveDropMissions"

INetworkStringTable *g_StringTableReactiveDropCampaigns = NULL;
INetworkStringTable *g_StringTableReactiveDropMissions = NULL;
static CStringPool s_MissionStringTable{ StringPoolCaseSensitive };
static CUtlVector<RD_Campaign_t> s_UnpackedCampaigns;
static CUtlVector<RD_Mission_t> s_UnpackedMissions;
static bool s_bRebuildUnpackedMissionData = false;
int ReactiveDropMissions::s_nDataResets = 1;

static const char *s_szCampaignNamesFirst[] =
{
	"jacob",
#if defined( RD_6A_CAMPAIGNS_ACCIDENT32 ) && defined( RD_NEW_CAMPAIGN_SPOTLIGHT )
	"rd_accident32",
#endif
#if defined( RD_6A_CAMPAIGNS_ADANAXIS ) && defined( RD_NEW_CAMPAIGN_SPOTLIGHT )
	"rd_adanaxis",
#endif
	"rd-operationcleansweep",
	"rd_nh_campaigns",
	"rd-tarnorcampaign1",
	"rd_paranoia",
#if defined( RD_6A_CAMPAIGNS_ACCIDENT32 ) && !defined( RD_NEW_CAMPAIGN_SPOTLIGHT )
	"rd_accident32",
#endif
	"rd-area9800",
#if defined( RD_6A_CAMPAIGNS_ADANAXIS ) && !defined( RD_NEW_CAMPAIGN_SPOTLIGHT )
	"rd_adanaxis",
#endif
	"tilarus5",
	"rd_biogen_corporation",
	"rd_research7",
	"rd_lanasescape_campaign",
};

static const char *s_szMissionNamesFirst[] =
{
	"dm_desert",
	"dm_deima",
	"dm_residential",
	"dm_testlab",
	"rd-bonus_mission1",
	"rd-bonus_mission2",
	"rd-bonus_mission3",
	"rd-bonus_mission4",
	"rd-bonus_mission5",
	"rd-bonus_mission6",
	"rd-bonus_mission7",
#ifdef RD_6A_CAMPAIGNS_ACCIDENT32
	"rd-acc_complex",
	"rd-ht-marine_academy",
#endif
#ifdef RD_6A_CAMPAIGNS_ADANAXIS
	"rd-ada_new_beginning",
	"rd-ada_anomaly",
#endif
	"rd-bonus10_sewrev",
	"rd-bonus12_rydrev",
	"rd-bonus14_cargrev",
	"rd-bonus15_landrev",
};

#pragma pack(push, 1)
struct NetworkedMissionMetadata_t
{
	PublishedFileId_t WorkshopID;
	bool TagBonus : 1;
	bool TagDeathmatch : 1;
	bool TagEndless : 1;
	bool _Reserved1 : 1;
	bool _Reserved2 : 1;
	bool _Reserved3 : 1;
	bool _Reserved4 : 1;
	bool _Reserved5 : 1;

	constexpr void Clear()
	{
		WorkshopID = k_PublishedFileIdInvalid;
		TagBonus = false;
		TagDeathmatch = false;
		TagEndless = false;
	}
	void SetFromFile( const char *szKVFileName )
	{
		Clear();

		WorkshopID = g_ReactiveDropWorkshop.FindAddonProvidingFile( szKVFileName );

		const CReactiveDropWorkshop::WorkshopItem_t & info = g_ReactiveDropWorkshop.TryQueryAddon( WorkshopID );
		char szBaseName[MAX_PATH];
		V_FileBase( szKVFileName, szBaseName, sizeof( szBaseName ) );
		if ( info.bAdminOverrideDeathmatch && !V_strnicmp( szKVFileName, "dm_", 3 ) )
		{
			TagDeathmatch = true;
		}
		else if ( info.bAdminOverrideBonus )
		{
			TagBonus = true;
		}

		KeyValues::AutoDelete pKV( "GAME" );
		if ( UTIL_RD_LoadKeyValuesFromFile( pKV, filesystem, szKVFileName, "GAME" ) )
		{
			FOR_EACH_VALUE( pKV, pValue )
			{
				if ( V_stricmp( pValue->GetName(), "tag" ) )
					continue;

				if ( !V_stricmp( pValue->GetString(), "bonus" ) )
					TagBonus = true;
				else if ( !V_stricmp( pValue->GetString(), "deathmatch" ) )
					TagDeathmatch = true;
				else if ( !V_stricmp( pValue->GetString(), "endless" ) )
					TagEndless = true;
			}
		}
	}
};
#pragma pack(pop)

// workshop addons with adminoverride_Bonus or adminoverride_Deathmatch
// have been manually checked to ensure they have only one-mission campaigns
// and that the name of the campaign is the same as or substantially similar to
// the name of the campaign's only mission.
static bool ShouldIgnoreCampaign( const char *szName )
{
	char szPath[MAX_PATH];
	V_snprintf( szPath, sizeof( szPath ), "resource/campaigns/%s.txt", szName );

	PublishedFileId_t iAddon = g_ReactiveDropWorkshop.FindAddonProvidingFile( szPath );
	if ( iAddon == k_PublishedFileIdInvalid )
	{
		return false;
	}

	const CReactiveDropWorkshop::WorkshopItem_t & item = g_ReactiveDropWorkshop.TryQueryAddon( iAddon );
	return item.bAdminOverrideBonus || item.bAdminOverrideDeathmatch;
}

#ifdef GAME_DLL
ConVar rd_debug_string_tables( "rd_debug_string_tables", "0", FCVAR_NONE, "log the creation of string tables for the mission chooser" );

void ReactiveDropMissions::CreateNetworkStringTables()
{
	g_StringTableReactiveDropCampaigns = networkstringtable->CreateStringTable( RD_CAMPAIGNS_STRINGTABLE_NAME, RD_MAX_CAMPAIGNS );
	Assert( g_StringTableReactiveDropCampaigns );

	g_StringTableReactiveDropMissions = networkstringtable->CreateStringTable( RD_MISSIONS_STRINGTABLE_NAME, RD_MAX_MISSIONS );
	Assert( g_StringTableReactiveDropMissions );

	ClearServerCache();
}
void ReactiveDropMissions::ClearServerCache()
{
	if ( !g_StringTableReactiveDropCampaigns )
	{
		Assert( !g_StringTableReactiveDropMissions );
		// not initialized yet
		return;
	}
	Assert( g_StringTableReactiveDropMissions );

	// TODO: purge old data

	char szKVFileName[MAX_PATH];
	NetworkedMissionMetadata_t metadata{};

	for ( int i = 0; i < NELEMS( s_szCampaignNamesFirst ); i++ )
	{
		const char *szBaseName = s_szCampaignNamesFirst[i];
		V_snprintf( szKVFileName, sizeof( szKVFileName ), "resource/campaigns/%s.txt", szBaseName );
		metadata.SetFromFile( szKVFileName );

		if ( ShouldIgnoreCampaign( szBaseName ) )
		{
			if ( rd_debug_string_tables.GetBool() )
			{
				Msg( "Not adding campaign to string table (marked as single mission by admin): %s, workshop %llu (official)\n", szBaseName, metadata.WorkshopID );
			}
			continue;
		}

		int index = g_StringTableReactiveDropCampaigns->AddString( true, szBaseName, sizeof( metadata ), &metadata );
		if ( rd_debug_string_tables.GetBool() )
		{
			Msg( "Adding campaign %d to string table: %s, workshop %llu (official)\n", index, szBaseName, metadata.WorkshopID );
		}
	}

	for ( int i = 0; i < NELEMS( s_szMissionNamesFirst ); i++ )
	{
		const char *szBaseName = s_szMissionNamesFirst[i];
		V_snprintf( szKVFileName, sizeof( szKVFileName ), "resource/overviews/%s.txt", szBaseName );
		metadata.SetFromFile( szKVFileName );

		int index = g_StringTableReactiveDropMissions->AddString( true, szBaseName, sizeof( metadata ), &metadata );
		if ( rd_debug_string_tables.GetBool() )
		{
			Msg( "Adding mission %d to string table: %s, workshop %llu (official)\n", index, szBaseName, metadata.WorkshopID );
		}
	}

	char szBaseName[MAX_PATH];
	FileFindHandle_t hFind;
	for ( const char *pszCampaign = filesystem->FindFirstEx( "resource/campaigns/*.txt", "GAME", &hFind ); pszCampaign; pszCampaign = filesystem->FindNext( hFind ) )
	{
		V_FileBase( pszCampaign, szBaseName, sizeof( szBaseName ) );
		V_snprintf( szKVFileName, sizeof( szKVFileName ), "resource/campaigns/%s", pszCampaign );
		metadata.SetFromFile( szKVFileName );

		if ( ShouldIgnoreCampaign( szBaseName ) )
		{
			if ( rd_debug_string_tables.GetBool() )
			{
				Msg( "Not adding campaign to string table (marked as single mission by admin): %s, workshop %llu\n", szBaseName, metadata.WorkshopID );
			}
			continue;
		}

		int index = g_StringTableReactiveDropCampaigns->AddString( true, szBaseName, sizeof( metadata ), &metadata );
		if ( rd_debug_string_tables.GetBool() )
		{
			Msg( "Adding campaign %d to string table: %s, workshop %llu\n", index, szBaseName, metadata.WorkshopID );
		}
	}
	filesystem->FindClose( hFind );

	for ( const char *pszMission = filesystem->FindFirstEx( "resource/overviews/*.txt", "GAME", &hFind ); pszMission; pszMission = filesystem->FindNext( hFind ) )
	{
		V_FileBase( pszMission, szBaseName, sizeof( szBaseName ) );
		V_snprintf( szKVFileName, sizeof( szKVFileName ), "resource/overviews/%s", pszMission );
		metadata.SetFromFile( szKVFileName );

		int index = g_StringTableReactiveDropMissions->AddString( true, szBaseName, sizeof( metadata ), &metadata );
		if ( rd_debug_string_tables.GetBool() )
		{
			Msg( "Adding mission %d to string table: %s, workshop %llu\n", index, szBaseName, metadata.WorkshopID );
		}
	}
	filesystem->FindClose( hFind );

	s_bRebuildUnpackedMissionData = true;
}
#else
static void OnReactiveDropMissionsStringChanged(
	void *object,
	INetworkStringTable *stringTable,
	int stringNumber,
	const char *newString,
	void const *newData )
{
	s_bRebuildUnpackedMissionData = true;
}

void ReactiveDropMissions::InstallStringTableCallback( const char *tableName )
{
	if ( 0 == V_strcasecmp( tableName, RD_CAMPAIGNS_STRINGTABLE_NAME ) )
	{
		g_StringTableReactiveDropCampaigns = networkstringtable->FindTable( tableName );
		if ( g_StringTableReactiveDropCampaigns )
			g_StringTableReactiveDropCampaigns->SetStringChangedCallback( NULL, OnReactiveDropMissionsStringChanged );

		s_bRebuildUnpackedMissionData = true;
	}
	else if ( 0 == V_strcasecmp( tableName, RD_MISSIONS_STRINGTABLE_NAME ) )
	{
		g_StringTableReactiveDropMissions = networkstringtable->FindTable( tableName );
		if ( g_StringTableReactiveDropMissions )
			g_StringTableReactiveDropMissions->SetStringChangedCallback( NULL, OnReactiveDropMissionsStringChanged );

		s_bRebuildUnpackedMissionData = true;
	}
}

static class CReactiveDropMissionsClientSystem : public CAutoGameSystem
{
public:
	CReactiveDropMissionsClientSystem() : CAutoGameSystem( "CReactiveDropMissionsClientSystem" ), m_LocalCampaigns( 0, 16, true ), m_LocalMissions( 0, 16, true )
	{
		Clear();
	}

	CUtlSymbolTable m_LocalCampaigns;
	CUtlSymbolTable m_LocalMissions;
	bool m_bInit;

	void Clear()
	{
		m_LocalCampaigns.RemoveAll();
		m_LocalMissions.RemoveAll();
		m_bInit = false;
	}

	virtual void LevelInitPreEntity()
	{
		Clear();
	}

	virtual void LevelShutdownPostEntity()
	{
		// we might be leaving the server
		s_bRebuildUnpackedMissionData = true;
	}

	void ReadMissionList()
	{
		if ( m_bInit )
		{
			return;
		}

		for ( int i = 0; i < NELEMS( s_szCampaignNamesFirst ); i++ )
		{
			Assert( filesystem->FileExists( CFmtStr( "resource/campaigns/%s.txt", s_szCampaignNamesFirst[i] ), "GAME" ) );
			if ( ShouldIgnoreCampaign( s_szCampaignNamesFirst[i] ) )
				continue;

			m_LocalCampaigns.AddString( s_szCampaignNamesFirst[i] );
		}

		for ( int i = 0; i < NELEMS( s_szMissionNamesFirst ); i++ )
		{
			Assert( filesystem->FileExists( CFmtStr( "resource/overviews/%s.txt", s_szMissionNamesFirst[i] ), "GAME" ) );
			m_LocalMissions.AddString( s_szMissionNamesFirst[i] );
		}

		FileFindHandle_t hFind;
		for ( const char *pszCampaign = filesystem->FindFirstEx( "resource/campaigns/*.txt", "GAME", &hFind ); pszCampaign; pszCampaign = filesystem->FindNext( hFind ) )
		{
			char szCampaignName[MAX_PATH];
			Q_FileBase( pszCampaign, szCampaignName, sizeof( szCampaignName ) );
			if ( ShouldIgnoreCampaign( szCampaignName ) )
				continue;

			m_LocalCampaigns.AddString( szCampaignName );
		}
		filesystem->FindClose( hFind );

		for ( const char *pszMission = filesystem->FindFirstEx( "resource/overviews/*.txt", "GAME", &hFind ); pszMission; pszMission = filesystem->FindNext( hFind ) )
		{
			char szMissionName[MAX_PATH];
			Q_FileBase( pszMission, szMissionName, sizeof( szMissionName ) );
			m_LocalMissions.AddString( szMissionName );
		}
		filesystem->FindClose( hFind );

		m_bInit = true;
		s_bRebuildUnpackedMissionData = true;
	}
} s_ClientMissions;

void ReactiveDropMissions::ClearClientCache()
{
	s_ClientMissions.Clear();
	s_ClientMissions.ReadMissionList();
}
#endif

int ReactiveDropMissions::CountCampaigns()
{
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		s_ClientMissions.ReadMissionList();
		return s_ClientMissions.m_LocalCampaigns.GetNumStrings();
	}
#endif

	Assert( g_StringTableReactiveDropCampaigns );
	if ( !g_StringTableReactiveDropCampaigns )
	{
		return 0;
	}

	return g_StringTableReactiveDropCampaigns->GetNumStrings();
}

int ReactiveDropMissions::CountMissions()
{
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		s_ClientMissions.ReadMissionList();
		return s_ClientMissions.m_LocalMissions.GetNumStrings();
	}
#endif

	Assert( g_StringTableReactiveDropMissions );
	if ( !g_StringTableReactiveDropMissions )
	{
		return 0;
	}

	return g_StringTableReactiveDropMissions->GetNumStrings();
}

const char *ReactiveDropMissions::CampaignName( int index )
{
	Assert( index >= 0 && index < CountCampaigns() );

#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		s_ClientMissions.ReadMissionList();
		return s_ClientMissions.m_LocalCampaigns.String( index );
	}
#endif

	Assert( g_StringTableReactiveDropCampaigns );
	if ( !g_StringTableReactiveDropCampaigns )
	{
		return NULL;
	}

	return g_StringTableReactiveDropCampaigns->GetString( index );
}

const char *ReactiveDropMissions::MissionName( int index )
{
	Assert( index >= 0 && index < CountMissions() );

#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		s_ClientMissions.ReadMissionList();
		return s_ClientMissions.m_LocalMissions.String( index );
	}
#endif

	Assert( g_StringTableReactiveDropMissions );
	if ( !g_StringTableReactiveDropMissions )
	{
		return NULL;
	}

	return g_StringTableReactiveDropMissions->GetString( index );
}

static bool ReadCampaignData( KeyValues *pKV, int index )
{
	Assert( pKV );

	const char *pszCampaignName;
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		pszCampaignName = s_ClientMissions.m_LocalCampaigns.String( index );
	}
	else
#endif
	{
		Assert( g_StringTableReactiveDropCampaigns );
		if ( !g_StringTableReactiveDropCampaigns )
			return false;

		pszCampaignName = g_StringTableReactiveDropCampaigns->GetString( index );
	}

	char szPath[MAX_PATH];
	V_snprintf( szPath, sizeof( szPath ), "resource/campaigns/%s.txt", pszCampaignName );

	return UTIL_RD_LoadKeyValuesFromFile( pKV, filesystem, szPath, "GAME" );
}

static bool ReadMissionData( KeyValues *pKV, int index )
{
	Assert( pKV );

	const char *pszMissionName;
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		pszMissionName = s_ClientMissions.m_LocalMissions.String( index );
	}
	else
#endif
	{
		Assert( g_StringTableReactiveDropMissions );
		if ( !g_StringTableReactiveDropMissions )
			return false;

		pszMissionName = g_StringTableReactiveDropMissions->GetString( index );
	}

	char szPath[MAX_PATH];
	V_snprintf( szPath, sizeof( szPath ), "resource/overviews/%s.txt", pszMissionName );

	return UTIL_RD_LoadKeyValuesFromFile( pKV, filesystem, szPath, "GAME" );
}

static void ClearUnpackedMissionData()
{
	s_MissionStringTable.FreeAll();
	s_UnpackedCampaigns.Purge();
	s_UnpackedMissions.Purge();

	s_UnpackedCampaigns.SetCount( ReactiveDropMissions::CountCampaigns() );
	s_UnpackedMissions.SetCount( ReactiveDropMissions::CountMissions() );

	s_bRebuildUnpackedMissionData = false;
	ReactiveDropMissions::s_nDataResets++;
}

static string_t AllocMissionsPooledString( const char *pszValue )
{
	if ( pszValue )
		return MAKE_STRING( s_MissionStringTable.Allocate( pszValue ) );
	return NULL_STRING;
}

static void UnpackNetworkedTags( CUtlVector<string_t> &tags, INetworkStringTable *pStringTable, int index )
{
	Assert( pStringTable );
	if ( !pStringTable )
		return;

	int length;
	const NetworkedMissionMetadata_t *pMetadata = static_cast<const NetworkedMissionMetadata_t *>( pStringTable->GetStringUserData( index, &length ) );

	if ( pMetadata->TagBonus )
		tags.AddToTail( MAKE_STRING( "bonus" ) );
	if ( pMetadata->TagDeathmatch )
		tags.AddToTail( MAKE_STRING( "deathmatch" ) );
	if ( pMetadata->TagEndless )
		tags.AddToTail( MAKE_STRING( "endless" ) );
}

const RD_Campaign_t *ReactiveDropMissions::GetCampaign( int index )
{
	if ( s_bRebuildUnpackedMissionData )
		ClearUnpackedMissionData();

	Assert( s_UnpackedCampaigns.Count() == CountCampaigns() );
	// somehow, the campaign string table is getting stuff added to it without firing the change listener.
	// pretend it did notify us of the change if that happens.
	if ( s_UnpackedCampaigns.Count() != CountCampaigns() )
		ClearUnpackedMissionData();

	if ( index < 0 || index >= CountCampaigns() )
	{
		Assert( index == -1 );
		return NULL;
	}

	RD_Campaign_t *pCampaign = &s_UnpackedCampaigns[index];
	if ( pCampaign->BaseName[0] )
		return pCampaign;

	V_strncpy( pCampaign->BaseName, CampaignName( index ), sizeof( pCampaign->BaseName ) );
	pCampaign->WorkshopID = CampaignWorkshopID( index );

	KeyValues::AutoDelete pKV( "GAME" );
	if ( !ReadCampaignData( pKV, index ) )
	{
		pCampaign->CampaignName = AllocMissionsPooledString( pCampaign->BaseName );
		pCampaign->CampaignDescription = MAKE_STRING( "" );

		pCampaign->ChooseCampaignTexture = MAKE_STRING( "swarm/missionpics/addonmissionpic" );
		pCampaign->CampaignTextureName = MAKE_STRING( "../tools/toolsblack" );
		pCampaign->CampaignTextureLayer[0] = MAKE_STRING( "swarm/campaign/campaignmap_emptylayer" );
		pCampaign->CampaignTextureLayer[1] = MAKE_STRING( "swarm/campaign/campaignmap_emptylayer" );
		pCampaign->CampaignTextureLayer[2] = MAKE_STRING( "swarm/campaign/campaignmap_emptylayer" );

#ifdef CLIENT_DLL
		Assert( engine->IsConnected() );
		if ( engine->IsConnected() )
#endif
		{
			UnpackNetworkedTags( pCampaign->Tags, g_StringTableReactiveDropCampaigns, index );
		}

		return pCampaign;
	}

	pCampaign->Installed = true;

	pCampaign->CampaignName = AllocMissionsPooledString( pKV->GetString( "CampaignName" ) );
	pCampaign->CampaignDescription = AllocMissionsPooledString( pKV->GetString( "CampaignDescription" ) );
	pCampaign->CustomCreditsFile = AllocMissionsPooledString( pKV->GetString( "CustomCreditsFile", DEFAULT_CREDITS_FILE ) );

	pCampaign->ChooseCampaignTexture = AllocMissionsPooledString( pKV->GetString( "ChooseCampaignTexture" ) );
	pCampaign->CampaignTextureName = AllocMissionsPooledString( pKV->GetString( "CampaignTextureName" ) );
	pCampaign->CampaignTextureLayer[0] = AllocMissionsPooledString( pKV->GetString( "CampaignTextureLayer1" ) );
	pCampaign->CampaignTextureLayer[1] = AllocMissionsPooledString( pKV->GetString( "CampaignTextureLayer2" ) );
	pCampaign->CampaignTextureLayer[2] = AllocMissionsPooledString( pKV->GetString( "CampaignTextureLayer3" ) );

	pCampaign->GalaxyX = clamp( pKV->GetInt( "GalaxyX" ), 0, 1023 );
	pCampaign->GalaxyY = clamp( pKV->GetInt( "GalaxyY" ), 0, 1023 );

	for ( int i = 0; i < ASW_NUM_SEARCH_LIGHTS; i++ )
	{
		pCampaign->SearchLightX[i] = clamp( pKV->GetInt( CFmtStr( "Searchlight%dX", i + 1 ) ), 0, 1023 );
		pCampaign->SearchLightY[i] = clamp( pKV->GetInt( CFmtStr( "Searchlight%dY", i + 1 ) ), 0, 1023 );
		pCampaign->SearchLightAngle[i] = pKV->GetInt( CFmtStr( "Searchlight%dAngle", i + 1 ) ) % 360;
	}

	FOR_EACH_VALUE( pKV, pValue )
	{
		if ( !V_stricmp( pValue->GetName(), "tag" ) )
		{
			pCampaign->Tags.AddToTail( AllocMissionsPooledString( pValue->GetString() ) );
		}
	}

	// first pass: only record map names
	FOR_EACH_TRUE_SUBKEY( pKV, pSubKey )
	{
		if ( V_stricmp( pSubKey->GetName(), "MISSION" ) )
		{
			continue;
		}

		RD_Campaign_Mission_t *pMission = &pCampaign->Missions[pCampaign->Missions.AddToTail()];
		V_strncpy( pMission->MapName, pSubKey->GetString( "MapName" ), sizeof( pMission->MapName ) );
	}

	int iMission = 0;

	// second pass: actually decode mission data
	FOR_EACH_TRUE_SUBKEY( pKV, pSubKey )
	{
		if ( !V_stricmp( pSubKey->GetName(), "MISSION" ) )
		{
			RD_Campaign_Mission_t *pMission = &pCampaign->Missions[iMission];
			Assert( !V_strcmp( pMission->MapName, pSubKey->GetString( "MapName" ) ) );

			pMission->CampaignIndex = iMission;
			pMission->MissionName = AllocMissionsPooledString( pSubKey->GetString( "MissionName" ) );
			pMission->LocationDescription = AllocMissionsPooledString( pSubKey->GetString( "LocationDescription" ) );
			pMission->ShortBriefing = AllocMissionsPooledString( pSubKey->GetString( "ShortBriefing" ) );
			pMission->ThreatString = AllocMissionsPooledString( pSubKey->GetString( "ThreatString" ) );
			pMission->LocationX = clamp( pSubKey->GetInt( "LocationX" ), 0, 1023 );
			pMission->LocationY = clamp( pSubKey->GetInt( "LocationY" ), 0, 1023 );
			pMission->DifficultyModifier = pSubKey->GetInt( "DifficultyModifier" );
			pMission->AlwaysVisible = pSubKey->GetBool( "AlwaysVisible" );
			pMission->NeedsMoreThanOneMarine = pSubKey->GetBool( "NeedsMoreThanOneMarine" );

			CSplitString Links( pSubKey->GetString( "Links" ), " " );
			FOR_EACH_VEC( Links, i )
			{
				FOR_EACH_VEC( pCampaign->Missions, j )
				{
					if ( !V_stricmp( Links[i], pCampaign->Missions[j].MapName ) )
					{
						pMission->Links.AddToTail( j );
						break;
					}
				}
			}

			iMission++;
		}
	}

	Assert( iMission == pCampaign->Missions.Count() );

	return pCampaign;
}

const RD_Mission_t *ReactiveDropMissions::GetMission( int index )
{
	if ( s_bRebuildUnpackedMissionData )
		ClearUnpackedMissionData();

	Assert( s_UnpackedMissions.Count() == CountMissions() );
	if ( s_UnpackedMissions.Count() != CountMissions() )
		ClearUnpackedMissionData();

	if ( index < 0 || index >= CountMissions() )
	{
		Assert( index == -1 );
		return NULL;
	}

	RD_Mission_t *pMission = &s_UnpackedMissions[index];
	if ( pMission->BaseName[0] )
		return pMission;

	V_strncpy( pMission->BaseName, MissionName( index ), sizeof( pMission->BaseName ) );
	pMission->WorkshopID = MissionWorkshopID( index );

	KeyValues::AutoDelete pKV( "GAME" );
	if ( !ReadMissionData( pKV, index ) )
	{
		pMission->Material = MAKE_STRING( "tools/toolsblack" );
		pMission->BriefingMaterial = MAKE_STRING( "tools/toolsblack" );

		pMission->MissionTitle = AllocMissionsPooledString( pMission->BaseName );
		pMission->Description = MAKE_STRING( "" );
		pMission->Image = MAKE_STRING( "swarm/missionpics/addonmissionpic" );

		pMission->Author = MAKE_STRING( "" );
		pMission->Website = MAKE_STRING( "" );
		pMission->Version = MAKE_STRING( "" );

		UnpackNetworkedTags( pMission->Tags, g_StringTableReactiveDropMissions, index );

		return pMission;
	}

	pMission->Installed = true;

	pMission->PosX = pKV->GetInt( "pos_x" );
	pMission->PosY = pKV->GetInt( "pos_y" );
	pMission->Scale = pKV->GetFloat( "scale", 1.0f );

	pMission->Material = AllocMissionsPooledString( pKV->GetString( "material" ) );
	pMission->BriefingMaterial = AllocMissionsPooledString( pKV->GetString( "briefingmaterial" ) );

	pMission->MissionTitle = AllocMissionsPooledString( pKV->GetString( "missiontitle" ) );
	pMission->Description = AllocMissionsPooledString( pKV->GetString( "description" ) );
	pMission->Image = AllocMissionsPooledString( pKV->GetString( "image" ) );

	const CReactiveDropWorkshop::WorkshopItem_t & info = g_ReactiveDropWorkshop.TryQueryAddon( pMission->WorkshopID );

	const char *szDefaultCredits = DEFAULT_CREDITS_FILE;
	KeyValues::AutoDelete pCampaign( (KeyValues *)NULL );
	if ( info.bAdminOverrideBonus || info.bAdminOverrideDeathmatch )
	{
		UtlSymId_t sym = info.kvTags.Find( "campaign_mission" );
		if ( sym != info.kvTags.InvalidIndex() )
		{
			const CUtlStringList & campaignMissions = info.kvTags[sym];
			char szSuffix[MAX_PATH];
			V_snprintf( szSuffix, sizeof( szSuffix ), "/0/%s", pMission->BaseName );
			FOR_EACH_VEC( campaignMissions, i )
			{
				if ( const char *pCampaignNameEnd = V_stristr( campaignMissions[i], szSuffix ) )
				{
					V_strncpy( szSuffix, campaignMissions[i], pCampaignNameEnd - campaignMissions[i] );
					pCampaign.Assign( new KeyValues( "GAME" ) );
					if ( UTIL_RD_LoadKeyValuesFromFile( pCampaign, filesystem, CFmtStr( "resource/campaigns/%s.txt", szSuffix ), "GAME" ) )
					{
						szDefaultCredits = pCampaign->GetString( "CustomCreditsFile", szDefaultCredits );
					}
					break;
				}
			}
		}
	}

	pMission->CustomCreditsFile = AllocMissionsPooledString( pKV->GetString( "CustomCreditsFile", szDefaultCredits ) );

	pMission->Author = AllocMissionsPooledString( pKV->GetString( "author" ) );
	pMission->Website = AllocMissionsPooledString( pKV->GetString( "website" ) );
	pMission->Version = AllocMissionsPooledString( pKV->GetString( "version" ) );
	pMission->Builtin = pKV->GetBool( "builtin" );

	if ( KeyValues *pVerticalSections = pKV->FindKey( "verticalsections" ) )
	{
		FOR_EACH_TRUE_SUBKEY( pVerticalSections, pSection )
		{
			int iSection = pMission->VerticalSections.AddToTail();
			pMission->VerticalSections[iSection].Material = AllocMissionsPooledString( pSection->GetName() );
			pMission->VerticalSections[iSection].AltitudeMin = pSection->GetFloat( "AltitudeMin", MIN_COORD_FLOAT );
			pMission->VerticalSections[iSection].AltitudeMax = pSection->GetFloat( "AltitudeMax", MAX_COORD_FLOAT );
		}
	}

	FOR_EACH_VALUE( pKV, pValue )
	{
		if ( !V_stricmp( pValue->GetName(), "tag" ) )
		{
			pMission->Tags.AddToTail( AllocMissionsPooledString( pValue->GetString() ) );
		}
	}

	if ( info.bAdminOverrideDeathmatch && !V_strnicmp( pMission->BaseName, "dm_", 3 ) )
	{
		pMission->Tags.AddToTail( MAKE_STRING( "deathmatch" ) );
	}
	else if ( info.bAdminOverrideBonus )
	{
		pMission->Tags.AddToTail( MAKE_STRING( "bonus" ) );
	}

	return pMission;
}

PublishedFileId_t ReactiveDropMissions::CampaignWorkshopID( int index )
{
	if ( index < 0 || index >= CountCampaigns() )
	{
		Assert( index == -1 );
		return k_PublishedFileIdInvalid;
	}

#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		const char *pszCampaignName = s_ClientMissions.m_LocalCampaigns.String( index );

		char szPath[MAX_PATH];
		V_snprintf( szPath, sizeof( szPath ), "resource/campaigns/%s.txt", pszCampaignName );

		return g_ReactiveDropWorkshop.FindAddonProvidingFile( szPath );
	}
#endif

	Assert( g_StringTableReactiveDropCampaigns );

	int length;
	const void *pRawData = g_StringTableReactiveDropCampaigns->GetStringUserData( index, &length );
	Assert( length == 9 );
	const NetworkedMissionMetadata_t *pData = static_cast<const NetworkedMissionMetadata_t *>( pRawData );
	Assert( pData );
	return pData->WorkshopID;
}

PublishedFileId_t ReactiveDropMissions::MissionWorkshopID( int index )
{
	if ( index < 0 || index >= CountMissions() )
	{
		Assert( index == -1 );
		return k_PublishedFileIdInvalid;
	}

#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		const char *pszMissionName = s_ClientMissions.m_LocalMissions.String( index );

		char szPath[MAX_PATH];
		V_snprintf( szPath, sizeof( szPath ), "resource/overviews/%s.txt", pszMissionName );

		return g_ReactiveDropWorkshop.FindAddonProvidingFile( szPath );
	}
#endif

	Assert( g_StringTableReactiveDropMissions );

	int length;
	const void *pRawData = g_StringTableReactiveDropMissions->GetStringUserData( index, &length );
	Assert( length == 9 );
	const NetworkedMissionMetadata_t *pData = static_cast< const NetworkedMissionMetadata_t * >( pRawData );
	Assert( pData );
	return pData->WorkshopID;
}

int ReactiveDropMissions::GetCampaignIndex( const char *name )
{
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		s_ClientMissions.ReadMissionList();
		CUtlSymbol sym = s_ClientMissions.m_LocalCampaigns.Find( name );
		if ( sym.IsValid() )
			return sym;
		return -1;
	}
#endif

	Assert( g_StringTableReactiveDropCampaigns );
	if ( !g_StringTableReactiveDropCampaigns )
		return -1;

	int index = g_StringTableReactiveDropCampaigns->FindStringIndex( name );
	if ( index != INVALID_STRING_INDEX )
		return index;
	return -1;
}

int ReactiveDropMissions::GetMissionIndex( const char *name )
{
#ifdef CLIENT_DLL
	if ( !engine->IsInGame() )
	{
		s_ClientMissions.ReadMissionList();
		CUtlSymbol sym = s_ClientMissions.m_LocalMissions.Find( name );
		if ( sym.IsValid() )
			return sym;
		return -1;
	}
#endif

	Assert( g_StringTableReactiveDropMissions );
	if ( !g_StringTableReactiveDropMissions )
		return -1;

	int index = g_StringTableReactiveDropMissions->FindStringIndex( name );
	if ( index != INVALID_STRING_INDEX )
		return index;
	return -1;
}

const RD_Campaign_t *CampaignHandle::Get()
{
	if ( !s_bRebuildUnpackedMissionData && m_nDataResets == ReactiveDropMissions::s_nDataResets )
		return m_pCampaign;

	m_pCampaign = ReactiveDropMissions::GetCampaign( m_szBaseName );
	m_nDataResets = ReactiveDropMissions::s_nDataResets;

	return m_pCampaign;
}

void CampaignHandle::SetCampaign( const char *szBaseName )
{
	Assert( szBaseName );
	V_strncpy( m_szBaseName, szBaseName, sizeof( m_szBaseName ) );
	m_pCampaign = NULL;
	m_nDataResets = 0;
}

const RD_Campaign_Mission_t *CampaignMissionHandle::Get()
{
	const RD_Campaign_t *pCampaign = m_Campaign.Get();

	return pCampaign ? pCampaign->GetMission( m_iMission ) : NULL;
}

void CampaignMissionHandle::SetCampaignMission( const char *szBaseName, int iMission )
{
	Assert( szBaseName && iMission >= 0 );
	m_Campaign.SetCampaign( szBaseName );
	m_iMission = iMission;
}

const RD_Mission_t *MissionHandle::Get()
{
	if ( !s_bRebuildUnpackedMissionData && m_nDataResets == ReactiveDropMissions::s_nDataResets )
		return m_pMission;

	m_pMission = ReactiveDropMissions::GetMission( m_szBaseName );
	m_nDataResets = ReactiveDropMissions::s_nDataResets;

	return m_pMission;
}

void MissionHandle::SetMission( const char *szBaseName )
{
	Assert( szBaseName );
	V_strncpy( m_szBaseName, szBaseName, sizeof( m_szBaseName ) );
	m_pMission = NULL;
	m_nDataResets = 0;
}

bool RD_Campaign_t::HasTag( const char *tag ) const
{
	FOR_EACH_VEC( Tags, i )
	{
		if ( !V_stricmp( tag, STRING( Tags[i] ) ) )
		{
			return true;
		}
	}

	return false;
}

const RD_Campaign_Mission_t *RD_Campaign_t::GetMission( int iMissionIndex ) const
{
	if ( iMissionIndex >= 0 && iMissionIndex < Missions.Count() )
		return &Missions[iMissionIndex];

	return NULL;
}

const RD_Campaign_Mission_t *RD_Campaign_t::GetMissionByMapName( const char *szMapName ) const
{
	FOR_EACH_VEC( Missions, i )
	{
		if ( !V_stricmp( szMapName, Missions[i].MapName ) )
		{
			return &Missions[i];
		}
	}

	return NULL;
}

bool RD_Campaign_t::AreMissionsLinked( int from, int to ) const
{
	const RD_Campaign_Mission_t *pMission = GetMission( from );
	if ( !pMission )
		return false;

	FOR_EACH_VEC( pMission->Links, i )
	{
		if ( pMission->Links[i] == to )
		{
			return true;
		}
	}

	return false;
}

bool RD_Mission_t::HasTag( const char *tag ) const
{
	FOR_EACH_VEC( Tags, i )
	{
		if ( !V_stricmp( tag, STRING( Tags[i] ) ) )
		{
			return true;
		}
	}

	return false;
}

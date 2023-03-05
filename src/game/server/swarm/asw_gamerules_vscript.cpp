#include "cbase.h"
#include "asw_gamerules.h"
#include "asw_spawn_manager.h"
#include "asw_spawn_selection.h"
#include "asw_director.h"
#include "asw_grenade_cluster.h"
#include "asw_grenade_freeze.h"
#include "asw_weapon_healgrenade_shared.h"
#include "asw_buffgrenade_projectile.h"
#include "asw_laser_mine.h"
#include "asw_grenade_prifle.h"
#include "asw_grenade_vindicator.h"
#include "asw_rifle_grenade.h"
#include "asw_gas_grenade_projectile.h"
#include "asw_mine.h"
#include "asw_fire.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_game_resource.h"
#include "asw_campaign_save.h"
#include "rd_challenges_shared.h"
#include "rd_missions_shared.h"
#include "asw_achievements.h"
#include "asw_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar rd_challenge;

static ScriptVariant_t s_newArrayFunc;
static ScriptVariant_t s_arrayPushFunc;
static ScriptVariant_t s_arrayGetFunc;
static void CreateArray( ScriptVariant_t &array )
{
	if ( !g_pScriptVM ) return;
	Assert( array.m_type == FIELD_VOID );
	ScriptStatus_t status = g_pScriptVM->Call( s_newArrayFunc, NULL, false, &array );
	Assert( status == SCRIPT_DONE );
	( void )status;
}
static void ArrayPush( ScriptVariant_t &array, ScriptVariant_t item )
{
	if ( !g_pScriptVM ) return;
	ScriptStatus_t status = g_pScriptVM->Call( s_arrayPushFunc, NULL, false, &array, array, item );
	Assert( status == SCRIPT_DONE );
	( void )status;
}
static void ArrayGet( ScriptVariant_t &result, ScriptVariant_t array, ScriptVariant_t index )
{
	if ( !g_pScriptVM ) return;
	ScriptStatus_t status = g_pScriptVM->Call( s_arrayGetFunc, NULL, false, &result, array, index );
	Assert( status == SCRIPT_DONE );
	( void )status;
}

// BenLubar(key-values-director): this class provides the Director object for vscripts
class CASW_Director_VScript
{
public:
	HSCRIPT SpawnAlienAt( const char *szAlienClass, const Vector & vecPos, const Vector & vecAngle )
	{
		Assert( ASWSpawnManager() );
		if ( !ASWSpawnManager() )
		{
			return NULL;
		}

		QAngle angle( VectorExpand( vecAngle ) );
		CASW_Spawn_NPC npc( szAlienClass );

		return ToHScript( ASWSpawnManager()->SpawnAlienAt( &npc, vecPos, angle ) );
	}

	int SpawnAlienBatch( const char *szAlienClass, int iNumAliens, const Vector & vecPos, const Vector & vecAngle )
	{
		Assert( ASWSpawnManager() );
		if ( !ASWSpawnManager() )
		{
			return -1;
		}

		QAngle angle( VectorExpand( vecAngle ) );

		return ASWSpawnManager()->SpawnAlienBatch( szAlienClass, iNumAliens, vecPos, angle, 0.0f );
	}

	bool SpawnAlienAuto( const char *szAlienClass )
	{
		Assert( ASWSpawnManager() );
		if ( !ASWSpawnManager() )
		{
			return false;
		}

		return ASWSpawnManager()->ScriptSpawnAlienAtRandomNode( szAlienClass );
	}

	int GetAlienCount()
	{
		Assert( ASWSpawnManager() );
		if ( !ASWSpawnManager() )
		{
			return -1;
		}

		int iAlienCount = 0;
		for ( int i = 0; i < ASWSpawnManager()->GetNumAlienClasses(); i++ )
		{
			CBaseEntity *ent = NULL;
			while ( ( ent = gEntList.FindEntityByClassnameFast( ent, ASWSpawnManager()->GetAlienClass( i )->m_iszAlienClass ) ) != NULL )
			{
				iAlienCount++;
			}
		}

		return iAlienCount;
	}

	bool ValidSpawnPoint( const Vector vecPos, const Vector vecMins, const Vector vecMaxs )
	{
		Assert( ASWSpawnManager() );
		if ( !ASWSpawnManager() )
		{
			return false;
		}

		return ASWSpawnManager()->ValidSpawnPoint( vecPos, vecMins, vecMaxs, true, 0.0f );
	}

	bool IsOfflineGame()
	{
		CAlienSwarm* game = ASWGameRules();
		Assert( game );
		if ( !game )
		{
			return false;
		}

		return game->IsOfflineGame();
	}

	void RestartMission()
	{
		CAlienSwarm* game = ASWGameRules();
		Assert( game );
		if ( !game )
		{
			return;
		}

		game->RestartMission( NULL, true, true );
	}

	void MissionComplete( bool bSuccess )
	{
		CAlienSwarm* game = ASWGameRules();
		Assert( game );
		if ( !game )
		{
			return;
		}

		game->MissionComplete( bSuccess );
	}

	float GetIntensity( HSCRIPT hMarine )
	{
		CASW_Marine *pMarine = CASW_Marine::AsMarine( ToEnt( hMarine ) );
		CASW_Marine_Resource *pMR = pMarine ? pMarine->GetMarineResource() : NULL;
		if ( pMR && pMR->GetIntensity() )
		{
			return pMR->GetIntensity()->GetCurrent();
		}
		return 0;
	}

	float GetMaxIntensity()
	{
		Assert( ASWDirector() );
		if ( !ASWDirector() )
		{
			return 0;
		}

		return ASWDirector()->GetMaxIntensity();
	}

	void ResetIntensity( HSCRIPT hMarine )
	{
		CASW_Marine *pMarine = CASW_Marine::AsMarine( ToEnt( hMarine ) );
		CASW_Marine_Resource *pMR = pMarine ? pMarine->GetMarineResource() : NULL;
		if ( pMR && pMR->GetIntensity() )
		{
			pMR->GetIntensity()->Reset();
		}
	}

	void ResetIntensityForAllMarines()
	{
		Assert( ASWDirector() );
		if ( !ASWDirector() )
		{
			return;
		}

		ASWDirector()->ResetMarineIntensity();
	}

	void StartFinale()
	{
		Assert( ASWDirector() );
		if ( !ASWDirector() )
		{
			return;
		}

		ASWDirector()->StartFinale();
	}

	void StartHoldout()
	{
		Assert( ASWDirector() );
		if ( !ASWDirector() )
		{
			return;
		}

		ASWDirector()->StartHoldout();
	}

	void StopHoldout()
	{
		Assert( ASWDirector() );
		if ( !ASWDirector() )
		{
			return;
		}

		ASWDirector()->StopHoldout();
	}

	void SpawnHordeSoon()
	{
		Assert( ASWDirector() );
		if ( !ASWDirector() )
		{
			return;
		}

		ASWDirector()->SpawnHordeSoon();
	}

	float GetTimeToNextHorde()
	{
		Assert( ASWDirector() );
		if ( !ASWDirector() )
		{
			return -1;
		}

		return ASWDirector()->m_HordeTimer.GetRemainingTime();
	}

	void SetTimeToNextHorde( float flNumSeconds )
	{
		Assert( ASWDirector() );
		if ( !ASWDirector() )
		{
			return;
		}

		ASWDirector()->m_HordeTimer.Start( flNumSeconds );
	}

	ScriptVariant_t FindHordePosition( bool bNorth )
	{
		Vector position;
		QAngle discard;

		CASW_Spawn_Manager *pSpawnManager = ASWSpawnManager();
		Assert( pSpawnManager );
		if ( !pSpawnManager )
		{
			return SCRIPT_VARIANT_NULL;
		}

		pSpawnManager->UpdateCandidateNodes();
		if ( pSpawnManager->FindHordePos( bNorth, bNorth ? pSpawnManager->m_northCandidateNodes : pSpawnManager->m_southCandidateNodes, position, discard ) )
		{
			return ScriptVariant_t( position, true );
		}

		return SCRIPT_VARIANT_NULL;
	}

	int IsSpawningHorde()
	{
		Assert( ASWSpawnManager() );
		if ( !ASWSpawnManager() )
		{
			return 0;
		}

		return ASWSpawnManager()->GetHordeToSpawn();
	}
} g_ASWDirectorVScript;

BEGIN_SCRIPTDESC_ROOT_NAMED( CASW_Director_VScript, "CDirector", SCRIPT_SINGLETON "The AI director" )
	DEFINE_SCRIPTFUNC( SpawnAlienAt, "Spawn an alien by class name" )
	DEFINE_SCRIPTFUNC( SpawnAlienBatch, "Spawn a group of aliens by class name" )
	DEFINE_SCRIPTFUNC( SpawnAlienAuto, "Spawn an alien automatically near the marines" )
	DEFINE_SCRIPTFUNC( GetAlienCount, "Returns number of aliens currently spawned" )
	DEFINE_SCRIPTFUNC( ValidSpawnPoint, "Checks if the origin is a valid spawn point" )
	DEFINE_SCRIPTFUNC( IsOfflineGame, "Return true if game is in single player" )
	DEFINE_SCRIPTFUNC( RestartMission, "Restarts the mission" )
	DEFINE_SCRIPTFUNC( MissionComplete, "Completes the mission if true" )
	DEFINE_SCRIPTFUNC( GetIntensity, "Get the intensity value for a marine" )
	DEFINE_SCRIPTFUNC( GetMaxIntensity, "Get the maximum intensity value for all living marines" )
	DEFINE_SCRIPTFUNC( ResetIntensity, "Reset the intensity value for a marine to zero" )
	DEFINE_SCRIPTFUNC( ResetIntensityForAllMarines, "Reset the intensity value for all marines to zero" )
	DEFINE_SCRIPTFUNC( StartFinale, "Spawn a horde every few seconds for the rest of the level" )
	DEFINE_SCRIPTFUNC( StartHoldout, "Start spawning a horde every few seconds until stopped" )
	DEFINE_SCRIPTFUNC( StopHoldout, "Stop spawning hordes" )
	DEFINE_SCRIPTFUNC( SpawnHordeSoon, "Queue a horde to spawn soon (the same logic as when a hack starts)" )
	DEFINE_SCRIPTFUNC( GetTimeToNextHorde, "Get the current number of seconds until the director will try to spawn a horde" )
	DEFINE_SCRIPTFUNC( SetTimeToNextHorde, "Force the horde countdown timer to be set to this number of seconds" )
	DEFINE_SCRIPTFUNC( FindHordePosition, "Get a random position where a horde can spawn (returns null on fail)" )
	DEFINE_SCRIPTFUNC( IsSpawningHorde, "Get the number of aliens remaining to spawn in the current horde" )
END_SCRIPTDESC();

// Implemented based on the description from https://developer.valvesoftware.com/wiki/List_of_L4D2_Script_Functions#Convars
class CASW_Convars_VScript
{
public:
	ScriptVariant_t GetClientConvarValue( int clientIndex, const char *name )
	{
		const char *szValue = engine->GetClientConVarValue( clientIndex, name );
		if ( szValue )
		{
			return ScriptVariant_t( szValue, true );
		}
		return SCRIPT_VARIANT_NULL;
	}

	ScriptVariant_t GetStr( const char *name )
	{
		ConVarRef ref( name );
		if ( ref.IsValid() )
		{
			return ScriptVariant_t( ref.GetString(), true );
		}
		return SCRIPT_VARIANT_NULL;
	}

	ScriptVariant_t GetFloat( const char *name )
	{
		ConVarRef ref( name );
		if ( ref.IsValid() )
		{
			return ScriptVariant_t( ref.GetFloat() );
		}
		return SCRIPT_VARIANT_NULL;
	}

	void SetValue( const char *name, float value )
	{
		ConVarRef ref( name );
		if ( !ref.IsValid() )
		{
			return;
		}

		ASWGameRules()->SaveConvar( ref );
		
		ref.SetValue( value );
	}

	void SetValueString( const char *name, const char *value )
	{
		ConVarRef ref( name );
		if ( !ref.IsValid() )
		{
			return;
		}

		ASWGameRules()->SaveConvar( ref );
		
		ref.SetValue( value );
	}

	void ExecuteConCommand( const char *pszCommand )
	{
		CCommand cmd;
		cmd.Tokenize( pszCommand );

		ConCommand *command = cvar->FindCommand( cmd[0] );
		if ( command )
		{
			if ( !command->IsFlagSet( FCVAR_GAMEDLL ) )
				return;
			command->Dispatch( cmd );
		}
	}
} g_ASWConvarsVScript;

BEGIN_SCRIPTDESC_ROOT_NAMED( CASW_Convars_VScript, "Convars", SCRIPT_SINGLETON "Provides an interface for getting and setting convars on the server." )
	DEFINE_SCRIPTFUNC( GetClientConvarValue, "Returns the convar value for the entindex as a string. Only works with client convars with the FCVAR_USERINFO flag." )
	DEFINE_SCRIPTFUNC( GetStr, "Returns the convar as a string. May return null if no such convar." )
	DEFINE_SCRIPTFUNC( GetFloat, "Returns the convar as a float. May return null if no such convar." )
	DEFINE_SCRIPTFUNC( SetValue, "Sets the value of the convar to a numeric value." )
	DEFINE_SCRIPTFUNC( SetValueString, "Sets the value of the convar to a string." )
	DEFINE_SCRIPTFUNC( ExecuteConCommand, "Executes the convar command." )
END_SCRIPTDESC();

class CASW_Mission_Chooser_VScript
{
	void ChallengeToVScript( ScriptVariant_t &table, const char *szID, KeyValues *pKV )
	{
		if ( !g_pScriptVM ) return;

		g_pScriptVM->CreateTable( table );

		g_pScriptVM->SetValue( table, "id", szID );
		PublishedFileId_t iWorkshopID = ReactiveDropChallenges::WorkshopID( szID );
		g_pScriptVM->SetValue( table, "workshop", CFmtStr( "%llu", iWorkshopID ) );
		g_pScriptVM->SetValue( table, "has_script", filesystem->FileExists( CFmtStr( "scripts/vscripts/challenge_%s.nut", szID ), "GAME" ) );

		g_pScriptVM->SetValue( table, "name", pKV->GetString( "name" ) );
		g_pScriptVM->SetValue( table, "icon", pKV->GetString( "icon" ) );
		g_pScriptVM->SetValue( table, "description", pKV->GetString( "description" ) );
		g_pScriptVM->SetValue( table, "author", pKV->GetString( "author" ) );

		ScriptVariant_t convarTable;
		g_pScriptVM->CreateTable( convarTable );

		if ( KeyValues *pConVars = pKV->FindKey( "convars" ) )
		{
			FOR_EACH_VALUE( pConVars, pConVar )
			{
				g_pScriptVM->SetValue( convarTable, pConVar->GetName(), pConVar->GetString() );
			}
		}

		g_pScriptVM->SetValue( table, "convars", convarTable );
		g_pScriptVM->ReleaseValue( convarTable );
	}
	void CampaignToVScript( ScriptVariant_t &table, const RD_Campaign_t *pCampaign )
	{
		if ( !pCampaign )
		{
			table = SCRIPT_VARIANT_NULL;
			return;
		}

		if ( !g_pScriptVM ) return;

		g_pScriptVM->CreateTable( table );

		g_pScriptVM->SetValue( table, "id", pCampaign->BaseName );
		g_pScriptVM->SetValue( table, "workshop", CFmtStr( "%llu", pCampaign->WorkshopID ) );

		g_pScriptVM->SetValue( table, "name", STRING( pCampaign->CampaignName ) );
		g_pScriptVM->SetValue( table, "description", STRING( pCampaign->CampaignDescription ) );
		g_pScriptVM->SetValue( table, "credits", STRING( pCampaign->CustomCreditsFile ) );

		g_pScriptVM->SetValue( table, "icon", STRING( pCampaign->ChooseCampaignTexture ) );
		g_pScriptVM->SetValue( table, "material", STRING( pCampaign->CampaignTextureName ) );
		g_pScriptVM->SetValue( table, "material1", STRING( pCampaign->CampaignTextureLayer[0] ) );
		g_pScriptVM->SetValue( table, "material2", STRING( pCampaign->CampaignTextureLayer[1] ) );
		g_pScriptVM->SetValue( table, "material3", STRING( pCampaign->CampaignTextureLayer[2] ) );

		g_pScriptVM->SetValue( table, "x", pCampaign->GalaxyX );
		g_pScriptVM->SetValue( table, "y", pCampaign->GalaxyY );

		static_assert( ASW_NUM_SEARCH_LIGHTS == 4, "ASW_NUM_SEARCH_LIGHTS changed" );
		g_pScriptVM->SetValue( table, "search_light_x_1", pCampaign->SearchLightX[0] );
		g_pScriptVM->SetValue( table, "search_light_y_1", pCampaign->SearchLightY[0] );
		g_pScriptVM->SetValue( table, "search_light_angle_1", pCampaign->SearchLightAngle[0] );
		g_pScriptVM->SetValue( table, "search_light_x_2", pCampaign->SearchLightX[1] );
		g_pScriptVM->SetValue( table, "search_light_y_2", pCampaign->SearchLightY[1] );
		g_pScriptVM->SetValue( table, "search_light_angle_2", pCampaign->SearchLightAngle[1] );
		g_pScriptVM->SetValue( table, "search_light_x_3", pCampaign->SearchLightX[2] );
		g_pScriptVM->SetValue( table, "search_light_y_3", pCampaign->SearchLightY[2] );
		g_pScriptVM->SetValue( table, "search_light_angle_3", pCampaign->SearchLightAngle[2] );
		g_pScriptVM->SetValue( table, "search_light_x_4", pCampaign->SearchLightX[3] );
		g_pScriptVM->SetValue( table, "search_light_y_4", pCampaign->SearchLightY[3] );
		g_pScriptVM->SetValue( table, "search_light_angle_4", pCampaign->SearchLightAngle[3] );

		ScriptVariant_t missionsArray;
		CreateArray( missionsArray );

		FOR_EACH_VEC( pCampaign->Missions, i )
		{
			const RD_Campaign_Mission_t *pMission = &pCampaign->Missions[i];
			ScriptVariant_t missionTable;
			g_pScriptVM->CreateTable( missionTable );

			g_pScriptVM->SetValue( missionTable, "index", pMission->CampaignIndex );
			g_pScriptVM->SetValue( missionTable, "map", pMission->MapName );
			g_pScriptVM->SetValue( missionTable, "name", STRING( pMission->MissionName ) );
			g_pScriptVM->SetValue( missionTable, "location_description", STRING( pMission->LocationDescription ) );
			g_pScriptVM->SetValue( missionTable, "short_briefing", STRING( pMission->ShortBriefing ) );
			g_pScriptVM->SetValue( missionTable, "threat_string", STRING( pMission->ThreatString ) );
			g_pScriptVM->SetValue( missionTable, "x", pMission->LocationX );
			g_pScriptVM->SetValue( missionTable, "y", pMission->LocationY );
			g_pScriptVM->SetValue( missionTable, "difficulty_modifier", pMission->DifficultyModifier );
			g_pScriptVM->SetValue( missionTable, "always_visible", pMission->AlwaysVisible );
			g_pScriptVM->SetValue( missionTable, "needs_more_than_one_marine", pMission->NeedsMoreThanOneMarine );

			ScriptVariant_t linksArray;
			CreateArray( linksArray );
			FOR_EACH_VEC( pMission->Links, j )
			{
				ArrayPush( linksArray, pMission->Links[j] );
			}

			g_pScriptVM->SetValue( missionTable, "links", linksArray );
			g_pScriptVM->ReleaseValue( linksArray );

			ArrayPush( missionsArray, missionTable );
			g_pScriptVM->ReleaseValue( missionTable );
		}

		g_pScriptVM->SetValue( table, "missions", missionsArray );
		g_pScriptVM->ReleaseValue( missionsArray );

		ScriptVariant_t tagsArray;
		CreateArray( tagsArray );

		FOR_EACH_VEC( pCampaign->Tags, i )
		{
			ArrayPush( tagsArray, STRING( pCampaign->Tags[i] ) );
		}

		g_pScriptVM->SetValue( table, "tags", tagsArray );
		g_pScriptVM->ReleaseValue( tagsArray );
	}
	void MissionToVScript( ScriptVariant_t &table, const RD_Mission_t *pMission )
	{
		if ( !pMission )
		{
			table = SCRIPT_VARIANT_NULL;
			return;
		}

		if ( !g_pScriptVM ) return;

		g_pScriptVM->CreateTable( table );

		g_pScriptVM->SetValue( table, "id", pMission->BaseName );
		g_pScriptVM->SetValue( table, "workshop", CFmtStr( "%llu", pMission->WorkshopID ) );

		g_pScriptVM->SetValue( table, "pos_x", pMission->PosX );
		g_pScriptVM->SetValue( table, "pos_y", pMission->PosY );
		g_pScriptVM->SetValue( table, "scale", pMission->Scale );

		g_pScriptVM->SetValue( table, "material", STRING( pMission->Material ) );
		g_pScriptVM->SetValue( table, "briefing_material", STRING( pMission->BriefingMaterial ) );

		ScriptVariant_t verticalSectionsArray;
		CreateArray( verticalSectionsArray );

		FOR_EACH_VEC( pMission->VerticalSections, i )
		{
			ScriptVariant_t sectionTable;
			g_pScriptVM->CreateTable( sectionTable );

			g_pScriptVM->SetValue( sectionTable, "material", STRING( pMission->VerticalSections[i].Material ) );
			g_pScriptVM->SetValue( sectionTable, "altitude_min", pMission->VerticalSections[i].AltitudeMin );
			g_pScriptVM->SetValue( sectionTable, "altitude_max", pMission->VerticalSections[i].AltitudeMax );

			ArrayPush( verticalSectionsArray, sectionTable );
			g_pScriptVM->ReleaseValue( sectionTable );
		}

		g_pScriptVM->SetValue( table, "vertical_sections", verticalSectionsArray );
		g_pScriptVM->ReleaseValue( verticalSectionsArray );

		g_pScriptVM->SetValue( table, "name", STRING( pMission->MissionTitle ) );
		g_pScriptVM->SetValue( table, "description", STRING( pMission->Description ) );
		g_pScriptVM->SetValue( table, "icon", STRING( pMission->Image ) );
		g_pScriptVM->SetValue( table, "credits", STRING( pMission->CustomCreditsFile ) );

		g_pScriptVM->SetValue( table, "author", STRING( pMission->Author ) );
		g_pScriptVM->SetValue( table, "website", STRING( pMission->Website ) );
		g_pScriptVM->SetValue( table, "version", STRING( pMission->Version ) );
		// intentionally not including Builtin flag.

		ScriptVariant_t tagsArray;
		CreateArray( tagsArray );

		FOR_EACH_VEC( pMission->Tags, i )
		{
			ArrayPush( tagsArray, STRING( pMission->Tags[i] ) );
		}

		g_pScriptVM->SetValue( table, "tags", tagsArray );
		g_pScriptVM->ReleaseValue( tagsArray );
	}
public:
	int CountChallenges()
	{
		return ReactiveDropChallenges::Count();
	}
	int CountCampaigns()
	{
		return ReactiveDropMissions::CountCampaigns();
	}
	int CountMissions()
	{
		return ReactiveDropMissions::CountMissions();
	}
	ScriptVariant_t GetChallenge( int index )
	{
		ScriptVariant_t table;
		KeyValues::AutoDelete pKV( "CHALLENGE" );

		if ( ReactiveDropChallenges::ReadData( pKV, index ) )
		{
			ChallengeToVScript( table, ReactiveDropChallenges::Name( index ), pKV );
		}

		return table;
	}
	ScriptVariant_t GetChallengeByName( const char *szName )
	{
		ScriptVariant_t table;
		KeyValues::AutoDelete pKV( "CHALLENGE" );

		if ( ReactiveDropChallenges::ReadData( pKV, szName ) )
		{
			ChallengeToVScript( table, szName, pKV );
		}

		return table;
	}
	ScriptVariant_t GetCurrentChallenge()
	{
		ScriptVariant_t table;
		KeyValues::AutoDelete pKV( "CHALLENGE" );

		if ( ReactiveDropChallenges::ReadData( pKV, rd_challenge.GetString() ) )
		{
			ChallengeToVScript( table, rd_challenge.GetString(), pKV );
		}

		return table;
	}
	ScriptVariant_t GetCampaign( int index )
	{
		ScriptVariant_t table;
		CampaignToVScript( table, ReactiveDropMissions::GetCampaign( index ) );

		return table;
	}
	ScriptVariant_t GetCampaignByName( const char *szName )
	{
		ScriptVariant_t table;
		CampaignToVScript( table, ReactiveDropMissions::GetCampaign( szName ) );

		return table;
	}
	ScriptVariant_t GetCurrentCampaign()
	{
		ScriptVariant_t table;
		if ( CAlienSwarm *pGameRules = ASWGameRules() )
		{
			if ( CASW_Campaign_Save *pSave = pGameRules->IsCampaignGame() ? pGameRules->GetCampaignSave() : NULL )
			{
				CampaignToVScript( table, ReactiveDropMissions::GetCampaign( pSave->GetCampaignName() ) );
			}
		}

		return table;
	}
	ScriptVariant_t GetMission( int index )
	{
		ScriptVariant_t table;
		MissionToVScript( table, ReactiveDropMissions::GetMission( index ) );

		return table;
	}
	ScriptVariant_t GetMissionByName( const char *szName )
	{
		ScriptVariant_t table;
		MissionToVScript( table, ReactiveDropMissions::GetMission( szName ) );

		return table;
	}
	ScriptVariant_t GetCurrentMission()
	{
		ScriptVariant_t table;
		MissionToVScript( table, ReactiveDropMissions::GetMission( STRING( gpGlobals->mapname ) ) );

		return table;
	}
} g_ASWMissionChooserVScript;

BEGIN_SCRIPTDESC_ROOT_NAMED( CASW_Mission_Chooser_VScript, "MissionChooser", SCRIPT_SINGLETON "Provides an interface for querying campaigns, missions, and challenges on the server." )
	DEFINE_SCRIPTFUNC( CountChallenges, "Returns the number of installed challenges." )
	DEFINE_SCRIPTFUNC( CountCampaigns, "Returns the number of installed campaigns." )
	DEFINE_SCRIPTFUNC( CountMissions, "Returns the number of installed missions." )
	DEFINE_SCRIPTFUNC( GetChallenge, "Creates a table containing challenge data for a challenge by index." )
	DEFINE_SCRIPTFUNC( GetChallengeByName, "Creates a table containing challenge data for a challenge by name." )
	DEFINE_SCRIPTFUNC( GetCurrentChallenge, "Creates a table containing challenge data for a the current challenge." )
	DEFINE_SCRIPTFUNC( GetCampaign, "Creates a table containing campaign data for a campaign by index." )
	DEFINE_SCRIPTFUNC( GetCampaignByName, "Creates a table containing campaign data for a campaign by name." )
	DEFINE_SCRIPTFUNC( GetCurrentCampaign, "Creates a table containing campaign data for the current campaign." )
	DEFINE_SCRIPTFUNC( GetMission, "Creates a table containing mission overview data for a mission by index." )
	DEFINE_SCRIPTFUNC( GetMissionByName, "Creates a table containing mission overview data for a mission by name." )
	DEFINE_SCRIPTFUNC( GetCurrentMission, "Creates a table containing mission overview data for the current mission." )
END_SCRIPTDESC();

HSCRIPT Grenade_Cluster_Create_VScript( float flDamage, float flRadius, int iClusters, Vector position, Vector angles, Vector velocity, Vector angVelocity )
{
	CASW_Grenade_Cluster *pCluster = CASW_Grenade_Cluster::Cluster_Grenade_Create( flDamage, flRadius, iClusters, position, QAngle( VectorExpand( angles ) ), velocity, angVelocity, NULL, NULL );
	return ToHScript( pCluster );
}

HSCRIPT Script_DropFreezeGrenade( float flDamage, float flFreezeAmount, float flGrenadeRadius, const Vector origin )
{
	CASW_Grenade_Freeze *pFreeze = CASW_Grenade_Freeze::Freeze_Grenade_Create( flDamage, flFreezeAmount, flGrenadeRadius, 0, origin, QAngle(0,0,0), Vector(0,0,0), AngularImpulse(0,0,0), NULL, NULL );
	if ( pFreeze )
	{
		pFreeze->SetGravity( 2.0f );
		pFreeze->SetExplodeOnWorldContact( true );
	}
	return ToHScript( pFreeze );
}

HSCRIPT Script_PlaceHealBeacon( float flHealAmount, float flHealthPerSecond, float flInfestationCureAmount, float flDuration, float flGrenadeRadius, const Vector origin )
{
	CASW_AOEGrenade_Projectile *pHeal = CASW_HealGrenade_Projectile::Grenade_Projectile_Create( origin, QAngle(0,0,0), Vector(0,0,0), AngularImpulse(0,0,0), NULL, NULL, flHealthPerSecond, flInfestationCureAmount, flGrenadeRadius, flDuration, flHealAmount );
	return ToHScript( pHeal );
}

HSCRIPT Script_PlaceDamageAmplifier( float flDuration, float flGrenadeRadius, const Vector origin )
{
	CASW_BuffGrenade_Projectile *pBuff = CASW_BuffGrenade_Projectile::Grenade_Projectile_Create( origin, QAngle(0,0,0), Vector(0,0,0), AngularImpulse(0,0,0), NULL, NULL, flGrenadeRadius, flDuration );
	return ToHScript( pBuff );
}

HSCRIPT Script_PlantLaserMine( bool bFriendly, const Vector origin, const Vector angles )
{
	CASW_Laser_Mine *pMine = CASW_Laser_Mine::ASW_Laser_Mine_Create( origin, QAngle( VectorExpand( angles ) ), QAngle(0,0,0), NULL, NULL, bFriendly, NULL );
	return ToHScript( pMine );
}

HSCRIPT Script_PlantIncendiaryMine( const Vector origin, const Vector angles )
{
	CASW_Mine *pMine = CASW_Mine::ASW_Mine_Create( origin, QAngle( VectorExpand( angles ) ), Vector(0,0,0), AngularImpulse(0,0,0), NULL, NULL );
	return ToHScript( pMine );
}

HSCRIPT Script_DropStunGrenade( float flDamage, float fRadius, const Vector position )
{
	CASW_Grenade_PRifle *pGrenade = CASW_Grenade_PRifle::PRifle_Grenade_Create( flDamage, fRadius, position, QAngle(0,0,0), Vector(0,0,0), AngularImpulse(0,0,0), NULL, NULL );
	return ToHScript( pGrenade );
}

HSCRIPT Script_DropIncendiaryGrenade( float flDamage, float fRadius, const Vector position )
{
	CASW_Grenade_Vindicator *pGrenade = CASW_Grenade_Vindicator::Vindicator_Grenade_Create( flDamage, fRadius, position, QAngle(0,0,0), Vector(0,0,0), AngularImpulse(0,0,0), NULL, NULL );
	return ToHScript( pGrenade );
}

HSCRIPT Script_DropFragGrenade( float flDamage, float fRadius, const Vector position )
{
	CASW_Rifle_Grenade *pGrenade = CASW_Rifle_Grenade::Rifle_Grenade_Create( flDamage, fRadius, position, QAngle(0,0,0), Vector(0,0,0), AngularImpulse(0,0,0), NULL, NULL );
	return ToHScript( pGrenade );
}

HSCRIPT Script_DropGasGrenade( float flDamage, float fInterval, float fDuration, float fFuse, const Vector position )
{
	CASW_Gas_Grenade_Projectile *pGas_Grenade = CASW_Gas_Grenade_Projectile::Gas_Grenade_Projectile_Create( position, QAngle(0,0,0), Vector(0,0,0), AngularImpulse(0,0,0), NULL, NULL, flDamage, fInterval, fDuration, fFuse );
	return ToHScript( pGas_Grenade );
}

void ScriptStartStim( float flDuration )
{
	CAlienSwarm* game = ASWGameRules();
	Assert( game );
	if ( !game )
	{
		return;
	}

	game->StartStim( flDuration, NULL );
}

void ScriptStopStim()
{
	CAlienSwarm* game = ASWGameRules();
	Assert( game );
	if ( !game )
	{
		return;
	}

	game->StopStim();
}

HSCRIPT Script_StartFire( const Vector position, float duration, int flags )
{
	CFire *pFire = FireSystem_StartFire( position, 64, 4, duration, flags, NULL, FIRE_NATURAL, 0, NULL );
	return ToHScript( pFire );
}

void Script_CheckSpecialAchievementEligibility()
{
	CAlienSwarm *pAlienSwarm = ASWGameRules();
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( !pAlienSwarm || !pGameResource || pAlienSwarm->m_iMissionWorkshopID.Get() || pAlienSwarm->m_bChallengeActiveThisMission )
	{
		return;
	}

	if ( FStrEq( STRING( gpGlobals->mapname ), "rd-acc6_labruins" ) )
	{
		CBaseEntity *pScriptEnt = gEntList.FindEntityByName( NULL, "script_doorlogic" );
		if ( !pScriptEnt || !pScriptEnt->GetScriptScope() )
			return;

		ScriptVariant_t bBossBattleEnded, iHitCount;
		if ( !g_pScriptVM->GetValue( pScriptEnt->GetScriptScope(), "bBossBattleEnded", &bBossBattleEnded ) )
			return;
		if ( !g_pScriptVM->GetValue( pScriptEnt->GetScriptScope(), "iHitCount", &iHitCount ) )
			return;

		if ( bBossBattleEnded.Get<bool>() && iHitCount.Get<int>() == 3 )
		{
			for ( int iMR = 0; iMR < pGameResource->GetMaxMarineResources(); iMR++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( iMR );
				if ( !pMR )
					continue;

				if ( pMR->GetHealthPercent() > 0 && pMR->IsInhabited() && pMR->GetCommander() )
				{
					pMR->GetCommander()->AwardAchievement( ACHIEVEMENT_RD_ACC_MUONGEM_KILL );
				}
			}
		}
	}

	if ( FStrEq( STRING( gpGlobals->mapname ), "rd-ht-marine_academy" ) )
	{
		CBaseEntity *pRulesProxy = gEntList.FindEntityByClassname( NULL, "asw_gamerules" );
		if ( !pRulesProxy || !pRulesProxy->GetScriptScope() )
			return;

		ScriptVariant_t element, CurrentArea, AreaVisitCount;
		if ( !g_pScriptVM->GetValue( pRulesProxy->GetScriptScope(), "g_current_area", &CurrentArea ) )
			return;
		if ( !g_pScriptVM->GetValue( pRulesProxy->GetScriptScope(), "g_area_scores_t", &AreaVisitCount ) )
			return;

		// entering volcano
		if ( CurrentArea.Get<int>() == 6 )
		{
			for ( int i = 1; i < 6; i++ )
			{
				ArrayGet( element, AreaVisitCount, i );

				if ( element.Get<int>() != 1 )
				{
					return;
				}
			}

			for ( int iMR = 0; iMR < pGameResource->GetMaxMarineResources(); iMR++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( iMR );
				if ( !pMR )
					continue;

				if ( pMR->GetHealthPercent() > 0 && pMR->IsInhabited() && pMR->GetCommander() )
				{
					pMR->GetCommander()->AwardAchievement( ACHIEVEMENT_RD_MA_VISIT_EACH_ZONE );
				}
			}

			return;
		}

		// entering lobby
		if ( CurrentArea.Get<int>() == 0 )
		{
			ArrayGet( element, AreaVisitCount, 6 );
			if ( element.Get<int>() != 1 )
			{
				return;
			}

			for ( int iMR = 0; iMR < pGameResource->GetMaxMarineResources(); iMR++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( iMR );
				if ( !pMR )
					continue;

				if ( pMR->GetHealthPercent() <= 0 )
					return;
			}


			for ( int iMR = 0; iMR < pGameResource->GetMaxMarineResources(); iMR++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( iMR );
				if ( !pMR )
					continue;

				if ( pMR->IsInhabited() && pMR->GetCommander() )
				{
					pMR->GetCommander()->AwardAchievement( ACHIEVEMENT_RD_MA_REACH_VOLCANO_ALIVE );
				}
			}

			return;
		}

		return;
	}
}

const Vector &Script_GetHullMins( int hull )
{
	if ( hull < 0 || hull > ( NUM_HULLS - 1 ) )
		return vec3_origin;

	return NAI_Hull::Mins( hull );
}

const Vector &Script_GetHullMaxs( int hull )
{
	if ( hull < 0 || hull > ( NUM_HULLS - 1 ) )
		return vec3_origin;

	return NAI_Hull::Maxs( hull );
}

HSCRIPT Script_FindNearestNPC( const Vector& vecOrigin, bool bCheckZ, float flRadius )
{
	float flNearest = FLT_MAX;
	CAI_BaseNPC* pNearestNPC = NULL;

	if ( flRadius > 23171 ) //ceil( sqrt(16384^2 + 16384^2) ) max reasonable radius due to hammer.
		flRadius = -1;

	CAI_BaseNPC** ppAIs = g_AI_Manager.AccessAIs();
	int nAIs = g_AI_Manager.NumAIs();

	for (int i = 0; i < nAIs; i++)
	{
		CAI_BaseNPC* pNPC = ppAIs[i];

		if ( pNPC->IsAlive() )
		{
			// Ignore hidden objects
			if ( pNPC->IsEffectActive( EF_NODRAW ) )
				continue;

			//Ignore invisible objects
			if ( pNPC->GetRenderMode() == kRenderNone )
				continue;

			// Don't bother with NPC's that are below me.
			if ( bCheckZ && ( pNPC->EyePosition().z + 4.0f ) < vecOrigin.z )
				continue;

			// Disregard things that want to be disregarded
			if ( pNPC->Classify() == CLASS_NONE )
				continue;

			// Disregard bullseyes
			if ( pNPC->Classify() == CLASS_BULLSEYE )
				continue;

			// Ignore marines
			if ( pNPC->Classify() == CLASS_ASW_MARINE || pNPC->Classify() == CLASS_ASW_COLONIST )
				continue;

			float flDist = ( vecOrigin - pNPC->GetAbsOrigin() ).LengthSqr();

			if ( flDist < flNearest && ( flRadius < 0 || ( flRadius > 0 && flDist <= flRadius * flRadius ) ) )
			{
				flNearest = flDist;
				pNearestNPC = pNPC;
			}
		}
	}
	return ToHScript( pNearestNPC );
}

void Script_GamePause( bool bPause )
{
	if ( bPause != engine->IsPaused() )
	{
		engine->Pause( bPause, true );
	}
}

bool Script_IsAnniversaryWeek()
{
	CAlienSwarm *pGameRules = ASWGameRules();
	Assert( pGameRules );
	return pGameRules ? pGameRules->IsAnniversaryWeek() : false;
}

void CAlienSwarm::RegisterScriptFunctions()
{
	if ( !g_pScriptVM ) return;

	// The VScript API doesn't support arrays, but Squirrel does, and we want to use them. Make some helper functions that we can call later:
	HSCRIPT arrayHelperScript = g_pScriptVM->CompileScript( "function newArray() { return [] }\nfunction arrayPush(a, v) { a.push(v); return a }\nfunction arrayGet(a, i) { return a[i] }\n" );
	HSCRIPT arrayHelperScope = g_pScriptVM->CreateScope( "arrayhelpers" );
	ScriptStatus_t status = g_pScriptVM->Run( arrayHelperScript, arrayHelperScope, false );
	Assert( status == SCRIPT_DONE );
	( void )status;
	g_pScriptVM->ReleaseScript( arrayHelperScript );
	bool bSuccess = g_pScriptVM->GetValue( arrayHelperScope, "newArray", &s_newArrayFunc );
	Assert( bSuccess );
	( void )bSuccess;
	bSuccess = g_pScriptVM->GetValue( arrayHelperScope, "arrayPush", &s_arrayPushFunc );
	Assert( bSuccess );
	( void )bSuccess;
	bSuccess = g_pScriptVM->GetValue( arrayHelperScope, "arrayGet", &s_arrayGetFunc );
	Assert( bSuccess );
	( void )bSuccess;
	g_pScriptVM->ReleaseScope( arrayHelperScope );

	g_pScriptVM->RegisterInstance( &g_ASWDirectorVScript, "Director" );
	g_pScriptVM->RegisterInstance( &g_ASWConvarsVScript, "Convars" );
	g_pScriptVM->RegisterInstance( &g_ASWMissionChooserVScript, "MissionChooser" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Grenade_Cluster_Create_VScript, "CreateGrenadeCluster", "create grenade cluster (damage, radius, count, position, angles, velocity, angularVelocity)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_DropFreezeGrenade, "DropFreezeGrenade", "Drops a freeze grenade (damage, freezeAmount, radius, position)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_PlaceHealBeacon, "PlaceHealBeacon", "Places a heal beacon (healAmount, healthPerSecond, infestationCureAmount, duration, radius, position)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_PlaceDamageAmplifier, "PlaceDamageAmplifier", "Places a damage amplifier (duration, radius, position)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_PlantLaserMine, "PlantLaserMine", "Plants a laser mine (friendly, position, angles)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_PlantIncendiaryMine, "PlantIncendiaryMine", "Plants an incendiary mine (position, angles)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_DropStunGrenade, "DropStunGrenade", "Drops a stun grenade (damage, radius, position)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_DropIncendiaryGrenade, "DropIncendiaryGrenade", "Drops an incendiary grenade (damage, radius, position)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_DropFragGrenade, "DropFragGrenade", "Drops a frag grenade (damage, radius, position)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_DropGasGrenade, "DropGasGrenade", "Drops a gas grenade (damage, dmgInterval, duration, fuse, position)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptStartStim, "StartStim", "Activates a stim pack for desired duration" );
	ScriptRegisterFunctionNamed( g_pScriptVM, ScriptStopStim, "StopStim", "Stops any active stim pack" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_StartFire, "StartFire", "Starts a fire (position, duration, flags)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_CheckSpecialAchievementEligibility, "CheckSpecialAchievementEligibility", "Internal function for official missions with complex mid-mission achievement requirements" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetHullMins, "GetHullMins", "Returns a Vector for the hull mins (hullType)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_GetHullMaxs, "GetHullMaxs", "Returns a Vector for the hull maxs (hullType)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_FindNearestNPC, "FindNearestNPC", "Find nearest NPC to position (position, ZcoordCheck, radius)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_GamePause, "GamePause", "Pause or unpause the game" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_IsAnniversaryWeek, "IsAnniversaryWeek", "Returns true if it is the anniversary week of Alien Swarm: Reactive Drop" );
}

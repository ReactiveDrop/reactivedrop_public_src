#include "cbase.h"
#include "asw_gamerules.h"
#include "asw_spawn_manager.h"
#include "asw_director.h"
#include "asw_grenade_cluster.h"
#include "asw_grenade_freeze.h"
#include "asw_weapon_healgrenade_shared.h"
#include "asw_buffgrenade_projectile.h"
#include "asw_laser_mine.h"
#include "asw_mine.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_game_resource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

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

		return ToHScript( ASWSpawnManager()->SpawnAlienAt( szAlienClass, vecPos, angle ) );
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
		Assert( ASWGameResource() );
		if ( !ASWGameResource() )
		{
			return;
		}

		for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
		{
			CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
			if ( pMR && pMR->GetIntensity() )
			{
				pMR->GetIntensity()->Reset();
			}
		}
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
END_SCRIPTDESC();

// Implemented based on the description from https://developer.valvesoftware.com/wiki/List_of_L4D2_Script_Functions#Convars
class CASW_Convars_VScript
{
public:
	ScriptVariant_t GetClientConvarValue( int clientIndex, const char *name )
	{
		const char *cvar = engine->GetClientConVarValue( clientIndex, name );
		if ( cvar )
		{
			return ScriptVariant_t( cvar, true );
		}
		return SCRIPT_VARIANT_NULL;
	}

	ScriptVariant_t GetStr( const char *name )
	{
		ConVarRef cvar( name );
		if ( cvar.IsValid() )
		{
			return ScriptVariant_t( cvar.GetString(), true );
		}
		return SCRIPT_VARIANT_NULL;
	}

	ScriptVariant_t GetFloat( const char *name )
	{
		ConVarRef cvar( name );
		if ( cvar.IsValid() )
		{
			return ScriptVariant_t( cvar.GetFloat() );
		}
		return SCRIPT_VARIANT_NULL;
	}

	void SetValue( const char *name, float value )
	{
		ConVarRef cvar( name );
		if ( !cvar.IsValid() )
		{
			return;
		}

		ASWGameRules()->SaveConvar( cvar );
		
		cvar.SetValue( value );
	}

	void SetValueString( const char *name, const char *value )
	{
		ConVarRef cvar( name );
		if ( !cvar.IsValid() )
		{
			return;
		}

		ASWGameRules()->SaveConvar( cvar );
		
		cvar.SetValue( value );
	}

	void ExecuteConCommand( const char *pszCommand )
	{
		CUtlStringList szCommandList;
		CCommand cmd;
		cmd.Tokenize( pszCommand );
		V_SplitString( pszCommand, " ", szCommandList );

		ConCommand *command = (ConCommand *)cvar->FindCommand( szCommandList[0] );
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
	CASW_AOEGrenade_Projectile *pHeal = CASW_HealGrenade_Projectile::Grenade_Projectile_Create( origin, QAngle(0,0,0), Vector(0,0,0), AngularImpulse(0,0,0), NULL, flHealthPerSecond, flInfestationCureAmount, flGrenadeRadius, flDuration, flHealAmount );
	return ToHScript( pHeal );
}

HSCRIPT Script_PlaceDamageAmplifier( float flDuration, float flGrenadeRadius, const Vector origin )
{
	CASW_BuffGrenade_Projectile *pBuff = CASW_BuffGrenade_Projectile::Grenade_Projectile_Create( origin, QAngle(0,0,0), Vector(0,0,0), AngularImpulse(0,0,0), NULL, flGrenadeRadius, flDuration );
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

void CAlienSwarm::RegisterScriptFunctions()
{
	g_pScriptVM->RegisterInstance( &g_ASWDirectorVScript, "Director" );
	g_pScriptVM->RegisterInstance( &g_ASWConvarsVScript, "Convars" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Grenade_Cluster_Create_VScript, "CreateGrenadeCluster", "create grenade cluster (damage, radius, count, position, angles, velocity, angularVelocity)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_DropFreezeGrenade, "DropFreezeGrenade", "Drops a freeze grenade (damage, freezeAmount, radius, position)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_PlaceHealBeacon, "PlaceHealBeacon", "Places a heal beacon (healAmount, healthPerSecond, infestationCureAmount, duration, radius, position)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_PlaceDamageAmplifier, "PlaceDamageAmplifier", "Places a damage amplifier (duration, radius, position)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_PlantLaserMine, "PlantLaserMine", "Plants a laser mine (friendly, position, angles)" );
	ScriptRegisterFunctionNamed( g_pScriptVM, Script_PlantIncendiaryMine, "PlantIncendiaryMine", "Plants an incendiary mine (position, angles)" );
}

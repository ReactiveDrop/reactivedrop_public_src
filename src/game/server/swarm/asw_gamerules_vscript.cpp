#include "cbase.h"
#include "asw_gamerules.h"
#include "asw_spawn_manager.h"
#include "asw_director.h"

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
} g_ASWDirectorVScript;

BEGIN_SCRIPTDESC_ROOT_NAMED( CASW_Director_VScript, "CDirector", SCRIPT_SINGLETON "The AI director" )
	DEFINE_SCRIPTFUNC( SpawnAlienAt, "Spawn an alien by class name" )
END_SCRIPTDESC();

// Implemented based on the description from https://developer.valvesoftware.com/wiki/List_of_L4D2_Script_Functions#Convars
class CASW_Convars_VScript
{
public:
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
} g_ASWConvarsVScript;

BEGIN_SCRIPTDESC_ROOT_NAMED( CASW_Convars_VScript, "Convars", SCRIPT_SINGLETON "Provides an interface for getting and setting convars on the server." )
	// DEFINE_SCRIPTFUNC( GetClientConvarValue, "Returns the convar value for the entindex as a string. Only works with client convars with the FCVAR_USERINFO flag." )
	DEFINE_SCRIPTFUNC( GetStr, "Returns the convar as a string. May return null if no such convar." )
	DEFINE_SCRIPTFUNC( GetFloat, "Returns the convar as a float. May return null if no such convar." )
	DEFINE_SCRIPTFUNC( SetValue, "Sets the value of the convar to a numeric value." )
	DEFINE_SCRIPTFUNC( SetValueString, "Sets the value of the convar to a string." )
END_SCRIPTDESC();

void CAlienSwarm::RegisterScriptFunctions()
{
	g_pScriptVM->RegisterInstance( &g_ASWDirectorVScript, "Director" );
	g_pScriptVM->RegisterInstance( &g_ASWConvarsVScript, "Convars" );
}

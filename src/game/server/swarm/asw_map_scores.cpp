#include "cbase.h"
#include <KeyValues.h>
#include <filesystem.h>
#include "asw_map_scores.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_Map_Scores *g_pMapScores = NULL;

CASW_Map_Scores *MapScores()
{
	if ( !g_pMapScores )
	{
		g_pMapScores = new CASW_Map_Scores();
	}
	return g_pMapScores;
}

CASW_Map_Scores::CASW_Map_Scores()
{
	m_pScoreKeyValues = NULL;
	LoadMapScores();
}

void CASW_Map_Scores::LoadMapScores()
{
	if ( m_pScoreKeyValues )
	{
		Msg( "Error: Attempted to load CASW_Map_Scores twice\n" );
		return;
	}
	m_pScoreKeyValues = new KeyValues( "MAPSCORES" );
	if ( !UTIL_RD_LoadKeyValuesFromFile( m_pScoreKeyValues, filesystem, "cfg/mapscores.dat" ) )
	{
		// if we get here, it means the player has no mapscores.dat file and therefore no scores
		// (but that's fine)
		return;
	}
}

bool CASW_Map_Scores::SaveMapScores()
{
	if ( !m_pScoreKeyValues )
	{
		Msg( "Error: Attempted to save CASW_Map_Scores without loading first\n" );
		return false;
	}

	return ( m_pScoreKeyValues->SaveToFile( filesystem, "cfg/mapscores.dat" ) );
}

// adds an entry for a particular map
void CASW_Map_Scores::OnMapCompleted( const char *szMapName, int iSkill, float fTime, int iKills )
{
	if ( !m_pScoreKeyValues )
	{
		Msg( "Error:  Attempted to save map stats without creating keyvalues\n" );
		return;
	}
	// see if we have a subkey for this map
	KeyValues *pkvMap = GetSubkeyForMap( szMapName );
	if ( !pkvMap )
	{
		// create one
		pkvMap = new KeyValues( "MAPENTRY" );
		m_pScoreKeyValues->AddSubKey( pkvMap );
	}

	// grab the current map data
	float fBestTime = pkvMap->GetFloat( GetBestTimeKeyName( iSkill ) );
	int iBestKills = pkvMap->GetInt( GetBestKillsKeyName( iSkill ) );

	// update it if we did better
	fBestTime = MIN( fBestTime, fTime );
	if ( fBestTime <= 0 )
		fBestTime = fTime;
	iBestKills = MAX( iBestKills, iKills );

	// set properties of the subkey
	pkvMap->SetString( "MapName", szMapName );
	pkvMap->SetFloat( GetBestTimeKeyName( iSkill ), fBestTime );
	pkvMap->SetInt( GetBestKillsKeyName( iSkill ), iBestKills );

	// save new entry
	SaveMapScores();
}

float CASW_Map_Scores::GetBestTime( const char *szMapName, int iSkill )
{
	KeyValues *pkvMap = GetSubkeyForMap( szMapName );
	if ( !pkvMap )
		return 0;

	return pkvMap->GetFloat( GetBestTimeKeyName( iSkill ) );
}

int CASW_Map_Scores::GetBestKills( const char *szMapName, int iSkill )
{
	KeyValues *pkvMap = GetSubkeyForMap( szMapName );
	if ( !pkvMap )
		return 0;

	return pkvMap->GetInt( GetBestKillsKeyName( iSkill ) );
}

KeyValues *CASW_Map_Scores::GetSubkeyForMap( const char *szMapName )
{
	// now go through each mission section, adding it
	KeyValues *pkvMap = m_pScoreKeyValues->GetFirstSubKey();
	while ( pkvMap )
	{
		if ( Q_stricmp( pkvMap->GetName(), "MAPENTRY" ) == 0 )
		{
			const char *szName = pkvMap->GetString( "MapName" );
			if ( !Q_stricmp( szName, szMapName ) )
				return pkvMap;
		}
		pkvMap = pkvMap->GetNextKey();
	}
	return NULL;
}

const char *CASW_Map_Scores::GetBestKillsKeyName( int iSkill )
{
	switch ( iSkill )
	{
	case 1: return "BestKillsEasy"; break;
	case 3: return "BestKillsHard"; break;
	case 4: return "BestKillsInsane"; break;
	case 5: return "BestKillsImba"; break;
	default: break;
	}
	return "BestKillsNormal";
}

const char *CASW_Map_Scores::GetBestTimeKeyName( int iSkill )
{
	switch ( iSkill )
	{
	case 1: return "BestTimeEasy"; break;
	case 3: return "BestTimeHard"; break;
	case 4: return "BestTimeInsane"; break;
	case 5: return "BestTimeImba"; break;
	default: break;
	}
	return "BestTimeNormal";
}

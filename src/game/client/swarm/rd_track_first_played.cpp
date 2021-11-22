#include "cbase.h"
#include "steam/steam_api.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifdef REACTIVEDROP_TRACKFIRSTPLAYED	// this is obsolete

class CReactiveDrop_TrackFirstPlayed : public CAutoGameSystem
{
public:
	CReactiveDrop_TrackFirstPlayed() : CAutoGameSystem( "CReactiveDrop_TrackFirstPlayed" )
	{
	}

	virtual bool Init()
	{
		if ( !steamapicontext || !steamapicontext->SteamUserStats() || !steamapicontext->SteamUtils() )
		{
			Warning( "No Steam API context for tracking first played!\n" );
			return true;
		}

		SteamAPICall_t hCall = steamapicontext->SteamUserStats()->FindLeaderboard( "RD_First_Played" );
		m_Find.Set( hCall, this, &CReactiveDrop_TrackFirstPlayed::Find );

		return true;
	}

	CCallResult<CReactiveDrop_TrackFirstPlayed, LeaderboardFindResult_t> m_Find;
	void Find( LeaderboardFindResult_t *pResult, bool bIOError )
	{
		if ( bIOError )
		{
			Warning( "Failed to track first played time: IO error\n" );
			return;
		}

		if ( !pResult->m_bLeaderboardFound )
		{
			Warning( "Failed to track first played time: leaderboard not found\n" );
			return;
		}

		SteamAPICall_t hCall = steamapicontext->SteamUserStats()->UploadLeaderboardScore( pResult->m_hSteamLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, steamapicontext->SteamUtils()->GetServerRealTime(), NULL, 0 );
		m_Uploaded.Set( hCall, this, &CReactiveDrop_TrackFirstPlayed::Uploaded );
	}

	CCallResult<CReactiveDrop_TrackFirstPlayed, LeaderboardScoreUploaded_t> m_Uploaded;
	void Uploaded( LeaderboardScoreUploaded_t *pResult, bool bIOError )
	{
		if ( bIOError )
		{
			Warning( "Failed to track first played time: IO error during upload\n" );
			return;
		}

		if ( !pResult->m_bSuccess )
		{
			Warning( "Failed to track first played time\n" );
			return;
		}

		if ( pResult->m_bScoreChanged )
		{
			// TODO: remove this after all testers have started the game once we release
			steamapicontext->SteamUserStats()->ResetAllStats( true );
		}
	}
};

CReactiveDrop_TrackFirstPlayed g_ReactiveDrop_TrackFirstPlayed;

#endif 

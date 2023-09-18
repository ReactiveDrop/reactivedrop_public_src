#include "cbase.h"
#include "asw_jukebox.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_jukebox, CASW_Jukebox );

BEGIN_DATADESC( CASW_Jukebox )
	DEFINE_KEYFIELD( m_fFadeInTime, FIELD_FLOAT, "FadeInTime" ),
	DEFINE_KEYFIELD( m_fFadeOutTime, FIELD_FLOAT, "FadeOutTime" ),
	DEFINE_KEYFIELD( m_szDefaultMusic, FIELD_SOUNDNAME, "DefaultMusic" ),
	DEFINE_KEYFIELD( m_szTrackName, FIELD_STRING, "TrackName" ),
	DEFINE_KEYFIELD( m_szAlbumName, FIELD_STRING, "AlbumName" ),
	DEFINE_KEYFIELD( m_szArtistName, FIELD_STRING, "ArtistName" ),
	DEFINE_KEYFIELD( m_bInterruptCustomTrack, FIELD_BOOLEAN, "InterruptCustomTrack" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "StartMusic", InputMusicStart ),
	DEFINE_INPUTFUNC( FIELD_VOID, "StopMusic", InputMusicStop ),
END_DATADESC()

void CASW_Jukebox::Precache()
{
	BaseClass::Precache();

	if ( m_szDefaultMusic != NULL_STRING )
	{
		PrecacheScriptSound( STRING( m_szDefaultMusic ) );
	}
}

void CASW_Jukebox::InputMusicStart( inputdata_t &inputdata )
{
	// Send each client the music start event
	IGameEvent *event = gameeventmanager->CreateEvent( "jukebox_play_random" );
	if ( event )
	{
		event->SetFloat( "fadeintime", m_fFadeInTime );
		if ( m_szDefaultMusic != NULL_STRING )
			event->SetString( "defaulttrack", STRING( m_szDefaultMusic ) );
		if ( m_szTrackName != NULL_STRING )
			event->SetString( "trackname", STRING( m_szTrackName ) );
		if ( m_szAlbumName != NULL_STRING )
			event->SetString( "albumname", STRING( m_szAlbumName ) );
		if ( m_szArtistName != NULL_STRING )
			event->SetString( "artistname", STRING( m_szArtistName ) );
		event->SetBool( "interruptcustom", m_bInterruptCustomTrack );
		gameeventmanager->FireEvent( event );

		// Stop stim music if it's playing
		ASWGameRules()->m_fPreventStimMusicTime = 5.0f;
	}
}

void CASW_Jukebox::InputMusicStop( inputdata_t &inputdata )
{
	// Tell each client to stop the music
	IGameEvent *event = gameeventmanager->CreateEvent( "jukebox_stop" );
	if ( event )
	{
		event->SetFloat( "fadeouttime", m_fFadeOutTime );
		gameeventmanager->FireEvent( event );
	}
}

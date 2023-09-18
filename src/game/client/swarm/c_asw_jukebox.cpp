#include "cbase.h"
#include "c_asw_jukebox.h"
#include "soundenvelope.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "fmtstr.h"
#include "asw_util_shared.h"
#include "rd_hud_now_playing.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PLAYLIST_FILENAME "scripts/jukebox_playlist.txt"

CASWJukeboxPlaylist g_ASWJukebox;

TrackInfo_t::TrackInfo_t()
{
}

TrackInfo_t::TrackInfo_t( const wchar_t *wszTrackName, const char *szHexname, const wchar_t *wszAlbum, const wchar_t *wszArtist )
{
	V_wcsncpy( m_wszTrackName, wszTrackName, sizeof( m_wszTrackName ) );
	V_strncpy( m_szHexname, szHexname, sizeof( m_szHexname ) );
	V_wcsncpy( m_wszAlbum, wszAlbum, sizeof( m_wszAlbum ) );
	V_wcsncpy( m_wszArtist, wszArtist, sizeof( m_wszArtist ) );
}

void TrackInfo_t::PrepareKVForListView( KeyValues *kv )
{
	// add the file to the list
	kv->SetWString( "text", m_wszTrackName );
	kv->SetWString( "artist", m_wszArtist );
	kv->SetWString( "album", m_wszAlbum );
}

bool TrackInfo_t::operator==( const TrackInfo_t& rhs ) const
{
	return FStrEq( m_szHexname, rhs.m_szHexname );
}

void CASWJukeboxPlaylist::AddMusicToPlaylist( const wchar_t *wszTrackName, const char *szHexname, const wchar_t *wszAlbum, const wchar_t *wszArtist )
{
	FOR_EACH_VEC( m_CombatMusicPlaylist, i )
	{
		if ( FStrEq( m_CombatMusicPlaylist[i].m_szHexname, szHexname ) )
			return;
	}
	
	m_CombatMusicPlaylist.AddToTail( TrackInfo_t( wszTrackName, szHexname, wszAlbum, wszArtist ) );
	enginesound->PrecacheSound( CFmtStr{ "*#music/_mp3/%s.mp3", szHexname } );
}

void CASWJukeboxPlaylist::LoadPlaylistKV()
{
	KeyValues::AutoDelete pKV{ "playlist" };
	if ( UTIL_RD_LoadKeyValuesFromFile( pKV, g_pFullFileSystem, PLAYLIST_FILENAME, "MOD" ) )
	{
		// If the load succeeded, create the playlist
		FOR_EACH_TRUE_SUBKEY( pKV, sub )
		{
			const wchar_t *wszTrackName = sub->GetWString( "TrackName" );
			const char *szHexName = sub->GetString( "HexName" );
			const wchar_t *wszAlbum = sub->GetWString( "Album" );
			const wchar_t *wszArtist = sub->GetWString( "Artist" );
			AddMusicToPlaylist( wszTrackName, szHexName, wszAlbum, wszArtist );
		}
	}
}

bool CASWJukeboxPlaylist::Init()
{
	m_pCombatMusic = NULL;
	m_iCurrentTrack = -1;

	// Load the saved playlist
	LoadPlaylistKV();

	ListenForGameEvent( "jukebox_play_random" );
	ListenForGameEvent( "jukebox_stop" );

	return true;
}

void CASWJukeboxPlaylist::Shutdown()
{
	StopListeningForAllEvents();
}

void CASWJukeboxPlaylist::FireGameEvent( IGameEvent *event )
{
	if ( !event )
		return;

	if( FStrEq( event->GetName(), "jukebox_play_random") )
	{
		// Play some random music
		float fadeInTime = event->GetFloat( "fadeintime" );
		const char *szDefaultTrack = event->GetString( "defaulttrack" );
		const char *szTrackName = event->GetString( "trackname" );
		const char *szAlbumName = event->GetString( "albumname" );
		const char *szArtistName = event->GetString( "artistname" );
		bool bInterruptCustomTrack = event->GetBool( "interruptcustom" );
		PlayRandomTrack( fadeInTime, szDefaultTrack, szTrackName, szAlbumName, szArtistName, bInterruptCustomTrack );
	}
	else if( FStrEq( event->GetName(), "jukebox_stop" ) )
	{
		// Always fade the music from the stop event
		float fadeOutTime = event->GetFloat( "fadeouttime" );
		StopTrack( false, fadeOutTime );
	}
}

void CASWJukeboxPlaylist::PlayRandomTrack( float fadeInTime, const char *szDefaultTrack, const char *szTrackName, const char *szAlbumName, const char *szArtistName, bool bInterruptCustomTrack )
{
	// Choose a random track to play
	int count = m_CombatMusicPlaylist.Count();
	int index = 0;
	CSoundPatch *pNewSound = null;
	CLocalPlayerFilter filter;
	if ( count == 0 )
	{
		DevMsg( "JUKEBOX: Playing Track: %s\n", szDefaultTrack );
		if ( szDefaultTrack[0] == '\0' )
			return;

		pNewSound = CSoundEnvelopeController::GetController().SoundCreate( filter, 0, CHAN_STATIC, szDefaultTrack, SNDLVL_NONE );
		TryLocalize( szTrackName, m_wszTrackName, sizeof( m_wszTrackName ) );
		TryLocalize( szAlbumName, m_wszAlbumName, sizeof( m_wszAlbumName ) );
		TryLocalize( szArtistName, m_wszArtistName, sizeof( m_wszArtistName ) );
	}
	else
	{
		if ( m_pCombatMusic && !bInterruptCustomTrack )
			return;

		// If there's more than one track, randomize it so the current track doesn't repeat itself
		if ( count > 1 )
		{
			if ( m_iCurrentTrack == -1 )
			{
				index = RandomInt( 0, count - 1 );
			}
			else
			{
				index = RandomInt( 0, count - 2 );
				if ( index >= m_iCurrentTrack )
					index++;
			}
		}

		CFmtStr szFilename{ "*#music/_mp3/%s.mp3", m_CombatMusicPlaylist[index].m_szHexname };
		DevMsg( "JUKEBOX: Playing Track: %s\n", szFilename.Access() );
		pNewSound = CSoundEnvelopeController::GetController().SoundCreate( filter, 0, CHAN_STATIC, szFilename, SNDLVL_NONE );
		V_wcsncpy( m_wszTrackName, m_CombatMusicPlaylist[index].m_wszTrackName, sizeof( m_wszTrackName ) );
		V_wcsncpy( m_wszAlbumName, m_CombatMusicPlaylist[index].m_wszAlbum, sizeof( m_wszAlbumName ) );
		V_wcsncpy( m_wszArtistName, m_CombatMusicPlaylist[index].m_wszArtist, sizeof( m_wszArtistName ) );
	}

	if ( !pNewSound )
	{
		return;
	}

	float flDelay = 0.0f;

	// If combat music is already playing, cross fade to the new track.
	if ( m_pCombatMusic )
	{
		if ( count == 1 )
		{
			CSoundEnvelopeController::GetController().SoundDestroy( pNewSound );
			return;
		}

		StopTrack( false, fadeInTime );
		flDelay = 1.0f;
	}

	CSoundEnvelopeController::GetController().Play( pNewSound, 0.0f, 100, flDelay );
	CSoundEnvelopeController::GetController().SoundChangeVolume( pNewSound, 1.0f, fadeInTime );

	m_pCombatMusic = pNewSound;
	m_iCurrentTrack = index;

	if ( m_wszTrackName[0] != L'\0' )
	{
		CRD_HUD_Now_Playing *pNowPlaying = GET_FULLSCREEN_HUDELEMENT( CRD_HUD_Now_Playing );
		if ( pNowPlaying )
		{
			pNowPlaying->ShowAfterDelay( flDelay );
		}
	}
}

void CASWJukeboxPlaylist::StopTrack( bool immediate /*= true*/, float fadeOutTime /*= 1.0f */ )
{
	if ( m_pCombatMusic )
	{
		if ( immediate )
			CSoundEnvelopeController::GetController().SoundDestroy( m_pCombatMusic );
		else
			CSoundEnvelopeController::GetController().SoundFadeOut( m_pCombatMusic, fadeOutTime, true );

		m_pCombatMusic = NULL;
		m_iCurrentTrack = -1;

		CRD_HUD_Now_Playing *pNowPlaying = GET_FULLSCREEN_HUDELEMENT( CRD_HUD_Now_Playing );
		if ( pNowPlaying )
		{
			pNowPlaying->HideEarly();
		}
	}
}

void CASWJukeboxPlaylist::LevelShutdownPostEntity( void )
{
	// Stop music
	StopTrack( true );
	RemoveAllMusic();
}

void CASWJukeboxPlaylist::ExportPlayistKV( void )
{
	KeyValues::AutoDelete pPlaylistKV{ "playlist" };

	for ( int i = 0; i < m_CombatMusicPlaylist.Count(); ++i )
	{
		KeyValues *pTrackKV = new KeyValues( "Track" );
		pTrackKV->SetWString( "TrackName", m_CombatMusicPlaylist[i].m_wszTrackName );
		pTrackKV->SetString( "HexName", m_CombatMusicPlaylist[i].m_szHexname );
		pTrackKV->SetWString( "Album", m_CombatMusicPlaylist[i].m_wszAlbum );
		pTrackKV->SetWString( "Artist", m_CombatMusicPlaylist[i].m_wszArtist );
		pPlaylistKV->AddSubKey( pTrackKV );
	}

	pPlaylistKV->SaveToFile( g_pFullFileSystem, PLAYLIST_FILENAME, "MOD" );
}

void CASWJukeboxPlaylist::SavePlaylist()
{
	Cleanup();
	ExportPlayistKV();
}

const wchar_t *CASWJukeboxPlaylist::GetTrackName( int index )
{
	if ( index >= m_CombatMusicPlaylist.Count() )
		return NULL;
	else
		return m_CombatMusicPlaylist[index].m_wszTrackName;
}

const wchar_t *CASWJukeboxPlaylist::GetTrackArtist( int index )
{
	if ( index >= m_CombatMusicPlaylist.Count() )
		return NULL;
	else
		return m_CombatMusicPlaylist[index].m_wszArtist;
}

const wchar_t *CASWJukeboxPlaylist::GetTrackAlbum( int index )
{
	if ( index >= m_CombatMusicPlaylist.Count() )
		return NULL;
	else
		return m_CombatMusicPlaylist[index].m_wszAlbum;
}

int CASWJukeboxPlaylist::GetTrackCount()
{
	return m_CombatMusicPlaylist.Count();
}

void CASWJukeboxPlaylist::RemoveMusicFromPlaylist( const char *szHexnameToRemove )
{
	if ( m_CombatMusicPlaylist.Count() > 0 )
	{
		TrackInfo_t temp;
		V_strncpy( temp.m_szHexname, szHexnameToRemove, sizeof( temp.m_szHexname ) );

		m_CombatMusicPlaylist.FindAndFastRemove( temp );
	}
}

void CASWJukeboxPlaylist::PrepareTrackKV( int index, KeyValues *pKV )
{
	if ( index >= m_CombatMusicPlaylist.Count() && !m_CombatMusicPlaylist[index].m_bIsMarkedForDeletion )
		return;
	else
		m_CombatMusicPlaylist[index].PrepareKVForListView( pKV );
}

void CASWJukeboxPlaylist::MarkTrackForDeletion( int index )
{
	if ( index >= m_CombatMusicPlaylist.Count() )
		return;

	if ( index == m_iCurrentTrack )
		StopTrack( true, 0.0f );

	m_CombatMusicPlaylist[index].m_bIsMarkedForDeletion = true;

	// Delete the audio file too
	CFmtStr szFullPath{ "sound/music/_mp3/%s.mp3", m_CombatMusicPlaylist[index].m_szHexname };
	if ( g_pFullFileSystem->FileExists( szFullPath, NULL ) )
	{
		g_pFullFileSystem->RemoveFile( szFullPath, NULL );
	}
}

void CASWJukeboxPlaylist::Cleanup( void )
{
	for ( int i = 0; i < m_CombatMusicPlaylist.Count(); )
	{
		if ( m_CombatMusicPlaylist[i].m_bIsMarkedForDeletion )
			m_CombatMusicPlaylist.Remove( i );
		else
			++i;
	}
}

void CASWJukeboxPlaylist::LevelInitPostEntity( void )
{
	RemoveAllMusic();
	// Load the saved playlist
	LoadPlaylistKV();

	CRD_HUD_Now_Playing *pNowPlaying = GET_FULLSCREEN_HUDELEMENT( CRD_HUD_Now_Playing );
	if ( pNowPlaying )
	{
		pNowPlaying->HideImmediately();
	}
}

void CASWJukeboxPlaylist::RemoveAllMusic( void )
{
	StopTrack( true, 0.0f );
	m_CombatMusicPlaylist.RemoveAll();
}

bool CASWJukeboxPlaylist::IsMusicPlaying( void )
{
	return m_pCombatMusic != NULL;
}

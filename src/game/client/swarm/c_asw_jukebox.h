//====== Copyright © 1996-2006, Valve Corporation, All rights reserved. =======//
//
// Purpose: Handles construction of a user defined music playlist.
//
//=============================================================================//
#ifndef _DEFINED_C_ASW_JUKEBOX_H
#define _DEFINED_C_ASW_JUKEBOX_H

#include "igamesystem.h"
#include "GameEventListener.h"
#include "utlvector.h"
#include "platform.h"

class CSoundPatch;

// Information about music tracks
struct TrackInfo_t
{
	TrackInfo_t();
	TrackInfo_t( const wchar_t *wszTrackName, const char *szHexname, const wchar_t *wszAlbum, const wchar_t *wszArtist );

	char	m_szHexname[9]{};
	wchar_t	m_wszTrackName[256]{};
	wchar_t	m_wszAlbum[256]{};
	wchar_t	m_wszArtist[256]{};
	bool	m_bIsMarkedForDeletion{ false };

	bool	operator==( const TrackInfo_t& rhs ) const;
	void	PrepareKVForListView( KeyValues *kv );
};

struct ID3Info_t;

//-----------------------------------------------------------------------------
// Purpose: Lets a client define a playlist to use for various music events in game.
//-----------------------------------------------------------------------------
class CASWJukeboxPlaylist : public CGameEventListener, public CAutoGameSystem
{
public:
	void AddMusicToPlaylist( const wchar_t *wszTrackName, const char *szHexname, const wchar_t *wszAlbum, const wchar_t *wszArtist );
	void MarkTrackForDeletion( int index );

	// CAutoGameSystem methods
	virtual bool Init( void );
	virtual void Shutdown( void );
	virtual void LevelInitPostEntity( void );
	virtual void LevelShutdownPostEntity( void );

	// CGameEventListener methods
	virtual void FireGameEvent( IGameEvent *event );
	void StopTrack( bool immediate = true, float fadeOutTime = 1.0f );
	void SavePlaylist( void );
	const wchar_t *GetTrackName( int index );
	const wchar_t *GetTrackArtist( int index );
	const wchar_t *GetTrackAlbum( int index );
	void PrepareTrackKV( int index, KeyValues *pKV );
	int GetTrackCount( void );
	void Cleanup( void );
	bool IsMusicPlaying( void );

private:
	void RemoveAllMusic( void );
	void RemoveMusicFromPlaylist( const char* szHexnameToRemove );
	void LoadPlaylistKV( void );
	void ExportPlayistKV( void );
	void PlayRandomTrack( float fadeInTime = 1.0f, const char *szDefaultTrack = NULL, const char *szTrackName = NULL, const char *szAlbumName = NULL, const char *szArtistName = NULL, bool bInterruptCustomTrack = true );

	typedef CUtlVector< TrackInfo_t > TrackList_t;
	TrackList_t		m_CombatMusicPlaylist;
	CSoundPatch		*m_pCombatMusic{}; // The current combat music that's playing
	int				m_iCurrentTrack{ -1 }; // The index of the current track being played
	wchar_t			m_wszTrackName[256]{};
	wchar_t			m_wszAlbumName[256]{};
	wchar_t			m_wszArtistName[256]{};
};

extern CASWJukeboxPlaylist g_ASWJukebox;

#endif // #ifndef _DEFINED_C_ASW_JUKEBOX_H

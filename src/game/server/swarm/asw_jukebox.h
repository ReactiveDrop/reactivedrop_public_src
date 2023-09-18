#ifndef _DEFINED_ASW_JUKEBOX_H
#define _DEFINED_ASW_JUKEBOX_H

class CASW_Jukebox : public CBaseEntity
{
public:
	DECLARE_CLASS( CASW_Jukebox, CBaseEntity );
	DECLARE_DATADESC();

	void Precache() override;

	void InputMusicStart( inputdata_t &inputdata );
	void InputMusicStop( inputdata_t &inputdata );
protected:
	float m_fFadeInTime{ 1 };
	float m_fFadeOutTime{ 1 };
	string_t m_szDefaultMusic{ NULL_STRING };
	string_t m_szTrackName{ NULL_STRING };
	string_t m_szAlbumName{ NULL_STRING };
	string_t m_szArtistName{ NULL_STRING };
	bool m_bInterruptCustomTrack{ true };
};

#endif // #ifndef _DEFINED_ASW_JUKEBOX_H
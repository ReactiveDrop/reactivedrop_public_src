#ifndef _INCLUDED_C_ASW_SENTRY_TOP_FLAMER_H
#define _INCLUDED_C_ASW_SENTRY_TOP_FLAMER_H


#include "asw_shareddefs.h"

// class C_ASW_Sentry_Base;
class CASWGenericEmitter;

extern ConVar asw_sentry_emitter_test;

class C_ASW_Sentry_Top_Flamer : public C_ASW_Sentry_Top
{
public:
	DECLARE_CLASS( C_ASW_Sentry_Top_Flamer, C_ASW_Sentry_Top );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_ASW_Sentry_Top_Flamer();

	virtual void OnDataChanged( DataUpdateType_t updateType );

	inline bool IsPlayingFiringLoopSound( );
	virtual bool HasPilotLight() { return true; }

	virtual void UpdateOnRemove();

protected:
	void OnStartFiring();
	void OnStopFiring();

	CUtlReference<CNewParticleEffect> m_hFiringEffect;
	CNetworkVar( bool, m_bFiring );
	CNetworkVar( float, m_flPitchHack ); // just for a second to deal with stairs until I get proper pitch gimballing on base class
	CSoundPatch *m_pFlamerLoopSound;

	const char *m_szParticleEffectFireName;

	// sound scripts for begin, during, end (initialize in your constructor)
	// allowed to be NULL
	const char *m_szBeginFireSoundScriptName;
	const char *m_szDuringFireSoundScriptName;
	const char *m_szEndFireSoundScriptName;

	// shadows of network vars so we can detect their change in OnDataChanged
	bool m_bFiringShadow : 1;
};


inline bool C_ASW_Sentry_Top_Flamer::IsPlayingFiringLoopSound( )
{
	return m_pFlamerLoopSound != NULL;
}

#endif /* _INCLUDED_C_ASW_SENTRY_TOP_H */
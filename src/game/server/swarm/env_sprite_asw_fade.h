#ifndef _INCLUDED_ENV_SPRITE_ASW_FADE_H
#define _INCLUDED_ENV_SPRITE_ASW_FADE_H
#ifdef _WIN32
#pragma once
#endif

#include "Sprite.h"
#include "asw_shareddefs.h"

class CASW_Inhabitable_NPC;

class CEnv_Sprite_ASW_Fade : public CSprite
{
public:
	DECLARE_CLASS( CEnv_Sprite_ASW_Fade, CSprite );

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DECLARE_ENT_SCRIPTDESC();

	CEnv_Sprite_ASW_Fade();

	bool ShouldFade( CASW_Inhabitable_NPC *pNPC );

	int GetMarineIndex( CBaseEntity *pEnt, const char *szContext );

	void InputShowToMarine( inputdata_t &inputdata );
	void InputHideFromMarine( inputdata_t &inputdata );
	void InputSetVisibleForAllMarines( inputdata_t &inputdata );
	void ScriptSetVisibleForMarine( HSCRIPT hMarine, bool bVisible );
	bool ScriptIsVisibleForMarine( HSCRIPT hMarine, bool bConsiderPositionalFade );

	CNetworkVector( m_vecFadeOrigin );

	CNetworkVar( byte, m_nFadeOpacity );
	CNetworkVar( bool, m_bAllowFade );

	ASSERT_INVARIANT( ASW_MAX_MARINE_RESOURCES <= 32 );
	CNetworkVar( uint32_t, m_fVisibleToMarine );
};

#endif // _INCLUDED_ENV_SPRITE_ASW_FADE_H

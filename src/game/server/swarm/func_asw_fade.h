#ifndef _INCLUDED_FUNC_ASW_FADE_H
#define _INCLUDED_FUNC_ASW_FADE_H
#ifdef _WIN32
#pragma once
#endif

#include "modelentities.h"

class CASW_Inhabitable_NPC;

class CFunc_ASW_Fade : public CFuncBrush
{
public:
	DECLARE_CLASS( CFunc_ASW_Fade, CFuncBrush );

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CFunc_ASW_Fade();

	virtual void Spawn() override;
	bool ShouldFade( CASW_Inhabitable_NPC *pNPC );

	CNetworkVar( bool, m_bHasProxies );
protected:
	bool m_bCollideWithGrenades;
	CNetworkVar( byte, m_nFadeOpacity );
	CNetworkVar( bool, m_bAllowFade );
};


#endif // _INCLUDED_FUNC_ASW_FADE_H

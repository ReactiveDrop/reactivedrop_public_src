#ifndef _INCLUDED_PROP_ASW_FADE_H
#define _INCLUDED_PROP_ASW_FADE_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_prop_dynamic.h"

class CASW_Inhabitable_NPC;

class CProp_ASW_Fade : public CASW_Prop_Dynamic
{
public:
	DECLARE_CLASS( CProp_ASW_Fade, CASW_Prop_Dynamic );

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CProp_ASW_Fade();

	virtual void Spawn() override;
	bool ShouldFade( CASW_Inhabitable_NPC *pNPC );

	CNetworkVar( bool, m_bHasProxies );
	CNetworkVar( float, m_flFadeOriginOffset );
protected:
	Vector m_vecFadeOrigin;
	CNetworkVar( byte, m_nFadeOpacity );
	CNetworkVar( bool, m_bAllowFade );
};


#endif // _INCLUDED_PROP_ASW_FADE_H

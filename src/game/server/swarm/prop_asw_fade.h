#ifndef _INCLUDED_PROP_ASW_FADE_H
#define _INCLUDED_PROP_ASW_FADE_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_prop_dynamic.h"

class CProp_ASW_Fade : public CASW_Prop_Dynamic
{
public:
	DECLARE_CLASS( CProp_ASW_Fade, CASW_Prop_Dynamic );

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CProp_ASW_Fade();

	CNetworkVector( m_vecFadeOrigin );
protected:
	CNetworkVar( byte, m_nFadeOpacity );
	CNetworkVar( bool, m_bAllowFade );
};


#endif // _INCLUDED_PROP_ASW_FADE_H

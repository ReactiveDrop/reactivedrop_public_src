#ifndef _INCLUDED_PROP_ASW_FADE_H
#define _INCLUDED_PROP_ASW_FADE_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_prop_dynamic.h"
#include "asw_shareddefs.h"

class CASW_Inhabitable_NPC;

class CProp_ASW_Fade : public CASW_Prop_Dynamic
{
public:
	DECLARE_CLASS( CProp_ASW_Fade, CASW_Prop_Dynamic );

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DECLARE_ENT_SCRIPTDESC();

	CProp_ASW_Fade();

	virtual void Spawn() override;
	bool ShouldFade( CASW_Inhabitable_NPC *pNPC );

	void ApplyGrenadeCollisionRules( CBaseEntity *pGrenade );
	void ApplyMarineCollisionRules( CBaseEntity *pMarine, int iMR );

	void SetGrenadeCollisionRules( inputdata_t &inputdata );
	void SetMarineCollisionRules( inputdata_t &inputdata );

	int GetMarineIndex( CBaseEntity *pEnt, const char *szContext );

	void InputShowToMarine( inputdata_t &inputdata );
	void InputHideFromMarine( inputdata_t &inputdata );
	void InputSetVisibleForAllMarines( inputdata_t &inputdata );
	void ScriptSetVisibleForMarine( HSCRIPT hMarine, bool bVisible );
	bool ScriptIsVisibleForMarine( HSCRIPT hMarine, bool bConsiderPositionalFade );

	CNetworkVar( bool, m_bHasProxies );
	CNetworkVar( float, m_flFadeOriginOffset );
	// 0 = only when grenade spawns above the prop
	// 1 = always
	// 2 = never
	byte m_iCollideWithGrenades = 1;
	CNetworkVar( bool, m_bCollideWithMarines );

	Vector m_vecFadeOrigin;
	CNetworkVar( byte, m_nFadeOpacity );
	CNetworkVar( bool, m_bAllowFade );

	ASSERT_INVARIANT( ASW_MAX_MARINE_RESOURCES <= 32 );
	CNetworkVar( uint32_t, m_fVisibleToMarine );
	CNetworkVar( bool, m_bSolidWhenInvisible );
};


#endif // _INCLUDED_PROP_ASW_FADE_H

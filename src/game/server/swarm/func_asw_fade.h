#ifndef _INCLUDED_FUNC_ASW_FADE_H
#define _INCLUDED_FUNC_ASW_FADE_H
#ifdef _WIN32
#pragma once
#endif

#include "modelentities.h"
#include "asw_shareddefs.h"

class CASW_Inhabitable_NPC;

class CFunc_ASW_Fade : public CFuncBrush
{
public:
	DECLARE_CLASS( CFunc_ASW_Fade, CFuncBrush );

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	DECLARE_ENT_SCRIPTDESC();

	static constexpr const char *s_pszExplosiveClasses[] =
	{
		"asw_mine",
		"npc_grenade_frag",
		"asw_rocket",
		"grenadespit",
		"asw_missile_round",
		"asw_grenade_cluster",
		"asw_flare_projectile",
		"asw_laser_mine",
		"asw_grenade_vindicator",
		"asw_gas_grenade_projectile",
		"asw_bait",
		"asw_grenade_prifle",
		nullptr
	};

	CFunc_ASW_Fade();

	virtual void Spawn() override;
	bool ShouldFade( CASW_Inhabitable_NPC *pNPC );

	static void ApplyAllGrenadeCollisionRules( CBaseEntity *pGrenade );
	static void ApplyAllMarineCollisionRules( CBaseEntity *pMarine );

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
	// 0 = only when grenade spawns above the brush
	// 1 = always
	// 2 = never
	byte m_iCollideWithGrenades;
	CNetworkVar( bool, m_bCollideWithMarines );

	CNetworkVar( byte, m_nFadeOpacity );
	CNetworkVar( bool, m_bAllowFade );

	ASSERT_INVARIANT( ASW_MAX_MARINE_RESOURCES <= 32 );
	CNetworkVar( uint32_t, m_fVisibleToMarine );
	CNetworkVar( bool, m_bSolidWhenInvisible );
};

#endif // _INCLUDED_FUNC_ASW_FADE_H

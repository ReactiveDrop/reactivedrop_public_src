#ifndef _INCLUDED_ASW_SHAMAN_H
#define _INCLUDED_ASW_SHAMAN_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_alien.h"
#include "ai_blended_movement.h"
#include "util_shared.h"
#include "ai_speech.h"
#include "asw_ai_behavior_combat_stun.h"
#include "asw_ai_behavior_heal_other.h"
#include "asw_ai_behavior_scuttle.h"
#include "asw_ai_behavior_fear.h"
#include "asw_ai_behavior_idle.h"


class CASW_Shaman : public CASW_Alien
{
public:

	DECLARE_CLASS( CASW_Shaman, CASW_Alien );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CASW_Shaman();

	virtual	void		Spawn();
	virtual int			GetBaseHealth() override;
	virtual void		Precache();
	virtual void		NPCThink();

#if 0
	virtual float		GetMovementSpeedModifier();
#endif
	virtual float		MaxYawSpeed( void );

	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_SHAMAN; }

	virtual void		HandleAnimEvent( animevent_t *pEvent );

	virtual bool		CreateBehaviors();

	virtual bool		ShouldGib( const CTakeDamageInfo &info ) { return false; }

	// sounds
	virtual void PainSound( const CTakeDamageInfo &info );
	virtual void DeathSound( const CTakeDamageInfo &info );

	void SetCurrentHealingTarget( CBaseEntity *pTarget );

	CNetworkVar( EHANDLE, m_hHealingTarget );

private:
	CAI_ASW_CombatStunBehavior		m_CombatStunBehavior;
	CAI_ASW_HealOtherBehavior		m_HealOtherBehavior;
	CAI_ASW_FearBehavior			m_FearBehavior;
	CAI_ASW_ScuttleBehavior			m_ScuttleBehavior;
	CAI_ASW_IdleBehavior			m_IdleBehavior;
	float							m_flHealTime;

protected:
	DEFINE_CUSTOM_AI;
};

#endif	//_INCLUDED_ASW_SHAMAN_H

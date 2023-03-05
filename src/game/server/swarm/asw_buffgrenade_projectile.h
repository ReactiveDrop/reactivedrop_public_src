#ifndef _DEFINED_ASW_BUFFGREN_PROJECTILE_H
#define _DEFINED_ASW_BUFFGREN_PROJECTILE_H
#pragma once

#include "asw_aoegrenade_projectile.h"

class CASW_Marine;
class CASW_Skill_Details;

class CASW_BuffGrenade_Projectile : public CASW_AOEGrenade_Projectile
{
	DECLARE_CLASS( CASW_BuffGrenade_Projectile, CASW_AOEGrenade_Projectile );

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:

	CASW_BuffGrenade_Projectile();

	void Precache( void );

	static CASW_BuffGrenade_Projectile* Grenade_Projectile_Create( const Vector &position, const QAngle &angles, const Vector &velocity,
		const AngularImpulse &angVelocity, CBaseEntity *pOwner, CBaseEntity *pCreator,
		float flRadius, float flDuration );

	virtual float GetGrenadeGravity( void );

	virtual bool ShouldTouchEntity( CBaseEntity *pEntity );

	virtual void OnBurnout( void );

	virtual void StartAOE( CBaseEntity *pEntity );
	virtual void DoAOE( CBaseEntity *pEntity );
	virtual bool StopAOE( CBaseEntity *pEntity );

	void LoseTimeForMoving();

	int GetBuffedMarineCount() { return m_hBuffedMarines.Count(); }

	void AttachToMarine( CASW_Marine *pMarine );

protected:
	// if this buff grenade was deployed by a marine, these describe the skill used
	CASW_Skill_Details *m_pSkill;
	int m_iSkillPoints;

	CUtlVector<EHANDLE> m_hBuffedMarines;

	Vector m_vecLastOrigin;
};

#endif // _DEFINED_ASW_BUFFGREN_PROJECTILE_H

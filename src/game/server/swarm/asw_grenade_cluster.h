#ifndef _INCLUDED_ASW_GRENADE_CLUSTER_H
#define _INCLUDED_ASW_GRENADE_CLUSTER_H
#pragma once

#include "asw_grenade_vindicator.h"

class CASW_Grenade_Cluster : public CASW_Grenade_Vindicator
{
public:
	DECLARE_CLASS( CASW_Grenade_Cluster, CASW_Grenade_Vindicator );
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();
				
	virtual void Spawn();
	virtual void DoExplosion();
	virtual void Detonate();
	virtual void Precache();
	virtual void CheckNearbyDrones();
	virtual void SetFuseLength(float fSeconds);
	void VGrenadeTouch(CBaseEntity* pOther);
	static CASW_Grenade_Cluster *Cluster_Grenade_Create( float flDamage, float fRadius, int iClusters, const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, CBaseEntity *pCreatorWeapon );	
	virtual float GetEarliestAOEDetonationTime();
	virtual void SetClusters(int iClusters, bool bMaster = false);
	virtual float GetEarliestTouchDetonationTime();

	virtual int OnTakeDamage( const CTakeDamageInfo& info );
	bool m_bTeslaAmped;

	float m_fDetonateTime;
	float m_fEarliestAOEDetonationTime;
	Class_T m_CreatorWeaponClass;

	void SetAdvancedRicochet(bool bRicochet) { m_bAdvancedRicochet = bRicochet; }		// if set, grenade will ricochet with increaced velocity if no world contact explosion specified.
	bool m_bAdvancedRicochet;

	virtual void Disable();
	virtual void Enable();
	virtual void EnableWithReset();
	virtual void ReflectBack();
	virtual void ReflectRandomly();
	virtual void InputDisable(inputdata_t& inputdata);
	virtual void InputEnable(inputdata_t& inputdata);
	virtual void InputEnableWithReset(inputdata_t& inputdata);
	virtual void InputReflectBack(inputdata_t& inputdata);
	virtual void InputReflectRandomly(inputdata_t& inputdata);
	float m_fSavedDisableTime;
	bool m_bDisabled;
	int m_iMaxRicochets;
	Vector m_vecInitPos;
	// Classification
	virtual Class_T Classify( void ) { return (Class_T)CLASS_ASW_GRENADE_CLUSER; }
};

#endif // _INCLUDED_ASW_GRENADE_CLUSTER_H

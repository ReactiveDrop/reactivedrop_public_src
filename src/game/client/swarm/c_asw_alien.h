#ifndef _INCLUDED_C_ASW_ALIEN_H
#define _INCLUDED_C_ASW_ALIEN_H

#include "asw_alien_shared.h"
#include "c_asw_inhabitable_npc.h"
#include "asw_shareddefs.h"

class CNewParticleEffect;

class C_ASW_Alien : public C_ASW_Inhabitable_NPC
{
public:
	DECLARE_CLASS( C_ASW_Alien, C_ASW_Inhabitable_NPC );
	DECLARE_CLIENTCLASS();
	#include "asw_alien_shared_classmembers.h"

					C_ASW_Alien();
	virtual			~C_ASW_Alien();

	virtual bool IsAlien( void ) const { return true; }
	virtual bool IsAlienClassType( void ) const { return true; }

	// death;
	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	virtual void Bleed( const CTakeDamageInfo &info, const Vector &vecPos, const Vector &vecDir, trace_t *ptr );
	virtual void DoBloodDecal( float flDamage, const Vector &vecPos, const Vector &vecDir, trace_t *ptr, int bitsDamageType );
	virtual const char *GetDeathParticleEffectName( void ) { return "drone_death"; }
	virtual const char *GetBigDeathParticleEffectName( void ) { return "drone_death_big"; }
	virtual const char *GetSmallDeathParticleEffectName( void ) { return "drone_death_sml"; }
	virtual const char *GetRagdollGibParticleEffectName( void ) { return "drone_ragdoll_gib"; }
	virtual C_ClientRagdoll* CreateClientRagdoll( bool bRestoring = false );
	virtual C_BaseAnimating* BecomeRagdollOnClient( void );
	DeathStyle_t m_nDeathStyle;
	inline  bool IsHurler(); ///< is this drone set to go flinging at the camera
	inline bool IsMeleeThrown();
	virtual bool HasCustomDeathForce(){ return false; };
	virtual Vector GetCustomDeathForce(){ return vec3_origin; };

	// footsteps
	void FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options );
	void MarineStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity );
	surfacedata_t* GetGroundSurface();
	void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
	virtual void DoAlienFootstep( Vector &vecOrigin, float fvol );	
	bool m_bStepSideLeft;

	// custom shadow
	virtual bool GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const;
	ShadowType_t ShadowCastType();
	void GetShadowFromFlashlight(Vector &vecDir, float &fContribution) const;
	float m_fLastCustomContribution;
	Vector m_vecLastCustomDir;
	int m_iLastCustomFrame;

	int m_nLastSetModel;

	virtual float	GetInterpolationAmount( int flags );

	virtual float MaxSpeed();
	virtual float GetBasePlayerYawRate();
	float m_flAlienWalkSpeed;
	bool m_bInhabitedMovementAllowed;
private:
	C_ASW_Alien( const C_ASW_Alien & ) = delete; // not defined, not accessible
	static float sm_flLastFootstepTime;
};

extern ConVar asw_drone_ridiculous;
inline bool C_ASW_Alien::IsHurler()
{
	return m_nDeathStyle == kDIE_HURL || asw_drone_ridiculous.GetBool();
}

inline bool C_ASW_Alien::IsMeleeThrown()
{
	return m_nDeathStyle == kDIE_MELEE_THROW;
}

#endif /* _INCLUDED_C_ASW_ALIEN_H */
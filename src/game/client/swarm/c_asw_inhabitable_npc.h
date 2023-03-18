#ifndef _INCLUDED_C_ASW_INHABITABLE_NPC_H
#define _INCLUDED_C_ASW_INHABITABLE_NPC_H
#ifdef _WIN32
#pragma once
#endif

#include "c_ai_basenpc.h"
#include "glow_outline_effect.h"
#include "object_motion_blur_effect.h"
#include "asw_player_shared.h"
#include "iasw_client_aim_target.h"

class C_ASW_Player;
class C_ASW_Weapon;

class C_ASW_Inhabitable_NPC : public C_AI_BaseNPC, public IASW_Client_Aim_Target
{
public:
	DECLARE_CLASS( C_ASW_Inhabitable_NPC, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_ASW_Inhabitable_NPC();
	virtual ~C_ASW_Inhabitable_NPC();

	bool IsInhabitableNPC() const override { return true; }

	virtual bool IsInhabited();
	C_ASW_Player *GetCommander() const;
	CNetworkHandle( C_ASW_Player, m_Commander );
	const char *GetPlayerName() const;

	virtual void PostDataUpdate( DataUpdateType_t updateType ) override;
	virtual void UpdateOnRemove( void ) override;
	virtual bool ShouldPredict( void ) override;
	virtual C_BasePlayer *GetPredictionOwner() override;
	virtual void InitPredictable( C_BasePlayer *pOwner ) override;

	virtual bool IsAlien( void ) const { return false; }
	virtual void ClientThink( void ) override;
	virtual void PhysicsSimulate() override;

	// health
	virtual int	GetHealth() const { return m_iHealth; }

	// using entities over time
	C_BaseEntity *GetUsingEntity() { return m_hUsingEntity.Get(); }
	CNetworkHandle( C_BaseEntity, m_hUsingEntity );	// if set, marine will face this object
	const Vector &GetFacingPoint();
	void SetFacingPoint( const Vector &vec, float fDuration );
	Vector m_vecFacingPoint, m_vecFacingPointFromServer;
	float m_fStopFacingPointTime;

	C_ASW_Weapon *GetActiveASWWeapon( void ) const;
	C_ASW_Weapon *GetASWWeapon( int index ) const;
	virtual Vector Weapon_ShootPosition();
	int m_iDamageAttributeEffects;

	void TickRedName( float delta );
	float m_fRedNamePulse;	// from 0 to 1, how red the marine's name should appear on the HUD for medics
	bool m_bRedNamePulseUp;

	bool m_bUseLastRenderedEyePosition;
	float m_fLastTurningYaw;

	virtual float MaxSpeed();
	virtual bool IsMovementFrozen();
	virtual bool ShouldMoveSlow() const;
	virtual float GetBasePlayerYawRate();
	int m_nOldButtons;
	CNetworkVar( bool, m_bWalking );
	CNetworkVar( bool, m_bInhabited );
	ASW_Controls_t GetASWControls();
	CNetworkVar( int, m_iControlsOverride );
	CNetworkVar( float, m_fSpeedScale );
	CNetworkVar( float, m_fHurtSlowMoveTime );

	// Texture names and surface data, used by CASW_MarineGameMovement
	int				m_surfaceProps;
	surfacedata_t *m_pSurfaceData;
	float			m_surfaceFriction;
	char			m_chTextureType;
	char			m_chPreviousTextureType;	// Separate from m_chTextureType. This is cleared if the player's not on the ground.

	// Glows are enabled when the sniper scope is used
	CGlowObject m_GlowObject;
	CMotionBlurObject m_MotionBlurObject;
	virtual void UpdateGlowObject();
	CNetworkVector( m_vecGlowColor );
	CNetworkVar( float, m_flGlowAlpha );
	CNetworkVar( bool, m_bGlowWhenOccluded );
	CNetworkVar( bool, m_bGlowWhenUnoccluded );
	CNetworkVar( bool, m_bGlowFullBloom );

	virtual void MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );
	virtual void MakeUnattachedTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );

	// aim target interface
	IMPLEMENT_AUTO_LIST_GET();

	virtual float GetRadius() { return 23; }
	virtual bool IsAimTarget() { return GetHealth() > 0; }
	virtual const Vector &GetAimTargetPos( const Vector &vecFiringSrc, bool bWeaponPrefersFlatAiming ) { return m_vecLastRenderedPos; }
	virtual const Vector &GetAimTargetRadiusPos( const Vector &vecFiringSrc ) { return m_vecAutoTargetRadiusPos; }
	virtual Vector GetLocalAutoTargetRadiusPos() { return m_vecLastRenderedPos; }

	// storing our location for autoaim
	Vector m_vecLastRenderedPos;
	Vector m_vecAutoTargetRadiusPos;

	CNetworkVar( bool, m_bOnFire );
	CNetworkVar( bool, m_bElectroStunned );
	float m_fNextElectroStunEffect;
	CUtlReference<CNewParticleEffect> m_pBurningEffect;
	void UpdateFireEmitters( void );
	bool m_bClientOnFire;

	CNetworkVar( int, m_iAlienClassIndex );

private:
	C_ASW_Inhabitable_NPC( const C_ASW_Inhabitable_NPC & ) = delete; // not defined, not accessible
};

#endif // _INCLUDED_C_ASW_INHABITABLE_NPC_H

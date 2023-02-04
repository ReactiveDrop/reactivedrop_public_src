#ifndef _INCLUDED_C_ASW_MARINE_H
#define _INCLUDED_C_ASW_MARINE_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_shareddefs.h"
#include "asw_marine_shared.h"
#include "c_ai_basenpc.h"
#include "c_asw_vphysics_npc.h"
#include "iasw_client_aim_target.h"
#include "iasw_client_usable_entity.h"
#include "asw_playeranimstate.h"
#include "beamdraw.h"
#include "object_motion_blur_effect.h"
#include "asw_deathmatch_mode.h"
#include "dlight.h"

class C_ASW_Player;
class C_ASW_Marine_Resource;
class C_ASW_Weapon;
class CFlashlightEffect;
class C_ASW_Door_Area;
class CASW_Marine_Profile;
class CSoundPatch;
class IASW_Client_Vehicle;
class CASWGenericEmitter;
class C_ASW_Remote_Turret;
class C_PointCamera;
class C_ASW_Hack;
class C_ASW_Emitter;
class C_ASW_Order_Arrow;
class C_ASW_Pickup_Weapon;
class CNewParticleEffect;
class CASW_Melee_Attack;

#define CASW_Remote_Turret C_ASW_Remote_Turret

/*  We inherit C_ASW_Marine from IASW_Client_Aim_Target to allow autoaiming
	on marines for deathmatch
*/
class C_ASW_Marine : public C_ASW_VPhysics_NPC, public IASWPlayerAnimStateHelpers, public IASW_Client_Usable_Entity
{
public:
	DECLARE_CLASS( C_ASW_Marine, C_ASW_VPhysics_NPC );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

					C_ASW_Marine();
	virtual			~C_ASW_Marine();

	// aim target interface, allows autoaiming onto marines 
	IMPLEMENT_AUTO_LIST_GET();
	virtual bool IsAimTarget();

	virtual void	ClientThink();
	void			PostThink();	// called after moving when the marine is being inhabited
	bool			Simulate();
	Class_T		Classify( void ) { return (Class_T) CLASS_ASW_MARINE; }

	// Use this in preference to CASW_Marine::AsMarine( pEnt ) :
	static inline C_ASW_Marine *AsMarine( CBaseEntity *pEnt );

	// camera
	virtual const QAngle& GetRenderAngles();
	virtual int DrawModel( int flags, const RenderableInstance_t &instance );
	virtual const QAngle& ASWEyeAngles( void );	
	virtual void BuildTransformations( CStudioHdr *pHdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed );	// for left hand IK	
	Vector EyePosition(void);
	// custom render loc test for elevators
	Vector m_vecCustomRenderOrigin;
	virtual const Vector& GetRenderOrigin();
	virtual void MouseOverEntity(C_BaseEntity* pEnt, Vector vecCrosshairAimingPos);
	void ForceVisibleFirstPerson( bool bForce );
	byte m_PrevRenderAlpha;
	bool m_bIsHiddenLocal;
		
	// networking
	void NotifyShouldTransmit( ShouldTransmitState_t state );	
	virtual void UpdateClientSideAnimation();
	void OnDataChanged( DataUpdateType_t updateType );
	// prediction smoothing on elevators
	void NotePredictionError( const Vector &vDelta );
	void GetPredictionErrorSmoothingVector( Vector &vOffset );
	Vector m_vecPredictionError;
	float m_flPredictionErrorTime;
	CNetworkVar(float, m_fAIPitch);
	QAngle m_AIEyeAngles;
	Vector m_vecLastRenderedPos;	// marine position stored while drawing (since actual marine position is unreliable during CreateMove)
	bool m_bLastNoDraw;
	
	// ammo
	int GetAllAmmoCount( void );
	int GetTotalAmmoCount(int iAmmoIndex);
	int GetTotalAmmoCount(char *szName);
	int GetWeaponAmmoCount(int iAmmoIndex);
	int GetWeaponAmmoCount(char *szName);
	int GetNumberOfWeaponsUsingAmmo(int iAmmoType);
	bool CanPickupPrimaryAmmo();
	bool Weapon_Switch( CBaseCombatWeapon *pWeapon, int viewmodelindex=0 );
	virtual bool Weapon_CanSwitchTo( CBaseCombatWeapon *pWeapon );

	// weapon
	void DoDamagePowerupEffects( CBaseEntity *pTarget, CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	int m_iDamageAttributeEffects;
	virtual void FireBullets( const FireBulletsInfo_t &info );
	virtual void FireRegularBullets( const FireBulletsInfo_t &info );
	virtual void FirePenetratingBullets( const FireBulletsInfo_t &info, int iMaxPenetrate, float fPenetrateChance, int iSeedPlus, bool bAllowChange=true, Vector *pPiercingTracerEnd = NULL, bool bSegmentTracer = true );
	virtual void FireBouncingBullets( const FireBulletsInfo_t &info, int iMaxBounce, int iSeedPlus = 0, bool bAllowHittingAttacker = false );
	CBaseCombatWeapon* GetLastWeaponSwitchedTo();
	EHANDLE m_hLastWeaponSwitchedTo;
	bool m_bLastWeaponBeforeTempWasSecondary;
	virtual CBaseCombatWeapon* ASWAnim_GetActiveWeapon();
	virtual void ProcessMuzzleFlashEvent();
	virtual Vector			Weapon_ShootPosition();
	int GetWeaponPositionForPickup( const char* szWeaponClass, bool bIsTemporary );	// returns which slot in the m_hWeapons array this pickup should go in	
	int GetWeaponIndex( CBaseCombatWeapon *pWeapon ) const;		// returns weapon's position in our myweapons array
	Vector GetOffhandThrowSource( const Vector *vecStandingPos = NULL );
	virtual bool IsFiring();

	bool ShouldPreventLaserSight() { return m_flPreventLaserSightTime.Get() < 0.0f || m_flPreventLaserSightTime.Get() > gpGlobals->curtime; }
	CNetworkVar( float, m_flPreventLaserSightTime );

	// shadow
	Vector m_ShadowDirection;
	bool GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const;	
	ShadowType_t ShadowCastType();

	// commander/inhabiting
	C_ASW_Marine_Resource* GetMarineResource();
	bool IsInhabited();
	CHandle<C_ASW_Marine_Resource> m_hMarineResource;
	CASW_Marine_Profile* GetMarineProfile();
	CNetworkVar( int, m_nMarineProfile );

	// scanner
	inline float GetBlipStrength() { return m_CurrentBlipStrength; }
	float m_CurrentBlipStrength;
	int m_CurrentBlipDirection;	// -1 or 1 for shrinking/growing
	float m_LastThinkTime;
	CNetworkHandle(CBaseEntity, m_hMarineFollowTarget);		// the marine we're currently ordered to follow, to be shown on the scanner

	// sound
	surfacedata_t* GetGroundSurface();
	void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
	void MarineStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity );
	void DoWaterRipples();
	float m_flNextChatter;

	// footprints
	const char *GetMarineFootprintParticleName( surfacedata_t *psurface );
	void	MarineFootprintFX( surfacedata_t *psurface, const Vector &vecVelocity );
	bool	m_bIsFootprintOnLeft;

	// animation
	bool IsAnimatingReload();	// are we currently playing our reload anim?
	bool IsDoingEmoteGesture();	// is the marine doing some arm signal?
	void FireEvent( const Vector& origin, const QAngle& angles, int event, const char *options );
	IASWPlayerAnimState *m_PlayerAnimState;
	void DoAnimationEvent( PlayerAnimEvent_t event );
	virtual bool ASWAnim_CanMove();

	// movement
	void AvoidPhysicsProps( CUserCmd *pCmd );
	void PhysicsSimulate(void);
	float GetStopTime() { return m_fStopMarineTime; }
	void SetStopTime(float fTime) { m_fStopMarineTime = fTime; }
	float m_fStopMarineTime;
	float MaxSpeed();
	virtual void					EstimateAbsVelocity( Vector& vel );	// asw made virtual
	CNetworkVar(bool, m_bPreventMovement);
	CNetworkVar( bool, m_bForceWalking );
	CNetworkVar( bool, m_bRolls );

	// orders
	CNetworkVar(int, m_ASWOrders);
	ASW_Orders GetASWOrders() { return (ASW_Orders) m_ASWOrders.Get(); }
	CHandle<C_ASW_Order_Arrow> m_hOrderArrow;

	// flashlight	
	void UpdateFlashlight();
	bool CreateLightEffects();	
	CFlashlightEffect *m_pFlashlight;	// projector flashlight
	void ReleaseFlashlightBeam();		// release beam flashlight
	Beam_t	*m_pFlashlightBeam;
	dlight_t* m_pFlashlightDLight;

	// hacking
	bool m_bHacking;
	CNetworkHandle( C_ASW_Hack, m_hCurrentHack );
	bool IsHacking( void );
	bool IsUsingComputerOrButtonPanel();

	// gun effects
	void CreateWeaponEmitters();
	void DoMuzzleFlash();
	void DoImpactEffect( trace_t &tr, int nDamageType );

	float GetDamageBuffEndTime() { return m_flDamageBuffEndTime.Get(); }
	CNetworkVar( float, m_flDamageBuffEndTime );

	void UpdateElectrifiedArmor();
	float GetElectrifiedArmorEndTime() { return m_flElectrifiedArmorEndTime.Get(); }
	bool IsElectrifiedArmorActive() { return GetElectrifiedArmorEndTime() > gpGlobals->curtime; }
	CNetworkVar( float, m_flElectrifiedArmorEndTime );
	bool m_bClientElectrifiedArmor;
	CUtlReference<CNewParticleEffect> m_pElectrifiedArmorEmitter;

	// flamer
	CSmartPtr<CASWGenericEmitter> m_hFlameEmitter;
	CSmartPtr<CASWGenericEmitter> m_hFlameStreamEmitter;	
	void StartFlamerLoop();
	void StopFlamerLoop();
	bool bPlayingFlamerSound;
	float m_fFlameTime;	
	CSoundPatch* m_pFlamerLoopSound;
	// fire extinguisher
	// weapons handle their own effects now
	//CSmartPtr<CASWGenericEmitter> m_hFireExtinguisherEmitter;	
	void StartFireExtinguisherLoop();
	void StopFireExtinguisherLoop();
	bool bPlayingFireExtinguisherSound;
	float m_fFireExtinguisherTime;
	CSoundPatch* m_pFireExtinguisherLoopSound;

	// these methods are needed for minigun to play looped sound file when shooting
	void StartMinigunLoop();
	void StopMinigunLoop();
	bool bPlayingMinigunSound;
	//float m_fMinigunTime;	
	CSoundPatch* m_pMinigunLoopSound;

	// health related
	float m_flNextDamageSparkTime;
	CUtlReference<CNewParticleEffect> m_hLowHeathEffect;
	CUtlReference<CNewParticleEffect> m_hCriticalHeathEffect;
	CUtlReference<CNewParticleEffect> m_hSentryBuildDisplay;

	void CreateSentryBuildDisplay();
	void DestroySentryBuildDisplay();
	void SetSentryBuildDisplayEnabled( bool state );
	void UpdateDamageEffects( bool bHealthChanged );

	//void CreateHealEmitter();
	CUtlReference<CNewParticleEffect> m_pHealEmitter;
	CNetworkVar(bool, m_bSlowHeal);
	CNetworkVar(int, m_iSlowHealAmount);
	bool IsInfested();
	void UpdateHeartbeat();
	float m_fNextHeartbeat;	// time for the next heartbeat sound effect
	virtual int	GetHealth() const { return m_iHealth; }
	CNetworkVar(float, m_fFFGuardTime);	// if set, marine cannot fire any weapons for a certain amount of time
	int GetMaxHealth( void ) const { return m_iMaxHealth; }
	int  m_iMaxHealth;
	int  m_iOldHealth;
	float m_fLastHealTime;
	CNetworkVar(bool, m_bOnFire);
	bool m_bClientOnFire;
	CNewParticleEffect	*m_pBurningEffect;
	void UpdateFireEmitters();
	virtual void UpdateOnRemove();
	virtual bool TestHitboxes( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr );
	CNetworkVar(float, m_fInfestedTime);		// how many seconds of infestation we have left
	CNetworkVar(float, m_fInfestedStartTime);	// when the marine first got infested
	virtual void ImpactTrace( trace_t *pTrace, int iDamageType, char *pCustomImpactName );
	virtual C_ClientRagdoll* CreateClientRagdoll( bool bRestoring = false );
	virtual C_BaseAnimating* BecomeRagdollOnClient();
	virtual void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	virtual void Bleed( const CTakeDamageInfo &info, const Vector &vecPos, const Vector &vecDir, trace_t *ptr );

	// snow
	//CSmartPtr<CASWGenericEmitter> m_hSnowEmitter;

	// client usable entity
	virtual bool IsUsable( C_BaseEntity *pUser );
	virtual bool GetUseAction( ASWUseAction & action, C_ASW_Inhabitable_NPC *pUser );
	virtual void CustomPaint( int ix, int iy, int alpha, vgui::Panel *pUseIcon ) {}
	virtual bool ShouldPaintBoxAround() { return m_bKnockedOut; }
	virtual bool NeedsLOSCheck() { return m_bKnockedOut; }

	static int GetReviveIconTextureID();
	static int s_nReviveIconTextureID;

	// emote system
	void TickEmotes(float d);
	bool TickEmote(float d, bool bEmote, bool& bClientEmote, float& fEmoteTime);
	CNetworkVar(bool, bEmoteMedic);
	CNetworkVar(bool, bEmoteAmmo);
	CNetworkVar(bool, bEmoteSmile);
	CNetworkVar(bool, bEmoteStop);
	CNetworkVar(bool, bEmoteGo);
	CNetworkVar(bool, bEmoteExclaim);
	CNetworkVar(bool, bEmoteAnimeSmile);
	CNetworkVar(bool, bEmoteQuestion);
	bool bClientEmoteMedic, bClientEmoteAmmo, bClientEmoteSmile, bClientEmoteStop,
		bClientEmoteGo, bClientEmoteExclaim, bClientEmoteAnimeSmile, bClientEmoteQuestion;
	float fEmoteMedicTime, fEmoteAmmoTime, fEmoteSmileTime, fEmoteStopTime,
		fEmoteGoTime, fEmoteExclaimTime, fEmoteAnimeSmileTime, fEmoteQuestionTime;
	float m_flLastMedicCall, m_flLastAmmoCall;

	// driving
	IASW_Client_Vehicle *GetASWVehicle();
	bool IsDriving() { return m_bIsInVehicle && m_iVehicleSeat < 0; }
	bool IsInVehicle() { return m_bIsInVehicle; }
	CNetworkHandle( CBaseEntity, m_hASWVehicle );
	int m_iVehicleSeat;
	bool m_bIsInVehicle;

	// controlling a turret
	bool IsControllingTurret();
	C_ASW_Remote_Turret* GetRemoteTurret();
	CNetworkHandle( C_ASW_Remote_Turret, m_hRemoteTurret );

	// knocked out
	bool m_bKnockedOut;

	// poisoned
	void SetPoisoned(float f);	// makes the marine poisoned
	float m_fPoison;

	// for smooth turning of the marine
	Vector m_vLaserSightCorrection;
	float m_flLaserSightLength;

	CNetworkVar( bool, m_bAICrouch );		// if set, the AI will appear crouched when still

	// melee
	void PlayMeleeImpactEffects( CBaseEntity *pEntity, trace_t *tr );
	float m_fNextMeleeTime;
	CNetworkVar( int, m_iMeleeAttackID );
	CNetworkVar( float, m_flKnockdownYaw );
	CASW_Melee_Attack *GetCurrentMeleeAttack();
	Vector m_vecMeleeStartPos;
	float m_flMeleeStartTime, m_flMeleeLastCycle;
	CNetworkVar( float, m_flMeleeYaw );
	CNetworkVar( bool, m_bFaceMeleeYaw );
	bool m_bMeleeCollisionDamage;
	bool m_bMeleeComboKeypressAllowed;
	bool m_bMeleeComboKeyPressed;
	bool m_bMeleeComboTransitionAllowed;
	bool m_bMeleeMadeContact;
	int m_iUsableItemsOnMeleePress;
	ASW_Melee_Movement_t m_iMeleeAllowMovement;
	bool m_bMeleeKeyReleased;
	bool m_bPlayedMeleeHitSound;
#ifdef MELEE_CHARGE_ATTACKS
	CNetworkVar( float, m_flMeleeHeavyKeyHoldStart ); 
	bool m_bMeleeHeavyKeyHeld;
#endif
	bool m_bMeleeChargeActivate;
	virtual void HandlePredictedAnimEvent( int event, const char* options );
	int m_iPredictedEvent[ASW_MAX_PREDICTED_MELEE_EVENTS];
	float m_flPredictedEventTime[ASW_MAX_PREDICTED_MELEE_EVENTS];
	const char* m_szPredictedEventOptions[ASW_MAX_PREDICTED_MELEE_EVENTS];
	int m_iNumPredictedEvents;
	int m_iOnLandMeleeAttackID;
	EHANDLE m_hMeleeLockTarget;				// for autoaiming melee attacks
	float GetBaseMeleeDamage() { return m_flBaseMeleeDamage; }
	float m_flBaseMeleeDamage;
	void DoMeleeDamageTrace( float flYawStart, float flYawEnd );
	void ApplyMeleeDamage( CBaseEntity *pHitEntity, CTakeDamageInfo dmgInfo, Vector &vecAttackDir, trace_t *tr );
	CBaseEntity *MeleeTraceHullAttack( const Vector &vecStart, const Vector &vecEnd, const Vector &vecMins, const Vector &vecMaxs, bool bHitBehindMarine, float flAttackCone );
	void ApplyPassiveMeleeDamageEffects( CTakeDamageInfo &dmgInfo );
	bool HasPowerFist();

	bool IsReflectingProjectiles() { return m_bReflectingProjectiles; }

	CNetworkVar( bool, m_bReflectingProjectiles );
	CUtlVector<CBaseEntity*>	m_RecentMeleeHits;
	bool	m_bNoAirControl;

	// jump jets
	CNetworkVar( int, m_iJumpJetting );
	Vector	m_vecJumpJetStart;
	CNetworkVar( Vector, m_vecJumpJetEnd );  // reactivedrop: see same var in asw_marine.h
	CNetworkVar( float, m_fJumpJetDurationOverride);  // reactivedrop: see same var in asw_marine.h
	CNetworkVar( float, m_fJumpJetAnimationDurationOverride);  // reactivedrop: see same var in asw_marine.h
	float	m_flJumpJetStartTime;
	float	m_flJumpJetEndTime;
	void UpdateJumpJetEffects();
	CUtlReference<CNewParticleEffect> m_pJumpJetEffect[2];
	/**
		Stops jump jet particle effects and sounds
	*/
	void StopJumpJetEffects();


	// shoulder cone
	virtual void CreateShoulderCone();
	EHANDLE m_hShoulderCone;

	// backpack
	virtual void CreateBackpack( C_BaseCombatWeapon *pWeapon );
	virtual void RemoveBackpack();
	EHANDLE m_hBackpack;
	const char *m_sBackpackModel;

	// powerup
	bool HasAnyPowerups( void ) { return m_iPowerupType >= 0; }
	int m_iPowerupType;
	float m_flPowerupExpireTime;
	bool m_bPowerupExpires;

	int GetForcedActionRequest() { return m_iForcedActionRequest.Get(); }
	void ClearForcedActionRequest() { m_iForcedActionRequest = 0; }
	bool CanDoForcedAction( int iForcedAction );		// check if we're allowed to perform a forced action (certain abilities limit this)
	CNetworkVar( int, m_iForcedActionRequest );
	static C_ASW_Marine* GetLocalMarine();
	static C_ASW_Marine* GetViewMarine();

private:
	CMotionBlurObject m_MotionBlurObject;

	C_ASW_Marine( const C_ASW_Marine & ); // not defined, not accessible
	float m_fLastYawHack, m_fLastPitchHack;
	bool m_bStepSideLeft;

	CUtlVector<EHANDLE>	m_TouchingDoors;

	inline C_BaseEntity *GetSquadLeader()  // If I'm following in a squad, get the leader I'm following
	{
		return m_hMarineFollowTarget.Get();
	}

	void StopElectifiedArmorEffects(bool bLocalPlayer);
};


inline C_ASW_Marine *C_ASW_Marine::AsMarine( CBaseEntity *pEnt )
{
	return ( pEnt && pEnt->Classify() == CLASS_ASW_MARINE ) ? assert_cast<C_ASW_Marine *>(pEnt) : NULL;
}


#endif // _INCLUDED_C_ASW_MARINE_H

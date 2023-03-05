#ifndef ASW_INHABITABLE_NPC_H
#define ASW_INHABITABLE_NPC_H
#pragma once

#include "ai_playerally.h"
#include "iasw_spawnable_npc.h"
#include "asw_player_shared.h"
#include "asw_lag_compensation.h"

class CASW_Player;
class CASW_Weapon;
class CEnvTonemapController;
class CTonemapTrigger;

#define ASW_ALIEN_HEALTH_BITS 14

class CASW_Inhabitable_NPC : public CAI_PlayerAlly, public IASW_Spawnable_NPC
{
public:
	DECLARE_CLASS( CASW_Inhabitable_NPC, CAI_PlayerAlly );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();

	CASW_Inhabitable_NPC();
	virtual ~CASW_Inhabitable_NPC();

	bool IsInhabitableNPC() const override { return true; }

	virtual void Precache() override;
	virtual void Spawn() override;
	virtual void OnRestore() override;
	virtual void NPCInit() override;
	virtual void NPCThink() override;
	virtual int DrawDebugTextOverlays() override;

	// player control
	void SetCommander( CASW_Player *player );
	CASW_Player *GetCommander() const;
	HSCRIPT ScriptGetCommander() const;
	CNetworkHandle( CASW_Player, m_Commander );

	virtual bool IsInhabited();
	virtual void SetInhabited( bool bInhabited ) { m_bInhabited = bInhabited; }
	virtual void InhabitedBy( CASW_Player *player ) { }
	virtual void UninhabitedBy( CASW_Player *player ) { }

	void SetInitialCommander( CASW_Player *player );
	char m_szInitialCommanderNetworkID[64]; // ASWNetworkID of the first commander for this marine in this mission
	const char *GetPlayerName() const;
	virtual void Suicide();

	// gun
	CASW_Weapon *GetASWWeapon( int index ) const;
	CASW_Weapon *GetActiveASWWeapon() const;
	int m_iDamageAttributeEffects;

	virtual bool AIWantsToFire() { return false; }
	virtual bool AIWantsToFire2() { return false; }
	virtual bool AIWantsToReload() { return false; }

	virtual void DoImpactEffect( trace_t &tr, int nDamageType ) override;
	virtual void DoMuzzleFlash() override;
	virtual void MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType ) override;
	virtual void MakeUnattachedTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );

	// interacting
	virtual bool StartUsing( CBaseEntity *pEntity );
	virtual void StopUsing();
	inline CBaseEntity *GetUsingEntity() const { return m_hUsingEntity.Get(); }
	CNetworkHandle( CBaseEntity, m_hUsingEntity );
	void SetFacingPoint( const Vector &vec, float fDuration );
	CNetworkVar( Vector, m_vecFacingPointFromServer );
	float m_fStopFacingPointTime;

	// moving
	virtual int TranslateSchedule( int scheduleType ) override;
	virtual float MaxSpeed();
	virtual bool ShouldMoveSlow() const;
	virtual float GetIdealSpeed( void ) const override;
	virtual bool ModifyAutoMovement( Vector &vecNewPos );
	virtual bool OverrideMove( float flInterval ) override;
	int m_nOldButtons;
	CNetworkVar( bool, m_bWalking );
	CNetworkVar( bool, m_bInhabited );
	ASW_Controls_t GetASWControls();
	CNetworkVar( int, m_iControlsOverride );
	void ScriptSetControls( int iControls );

	virtual void InhabitedPhysicsSimulate() {}
	virtual void PhysicsSimulate() override;
	CASW_Lag_Compensation m_LagCompensation;
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_vecBaseVelocity );

	// Texture names and surface data, used by CASW_MarineGameMovement
	int				m_surfaceProps;
	surfacedata_t *m_pSurfaceData;
	float			m_surfaceFriction;
	char			m_chTextureType;
	char			m_chPreviousTextureType;	// Separate from m_chTextureType. This is cleared if the player's not on the ground.

	// fx overrides
	void ScriptSetFogController( HSCRIPT hEnt );
	CHandle<CFogController> m_hFogController;
	void ScriptSetPostProcessController( HSCRIPT hEnt );
	CHandle<CPostProcessController> m_hPostProcessController;
	void ScriptSetColorCorrection( HSCRIPT hEnt );
	CHandle<CColorCorrection> m_hColorCorrection;
	void ScriptSetTonemapController( HSCRIPT hEnt );
	CHandle<CEnvTonemapController> m_hTonemapController;
	void OnTonemapTriggerStartTouch( CTonemapTrigger *pTonemapTrigger );
	void OnTonemapTriggerEndTouch( CTonemapTrigger *pTonemapTrigger );
	CUtlVector<CHandle<CTonemapTrigger>> m_hTriggerTonemapList;
	void ScriptSetGlow( Vector vecColor, float flAlpha, bool bGlowWhenOccluded, bool bGlowWhenUnoccluded, bool bFullBloom );
	CNetworkVector( m_vecGlowColor );
	CNetworkVar( float, m_flGlowAlpha );
	CNetworkVar( bool, m_bGlowWhenOccluded );
	CNetworkVar( bool, m_bGlowWhenUnoccluded );
	CNetworkVar( bool, m_bGlowFullBloom );

	// health
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_iHealth );
	virtual void SetHealth( int amt ) override;
	virtual void SetHealthByDifficultyLevel() override;
	virtual int GetBaseHealth() { Assert( 0 ); return 100; } // should be overridden
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo &info ) override;
	virtual void Event_Killed( const CTakeDamageInfo &info ) override;

	// burning
	virtual	bool AllowedToIgnite( void ) override { return m_bFlammable; }
	virtual void ASW_Ignite( float flFlameLifetime, float flSize, CBaseEntity *pAttacker, CBaseEntity *pDamagingWeapon = NULL ) override;
	virtual void Ignite( float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false ) override;
	virtual void ScriptIgnite( float flFlameLifetime );
	virtual void Extinguish() override;

	// electro stun
	virtual void ElectroStun( float flStunTime ) override;
	virtual void ScriptElectroStun( float flStunTime );
	bool IsElectroStunned() { return m_bElectroStunned.Get(); }
	float m_fNextStunSound;
	CNetworkVar( float, m_fHurtSlowMoveTime );
	float m_flElectroStunSlowMoveTime;

	// freezeing
	virtual bool CanBeFullyFrozen() { return true; }
	virtual void Freeze( float flFreezeAmount, CBaseEntity *pFreezer, Ray_t *pFreezeRay ) override;
	virtual void Unfreeze() override;
	virtual void ScriptFreeze( float flFreezeAmount );
	virtual bool ShouldBecomeStatue( void ) override { return false; }
	virtual bool IsMovementFrozen( void ) override;
	virtual void UpdateThawRate();
	virtual float IceStatueChance() { return 0.01f; } // chance ice statue does not immediately shatter
	virtual float StatueShatterDelay() { return 0.0f; } // if the statue does shatter, how long does it wait before doing so?
	float m_flFreezeResistance;
	float m_flFrozenTime;
	float m_flBaseThawRate;

	// IASW_Spawnable_NPC implementation
	CHandle<CASW_Base_Spawner> m_hSpawner;
	virtual void SetSpawner( CASW_Base_Spawner *spawner ) override { m_hSpawner = spawner; }
	virtual CAI_BaseNPC *GetNPC() override { return this; }
	virtual void SetAlienOrders( AlienOrder_t Orders, Vector vecOrderSpot, CBaseEntity *pOrderObject ) override;
	virtual AlienOrder_t GetAlienOrders() override { return m_AlienOrders; }
	virtual void ClearAlienOrders();
	virtual void MoveAside() override;
	virtual void SetHoldoutAlien() override { m_bHoldoutAlien = true; }
	virtual bool IsHoldoutAlien() override { return m_bHoldoutAlien; }
	virtual bool CanStartBurrowed() override { return false; }
	virtual void StartBurrowed() override {}
	virtual void SetUnburrowActivity( string_t ) override {}
	virtual void SetUnburrowIdleActivity( string_t ) override {}
	virtual void OnSwarmSensed( int iDistance ) override {}
	virtual void OnSwarmSenseEntity( CBaseEntity *pEnt ) override {}

	void ScriptOrderMoveTo( HSCRIPT hOrderObject, bool bIgnoreMarines );
	void ScriptChaseNearestMarine();

	void SetSpawnZombineOnMarineKill( bool bSpawn );
	bool m_bSpawnZombineOnMarineKill;

	AlienOrder_t m_AlienOrders;
	Vector m_vecAlienOrderSpot;
	EHANDLE m_AlienOrderObject;
	bool m_bHoldoutAlien;
	bool m_bIgnoreMarines;
	bool m_bFailedMoveTo;
	bool m_bWasOnFireForStats;
	bool m_bFlammable;
	bool m_bTeslable;
	bool m_bFreezable;
	bool m_bFlinchable;
	bool m_bGrenadeReflector;
	CNetworkVar( bool, m_bOnFire );
	CNetworkVar( bool, m_bElectroStunned );
	int  m_iHealthBonus;
	float m_fSizeScale;
	CNetworkVar( float, m_fSpeedScale );
};

#endif /* ASW_INHABITABLE_NPC_H */

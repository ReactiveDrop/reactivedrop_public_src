#ifndef _INCLUDED_ASW_VEHICLE_JEEP_H
#define _INCLUDED_ASW_VEHICLE_JEEP_H
#ifdef _WIN32
#pragma once
#endif

#include "vehicle_base.h"
#include "iasw_vehicle.h"
#include "asw_marine.h"

#define ASW_JEEP_WHEEL_COUNT	4

struct ASWJeepWaterData_t
{
	bool		m_bWheelInWater[ASW_JEEP_WHEEL_COUNT];
	bool		m_bWheelWasInWater[ASW_JEEP_WHEEL_COUNT];
	Vector		m_vecWheelContactPoints[ASW_JEEP_WHEEL_COUNT];
	float		m_flNextRippleTime[ASW_JEEP_WHEEL_COUNT];
	bool		m_bBodyInWater;
	bool		m_bBodyWasInWater;

	DECLARE_SIMPLE_DATADESC();
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CASW_PropJeep : public CPropVehicleDriveable, public IASW_Vehicle
{
	DECLARE_CLASS( CASW_PropJeep, CPropVehicleDriveable );

public:

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();

	CASW_PropJeep( void );

	// CPropVehicle
	virtual void	ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData );
	virtual void	DriveVehicle( float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased );
	virtual void	SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move );
	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual void	DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles );
	virtual bool	AllowBlockedExit( CBasePlayer *pPlayer, int nRole ) { return false; }
	virtual bool	CanExitVehicle( CBaseEntity *pEntity );
	virtual bool	IsVehicleBodyInWater() { return m_WaterData.m_bBodyInWater; }

	// CBaseEntity
	void			Think(void);
	void			Precache( void );
	void			Spawn( void ); 

	virtual void	CreateServerVehicle( void );
	virtual Vector	BodyTarget( const Vector &posSrc, bool bNoisy = true );
	virtual void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	virtual int		OnTakeDamage( const CTakeDamageInfo &info );
	virtual void	EnterVehicle( CBasePlayer *pPlayer );
	virtual void	ExitVehicle( int iRole );

	void			AimGunAt( Vector *endPos, float flInterval );
	bool			TauCannonHasBeenCutOff( void ) { return m_bGunHasBeenCutOff; }

	// NPC Driving
	bool			NPC_HasPrimaryWeapon( void ) { return true; }
	void			NPC_AimPrimaryWeapon( Vector vecTarget );

	const char		*GetTracerType( void ) { return "AR2Tracer"; }
	void			DoImpactEffect( trace_t &tr, int nDamageType );

	bool HeadlightIsOn( void ) { return m_bHeadlightIsOn; }
	void HeadlightTurnOn( void ) { m_bHeadlightIsOn = true; }
	void HeadlightTurnOff( void ) { m_bHeadlightIsOn = false; }

	int FindClosestEmptySeat( Vector vecPoint );

	// implement our asw vehicle interface
	virtual int ASWGetNumPassengers() override;
	virtual void ASWSetDriver( CASW_Marine *pDriver ) override;
	virtual CASW_Marine *ASWGetDriver() override;
	virtual void ASWSetPassenger( int i, CASW_Marine *pPassenger ) override;
	virtual CASW_Marine *ASWGetPassenger( int i ) override;
	virtual int ASWGetSeatPosition( int i, Vector &origin, QAngle &angles ) override;
	virtual void ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType ) override;
	virtual CBaseEntity *GetEntity() override { return this; }
	virtual void ASWStartEngine() override { StartEngine(); }
	virtual void ASWStopEngine() override { StopEngine(); }
	virtual void ASWGetCameraOverrides( int *pControls, float *pPitch, float *pDist, float *pHeight ) override;
	virtual bool IsUsable( CBaseEntity *pUser ) override;
	virtual void NPCStartedUsing( CASW_Inhabitable_NPC *pNPC ) override {}
	virtual void NPCStoppedUsing( CASW_Inhabitable_NPC *pNPC ) override {}
	virtual void NPCUsing( CASW_Inhabitable_NPC *pNPC, float fDeltaTime ) override {}
	virtual bool NeedsLOSCheck() override { return false; }
	CNetworkArray( CHandle<CASW_Marine>, m_hPassenger, 10 );
	int m_iPassengerAttachment[10];
	CNetworkVar( unsigned int, m_iPassengerBits );

	// script wrapper funcs
	int ScriptGetCameraControls();
	void ScriptSetCameraControls( int controls );
	float ScriptGetCameraPitch();
	void ScriptSetCameraPitch( float pitch );
	float ScriptGetCameraDist();
	void ScriptSetCameraDist( float dist );
	float ScriptGetCameraHeight();
	void ScriptSetCameraHeight( float height );
	HSCRIPT ScriptGetDriver();
	HSCRIPT ScriptGetPassenger( int seat );
	int ScriptGetSpeed();
	int ScriptGetMaxSpeed();
	int ScriptGetRPM();
	float ScriptGetThrottle();
	bool ScriptHasBoost();
	int ScriptBoostTimeLeft();

private:

	void		FireCannon( void );
	void		ChargeCannon( void );
	void		FireChargedCannon( void );

	void		DrawBeam( const Vector &startPos, const Vector &endPos, float width );
	void		StopChargeSound( void );
	void		GetCannonAim( Vector *resultDir );

	void		InitWaterData( void );
	void		HandleWater( void );
	bool		CheckWater( void );
	void		CheckWaterLevel( void );
	void		CreateSplash( const Vector &vecPosition );
	void		CreateRipple( const Vector &vecPosition );

	void		UpdateSteeringAngle( void );
	void		CreateDangerSounds( void );

	void		ComputePDControllerCoefficients( float *pCoefficientsOut, float flFrequency, float flDampening, float flDeltaTime );
	void		DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );
	void		DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );


	void		InputStartRemoveTauCannon( inputdata_t &inputdata );
	void		InputFinishRemoveTauCannon( inputdata_t &inputdata );

private:

	bool			m_bGunHasBeenCutOff;
	float			m_flDangerSoundTime;
	int				m_nBulletType;
	bool			m_bCannonCharging;
	float			m_flCannonTime;
	float			m_flCannonChargeStartTime;
	Vector			m_vecGunOrigin;
	CSoundPatch		*m_sndCannonCharge;
	int				m_nSpinPos;
	float			m_aimYaw;
	float			m_aimPitch;
	float			m_throttleDisableTime;
	float			m_flAmmoCrateCloseTime;

	// handbrake after the fact to keep vehicles from rolling
	float			m_flHandbrakeTime;
	bool			m_bInitialHandbrake;

	float			m_flOverturnedTime;

	Vector			m_vecLastEyePos;
	Vector			m_vecLastEyeTarget;
	Vector			m_vecEyeSpeed;
	Vector			m_vecTargetSpeed;

	ASWJeepWaterData_t	m_WaterData;

	int				m_iNumberOfEntries;
	int				m_nAmmoType;

	// Seagull perching
	float			m_flPlayerExitedTime;	// Time at which the player last left this vehicle
	float			m_flLastSawPlayerAt;	// Time at which we last saw the player
	EHANDLE			m_hLastPlayerInVehicle;		

	CNetworkVar( bool, m_bHeadlightIsOn );

	// Camera overrides
	CNetworkVar( int, m_iCamControlsOverride );
	CNetworkVar( float, m_flCamPitchOverride );
	CNetworkVar( float, m_flCamDistOverride );
	CNetworkVar( float, m_flCamHeightOverride );
};

#endif // _INCLUDED_ASW_VEHICLE_JEEP_H

#ifndef _INCLUDED_C_ASW_JEEP_H
#define _INCLUDED_C_ASW_JEEP_H
#pragma once

#include "c_prop_vehicle.h"
#include "c_asw_fourwheelvehiclephysics.h"	// asw
#include "iasw_client_vehicle.h"
#include "c_asw_marine.h"

#define JEEP_DELTA_LENGTH_MAX	12.0f			// 1 foot
#define JEEP_FRAMETIME_MIN		1e-6
#define JEEP_HEADLIGHT_DISTANCE 1000

class CHeadlightEffect;

//=============================================================================
//
// Client-side Jeep Class
//
class C_ASW_PropJeep : public C_PropVehicleDriveable, public IASW_Client_Vehicle
{

	DECLARE_CLASS( C_ASW_PropJeep, C_PropVehicleDriveable );

public:

	DECLARE_CLIENTCLASS();
	DECLARE_INTERPOLATION();

	C_ASW_PropJeep();
	virtual ~C_ASW_PropJeep();

public:

	void UpdateViewAngles( C_BasePlayer *pLocalPlayer, CUserCmd *pCmd );
	void DampenEyePosition( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles );

	void OnEnteredVehicle( C_BasePlayer *pPlayer );
	bool Simulate( void );

public:

	void DampenForwardMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );
	void DampenUpMotion( Vector &vecVehicleEyePos, QAngle &vecVehicleEyeAngles, float flFrameTime );
	void ComputePDControllerCoefficients( float *pCoefficientsOut, float flFrequency, float flDampening, float flDeltaTime );

	// implement our asw vehicle interface
	virtual int ASWGetSeatPosition( int i, Vector &origin, QAngle &angles ) override;
	virtual int ASWGetNumPassengers() override;
	virtual C_ASW_Marine *ASWGetDriver() override;
	virtual C_ASW_Marine *ASWGetPassenger( int i ) override;
	CNetworkArray( CHandle<CASW_Marine>, m_hPassenger, 10 );
	int m_iPassengerAttachment[10];
	CNetworkVar( unsigned int, m_iPassengerBits );
	// implement client vehicle interface
	virtual bool ValidUseTarget() { return true; }
	virtual int GetDriveIconTexture();
	virtual int GetRideIconTexture();
	virtual C_BaseEntity* GetEntity() { return this; }
	static bool s_bLoadedRideIconTexture;
	static int s_nRideIconTextureID;
	static bool s_bLoadedDriveIconTexture;
	static int s_nDriveIconTextureID;
	// no clientside prediction for this kind of vehicle
	virtual void SetupMove( C_BasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move ) { } 
	virtual void ProcessMovement( C_BasePlayer *pPlayer, CMoveData *pMoveData ) { }
	virtual void ASWStartEngine() { } 
	virtual void ASWStopEngine() { }

	int FindClosestEmptySeat( Vector vecPoint );
	bool MarineInVehicle();

	virtual bool IsUsable(C_BaseEntity *pUser);
	virtual bool GetUseAction(ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser);
	virtual void CustomPaint(int ix, int iy, int alpha, vgui::Panel *pUseIcon) { }
	virtual bool ShouldPaintBoxAround() { return !MarineInVehicle(); }

protected:

	Vector							m_vecSmoothedVelocity;
	Vector		m_vecLastEyePos;
	Vector		m_vecLastEyeTarget;
	Vector		m_vecEyeSpeed;
	Vector		m_vecTargetSpeed;

	float		m_flViewAngleDeltaTime;

	float		m_flJeepFOV;
	CHeadlightEffect *m_pHeadlight;
	bool		m_bHeadlightIsOn;	
};

#endif // _INCLUDED_C_ASW_JEEP_H
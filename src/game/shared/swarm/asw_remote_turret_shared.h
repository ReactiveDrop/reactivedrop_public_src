#ifndef _INCLUDED_ASW_REMOTE_TURRET_H
#define _INCLUDED_ASW_REMOTE_TURRET_H
#pragma once

#ifdef CLIENT_DLL
#define CASW_Remote_Turret C_ASW_Remote_Turret
class C_BasePlayer;
class C_ASW_Marine;
#else
class CBasePlayer;
class CASW_Marine;
#endif
class CUserCmd;
class IMoveHelper;
class CMoveData;

// a turret in the world that can be remote controlled by a player (from a computer).
class CASW_Remote_Turret : public CBaseAnimating
{
public:
	DECLARE_CLASS( CASW_Remote_Turret, CBaseAnimating );
	DECLARE_NETWORKCLASS();

	CASW_Remote_Turret();
	virtual ~CASW_Remote_Turret();

	void SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move );
	void ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData );
	const QAngle &EyeAngles();
	const QAngle &RealEyeAngles();
	void SmoothTurretAngle( QAngle &ang );
	Vector GetTurretCamPosition();
	Vector GetTurretMuzzlePosition();
	void GetButtons( bool &bAttack1, bool &bAttack2, bool &bReload );
	void FireTurret( CBasePlayer *pPlayer );
	int GetSentryDamage();

	virtual void MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
	void Precache();
	void Spawn();
	int UpdateTransmitState();
	int ShouldTransmit( const CCheckTransmitInfo *pInfo );
	void StopUsingTurret();
	void StartedUsingTurret( CASW_Marine *pUser, CBaseEntity *pComputerArea );
	QAngle AutoaimDeflection( Vector &vecSrc, const QAngle &eyeAngles, autoaim_params_t &params );
	float GetAutoaimScore( const Vector &eyePosition, const Vector &viewDir, const Vector &vecTarget, CBaseEntity *pTarget, float fScale );

	COutputEvent m_OnStartedUsing;
	COutputEvent m_OnStoppedUsing;

	CNetworkQAngle( m_angEyeAngles );
#else
	int m_iFireSequence;
	int m_iIdleSequence;
	int m_iIdleOffSequence;
	int m_iTurnOnSequence;
	int m_iTurnOffSequence;
	virtual void ClientThink();
	virtual void ReachedEndOfSequence();
	QAngle m_angEyeAngles;
	CInterpolatedVar<QAngle> m_iv_angEyeAngles;
	float GetMuzzleFlashScale();
	int GetMuzzleAttachment();
	void ProcessMuzzleFlashEvent();

	float m_flNextTurnSound;
	bool m_bLastUser;
	QAngle m_LastAngle;
	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void CreateMove( float flInputSampleTime, CUserCmd *pCmd );
	void ASWRemoteTurretTracer( const Vector &vecEnd );
#endif
	CNetworkVar( bool, m_bUpsideDown );
#ifdef CLIENT_DLL
	C_ASW_Marine *GetUser();
	CNetworkHandle( C_ASW_Marine, m_hUser );
#else
	CASW_Marine *GetUser();
	CNetworkHandle( CASW_Marine, m_hUser );
	EHANDLE m_hComputerArea;
#endif
	float m_fNextFireTime;
	int m_iAmmoType;
	virtual const Vector &GetBulletSpread( void )
	{
		static const Vector cone = VECTOR_CONE_PRECALCULATED;

		return cone;
	}
	virtual Class_T Classify( void ) { return ( Class_T )CLASS_ASW_REMOTE_TURRET; }

	CNetworkQAngle( m_angDefault );	// reference angle for view limits
	CNetworkQAngle( m_angViewLimit );	// how far we can look either side
};

#endif /* _INCLUDED_ASW_REMOTE_TURRET_H */

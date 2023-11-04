#include "cbase.h"
#ifdef CLIENT_DLL
#include "c_asw_sentry_top.h"
#include "c_asw_sentry_base.h"
#include "c_asw_marine.h"
#include "c_asw_player.h"
#define CASW_Sentry_Top C_ASW_Sentry_Top
#define CASW_Sentry_Base C_ASW_Sentry_Base
#else
#include "asw_sentry_top.h"
#include "asw_sentry_base.h"
#include "asw_marine.h"
#include "asw_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int CASW_Sentry_Top::GetSentryAngle()
{
	return m_iSentryAngle;
}

float CASW_Sentry_Top::GetScanAngle()
{
	return GetSentryAngle() * sin( gpGlobals->curtime * 3.0f );
}

void CASW_Sentry_Top::UpdatePose()
{
	if ( m_iPoseParamPitch == -2 )
		m_iPoseParamPitch = LookupPoseParameter( "aim_pitch" );
	if ( m_iPoseParamYaw == -2 )
		m_iPoseParamYaw = LookupPoseParameter( "aim_yaw" );
	if ( m_iPoseParamAmmoRemaining == -2 )
		m_iPoseParamAmmoRemaining = LookupPoseParameter( "ammo_remaining" );

	bool bReturning = AlmostEqual( m_fGoalYaw, GetDeployYaw() );
	QAngle angles = GetAbsAngles();

	float flTargetPitch = bReturning ? 0.0f : ( m_fGoalPitch - angles.x );
	float flTargetYaw = bReturning ? angles.y : m_fGoalYaw;

	CASW_Sentry_Base *pBase = GetSentryBase();
	if ( bReturning )
	{
		CASW_Marine *pLastDisassembler = pBase ? CASW_Marine::AsMarine( pBase->m_hLastDisassembler ) : NULL;
		CASW_Player *pDisassemblePlayer = pLastDisassembler && pLastDisassembler->IsInhabited() && pLastDisassembler->GetUsingEntity() == pBase ? pLastDisassembler->GetCommander() : NULL;
		if ( pDisassemblePlayer )
		{
			float flDisassembleProgress = ( ( gpGlobals->curtime - pDisassemblePlayer->m_flUseKeyDownTime ) - 0.2f ) / ( ASW_USE_KEY_HOLD_SENTRY_TIME - 0.2f );
			flTargetPitch = flDisassembleProgress * -90.0f;
		}
	}

	float flPitchSpeed = ( bReturning ? 30.0f : 45.0f ) * gpGlobals->frametime;
	float flYawSpeed = ( bReturning ? 15.0f : 540.0f ) * gpGlobals->frametime;

	m_fAimPitch = ApproachAngle( flTargetPitch, m_fAimPitch, flPitchSpeed );
	m_fCameraYaw = ApproachAngle( flTargetYaw, m_fCameraYaw, flYawSpeed );

	int iAmmo = pBase ? pBase->GetAmmo() : 999999;
	// if we're doing an animation, assume it's the firing animation unless it's the first one defined (bindpose)
	if ( !SequenceLoops() && GetSequence() != 0 )
	{
		if ( IsSequenceFinished() )
		{
			// return to bindpose
			ResetSequence( 0 );
		}
		else
		{
			iAmmo++;
		}
	}

	if ( m_iPoseParamPitch != -1 )
		SetPoseParameter( m_iPoseParamPitch, AngleNormalize( m_fAimPitch ) );
	if ( m_iPoseParamYaw != -1 )
		SetPoseParameter( m_iPoseParamYaw, AngleNormalize( m_fCameraYaw - angles.y ) );
	if ( m_iPoseParamAmmoRemaining != -1 )
		SetPoseParameter( m_iPoseParamAmmoRemaining, iAmmo );
}

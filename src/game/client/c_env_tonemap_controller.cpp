//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "c_user_message_register.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"


extern bool g_bUseCustomAutoExposureMin;
extern bool g_bUseCustomAutoExposureMax;
extern bool g_bUseCustomBloomScale;
extern bool g_bUseCustomManualTonemapRate;
extern float g_flCustomAutoExposureMin;
extern float g_flCustomAutoExposureMax;
extern float g_flCustomBloomScale;
extern float g_flCustomBloomScaleMinimum;
extern float g_flCustomManualTonemapRate;

EHANDLE g_hTonemapControllerInUse = NULL;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_EnvTonemapController : public C_BaseEntity
{
	DECLARE_CLASS( C_EnvTonemapController, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();

	C_EnvTonemapController();

private:
	bool m_bUseCustomAutoExposureMin;
	bool m_bUseCustomAutoExposureMax;
	bool m_bUseCustomBloomScale;
	bool m_bUseCustomManualTonemapRate;
	float m_flCustomAutoExposureMin;
	float m_flCustomAutoExposureMax;
	float m_flCustomBloomScale;
	float m_flCustomBloomScaleMinimum;
	float m_flBloomExponent;
	float m_flBloomSaturation;
	float m_flCustomManualTonemapRate;
private:
	C_EnvTonemapController( const C_EnvTonemapController & );

	friend void GetTonemapSettingsFromEnvTonemapController( void );
	friend void __MsgFunc_EditTonemapSettings( bf_read &msg );
};

IMPLEMENT_CLIENTCLASS_DT( C_EnvTonemapController, DT_EnvTonemapController, CEnvTonemapController )
	RecvPropInt( RECVINFO(m_bUseCustomAutoExposureMin) ),
	RecvPropInt( RECVINFO(m_bUseCustomAutoExposureMax) ),
	RecvPropInt( RECVINFO(m_bUseCustomBloomScale) ),
	RecvPropInt( RECVINFO(m_bUseCustomManualTonemapRate) ),
	RecvPropFloat( RECVINFO(m_flCustomAutoExposureMin) ),
	RecvPropFloat( RECVINFO(m_flCustomAutoExposureMax) ),
	RecvPropFloat( RECVINFO(m_flCustomBloomScale) ),
	RecvPropFloat( RECVINFO(m_flCustomBloomScaleMinimum) ),
	RecvPropFloat( RECVINFO(m_flBloomExponent) ),
	RecvPropFloat( RECVINFO(m_flBloomSaturation) ),
	RecvPropFloat( RECVINFO(m_flCustomManualTonemapRate) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_EnvTonemapController::C_EnvTonemapController( void )
{
	m_bUseCustomAutoExposureMin = false;
	m_bUseCustomAutoExposureMax = false;
	m_bUseCustomBloomScale = false;
	m_bUseCustomManualTonemapRate = false;
	m_flCustomAutoExposureMin = 0;
	m_flCustomAutoExposureMax = 0;
	m_flCustomBloomScale = 0.0f;
	m_flCustomBloomScaleMinimum = 0.0f;
	m_flBloomExponent = 2.5f;
	m_flBloomSaturation = 1.0f;
	m_flCustomManualTonemapRate = 1.0f;
}

void GetTonemapSettingsFromEnvTonemapController( void )
{	
	C_BasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();
	if ( localPlayer )
	{
		C_EnvTonemapController *tonemapController = dynamic_cast< C_EnvTonemapController * >(localPlayer->m_hTonemapController.Get());
		if ( tonemapController != NULL )
		{	
			g_bUseCustomAutoExposureMin = tonemapController->m_bUseCustomAutoExposureMin;
			g_bUseCustomAutoExposureMax = tonemapController->m_bUseCustomAutoExposureMax;
			g_bUseCustomBloomScale = tonemapController->m_bUseCustomBloomScale;
			g_bUseCustomManualTonemapRate = tonemapController->m_bUseCustomManualTonemapRate;
			g_flCustomAutoExposureMin = tonemapController->m_flCustomAutoExposureMin;
			g_flCustomAutoExposureMax = tonemapController->m_flCustomAutoExposureMax;
			g_flCustomBloomScale = tonemapController->m_flCustomBloomScale;
			g_flCustomBloomScaleMinimum = tonemapController->m_flCustomBloomScaleMinimum;
			g_flCustomManualTonemapRate = tonemapController->m_flCustomManualTonemapRate;
			return;
		}
	}

	g_bUseCustomAutoExposureMin = false;
	g_bUseCustomAutoExposureMax = false;
	g_bUseCustomBloomScale = false;
	g_bUseCustomManualTonemapRate = false;
}

enum TonemapSettings_t
{
	TNMP_AUTOEXPOSURE = 0,
	TNMP_BLOOMSCALE,
	TNMP_TONEMAPRATE
};

void __MsgFunc_EditTonemapSettings( bf_read &msg )
{
	C_BasePlayer *localPlayer = C_BasePlayer::GetLocalPlayer();
	if ( !localPlayer )
		return;
	
	C_EnvTonemapController *tonemapController = dynamic_cast< C_EnvTonemapController * >(localPlayer->m_hTonemapController.Get());
	if ( !tonemapController )
		return;

	byte setting = msg.ReadByte();

	switch ( setting )
	{
		case TNMP_AUTOEXPOSURE:
		{
			tonemapController->m_bUseCustomAutoExposureMin = true;
			tonemapController->m_bUseCustomAutoExposureMax = true;
			tonemapController->m_flCustomAutoExposureMin = msg.ReadFloat();
			tonemapController->m_flCustomAutoExposureMax = msg.ReadFloat();
			
			return;
		}
		case TNMP_BLOOMSCALE:
		{
			tonemapController->m_bUseCustomBloomScale = true;
			tonemapController->m_flCustomBloomScale = msg.ReadFloat();

			return;
		}
		case TNMP_TONEMAPRATE:
		{
			tonemapController->m_bUseCustomManualTonemapRate = true;
			tonemapController->m_flCustomManualTonemapRate = msg.ReadFloat();

			return;
		}
		default:
			return;
	}
}
USER_MESSAGE_REGISTER( EditTonemapSettings );
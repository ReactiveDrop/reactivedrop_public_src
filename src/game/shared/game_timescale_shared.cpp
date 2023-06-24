//============ Copyright (c) Valve Corporation, All rights reserved. ============
#include "cbase.h"
#include "game_timescale_shared.h"

#ifdef CLIENT_DLL
	#include "c_user_message_register.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef CLIENT_DLL
ConVar rd_scale_rates( "rd_scale_rates", "1", FCVAR_NONE, "Automatically change cl_updaterate and cl_cmdrate during slow motion." );
#endif

CGameTimescale g_GameTimescale;
CGameTimescale* GameTimescale() { return &g_GameTimescale; }


CGameTimescale::CGameTimescale( void ) : CAutoGameSystemPerFrame( "CGameTimescale" )
{
	m_flStartBlendRealtime = 0.0f;
}

CGameTimescale::~CGameTimescale()
{
}


bool CGameTimescale::Init()
{
#ifdef CLIENT_DLL
	m_pUpdateRate = g_pCVar->FindVar( "cl_updaterate" );
	Assert( m_pUpdateRate );
	m_flBaseUpdateRate = m_pUpdateRate->GetFloat();
	m_pUpdateRate->InstallChangeCallback( &UpdateRateChangedManually );

	m_pCmdRate = g_pCVar->FindVar( "cl_cmdrate" );
	Assert( m_pCmdRate );
	m_flBaseCmdRate = m_pCmdRate->GetFloat();
	m_pCmdRate->InstallChangeCallback( &CmdRateChangedManually );

	m_bUpdatingRates = false;
#endif

	ResetTimescale();

	return true;
}

void CGameTimescale::Shutdown()
{
	ResetTimescale();
}

#ifdef CLIENT_DLL
	void CGameTimescale::Update( float frametime )
	{
		UpdateTimescale();
	}
#else
	void CGameTimescale::FrameUpdatePostEntityThink()
	{
		UpdateTimescale();
	}
#endif

void CGameTimescale::LevelInitPostEntity()
{
	ResetTimescale();
}

void CGameTimescale::LevelShutdownPostEntity()
{
	ResetTimescale();
}

void CGameTimescale::SetCurrentTimescale( float flTimescale )
{
	if ( m_flCurrentTimescale == flTimescale && m_flCurrentTimescale == engine->GetTimescale() )
		return;

	// No ramp in/out, just set it!
	m_flDesiredTimescale = flTimescale;
	m_flCurrentTimescale = m_flDesiredTimescale;

	m_flDurationRealTimeSeconds = 0.0f;
	m_nInterpolatorType = INTERPOLATOR_LINEAR;

	m_flStartBlendTime = 0.0f;
	m_flStartBlendRealtime = 0.0f;

#ifndef CLIENT_DLL
	engine->SetTimescale( m_flCurrentTimescale );

	// Pass the change info to the client so it can do prediction
	CReliableBroadcastRecipientFilter filter;
	UserMessageBegin( filter, "CurrentTimescale" );
		WRITE_FLOAT( m_flCurrentTimescale );
	MessageEnd();
#else
	if ( rd_scale_rates.GetBool() )
	{
		m_bUpdatingRates = true;
		m_pUpdateRate->SetValue( m_flBaseUpdateRate / flTimescale );
		m_pCmdRate->SetValue( m_flBaseCmdRate / flTimescale );
		m_bUpdatingRates = false;
	}
#endif
}

void CGameTimescale::SetDesiredTimescaleAtTime( float flDesiredTimescale, float flDurationRealTimeSeconds /*= 0.0f*/, Interpolators_e nInterpolatorType /*= INTERPOLATOR_LINEAR*/, float flStartBlendTime /*= 0.0f*/ )
{
	SetDesiredTimescale( flDesiredTimescale, flDurationRealTimeSeconds, nInterpolatorType, MAX( 0.0f, flStartBlendTime - gpGlobals->curtime ) );
}

void CGameTimescale::SetDesiredTimescale( float flDesiredTimescale, float flDurationRealTimeSeconds /*= 0.0f*/, Interpolators_e nInterpolatorType /*= INTERPOLATOR_LINEAR*/, float flDelayRealtime /*= 0.0f*/ )
{
	if ( m_flDesiredTimescale == flDesiredTimescale )
		return;

	m_flDesiredTimescale = flDesiredTimescale;
	m_flDurationRealTimeSeconds = flDurationRealTimeSeconds;
	m_nInterpolatorType = nInterpolatorType;

	m_flStartBlendTime = gpGlobals->curtime + flDelayRealtime * m_flCurrentTimescale;

	if ( gpGlobals->curtime >= m_flStartBlendTime )
	{
		m_flStartBlendRealtime = gpGlobals->realtime;
	}
	else
	{
		m_flStartBlendRealtime = 0.0f;
	}

	m_flStartTimescale = m_flCurrentTimescale;

#ifndef CLIENT_DLL
	// Pass the change info to the client so it can do prediction
	CReliableBroadcastRecipientFilter filter;
	UserMessageBegin( filter, "DesiredTimescale" );
		WRITE_FLOAT( m_flDesiredTimescale );
		WRITE_FLOAT( m_flDurationRealTimeSeconds );
		WRITE_BYTE( m_nInterpolatorType );
		WRITE_FLOAT( m_flStartBlendTime );
	MessageEnd();
#endif
}


void CGameTimescale::UpdateTimescale( void )
{
	if ( engine->IsPaused() )
		return;

	if ( m_flCurrentTimescale != m_flDesiredTimescale )
	{
		if ( gpGlobals->curtime >= m_flStartBlendTime )
		{
			if ( m_flStartBlendRealtime == 0.0f )
			{
				m_flStartBlendRealtime = gpGlobals->realtime;
			}

			float flInterp = 1.0f;

			if ( m_flDurationRealTimeSeconds > 0.0f )
			{
				flInterp = MIN( 1.0f, ( gpGlobals->realtime - m_flStartBlendRealtime ) / m_flDurationRealTimeSeconds );
			}

			switch ( m_nInterpolatorType )
			{
			case INTERPOLATOR_ACCEL:
				flInterp = Bias( flInterp, 0.25f );
				break;

			case INTERPOLATOR_DEACCEL:
				flInterp = Bias( flInterp, 0.75f );
				break;

			case INTERPOLATOR_EASE_IN_OUT:
				flInterp = Gain( flInterp, 0.75f );
				break;
			}

			m_flCurrentTimescale = m_flStartTimescale * ( 1.0f - flInterp ) + m_flDesiredTimescale * flInterp;

#ifdef CLIENT_DLL
			if ( rd_scale_rates.GetBool() && flInterp == 1.0f )
			{
				m_bUpdatingRates = true;
				m_pUpdateRate->SetValue( m_flBaseUpdateRate / MIN( m_flCurrentTimescale, m_flDesiredTimescale ) );
				m_pCmdRate->SetValue( m_flBaseCmdRate / MIN( m_flCurrentTimescale, m_flDesiredTimescale ) );
				m_bUpdatingRates = false;
			}
#endif
		}
	}

	if ( m_flCurrentTimescale != engine->GetTimescale() )
	{
		engine->SetTimescale( m_flCurrentTimescale );
	}
}

void CGameTimescale::ResetTimescale( void )
{
	m_flDesiredTimescale = 1.0f;
	m_flCurrentTimescale = 1.0f;

	m_flDurationRealTimeSeconds = 0.0f;
	m_nInterpolatorType = INTERPOLATOR_LINEAR;

	m_flStartBlendTime = 0.0f;
	m_flStartBlendRealtime = 0.0f;

	engine->SetTimescale( 1.0f );

#ifdef CLIENT_DLL
	if ( rd_scale_rates.GetBool() )
	{
		m_bUpdatingRates = true;
		m_pUpdateRate->SetValue( m_flBaseUpdateRate );
		m_pCmdRate->SetValue( m_flBaseCmdRate );
		m_bUpdatingRates = false;
	}
#endif
}


#ifdef CLIENT_DLL

void CGameTimescale::UpdateRateChangedManually( IConVar *var, const char *pOldValue, float flOldValue )
{
	if ( !g_GameTimescale.m_bUpdatingRates )
	{
		g_GameTimescale.m_flBaseUpdateRate = ConVarRef( var ).GetFloat();
	}
}

void CGameTimescale::CmdRateChangedManually( IConVar *var, const char *pOldValue, float flOldValue )
{
	if ( !g_GameTimescale.m_bUpdatingRates )
	{
		g_GameTimescale.m_flBaseCmdRate = ConVarRef( var ).GetFloat();
	}
}

void __MsgFunc_CurrentTimescale( bf_read &msg )
{
	GameTimescale()->SetCurrentTimescale( msg.ReadFloat() );
}
USER_MESSAGE_REGISTER( CurrentTimescale );

void __MsgFunc_DesiredTimescale( bf_read &msg )
{
	float flDesiredTimescale = msg.ReadFloat();
	float flDurationRealTimeSeconds = msg.ReadFloat();
	CGameTimescale::Interpolators_e nInterpolatorType = static_cast< CGameTimescale::Interpolators_e >( msg.ReadByte() );
	float flStartBlendTime = msg.ReadFloat();

	GameTimescale()->SetDesiredTimescaleAtTime( flDesiredTimescale, flDurationRealTimeSeconds, nInterpolatorType, flStartBlendTime );
}
USER_MESSAGE_REGISTER( DesiredTimescale );

#endif //#ifdef CLIENT_DLL
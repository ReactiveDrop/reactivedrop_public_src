//============ Copyright (c) Valve Corporation, All rights reserved. ============
#include "cbase.h"
#include "game_timescale_shared.h"

#ifdef CLIENT_DLL
	#include "c_user_message_register.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


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
	// reactivedrop: #iss-adrenaline-lag
	// 'Adrenaline lag' issue happens when users either use adrenaline or slow
	// motion happens during mission. Clients get the reduced update rate(usually
	// 21 updates per second) and sometimes packet loss, which is so huge it
	// makes client's game freeze for several seconds. 

	// A workaround for this issue is to change host_timescale cvar to the exactly
	// same value as engine->SetTimescale() call is set to.
	// This way there is no decrease in update rate and there is no loss during
	// slow motion. 
	// host_timescale only works as expected when sv_cheats 1, it can slow down
	// and speed up game time. Changing host_timescale when sv_cheats 0 results
	// in update rate change, without game time change
	// 
	ConVarRef host_timescale( "host_timescale" ); 
	ConVarRef sv_cheats( "sv_cheats" );
	if ( host_timescale.IsValid() && sv_cheats.IsValid() && !sv_cheats.GetBool() )
	{
		host_timescale.SetValue( m_flCurrentTimescale );
	}

	// Pass the change info to the client so it can do prediction
	CReliableBroadcastRecipientFilter filter;
	UserMessageBegin( filter, "CurrentTimescale" );
		WRITE_FLOAT( m_flCurrentTimescale );
	MessageEnd();
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
		}
	}

	if ( m_flCurrentTimescale != engine->GetTimescale() )
	{
#ifdef CLIENT_DLL
		// BenLubar(demo-timescale): set the demo timescale to the inverse of the timescale we want so it's not applied twice
		if ( engine->IsPlayingDemo() )
		{
			engine->ClientCmd_Unrestricted( UTIL_VarArgs( "demo_timescale %f\n", 1.0f / m_flCurrentTimescale ) );
		}
#endif
		engine->SetTimescale( m_flCurrentTimescale );
#ifndef CLIENT_DLL
		// reactivedrop: #iss-adrenaline-lag
		ConVarRef host_timescale( "host_timescale" );
		ConVarRef sv_cheats( "sv_cheats" );
		if ( host_timescale.IsValid() && sv_cheats.IsValid() && !sv_cheats.GetBool() )
		{
			host_timescale.SetValue( m_flCurrentTimescale );
		}
#endif
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

#ifndef CLIENT_DLL
	// reactivedrop: #iss-adrenaline-lag
	ConVarRef host_timescale( "host_timescale" );
	ConVarRef sv_cheats( "sv_cheats" );
	if ( host_timescale.IsValid() && sv_cheats.IsValid() && !sv_cheats.GetBool() )
	{
		host_timescale.SetValue( 1.0f );
	}
#endif 
}


#ifdef CLIENT_DLL

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
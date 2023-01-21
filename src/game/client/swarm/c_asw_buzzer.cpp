#include "cbase.h"
#include "c_AI_BaseNPC.h"
#include "soundenvelope.h"
#include "iasw_client_aim_target.h"
#include "c_asw_alien.h"
#include "c_asw_buzzer.h"
#include "c_asw_generic_emitter_entity.h"
#include "c_asw_fx.h"
#include "c_asw_player.h"
#include "baseparticleentity.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar glow_outline_color_alien;

// Buzzer is our flying poisoning alien (based on the hl2 manhack code)

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Buzzer, DT_ASW_Buzzer, CASW_Buzzer)
	RecvPropIntWithMinusOneFlag(RECVINFO(m_nEnginePitch1)),
	RecvPropFloat(RECVINFO(m_flEnginePitch1Time)),
END_RECV_TABLE()


C_ASW_Buzzer::C_ASW_Buzzer()
{
	m_GlowObject.SetColor( glow_outline_color_alien.GetColorAsVector() );
	m_GlowObject.SetAlpha( 0.55f );
	m_GlowObject.SetRenderFlags( false, false );
	m_GlowObject.SetFullBloomRender( true );

	m_pEngineSound1 = NULL;
}

C_ASW_Buzzer::~C_ASW_Buzzer()
{
	if ( m_pTrailEffect )
	{
		ParticleProp()->StopEmissionAndDestroyImmediately( m_pTrailEffect );
		m_pTrailEffect = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Start the buzzer's engine sound.
//-----------------------------------------------------------------------------
void C_ASW_Buzzer::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	if (( m_nEnginePitch1 <= 0 ) )
	{
		SoundShutdown();
	}
	else
	{
		SoundInit();
	}

	if ( type == DATA_UPDATE_CREATED )
	{
		if ( !m_pTrailEffect )
		{
			m_pTrailEffect = ParticleProp()->Create( "buzzer_trail", PATTACH_ABSORIGIN_FOLLOW );
		}
	}
}

void C_ASW_Buzzer::OnRestore()
{
	BaseClass::OnRestore();
	SoundInit();	
}


//-----------------------------------------------------------------------------
// Purpose: Start the buzzer's engine sound.
//-----------------------------------------------------------------------------
void C_ASW_Buzzer::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove();
	SoundShutdown();

	if ( m_pTrailEffect )
	{
		ParticleProp()->StopEmission( m_pTrailEffect, false, true, false );
		m_pTrailEffect = NULL;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Start the buzzer's engine sound.
//-----------------------------------------------------------------------------
void C_ASW_Buzzer::SoundInit( void )
{
	if (( m_nEnginePitch1 <= 0 ) )
		return;

	// play an engine start sound!!
	CPASAttenuationFilter filter( this );

	// Bring up the engine looping sound.
	if( !m_pEngineSound1 )
	{
		m_pEngineSound1 = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_Buzzer.Idle" );
		CSoundEnvelopeController::GetController().Play( m_pEngineSound1, 0.0, m_nEnginePitch1 );
		CSoundEnvelopeController::GetController().SoundChangeVolume( m_pEngineSound1, 0.7, 2.0 );
	}
}

void C_ASW_Buzzer::SoundShutdown(void)
{	
	if ( m_pEngineSound1 )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pEngineSound1 );
		m_pEngineSound1 = NULL;
	}
}

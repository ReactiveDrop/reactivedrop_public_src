#include "cbase.h"
#include "c_asw_parasite.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar rd_alien_vertical_shadow_jumping;
extern ConVar rd_alien_vertical_shadow_distance;

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Parasite, DT_ASW_Parasite, CASW_Parasite)
	RecvPropBool(RECVINFO(m_bStartIdleSound)),
	RecvPropBool(RECVINFO(m_bDoEggIdle)),
	RecvPropBool(RECVINFO(m_bInfesting)),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_ASW_Parasite )

END_PREDICTION_DATA()

C_ASW_Parasite::C_ASW_Parasite()
{
	m_pLoopingSound = NULL;
	m_bStartIdleSound = false;
	m_bDoEggIdle = false;
	m_bInfesting = false;
}


C_ASW_Parasite::~C_ASW_Parasite()
{

}

void C_ASW_Parasite::OnRestore()
{
	BaseClass::OnRestore();
	if (m_bStartIdleSound)
	{
		SoundInit();
	}
}

void C_ASW_Parasite::OnDataChanged( DataUpdateType_t updateType )
{
	if (m_bStartIdleSound)
	{
		SoundInit();
	}

	BaseClass::OnDataChanged( updateType );
}

void C_ASW_Parasite::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
	SoundShutdown();
}

void C_ASW_Parasite::SoundInit()
{
	CPASAttenuationFilter filter( this );

	// Start the parasite's looping sound
	if( !m_pLoopingSound )
	{
		m_pLoopingSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_Parasite.Idle" );
		CSoundEnvelopeController::GetController().Play( m_pLoopingSound, 0.0, 100 );
		CSoundEnvelopeController::GetController().SoundChangeVolume( m_pLoopingSound, 1.0, 1.0 );
	}
}

void C_ASW_Parasite::SoundShutdown()
{	
	if ( m_pLoopingSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pLoopingSound );
		m_pLoopingSound = NULL;
	}
}

bool C_ASW_Parasite::IsAimTarget()
{
	return BaseClass::IsAimTarget() && !m_bDoEggIdle && !m_bInfesting;
}

ShadowType_t C_ASW_Parasite::ShadowCastType()
{
	if ( rd_alien_vertical_shadow_jumping.GetBool() && IsJumping() )
	{
		return BaseClass::BaseClass::ShadowCastType();
	}

	return BaseClass::ShadowCastType();
}

bool C_ASW_Parasite::GetShadowCastDistance( float *pDistance, ShadowType_t shadowType ) const
{
	if ( rd_alien_vertical_shadow_jumping.GetBool() && IsJumping() )
	{
		*pDistance = rd_alien_vertical_shadow_distance.GetFloat();
		return true;
	}

	return BaseClass::GetShadowCastDistance( pDistance, shadowType );
}

bool C_ASW_Parasite::GetShadowCastDirection( Vector *pDirection, ShadowType_t shadowType ) const
{
	if ( rd_alien_vertical_shadow_jumping.GetBool() && IsJumping() )
	{
		pDirection->Init( 0, 0, -1 );
		return true;
	}

	return BaseClass::GetShadowCastDirection( pDirection, shadowType );
}

bool C_ASW_Parasite::IsJumping() const
{
	if ( m_bDoEggIdle || m_bInfesting )
		return false;

	C_ASW_Parasite *pParasite = const_cast< C_ASW_Parasite * >( this );
	if ( pParasite->GetSequenceActivity( pParasite->GetSequence() ) == ACT_RANGE_ATTACK1 )
		return true;

	return false;
}

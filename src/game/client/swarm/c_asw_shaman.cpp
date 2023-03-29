#include "cbase.h"
#include "c_baseentity.h"
#include "c_asw_shaman.h"
#include "c_asw_clientragdoll.h"
#include "asw_fx_shared.h"
#include "functionproxy.h"
#include "imaterialproxydict.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Shaman, DT_ASW_Shaman, CASW_Shaman)
	RecvPropEHandle		( RECVINFO( m_hHealingTarget ) ),
END_RECV_TABLE()

C_ASW_Shaman::C_ASW_Shaman()
{
	m_bIsPlayingBeamSound = false;
}


C_ASW_Shaman::~C_ASW_Shaman()
{
	
}

C_BaseAnimating * C_ASW_Shaman::BecomeRagdollOnClient( void )
{
	if ( m_pHealEffect )
	{
		m_pHealEffect->StopEmission( false, true, false );
		m_pHealEffect = NULL;
	}

	return BaseClass::BecomeRagdollOnClient();

}

C_ClientRagdoll *C_ASW_Shaman::CreateClientRagdoll( bool bRestoring )
{
	return new C_ASW_ClientRagdoll( bRestoring );
}

void C_ASW_Shaman::ClientThink()
{
	BaseClass::ClientThink();

	UpdateEffects();
}

void C_ASW_Shaman::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();

	if ( m_pHealEffect )
	{
		m_pHealEffect->StopEmission();
		m_pHealEffect = NULL;
	}

	StopBeamSound( false );
}

void C_ASW_Shaman::UpdateEffects()
{
	if ( !m_hHealingTarget.Get() || GetHealth() <= 0 )
	{
		if ( m_pHealEffect )
		{
			m_pHealEffect->StopEmission();
			m_pHealEffect = NULL;
		}

		StopBeamSound( true );

		return;
	}

	if ( m_pHealEffect )
	{
		if ( m_pHealEffect->GetControlPointEntity( 1 ) != m_hHealingTarget.Get() )
		{
			m_pHealEffect->StopEmission();
			m_pHealEffect = NULL;

			StopBeamSound( true );
		}
	}
			
	if ( !m_pHealEffect )
	{
		m_pHealEffect = ParticleProp()->Create( "shaman_heal_attach", PATTACH_POINT_FOLLOW, "nozzle" );	// "heal_receiver"

		StartBeamSound( true );
	}

	Assert( m_pHealEffect );

	if ( m_pHealEffect->GetControlPointEntity( 1 ) == NULL )
	{
		C_BaseEntity *pTarget = m_hHealingTarget.Get();
		Vector vOffset( 0.0f, 0.0f, pTarget->WorldSpaceCenter().z - pTarget->GetAbsOrigin().z );

		ParticleProp()->AddControlPoint( m_pHealEffect, 1, pTarget, PATTACH_ABSORIGIN_FOLLOW, NULL, vOffset );
		m_pHealEffect->SetControlPointOrientation( 0, pTarget->Forward(), -pTarget->Left(), pTarget->Up() );
	}
}

void C_ASW_Shaman::StartBeamSound( bool bIsHeal )
{
	if ( !m_bIsPlayingBeamSound )
	{
		EmitSound( bIsHeal ? "ASW_Mender.BeamHeal" : "ASW_Mender.BeamLeech" );

		m_bIsPlayingBeamSound = true;
	}
}

void C_ASW_Shaman::StopBeamSound( bool bPlayEndSound )
{
	if ( m_bIsPlayingBeamSound )
	{
		StopSound( "ASW_Mender.BeamHeal" );
		StopSound( "ASW_Mender.BeamLeech" );

		m_bIsPlayingBeamSound = false;

		if ( bPlayEndSound )
		{
			EmitSound( "ASW_Mender.BeamStop" );
		}
	}
}

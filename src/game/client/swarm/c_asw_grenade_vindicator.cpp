#include "cbase.h"
#include "c_asw_grenade_vindicator.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Grenade_Vindicator, DT_ASW_Grenade_Vindicator, CASW_Grenade_Vindicator )
	
END_RECV_TABLE();

C_ASW_Grenade_Vindicator::C_ASW_Grenade_Vindicator()
{
	m_pSmokeTrail = NULL;
}

void C_ASW_Grenade_Vindicator::OnDataChanged(DataUpdateType_t updateType)
{
	if ( updateType == DATA_UPDATE_CREATED )
		CreateSmokeTrail();
}

void C_ASW_Grenade_Vindicator::CreateSmokeTrail()
{
	if ( m_pSmokeTrail )
		return;

	m_pSmokeTrail = ParticleProp()->Create( "rocket_trail_small", PATTACH_ABSORIGIN_FOLLOW, -1, Vector( 0, 0, 0 ) );
}

void C_ASW_Grenade_Vindicator::UpdateOnRemove()
{
	if ( m_pSmokeTrail )
	{
		m_pSmokeTrail->StopEmission();
		m_pSmokeTrail = NULL;
	}
}
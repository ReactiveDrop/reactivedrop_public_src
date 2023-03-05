#include "cbase.h"
#include "precache_register.h"
#include "particles_simple.h"
#include "iefx.h"
#include "dlight.h"
#include "view.h"
#include "fx.h"
#include "clientsideeffects.h"
#include "c_pixel_visibility.h"
#include "asw_shareddefs.h"
#include "c_asw_gas_grenade_projectile.h"
#include "soundenvelope.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//Precahce the effects
//PRECACHE_REGISTER_BEGIN( GLOBAL, ASWPrecacheEffectFlares )
//PRECACHE_REGISTER_END()


IMPLEMENT_CLIENTCLASS_DT( C_ASW_Gas_Grenade_Projectile, DT_ASW_Gas_Grenade_Projectile, CASW_Gas_Grenade_Projectile )
	RecvPropFloat( RECVINFO( m_flTimeBurnOut ) ),
	RecvPropFloat( RECVINFO( m_flScale ) ),
	RecvPropInt( RECVINFO( m_bSmoke ) ),
	RecvPropDataTable( RECVINFO_DT( m_ProjectileData ), 0, &REFERENCE_RECV_TABLE( DT_RD_ProjectileData ) ),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_ASW_Gas_Grenade_Projectile::C_ASW_Gas_Grenade_Projectile()
{
	m_flTimeBurnOut	= 0.0f;
	m_flTimeCreated = gpGlobals->curtime;

	m_bSmoke		= true;
	m_bStopped		= false;

	//SetDynamicallyAllocated( false );
	m_queryHandle = 0;
}

C_ASW_Gas_Grenade_Projectile::~C_ASW_Gas_Grenade_Projectile( void )
{
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : state - 
//-----------------------------------------------------------------------------
void C_ASW_Gas_Grenade_Projectile::NotifyShouldTransmit( ShouldTransmitState_t state )
{
	if ( state == SHOULDTRANSMIT_END )
	{
		AddEffects( EF_NODRAW );
	}
	else if ( state == SHOULDTRANSMIT_START )
	{
		RemoveEffects( EF_NODRAW );
	}

	BaseClass::NotifyShouldTransmit( state );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_ASW_Gas_Grenade_Projectile::OnDataChanged( DataUpdateType_t updateType )
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink(gpGlobals->curtime);
		//SetSortOrigin( GetAbsOrigin() );
	}

	BaseClass::OnDataChanged( updateType );
}

const Vector& C_ASW_Gas_Grenade_Projectile::GetEffectOrigin()
{
	static Vector s_vecEffectPos;
	Vector forward, right, up;
	AngleVectors(GetAbsAngles(), &forward, &right, &up);
	s_vecEffectPos = GetAbsOrigin() + up * 5;
	return s_vecEffectPos;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : timeDelta - 
//-----------------------------------------------------------------------------
void C_ASW_Gas_Grenade_Projectile::ClientThink( void )
{
	float baseScale = m_flScale;

	//Account for fading out
	if ( ( m_flTimeBurnOut != -1.0f ) && ( ( m_flTimeBurnOut - gpGlobals->curtime ) <= 10.0f ) )
	{
		baseScale *= ( ( m_flTimeBurnOut - gpGlobals->curtime ) / 10.0f );
	}

	if ( baseScale < 0.01f )
		return;

	if ( m_pTrailEffect.GetObject() == NULL && !m_bStopped )
	{
		m_pTrailEffect = ParticleProp()->Create( "gas_trail_small", PATTACH_ABSORIGIN_FOLLOW, -1, GetEffectOrigin() - GetAbsOrigin() );
		Assert( m_pTrailEffect.GetObject() != NULL );
	}

	if ( !m_bStopped && gpGlobals->curtime - m_flTimeCreated > 2.0f )
	{
		ParticleProp()->StopEmission( m_pTrailEffect );
		m_bStopped = true;
	}

	SetNextClientThink(gpGlobals->curtime + 0.1f);
}

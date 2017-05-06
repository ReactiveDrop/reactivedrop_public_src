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
END_RECV_TABLE()

// flares maintain a linked list of themselves, for quick checking for autoaim
//C_ASW_Gas_Grenade_Projectile* g_pHeadFlare = NULL;

extern ConVar asw_flare_r;
extern ConVar asw_flare_g;
extern ConVar asw_flare_b;

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
C_ASW_Gas_Grenade_Projectile::C_ASW_Gas_Grenade_Projectile()
{
	m_flTimeBurnOut	= 0.0f;

	m_bSmoke		= true;

	//SetDynamicallyAllocated( false );
	m_queryHandle = 0;

	m_pFlareEffect = NULL;
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
		SoundInit();
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
//void C_ASW_Gas_Grenade_Projectile::Update( float timeDelta )
void C_ASW_Gas_Grenade_Projectile::ClientThink( void )
{
	if ( m_pFlareEffect.GetObject() == NULL )
	{	
		//m_flTimePulse = gpGlobals->curtime + asw_buffgrenade_pulse_interval.GetFloat();

		m_pFlareEffect = ParticleProp()->Create( "flare_fx_main", PATTACH_ABSORIGIN_FOLLOW, -1, GetEffectOrigin() - GetAbsOrigin() );
		//flare_fx_main
	}

	float	baseScale = m_flScale;

	//Account for fading out
	if ( ( m_flTimeBurnOut != -1.0f ) && ( ( m_flTimeBurnOut - gpGlobals->curtime ) <= 10.0f ) )
	{
		baseScale *= ( ( m_flTimeBurnOut - gpGlobals->curtime ) / 10.0f );

		CSoundEnvelopeController::GetController().SoundChangeVolume( m_pBurnSound, clamp<float>(0.6f * baseScale, 0.0f, 0.6f), 0 );
	}

	if ( baseScale < 0.01f )
		return;

	SetNextClientThink(gpGlobals->curtime + 0.1f);
}

void C_ASW_Gas_Grenade_Projectile::OnRestore()
{
	BaseClass::OnRestore();
	SoundInit();	
}

void C_ASW_Gas_Grenade_Projectile::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
	SoundShutdown();
}

void C_ASW_Gas_Grenade_Projectile::SoundInit()
{
	// play flare start sound!!
	CPASAttenuationFilter filter( this );

	EmitSound("ASW_Flare.IgniteFlare");

	// Bring up the flare burning loop sound
	if( !m_pBurnSound )
	{
		m_pBurnSound = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_Flare.FlareLoop" );
		CSoundEnvelopeController::GetController().Play( m_pBurnSound, 0.0, 100 );
		CSoundEnvelopeController::GetController().SoundChangeVolume( m_pBurnSound, 0.6, 2.0 );
	}
}

void C_ASW_Gas_Grenade_Projectile::SoundShutdown()
{	
	if ( m_pBurnSound )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pBurnSound );
		m_pBurnSound = NULL;
	}
}

#include "cbase.h"
#include "c_asw_inhabitable_npc.h"
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "game_timescale_shared.h"
#include "c_te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar rd_highlight_active_character( "rd_highlight_active_character", "0", FCVAR_ARCHIVE );
extern ConVar asw_controls;

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Inhabitable_NPC, DT_ASW_Inhabitable_NPC, CASW_Inhabitable_NPC )
	RecvPropEHandle( RECVINFO( m_Commander ) ),
	RecvPropEHandle( RECVINFO( m_hUsingEntity ) ),
	RecvPropVector( RECVINFO( m_vecFacingPointFromServer ) ),
	RecvPropBool( RECVINFO( m_bInhabited ) ),
	RecvPropBool( RECVINFO( m_bWalking ) ),
	RecvPropIntWithMinusOneFlag( RECVINFO( m_iControlsOverride ) ),
	RecvPropInt( RECVINFO( m_iHealth ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_ASW_Inhabitable_NPC )
	DEFINE_FIELD( m_nOldButtons, FIELD_INTEGER ),
END_PREDICTION_DATA()

C_ASW_Inhabitable_NPC::C_ASW_Inhabitable_NPC() :
	m_GlowObject( this ),
	m_MotionBlurObject( this, 0.0f )
{
	m_fRedNamePulse = 0;
	m_bRedNamePulseUp = true;

	m_nOldButtons = 0;
	m_bInhabited = false;
	m_iControlsOverride = -1;

	m_surfaceProps = 0;
	m_pSurfaceData = NULL;
	m_surfaceFriction = 1.0f;
	m_chTextureType = m_chPreviousTextureType = 0;
}

C_ASW_Inhabitable_NPC::~C_ASW_Inhabitable_NPC()
{
}

bool C_ASW_Inhabitable_NPC::IsInhabited()
{
	return m_bInhabited;
}

C_ASW_Player *C_ASW_Inhabitable_NPC::GetCommander() const
{
	return m_Commander.Get();
}

const char *C_ASW_Inhabitable_NPC::GetPlayerName() const
{
	if ( GetCommander() )
	{
		return GetCommander()->GetPlayerName();
	}

	return BaseClass::GetPlayerName();
}

void C_ASW_Inhabitable_NPC::PostDataUpdate( DataUpdateType_t updateType )
{
	bool bPredict = ShouldPredict();
	if ( bPredict )
	{
		SetSimulatedEveryTick( true );
	}
	else
	{
		SetSimulatedEveryTick( false );

		// estimate velocity for non local players
		float flTimeDelta = m_flSimulationTime - m_flOldSimulationTime;
		if ( flTimeDelta > 0 && !IsEffectActive( EF_NOINTERP ) )
		{
			Vector newVelo = ( GetNetworkOrigin() - GetOldOrigin() ) / flTimeDelta;
			SetAbsVelocity( newVelo );
		}
	}

	// if player has switched into this marine, set it to be prediction eligible
	if ( bPredict )
	{
		// C_BaseEntity assumes we're networking the entity's angles, so pretend that it
		// networked the same value we already have.
		//SetNetworkAngles( GetLocalAngles() );
		SetPredictionEligible( true );
	}
	else
	{
		SetPredictionEligible( false );
	}

	BaseClass::PostDataUpdate( updateType );

	if ( GetPredictable() && !bPredict )
	{
		MDLCACHE_CRITICAL_SECTION();
		ShutdownPredictable();
	}

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

bool C_ASW_Inhabitable_NPC::ShouldPredict()
{
	return C_BasePlayer::IsLocalPlayer( GetCommander() ) && IsInhabited();
}

C_BasePlayer *C_ASW_Inhabitable_NPC::GetPredictionOwner()
{
	return GetCommander();
}

void C_ASW_Inhabitable_NPC::InitPredictable( C_BasePlayer *pOwner )
{
	SetLocalVelocity( vec3_origin );
	BaseClass::InitPredictable( pOwner );
}

void C_ASW_Inhabitable_NPC::ClientThink()
{
	BaseClass::ClientThink();

	m_vecLastRenderedPos = WorldSpaceCenter();
	m_vecAutoTargetRadiusPos = GetLocalAutoTargetRadiusPos();

	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( rd_highlight_active_character.GetBool() && pPlayer && pPlayer->GetViewNPC() == this )
	{
		m_GlowObject.SetRenderFlags( true, true );
	}
	else if ( IsAlien() && pPlayer && pPlayer->IsSniperScopeActive() )
	{
		m_GlowObject.SetRenderFlags( true, true );
	}
	else
	{
		m_GlowObject.SetRenderFlags( false, false );
	}
}

const Vector &C_ASW_Inhabitable_NPC::GetFacingPoint()
{
	if ( m_vecFacingPointFromServer != vec3_origin )
	{
		return m_vecFacingPointFromServer;
	}

	return m_vecFacingPoint;
}

void C_ASW_Inhabitable_NPC::SetFacingPoint( const Vector & vec, float fDuration )
{
	m_vecFacingPoint = vec;
	m_fStopFacingPointTime = gpGlobals->curtime + fDuration;
}

C_ASW_Weapon *C_ASW_Inhabitable_NPC::GetActiveASWWeapon( void ) const
{
	return assert_cast< C_ASW_Weapon * >( GetActiveWeapon() );
}

C_ASW_Weapon *C_ASW_Inhabitable_NPC::GetASWWeapon( int i ) const
{
	return assert_cast< C_ASW_Weapon * >( GetWeapon( i ) );
}

// when marine's health falls below this, name starts to blink red
#define MARINE_NAME_PULSE_THRESHOLD 0.5f
void C_ASW_Inhabitable_NPC::TickRedName( float delta )
{
	// deltatime should be normal regardless of slowmo
	float fTimeScale = GameTimescale()->GetCurrentTimescale();

	delta *= ( 1.0f / fTimeScale );

	float fHealth = ( float )GetHealth() / ( float )GetMaxHealth();

	if ( fHealth > MARINE_NAME_PULSE_THRESHOLD || fHealth <= 0 )
	{
		m_fRedNamePulse -= delta * 2;	// take 0.5 seconds to fade completely to normal
	}
	else
	{
		float drate = ( ( MARINE_NAME_PULSE_THRESHOLD - fHealth ) / 0.1f ) + 2.0f;
		if ( m_bRedNamePulseUp )
		{
			m_fRedNamePulse += drate * delta * 0.5f;
		}
		else
		{
			m_fRedNamePulse -= drate * delta * 0.5f;
		}
		// how quick should we pulse?  at 60, once per second  i.e. 2d
		// at 0, 4 times a second?  i.e. 8d
	}

	if ( m_fRedNamePulse <= 0 )
	{
		m_fRedNamePulse = 0;
		m_bRedNamePulseUp = true;
	}
	if ( m_fRedNamePulse >= 1.0f )
	{
		m_fRedNamePulse = 1.0f;
		m_bRedNamePulseUp = false;
	}
}

float C_ASW_Inhabitable_NPC::MaxSpeed()
{
	return 300;
}

float C_ASW_Inhabitable_NPC::GetBasePlayerYawRate()
{
	extern ConVar asw_marine_linear_turn_rate;
	return asw_marine_linear_turn_rate.GetFloat();
}

ASW_Controls_t C_ASW_Inhabitable_NPC::GetASWControls()
{
	if ( m_iControlsOverride >= 0 )
	{
		return ( ASW_Controls_t )m_iControlsOverride.Get();
	}

	return ( ASW_Controls_t )asw_controls.GetInt();
}

void C_ASW_Inhabitable_NPC::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	const char *tracer = "ASWUTracer";
	if ( GetActiveASWWeapon() )
		tracer = GetActiveASWWeapon()->GetUTracerType();

	CEffectData data;
	data.m_vOrigin = tr.endpos;
	data.m_hEntity = this;
	data.m_nMaterial = m_iDamageAttributeEffects;

	DispatchEffect( tracer, data );
}

void C_ASW_Inhabitable_NPC::MakeUnattachedTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	const char *tracer = "ASWUTracerUnattached";

	CEffectData data;
	data.m_vOrigin = tr.endpos;
	data.m_hEntity = this;
	data.m_vStart = vecTracerSrc;

	DispatchEffect( tracer, data );
}

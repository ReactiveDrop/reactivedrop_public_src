#include "cbase.h"
#include "c_asw_inhabitable_npc.h"
#include "c_asw_player.h"
#include "c_asw_weapon.h"
#include "game_timescale_shared.h"
#include "c_te_effect_dispatch.h"
#include "c_asw_fx.h"
#include "asw_util_shared.h"

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
	RecvPropVector( RECVINFO( m_vecBaseVelocity ) ),
	RecvPropBool( RECVINFO( m_bElectroStunned ) ),
	RecvPropBool( RECVINFO( m_bOnFire ) ),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_ASW_Inhabitable_NPC )
	DEFINE_FIELD( m_nOldButtons, FIELD_INTEGER ),
	DEFINE_PRED_FIELD_TOL( m_vecBaseVelocity, FIELD_VECTOR, FTYPEDESC_INSENDTABLE, 0.05 ),
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

	m_bOnFire = false;
	m_bElectroStunned = false;
	m_fNextElectroStunEffect = 0;
	m_pBurningEffect = NULL;
}

C_ASW_Inhabitable_NPC::~C_ASW_Inhabitable_NPC()
{
	m_bOnFire = false;
	UpdateFireEmitters();
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
	// If this entity was new, then latch in various values no matter what.
	if ( updateType == DATA_UPDATE_CREATED )
	{
		// We want to think every frame.
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

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
}

void C_ASW_Inhabitable_NPC::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
	m_bOnFire = false;
	UpdateFireEmitters();
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

	if ( GetHealth() > 0 && m_bElectroStunned && m_fNextElectroStunEffect <= gpGlobals->curtime )
	{
		// apply electro stun effect
		HACK_GETLOCALPLAYER_GUARD( "C_ASW_Alien::ClientThink FX_ElectroStun" );
		FX_ElectroStun( this );
		m_fNextElectroStunEffect = gpGlobals->curtime + RandomFloat( 0.3, 1.0 );
		//Msg( "%f - ElectroStunEffect\n", gpGlobals->curtime );
	}

	UpdateFireEmitters();
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

Vector C_ASW_Inhabitable_NPC::Weapon_ShootPosition()
{
	Vector right;
	GetVectors( NULL, &right, NULL );

	// TODO
	return GetAbsOrigin() + Vector( 0, 0, 34 ) + 8 * right;
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

void C_ASW_Inhabitable_NPC::UpdateFireEmitters( void )
{
	bool bOnFire = ( m_bOnFire.Get() && !IsEffectActive( EF_NODRAW ) );
	if ( bOnFire != m_bClientOnFire )
	{
		m_bClientOnFire = bOnFire;
		if ( m_bClientOnFire )
		{
			if ( !m_pBurningEffect )
			{
				m_pBurningEffect = UTIL_ASW_CreateFireEffect( this );
			}
			EmitSound( "ASWFire.BurningFlesh" );
		}
		else
		{
			if ( m_pBurningEffect )
			{
				ParticleProp()->StopEmission( m_pBurningEffect );
				m_pBurningEffect = NULL;
			}
			StopSound( "ASWFire.BurningFlesh" );
			if ( C_BaseEntity::IsAbsQueriesValid() )
				EmitSound( "ASWFire.StopBurning" );
		}
	}
}

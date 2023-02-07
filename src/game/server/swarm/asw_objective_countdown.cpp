#include "cbase.h"
#include "asw_objective_countdown.h"
#include "asw_gamerules.h"
#include "asw_game_resource.h"
#include "asw_marine_resource.h"
#include "asw_marine.h"
#include "triggers.h"
#include "util.h"
#include "te_effect_dispatch.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_objective_countdown, CASW_Objective_Countdown );

IMPLEMENT_SERVERCLASS_ST( CASW_Objective_Countdown, DT_ASW_Objective_Countdown )
	SendPropTime( SENDINFO( m_fCountdownFinishTime ) ),
	SendPropStringT( SENDINFO( m_szWarningText ) ),
	SendPropStringT( SENDINFO( m_szCaptionText ) ),
	SendPropStringT( SENDINFO( m_szSound60 ) ),
	SendPropStringT( SENDINFO( m_szSound30 ) ),
	SendPropStringT( SENDINFO( m_szSound10 ) ),
	SendPropStringT( SENDINFO( m_szSoundFail ) ),
	SendPropStringT( SENDINFO( m_szSoundFailLF ) ),
	SendPropInt( SENDINFO( m_FailColor ), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt32 ),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Objective_Countdown )
	DEFINE_FIELD( m_bCountdownStarted, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fCountdownFinishTime, FIELD_TIME ),
	DEFINE_KEYFIELD( m_fCountdownLength, FIELD_FLOAT, "CountdownLength" ),
	DEFINE_KEYFIELD( m_szWarningText, FIELD_STRING, "WarningText" ),
	DEFINE_KEYFIELD( m_szCaptionText, FIELD_STRING, "CaptionText" ),
	DEFINE_KEYFIELD( m_szSound60, FIELD_SOUNDNAME, "Sound60" ),
	DEFINE_KEYFIELD( m_szSound30, FIELD_SOUNDNAME, "Sound30" ),
	DEFINE_KEYFIELD( m_szSound10, FIELD_SOUNDNAME, "Sound10" ),
	DEFINE_KEYFIELD( m_szSoundFail, FIELD_SOUNDNAME, "SoundFail" ),
	DEFINE_KEYFIELD( m_szSoundFailLF, FIELD_SOUNDNAME, "SoundFailLF" ),
	DEFINE_KEYFIELD( m_FailColor, FIELD_COLOR32, "FailColor" ),

	DEFINE_THINKFUNC( ExplodeLevel ),
	DEFINE_THINKFUNC( FailMission ),

	DEFINE_INPUTFUNC( FIELD_VOID, "StartCountdown", InputStartCountdown ),
	DEFINE_INPUTFUNC( FIELD_VOID, "CancelCountdown", InputCancelCountdown ),
	DEFINE_OUTPUT( m_OnCountdownFailed, "OnCountdownFailed" ),
END_DATADESC()


CASW_Objective_Countdown::CASW_Objective_Countdown()
{
	m_bVisible = false;
	m_bOptional = true;
	m_fCountdownFinishTime = 0;
	m_szWarningText = NULL_STRING;
	m_szCaptionText = NULL_STRING;
	m_szSound60 = NULL_STRING;
	m_szSound30 = NULL_STRING;
	m_szSound10 = NULL_STRING;
	m_szSoundFail = NULL_STRING;
	m_szSoundFailLF = NULL_STRING;
	m_FailColor.Init( 255, 255, 255, 255 );
}

void CASW_Objective_Countdown::Spawn()
{
	BaseClass::Spawn();

	if ( m_szWarningText.Get() == NULL_STRING )
		m_szWarningText = AllocPooledString( "#asw_nuclear_warning" );
	if ( m_szCaptionText.Get() == NULL_STRING )
		m_szCaptionText = AllocPooledString( "#asw_nuclear_detonationi" );
	if ( m_szSound60.Get() == NULL_STRING )
		m_szSound60 = AllocPooledString( "ASWCountdown.Countdown60" );
	if ( m_szSound30.Get() == NULL_STRING )
		m_szSound30 = AllocPooledString( "ASWCountdown.Countdown30" );
	if ( m_szSound10.Get() == NULL_STRING )
		m_szSound10 = AllocPooledString( "ASWCountdown.Countdown10" );
	if ( m_szSoundFail.Get() == NULL_STRING )
		m_szSoundFail = AllocPooledString( "ASW.WarheadExplosion" );
	if ( m_szSoundFailLF.Get() == NULL_STRING )
		m_szSoundFailLF = AllocPooledString( "ASW.WarheadExplosionLF" );

	Precache();
}

void CASW_Objective_Countdown::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( STRING( m_szSound60.Get() ) );
	PrecacheScriptSound( STRING( m_szSound30.Get() ) );
	PrecacheScriptSound( STRING( m_szSound10.Get() ) );
	PrecacheScriptSound( STRING( m_szSoundFail.Get() ) );
	PrecacheScriptSound( STRING( m_szSoundFailLF.Get() ) );
}

CASW_Objective_Countdown::~CASW_Objective_Countdown()
{
}

void CASW_Objective_Countdown::InputStartCountdown( inputdata_t &inputdata )
{
	if ( !m_bCountdownStarted )
	{
		m_bVisible = true;
		m_bCountdownStarted = true;
		m_bOptional = false;
		m_fCountdownFinishTime = gpGlobals->curtime + m_fCountdownLength;
		SetThink( &CASW_Objective_Countdown::ExplodeLevel );
		SetNextThink( m_fCountdownFinishTime );
		SetComplete( false );
	}
}

void CASW_Objective_Countdown::InputCancelCountdown( inputdata_t &inputdata )
{
	if ( m_bCountdownStarted )
	{
		m_fCountdownFinishTime = 0;
		m_bCountdownStarted = false;
		SetThink( NULL );
		SetComplete( true );
	}
}

void CASW_Objective_Countdown::ExplodeLevel()
{
	m_OnCountdownFailed.FireOutput( this, this );

	// spawn clientside effect to make explosion sound + graphics
	CEffectData	data;
	data.m_nEntIndex = entindex();
	CReliableBroadcastRecipientFilter filter;
	DispatchEffect( filter, 0.0, "ASWExplodeMap", data );

	SetThink( &CASW_Objective_Countdown::FailMission );
	SetNextThink( gpGlobals->curtime + 7 );
}

void CASW_Objective_Countdown::FailMission()
{
	ASWGameRules()->ExplodedLevel( this );
}

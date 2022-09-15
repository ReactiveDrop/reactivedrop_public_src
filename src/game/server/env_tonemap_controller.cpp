//====== Copyright � 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "env_tonemap_controller.h"
#include "baseentity.h"
#include "entityoutput.h"
#include "convar.h"
#include "triggers.h"
#ifdef INFESTED_DLL
#include "asw_inhabitable_npc.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


ConVar mat_hdr_tonemapscale( "mat_hdr_tonemapscale", "1.0", FCVAR_CHEAT, "The HDR tonemap scale. 1 = Use autoexposure, 0 = eyes fully closed, 16 = eyes wide open." );

// 0 - eyes fully closed / fully black
// 1 - nominal 
// 16 - eyes wide open / fully white

LINK_ENTITY_TO_CLASS( env_tonemap_controller, CEnvTonemapController );

BEGIN_DATADESC( CEnvTonemapController )
	DEFINE_FIELD( m_flBlendTonemapStart, FIELD_FLOAT ),
	DEFINE_FIELD( m_flBlendTonemapEnd, FIELD_FLOAT ),
	DEFINE_FIELD( m_flBlendEndTime, FIELD_TIME ),
	DEFINE_FIELD( m_flBlendStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_bUseCustomAutoExposureMin, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bUseCustomAutoExposureMax, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flCustomAutoExposureMin, FIELD_FLOAT ),
	DEFINE_FIELD( m_flCustomAutoExposureMax, FIELD_FLOAT ),
	DEFINE_FIELD( m_flCustomBloomScale, FIELD_FLOAT ),
	DEFINE_FIELD( m_flCustomBloomScaleMinimum, FIELD_FLOAT ),
	DEFINE_FIELD( m_bUseCustomBloomScale, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_flBloomExponent, FIELD_FLOAT ),
	DEFINE_FIELD( m_flBloomSaturation, FIELD_FLOAT ),

	DEFINE_FIELD( m_bUseCustomManualTonemapRate, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flCustomManualTonemapRate, FIELD_FLOAT ),

	DEFINE_THINKFUNC( UpdateTonemapScaleBlend ),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetTonemapScale", InputSetTonemapScale ),
	DEFINE_INPUTFUNC( FIELD_STRING, "BlendTonemapScale", InputBlendTonemapScale ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetTonemapRate", InputSetTonemapRate ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetAutoExposureMin", InputSetAutoExposureMin ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetAutoExposureMax", InputSetAutoExposureMax ),
	DEFINE_INPUTFUNC( FIELD_VOID, "UseDefaultAutoExposure", InputUseDefaultAutoExposure ),
	DEFINE_INPUTFUNC( FIELD_VOID, "UseDefaultBloomScale", InputUseDefaultBloomScale ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetBloomScale", InputSetBloomScale ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetBloomScaleRange", InputSetBloomScaleRange ),

	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetBloomExponent", InputSetBloomExponent ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetBloomSaturation", InputSetBloomSaturation ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CEnvTonemapController, DT_EnvTonemapController )
	SendPropInt( SENDINFO(m_bUseCustomAutoExposureMin), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_bUseCustomAutoExposureMax), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_bUseCustomBloomScale), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO(m_bUseCustomManualTonemapRate), 1, SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO(m_flCustomAutoExposureMin), 0, SPROP_NOSCALE),
	SendPropFloat( SENDINFO(m_flCustomAutoExposureMax), 0, SPROP_NOSCALE),
	SendPropFloat( SENDINFO(m_flCustomBloomScale), 0, SPROP_NOSCALE),
	SendPropFloat( SENDINFO(m_flCustomBloomScaleMinimum), 0, SPROP_NOSCALE),
	SendPropFloat( SENDINFO(m_flBloomExponent), 0, SPROP_NOSCALE),
	SendPropFloat( SENDINFO(m_flBloomSaturation), 0, SPROP_NOSCALE),
	SendPropFloat( SENDINFO(m_flCustomManualTonemapRate), 0, SPROP_NOSCALE),
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEnvTonemapController::CEnvTonemapController()
{
	m_flBloomExponent = 2.5f;
	m_flBloomSaturation = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvTonemapController::Spawn( void )
{
	SetSolid( SOLID_NONE );
	SetMoveType( MOVETYPE_NONE );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CEnvTonemapController::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//-----------------------------------------------------------------------------
// Purpose: Set the tonemap scale to the specified value
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputSetTonemapScale( inputdata_t &inputdata )
{
	float flRemapped = inputdata.value.Float();
	mat_hdr_tonemapscale.SetValue( flRemapped );
}

//-----------------------------------------------------------------------------
// Purpose: Blend the tonemap scale to the specified value
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputBlendTonemapScale( inputdata_t &inputdata )
{
	char parseString[255];
	Q_strncpy(parseString, inputdata.value.String(), sizeof(parseString));

	// Get the target tonemap scale
	char *pszParam = strtok(parseString," ");
	if ( !pszParam || !pszParam[0] )
	{
		Warning("%s (%s) received BlendTonemapScale input without a target tonemap scale. Syntax: <target tonemap scale> <blend time>\n", GetClassname(), GetDebugName() );
		return;
	}
	m_flBlendTonemapEnd = atof( pszParam );

	// Get the blend time
	pszParam = strtok(NULL," ");
	if ( !pszParam || !pszParam[0] )
	{
		Warning("%s (%s) received BlendTonemapScale input without a blend time. Syntax: <target tonemap scale> <blend time>\n", GetClassname(), GetDebugName() );
		return;
	}
	m_flBlendEndTime = gpGlobals->curtime + atof( pszParam );

	m_flBlendStartTime = gpGlobals->curtime;
	m_flBlendTonemapStart = mat_hdr_tonemapscale.GetFloat();

	// Start thinking
	SetNextThink( gpGlobals->curtime + 0.1 );
	SetThink( &CEnvTonemapController::UpdateTonemapScaleBlend );
}

//-----------------------------------------------------------------------------
// Purpose: set a base and minimum bloom scale
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputSetBloomScaleRange( inputdata_t &inputdata )
{
	float bloom_max=1, bloom_min=1;
	int nargs=sscanf("%f %f",inputdata.value.String(), bloom_max, bloom_min );
	if (nargs != 2)
	{
		Warning("%s (%s) received SetBloomScaleRange input without 2 arguments. Syntax: <max bloom> <min bloom>\n", GetClassname(), GetDebugName() );
		return;
	}
	m_flCustomBloomScale = bloom_max;
	m_flCustomBloomScaleMinimum = bloom_min;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputSetTonemapRate( inputdata_t &inputdata )
{
	m_flCustomManualTonemapRate = inputdata.value.Float();
	m_bUseCustomManualTonemapRate = true;
}

//-----------------------------------------------------------------------------
// Purpose: Blend the tonemap scale to the specified value
//-----------------------------------------------------------------------------
void CEnvTonemapController::UpdateTonemapScaleBlend( void )
{ 
	float flRemapped = RemapValClamped( gpGlobals->curtime, m_flBlendStartTime, m_flBlendEndTime, m_flBlendTonemapStart, m_flBlendTonemapEnd );
	mat_hdr_tonemapscale.SetValue( flRemapped );

	//Msg("Setting tonemap scale to %f (curtime %f, %f -> %f)\n", flRemapped, gpGlobals->curtime, m_flBlendStartTime, m_flBlendEndTime ); 

	// Stop when we're out of the blend range
	if ( gpGlobals->curtime >= m_flBlendEndTime )
		return;

	SetNextThink( gpGlobals->curtime + 0.1 );
}

//-----------------------------------------------------------------------------
// Purpose: Set the auto exposure min to the specified value
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputSetAutoExposureMin( inputdata_t &inputdata )
{
	m_flCustomAutoExposureMin = inputdata.value.Float();
	m_bUseCustomAutoExposureMin = true;
}

//-----------------------------------------------------------------------------
// Purpose: Set the auto exposure max to the specified value
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputSetAutoExposureMax( inputdata_t &inputdata )
{
	m_flCustomAutoExposureMax = inputdata.value.Float();
	m_bUseCustomAutoExposureMax = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputUseDefaultAutoExposure( inputdata_t &inputdata )
{
	m_bUseCustomAutoExposureMin = false;
	m_bUseCustomAutoExposureMax = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputSetBloomScale( inputdata_t &inputdata )
{
	m_flCustomBloomScale = inputdata.value.Float();
	m_flCustomBloomScaleMinimum = m_flCustomBloomScale;
	m_bUseCustomBloomScale = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputUseDefaultBloomScale( inputdata_t &inputdata )
{
	m_bUseCustomBloomScale = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputSetBloomExponent( inputdata_t &inputdata )
{
	m_flBloomExponent = inputdata.value.Float();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvTonemapController::InputSetBloomSaturation( inputdata_t &inputdata )
{
	m_flBloomSaturation = inputdata.value.Float();
}


//--------------------------------------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS( trigger_tonemap, CTonemapTrigger );

BEGIN_DATADESC( CTonemapTrigger )
	DEFINE_KEYFIELD( m_tonemapControllerName,	FIELD_STRING,	"TonemapName" ),
END_DATADESC()


//--------------------------------------------------------------------------------------------------------
void CTonemapTrigger::Spawn( void )
{
	AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS );

	BaseClass::Spawn();
	InitTrigger();

	m_hTonemapController = gEntList.FindEntityByName( NULL, m_tonemapControllerName );
}


//--------------------------------------------------------------------------------------------------------
void CTonemapTrigger::StartTouch( CBaseEntity *other )
{
	if ( !PassesTriggerFilters( other ) )
		return;

	BaseClass::StartTouch( other );

#ifndef INFESTED_DLL
	CBasePlayer *player = ToBasePlayer( other );
	if ( !player )
		return;

	player->OnTonemapTriggerStartTouch( this );
#else
	CASW_Inhabitable_NPC *pNPC = dynamic_cast< CASW_Inhabitable_NPC * >( other );
	if ( !pNPC )
		return;

	pNPC->OnTonemapTriggerStartTouch( this );
#endif
}


//--------------------------------------------------------------------------------------------------------
void CTonemapTrigger::EndTouch( CBaseEntity *other )
{
	if ( !PassesTriggerFilters( other ) )
		return;

	BaseClass::EndTouch( other );

#ifndef INFESTED_DLL
	CBasePlayer *player = ToBasePlayer( other );
	if ( !player )
		return;

	player->OnTonemapTriggerEndTouch( this );
#else
	CASW_Inhabitable_NPC *pNPC = dynamic_cast< CASW_Inhabitable_NPC * >( other );
	if ( !pNPC )
		return;

	pNPC->OnTonemapTriggerEndTouch( this );
#endif
}


//-----------------------------------------------------------------------------
// Purpose: Clear out the tonemap controller.
//-----------------------------------------------------------------------------
void CTonemapSystem::LevelInitPreEntity( void )
{
	m_hMasterController = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: On level load find the master fog controller.  If no controller is 
//			set as Master, use the first fog controller found.
//-----------------------------------------------------------------------------
void CTonemapSystem::LevelInitPostEntity( void )
{
	// Overall master controller
	CEnvTonemapController *pTonemapController = NULL;
	do
	{
		pTonemapController = static_cast< CEnvTonemapController * >( gEntList.FindEntityByClassname( pTonemapController, "env_tonemap_controller" ) );
		if ( pTonemapController )
		{
			if ( pTonemapController->IsMaster() )
			{
				m_hMasterController = pTonemapController;
				break;
			}

			if ( m_hMasterController == NULL )
			{
				m_hMasterController = pTonemapController;
			}
		}
	} while ( pTonemapController );
}


//--------------------------------------------------------------------------------------------------------
CTonemapSystem s_TonemapSystem( "TonemapSystem" );


//--------------------------------------------------------------------------------------------------------
CTonemapSystem *TheTonemapSystem( void )
{
	return &s_TonemapSystem;
}


//--------------------------------------------------------------------------------------------------------

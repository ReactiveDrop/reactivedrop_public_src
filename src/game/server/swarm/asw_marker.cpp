#include "cbase.h"
#include "asw_marker.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_marker, CASW_Marker );

IMPLEMENT_SERVERCLASS_ST( CASW_Marker, DT_ASW_Marker )
	SendPropStringT( SENDINFO( m_ObjectiveName ) ),
	SendPropInt( SENDINFO( m_nMapWidth) ),
	SendPropInt( SENDINFO( m_nMapHeight ) ),
	SendPropBool( SENDINFO( m_bComplete ) ),
	SendPropBool( SENDINFO( m_bEnabled ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Marker )
	DEFINE_KEYFIELD( m_ObjectiveName,	FIELD_STRING, "objectivename" ),
	DEFINE_KEYFIELD( m_nMapWidth,		FIELD_INTEGER, "mapwidth" ),
	DEFINE_KEYFIELD( m_nMapHeight,		FIELD_INTEGER, "mapheight" ),
	DEFINE_KEYFIELD( m_bStartDisabled,	FIELD_BOOLEAN, "StartDisabled" ),
	DEFINE_FIELD( m_bEnabled,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bComplete,			FIELD_BOOLEAN ),

	DEFINE_INPUTFUNC( FIELD_VOID,		"SetComplete",	InputSetComplete ),
	DEFINE_INPUTFUNC( FIELD_VOID,		"SetIncomplete",	InputSetIncomplete ),
	DEFINE_INPUTFUNC( FIELD_VOID,		"Enable",		InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID,		"Disable",		InputDisable ),
END_DATADESC()


void CASW_Marker::Spawn( void )
{
	BaseClass::Spawn();

	m_bEnabled = !m_bStartDisabled;
}

// always send this info to players
int CASW_Marker::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

void CASW_Marker::InputSetComplete( inputdata_t &inputdata )
{
	m_bComplete = true;
}

void CASW_Marker::InputSetIncomplete( inputdata_t &inputdata )
{
	m_bComplete = false;
}

void CASW_Marker::InputEnable( inputdata_t &inputdata )
{
	m_bEnabled = true;
}

void CASW_Marker::InputDisable( inputdata_t &inputdata )
{
	m_bEnabled = false;
}

#include "cbase.h"
#include "asw_objective_triggered.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// An objective controlled by mapper inputs

LINK_ENTITY_TO_CLASS( asw_objective_triggered, CASW_Objective_Triggered );

IMPLEMENT_SERVERCLASS_ST( CASW_Objective_Triggered, DT_ASW_Objective_Triggered )
	SendPropInt( SENDINFO( m_nProgress ), -1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nMaxProgress ), -1, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bShowPercentage ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Objective_Triggered )
	DEFINE_INPUT( m_nProgress, FIELD_INTEGER, "SetProgress" ),
	DEFINE_INPUT( m_nMaxProgress, FIELD_INTEGER, "SetMaxProgress" ),
	DEFINE_INPUT( m_bShowPercentage, FIELD_BOOLEAN, "SetShowPercentage" ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetComplete", InputSetComplete ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetIncomplete", InputSetIncomplete ),
	DEFINE_INPUTFUNC( FIELD_VOID, "SetFailed", InputSetFailed ),
END_DATADESC()


CASW_Objective_Triggered::CASW_Objective_Triggered()
{
	m_nProgress = 0;
	m_nMaxProgress = 0;
	m_bShowPercentage = false;
}

CASW_Objective_Triggered::~CASW_Objective_Triggered()
{
}

void CASW_Objective_Triggered::InputSetComplete( inputdata_t &inputdata )
{
	SetComplete( true );
}

void CASW_Objective_Triggered::InputSetIncomplete( inputdata_t &inputdata )
{
	SetComplete( false );
	SetFailed( false );
}

void CASW_Objective_Triggered::InputSetFailed( inputdata_t &inputdata )
{
	SetFailed( true );
}

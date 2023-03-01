#include "cbase.h"
#include "rd_director_triggers.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( func_rd_no_director_aliens, CRD_No_Director_Aliens );

BEGIN_DATADESC( CRD_No_Director_Aliens )
	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
END_DATADESC()

IMPLEMENT_AUTO_LIST( IRD_No_Director_Aliens );

void CRD_No_Director_Aliens::Spawn()
{
	SetSolid( SOLID_BSP );
	SetSolidFlags( FSOLID_NOT_SOLID );
	SetModel( STRING( GetModelName() ) );

	BaseClass::Spawn();
}

void CRD_No_Director_Aliens::InputEnable( inputdata_t & inputdata )
{
	m_bDisabled = false;
}

void CRD_No_Director_Aliens::InputDisable( inputdata_t & inputdata )
{
	m_bDisabled = true;
}

void CRD_No_Director_Aliens::InputToggle( inputdata_t & inputdata )
{
	m_bDisabled = !m_bDisabled;
}

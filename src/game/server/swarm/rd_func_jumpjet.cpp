#include "cbase.h"
#include "rd_func_jumpjet.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_AUTO_LIST( IJumpJetVolumes );

LINK_ENTITY_TO_CLASS( rd_func_jumpjet, CRD_Func_JumpJet );
LINK_ENTITY_TO_CLASS( jumpjet_volume, CRD_Func_JumpJet );

BEGIN_DATADESC( CRD_Func_JumpJet )
	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
END_DATADESC()

CRD_Func_JumpJet::CRD_Func_JumpJet()
{
	m_bDisabled = false;
}

CRD_Func_JumpJet::~CRD_Func_JumpJet()
{
}

void CRD_Func_JumpJet::Spawn()
{
	BaseClass::Spawn();

	SetSolid( SOLID_BSP );
	SetSolidFlags( FSOLID_NOT_SOLID );
	SetModel( STRING( GetModelName() ) );
}

bool CRD_Func_JumpJet::IsPointInBounds( const Vector & vec )
{
	return !m_bDisabled && CollisionProp()->IsPointInBounds( vec );
}

void CRD_Func_JumpJet::InputEnable( inputdata_t & data )
{
	m_bDisabled = false;
}

void CRD_Func_JumpJet::InputDisable( inputdata_t & data )
{
	m_bDisabled = true;
}

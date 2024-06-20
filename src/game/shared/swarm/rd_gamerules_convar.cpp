#include "cbase.h"
#include "rd_gamerules_convar.h"

CRD_GameRulesConVarCollection::CRD_GameRulesConVarCollection( const char *szName ) :
	m_szName( szName )
{
}

void CRD_GameRulesConVarCollection::AddVar( CRD_GameRulesConVar *pVar )
{
	Assert( this == &pVar->m_Collection );

	while ( m_Vars.Count() <= pVar->m_Index )
		m_Vars.AddToTail( NULL );

	Assert( m_Vars[pVar->m_Index] == NULL );
	m_Vars[pVar->m_Index] = pVar;
}

CRD_GameRulesConVar::CRD_GameRulesConVar( CRD_GameRulesConVarCollection &collection, int index, const char *szName, const char *szDefaultValue, const char *szHelpString, bool bHasMinValue, float flMinValue, bool bHasMaxValue, float flMaxValue ) :
	m_Collection( collection ),
	m_Index( index ),
	m_Var( szName, szDefaultValue, FCVAR_REPLICATED | FCVAR_CHEAT, szHelpString, bHasMinValue, flMinValue, bHasMaxValue, flMaxValue, &CRD_GameRulesConVar::OnChanged )
{
	collection.AddVar( this );
}

void CRD_GameRulesConVar::OnChanged( IConVar *pVar, const char *szOldValue, float flOldValue )
{
}

CRD_GameRulesConVarFloat::CRD_GameRulesConVarFloat( CRD_GameRulesConVarCollection &collection, int index, const char *szName, const char *szDefaultValue, const char *szHelpString, bool bHasMinValue, float flMinValue, bool bHasMaxValue, float flMaxValue ) :
	CRD_GameRulesConVar( collection, index, szName, szDefaultValue, szHelpString, bHasMinValue, flMinValue, bHasMaxValue, flMaxValue )
{
}

float CRD_GameRulesConVarFloat::GetFloat()
{
	return m_Var.GetFloat();
}

CRD_GameRulesConVarBool::CRD_GameRulesConVarBool( CRD_GameRulesConVarCollection &collection, int index, const char *szName, const char *szDefaultValue, const char *szHelpString ) :
	CRD_GameRulesConVar( collection, index, szName, szDefaultValue, szHelpString, false, 0, false, 0 )
{
}

bool CRD_GameRulesConVarBool::GetBool()
{
	return m_Var.GetBool();
}

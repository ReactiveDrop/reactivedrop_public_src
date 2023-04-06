#include "cbase.h"
#include "rd_boss_bar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( rd_boss_bar, CRD_Boss_Bar );

IMPLEMENT_SERVERCLASS_ST( CRD_Boss_Bar, DT_RD_Boss_Bar )
	SendPropInt(SENDINFO(m_BarMode)),
	SendPropFloat(SENDINFO(m_flBarValue), 32, SPROP_CHANGES_OFTEN),
	SendPropFloat(SENDINFO(m_flBarMaxValue)),
	SendPropStringT(SENDINFO(m_iszBarID)),
	SendPropStringT(SENDINFO(m_iszBarTitle)),
	SendPropInt(SENDINFO(m_iBarColumn)),
	SendPropInt(SENDINFO(m_iBarRow)),
	SendPropFloat(SENDINFO(m_flBarHeightScale)),
	SendPropInt(SENDINFO(m_BarFGColor), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt32),
	SendPropInt(SENDINFO(m_BarBGColor), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt32),
	SendPropInt(SENDINFO(m_BarBorderColor), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt32),
	SendPropInt(SENDINFO(m_BarFlashColor), 32, SPROP_UNSIGNED, SendProxy_Color32ToInt32),
	SendPropFloat(SENDINFO(m_flBarFlashSustain)),
	SendPropFloat(SENDINFO(m_flBarFlashInterpolate)),
	SendPropBool(SENDINFO(m_bEnabled)),
	SendPropFloat(SENDINFO(m_flBarRadius)),
END_SEND_TABLE()

BEGIN_DATADESC( CRD_Boss_Bar )
	DEFINE_KEYFIELD( m_BarMode, FIELD_INTEGER, "BarMode" ),
	DEFINE_INPUT( m_flBarValue, FIELD_FLOAT, "BarValue" ),
	DEFINE_INPUT( m_flBarMaxValue, FIELD_FLOAT, "BarMax" ),
	DEFINE_KEYFIELD( m_iszBarID, FIELD_STRING, "BarID" ),
	DEFINE_INPUT( m_iszBarTitle, FIELD_STRING, "BarTitle" ),
	DEFINE_KEYFIELD(m_iBarColumn, FIELD_INTEGER, "BarColumn" ),
	DEFINE_KEYFIELD( m_iBarRow, FIELD_INTEGER, "BarRow" ),
	DEFINE_INPUT( m_flBarHeightScale, FIELD_FLOAT, "BarScale" ),
	DEFINE_INPUT( m_BarFGColor, FIELD_COLOR32, "BarFGColor" ),
	DEFINE_INPUT( m_BarBGColor, FIELD_COLOR32, "BarBGColor" ),
	DEFINE_INPUT( m_BarBorderColor, FIELD_COLOR32, "BarBorderColor" ),
	DEFINE_INPUT( m_BarFlashColor, FIELD_COLOR32, "BarFlashColor" ),
	DEFINE_KEYFIELD( m_flBarFlashSustain, FIELD_FLOAT, "BarFlashSustain" ),
	DEFINE_KEYFIELD( m_flBarFlashInterpolate, FIELD_FLOAT, "BarFlashInterpolate" ),
	DEFINE_FIELD( m_bEnabled, FIELD_BOOLEAN ),
	DEFINE_KEYFIELD( m_flBarRadius, FIELD_FLOAT, "BarRadius" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
END_DATADESC()

CRD_Boss_Bar::CRD_Boss_Bar()
{
	m_flBarRadius = -1;
}

int CRD_Boss_Bar::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

void CRD_Boss_Bar::InputEnable( inputdata_t & inputdata )
{
	m_bEnabled = true;
}

void CRD_Boss_Bar::InputDisable( inputdata_t & inputdata )
{
	m_bEnabled = false;
}

#include "cbase.h"
#include "C_ASW_Objective_Kill_Eggs.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/Panel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Objective_Kill_Eggs, DT_ASW_Objective_Kill_Eggs, CASW_Objective_Kill_Eggs)	
	RecvPropInt		(RECVINFO(m_iTargetKills)),
	RecvPropInt		(RECVINFO(m_iCurrentKills)),
END_RECV_TABLE()

C_ASW_Objective_Kill_Eggs::C_ASW_Objective_Kill_Eggs()
{
	m_pKillText = NULL;
	m_pAlienPluralText = NULL;
	m_pAlienSingularText = NULL;
	m_bFoundText = false;
	m_wszTitleBuffer[0] = '\0';
	m_iLastKills = -1;
}

void C_ASW_Objective_Kill_Eggs::OnDataChanged(DataUpdateType_t updateType)
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		FindText();
	}

	BaseClass::OnDataChanged(updateType);
}

bool C_ASW_Objective_Kill_Eggs::NeedsTitleUpdate()
{
	int iKills = MIN( m_iTargetKills.Get(), m_iCurrentKills.Get() );

	return iKills != m_iLastKills;
}

const wchar_t *C_ASW_Objective_Kill_Eggs::GetObjectiveTitle()
{
	if ( !m_bFoundText || !m_pKillText || !m_pAlienPluralText || !m_pAlienSingularText )
	{
		return L"";
	}

	int iKills = MIN( m_iTargetKills.Get(), m_iCurrentKills.Get() );

	if ( iKills != m_iLastKills )	// update the string
	{
		m_iLastKills = iKills;

		wchar_t wszNum[24];
		V_snwprintf( wszNum, NELEMS( wszNum ), L"%d", iKills );

		wchar_t wszNum2[24];
		V_snwprintf( wszNum2, NELEMS( wszNum2 ), L"%d", m_iTargetKills.Get() );

		g_pVGuiLocalize->ConstructString( m_wszTitleBuffer, sizeof( m_wszTitleBuffer ),
			g_pVGuiLocalize->Find( "#asw_kill_objective_format" ), 4,
			m_pKillText, m_pAlienPluralText, wszNum, wszNum2 );
	}

	return m_wszTitleBuffer;
}

void C_ASW_Objective_Kill_Eggs::FindText()
{
	m_pKillText = g_pVGuiLocalize->Find( "#asw_destroy" );
	m_pAlienPluralText = GetPluralText();
	m_pAlienSingularText = GetSingularText();
	m_bFoundText = true;
}

wchar_t *C_ASW_Objective_Kill_Eggs::GetPluralText()
{
	return g_pVGuiLocalize->Find( "#asw_eggs" );
}

wchar_t *C_ASW_Objective_Kill_Eggs::GetSingularText()
{
	return g_pVGuiLocalize->Find( "#asw_egg" );
}

float C_ASW_Objective_Kill_Eggs::GetObjectiveProgress()
{
	if ( m_iTargetKills <= 0 )
		return BaseClass::GetObjectiveProgress();

	float flProgress = ( float )m_iCurrentKills.Get() / ( float )m_iTargetKills.Get();
	flProgress = clamp<float>( flProgress, 0.0f, 1.0f );

	return flProgress;
}

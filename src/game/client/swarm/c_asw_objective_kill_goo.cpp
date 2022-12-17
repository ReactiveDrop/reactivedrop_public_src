#include "cbase.h"
#include "c_asw_objective_kill_goo.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/Panel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Objective_Kill_Goo, DT_ASW_Objective_Kill_Goo, CASW_Objective_Kill_Goo)	
	RecvPropInt		(RECVINFO(m_iTargetKills)),
	RecvPropInt		(RECVINFO(m_iCurrentKills)),
END_RECV_TABLE()

C_ASW_Objective_Kill_Goo::C_ASW_Objective_Kill_Goo()
{
	m_wszTitleBuffer[0] = '\0';
	m_iLastKills = -1;
}

bool C_ASW_Objective_Kill_Goo::NeedsTitleUpdate()
{
	int iKills = MIN( m_iTargetKills.Get(), m_iCurrentKills.Get() );

	return iKills != m_iLastKills;
}

const wchar_t *C_ASW_Objective_Kill_Goo::GetObjectiveTitle()
{
	int iKills = MIN( m_iTargetKills.Get(), m_iCurrentKills.Get() );

	if ( iKills != m_iLastKills )	// update the string
	{
		m_iLastKills = iKills;

		wchar_t wszNum[24];
		V_snwprintf( wszNum, NELEMS( wszNum ), L"%d", iKills );

		wchar_t wszNum2[24];
		V_snwprintf( wszNum2, NELEMS( wszNum2 ), L"%d", m_iTargetKills.Get() );

		g_pVGuiLocalize->ConstructString( m_wszTitleBuffer, sizeof( m_wszTitleBuffer ),
			g_pVGuiLocalize->Find( "#asw_kill_goo_objective_format" ), 2,
			wszNum, wszNum2 );
	}

	return m_wszTitleBuffer;
}

float C_ASW_Objective_Kill_Goo::GetObjectiveProgress()
{
	if ( m_iTargetKills <= 0 )
		return BaseClass::GetObjectiveProgress();

	float flProgress = ( float )m_iCurrentKills.Get() / ( float )m_iTargetKills.Get();
	flProgress = clamp<float>( flProgress, 0.0f, 1.0f );

	return flProgress;
}

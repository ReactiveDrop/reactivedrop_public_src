#include "cbase.h"
#include "c_asw_objective_triggered.h"
#include "asw_hud_objective.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Objective_Triggered, DT_ASW_Objective_Triggered, CASW_Objective_Triggered )
	RecvPropInt( RECVINFO( m_nProgress ) ),
	RecvPropInt( RECVINFO( m_nMaxProgress ) ),
END_RECV_TABLE()

C_ASW_Objective_Triggered::C_ASW_Objective_Triggered()
{
	m_nProgress = 0;
	m_nMaxProgress = 0;
	m_nPrevProgress = 0;
	m_bNeedsUpdate = true;
}

void C_ASW_Objective_Triggered::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	m_bNeedsUpdate = true;

	if ( m_nProgress != m_nPrevProgress )
	{
		FOR_EACH_VALID_SPLITSCREEN_PLAYER( hh )
		{
			ACTIVE_SPLITSCREEN_PLAYER_GUARD( hh );

			CASWHudObjective *pObjective = GET_HUDELEMENT( CASWHudObjective );
			if ( pObjective )
			{
				pObjective->ShowObjectives( 5.0f );
			}
		}

		m_nPrevProgress = m_nProgress;
	}
}

bool C_ASW_Objective_Triggered::NeedsTitleUpdate()
{
	return m_bNeedsUpdate;
}

const wchar_t *C_ASW_Objective_Triggered::GetObjectiveTitle()
{
	if ( m_bNeedsUpdate )
	{
		if ( m_nMaxProgress <= 0 )
		{
			V_wcsncpy( m_wszObjectiveTitle, BaseClass::GetObjectiveTitle(), sizeof( m_wszObjectiveTitle ) );
		}
		else
		{
			V_snwprintf( m_wszObjectiveTitle, ARRAYSIZE( m_wszObjectiveTitle ), L"%s %d/%d", BaseClass::GetObjectiveTitle(), m_nProgress.Get(), m_nMaxProgress.Get() );
		}

		m_bNeedsUpdate = false;
	}

	return m_wszObjectiveTitle;
}

float C_ASW_Objective_Triggered::GetObjectiveProgress()
{
	if ( m_nMaxProgress <= 0 )
	{
		return BaseClass::GetObjectiveProgress();
	}

	float flProgress = ( float )m_nProgress / ( float )m_nMaxProgress;
	flProgress = clamp<float>( flProgress, 0.0f, 1.0f );

	return flProgress;
}

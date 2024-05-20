#include "cbase.h"
#include "c_asw_objective_triggered.h"
#include "asw_hud_objective.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Objective_Triggered, DT_ASW_Objective_Triggered, CASW_Objective_Triggered )
	RecvPropInt( RECVINFO( m_nProgress ) ),
	RecvPropInt( RECVINFO( m_nMaxProgress ) ),
	RecvPropBool( RECVINFO( m_bShowPercentage ) ),
END_RECV_TABLE()

C_ASW_Objective_Triggered::C_ASW_Objective_Triggered()
{
	m_nProgress = 0;
	m_nMaxProgress = 0;
	m_bShowPercentage = false;
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
		else if ( m_bShowPercentage )
		{
			int nProgressPermille = m_nProgress * 1000 / m_nMaxProgress;
			if ( nProgressPermille > 1000 )
				nProgressPermille = 1000;
			wchar_t wszPercent[4], wszTenths[2];
			V_snwprintf( wszPercent, NELEMS( wszPercent ), L"%d", nProgressPermille / 10 );
			V_snwprintf( wszTenths, NELEMS( wszTenths ), L"%d", nProgressPermille % 10 );
			g_pVGuiLocalize->ConstructString( m_wszObjectiveTitle, sizeof( m_wszObjectiveTitle ), g_pVGuiLocalize->Find( "#asw_generic_objective_percent" ), 3, BaseClass::GetObjectiveTitle(), wszPercent, wszTenths );
		}
		else
		{
			wchar_t wszProgress[11], wszMaxProgress[11];
			V_snwprintf( wszProgress, NELEMS( wszProgress ), L"%d", m_nProgress.Get() );
			V_snwprintf( wszMaxProgress, NELEMS( wszMaxProgress ), L"%d", m_nMaxProgress.Get() );
			g_pVGuiLocalize->ConstructString( m_wszObjectiveTitle, sizeof( m_wszObjectiveTitle ), g_pVGuiLocalize->Find( "#asw_generic_objective_fraction" ), 3, BaseClass::GetObjectiveTitle(), wszProgress, wszMaxProgress );
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

	float flProgress = float( m_nProgress ) / float( m_nMaxProgress );
	flProgress = clamp<float>( flProgress, 0.0f, 1.0f );

	return flProgress;
}

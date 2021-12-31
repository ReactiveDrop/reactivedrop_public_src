#include "cbase.h"
#include "c_rd_boss_bar.h"
#include "rd_hud_boss_bar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_RD_Boss_Bar, DT_RD_Boss_Bar, CRD_Boss_Bar )
	RecvPropInt(RECVINFO(m_BarMode)),
	RecvPropFloat(RECVINFO(m_flBarValue), SPROP_CHANGES_OFTEN),
	RecvPropFloat(RECVINFO(m_flBarMaxValue)),
	RecvPropString(RECVINFO(m_iszBarID)),
	RecvPropString(RECVINFO(m_iszBarTitle)),
	RecvPropInt(RECVINFO(m_iBarColumn)),
	RecvPropInt(RECVINFO(m_iBarRow)),
	RecvPropFloat(RECVINFO(m_flBarHeightScale)),
	RecvPropInt(RECVINFO(m_BarFGColor), 0, RecvProxy_Int32ToColor32),
	RecvPropInt(RECVINFO(m_BarBGColor), 0, RecvProxy_Int32ToColor32),
	RecvPropInt(RECVINFO(m_BarBorderColor), 0, RecvProxy_Int32ToColor32),
	RecvPropInt(RECVINFO(m_BarFlashColor), 0, RecvProxy_Int32ToColor32),
	RecvPropFloat(RECVINFO(m_flBarFlashSustain)),
	RecvPropFloat(RECVINFO(m_flBarFlashInterpolate)),
	RecvPropBool(RECVINFO(m_bEnabled)),
END_RECV_TABLE()

void C_RD_Boss_Bar::PostDataUpdate( DataUpdateType_t updateType )
{
	BaseClass::PostDataUpdate( updateType );

	float flSustain = MAX( m_flBarFlashSustain, 0 );
	float flInterpolate = MAX( m_flBarFlashInterpolate, 0 );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_flBarValuePrev = m_flBarValue;
		m_flBarValueLastChanged = gpGlobals->curtime - flSustain - flInterpolate;
	}
	else if ( m_flBarValue != m_flBarValueTruePrev )
	{
		if ( m_flBarValueLastChanged < gpGlobals->curtime - flSustain - flInterpolate )
		{
			m_flBarValuePrev = m_flBarValueTruePrev;
		}
		else if ( m_flBarValueLastChanged < gpGlobals->curtime - flSustain )
		{
			m_flBarValuePrev = Lerp( ( gpGlobals->curtime - m_flBarValueLastChanged - flSustain ) / flInterpolate, m_flBarValuePrev, m_flBarValueTruePrev );
		}

		m_flBarValueLastChanged = gpGlobals->curtime;
	}

	m_flBarValueTruePrev = m_flBarValue;

	FOR_EACH_VALID_SPLITSCREEN_PLAYER(slot)
	{
		ACTIVE_SPLITSCREEN_PLAYER_GUARD(slot);

		(GET_HUDELEMENT( CRD_Hud_Boss_Bars ))->OnBossBarEntityChanged( this, updateType == DATA_UPDATE_CREATED );
	}
}

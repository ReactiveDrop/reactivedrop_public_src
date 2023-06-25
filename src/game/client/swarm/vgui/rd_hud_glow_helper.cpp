#include "cbase.h"
#include "rd_hud_glow_helper.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar rd_reduce_motion;
ConVar rd_main_menu_glow_rampup( "rd_main_menu_glow_rampup", "0.1", FCVAR_NONE, "How many seconds does it take an unfocused button on the main menu to become focused?" );
ConVar rd_main_menu_glow_rampdown( "rd_main_menu_glow_rampdown", "0.25", FCVAR_NONE, "How many seconds does it take a focused button on the main menu to become unfocused?" );
ConVar rd_main_menu_glow_period( "rd_main_menu_glow_period", "3", FCVAR_NONE, "How many seconds does it take a focused button on the main menu to fade in or out while remaining focused?" );
ConVar rd_main_menu_glow_min( "rd_main_menu_glow_min", "192", FCVAR_NONE, "Fade out glow intensity." );
ConVar rd_main_menu_glow_max( "rd_main_menu_glow_max", "255", FCVAR_NONE, "Full glow intensity." );

uint8_t HUDGlowHelper_t::Update( bool bShouldGlow )
{
	int iTarget = bShouldGlow ? 65535 : 0;

	if ( rd_reduce_motion.GetBool() )
	{
		m_iGlow = iTarget;
		m_iSustain = 0;
	}
	else
	{
		if ( m_iGlow != iTarget )
		{
			int iGlow = m_iGlow;
			float flRampTime = bShouldGlow ? rd_main_menu_glow_rampup.GetFloat() : rd_main_menu_glow_rampdown.GetFloat();
			if ( flRampTime <= 0 )
				iGlow = iTarget;
			else if ( bShouldGlow )
				iGlow += gpGlobals->absoluteframetime / flRampTime * 65535;
			else
				iGlow -= gpGlobals->absoluteframetime / flRampTime * 65535;

			m_iGlow = uint16_t( clamp<int>( iGlow, 0, 65535 ) );
		}

		if ( m_iGlow == 65535 && rd_main_menu_glow_period.GetFloat() > 0 )
			m_iSustain += uint16_t( 32767.5f * gpGlobals->absoluteframetime / rd_main_menu_glow_period.GetFloat() );
		else if ( m_iGlow == 0 )
			m_iSustain = 0;
	}

	return Get();
}

uint8_t HUDGlowHelper_t::Get() const
{
	if ( m_iGlow == 65535 )
		return Lerp( fabsf( m_iSustain / 32767.5f - 1.0f ), rd_main_menu_glow_min.GetInt(), rd_main_menu_glow_max.GetInt() );

	return Lerp( m_iGlow / 65535.0f, 0, rd_main_menu_glow_max.GetInt() );
}

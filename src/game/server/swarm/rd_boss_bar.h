#pragma once

#include "rd_boss_bar_shared.h"
#include "baseentity.h"

class CRD_Boss_Bar : public CBaseEntity
{
	DECLARE_CLASS( CRD_Boss_Bar, CBaseEntity );
public:
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CNetworkVar( RD_Boss_Bar_Mode, m_BarMode );
	CNetworkVar( float, m_flBarValue );
	CNetworkVar( float, m_flBarMaxValue );
	CNetworkVar( string_t, m_iszBarID );
	CNetworkVar( string_t, m_iszBarTitle );
	CNetworkVar( int, m_iBarColumn );
	CNetworkVar( int, m_iBarRow );
	CNetworkVar( float, m_flBarHeightScale );
	CNetworkColor32( m_BarFGColor );
	CNetworkColor32( m_BarBGColor );
	CNetworkColor32( m_BarBorderColor );
	CNetworkColor32( m_BarFlashColor );
	CNetworkVar( float, m_flBarFlashSustain );
	CNetworkVar( float, m_flBarFlashInterpolate );
	CNetworkVar( bool, m_bEnabled );
	CNetworkVar( float, m_flBarRadius );

	CRD_Boss_Bar();

	virtual int ShouldTransmit( const CCheckTransmitInfo *pInfo );

	void InputEnable( inputdata_t & inputdata );
	void InputDisable( inputdata_t & inputdata );
};

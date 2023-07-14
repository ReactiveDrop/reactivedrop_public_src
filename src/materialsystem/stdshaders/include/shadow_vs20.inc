// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $DOWATERFOG == 0 ) && ( $DOPIXELFOG && $HARDWAREFOGBLEND )
// ( $DOWATERFOG == 0 ) && ( $HARDWAREFOGBLEND == 0 ) && ( $DOPIXELFOG == 0 )
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class shadow_vs20_Static_Index
{
	unsigned int m_nDEFERRED_SHADOWS : 2;
	unsigned int m_nDOPIXELFOG : 2;
	unsigned int m_nHARDWAREFOGBLEND : 2;
#ifdef _DEBUG
	bool m_bDEFERRED_SHADOWS : 1;
#endif	// _DEBUG
public:
	void SetDEFERRED_SHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDEFERRED_SHADOWS = i;
#ifdef _DEBUG
		m_bDEFERRED_SHADOWS = true;
#endif	// _DEBUG
	}

	void SetDOPIXELFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOPIXELFOG = i;
	}

	void SetHARDWAREFOGBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHARDWAREFOGBLEND = i;
	}

	shadow_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nDEFERRED_SHADOWS = 0;
		m_nDOPIXELFOG = g_pHardwareConfig->SupportsPixelShaders_2_b() ;
		m_nHARDWAREFOGBLEND = !g_pHardwareConfig->SupportsPixelShaders_2_b() ;
#ifdef _DEBUG
		m_bDEFERRED_SHADOWS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDEFERRED_SHADOWS );
		return ( 2 * m_nDEFERRED_SHADOWS ) + ( 4 * m_nDOPIXELFOG ) + ( 8 * m_nHARDWAREFOGBLEND ) + 0;
	}
};

#define shaderStaticTest_shadow_vs20 vsh_forgot_to_set_static_DEFERRED_SHADOWS


class shadow_vs20_Dynamic_Index
{
	unsigned int m_nDOWATERFOG : 2;
public:
	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
	}

	shadow_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nDOWATERFOG = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
	}

	int GetIndex() const
	{
		return ( 1 * m_nDOWATERFOG ) + 0;
	}
};

#define shaderDynamicTest_shadow_vs20 1


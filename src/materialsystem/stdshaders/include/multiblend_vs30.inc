// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $DOWATERFOG == 0 ) && ( $DOPIXELFOG && $HARDWAREFOGBLEND )
// ( $DOWATERFOG == 0 ) && ( $HARDWAREFOGBLEND == 0 ) && ( $DOPIXELFOG == 0 )
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class multiblend_vs30_Static_Index
{
	unsigned int m_nMODEL : 2;
	unsigned int m_nDOPIXELFOG : 2;
	unsigned int m_nHARDWAREFOGBLEND : 1;
	unsigned int m_nSPECULAR : 2;
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nFOW : 2;
#ifdef _DEBUG
	bool m_bMODEL : 1;
	bool m_bSPECULAR : 1;
	bool m_bFLASHLIGHT : 1;
	bool m_bFOW : 1;
#endif	// _DEBUG
public:
	void SetMODEL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMODEL = i;
#ifdef _DEBUG
		m_bMODEL = true;
#endif	// _DEBUG
	}

	void SetDOPIXELFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOPIXELFOG = i;
	}

	void SetHARDWAREFOGBLEND( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nHARDWAREFOGBLEND = i;
	}

	void SetSPECULAR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSPECULAR = i;
#ifdef _DEBUG
		m_bSPECULAR = true;
#endif	// _DEBUG
	}

	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif	// _DEBUG
	}

	void SetFOW( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFOW = i;
#ifdef _DEBUG
		m_bFOW = true;
#endif	// _DEBUG
	}

	multiblend_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nMODEL = 0;
		m_nDOPIXELFOG = g_pHardwareConfig->SupportsPixelShaders_2_b() ;
		m_nHARDWAREFOGBLEND = 0 ;
		m_nSPECULAR = 0;
		m_nFLASHLIGHT = 0;
		m_nFOW = 0;
#ifdef _DEBUG
		m_bMODEL = false;
		m_bSPECULAR = false;
		m_bFLASHLIGHT = false;
		m_bFOW = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bMODEL && m_bSPECULAR && m_bFLASHLIGHT && m_bFOW );
		return ( 8 * m_nMODEL ) + ( 16 * m_nDOPIXELFOG ) + ( 32 * m_nHARDWAREFOGBLEND ) + ( 32 * m_nSPECULAR ) + ( 64 * m_nFLASHLIGHT ) + ( 128 * m_nFOW ) + 0;
	}
};

#define shaderStaticTest_multiblend_vs30 vsh_forgot_to_set_static_MODEL + vsh_forgot_to_set_static_SPECULAR + vsh_forgot_to_set_static_FLASHLIGHT + vsh_forgot_to_set_static_FOW


class multiblend_vs30_Dynamic_Index
{
	unsigned int m_nDOWATERFOG : 2;
	unsigned int m_nSKINNING : 2;
	unsigned int m_nMORPHING : 2;
#ifdef _DEBUG
	bool m_bSKINNING : 1;
#endif	// _DEBUG
public:
	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
	}

	void SetSKINNING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSKINNING = i;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif	// _DEBUG
	}

	void SetMORPHING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMORPHING = i;
	}

	multiblend_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nDOWATERFOG = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
		m_nSKINNING = 0;
		m_nMORPHING = pShaderAPI->IsHWMorphingEnabled() ;
#ifdef _DEBUG
		m_bSKINNING = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bSKINNING );
		return ( 1 * m_nDOWATERFOG ) + ( 2 * m_nSKINNING ) + ( 4 * m_nMORPHING ) + 0;
	}
};

#define shaderDynamicTest_multiblend_vs30 vsh_forgot_to_set_dynamic_SKINNING


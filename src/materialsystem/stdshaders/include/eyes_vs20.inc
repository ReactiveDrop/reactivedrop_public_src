// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $DOWATERFOG == 0 ) && ( $DOPIXELFOG && $HARDWAREFOGBLEND )
// ( $DOWATERFOG == 0 ) && ( $HARDWAREFOGBLEND == 0 ) && ( $DOPIXELFOG == 0 )
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class eyes_vs20_Static_Index
{
	unsigned int m_nINTRO : 2;
	unsigned int m_nHALFLAMBERT : 2;
	unsigned int m_nDOPIXELFOG : 2;
	unsigned int m_nHARDWAREFOGBLEND : 2;
#ifdef _DEBUG
	bool m_bINTRO : 1;
	bool m_bHALFLAMBERT : 1;
#endif	// _DEBUG
public:
	void SetINTRO( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nINTRO = i;
#ifdef _DEBUG
		m_bINTRO = true;
#endif	// _DEBUG
	}

	void SetHALFLAMBERT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHALFLAMBERT = i;
#ifdef _DEBUG
		m_bHALFLAMBERT = true;
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

	eyes_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nINTRO = 0;
		m_nHALFLAMBERT = 0;
		m_nDOPIXELFOG = g_pHardwareConfig->SupportsPixelShaders_2_b() ;
		m_nHARDWAREFOGBLEND = !g_pHardwareConfig->SupportsPixelShaders_2_b() ;
#ifdef _DEBUG
		m_bINTRO = false;
		m_bHALFLAMBERT = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bINTRO && m_bHALFLAMBERT );
		return ( 32 * m_nINTRO ) + ( 64 * m_nHALFLAMBERT ) + ( 128 * m_nDOPIXELFOG ) + ( 256 * m_nHARDWAREFOGBLEND ) + 0;
	}
};

#define shaderStaticTest_eyes_vs20 vsh_forgot_to_set_static_INTRO + vsh_forgot_to_set_static_HALFLAMBERT


class eyes_vs20_Dynamic_Index
{
	unsigned int m_nCOMPRESSED_VERTS : 2;
	unsigned int m_nSKINNING : 2;
	unsigned int m_nDOWATERFOG : 2;
	unsigned int m_nDYNAMIC_LIGHT : 2;
	unsigned int m_nSTATIC_LIGHT : 2;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bSKINNING : 1;
	bool m_bDYNAMIC_LIGHT : 1;
	bool m_bSTATIC_LIGHT : 1;
#endif	// _DEBUG
public:
	void SetCOMPRESSED_VERTS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOMPRESSED_VERTS = i;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = true;
#endif	// _DEBUG
	}

	void SetSKINNING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSKINNING = i;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif	// _DEBUG
	}

	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
	}

	void SetDYNAMIC_LIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDYNAMIC_LIGHT = i;
#ifdef _DEBUG
		m_bDYNAMIC_LIGHT = true;
#endif	// _DEBUG
	}

	void SetSTATIC_LIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSTATIC_LIGHT = i;
#ifdef _DEBUG
		m_bSTATIC_LIGHT = true;
#endif	// _DEBUG
	}

	eyes_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nSKINNING = 0;
		m_nDOWATERFOG = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
		m_nDYNAMIC_LIGHT = 0;
		m_nSTATIC_LIGHT = 0;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bSKINNING = false;
		m_bDYNAMIC_LIGHT = false;
		m_bSTATIC_LIGHT = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bSKINNING && m_bDYNAMIC_LIGHT && m_bSTATIC_LIGHT );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nSKINNING ) + ( 4 * m_nDOWATERFOG ) + ( 8 * m_nDYNAMIC_LIGHT ) + ( 16 * m_nSTATIC_LIGHT ) + 0;
	}
};

#define shaderDynamicTest_eyes_vs20 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_SKINNING + vsh_forgot_to_set_dynamic_DYNAMIC_LIGHT + vsh_forgot_to_set_dynamic_STATIC_LIGHT


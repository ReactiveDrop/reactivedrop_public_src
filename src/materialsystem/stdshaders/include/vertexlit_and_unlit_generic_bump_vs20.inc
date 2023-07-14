// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $STATICLIGHT3 || $MORPHING || $SKINNING || $COMPRESSED_VERTS) && $TESSELLATION
// ( $DOWATERFOG == 0 ) && ( $DOPIXELFOG && $HARDWAREFOGBLEND )
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class vertexlit_and_unlit_generic_bump_vs20_Static_Index
{
	unsigned int m_nHALFLAMBERT : 2;
	unsigned int m_nUSE_WITH_2B : 2;
	unsigned int m_nNORMALIZEEYEVEC : 2;
	unsigned int m_nWORLD_NORMAL : 1;
	unsigned int m_nSTATICLIGHT3 : 1;
	unsigned int m_nDOPIXELFOG : 2;
	unsigned int m_nHARDWAREFOGBLEND : 2;
#ifdef _DEBUG
	bool m_bHALFLAMBERT : 1;
	bool m_bUSE_WITH_2B : 1;
	bool m_bNORMALIZEEYEVEC : 1;
	bool m_bWORLD_NORMAL : 1;
	bool m_bSTATICLIGHT3 : 1;
#endif	// _DEBUG
public:
	void SetHALFLAMBERT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHALFLAMBERT = i;
#ifdef _DEBUG
		m_bHALFLAMBERT = true;
#endif	// _DEBUG
	}

	void SetUSE_WITH_2B( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nUSE_WITH_2B = i;
#ifdef _DEBUG
		m_bUSE_WITH_2B = true;
#endif	// _DEBUG
	}

	void SetNORMALIZEEYEVEC( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nNORMALIZEEYEVEC = i;
#ifdef _DEBUG
		m_bNORMALIZEEYEVEC = true;
#endif	// _DEBUG
	}

	void SetWORLD_NORMAL( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nWORLD_NORMAL = i;
#ifdef _DEBUG
		m_bWORLD_NORMAL = true;
#endif	// _DEBUG
	}

	void SetSTATICLIGHT3( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nSTATICLIGHT3 = i;
#ifdef _DEBUG
		m_bSTATICLIGHT3 = true;
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

	vertexlit_and_unlit_generic_bump_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nHALFLAMBERT = 0;
		m_nUSE_WITH_2B = 0;
		m_nNORMALIZEEYEVEC = 0;
		m_nWORLD_NORMAL = 0;
		m_nSTATICLIGHT3 = 0;
		m_nDOPIXELFOG = ( g_pHardwareConfig->SupportsPixelShaders_2_b() && !IS_FLAG_SET( MATERIAL_VAR_VERTEXFOG ) ) ? 1 : 0 ;
		m_nHARDWAREFOGBLEND = !g_pHardwareConfig->SupportsPixelShaders_2_b() ;
#ifdef _DEBUG
		m_bHALFLAMBERT = false;
		m_bUSE_WITH_2B = false;
		m_bNORMALIZEEYEVEC = false;
		m_bWORLD_NORMAL = false;
		m_bSTATICLIGHT3 = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bHALFLAMBERT && m_bUSE_WITH_2B && m_bNORMALIZEEYEVEC && m_bWORLD_NORMAL && m_bSTATICLIGHT3 );
		return ( 8 * m_nHALFLAMBERT ) + ( 16 * m_nUSE_WITH_2B ) + ( 32 * m_nNORMALIZEEYEVEC ) + ( 64 * m_nWORLD_NORMAL ) + ( 64 * m_nSTATICLIGHT3 ) + ( 64 * m_nDOPIXELFOG ) + ( 128 * m_nHARDWAREFOGBLEND ) + 0;
	}
};

#define shaderStaticTest_vertexlit_and_unlit_generic_bump_vs20 vsh_forgot_to_set_static_HALFLAMBERT + vsh_forgot_to_set_static_USE_WITH_2B + vsh_forgot_to_set_static_NORMALIZEEYEVEC + vsh_forgot_to_set_static_WORLD_NORMAL + vsh_forgot_to_set_static_STATICLIGHT3


class vertexlit_and_unlit_generic_bump_vs20_Dynamic_Index
{
	unsigned int m_nCOMPRESSED_VERTS : 2;
	unsigned int m_nSKINNING : 2;
	unsigned int m_nTESSELLATION : 1;
	unsigned int m_nDOWATERFOG : 2;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bSKINNING : 1;
	bool m_bTESSELLATION : 1;
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

	void SetTESSELLATION( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nTESSELLATION = i;
#ifdef _DEBUG
		m_bTESSELLATION = true;
#endif	// _DEBUG
	}

	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
	}

	vertexlit_and_unlit_generic_bump_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nSKINNING = 0;
		m_nTESSELLATION = 0;
		m_nDOWATERFOG = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bSKINNING = false;
		m_bTESSELLATION = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bSKINNING && m_bTESSELLATION );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nSKINNING ) + ( 4 * m_nTESSELLATION ) + ( 4 * m_nDOWATERFOG ) + 0;
	}
};

#define shaderDynamicTest_vertexlit_and_unlit_generic_bump_vs20 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_SKINNING + vsh_forgot_to_set_dynamic_TESSELLATION


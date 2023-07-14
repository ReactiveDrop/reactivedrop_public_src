// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ($SEPARATE_DETAIL_UVS) && ($SEAMLESS_DETAIL)
// ( $FOW ) && ( $SEAMLESS_DETAIL )
// ( $FOW ) && ( $SEAMLESS_BASE )
// ( $FOW ) && ( $TESSELLATION )
// ($TREESWAY) && ($MORPHING)
// ($TREESWAY) && ($TESSELLATION)
// ($TREESWAY) && ($FOW)
// ( $TREESWAY ) && ( $SEAMLESS_DETAIL || $SEAMLESS_BASE )
// ( $DOWATERFOG == 0 ) && ( $DOPIXELFOG && $HARDWAREFOGBLEND )
// ( $STATICLIGHT3 || $VERTEXCOLOR || $SEAMLESS_BASE || $SEAMLESS_DETAIL || $SEPARATE_DETAIL_UVS || $MORPHING || $SKINNING || $COMPRESSED_VERTS ) && $TESSELLATION
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class vertexlit_and_unlit_generic_vs30_Static_Index
{
	unsigned int m_nVERTEXCOLOR : 2;
	unsigned int m_nCUBEMAP : 2;
	unsigned int m_nHALFLAMBERT : 2;
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nSEAMLESS_BASE : 2;
	unsigned int m_nSEAMLESS_DETAIL : 2;
	unsigned int m_nSEPARATE_DETAIL_UVS : 2;
	unsigned int m_nDECAL : 2;
	unsigned int m_nSTATICLIGHT3 : 1;
	unsigned int m_nLIGHTING_PREVIEW : 3;
	unsigned int m_nFOW : 2;
	unsigned int m_nTREESWAY : 2;
	unsigned int m_nDOPIXELFOG : 2;
	unsigned int m_nHARDWAREFOGBLEND : 1;
#ifdef _DEBUG
	bool m_bVERTEXCOLOR : 1;
	bool m_bCUBEMAP : 1;
	bool m_bHALFLAMBERT : 1;
	bool m_bFLASHLIGHT : 1;
	bool m_bSEAMLESS_BASE : 1;
	bool m_bSEAMLESS_DETAIL : 1;
	bool m_bSEPARATE_DETAIL_UVS : 1;
	bool m_bDECAL : 1;
	bool m_bSTATICLIGHT3 : 1;
	bool m_bLIGHTING_PREVIEW : 1;
	bool m_bFOW : 1;
	bool m_bTREESWAY : 1;
#endif	// _DEBUG
public:
	void SetVERTEXCOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVERTEXCOLOR = i;
#ifdef _DEBUG
		m_bVERTEXCOLOR = true;
#endif	// _DEBUG
	}

	void SetCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCUBEMAP = i;
#ifdef _DEBUG
		m_bCUBEMAP = true;
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

	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif	// _DEBUG
	}

	void SetSEAMLESS_BASE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS_BASE = i;
#ifdef _DEBUG
		m_bSEAMLESS_BASE = true;
#endif	// _DEBUG
	}

	void SetSEAMLESS_DETAIL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS_DETAIL = i;
#ifdef _DEBUG
		m_bSEAMLESS_DETAIL = true;
#endif	// _DEBUG
	}

	void SetSEPARATE_DETAIL_UVS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEPARATE_DETAIL_UVS = i;
#ifdef _DEBUG
		m_bSEPARATE_DETAIL_UVS = true;
#endif	// _DEBUG
	}

	void SetDECAL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDECAL = i;
#ifdef _DEBUG
		m_bDECAL = true;
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

	void SetLIGHTING_PREVIEW( int i )
	{
		Assert( i >= 0 && i <= 3 );
		m_nLIGHTING_PREVIEW = i;
#ifdef _DEBUG
		m_bLIGHTING_PREVIEW = true;
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

	void SetTREESWAY( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nTREESWAY = i;
#ifdef _DEBUG
		m_bTREESWAY = true;
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

	vertexlit_and_unlit_generic_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nVERTEXCOLOR = 0;
		m_nCUBEMAP = 0;
		m_nHALFLAMBERT = 0;
		m_nFLASHLIGHT = 0;
		m_nSEAMLESS_BASE = 0;
		m_nSEAMLESS_DETAIL = 0;
		m_nSEPARATE_DETAIL_UVS = 0;
		m_nDECAL = 0;
		m_nSTATICLIGHT3 = 0;
		m_nLIGHTING_PREVIEW = 0;
		m_nFOW = 0;
		m_nTREESWAY = 0;
		m_nDOPIXELFOG = ( g_pHardwareConfig->SupportsPixelShaders_2_b() && !IS_FLAG_SET( MATERIAL_VAR_VERTEXFOG ) ) ? 1 : 0 ;
		m_nHARDWAREFOGBLEND = 0 ;
#ifdef _DEBUG
		m_bVERTEXCOLOR = false;
		m_bCUBEMAP = false;
		m_bHALFLAMBERT = false;
		m_bFLASHLIGHT = false;
		m_bSEAMLESS_BASE = false;
		m_bSEAMLESS_DETAIL = false;
		m_bSEPARATE_DETAIL_UVS = false;
		m_bDECAL = false;
		m_bSTATICLIGHT3 = false;
		m_bLIGHTING_PREVIEW = false;
		m_bFOW = false;
		m_bTREESWAY = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bVERTEXCOLOR && m_bCUBEMAP && m_bHALFLAMBERT && m_bFLASHLIGHT && m_bSEAMLESS_BASE && m_bSEAMLESS_DETAIL && m_bSEPARATE_DETAIL_UVS && m_bDECAL && m_bSTATICLIGHT3 && m_bLIGHTING_PREVIEW && m_bFOW && m_bTREESWAY );
		AssertMsg( !( m_nSEPARATE_DETAIL_UVS && m_nSEAMLESS_DETAIL ), "Invalid combo combination ( SEPARATE_DETAIL_UVS && SEAMLESS_DETAIL )" );
		AssertMsg( !( m_nFOW && m_nSEAMLESS_DETAIL ), "Invalid combo combination ( FOW && SEAMLESS_DETAIL )" );
		AssertMsg( !( m_nFOW && m_nSEAMLESS_BASE ), "Invalid combo combination ( FOW && SEAMLESS_BASE )" );
		AssertMsg( !( m_nTREESWAY && m_nFOW ), "Invalid combo combination ( TREESWAY && FOW )" );
		AssertMsg( !( m_nTREESWAY && ( m_nSEAMLESS_DETAIL || m_nSEAMLESS_BASE ) ), "Invalid combo combination ( TREESWAY && ( SEAMLESS_DETAIL || SEAMLESS_BASE ) )" );
		return ( 96 * m_nVERTEXCOLOR ) + ( 192 * m_nCUBEMAP ) + ( 384 * m_nHALFLAMBERT ) + ( 768 * m_nFLASHLIGHT ) + ( 1536 * m_nSEAMLESS_BASE ) + ( 3072 * m_nSEAMLESS_DETAIL ) + ( 6144 * m_nSEPARATE_DETAIL_UVS ) + ( 12288 * m_nDECAL ) + ( 24576 * m_nSTATICLIGHT3 ) + ( 24576 * m_nLIGHTING_PREVIEW ) + ( 98304 * m_nFOW ) + ( 196608 * m_nTREESWAY ) + ( 589824 * m_nDOPIXELFOG ) + ( 1179648 * m_nHARDWAREFOGBLEND ) + 0;
	}
};

#define shaderStaticTest_vertexlit_and_unlit_generic_vs30 vsh_forgot_to_set_static_VERTEXCOLOR + vsh_forgot_to_set_static_CUBEMAP + vsh_forgot_to_set_static_HALFLAMBERT + vsh_forgot_to_set_static_FLASHLIGHT + vsh_forgot_to_set_static_SEAMLESS_BASE + vsh_forgot_to_set_static_SEAMLESS_DETAIL + vsh_forgot_to_set_static_SEPARATE_DETAIL_UVS + vsh_forgot_to_set_static_DECAL + vsh_forgot_to_set_static_STATICLIGHT3 + vsh_forgot_to_set_static_LIGHTING_PREVIEW + vsh_forgot_to_set_static_FOW + vsh_forgot_to_set_static_TREESWAY


class vertexlit_and_unlit_generic_vs30_Dynamic_Index
{
	unsigned int m_nCOMPRESSED_VERTS : 2;
	unsigned int m_nDYNAMIC_LIGHT : 2;
	unsigned int m_nSKINNING : 2;
	unsigned int m_nMORPHING : 2;
	unsigned int m_nTESSELLATION : 2;
	unsigned int m_nDOWATERFOG : 2;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bDYNAMIC_LIGHT : 1;
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

	void SetDYNAMIC_LIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDYNAMIC_LIGHT = i;
#ifdef _DEBUG
		m_bDYNAMIC_LIGHT = true;
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

	void SetMORPHING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMORPHING = i;
	}

	void SetTESSELLATION( int i )
	{
		Assert( i >= 0 && i <= 2 );
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

	vertexlit_and_unlit_generic_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nDYNAMIC_LIGHT = 0;
		m_nSKINNING = 0;
		m_nMORPHING = pShaderAPI->IsHWMorphingEnabled() ;
		m_nTESSELLATION = 0;
		m_nDOWATERFOG = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bDYNAMIC_LIGHT = false;
		m_bSKINNING = false;
		m_bTESSELLATION = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bDYNAMIC_LIGHT && m_bSKINNING && m_bTESSELLATION );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nDYNAMIC_LIGHT ) + ( 4 * m_nSKINNING ) + ( 8 * m_nMORPHING ) + ( 16 * m_nTESSELLATION ) + ( 48 * m_nDOWATERFOG ) + 0;
	}
};

#define shaderDynamicTest_vertexlit_and_unlit_generic_vs30 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_DYNAMIC_LIGHT + vsh_forgot_to_set_dynamic_SKINNING + vsh_forgot_to_set_dynamic_TESSELLATION


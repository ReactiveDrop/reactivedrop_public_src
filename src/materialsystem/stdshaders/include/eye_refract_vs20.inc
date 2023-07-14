// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $DOWATERFOG == 0 ) && ( $DOPIXELFOG && $HARDWAREFOGBLEND )
// ( $DOWATERFOG == 0 ) && ( $HARDWAREFOGBLEND == 0 ) && ( $DOPIXELFOG == 0 )
// ( $WORLD_NORMAL ) && $INTRO
// ( $MORPHING || $SKINNING || $COMPRESSED_VERTS ) && $TESSELLATION
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class eye_refract_vs20_Static_Index
{
	unsigned int m_nINTRO : 2;
	unsigned int m_nHALFLAMBERT : 2;
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nLIGHTWARPTEXTURE : 2;
	unsigned int m_nWORLD_NORMAL : 1;
	unsigned int m_nDOPIXELFOG : 2;
	unsigned int m_nHARDWAREFOGBLEND : 2;
#ifdef _DEBUG
	bool m_bINTRO : 1;
	bool m_bHALFLAMBERT : 1;
	bool m_bFLASHLIGHT : 1;
	bool m_bLIGHTWARPTEXTURE : 1;
	bool m_bWORLD_NORMAL : 1;
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

	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif	// _DEBUG
	}

	void SetLIGHTWARPTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTWARPTEXTURE = i;
#ifdef _DEBUG
		m_bLIGHTWARPTEXTURE = true;
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

	eye_refract_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nINTRO = 0;
		m_nHALFLAMBERT = 0;
		m_nFLASHLIGHT = 0;
		m_nLIGHTWARPTEXTURE = 0;
		m_nWORLD_NORMAL = 0;
		m_nDOPIXELFOG = g_pHardwareConfig->SupportsPixelShaders_2_b() ;
		m_nHARDWAREFOGBLEND = !g_pHardwareConfig->SupportsPixelShaders_2_b() ;
#ifdef _DEBUG
		m_bINTRO = false;
		m_bHALFLAMBERT = false;
		m_bFLASHLIGHT = false;
		m_bLIGHTWARPTEXTURE = false;
		m_bWORLD_NORMAL = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bINTRO && m_bHALFLAMBERT && m_bFLASHLIGHT && m_bLIGHTWARPTEXTURE && m_bWORLD_NORMAL );
		AssertMsg( !( m_nWORLD_NORMAL && m_nINTRO ), "Invalid combo combination ( WORLD_NORMAL && INTRO )" );
		return ( 80 * m_nINTRO ) + ( 160 * m_nHALFLAMBERT ) + ( 320 * m_nFLASHLIGHT ) + ( 640 * m_nLIGHTWARPTEXTURE ) + ( 1280 * m_nWORLD_NORMAL ) + ( 1280 * m_nDOPIXELFOG ) + ( 2560 * m_nHARDWAREFOGBLEND ) + 0;
	}
};

#define shaderStaticTest_eye_refract_vs20 vsh_forgot_to_set_static_INTRO + vsh_forgot_to_set_static_HALFLAMBERT + vsh_forgot_to_set_static_FLASHLIGHT + vsh_forgot_to_set_static_LIGHTWARPTEXTURE + vsh_forgot_to_set_static_WORLD_NORMAL


class eye_refract_vs20_Dynamic_Index
{
	unsigned int m_nCOMPRESSED_VERTS : 2;
	unsigned int m_nSKINNING : 2;
	unsigned int m_nDOWATERFOG : 2;
	unsigned int m_nDYNAMIC_LIGHT : 2;
	unsigned int m_nNUM_LIGHTS : 3;
	unsigned int m_nTESSELLATION : 1;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bSKINNING : 1;
	bool m_bDYNAMIC_LIGHT : 1;
	bool m_bNUM_LIGHTS : 1;
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

	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
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

	eye_refract_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nSKINNING = 0;
		m_nDOWATERFOG = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
		m_nDYNAMIC_LIGHT = 0;
		m_nNUM_LIGHTS = 0;
		m_nTESSELLATION = 0;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bSKINNING = false;
		m_bDYNAMIC_LIGHT = false;
		m_bNUM_LIGHTS = false;
		m_bTESSELLATION = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bSKINNING && m_bDYNAMIC_LIGHT && m_bNUM_LIGHTS && m_bTESSELLATION );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nSKINNING ) + ( 4 * m_nDOWATERFOG ) + ( 8 * m_nDYNAMIC_LIGHT ) + ( 16 * m_nNUM_LIGHTS ) + ( 80 * m_nTESSELLATION ) + 0;
	}
};

#define shaderDynamicTest_eye_refract_vs20 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_SKINNING + vsh_forgot_to_set_dynamic_DYNAMIC_LIGHT + vsh_forgot_to_set_dynamic_NUM_LIGHTS + vsh_forgot_to_set_dynamic_TESSELLATION


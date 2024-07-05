// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $DOWATERFOG == 0 ) && ( $DOPIXELFOG && $HARDWAREFOGBLEND )
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class character_vs20_Static_Index
{
	unsigned int m_nWORLD_NORMAL : 1;
	unsigned int m_nDOPIXELFOG : 2;
	unsigned int m_nHARDWAREFOGBLEND : 2;
#ifdef _DEBUG
	bool m_bWORLD_NORMAL : 1;
#endif	// _DEBUG
public:
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

	character_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nWORLD_NORMAL = 0;
		m_nDOPIXELFOG = ( g_pHardwareConfig->SupportsPixelShaders_2_b() && !IS_FLAG_SET( MATERIAL_VAR_VERTEXFOG ) ) ? 1 : 0 ;
		m_nHARDWAREFOGBLEND = !g_pHardwareConfig->SupportsPixelShaders_2_b() ;
#ifdef _DEBUG
		m_bWORLD_NORMAL = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bWORLD_NORMAL );
		return ( 8 * m_nWORLD_NORMAL ) + ( 8 * m_nDOPIXELFOG ) + ( 16 * m_nHARDWAREFOGBLEND ) + 0;
	}
};

#define shaderStaticTest_character_vs20 vsh_forgot_to_set_static_WORLD_NORMAL


class character_vs20_Dynamic_Index
{
	unsigned int m_nCOMPRESSED_VERTS : 2;
	unsigned int m_nSKINNING : 2;
	unsigned int m_nDOWATERFOG : 2;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bSKINNING : 1;
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

	character_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nSKINNING = 0;
		m_nDOWATERFOG = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bSKINNING = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bSKINNING );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nSKINNING ) + ( 4 * m_nDOWATERFOG ) + 0;
	}
};

#define shaderDynamicTest_character_vs20 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_SKINNING


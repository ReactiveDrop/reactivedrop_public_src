// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $DOWATERFOG == 0 ) && ( $DOPIXELFOG && $HARDWAREFOGBLEND )
// ( $DOWATERFOG == 0 ) && ( $HARDWAREFOGBLEND == 0 ) && ( $DOPIXELFOG == 0 )
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class lightmappedgeneric_decal_vs20_Static_Index
{
	unsigned int m_nDOPIXELFOG : 2;
	unsigned int m_nHARDWAREFOGBLEND : 2;
public:
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

	lightmappedgeneric_decal_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nDOPIXELFOG = g_pHardwareConfig->SupportsPixelShaders_2_b() ;
		m_nHARDWAREFOGBLEND = !g_pHardwareConfig->SupportsPixelShaders_2_b() ;
	}

	int GetIndex() const
	{
		return ( 2 * m_nDOPIXELFOG ) + ( 4 * m_nHARDWAREFOGBLEND ) + 0;
	}
};

#define shaderStaticTest_lightmappedgeneric_decal_vs20 1


class lightmappedgeneric_decal_vs20_Dynamic_Index
{
	unsigned int m_nDOWATERFOG : 2;
public:
	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
	}

	lightmappedgeneric_decal_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nDOWATERFOG = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
	}

	int GetIndex() const
	{
		return ( 1 * m_nDOWATERFOG ) + 0;
	}
};

#define shaderDynamicTest_lightmappedgeneric_decal_vs20 1


// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class WaterCheap_vs20_Static_Index
{
	unsigned int m_nBLEND : 2;
#ifdef _DEBUG
	bool m_bBLEND : 1;
#endif	// _DEBUG
public:
	void SetBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBLEND = i;
#ifdef _DEBUG
		m_bBLEND = true;
#endif	// _DEBUG
	}

	WaterCheap_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBLEND = 0;
#ifdef _DEBUG
		m_bBLEND = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBLEND );
		return ( 1 * m_nBLEND ) + 0;
	}
};

#define shaderStaticTest_WaterCheap_vs20 vsh_forgot_to_set_static_BLEND


class WaterCheap_vs20_Dynamic_Index
{
public:
	WaterCheap_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_WaterCheap_vs20 1


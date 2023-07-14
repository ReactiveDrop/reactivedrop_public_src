// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class screenspaceeffect_vs20_Static_Index
{
	unsigned int m_nVERTEXCOLOR : 2;
	unsigned int m_nTRANSFORMVERTS : 2;
public:
	void SetVERTEXCOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVERTEXCOLOR = i;
	}

	void SetTRANSFORMVERTS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTRANSFORMVERTS = i;
	}

	screenspaceeffect_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nVERTEXCOLOR = 0;
		m_nTRANSFORMVERTS = 0;
	}

	int GetIndex() const
	{
		return ( 1 * m_nVERTEXCOLOR ) + ( 2 * m_nTRANSFORMVERTS ) + 0;
	}
};

#define shaderStaticTest_screenspaceeffect_vs20 1


class screenspaceeffect_vs20_Dynamic_Index
{
public:
	screenspaceeffect_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_screenspaceeffect_vs20 1


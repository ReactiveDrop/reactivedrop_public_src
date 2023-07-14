// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class morphaccumulate_vs30_Static_Index
{
	unsigned int m_nCONSTANTBASEDMORPH : 2;
#ifdef _DEBUG
	bool m_bCONSTANTBASEDMORPH : 1;
#endif	// _DEBUG
public:
	void SetCONSTANTBASEDMORPH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCONSTANTBASEDMORPH = i;
#ifdef _DEBUG
		m_bCONSTANTBASEDMORPH = true;
#endif	// _DEBUG
	}

	morphaccumulate_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nCONSTANTBASEDMORPH = 0;
#ifdef _DEBUG
		m_bCONSTANTBASEDMORPH = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCONSTANTBASEDMORPH );
		return ( 1 * m_nCONSTANTBASEDMORPH ) + 0;
	}
};

#define shaderStaticTest_morphaccumulate_vs30 vsh_forgot_to_set_static_CONSTANTBASEDMORPH


class morphaccumulate_vs30_Dynamic_Index
{
public:
	morphaccumulate_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_morphaccumulate_vs30 1


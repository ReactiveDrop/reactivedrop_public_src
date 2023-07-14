// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#pragma once
#include "shaderlib/cshader.h"
class debugdrawenvmapmask_ps20b_Static_Index
{
public:
	debugdrawenvmapmask_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_debugdrawenvmapmask_ps20b 1


class debugdrawenvmapmask_ps20b_Dynamic_Index
{
	unsigned int m_nSHOWALPHA : 2;
#ifdef _DEBUG
	bool m_bSHOWALPHA : 1;
#endif	// _DEBUG
public:
	void SetSHOWALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSHOWALPHA = i;
#ifdef _DEBUG
		m_bSHOWALPHA = true;
#endif	// _DEBUG
	}

	debugdrawenvmapmask_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nSHOWALPHA = 0;
#ifdef _DEBUG
		m_bSHOWALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bSHOWALPHA );
		return ( 1 * m_nSHOWALPHA ) + 0;
	}
};

#define shaderDynamicTest_debugdrawenvmapmask_ps20b psh_forgot_to_set_dynamic_SHOWALPHA


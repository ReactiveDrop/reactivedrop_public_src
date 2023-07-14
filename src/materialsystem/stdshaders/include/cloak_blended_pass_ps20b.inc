// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#pragma once
#include "shaderlib/cshader.h"
class cloak_blended_pass_ps20b_Static_Index
{
	unsigned int m_nBUMPMAP : 2;
#ifdef _DEBUG
	bool m_bBUMPMAP : 1;
#endif	// _DEBUG
public:
	void SetBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMAP = i;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif	// _DEBUG
	}

	cloak_blended_pass_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBUMPMAP = 0;
#ifdef _DEBUG
		m_bBUMPMAP = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBUMPMAP );
		return ( 1 * m_nBUMPMAP ) + 0;
	}
};

#define shaderStaticTest_cloak_blended_pass_ps20b psh_forgot_to_set_static_BUMPMAP


class cloak_blended_pass_ps20b_Dynamic_Index
{
public:
	cloak_blended_pass_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_cloak_blended_pass_ps20b 1


// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#pragma once
#include "shaderlib/cshader.h"
class motion_blur_ps20b_Static_Index
{
public:
	motion_blur_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_motion_blur_ps20b 1


class motion_blur_ps20b_Dynamic_Index
{
	unsigned int m_nD_NUM_BLUR_SAMPLES : 4;
#ifdef _DEBUG
	bool m_bD_NUM_BLUR_SAMPLES : 1;
#endif	// _DEBUG
public:
	void SetD_NUM_BLUR_SAMPLES( int i )
	{
		Assert( i >= 0 && i <= 14 );
		m_nD_NUM_BLUR_SAMPLES = i;
#ifdef _DEBUG
		m_bD_NUM_BLUR_SAMPLES = true;
#endif	// _DEBUG
	}

	motion_blur_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nD_NUM_BLUR_SAMPLES = 0;
#ifdef _DEBUG
		m_bD_NUM_BLUR_SAMPLES = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bD_NUM_BLUR_SAMPLES );
		return ( 1 * m_nD_NUM_BLUR_SAMPLES ) + 0;
	}
};

#define shaderDynamicTest_motion_blur_ps20b psh_forgot_to_set_dynamic_D_NUM_BLUR_SAMPLES


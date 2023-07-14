// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#pragma once
#include "shaderlib/cshader.h"
class DebugTextureView_ps20b_Static_Index
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

	DebugTextureView_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nSHOWALPHA = 0;
#ifdef _DEBUG
		m_bSHOWALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bSHOWALPHA );
		return ( 2 * m_nSHOWALPHA ) + 0;
	}
};

#define shaderStaticTest_DebugTextureView_ps20b psh_forgot_to_set_static_SHOWALPHA


class DebugTextureView_ps20b_Dynamic_Index
{
	unsigned int m_nISCUBEMAP : 2;
#ifdef _DEBUG
	bool m_bISCUBEMAP : 1;
#endif	// _DEBUG
public:
	void SetISCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nISCUBEMAP = i;
#ifdef _DEBUG
		m_bISCUBEMAP = true;
#endif	// _DEBUG
	}

	DebugTextureView_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nISCUBEMAP = 0;
#ifdef _DEBUG
		m_bISCUBEMAP = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bISCUBEMAP );
		return ( 1 * m_nISCUBEMAP ) + 0;
	}
};

#define shaderDynamicTest_DebugTextureView_ps20b psh_forgot_to_set_dynamic_ISCUBEMAP


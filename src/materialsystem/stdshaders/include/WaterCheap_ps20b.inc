// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#pragma once
#include "shaderlib/cshader.h"
class WaterCheap_ps20b_Static_Index
{
	unsigned int m_nMULTITEXTURE : 2;
	unsigned int m_nFRESNEL : 2;
	unsigned int m_nBLEND : 2;
	unsigned int m_nREFRACTALPHA : 2;
	unsigned int m_nHDRTYPE : 2;
	unsigned int m_nFLOWMAP : 2;
	unsigned int m_nFLOW_DEBUG : 2;
#ifdef _DEBUG
	bool m_bMULTITEXTURE : 1;
	bool m_bFRESNEL : 1;
	bool m_bBLEND : 1;
	bool m_bREFRACTALPHA : 1;
	bool m_bHDRTYPE : 1;
	bool m_bFLOWMAP : 1;
	bool m_bFLOW_DEBUG : 1;
#endif	// _DEBUG
public:
	void SetMULTITEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMULTITEXTURE = i;
#ifdef _DEBUG
		m_bMULTITEXTURE = true;
#endif	// _DEBUG
	}

	void SetFRESNEL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFRESNEL = i;
#ifdef _DEBUG
		m_bFRESNEL = true;
#endif	// _DEBUG
	}

	void SetBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBLEND = i;
#ifdef _DEBUG
		m_bBLEND = true;
#endif	// _DEBUG
	}

	void SetREFRACTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nREFRACTALPHA = i;
#ifdef _DEBUG
		m_bREFRACTALPHA = true;
#endif	// _DEBUG
	}

	void SetHDRTYPE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nHDRTYPE = i;
#ifdef _DEBUG
		m_bHDRTYPE = true;
#endif	// _DEBUG
	}

	void SetFLOWMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLOWMAP = i;
#ifdef _DEBUG
		m_bFLOWMAP = true;
#endif	// _DEBUG
	}

	void SetFLOW_DEBUG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLOW_DEBUG = i;
#ifdef _DEBUG
		m_bFLOW_DEBUG = true;
#endif	// _DEBUG
	}

	WaterCheap_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nMULTITEXTURE = 0;
		m_nFRESNEL = 0;
		m_nBLEND = 0;
		m_nREFRACTALPHA = 0;
		m_nHDRTYPE = 0;
		m_nFLOWMAP = 0;
		m_nFLOW_DEBUG = 0;
#ifdef _DEBUG
		m_bMULTITEXTURE = false;
		m_bFRESNEL = false;
		m_bBLEND = false;
		m_bREFRACTALPHA = false;
		m_bHDRTYPE = false;
		m_bFLOWMAP = false;
		m_bFLOW_DEBUG = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bMULTITEXTURE && m_bFRESNEL && m_bBLEND && m_bREFRACTALPHA && m_bHDRTYPE && m_bFLOWMAP && m_bFLOW_DEBUG );
		return ( 4 * m_nMULTITEXTURE ) + ( 8 * m_nFRESNEL ) + ( 16 * m_nBLEND ) + ( 32 * m_nREFRACTALPHA ) + ( 64 * m_nHDRTYPE ) + ( 192 * m_nFLOWMAP ) + ( 384 * m_nFLOW_DEBUG ) + 0;
	}
};

#define shaderStaticTest_WaterCheap_ps20b psh_forgot_to_set_static_MULTITEXTURE + psh_forgot_to_set_static_FRESNEL + psh_forgot_to_set_static_BLEND + psh_forgot_to_set_static_REFRACTALPHA + psh_forgot_to_set_static_HDRTYPE + psh_forgot_to_set_static_FLOWMAP + psh_forgot_to_set_static_FLOW_DEBUG


class WaterCheap_ps20b_Dynamic_Index
{
	unsigned int m_nHDRENABLED : 2;
	unsigned int m_nPIXELFOGTYPE : 2;
#ifdef _DEBUG
	bool m_bHDRENABLED : 1;
#endif	// _DEBUG
public:
	void SetHDRENABLED( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHDRENABLED = i;
#ifdef _DEBUG
		m_bHDRENABLED = true;
#endif	// _DEBUG
	}

	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
	}

	WaterCheap_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nHDRENABLED = 0;
		m_nPIXELFOGTYPE = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
#ifdef _DEBUG
		m_bHDRENABLED = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bHDRENABLED );
		return ( 1 * m_nHDRENABLED ) + ( 2 * m_nPIXELFOGTYPE ) + 0;
	}
};

#define shaderDynamicTest_WaterCheap_ps20b psh_forgot_to_set_dynamic_HDRENABLED


// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#pragma once
#include "shaderlib/cshader.h"
class cloak_ps20b_Static_Index
{
	unsigned int m_nLIGHTWARPTEXTURE : 2;
#ifdef _DEBUG
	bool m_bLIGHTWARPTEXTURE : 1;
#endif	// _DEBUG
public:
	void SetLIGHTWARPTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTWARPTEXTURE = i;
#ifdef _DEBUG
		m_bLIGHTWARPTEXTURE = true;
#endif	// _DEBUG
	}

	cloak_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nLIGHTWARPTEXTURE = 0;
#ifdef _DEBUG
		m_bLIGHTWARPTEXTURE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bLIGHTWARPTEXTURE );
		return ( 20 * m_nLIGHTWARPTEXTURE ) + 0;
	}
};

#define shaderStaticTest_cloak_ps20b psh_forgot_to_set_static_LIGHTWARPTEXTURE


class cloak_ps20b_Dynamic_Index
{
	unsigned int m_nPIXELFOGTYPE : 2;
	unsigned int m_nWRITEWATERFOGTODESTALPHA : 2;
	unsigned int m_nNUM_LIGHTS : 3;
#ifdef _DEBUG
	bool m_bWRITEWATERFOGTODESTALPHA : 1;
	bool m_bNUM_LIGHTS : 1;
#endif	// _DEBUG
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
	}

	void SetWRITEWATERFOGTODESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITEWATERFOGTODESTALPHA = i;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif	// _DEBUG
	}

	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif	// _DEBUG
	}

	cloak_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nPIXELFOGTYPE = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
		m_nWRITEWATERFOGTODESTALPHA = 0;
		m_nNUM_LIGHTS = 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = false;
		m_bNUM_LIGHTS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bWRITEWATERFOGTODESTALPHA && m_bNUM_LIGHTS );
		AssertMsg( !( 1 && ( 1 && ( ( m_nPIXELFOGTYPE != 1 ) && m_nWRITEWATERFOGTODESTALPHA ) ) ), "Invalid combo combination ( 1 && ( 1 && ( ( PIXELFOGTYPE != 1 ) && WRITEWATERFOGTODESTALPHA ) ) )" );
		return ( 1 * m_nPIXELFOGTYPE ) + ( 2 * m_nWRITEWATERFOGTODESTALPHA ) + ( 4 * m_nNUM_LIGHTS ) + 0;
	}
};

#define shaderDynamicTest_cloak_ps20b psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_NUM_LIGHTS


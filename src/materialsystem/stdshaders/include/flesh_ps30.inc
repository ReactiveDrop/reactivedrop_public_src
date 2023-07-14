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
class flesh_ps30_Static_Index
{
	unsigned int m_nALPHABLEND : 2;
	unsigned int m_nTRANSMAT : 2;
	unsigned int m_nFRESNEL_WARP : 2;
	unsigned int m_nEFFECTS : 2;
	unsigned int m_nTINTING : 2;
	unsigned int m_nIRIDESCENCE : 2;
	unsigned int m_nBACK_SCATTER : 2;
	unsigned int m_nFORWARD_SCATTER : 2;
	unsigned int m_nHIGH_PRECISION_DEPTH : 2;
	unsigned int m_nINTERIOR_LAYER : 2;
	unsigned int m_nOPACITY_TEXTURE : 2;
	unsigned int m_nNORMAL2SOFT : 2;
	unsigned int m_nDETAIL : 2;
	unsigned int m_nFLASHLIGHTDEPTHFILTERMODE : 2;
#ifdef _DEBUG
	bool m_bALPHABLEND : 1;
	bool m_bTRANSMAT : 1;
	bool m_bFRESNEL_WARP : 1;
	bool m_bEFFECTS : 1;
	bool m_bTINTING : 1;
	bool m_bIRIDESCENCE : 1;
	bool m_bBACK_SCATTER : 1;
	bool m_bFORWARD_SCATTER : 1;
	bool m_bHIGH_PRECISION_DEPTH : 1;
	bool m_bINTERIOR_LAYER : 1;
	bool m_bOPACITY_TEXTURE : 1;
	bool m_bNORMAL2SOFT : 1;
	bool m_bDETAIL : 1;
	bool m_bFLASHLIGHTDEPTHFILTERMODE : 1;
#endif	// _DEBUG
public:
	void SetALPHABLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nALPHABLEND = i;
#ifdef _DEBUG
		m_bALPHABLEND = true;
#endif	// _DEBUG
	}

	void SetTRANSMAT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTRANSMAT = i;
#ifdef _DEBUG
		m_bTRANSMAT = true;
#endif	// _DEBUG
	}

	void SetFRESNEL_WARP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFRESNEL_WARP = i;
#ifdef _DEBUG
		m_bFRESNEL_WARP = true;
#endif	// _DEBUG
	}

	void SetEFFECTS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nEFFECTS = i;
#ifdef _DEBUG
		m_bEFFECTS = true;
#endif	// _DEBUG
	}

	void SetTINTING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTINTING = i;
#ifdef _DEBUG
		m_bTINTING = true;
#endif	// _DEBUG
	}

	void SetIRIDESCENCE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nIRIDESCENCE = i;
#ifdef _DEBUG
		m_bIRIDESCENCE = true;
#endif	// _DEBUG
	}

	void SetBACK_SCATTER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBACK_SCATTER = i;
#ifdef _DEBUG
		m_bBACK_SCATTER = true;
#endif	// _DEBUG
	}

	void SetFORWARD_SCATTER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFORWARD_SCATTER = i;
#ifdef _DEBUG
		m_bFORWARD_SCATTER = true;
#endif	// _DEBUG
	}

	void SetHIGH_PRECISION_DEPTH( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHIGH_PRECISION_DEPTH = i;
#ifdef _DEBUG
		m_bHIGH_PRECISION_DEPTH = true;
#endif	// _DEBUG
	}

	void SetINTERIOR_LAYER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nINTERIOR_LAYER = i;
#ifdef _DEBUG
		m_bINTERIOR_LAYER = true;
#endif	// _DEBUG
	}

	void SetOPACITY_TEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nOPACITY_TEXTURE = i;
#ifdef _DEBUG
		m_bOPACITY_TEXTURE = true;
#endif	// _DEBUG
	}

	void SetNORMAL2SOFT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nNORMAL2SOFT = i;
#ifdef _DEBUG
		m_bNORMAL2SOFT = true;
#endif	// _DEBUG
	}

	void SetDETAIL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAIL = i;
#ifdef _DEBUG
		m_bDETAIL = true;
#endif	// _DEBUG
	}

	void SetFLASHLIGHTDEPTHFILTERMODE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nFLASHLIGHTDEPTHFILTERMODE = i;
#ifdef _DEBUG
		m_bFLASHLIGHTDEPTHFILTERMODE = true;
#endif	// _DEBUG
	}

	flesh_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nALPHABLEND = 0;
		m_nTRANSMAT = 0;
		m_nFRESNEL_WARP = 0;
		m_nEFFECTS = 0;
		m_nTINTING = 0;
		m_nIRIDESCENCE = 0;
		m_nBACK_SCATTER = 0;
		m_nFORWARD_SCATTER = 0;
		m_nHIGH_PRECISION_DEPTH = 0;
		m_nINTERIOR_LAYER = 0;
		m_nOPACITY_TEXTURE = 0;
		m_nNORMAL2SOFT = 0;
		m_nDETAIL = 0;
		m_nFLASHLIGHTDEPTHFILTERMODE = 0;
#ifdef _DEBUG
		m_bALPHABLEND = false;
		m_bTRANSMAT = false;
		m_bFRESNEL_WARP = false;
		m_bEFFECTS = false;
		m_bTINTING = false;
		m_bIRIDESCENCE = false;
		m_bBACK_SCATTER = false;
		m_bFORWARD_SCATTER = false;
		m_bHIGH_PRECISION_DEPTH = false;
		m_bINTERIOR_LAYER = false;
		m_bOPACITY_TEXTURE = false;
		m_bNORMAL2SOFT = false;
		m_bDETAIL = false;
		m_bFLASHLIGHTDEPTHFILTERMODE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bALPHABLEND && m_bTRANSMAT && m_bFRESNEL_WARP && m_bEFFECTS && m_bTINTING && m_bIRIDESCENCE && m_bBACK_SCATTER && m_bFORWARD_SCATTER && m_bHIGH_PRECISION_DEPTH && m_bINTERIOR_LAYER && m_bOPACITY_TEXTURE && m_bNORMAL2SOFT && m_bDETAIL && m_bFLASHLIGHTDEPTHFILTERMODE );
		return ( 40 * m_nALPHABLEND ) + ( 80 * m_nTRANSMAT ) + ( 160 * m_nFRESNEL_WARP ) + ( 320 * m_nEFFECTS ) + ( 640 * m_nTINTING ) + ( 1280 * m_nIRIDESCENCE ) + ( 2560 * m_nBACK_SCATTER ) + ( 5120 * m_nFORWARD_SCATTER ) + ( 10240 * m_nHIGH_PRECISION_DEPTH ) + ( 20480 * m_nINTERIOR_LAYER ) + ( 40960 * m_nOPACITY_TEXTURE ) + ( 81920 * m_nNORMAL2SOFT ) + ( 163840 * m_nDETAIL ) + ( 327680 * m_nFLASHLIGHTDEPTHFILTERMODE ) + 0;
	}
};

#define shaderStaticTest_flesh_ps30 psh_forgot_to_set_static_ALPHABLEND + psh_forgot_to_set_static_TRANSMAT + psh_forgot_to_set_static_FRESNEL_WARP + psh_forgot_to_set_static_EFFECTS + psh_forgot_to_set_static_TINTING + psh_forgot_to_set_static_IRIDESCENCE + psh_forgot_to_set_static_BACK_SCATTER + psh_forgot_to_set_static_FORWARD_SCATTER + psh_forgot_to_set_static_HIGH_PRECISION_DEPTH + psh_forgot_to_set_static_INTERIOR_LAYER + psh_forgot_to_set_static_OPACITY_TEXTURE + psh_forgot_to_set_static_NORMAL2SOFT + psh_forgot_to_set_static_DETAIL + psh_forgot_to_set_static_FLASHLIGHTDEPTHFILTERMODE


class flesh_ps30_Dynamic_Index
{
	unsigned int m_nPIXELFOGTYPE : 2;
	unsigned int m_nNUM_LIGHTS : 3;
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nFLASHLIGHTSHADOWS : 2;
#ifdef _DEBUG
	bool m_bNUM_LIGHTS : 1;
	bool m_bFLASHLIGHT : 1;
	bool m_bFLASHLIGHTSHADOWS : 1;
#endif	// _DEBUG
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
	}

	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif	// _DEBUG
	}

	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif	// _DEBUG
	}

	void SetFLASHLIGHTSHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHTSHADOWS = i;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif	// _DEBUG
	}

	flesh_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nPIXELFOGTYPE = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
		m_nNUM_LIGHTS = 0;
		m_nFLASHLIGHT = 0;
		m_nFLASHLIGHTSHADOWS = 0;
#ifdef _DEBUG
		m_bNUM_LIGHTS = false;
		m_bFLASHLIGHT = false;
		m_bFLASHLIGHTSHADOWS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bNUM_LIGHTS && m_bFLASHLIGHT && m_bFLASHLIGHTSHADOWS );
		return ( 1 * m_nPIXELFOGTYPE ) + ( 2 * m_nNUM_LIGHTS ) + ( 10 * m_nFLASHLIGHT ) + ( 20 * m_nFLASHLIGHTSHADOWS ) + 0;
	}
};

#define shaderDynamicTest_flesh_ps30 psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_FLASHLIGHT + psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS


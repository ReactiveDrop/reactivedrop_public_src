// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// $MASKED && $BLUR
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
class Refract_ps20b_Static_Index
{
	unsigned int m_nMAGNIFY : 2;
	unsigned int m_nBLUR : 2;
	unsigned int m_nFADEOUTONSILHOUETTE : 2;
	unsigned int m_nCUBEMAP : 2;
	unsigned int m_nREFRACTTINTTEXTURE : 2;
	unsigned int m_nMASKED : 2;
	unsigned int m_nCOLORMODULATE : 2;
	unsigned int m_nSECONDARY_NORMAL : 2;
	unsigned int m_nMIRRORABOUTVIEWPORTEDGES : 1;
#ifdef _DEBUG
	bool m_bMAGNIFY : 1;
	bool m_bBLUR : 1;
	bool m_bFADEOUTONSILHOUETTE : 1;
	bool m_bCUBEMAP : 1;
	bool m_bREFRACTTINTTEXTURE : 1;
	bool m_bMASKED : 1;
	bool m_bCOLORMODULATE : 1;
	bool m_bSECONDARY_NORMAL : 1;
	bool m_bMIRRORABOUTVIEWPORTEDGES : 1;
#endif	// _DEBUG
public:
	void SetMAGNIFY( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMAGNIFY = i;
#ifdef _DEBUG
		m_bMAGNIFY = true;
#endif	// _DEBUG
	}

	void SetBLUR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBLUR = i;
#ifdef _DEBUG
		m_bBLUR = true;
#endif	// _DEBUG
	}

	void SetFADEOUTONSILHOUETTE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFADEOUTONSILHOUETTE = i;
#ifdef _DEBUG
		m_bFADEOUTONSILHOUETTE = true;
#endif	// _DEBUG
	}

	void SetCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCUBEMAP = i;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif	// _DEBUG
	}

	void SetREFRACTTINTTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nREFRACTTINTTEXTURE = i;
#ifdef _DEBUG
		m_bREFRACTTINTTEXTURE = true;
#endif	// _DEBUG
	}

	void SetMASKED( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMASKED = i;
#ifdef _DEBUG
		m_bMASKED = true;
#endif	// _DEBUG
	}

	void SetCOLORMODULATE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOLORMODULATE = i;
#ifdef _DEBUG
		m_bCOLORMODULATE = true;
#endif	// _DEBUG
	}

	void SetSECONDARY_NORMAL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSECONDARY_NORMAL = i;
#ifdef _DEBUG
		m_bSECONDARY_NORMAL = true;
#endif	// _DEBUG
	}

	void SetMIRRORABOUTVIEWPORTEDGES( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nMIRRORABOUTVIEWPORTEDGES = i;
#ifdef _DEBUG
		m_bMIRRORABOUTVIEWPORTEDGES = true;
#endif	// _DEBUG
	}

	Refract_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nMAGNIFY = 0;
		m_nBLUR = 0;
		m_nFADEOUTONSILHOUETTE = 0;
		m_nCUBEMAP = 0;
		m_nREFRACTTINTTEXTURE = 0;
		m_nMASKED = 0;
		m_nCOLORMODULATE = 0;
		m_nSECONDARY_NORMAL = 0;
		m_nMIRRORABOUTVIEWPORTEDGES = 0;
#ifdef _DEBUG
		m_bMAGNIFY = false;
		m_bBLUR = false;
		m_bFADEOUTONSILHOUETTE = false;
		m_bCUBEMAP = false;
		m_bREFRACTTINTTEXTURE = false;
		m_bMASKED = false;
		m_bCOLORMODULATE = false;
		m_bSECONDARY_NORMAL = false;
		m_bMIRRORABOUTVIEWPORTEDGES = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bMAGNIFY && m_bBLUR && m_bFADEOUTONSILHOUETTE && m_bCUBEMAP && m_bREFRACTTINTTEXTURE && m_bMASKED && m_bCOLORMODULATE && m_bSECONDARY_NORMAL && m_bMIRRORABOUTVIEWPORTEDGES );
		AssertMsg( !( m_nMASKED && m_nBLUR ), "Invalid combo combination ( MASKED && BLUR )" );
		return ( 4 * m_nMAGNIFY ) + ( 8 * m_nBLUR ) + ( 16 * m_nFADEOUTONSILHOUETTE ) + ( 32 * m_nCUBEMAP ) + ( 64 * m_nREFRACTTINTTEXTURE ) + ( 128 * m_nMASKED ) + ( 256 * m_nCOLORMODULATE ) + ( 512 * m_nSECONDARY_NORMAL ) + ( 1024 * m_nMIRRORABOUTVIEWPORTEDGES ) + 0;
	}
};

#define shaderStaticTest_Refract_ps20b psh_forgot_to_set_static_MAGNIFY + psh_forgot_to_set_static_BLUR + psh_forgot_to_set_static_FADEOUTONSILHOUETTE + psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_REFRACTTINTTEXTURE + psh_forgot_to_set_static_MASKED + psh_forgot_to_set_static_COLORMODULATE + psh_forgot_to_set_static_SECONDARY_NORMAL + psh_forgot_to_set_static_MIRRORABOUTVIEWPORTEDGES


class Refract_ps20b_Dynamic_Index
{
	unsigned int m_nPIXELFOGTYPE : 2;
	unsigned int m_nWRITE_DEPTH_TO_DESTALPHA : 2;
#ifdef _DEBUG
	bool m_bWRITE_DEPTH_TO_DESTALPHA : 1;
#endif	// _DEBUG
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
	}

	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITE_DEPTH_TO_DESTALPHA = i;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif	// _DEBUG
	}

	Refract_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nPIXELFOGTYPE = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bWRITE_DEPTH_TO_DESTALPHA );
		return ( 1 * m_nPIXELFOGTYPE ) + ( 2 * m_nWRITE_DEPTH_TO_DESTALPHA ) + 0;
	}
};

#define shaderDynamicTest_Refract_ps20b psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA


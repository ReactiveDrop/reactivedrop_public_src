// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $FLASHLIGHT == 1 ) && ( $CUBEMAP == 1 )
// ($DETAILTEXTURE == 0 ) && ( $DETAIL_BLEND_MODE != 0 )
// ($DETAILTEXTURE == 0 ) && ( $SEAMLESS_DETAIL )
// ($ENVMAPMASK || $SELFILLUM_ENVMAPMASK_ALPHA) && ($SEAMLESS_BASE || $SEAMLESS_DETAIL)
// $BASEALPHAENVMAPMASK && $ENVMAPMASK
// $BASEALPHAENVMAPMASK && $SELFILLUM
// $SELFILLUM && $SELFILLUM_ENVMAPMASK_ALPHA
// $SELFILLUM_ENVMAPMASK_ALPHA && (! $ENVMAPMASK)
// $BASEALPHAENVMAPMASK && ($SEAMLESS_BASE || $SEAMLESS_DETAIL)
// ($DISTANCEALPHA == 0) && ($DISTANCEALPHAFROMDETAIL || $SOFT_MASK || $OUTLINE || $OUTER_GLOW)
// ($DETAILTEXTURE == 0) && ($DISTANCEALPHAFROMDETAIL)
// ( $CUBEMAP == 0 ) && ( ( $ENVMAPFRESNEL == 1 ) || ( $BASEALPHAENVMAPMASK == 1 ) )
// ( $CUBEMAP == 0 ) && ( $ENVMAPMASK == 1 ) && ( $SELFILLUM_ENVMAPMASK_ALPHA == 0 )
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTSHADOWS == 1 )
// ($DISTANCEALPHA) && ($ENVMAPMASK || $BASEALPHAENVMAPMASK || $SELFILLUM || $SELFILLUM_ENVMAPMASK_ALPHA || $ENVMAPFRESNEL)
// ($DISTANCEALPHA) && ($SEAMLESS_BASE || $SEAMLESS_DETAIL || $CUBEMAP )
// ($DISTANCEALPHA) && ($WRITEWATERFOGTODESTALPHA || $PIXELFOGTYPE || $FLASHLIGHT || $FLASHLIGHTSHADOWS )
// $DESATURATEWITHBASEALPHA && ( $SEAMLESS_BASE || $DETAILTEXTURE )
// $DESATURATEWITHBASEALPHA && ( $SELFILLUM || $SELFILLUM_ENVMAPMASK_ALPHA )
// $DESATURATEWITHBASEALPHA && $BASEALPHAENVMAPMASK
// $FLASHLIGHTSHADOWS && ($LIGHTING_PREVIEW == 3)
// ( $FOW == 1 ) && ( $DETAILTEXTURE == 1 )
// ( $FOW == 1 ) && ( $SEAMLESS_DETAIL == 1)
// ( $FOW == 1 ) && ( $SEAMLESS_BASE == 1 )
// ( $FOW == 1 ) && ( $DESATURATEWITHBASEALPHA == 1 )
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
class vertexlit_and_unlit_generic_ps20b_Static_Index
{
	unsigned int m_nDETAILTEXTURE : 2;
	unsigned int m_nCUBEMAP : 2;
	unsigned int m_nDIFFUSELIGHTING : 2;
	unsigned int m_nENVMAPMASK : 2;
	unsigned int m_nBASEALPHAENVMAPMASK : 2;
	unsigned int m_nSELFILLUM : 2;
	unsigned int m_nVERTEXCOLOR : 2;
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nSELFILLUM_ENVMAPMASK_ALPHA : 2;
	unsigned int m_nDETAIL_BLEND_MODE : 4;
	unsigned int m_nSEAMLESS_BASE : 2;
	unsigned int m_nSEAMLESS_DETAIL : 2;
	unsigned int m_nDISTANCEALPHA : 2;
	unsigned int m_nDISTANCEALPHAFROMDETAIL : 2;
	unsigned int m_nSOFT_MASK : 2;
	unsigned int m_nOUTLINE : 2;
	unsigned int m_nOUTER_GLOW : 2;
	unsigned int m_nFLASHLIGHTDEPTHFILTERMODE : 2;
	unsigned int m_nSHADER_SRGB_READ : 1;
	unsigned int m_nDESATURATEWITHBASEALPHA : 2;
	unsigned int m_nLIGHTING_PREVIEW : 3;
	unsigned int m_nENVMAPFRESNEL : 2;
	unsigned int m_nFOW : 2;
	unsigned int m_nDOPIXELFOG : 2;
#ifdef _DEBUG
	bool m_bDETAILTEXTURE : 1;
	bool m_bCUBEMAP : 1;
	bool m_bDIFFUSELIGHTING : 1;
	bool m_bENVMAPMASK : 1;
	bool m_bBASEALPHAENVMAPMASK : 1;
	bool m_bSELFILLUM : 1;
	bool m_bVERTEXCOLOR : 1;
	bool m_bFLASHLIGHT : 1;
	bool m_bSELFILLUM_ENVMAPMASK_ALPHA : 1;
	bool m_bDETAIL_BLEND_MODE : 1;
	bool m_bSEAMLESS_BASE : 1;
	bool m_bSEAMLESS_DETAIL : 1;
	bool m_bDISTANCEALPHA : 1;
	bool m_bDISTANCEALPHAFROMDETAIL : 1;
	bool m_bSOFT_MASK : 1;
	bool m_bOUTLINE : 1;
	bool m_bOUTER_GLOW : 1;
	bool m_bFLASHLIGHTDEPTHFILTERMODE : 1;
	bool m_bSHADER_SRGB_READ : 1;
	bool m_bDESATURATEWITHBASEALPHA : 1;
	bool m_bLIGHTING_PREVIEW : 1;
	bool m_bENVMAPFRESNEL : 1;
	bool m_bFOW : 1;
#endif	// _DEBUG
public:
	void SetDETAILTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAILTEXTURE = i;
#ifdef _DEBUG
		m_bDETAILTEXTURE = true;
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

	void SetDIFFUSELIGHTING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDIFFUSELIGHTING = i;
#ifdef _DEBUG
		m_bDIFFUSELIGHTING = true;
#endif	// _DEBUG
	}

	void SetENVMAPMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAPMASK = i;
#ifdef _DEBUG
		m_bENVMAPMASK = true;
#endif	// _DEBUG
	}

	void SetBASEALPHAENVMAPMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASEALPHAENVMAPMASK = i;
#ifdef _DEBUG
		m_bBASEALPHAENVMAPMASK = true;
#endif	// _DEBUG
	}

	void SetSELFILLUM( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSELFILLUM = i;
#ifdef _DEBUG
		m_bSELFILLUM = true;
#endif	// _DEBUG
	}

	void SetVERTEXCOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVERTEXCOLOR = i;
#ifdef _DEBUG
		m_bVERTEXCOLOR = true;
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

	void SetSELFILLUM_ENVMAPMASK_ALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSELFILLUM_ENVMAPMASK_ALPHA = i;
#ifdef _DEBUG
		m_bSELFILLUM_ENVMAPMASK_ALPHA = true;
#endif	// _DEBUG
	}

	void SetDETAIL_BLEND_MODE( int i )
	{
		Assert( i >= 0 && i <= 9 );
		m_nDETAIL_BLEND_MODE = i;
#ifdef _DEBUG
		m_bDETAIL_BLEND_MODE = true;
#endif	// _DEBUG
	}

	void SetSEAMLESS_BASE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS_BASE = i;
#ifdef _DEBUG
		m_bSEAMLESS_BASE = true;
#endif	// _DEBUG
	}

	void SetSEAMLESS_DETAIL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS_DETAIL = i;
#ifdef _DEBUG
		m_bSEAMLESS_DETAIL = true;
#endif	// _DEBUG
	}

	void SetDISTANCEALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDISTANCEALPHA = i;
#ifdef _DEBUG
		m_bDISTANCEALPHA = true;
#endif	// _DEBUG
	}

	void SetDISTANCEALPHAFROMDETAIL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDISTANCEALPHAFROMDETAIL = i;
#ifdef _DEBUG
		m_bDISTANCEALPHAFROMDETAIL = true;
#endif	// _DEBUG
	}

	void SetSOFT_MASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSOFT_MASK = i;
#ifdef _DEBUG
		m_bSOFT_MASK = true;
#endif	// _DEBUG
	}

	void SetOUTLINE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nOUTLINE = i;
#ifdef _DEBUG
		m_bOUTLINE = true;
#endif	// _DEBUG
	}

	void SetOUTER_GLOW( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nOUTER_GLOW = i;
#ifdef _DEBUG
		m_bOUTER_GLOW = true;
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

	void SetSHADER_SRGB_READ( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nSHADER_SRGB_READ = i;
#ifdef _DEBUG
		m_bSHADER_SRGB_READ = true;
#endif	// _DEBUG
	}

	void SetDESATURATEWITHBASEALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDESATURATEWITHBASEALPHA = i;
#ifdef _DEBUG
		m_bDESATURATEWITHBASEALPHA = true;
#endif	// _DEBUG
	}

	void SetLIGHTING_PREVIEW( int i )
	{
		Assert( i >= 0 && i <= 3 );
		m_nLIGHTING_PREVIEW = i;
#ifdef _DEBUG
		m_bLIGHTING_PREVIEW = true;
#endif	// _DEBUG
	}

	void SetENVMAPFRESNEL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAPFRESNEL = i;
#ifdef _DEBUG
		m_bENVMAPFRESNEL = true;
#endif	// _DEBUG
	}

	void SetFOW( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFOW = i;
#ifdef _DEBUG
		m_bFOW = true;
#endif	// _DEBUG
	}

	void SetDOPIXELFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOPIXELFOG = i;
	}

	vertexlit_and_unlit_generic_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nDETAILTEXTURE = 0;
		m_nCUBEMAP = 0;
		m_nDIFFUSELIGHTING = 0;
		m_nENVMAPMASK = 0;
		m_nBASEALPHAENVMAPMASK = 0;
		m_nSELFILLUM = 0;
		m_nVERTEXCOLOR = 0;
		m_nFLASHLIGHT = 0;
		m_nSELFILLUM_ENVMAPMASK_ALPHA = 0;
		m_nDETAIL_BLEND_MODE = 0;
		m_nSEAMLESS_BASE = 0;
		m_nSEAMLESS_DETAIL = 0;
		m_nDISTANCEALPHA = 0;
		m_nDISTANCEALPHAFROMDETAIL = 0;
		m_nSOFT_MASK = 0;
		m_nOUTLINE = 0;
		m_nOUTER_GLOW = 0;
		m_nFLASHLIGHTDEPTHFILTERMODE = 0;
		m_nSHADER_SRGB_READ = 0;
		m_nDESATURATEWITHBASEALPHA = 0;
		m_nLIGHTING_PREVIEW = 0;
		m_nENVMAPFRESNEL = 0;
		m_nFOW = 0;
		m_nDOPIXELFOG = !IS_FLAG_SET( MATERIAL_VAR_VERTEXFOG ) ? 1 : 0 ;
#ifdef _DEBUG
		m_bDETAILTEXTURE = false;
		m_bCUBEMAP = false;
		m_bDIFFUSELIGHTING = false;
		m_bENVMAPMASK = false;
		m_bBASEALPHAENVMAPMASK = false;
		m_bSELFILLUM = false;
		m_bVERTEXCOLOR = false;
		m_bFLASHLIGHT = false;
		m_bSELFILLUM_ENVMAPMASK_ALPHA = false;
		m_bDETAIL_BLEND_MODE = false;
		m_bSEAMLESS_BASE = false;
		m_bSEAMLESS_DETAIL = false;
		m_bDISTANCEALPHA = false;
		m_bDISTANCEALPHAFROMDETAIL = false;
		m_bSOFT_MASK = false;
		m_bOUTLINE = false;
		m_bOUTER_GLOW = false;
		m_bFLASHLIGHTDEPTHFILTERMODE = false;
		m_bSHADER_SRGB_READ = false;
		m_bDESATURATEWITHBASEALPHA = false;
		m_bLIGHTING_PREVIEW = false;
		m_bENVMAPFRESNEL = false;
		m_bFOW = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDETAILTEXTURE && m_bCUBEMAP && m_bDIFFUSELIGHTING && m_bENVMAPMASK && m_bBASEALPHAENVMAPMASK && m_bSELFILLUM && m_bVERTEXCOLOR && m_bFLASHLIGHT && m_bSELFILLUM_ENVMAPMASK_ALPHA && m_bDETAIL_BLEND_MODE && m_bSEAMLESS_BASE && m_bSEAMLESS_DETAIL && m_bDISTANCEALPHA && m_bDISTANCEALPHAFROMDETAIL && m_bSOFT_MASK && m_bOUTLINE && m_bOUTER_GLOW && m_bFLASHLIGHTDEPTHFILTERMODE && m_bSHADER_SRGB_READ && m_bDESATURATEWITHBASEALPHA && m_bLIGHTING_PREVIEW && m_bENVMAPFRESNEL && m_bFOW );
		AssertMsg( !( ( m_nFLASHLIGHT == 1 ) && ( m_nCUBEMAP == 1 ) ), "Invalid combo combination ( ( FLASHLIGHT == 1 ) && ( CUBEMAP == 1 ) )" );
		AssertMsg( !( ( m_nDETAILTEXTURE == 0 ) && ( m_nDETAIL_BLEND_MODE != 0 ) ), "Invalid combo combination ( ( DETAILTEXTURE == 0 ) && ( DETAIL_BLEND_MODE != 0 ) )" );
		AssertMsg( !( ( m_nDETAILTEXTURE == 0 ) && m_nSEAMLESS_DETAIL ), "Invalid combo combination ( ( DETAILTEXTURE == 0 ) && SEAMLESS_DETAIL )" );
		AssertMsg( !( ( m_nENVMAPMASK || m_nSELFILLUM_ENVMAPMASK_ALPHA ) && ( m_nSEAMLESS_BASE || m_nSEAMLESS_DETAIL ) ), "Invalid combo combination ( ( ENVMAPMASK || SELFILLUM_ENVMAPMASK_ALPHA ) && ( SEAMLESS_BASE || SEAMLESS_DETAIL ) )" );
		AssertMsg( !( m_nBASEALPHAENVMAPMASK && m_nENVMAPMASK ), "Invalid combo combination ( BASEALPHAENVMAPMASK && ENVMAPMASK )" );
		AssertMsg( !( m_nBASEALPHAENVMAPMASK && m_nSELFILLUM ), "Invalid combo combination ( BASEALPHAENVMAPMASK && SELFILLUM )" );
		AssertMsg( !( m_nSELFILLUM && m_nSELFILLUM_ENVMAPMASK_ALPHA ), "Invalid combo combination ( SELFILLUM && SELFILLUM_ENVMAPMASK_ALPHA )" );
		AssertMsg( !( m_nSELFILLUM_ENVMAPMASK_ALPHA && !m_nENVMAPMASK ), "Invalid combo combination ( SELFILLUM_ENVMAPMASK_ALPHA && !ENVMAPMASK )" );
		AssertMsg( !( m_nBASEALPHAENVMAPMASK && ( m_nSEAMLESS_BASE || m_nSEAMLESS_DETAIL ) ), "Invalid combo combination ( BASEALPHAENVMAPMASK && ( SEAMLESS_BASE || SEAMLESS_DETAIL ) )" );
		AssertMsg( !( ( m_nDISTANCEALPHA == 0 ) && ( m_nDISTANCEALPHAFROMDETAIL || ( m_nSOFT_MASK || ( m_nOUTLINE || m_nOUTER_GLOW ) ) ) ), "Invalid combo combination ( ( DISTANCEALPHA == 0 ) && ( DISTANCEALPHAFROMDETAIL || ( SOFT_MASK || ( OUTLINE || OUTER_GLOW ) ) ) )" );
		AssertMsg( !( ( m_nDETAILTEXTURE == 0 ) && m_nDISTANCEALPHAFROMDETAIL ), "Invalid combo combination ( ( DETAILTEXTURE == 0 ) && DISTANCEALPHAFROMDETAIL )" );
		AssertMsg( !( ( m_nCUBEMAP == 0 ) && ( ( m_nENVMAPFRESNEL == 1 ) || ( m_nBASEALPHAENVMAPMASK == 1 ) ) ), "Invalid combo combination ( ( CUBEMAP == 0 ) && ( ( ENVMAPFRESNEL == 1 ) || ( BASEALPHAENVMAPMASK == 1 ) ) )" );
		AssertMsg( !( ( m_nCUBEMAP == 0 ) && ( ( m_nENVMAPMASK == 1 ) && ( m_nSELFILLUM_ENVMAPMASK_ALPHA == 0 ) ) ), "Invalid combo combination ( ( CUBEMAP == 0 ) && ( ( ENVMAPMASK == 1 ) && ( SELFILLUM_ENVMAPMASK_ALPHA == 0 ) ) )" );
		AssertMsg( !( m_nDISTANCEALPHA && ( m_nENVMAPMASK || ( m_nBASEALPHAENVMAPMASK || ( m_nSELFILLUM || ( m_nSELFILLUM_ENVMAPMASK_ALPHA || m_nENVMAPFRESNEL ) ) ) ) ), "Invalid combo combination ( DISTANCEALPHA && ( ENVMAPMASK || ( BASEALPHAENVMAPMASK || ( SELFILLUM || ( SELFILLUM_ENVMAPMASK_ALPHA || ENVMAPFRESNEL ) ) ) ) )" );
		AssertMsg( !( m_nDISTANCEALPHA && ( m_nSEAMLESS_BASE || ( m_nSEAMLESS_DETAIL || m_nCUBEMAP ) ) ), "Invalid combo combination ( DISTANCEALPHA && ( SEAMLESS_BASE || ( SEAMLESS_DETAIL || CUBEMAP ) ) )" );
		AssertMsg( !( m_nDESATURATEWITHBASEALPHA && ( m_nSEAMLESS_BASE || m_nDETAILTEXTURE ) ), "Invalid combo combination ( DESATURATEWITHBASEALPHA && ( SEAMLESS_BASE || DETAILTEXTURE ) )" );
		AssertMsg( !( m_nDESATURATEWITHBASEALPHA && ( m_nSELFILLUM || m_nSELFILLUM_ENVMAPMASK_ALPHA ) ), "Invalid combo combination ( DESATURATEWITHBASEALPHA && ( SELFILLUM || SELFILLUM_ENVMAPMASK_ALPHA ) )" );
		AssertMsg( !( m_nDESATURATEWITHBASEALPHA && m_nBASEALPHAENVMAPMASK ), "Invalid combo combination ( DESATURATEWITHBASEALPHA && BASEALPHAENVMAPMASK )" );
		AssertMsg( !( ( m_nFOW == 1 ) && ( m_nDETAILTEXTURE == 1 ) ), "Invalid combo combination ( ( FOW == 1 ) && ( DETAILTEXTURE == 1 ) )" );
		AssertMsg( !( ( m_nFOW == 1 ) && ( m_nSEAMLESS_DETAIL == 1 ) ), "Invalid combo combination ( ( FOW == 1 ) && ( SEAMLESS_DETAIL == 1 ) )" );
		AssertMsg( !( ( m_nFOW == 1 ) && ( m_nSEAMLESS_BASE == 1 ) ), "Invalid combo combination ( ( FOW == 1 ) && ( SEAMLESS_BASE == 1 ) )" );
		AssertMsg( !( ( m_nFOW == 1 ) && ( m_nDESATURATEWITHBASEALPHA == 1 ) ), "Invalid combo combination ( ( FOW == 1 ) && ( DESATURATEWITHBASEALPHA == 1 ) )" );
		return ( 8 * m_nDETAILTEXTURE ) + ( 16 * m_nCUBEMAP ) + ( 32 * m_nDIFFUSELIGHTING ) + ( 64 * m_nENVMAPMASK ) + ( 128 * m_nBASEALPHAENVMAPMASK ) + ( 256 * m_nSELFILLUM ) + ( 512 * m_nVERTEXCOLOR ) + ( 1024 * m_nFLASHLIGHT ) + ( 2048 * m_nSELFILLUM_ENVMAPMASK_ALPHA ) + ( 4096 * m_nDETAIL_BLEND_MODE ) + ( 40960 * m_nSEAMLESS_BASE ) + ( 81920 * m_nSEAMLESS_DETAIL ) + ( 163840 * m_nDISTANCEALPHA ) + ( 327680 * m_nDISTANCEALPHAFROMDETAIL ) + ( 655360 * m_nSOFT_MASK ) + ( 1310720 * m_nOUTLINE ) + ( 2621440 * m_nOUTER_GLOW ) + ( 5242880 * m_nFLASHLIGHTDEPTHFILTERMODE ) + ( 15728640 * m_nSHADER_SRGB_READ ) + ( 15728640 * m_nDESATURATEWITHBASEALPHA ) + ( 31457280 * m_nLIGHTING_PREVIEW ) + ( 125829120 * m_nENVMAPFRESNEL ) + ( 251658240 * m_nFOW ) + ( 503316480 * m_nDOPIXELFOG ) + 0;
	}
};

#define shaderStaticTest_vertexlit_and_unlit_generic_ps20b psh_forgot_to_set_static_DETAILTEXTURE + psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_DIFFUSELIGHTING + psh_forgot_to_set_static_ENVMAPMASK + psh_forgot_to_set_static_BASEALPHAENVMAPMASK + psh_forgot_to_set_static_SELFILLUM + psh_forgot_to_set_static_VERTEXCOLOR + psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_SELFILLUM_ENVMAPMASK_ALPHA + psh_forgot_to_set_static_DETAIL_BLEND_MODE + psh_forgot_to_set_static_SEAMLESS_BASE + psh_forgot_to_set_static_SEAMLESS_DETAIL + psh_forgot_to_set_static_DISTANCEALPHA + psh_forgot_to_set_static_DISTANCEALPHAFROMDETAIL + psh_forgot_to_set_static_SOFT_MASK + psh_forgot_to_set_static_OUTLINE + psh_forgot_to_set_static_OUTER_GLOW + psh_forgot_to_set_static_FLASHLIGHTDEPTHFILTERMODE + psh_forgot_to_set_static_SHADER_SRGB_READ + psh_forgot_to_set_static_DESATURATEWITHBASEALPHA + psh_forgot_to_set_static_LIGHTING_PREVIEW + psh_forgot_to_set_static_ENVMAPFRESNEL + psh_forgot_to_set_static_FOW


class vertexlit_and_unlit_generic_ps20b_Dynamic_Index
{
	unsigned int m_nFLASHLIGHTSHADOWS : 2;
	unsigned int m_nWRITEWATERFOGTODESTALPHA : 2;
	unsigned int m_nPIXELFOGTYPE : 2;
#ifdef _DEBUG
	bool m_bFLASHLIGHTSHADOWS : 1;
	bool m_bWRITEWATERFOGTODESTALPHA : 1;
#endif	// _DEBUG
public:
	void SetFLASHLIGHTSHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHTSHADOWS = i;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif	// _DEBUG
	}

	void SetWRITEWATERFOGTODESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITEWATERFOGTODESTALPHA = i;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif	// _DEBUG
	}

	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
	}

	vertexlit_and_unlit_generic_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nFLASHLIGHTSHADOWS = 0;
		m_nWRITEWATERFOGTODESTALPHA = 0;
		m_nPIXELFOGTYPE = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = false;
		m_bWRITEWATERFOGTODESTALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFLASHLIGHTSHADOWS && m_bWRITEWATERFOGTODESTALPHA );
		AssertMsg( !( 1 && ( 1 && ( ( m_nPIXELFOGTYPE != 1 ) && m_nWRITEWATERFOGTODESTALPHA ) ) ), "Invalid combo combination ( 1 && ( 1 && ( ( PIXELFOGTYPE != 1 ) && WRITEWATERFOGTODESTALPHA ) ) )" );
		AssertMsg( !( 1 && ( 1 && ( ( m_nPIXELFOGTYPE != 1 ) && m_nWRITEWATERFOGTODESTALPHA ) ) ), "Invalid combo combination ( 1 && ( 1 && ( ( PIXELFOGTYPE != 1 ) && WRITEWATERFOGTODESTALPHA ) ) )" );
		AssertMsg( !( 1 && ( 1 && ( ( m_nPIXELFOGTYPE != 1 ) && m_nWRITEWATERFOGTODESTALPHA ) ) ), "Invalid combo combination ( 1 && ( 1 && ( ( PIXELFOGTYPE != 1 ) && WRITEWATERFOGTODESTALPHA ) ) )" );
		return ( 1 * m_nFLASHLIGHTSHADOWS ) + ( 2 * m_nWRITEWATERFOGTODESTALPHA ) + ( 4 * m_nPIXELFOGTYPE ) + 0;
	}
};

#define shaderDynamicTest_vertexlit_and_unlit_generic_ps20b psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS + psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA


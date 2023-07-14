// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $DISTANCEALPHA == 0 ) && ( $SOFTEDGES || $OUTLINE )
// $DISTANCEALPHA  && $DUALSEQUENCE
// $DISTANCEALPHA && ( $MAXLUMFRAMEBLEND2 || $MAXLUMFRAMEBLEND1 )
// ( $DUALSEQUENCE == 0 ) && ( $SEQUENCE_BLEND_MODE != 0 )
// $PACKED_INTERPOLATOR && ( $ANIMBLEND || $MAXLUMFRAMEBLEND1 || $EXTRACTGREENALPHA || $DUALSEQUENCE )
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#pragma once
#include "shaderlib/cshader.h"
class spritecard_ps20b_Static_Index
{
	unsigned int m_nDUALSEQUENCE : 2;
	unsigned int m_nSEQUENCE_BLEND_MODE : 2;
	unsigned int m_nADDBASETEXTURE2 : 2;
	unsigned int m_nMAXLUMFRAMEBLEND1 : 2;
	unsigned int m_nMAXLUMFRAMEBLEND2 : 2;
	unsigned int m_nEXTRACTGREENALPHA : 2;
	unsigned int m_nCOLORRAMP : 2;
	unsigned int m_nANIMBLEND : 2;
	unsigned int m_nADDSELF : 2;
	unsigned int m_nMOD2X : 2;
	unsigned int m_nDEPTHBLEND : 1;
	unsigned int m_nINVERSEDEPTHBLEND : 1;
	unsigned int m_nSHADER_SRGB_READ : 1;
	unsigned int m_nCOLOR_LERP_PS : 2;
	unsigned int m_nPACKED_INTERPOLATOR : 2;
	unsigned int m_nDISTANCEALPHA : 2;
	unsigned int m_nSOFTEDGES : 2;
	unsigned int m_nOUTLINE : 2;
#ifdef _DEBUG
	bool m_bDUALSEQUENCE : 1;
	bool m_bSEQUENCE_BLEND_MODE : 1;
	bool m_bADDBASETEXTURE2 : 1;
	bool m_bMAXLUMFRAMEBLEND1 : 1;
	bool m_bMAXLUMFRAMEBLEND2 : 1;
	bool m_bEXTRACTGREENALPHA : 1;
	bool m_bCOLORRAMP : 1;
	bool m_bANIMBLEND : 1;
	bool m_bADDSELF : 1;
	bool m_bMOD2X : 1;
	bool m_bDEPTHBLEND : 1;
	bool m_bINVERSEDEPTHBLEND : 1;
	bool m_bSHADER_SRGB_READ : 1;
	bool m_bCOLOR_LERP_PS : 1;
	bool m_bPACKED_INTERPOLATOR : 1;
	bool m_bDISTANCEALPHA : 1;
	bool m_bSOFTEDGES : 1;
	bool m_bOUTLINE : 1;
#endif	// _DEBUG
public:
	void SetDUALSEQUENCE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDUALSEQUENCE = i;
#ifdef _DEBUG
		m_bDUALSEQUENCE = true;
#endif	// _DEBUG
	}

	void SetSEQUENCE_BLEND_MODE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nSEQUENCE_BLEND_MODE = i;
#ifdef _DEBUG
		m_bSEQUENCE_BLEND_MODE = true;
#endif	// _DEBUG
	}

	void SetADDBASETEXTURE2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nADDBASETEXTURE2 = i;
#ifdef _DEBUG
		m_bADDBASETEXTURE2 = true;
#endif	// _DEBUG
	}

	void SetMAXLUMFRAMEBLEND1( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMAXLUMFRAMEBLEND1 = i;
#ifdef _DEBUG
		m_bMAXLUMFRAMEBLEND1 = true;
#endif	// _DEBUG
	}

	void SetMAXLUMFRAMEBLEND2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMAXLUMFRAMEBLEND2 = i;
#ifdef _DEBUG
		m_bMAXLUMFRAMEBLEND2 = true;
#endif	// _DEBUG
	}

	void SetEXTRACTGREENALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nEXTRACTGREENALPHA = i;
#ifdef _DEBUG
		m_bEXTRACTGREENALPHA = true;
#endif	// _DEBUG
	}

	void SetCOLORRAMP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOLORRAMP = i;
#ifdef _DEBUG
		m_bCOLORRAMP = true;
#endif	// _DEBUG
	}

	void SetANIMBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nANIMBLEND = i;
#ifdef _DEBUG
		m_bANIMBLEND = true;
#endif	// _DEBUG
	}

	void SetADDSELF( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nADDSELF = i;
#ifdef _DEBUG
		m_bADDSELF = true;
#endif	// _DEBUG
	}

	void SetMOD2X( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMOD2X = i;
#ifdef _DEBUG
		m_bMOD2X = true;
#endif	// _DEBUG
	}

	void SetDEPTHBLEND( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nDEPTHBLEND = i;
#ifdef _DEBUG
		m_bDEPTHBLEND = true;
#endif	// _DEBUG
	}

	void SetINVERSEDEPTHBLEND( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nINVERSEDEPTHBLEND = i;
#ifdef _DEBUG
		m_bINVERSEDEPTHBLEND = true;
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

	void SetCOLOR_LERP_PS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOLOR_LERP_PS = i;
#ifdef _DEBUG
		m_bCOLOR_LERP_PS = true;
#endif	// _DEBUG
	}

	void SetPACKED_INTERPOLATOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPACKED_INTERPOLATOR = i;
#ifdef _DEBUG
		m_bPACKED_INTERPOLATOR = true;
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

	void SetSOFTEDGES( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSOFTEDGES = i;
#ifdef _DEBUG
		m_bSOFTEDGES = true;
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

	spritecard_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nDUALSEQUENCE = 0;
		m_nSEQUENCE_BLEND_MODE = 0;
		m_nADDBASETEXTURE2 = 0;
		m_nMAXLUMFRAMEBLEND1 = 0;
		m_nMAXLUMFRAMEBLEND2 = 0;
		m_nEXTRACTGREENALPHA = 0;
		m_nCOLORRAMP = 0;
		m_nANIMBLEND = 0;
		m_nADDSELF = 0;
		m_nMOD2X = 0;
		m_nDEPTHBLEND = 0;
		m_nINVERSEDEPTHBLEND = 0;
		m_nSHADER_SRGB_READ = 0;
		m_nCOLOR_LERP_PS = 0;
		m_nPACKED_INTERPOLATOR = 0;
		m_nDISTANCEALPHA = 0;
		m_nSOFTEDGES = 0;
		m_nOUTLINE = 0;
#ifdef _DEBUG
		m_bDUALSEQUENCE = false;
		m_bSEQUENCE_BLEND_MODE = false;
		m_bADDBASETEXTURE2 = false;
		m_bMAXLUMFRAMEBLEND1 = false;
		m_bMAXLUMFRAMEBLEND2 = false;
		m_bEXTRACTGREENALPHA = false;
		m_bCOLORRAMP = false;
		m_bANIMBLEND = false;
		m_bADDSELF = false;
		m_bMOD2X = false;
		m_bDEPTHBLEND = false;
		m_bINVERSEDEPTHBLEND = false;
		m_bSHADER_SRGB_READ = false;
		m_bCOLOR_LERP_PS = false;
		m_bPACKED_INTERPOLATOR = false;
		m_bDISTANCEALPHA = false;
		m_bSOFTEDGES = false;
		m_bOUTLINE = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDUALSEQUENCE && m_bSEQUENCE_BLEND_MODE && m_bADDBASETEXTURE2 && m_bMAXLUMFRAMEBLEND1 && m_bMAXLUMFRAMEBLEND2 && m_bEXTRACTGREENALPHA && m_bCOLORRAMP && m_bANIMBLEND && m_bADDSELF && m_bMOD2X && m_bDEPTHBLEND && m_bINVERSEDEPTHBLEND && m_bSHADER_SRGB_READ && m_bCOLOR_LERP_PS && m_bPACKED_INTERPOLATOR && m_bDISTANCEALPHA && m_bSOFTEDGES && m_bOUTLINE );
		AssertMsg( !( ( m_nDISTANCEALPHA == 0 ) && ( m_nSOFTEDGES || m_nOUTLINE ) ), "Invalid combo combination ( ( DISTANCEALPHA == 0 ) && ( SOFTEDGES || OUTLINE ) )" );
		AssertMsg( !( m_nDISTANCEALPHA && m_nDUALSEQUENCE ), "Invalid combo combination ( DISTANCEALPHA && DUALSEQUENCE )" );
		AssertMsg( !( m_nDISTANCEALPHA && ( m_nMAXLUMFRAMEBLEND2 || m_nMAXLUMFRAMEBLEND1 ) ), "Invalid combo combination ( DISTANCEALPHA && ( MAXLUMFRAMEBLEND2 || MAXLUMFRAMEBLEND1 ) )" );
		AssertMsg( !( ( m_nDUALSEQUENCE == 0 ) && ( m_nSEQUENCE_BLEND_MODE != 0 ) ), "Invalid combo combination ( ( DUALSEQUENCE == 0 ) && ( SEQUENCE_BLEND_MODE != 0 ) )" );
		AssertMsg( !( m_nPACKED_INTERPOLATOR && ( m_nANIMBLEND || ( m_nMAXLUMFRAMEBLEND1 || ( m_nEXTRACTGREENALPHA || m_nDUALSEQUENCE ) ) ) ), "Invalid combo combination ( PACKED_INTERPOLATOR && ( ANIMBLEND || ( MAXLUMFRAMEBLEND1 || ( EXTRACTGREENALPHA || DUALSEQUENCE ) ) ) )" );
		return ( 1 * m_nDUALSEQUENCE ) + ( 2 * m_nSEQUENCE_BLEND_MODE ) + ( 6 * m_nADDBASETEXTURE2 ) + ( 12 * m_nMAXLUMFRAMEBLEND1 ) + ( 24 * m_nMAXLUMFRAMEBLEND2 ) + ( 48 * m_nEXTRACTGREENALPHA ) + ( 96 * m_nCOLORRAMP ) + ( 192 * m_nANIMBLEND ) + ( 384 * m_nADDSELF ) + ( 768 * m_nMOD2X ) + ( 1536 * m_nDEPTHBLEND ) + ( 1536 * m_nINVERSEDEPTHBLEND ) + ( 1536 * m_nSHADER_SRGB_READ ) + ( 1536 * m_nCOLOR_LERP_PS ) + ( 3072 * m_nPACKED_INTERPOLATOR ) + ( 6144 * m_nDISTANCEALPHA ) + ( 12288 * m_nSOFTEDGES ) + ( 24576 * m_nOUTLINE ) + 0;
	}
};

#define shaderStaticTest_spritecard_ps20b psh_forgot_to_set_static_DUALSEQUENCE + psh_forgot_to_set_static_SEQUENCE_BLEND_MODE + psh_forgot_to_set_static_ADDBASETEXTURE2 + psh_forgot_to_set_static_MAXLUMFRAMEBLEND1 + psh_forgot_to_set_static_MAXLUMFRAMEBLEND2 + psh_forgot_to_set_static_EXTRACTGREENALPHA + psh_forgot_to_set_static_COLORRAMP + psh_forgot_to_set_static_ANIMBLEND + psh_forgot_to_set_static_ADDSELF + psh_forgot_to_set_static_MOD2X + psh_forgot_to_set_static_DEPTHBLEND + psh_forgot_to_set_static_INVERSEDEPTHBLEND + psh_forgot_to_set_static_SHADER_SRGB_READ + psh_forgot_to_set_static_COLOR_LERP_PS + psh_forgot_to_set_static_PACKED_INTERPOLATOR + psh_forgot_to_set_static_DISTANCEALPHA + psh_forgot_to_set_static_SOFTEDGES + psh_forgot_to_set_static_OUTLINE


class spritecard_ps20b_Dynamic_Index
{
public:
	spritecard_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_spritecard_ps20b 1


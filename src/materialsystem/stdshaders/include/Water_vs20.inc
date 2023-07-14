// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// $MULTITEXTURE && $BASETEXTURE
// $FLOWMAP && $MULTITEXTURE
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTSHADOWS == 1 )
// ( $FLASHLIGHT || $LIGHTMAPWATERFOG ) && ( ( $BASETEXTURE && !$FLOWMAP ) || $MULTITEXTURE )
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class Water_vs20_Static_Index
{
	unsigned int m_nBASETEXTURE : 2;
	unsigned int m_nMULTITEXTURE : 2;
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nLIGHTMAPWATERFOG : 2;
	unsigned int m_nFLOWMAP : 2;
#ifdef _DEBUG
	bool m_bBASETEXTURE : 1;
	bool m_bMULTITEXTURE : 1;
	bool m_bFLASHLIGHT : 1;
	bool m_bLIGHTMAPWATERFOG : 1;
	bool m_bFLOWMAP : 1;
#endif	// _DEBUG
public:
	void SetBASETEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASETEXTURE = i;
#ifdef _DEBUG
		m_bBASETEXTURE = true;
#endif	// _DEBUG
	}

	void SetMULTITEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMULTITEXTURE = i;
#ifdef _DEBUG
		m_bMULTITEXTURE = true;
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

	void SetLIGHTMAPWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTMAPWATERFOG = i;
#ifdef _DEBUG
		m_bLIGHTMAPWATERFOG = true;
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

	Water_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBASETEXTURE = 0;
		m_nMULTITEXTURE = 0;
		m_nFLASHLIGHT = 0;
		m_nLIGHTMAPWATERFOG = 0;
		m_nFLOWMAP = 0;
#ifdef _DEBUG
		m_bBASETEXTURE = false;
		m_bMULTITEXTURE = false;
		m_bFLASHLIGHT = false;
		m_bLIGHTMAPWATERFOG = false;
		m_bFLOWMAP = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBASETEXTURE && m_bMULTITEXTURE && m_bFLASHLIGHT && m_bLIGHTMAPWATERFOG && m_bFLOWMAP );
		AssertMsg( !( m_nMULTITEXTURE && m_nBASETEXTURE ), "Invalid combo combination ( MULTITEXTURE && BASETEXTURE )" );
		AssertMsg( !( m_nFLOWMAP && m_nMULTITEXTURE ), "Invalid combo combination ( FLOWMAP && MULTITEXTURE )" );
		AssertMsg( !( ( m_nFLASHLIGHT || m_nLIGHTMAPWATERFOG ) && ( ( m_nBASETEXTURE && !m_nFLOWMAP ) || m_nMULTITEXTURE ) ), "Invalid combo combination ( ( FLASHLIGHT || LIGHTMAPWATERFOG ) && ( ( BASETEXTURE && !FLOWMAP ) || MULTITEXTURE ) )" );
		return ( 1 * m_nBASETEXTURE ) + ( 2 * m_nMULTITEXTURE ) + ( 4 * m_nFLASHLIGHT ) + ( 8 * m_nLIGHTMAPWATERFOG ) + ( 16 * m_nFLOWMAP ) + 0;
	}
};

#define shaderStaticTest_Water_vs20 vsh_forgot_to_set_static_BASETEXTURE + vsh_forgot_to_set_static_MULTITEXTURE + vsh_forgot_to_set_static_FLASHLIGHT + vsh_forgot_to_set_static_LIGHTMAPWATERFOG + vsh_forgot_to_set_static_FLOWMAP


class Water_vs20_Dynamic_Index
{
public:
	Water_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_Water_vs20 1


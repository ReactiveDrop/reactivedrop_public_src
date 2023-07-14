// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $MORPHING || $SKINNING || $COMPRESSED_VERTS ) && $TESSELLATION
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class depthwrite_vs30_Static_Index
{
	unsigned int m_nONLY_PROJECT_POSITION : 1;
	unsigned int m_nTREESWAY : 2;
#ifdef _DEBUG
	bool m_bONLY_PROJECT_POSITION : 1;
	bool m_bTREESWAY : 1;
#endif	// _DEBUG
public:
	void SetONLY_PROJECT_POSITION( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nONLY_PROJECT_POSITION = i;
#ifdef _DEBUG
		m_bONLY_PROJECT_POSITION = true;
#endif	// _DEBUG
	}

	void SetTREESWAY( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nTREESWAY = i;
#ifdef _DEBUG
		m_bTREESWAY = true;
#endif	// _DEBUG
	}

	depthwrite_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nONLY_PROJECT_POSITION = 0;
		m_nTREESWAY = 0;
#ifdef _DEBUG
		m_bONLY_PROJECT_POSITION = false;
		m_bTREESWAY = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bONLY_PROJECT_POSITION && m_bTREESWAY );
		return ( 24 * m_nONLY_PROJECT_POSITION ) + ( 24 * m_nTREESWAY ) + 0;
	}
};

#define shaderStaticTest_depthwrite_vs30 vsh_forgot_to_set_static_ONLY_PROJECT_POSITION + vsh_forgot_to_set_static_TREESWAY


class depthwrite_vs30_Dynamic_Index
{
	unsigned int m_nCOMPRESSED_VERTS : 2;
	unsigned int m_nSKINNING : 2;
	unsigned int m_nMORPHING : 2;
	unsigned int m_nTESSELLATION : 2;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bSKINNING : 1;
	bool m_bTESSELLATION : 1;
#endif	// _DEBUG
public:
	void SetCOMPRESSED_VERTS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOMPRESSED_VERTS = i;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = true;
#endif	// _DEBUG
	}

	void SetSKINNING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSKINNING = i;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif	// _DEBUG
	}

	void SetMORPHING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMORPHING = i;
	}

	void SetTESSELLATION( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nTESSELLATION = i;
#ifdef _DEBUG
		m_bTESSELLATION = true;
#endif	// _DEBUG
	}

	depthwrite_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nSKINNING = 0;
		m_nMORPHING = pShaderAPI->IsHWMorphingEnabled() ;
		m_nTESSELLATION = 0;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bSKINNING = false;
		m_bTESSELLATION = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bSKINNING && m_bTESSELLATION );
		AssertMsg( !( ( m_nMORPHING || ( m_nSKINNING || m_nCOMPRESSED_VERTS ) ) && m_nTESSELLATION ), "Invalid combo combination ( ( MORPHING || ( SKINNING || COMPRESSED_VERTS ) ) && TESSELLATION )" );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nSKINNING ) + ( 4 * m_nMORPHING ) + ( 8 * m_nTESSELLATION ) + 0;
	}
};

#define shaderDynamicTest_depthwrite_vs30 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_SKINNING + vsh_forgot_to_set_dynamic_TESSELLATION


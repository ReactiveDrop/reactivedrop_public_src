// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class DebugTextureView_vs20_Static_Index
{
public:
	DebugTextureView_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_DebugTextureView_vs20 1


class DebugTextureView_vs20_Dynamic_Index
{
	unsigned int m_nCOMPRESSED_VERTS : 2;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
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

	DebugTextureView_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS );
		return ( 1 * m_nCOMPRESSED_VERTS ) + 0;
	}
};

#define shaderDynamicTest_DebugTextureView_vs20 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS


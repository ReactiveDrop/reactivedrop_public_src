#pragma once
#include "shaderlib/cshader.h"
class depthwrite_ps20b_Static_Index
{
public:
	depthwrite_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_depthwrite_ps20b 1


class depthwrite_ps20b_Dynamic_Index
{
	unsigned int m_nALPHACLIP : 2;
#ifdef _DEBUG
	bool m_bALPHACLIP : 1;
#endif	// _DEBUG
public:
	void SetALPHACLIP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nALPHACLIP = i;
#ifdef _DEBUG
		m_bALPHACLIP = true;
#endif	// _DEBUG
	}

	depthwrite_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nALPHACLIP = 0;
#ifdef _DEBUG
		m_bALPHACLIP = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bALPHACLIP );
		return ( 1 * m_nALPHACLIP ) + 0;
	}
};

#define shaderDynamicTest_depthwrite_ps20b psh_forgot_to_set_dynamic_ALPHACLIP


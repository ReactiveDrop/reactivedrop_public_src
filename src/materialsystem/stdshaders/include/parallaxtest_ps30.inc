#pragma once
#include "shaderlib/cshader.h"
class parallaxtest_ps30_Static_Index
{
public:
	parallaxtest_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_parallaxtest_ps30 1


class parallaxtest_ps30_Dynamic_Index
{
public:
	parallaxtest_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_parallaxtest_ps30 1


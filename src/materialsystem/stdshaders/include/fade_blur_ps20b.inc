#pragma once
#include "shaderlib/cshader.h"
class fade_blur_ps20b_Static_Index
{
public:
	fade_blur_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_fade_blur_ps20b 1


class fade_blur_ps20b_Dynamic_Index
{
public:
	fade_blur_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_fade_blur_ps20b 1


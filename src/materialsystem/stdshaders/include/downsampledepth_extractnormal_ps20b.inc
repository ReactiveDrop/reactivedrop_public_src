#pragma once
#include "shaderlib/cshader.h"
class downsampledepth_extractnormal_ps20b_Static_Index
{
public:
	downsampledepth_extractnormal_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_downsampledepth_extractnormal_ps20b 1


class downsampledepth_extractnormal_ps20b_Dynamic_Index
{
public:
	downsampledepth_extractnormal_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_downsampledepth_extractnormal_ps20b 1


#pragma once
#include "shaderlib/cshader.h"
class compositedeferredshadow_ps20b_Static_Index
{
public:
	compositedeferredshadow_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_compositedeferredshadow_ps20b 1


class compositedeferredshadow_ps20b_Dynamic_Index
{
public:
	compositedeferredshadow_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_compositedeferredshadow_ps20b 1


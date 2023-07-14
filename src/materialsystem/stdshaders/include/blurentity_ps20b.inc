#pragma once
#include "shaderlib/cshader.h"
class blurentity_ps20b_Static_Index
{
public:
	blurentity_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_blurentity_ps20b 1


class blurentity_ps20b_Dynamic_Index
{
public:
	blurentity_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_blurentity_ps20b 1


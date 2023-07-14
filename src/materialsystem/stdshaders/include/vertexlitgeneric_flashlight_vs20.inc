#pragma once
#include "shaderlib/cshader.h"
class vertexlitgeneric_flashlight_vs20_Static_Index
{
public:
	vertexlitgeneric_flashlight_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_vertexlitgeneric_flashlight_vs20 1


class vertexlitgeneric_flashlight_vs20_Dynamic_Index
{
public:
	vertexlitgeneric_flashlight_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_vertexlitgeneric_flashlight_vs20 1


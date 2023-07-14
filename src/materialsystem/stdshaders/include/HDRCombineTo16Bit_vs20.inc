// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class HDRCombineTo16Bit_vs20_Static_Index
{
public:
	HDRCombineTo16Bit_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_HDRCombineTo16Bit_vs20 1


class HDRCombineTo16Bit_vs20_Dynamic_Index
{
public:
	HDRCombineTo16Bit_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_HDRCombineTo16Bit_vs20 1


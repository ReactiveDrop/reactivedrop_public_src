#pragma once

struct RDShieldDome_DX9_Vars_t
{
	RDShieldDome_DX9_Vars_t() { memset( this, 0xFF, sizeof( RDShieldDome_DX9_Vars_t ) ); }

	int m_nBaseTexture;
	int m_nBaseTextureFrame;
	int m_nBaseTextureTransform;
};

void InitParamsRDShieldDome_DX9( CBaseVSShader *pShader, IMaterialVar **params, const char *pMaterialName, const RDShieldDome_DX9_Vars_t &info );
void InitRDShieldDome_DX9( CBaseVSShader *pShader, IMaterialVar **params, const RDShieldDome_DX9_Vars_t &info );
void DrawRDShieldDome_DX9( CBaseVSShader *pShader, IMaterialVar **params,
	IShaderDynamicAPI *pShaderAPI, IShaderShadow *pShaderShadow,
	const RDShieldDome_DX9_Vars_t &info, CBasePerMaterialContextData **pContextDataPtr,
	VertexCompressionType_t vertexCompression );

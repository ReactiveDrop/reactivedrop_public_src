#include "BaseVSShader.h"
#include "shielddome_dx9_helper.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

static RDShieldDome_DX9_Vars_t s_info;

DEFINE_FALLBACK_SHADER( RDShieldDome, RDShieldDome_DX9 )
BEGIN_VS_SHADER( RDShieldDome_DX9, "shield bubble proximity effect" )
	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	void SetupVars( RDShieldDome_DX9_Vars_t &info )
	{
		info.m_nBaseTexture = BASETEXTURE;
		info.m_nBaseTextureFrame = FRAME;
		info.m_nBaseTextureTransform = BASETEXTURETRANSFORM;
	}

	SHADER_INIT_PARAMS()
	{
		SetupVars( s_info );
		InitParamsRDShieldDome_DX9( this, params, pMaterialName, s_info );
	}

	SHADER_FALLBACK
	{
		// Shader isn't compiled for 2.0 (it is for 2.0b and 3.0)
		if ( !g_pHardwareConfig->SupportsPixelShaders_2_b() && !g_pHardwareConfig->HasFastVertexTextures() )
			return "Wireframe";
		return 0;
	}

	SHADER_INIT
	{
		SetupVars( s_info );
		InitRDShieldDome_DX9( this, params, s_info );
	}

	SHADER_DRAW
	{
		// assume init will have been called first so s_info is initialized.
		DrawRDShieldDome_DX9( this, params, pShaderAPI, pShaderShadow, s_info, pContextDataPtr, vertexCompression );
	}
END_SHADER

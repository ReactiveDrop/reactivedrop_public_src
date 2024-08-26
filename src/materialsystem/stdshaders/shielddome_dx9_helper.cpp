#include "BaseVSShader.h"
#include "shielddome_dx9_helper.h"
#include "shielddome_ps20b.inc"
#include "shielddome_ps30.inc"
#include "shielddome_vs20.inc"
#include "shielddome_vs30.inc"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

void InitParamsRDShieldDome_DX9( CBaseVSShader *pShader, IMaterialVar **params, const char *pMaterialName, const RDShieldDome_DX9_Vars_t &info )
{
	SET_FLAGS( MATERIAL_VAR_MODEL ); // this is not a brush shader
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT ); // we want lighting information
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING ); // we can hardware skin
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES ); // we need bumpmap data
}

void InitRDShieldDome_DX9( CBaseVSShader *pShader, IMaterialVar **params, const RDShieldDome_DX9_Vars_t &info )
{
	if ( info.m_nBaseTexture != -1 && params[info.m_nBaseTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture );
	}
}

void DrawRDShieldDome_DX9( CBaseVSShader *pShader, IMaterialVar **params,
	IShaderDynamicAPI *pShaderAPI, IShaderShadow *pShaderShadow,
	const RDShieldDome_DX9_Vars_t &info, CBasePerMaterialContextData **pContextDataPtr,
	VertexCompressionType_t vertexCompression )
{
	SHADOW_STATE
	{
		pShader->SetInitialShadowState();
		pShader->SetDefaultBlendingShadowState();
		pShader->DefaultFog();

		pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED, 1, NULL, 0 );

		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true ); // basetexture
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );
		pShaderShadow->EnableTexture( SHADER_SAMPLER1, true ); // depth buffer
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER1, false );
		pShaderShadow->EnableSRGBWrite( true );

		if ( g_pHardwareConfig->HasFastVertexTextures() )
		{
			DECLARE_STATIC_VERTEX_SHADER( shielddome_vs30 );
			SET_STATIC_VERTEX_SHADER( shielddome_vs30 );

			DECLARE_STATIC_PIXEL_SHADER( shielddome_ps30 );
			SET_STATIC_PIXEL_SHADER( shielddome_ps30 );
		}
		else
		{
			DECLARE_STATIC_VERTEX_SHADER( shielddome_vs20 );
			SET_STATIC_VERTEX_SHADER( shielddome_vs20 );

			// if we don't have 2.0b we should be falling back to wireframe before we hit this function
			Assert( g_pHardwareConfig->SupportsPixelShaders_2_b() );

			DECLARE_STATIC_PIXEL_SHADER( shielddome_ps20b );
			SET_STATIC_PIXEL_SHADER( shielddome_ps20b );
		}

		pShader->PI_BeginCommandBuffer();
		pShader->PI_SetPixelShaderAmbientLightCube( PSREG_AMBIENT_CUBE );
		pShader->PI_SetPixelShaderLocalLighting( PSREG_LIGHT_INFO_ARRAY );
		pShader->PI_EndCommandBuffer();
	}

	DYNAMIC_STATE
	{
		pShaderAPI->SetPixelShaderFogParams( PSREG_FOG_PARAMS );

		float vEyePos_SpecExponent[4];
		pShaderAPI->GetWorldSpaceCameraPosition( vEyePos_SpecExponent );
		vEyePos_SpecExponent[3] = 0.0f;
		pShaderAPI->SetPixelShaderConstant( PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1 );

		float vEyeDir[4];
		pShaderAPI->GetWorldSpaceCameraDirection( vEyeDir );
		pShaderAPI->SetPixelShaderConstant( PSREG_SHADER_CONTROLS_2, vEyeDir, 1 );

		pShader->SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nBaseTextureTransform );

		pShader->BindTexture( SHADER_SAMPLER0, info.m_nBaseTexture, info.m_nBaseTextureFrame );
		pShaderAPI->BindStandardTexture( SHADER_SAMPLER1, TEXTURE_FRAME_BUFFER_FULL_DEPTH );

		if ( g_pHardwareConfig->HasFastVertexTextures() )
		{
			DECLARE_DYNAMIC_VERTEX_SHADER( shielddome_vs30 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, ( int )vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
			SET_DYNAMIC_VERTEX_SHADER( shielddome_vs30 );

			DECLARE_DYNAMIC_PIXEL_SHADER( shielddome_ps30 );
			SET_DYNAMIC_PIXEL_SHADER( shielddome_ps30 );
		}
		else
		{
			DECLARE_DYNAMIC_VERTEX_SHADER( shielddome_vs20 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, ( int )vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
			SET_DYNAMIC_VERTEX_SHADER( shielddome_vs20 );

			Assert( g_pHardwareConfig->SupportsPixelShaders_2_b() );

			DECLARE_DYNAMIC_PIXEL_SHADER( shielddome_ps20b );
			SET_DYNAMIC_PIXEL_SHADER( shielddome_ps20b );
		}
	}

	pShader->Draw();
}

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
	bool bHasFlashlight = pShader->UsingFlashlight( params );

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
		pShaderShadow->EnableTexture( SHADER_SAMPLER2, true ); // random vectors
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER2, false );
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true ); // normalize
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER3, false );
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

		float flTime = pShaderAPI->CurrentTime();
		const float *vecWiggleParams = params[info.m_nWiggleParams]->GetVecValue();
		float vWiggle[4];
		vWiggle[0] = cosf( flTime * vecWiggleParams[1] ) * vecWiggleParams[0];
		vWiggle[1] = cosf( flTime * vecWiggleParams[2] ) * vecWiggleParams[0];
		vWiggle[2] = cosf( flTime * vecWiggleParams[3] ) * vecWiggleParams[0];
		vWiggle[3] = flTime * 1337.5;
		pShaderAPI->SetPixelShaderConstant( PSREG_SHADER_CONTROLS_2, vWiggle, 1 );

		pShader->SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nBaseTextureTransform );

		pShader->BindTexture( SHADER_SAMPLER0, info.m_nBaseTexture, info.m_nBaseTextureFrame );
		pShaderAPI->BindStandardTexture( SHADER_SAMPLER1, TEXTURE_FRAME_BUFFER_FULL_DEPTH );
		pShaderAPI->BindStandardTexture( SHADER_SAMPLER2, TEXTURE_SSAO_NOISE_2D );
		pShaderAPI->BindStandardTexture( SHADER_SAMPLER3, TEXTURE_NORMALIZATION_CUBEMAP_SIGNED );

		LightState_t lightState = { 0, false, false };
		if ( !bHasFlashlight )
		{
			pShaderAPI->GetDX9LightState( &lightState );
		}

		if ( g_pHardwareConfig->HasFastVertexTextures() )
		{
			DECLARE_DYNAMIC_VERTEX_SHADER( shielddome_vs30 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, ( int )vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, pShaderAPI->GetCurrentNumBones() > 0 );
			SET_DYNAMIC_VERTEX_SHADER( shielddome_vs30 );

			DECLARE_DYNAMIC_PIXEL_SHADER( shielddome_ps30 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( NUM_LIGHTS, lightState.m_nNumLights );
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
			SET_DYNAMIC_PIXEL_SHADER_COMBO( NUM_LIGHTS, lightState.m_nNumLights );
			SET_DYNAMIC_PIXEL_SHADER( shielddome_ps20b );
		}
	}

	pShader->Draw();
}

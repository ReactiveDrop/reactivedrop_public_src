//====== Copyright © 1996-2008, Valve Corporation, All rights reserved. =====//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "basevsshader.h"
#include "character_dx9_helper.h"

#include "shaderlib/commandbuilder.h"
#include "convar.h"

#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_fullbright( "mat_fullbright", "0", FCVAR_CHEAT );
static ConVar r_lightwarpidentity( "r_lightwarpidentity", "0", FCVAR_CHEAT );
static ConVar r_rimlight( "r_rimlight", "1", FCVAR_CHEAT );
static ConVar mat_displacementmap( "mat_displacementmap", "1", FCVAR_CHEAT );
static ConVar mat_force_vertexfog( "mat_force_vertexfog", "0", FCVAR_DEVELOPMENTONLY );


//-----------------------------------------------------------------------------
// Initialize shader parameters
//-----------------------------------------------------------------------------
void InitParamsCharacter_DX9( CBaseVSShader *pShader, IMaterialVar **params, const char *pMaterialName, Character_DX9_Vars_t &info )
{
	Assert( g_pHardwareConfig->SupportsPixelShaders_2_b() ); // if we don't have pixel shader 2.0b or better, we should have fallen back to VertexLitGeneric.

	InitIntParam( info.m_nShaderSrgbRead360, params, 0 );

	InitFloatParam( info.m_nAlphaTestReference, params, 0.0f );
	InitIntParam( info.m_nVertexAlphaTest, params, 0 );

	InitIntParam( info.m_nFlashlightNoLambert, params, 0 );

	if ( info.m_nDetailTint != -1 && !params[info.m_nDetailTint]->IsDefined() )
	{
		params[info.m_nDetailTint]->SetVecValue( 1.0f, 1.0f, 1.0f );
	}

	if ( info.m_nEnvmapTint != -1 && !params[info.m_nEnvmapTint]->IsDefined() )
	{
		params[info.m_nEnvmapTint]->SetVecValue( 1.0f, 1.0f, 1.0f );
	}

	InitIntParam( info.m_nEnvmapFrame, params, 0 );
	InitIntParam( info.m_nBumpFrame, params, 0 );
	InitFloatParam( info.m_nDetailTextureBlendFactor, params, 1.0 );
	InitIntParam( info.m_nReceiveFlashlight, params, 0 );

	InitFloatParam( info.m_nDetailScale, params, 4.0f );

	if ( ( info.m_nSelfIllumTint != -1 ) && ( !params[info.m_nSelfIllumTint]->IsDefined() ) )
	{
		params[info.m_nSelfIllumTint]->SetVecValue( 1.0f, 1.0f, 1.0f );
	}

	// Set the selfillummask flag2
	if ( ( info.m_nSelfIllumMask != -1 ) && ( params[info.m_nSelfIllumMask]->IsDefined() ) )
	{
		SET_FLAGS2( MATERIAL_VAR2_SELFILLUMMASK );
	}

	InitFloatParam( info.m_nSelfIllumMaskScale, params, 1.0f );

	// Override vertex fog via the global setting if it isn't enabled/disabled in the material file.
	if ( !IS_FLAG_DEFINED( MATERIAL_VAR_VERTEXFOG ) && mat_force_vertexfog.GetBool() )
	{
		SET_FLAGS( MATERIAL_VAR_VERTEXFOG );
	}

	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	Assert( info.m_nFlashlightTexture >= 0 );

	params[FLASHLIGHTTEXTURE]->SetStringValue( GetFlashlightTextureFilename() );

	// Write over $basetexture with $info.m_nBumpmap if we are going to be using diffuse normal mapping.
	if ( info.m_nAlbedo != -1 && g_pConfig->UseBumpmapping() && info.m_nBumpmap != -1 && params[info.m_nBumpmap]->IsDefined() && params[info.m_nAlbedo]->IsDefined() &&
		params[info.m_nBaseTexture]->IsDefined() )
	{
		params[info.m_nBaseTexture]->SetStringValue( params[info.m_nAlbedo]->GetStringValue() );
	}

	// This shader can be used with hw skinning
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

	// No texture means no env mask in base alpha
	if ( !params[info.m_nBaseTexture]->IsDefined() )
	{
		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	// If in decal mode, no debug override...
	if ( IS_FLAG_SET( MATERIAL_VAR_DECAL ) )
	{
		SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
	}

	// We always specify we're using user data, therefore we always need tangent spaces
	SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );

	if ( ( info.m_nSelfIllumFresnelMinMaxExp != -1 ) && ( !params[info.m_nSelfIllumFresnelMinMaxExp]->IsDefined() ) )
	{
		params[info.m_nSelfIllumFresnelMinMaxExp]->SetVecValue( 0.0f, 1.0f, 1.0f );
	}

	InitFloatParam( info.m_nEnvmapFresnel, params, 0.0f );
	InitFloatParam( info.m_nAmbientOcclusion, params, 0.0f );
	InitFloatParam( info.m_nDisplacementWrinkleMap, params, 0.0f );

	InitIntParam( info.m_nSelfIllumFresnel, params, 0 );
	InitIntParam( info.m_nBaseMapAlphaPhongMask, params, 0 );
	InitIntParam( info.m_nBaseMapLuminancePhongMask, params, 0 );
	InitIntParam( info.m_nShaderSrgbRead360, params, 0 );
	InitIntParam( info.m_nAllowDiffuseModulation, params, 1 );

	InitIntParam( info.m_nPhongDisableHalfLambert, params, 0 );
}


//-----------------------------------------------------------------------------
// Initialize shader
//-----------------------------------------------------------------------------

void InitCharacter_DX9( CBaseVSShader *pShader, IMaterialVar **params, Character_DX9_Vars_t &info )
{
	Assert( info.m_nFlashlightTexture >= 0 );
	pShader->LoadTexture( info.m_nFlashlightTexture );

	bool bIsBaseTextureTranslucent = false;
	if ( params[info.m_nBaseTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture );

		if ( params[info.m_nBaseTexture]->GetTextureValue()->IsTranslucent() )
		{
			bIsBaseTextureTranslucent = true;
		}

		if ( ( info.m_nWrinkle != -1 ) && ( info.m_nStretch != -1 ) &&
			params[info.m_nWrinkle]->IsDefined() && params[info.m_nStretch]->IsDefined() )
		{
			pShader->LoadTexture( info.m_nWrinkle );
			pShader->LoadTexture( info.m_nStretch );
		}
	}

	bool bHasSelfIllumMask = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && IS_FLAG2_SET( MATERIAL_VAR2_SELFILLUMMASK );

	// No alpha channel in any of the textures? No self illum or envmapmask
	if ( !bIsBaseTextureTranslucent )
	{
		bool bHasSelfIllumFresnel = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && ( info.m_nSelfIllumFresnel != -1 ) && ( params[info.m_nSelfIllumFresnel]->GetIntValue() != 0 );

		// Can still be self illum with no base alpha if using one of these alternate modes
		if ( !bHasSelfIllumFresnel && !bHasSelfIllumMask )
		{
			CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
		}

		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	if ( ( info.m_nPhongExponentTexture != -1 ) && params[info.m_nPhongExponentTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nPhongExponentTexture );
	}

	if ( ( info.m_nDiffuseWarpTexture != -1 ) && params[info.m_nDiffuseWarpTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nDiffuseWarpTexture );
	}

	if ( ( info.m_nPhongWarpTexture != -1 ) && params[info.m_nPhongWarpTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nPhongWarpTexture );
	}

	if ( info.m_nDetail != -1 && params[info.m_nDetail]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nDetail );
	}

	if ( g_pConfig->UseBumpmapping() )
	{
		if ( ( info.m_nBumpmap != -1 ) && params[info.m_nBumpmap]->IsDefined() )
		{
			pShader->LoadBumpMap( info.m_nBumpmap );
			SET_FLAGS2( MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL );

			if ( ( info.m_nNormalWrinkle != -1 ) && ( info.m_nNormalStretch != -1 ) &&
				params[info.m_nNormalWrinkle]->IsDefined() && params[info.m_nNormalStretch]->IsDefined() )
			{
				pShader->LoadTexture( info.m_nNormalWrinkle );
				pShader->LoadTexture( info.m_nNormalStretch );
			}
		}
	}

	if ( params[info.m_nEnvmap]->IsDefined() )
	{
		pShader->LoadCubeMap( info.m_nEnvmap );
	}

	if ( bHasSelfIllumMask )
	{
		pShader->LoadTexture( info.m_nSelfIllumMask );
	}

	if ( params[info.m_nDisplacementMap]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nDisplacementMap );
	}
}

class CCharacter_DX9_Context : public CBasePerMaterialContextData
{
public:
	CCommandBufferBuilder< CFixedCommandStorageBuffer< 800 > > m_SemiStaticCmdsOut;
};

void DrawCharacter_DX9( CBaseVSShader *pShader, IMaterialVar **params, IShaderDynamicAPI *pShaderAPI,
	IShaderShadow *pShaderShadow, Character_DX9_Vars_t &info, VertexCompressionType_t vertexCompression,
	CBasePerMaterialContextData **pContextDataPtr )
{
	CCharacter_DX9_Context *pContextData = reinterpret_cast< CCharacter_DX9_Context * > ( *pContextDataPtr );

	bool bHasFlashlight = pShader->UsingFlashlight( params );
	bool bIsDecal = IS_FLAG_SET( MATERIAL_VAR_DECAL );
	bool bIsAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0;
	BlendType_t nBlendType = pShader->EvaluateBlendRequirements( info.m_nBaseTexture, true );
	bool bFullyOpaque = ( nBlendType != BT_BLENDADD ) && ( nBlendType != BT_BLEND ) && !bIsAlphaTested && !bHasFlashlight; //dest alpha is free for special use
	bool bHasDisplacement = ( info.m_nDisplacementMap != -1 ) && params[info.m_nDisplacementMap]->IsTexture();
	bool bHasDisplacementWrinkles = ( info.m_nDisplacementWrinkleMap != -1 ) && params[info.m_nDisplacementWrinkleMap]->GetIntValue();

	if ( pShader->IsSnapshotting() )
	{
		int nDetailBlendMode = ( info.m_nDetailTextureCombineMode == -1 ) ? 0 : params[info.m_nDetailTextureCombineMode]->GetIntValue();
		bool bHasVertexColor = IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR );
		bool bHasVertexAlpha = IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA );
		bool bHasBaseTextureWrinkle = ( info.m_nBaseTexture != -1 ) && params[info.m_nBaseTexture]->IsTexture() &&
			( info.m_nWrinkle != -1 ) && params[info.m_nWrinkle]->IsTexture() &&
			( info.m_nStretch != -1 ) && params[info.m_nStretch]->IsTexture();
		bool bHasBumpWrinkle = ( info.m_nBumpmap != -1 ) && params[info.m_nBumpmap]->IsTexture() &&
			( info.m_nNormalWrinkle != -1 ) && params[info.m_nNormalWrinkle]->IsTexture() &&
			( info.m_nNormalStretch != -1 ) && params[info.m_nNormalStretch]->IsTexture();
		bool bHasRimLight = r_rimlight.GetBool() && ( info.m_nRimLight != -1 ) && ( params[info.m_nRimLight]->GetIntValue() != 0 );

		// look at color and alphamod stuff.
		// Unlit generic never uses the flashlight
		bool bHasEnvmap = !bHasFlashlight && params[info.m_nEnvmap]->IsTexture();

		// Alpha test: FIXME: shouldn't this be handled in CBaseVSShader::SetInitialShadowState
		pShaderShadow->EnableAlphaTest( bIsAlphaTested );

		if ( info.m_nAlphaTestReference != -1 && params[info.m_nAlphaTestReference]->GetFloatValue() > 0.0f )
		{
			pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[info.m_nAlphaTestReference]->GetFloatValue() );
		}

		// Based upon vendor and device dependent formats
		int nShadowFilterMode = bHasFlashlight ? g_pHardwareConfig->GetShadowFilterMode() : 0;
		if ( bHasFlashlight )
		{
			if ( params[info.m_nBaseTexture]->IsTexture() )
			{
				pShader->SetAdditiveBlendingShadowState( info.m_nBaseTexture, true );
			}

			if ( bIsAlphaTested )
			{
				// disable alpha test and use the zfunc zequals since alpha isn't guaranteed to 
				// be the same on both the regular pass and the flashlight pass.
				pShaderShadow->EnableAlphaTest( false );
				pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );
			}
			pShaderShadow->EnableBlending( true );
			pShaderShadow->EnableDepthWrites( false );

			// Be sure not to write to dest alpha
			pShaderShadow->EnableAlphaWrites( false );
		}

		if ( !bHasFlashlight ) // not flashlight pass
		{
			if ( params[info.m_nBaseTexture]->IsTexture() )
			{
				pShader->SetDefaultBlendingShadowState( info.m_nBaseTexture, true );
			}

			if ( bHasEnvmap )
			{
				pShaderShadow->EnableTexture( SHADER_SAMPLER8, true );	// Cubic environment map
				if ( g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE )
				{
					pShaderShadow->EnableSRGBRead( SHADER_SAMPLER8, true );
				}
			}
		}

		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
		int userDataSize = 0;

		// Always enable...will bind white if nothing specified...
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );		// Base (albedo) map
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

		if ( bHasBaseTextureWrinkle )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER9, true );	// Base (albedo) compression map
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER9, true );

			pShaderShadow->EnableTexture( SHADER_SAMPLER10, true );	// Base (albedo) stretch map
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER10, true );
		}

		if ( ( info.m_nDiffuseWarpTexture != -1 ) && params[info.m_nDiffuseWarpTexture]->IsTexture() )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER2, true );	// Diffuse warp texture
		}

		if ( ( info.m_nPhongWarpTexture != -1 ) && params[info.m_nPhongWarpTexture]->IsTexture() )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );	// Specular warp texture
		}

		// Specular exponent map or dummy
		pShaderShadow->EnableTexture( SHADER_SAMPLER7, true );	// Specular exponent map

		if ( bHasFlashlight )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER4, true );	// Shadow depth map
			pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER4 );
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER4, false );
			pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );	// Noise map
			pShaderShadow->EnableTexture( SHADER_SAMPLER6, true );	// Flashlight cookie
		}

		// Always enable, since flat normal will be bound
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );		// Normal map
		userDataSize = 4; // tangent S
		pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );		// Normalizing cube map

		if ( bHasBumpWrinkle || bHasBaseTextureWrinkle )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER11, true );	// Normal compression map
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER11, false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER12, true );	// Normal stretch map
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER12, false );
		}

		if ( ( info.m_nDetail != -1 ) && params[info.m_nDetail]->IsTexture() )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER13, true );
			if ( nDetailBlendMode != 0 ) //Not Mod2X
			{
				pShaderShadow->EnableSRGBRead( SHADER_SAMPLER13, true );
			}
		}

		if ( IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER14, true );
		}

		if ( bHasVertexColor || bHasVertexAlpha )
		{
			flags |= VERTEX_COLOR;
		}

		// Always enable ambient occlusion sampler on PC on DX10 parts
		if ( IsPC() && g_pHardwareConfig->HasFastVertexTextures() )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER15, true );
		}

		if ( bHasDisplacement && IsPC() && g_pHardwareConfig->HasFastVertexTextures() )
		{
			pShaderShadow->EnableVertexTexture( SHADER_VERTEXTEXTURE_SAMPLER2, true );
		}

		pShaderShadow->EnableSRGBWrite( true );

		// texcoord0 : base texcoord, texcoord2 : decal hw morph delta
		int pTexCoordDim[3] = { 2, 0, 3 };
		int nTexCoordCount = 1;

		// Special morphed decal information 
		if ( bIsDecal && g_pHardwareConfig->HasFastVertexTextures() )
		{
			nTexCoordCount = 3;
		}

		// This shader supports compressed vertices, so OR in that flag:
		flags |= VERTEX_FORMAT_COMPRESSED;

		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, pTexCoordDim, userDataSize );

		bool bWorldNormal = ( ENABLE_FIXED_LIGHTING_OUTPUTNORMAL_AND_DEPTH == ( IS_FLAG2_SET( MATERIAL_VAR2_USE_GBUFFER0 ) + 2 * IS_FLAG2_SET( MATERIAL_VAR2_USE_GBUFFER1 ) ) );

		// This is to allow phong materials to disable half lambert. Half lambert has always been forced on in phong,
		// so the only safe way to allow artists to disable half lambert is to create this param that disables the
		// default behavior of forcing half lambert on.
		bool bPhongHalfLambert = IS_PARAM_DEFINED( info.m_nPhongDisableHalfLambert ) ? ( params[info.m_nPhongDisableHalfLambert]->GetIntValue() == 0 ) : true;

		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			DECLARE_STATIC_VERTEX_SHADER( character_vs20 );
			SET_STATIC_VERTEX_SHADER_COMBO( WORLD_NORMAL, 0 );
			SET_STATIC_VERTEX_SHADER( character_vs20 );

			// Assume we're only going to get in here if we support 2b
			DECLARE_STATIC_PIXEL_SHADER( character_ps20b );
			SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHT, bHasFlashlight );
			SET_STATIC_PIXEL_SHADER_COMBO( SELFILLUM, IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && !bHasFlashlight );
			SET_STATIC_PIXEL_SHADER_COMBO( SELFILLUMFRESNEL, IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && ( info.m_nSelfIllumFresnel != -1 ) && ( params[info.m_nSelfIllumFresnel]->GetIntValue() != 0 ) && !bHasFlashlight );
			SET_STATIC_PIXEL_SHADER_COMBO( LIGHTWARPTEXTURE, ( info.m_nDiffuseWarpTexture != -1 ) && params[info.m_nDiffuseWarpTexture]->IsTexture() );
			SET_STATIC_PIXEL_SHADER_COMBO( PHONGWARPTEXTURE, ( info.m_nPhongWarpTexture != -1 ) && params[info.m_nPhongWarpTexture]->IsTexture() );
			SET_STATIC_PIXEL_SHADER_COMBO( WRINKLEMAP, bHasBaseTextureWrinkle );
			SET_STATIC_PIXEL_SHADER_COMBO( DETAILTEXTURE, ( info.m_nDetail != -1 ) && params[info.m_nDetail]->IsTexture() );
			SET_STATIC_PIXEL_SHADER_COMBO( DETAIL_BLEND_MODE, nDetailBlendMode );
			SET_STATIC_PIXEL_SHADER_COMBO( RIMLIGHT, bHasRimLight );
			SET_STATIC_PIXEL_SHADER_COMBO( CUBEMAP, bHasEnvmap );
			SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
			SET_STATIC_PIXEL_SHADER_COMBO( SHADER_SRGB_READ, 0 );
			SET_STATIC_PIXEL_SHADER_COMBO( WORLD_NORMAL, 0 );
			SET_STATIC_PIXEL_SHADER_COMBO( PHONG_HALFLAMBERT, bPhongHalfLambert );
			SET_STATIC_PIXEL_SHADER( character_ps20b );
		}
		else
		{
			// The vertex shader uses the vertex id stream
			SET_FLAGS2( MATERIAL_VAR2_USES_VERTEXID );
			SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_TESSELLATION );

			DECLARE_STATIC_VERTEX_SHADER( character_vs30 );
			SET_STATIC_VERTEX_SHADER_COMBO( WORLD_NORMAL, bWorldNormal );
			SET_STATIC_VERTEX_SHADER_COMBO( DECAL, bIsDecal );
			SET_STATIC_VERTEX_SHADER( character_vs30 );

			DECLARE_STATIC_PIXEL_SHADER( character_ps30 );
			SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHT, bHasFlashlight );
			SET_STATIC_PIXEL_SHADER_COMBO( SELFILLUM, IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && !bHasFlashlight );
			SET_STATIC_PIXEL_SHADER_COMBO( SELFILLUMFRESNEL, IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && ( info.m_nSelfIllumFresnel != -1 ) && ( params[info.m_nSelfIllumFresnel]->GetIntValue() != 0 ) && !bHasFlashlight );
			SET_STATIC_PIXEL_SHADER_COMBO( LIGHTWARPTEXTURE, ( info.m_nDiffuseWarpTexture != -1 ) && params[info.m_nDiffuseWarpTexture]->IsTexture() );
			SET_STATIC_PIXEL_SHADER_COMBO( PHONGWARPTEXTURE, ( info.m_nPhongWarpTexture != -1 ) && params[info.m_nPhongWarpTexture]->IsTexture() );
			SET_STATIC_PIXEL_SHADER_COMBO( WRINKLEMAP, bHasBaseTextureWrinkle );
			SET_STATIC_PIXEL_SHADER_COMBO( DETAILTEXTURE, ( info.m_nDetail != -1 ) && params[info.m_nDetail]->IsTexture() );
			SET_STATIC_PIXEL_SHADER_COMBO( DETAIL_BLEND_MODE, nDetailBlendMode );
			SET_STATIC_PIXEL_SHADER_COMBO( RIMLIGHT, bHasRimLight );
			SET_STATIC_PIXEL_SHADER_COMBO( CUBEMAP, bHasEnvmap );
			SET_STATIC_PIXEL_SHADER_COMBO( FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode );
			SET_STATIC_PIXEL_SHADER_COMBO( SHADER_SRGB_READ, 0 );
			SET_STATIC_PIXEL_SHADER_COMBO( WORLD_NORMAL, bWorldNormal );
			SET_STATIC_PIXEL_SHADER_COMBO( PHONG_HALFLAMBERT, bPhongHalfLambert );
			SET_STATIC_PIXEL_SHADER( character_ps30 );
		}

		if ( bHasFlashlight )
		{
			pShader->FogToBlack();
		}
		else
		{
			pShader->DefaultFog();
		}

		// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
		pShaderShadow->EnableAlphaWrites( bFullyOpaque );

		pShader->PI_BeginCommandBuffer();
		pShader->PI_SetPixelShaderAmbientLightCube( PSREG_AMBIENT_CUBE );
		pShader->PI_SetPixelShaderLocalLighting( PSREG_LIGHT_INFO_ARRAY );
		pShader->PI_SetVertexShaderAmbientLightCube();
		// material can opt out of per-instance modulation via $nodiffusemodulation
		bool bAllowDiffuseModulation = ( info.m_nAllowDiffuseModulation == -1 ) ? true : ( params[info.m_nAllowDiffuseModulation]->GetIntValue() != 0 );
		if ( bAllowDiffuseModulation )
		{
			pShader->PI_SetModulationPixelShaderDynamicState_LinearColorSpace( 1 );
		}
		else
		{
			pShader->PI_SetModulationPixelShaderDynamicState_Identity( 1 );
		}
		pShader->PI_EndCommandBuffer();
	}
	else // not snapshotting -- begin dynamic state
	{
		// Deal with semisatic
		if ( ( !pContextData ) || ( pContextData->m_bMaterialVarsChanged ) )
		{
			if ( !pContextData )								// make sure allocated
			{
				pContextData = new CCharacter_DX9_Context;
				*pContextDataPtr = pContextData;
			}

			pContextData->m_SemiStaticCmdsOut.Reset();
			pContextData->m_bMaterialVarsChanged = false;

			bool bHasBump = ( info.m_nBumpmap != -1 ) && params[info.m_nBumpmap]->IsTexture();
			bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
			bool bHasSelfIllumMask = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && ( info.m_nSelfIllumMask != -1 ) && params[info.m_nSelfIllumMask]->IsTexture();
			float fBlendFactor = ( info.m_nDetailTextureBlendFactor == -1 ) ? 1 : params[info.m_nDetailTextureBlendFactor]->GetFloatValue();
			bool bHasSpecularExponentTexture = ( info.m_nPhongExponentTexture != -1 ) && params[info.m_nPhongExponentTexture]->IsTexture();
			bool bHasPhongTintMap = bHasSpecularExponentTexture && ( info.m_nPhongAlbedoTint != -1 ) && ( params[info.m_nPhongAlbedoTint]->GetIntValue() != 0 );
			bool bHasNormalMapAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
			bool bHasRimLight = r_rimlight.GetBool() && ( info.m_nRimLight != -1 ) && ( params[info.m_nRimLight]->GetIntValue() != 0 );
			bool bHasRimMaskMap = bHasSpecularExponentTexture && bHasRimLight &&
				( info.m_nRimMask != -1 ) && ( params[info.m_nRimMask]->GetIntValue() != 0 );
			bool bHasBaseTextureWrinkle = ( info.m_nBaseTexture != -1 ) && params[info.m_nBaseTexture]->IsTexture() &&
				( info.m_nWrinkle != -1 ) && params[info.m_nWrinkle]->IsTexture() &&
				( info.m_nStretch != -1 ) && params[info.m_nStretch]->IsTexture();
			bool bHasBumpWrinkle = bHasBump &&
				( info.m_nNormalWrinkle != -1 ) && params[info.m_nNormalWrinkle]->IsTexture() &&
				( info.m_nNormalStretch != -1 ) && params[info.m_nNormalStretch]->IsTexture();

			if ( ( info.m_nBaseTexture != -1 ) && params[info.m_nBaseTexture]->IsTexture() )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER0, info.m_nBaseTexture, info.m_nBaseTextureFrame );
			}
			else
			{
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER0, TEXTURE_WHITE );
			}

			if ( ( info.m_nBaseTexture != -1 ) && params[info.m_nBaseTexture]->IsTexture() &&
				( info.m_nWrinkle != -1 ) && params[info.m_nWrinkle]->IsTexture() &&
				( info.m_nStretch != -1 ) && params[info.m_nStretch]->IsTexture() )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER9, info.m_nWrinkle, info.m_nBaseTextureFrame );
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER10, info.m_nStretch, info.m_nBaseTextureFrame );
			}

			if ( ( info.m_nDiffuseWarpTexture != -1 ) && params[info.m_nDiffuseWarpTexture]->IsTexture() )
			{
				if ( r_lightwarpidentity.GetBool() )
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER2, TEXTURE_IDENTITY_LIGHTWARP );
				}
				else
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER2, info.m_nDiffuseWarpTexture );
				}
			}

			if ( ( info.m_nPhongWarpTexture != -1 ) && params[info.m_nPhongWarpTexture]->IsTexture() )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER1, info.m_nPhongWarpTexture );
			}

			if ( bHasSpecularExponentTexture )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER7, info.m_nPhongExponentTexture );
			}
			else
			{
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER7, TEXTURE_WHITE );
			}

			if ( !g_pConfig->m_bFastNoBump )
			{
				if ( bHasBump )
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER3, info.m_nBumpmap, info.m_nBumpFrame );
				else
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER3, TEXTURE_NORMALMAP_FLAT );

				if ( bHasBumpWrinkle )
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER11, info.m_nNormalWrinkle, info.m_nBumpFrame );
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER12, info.m_nNormalStretch, info.m_nBumpFrame );
				}
				else if ( bHasBaseTextureWrinkle )
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER11, info.m_nBumpmap, info.m_nBumpFrame );
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER12, info.m_nBumpmap, info.m_nBumpFrame );
				}
			}
			else // Just flat bump maps
			{
				if ( bHasBump )
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER3, TEXTURE_NORMALMAP_FLAT );
				}

				if ( bHasBaseTextureWrinkle || bHasBumpWrinkle )
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER11, TEXTURE_NORMALMAP_FLAT );
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER12, TEXTURE_NORMALMAP_FLAT );
				}
			}

			if ( ( info.m_nDetail != -1 ) && params[info.m_nDetail]->IsTexture() )
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER13, info.m_nDetail, info.m_nDetailFrame );
			}

			if ( IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) )
			{
				if ( bHasSelfIllumMask )												// Separate texture for self illum?
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER14, info.m_nSelfIllumMask );	// Bind it
				}
				else																	// else
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER14, TEXTURE_BLACK );	// Bind dummy
				}
			}

			if ( !bHasFlashlight )
			{
				if ( ( info.m_nEnvmap != -1 ) && params[info.m_nEnvmap]->IsTexture() )
				{
					pContextData->m_SemiStaticCmdsOut.BindTexture( pShader, SHADER_SAMPLER8, info.m_nEnvmap, info.m_nEnvmapFrame );
				}
			}

			pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nBaseTextureTransform );

			if ( ( info.m_nDetail != -1 ) && params[info.m_nDetail]->IsTexture() )
			{
				if ( IS_PARAM_DEFINED( info.m_nDetailTextureTransform ) )
				{
					pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2,
						info.m_nDetailTextureTransform,
						info.m_nDetailScale );
				}
				else
				{
					pContextData->m_SemiStaticCmdsOut.SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2,
						info.m_nBaseTextureTransform,
						info.m_nDetailScale );
				}
			}

			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant_W( PSREG_SELFILLUMTINT, info.m_nSelfIllumTint, fBlendFactor );
			bool bInvertPhongMask = ( info.m_nInvertPhongMask != -1 ) && ( params[info.m_nInvertPhongMask]->GetIntValue() != 0 );
			float fInvertPhongMask = bInvertPhongMask ? 1 : 0;

			bool bHasBaseAlphaPhongMask = ( info.m_nBaseMapAlphaPhongMask != -1 ) && ( params[info.m_nBaseMapAlphaPhongMask]->GetIntValue() != 0 );
			float fHasBaseAlphaPhongMask = bHasBaseAlphaPhongMask ? 1 : 0;
			bool bBlendTintByBaseAlpha = ( info.m_nBlendTintByBaseAlpha != -1 ) && ( params[info.m_nBlendTintByBaseAlpha]->GetIntValue() != 0 );
			float fBlendTintByBaseAlpha = bBlendTintByBaseAlpha ? 1 : 0;

			// Controls for lerp-style paths through shader code
			float vShaderControls[4] = { fHasBaseAlphaPhongMask, 0.0f, 1.0f - fBlendTintByBaseAlpha, fInvertPhongMask };
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_SHADER_CONTROLS, vShaderControls, 1 );

			if ( IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && ( info.m_nSelfIllumFresnel != -1 ) && ( params[info.m_nSelfIllumFresnel]->GetIntValue() != 0 ) && !bHasFlashlight )
			{
				float vConstScaleBiasExp[4] = { 1.0f, 0.0f, 1.0f, 0.0f };
				float flMin = IS_PARAM_DEFINED( info.m_nSelfIllumFresnelMinMaxExp ) ? params[info.m_nSelfIllumFresnelMinMaxExp]->GetVecValue()[0] : 0.0f;
				float flMax = IS_PARAM_DEFINED( info.m_nSelfIllumFresnelMinMaxExp ) ? params[info.m_nSelfIllumFresnelMinMaxExp]->GetVecValue()[1] : 1.0f;
				float flExp = IS_PARAM_DEFINED( info.m_nSelfIllumFresnelMinMaxExp ) ? params[info.m_nSelfIllumFresnelMinMaxExp]->GetVecValue()[2] : 1.0f;

				vConstScaleBiasExp[1] = ( flMax != 0.0f ) ? ( flMin / flMax ) : 0.0f; // Bias
				vConstScaleBiasExp[0] = 1.0f - vConstScaleBiasExp[1]; // Scale
				vConstScaleBiasExp[2] = flExp; // Exp
				vConstScaleBiasExp[3] = flMax; // Brightness

				pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_SELFILLUM_SCALE_BIAS_EXP, vConstScaleBiasExp, 1 );
			}

			if ( !bHasFlashlight )
			{
				pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER5, TEXTURE_NORMALIZATION_CUBEMAP_SIGNED );

				if ( ( info.m_nEnvmap != -1 ) && params[info.m_nEnvmap]->IsTexture() )
				{
					float vEnvMapTint_MaskControl[4] = { 1.0f, 1.0f, 1.0f, 0.0f };

					// If we have a tint, grab it
					if ( ( info.m_nEnvmapTint != -1 ) && params[info.m_nEnvmapTint]->IsDefined() )
						params[info.m_nEnvmapTint]->GetVecValue( vEnvMapTint_MaskControl, 3 );

					// Set control for source of env map mask (normal alpha or base alpha)
					vEnvMapTint_MaskControl[3] = bHasNormalMapAlphaEnvmapMask ? 1.0f : 0.0f;

					// Handle mat_fullbright 2 (diffuse lighting only with 50% gamma space basetexture)
					if ( bLightingOnly )
					{
						vEnvMapTint_MaskControl[0] = vEnvMapTint_MaskControl[1] = vEnvMapTint_MaskControl[2] = 0.0f;
					}

					pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_ENVMAP_TINT__SHADOW_TWEAKS, vEnvMapTint_MaskControl, 1 );
				}
			}

			// Pack Phong exponent in with the eye position
			float vSpecularTint[4] = { 1, 1, 1, 4 };
			float vFresnelRanges_SpecBoost[4] = { 0, 0.5, 1, 1 }, vRimBoost[4] = { 1, 1, 1, 1 };

			// Get the tint parameter
			if ( ( info.m_nPhongTint != -1 ) && params[info.m_nPhongTint]->IsDefined() )
			{
				params[info.m_nPhongTint]->GetVecValue( vSpecularTint, 3 );
			}

			// Get the rim light power (goes in w of Phong tint)
			if ( bHasRimLight && ( info.m_nRimLightPower != -1 ) && params[info.m_nRimLightPower]->IsDefined() )
			{
				vSpecularTint[3] = params[info.m_nRimLightPower]->GetFloatValue();
				vSpecularTint[3] = MAX( vSpecularTint[3], 1.0f );	// Make sure this is at least 1
			}

			// Get the rim boost (goes in w of flashlight position)
			if ( bHasRimLight && ( info.m_nRimLightBoost != -1 ) && params[info.m_nRimLightBoost]->IsDefined() )
			{
				vRimBoost[3] = params[info.m_nRimLightBoost]->GetFloatValue();
			}

			// Single pass flashlight has to use a separate constant for this stuff since a flashlight constant is repurposed for rimlighting when doing multi-pass flashlight.
			if ( bHasRimLight )
			{
				if ( !bHasFlashlight )
				{
					float vRimMaskControl[4] = { 0, 0, 0, 0 }; // Only x is relevant in shader code
					vRimMaskControl[0] = bHasRimMaskMap ? params[info.m_nRimMask]->GetFloatValue() : 0.0f;

					// Rim mask...if this is true, use alpha channel of spec exponent texture to mask the rim term
					pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_FLASHLIGHT_ATTENUATION, vRimMaskControl, 1 );
				}
			}

			// If it's all zeros, there was no constant tint in the vmt
			if ( ( vSpecularTint[0] == 0.0f ) && ( vSpecularTint[1] == 0.0f ) && ( vSpecularTint[2] == 0.0f ) )
			{
				if ( bHasPhongTintMap )				// If we have a map to use, tell the shader
				{
					vSpecularTint[0] = -1;
				}
				else								// Otherwise, just tint with white
				{
					vSpecularTint[0] = 1.0f;
					vSpecularTint[1] = 1.0f;
					vSpecularTint[2] = 1.0f;
				}
			}

			// handle mat_fullbright 2 (diffuse lighting only)
			if ( bLightingOnly )
			{
				// BASETEXTURE
				if ( IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) && !bHasFlashlight )
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER0, TEXTURE_GREY_ALPHA_ZERO );

					if ( bHasBaseTextureWrinkle || bHasBumpWrinkle )
					{
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER9, TEXTURE_GREY_ALPHA_ZERO );	// Compressed wrinklemap
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER10, TEXTURE_GREY_ALPHA_ZERO );	// Stretched wrinklemap
					}
				}
				else
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER0, TEXTURE_GREY );

					if ( bHasBaseTextureWrinkle || bHasBumpWrinkle )
					{
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER9, TEXTURE_GREY );		// Compressed wrinklemap
						pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER10, TEXTURE_GREY );	// Stretched wrinklemap
					}
				}

				// DETAILTEXTURE
				if ( ( info.m_nDetail != -1 ) && params[info.m_nDetail]->IsTexture() )
				{
					pContextData->m_SemiStaticCmdsOut.BindStandardTexture( SHADER_SAMPLER13, TEXTURE_GREY );
				}

				// turn off specularity
				vSpecularTint[0] = vSpecularTint[1] = vSpecularTint[2] = 0.0f;
			}

			if ( ( info.m_nPhongFresnelRanges != -1 ) && params[info.m_nPhongFresnelRanges]->IsDefined() )
			{
				params[info.m_nPhongFresnelRanges]->GetVecValue( vFresnelRanges_SpecBoost, 3 );	// Grab optional Fresnel range parameters
			}

			if ( ( info.m_nPhongBoost != -1 ) && params[info.m_nPhongBoost]->IsDefined() )		// Grab optional Phong boost param
			{
				vFresnelRanges_SpecBoost[3] = params[info.m_nPhongBoost]->GetFloatValue();
			}
			else
			{
				vFresnelRanges_SpecBoost[3] = 1.0f;
			}

			bool bHasBaseLuminancePhongMask = ( info.m_nBaseMapLuminancePhongMask != -1 ) && ( params[info.m_nBaseMapLuminancePhongMask]->GetIntValue() != 0 );
			float fHasBaseLuminancePhongMask = bHasBaseLuminancePhongMask ? 1 : 0;
			float vShaderControls2[4] = { 0.0f, fHasBaseLuminancePhongMask, 0.0f, 0.0f };
			if ( !bHasFlashlight )
			{
				if ( ( info.m_nEnvmap != -1 ) && params[info.m_nEnvmap]->IsTexture() )
				{
					if ( ( info.m_nEnvmapFresnel != -1 ) && params[info.m_nEnvmapFresnel]->IsDefined() )
					{
						vShaderControls2[0] = params[info.m_nEnvmapFresnel]->GetFloatValue();
					}
				}
			}
			if ( ( info.m_nPhongExponent != -1 ) && params[info.m_nPhongExponent]->IsDefined() )
			{
				vShaderControls2[2] = params[info.m_nPhongExponent]->GetFloatValue();		// This overrides the channel in the map
			}
			else
			{
				vShaderControls2[2] = 0;													// Use the alpha channel of the normal map for the exponent
			}

			vShaderControls2[3] = bHasSelfIllumMask ? 1.0f : 0.0f;

			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_FRESNEL_SPEC_PARAMS, vFresnelRanges_SpecBoost, 1 );
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_FLASHLIGHT_POSITION_RIM_BOOST, vRimBoost, 1 );	// Rim boost in w on non-flashlight pass
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_SPEC_RIM_PARAMS, vSpecularTint, 1 );
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderConstant( PSREG_SHADER_CONTROLS_2, vShaderControls2, 1 );

			pContextData->m_SemiStaticCmdsOut.SetPixelShaderFogParams( PSREG_FOG_PARAMS );

			if ( bHasFlashlight )
			{
				CBCmdSetPixelShaderFlashlightState_t state;
				state.m_LightSampler = SHADER_SAMPLER6;
				state.m_DepthSampler = SHADER_SAMPLER4;
				state.m_ShadowNoiseSampler = SHADER_SAMPLER5;
				state.m_nColorConstant = PSREG_FLASHLIGHT_COLOR;
				state.m_nAttenConstant = PSREG_FLASHLIGHT_ATTENUATION;
				state.m_nOriginConstant = PSREG_FLASHLIGHT_POSITION_RIM_BOOST;
				state.m_nDepthTweakConstant = PSREG_ENVMAP_TINT__SHADOW_TWEAKS; // NOTE: Reg 43 not available on < ps3.0!
				state.m_nScreenScaleConstant = PSREG_FLASHLIGHT_SCREEN_SCALE;
				state.m_nWorldToTextureConstant = PSREG_FLASHLIGHT_TO_WORLD_TEXTURE;
				state.m_bFlashlightNoLambert = false;
				state.m_bSinglePassFlashlight = false;
				pContextData->m_SemiStaticCmdsOut.SetPixelShaderFlashlightState( state );

				if ( g_pHardwareConfig->HasFastVertexTextures() )
				{
					pContextData->m_SemiStaticCmdsOut.SetPixelShaderUberLightState(
						PSREG_UBERLIGHT_SMOOTH_EDGE_0, PSREG_UBERLIGHT_SMOOTH_EDGE_1,
						PSREG_UBERLIGHT_SMOOTH_EDGE_OOW, PSREG_UBERLIGHT_SHEAR_ROUND,
						PSREG_UBERLIGHT_AABB, PSREG_UBERLIGHT_WORLD_TO_LIGHT );
				}
			}
			pContextData->m_SemiStaticCmdsOut.End();
		}

		CCommandBufferBuilder< CFixedCommandStorageBuffer< 1000 > > DynamicCmdsOut;
		DynamicCmdsOut.Call( pContextData->m_SemiStaticCmdsOut.Base() );

		// On PC, we sample from ambient occlusion texture
		if ( g_pHardwareConfig->HasFastVertexTextures() )
		{
			ITexture *pAOTexture = pShaderAPI->GetTextureRenderingParameter( TEXTURE_RENDERPARM_AMBIENT_OCCLUSION );

			if ( pAOTexture )
			{
				DynamicCmdsOut.BindTexture( pShader, SHADER_SAMPLER15, pAOTexture, 0 );
			}
			else
			{
				DynamicCmdsOut.BindStandardTexture( SHADER_SAMPLER15, TEXTURE_WHITE );
			}
		}

		bool bFlashlightShadows = false;
		bool bUberlight = false;
		float flAmbientOcclusionStrength = ( info.m_nAmbientOcclusion == -1 ) ? 0.0f : params[info.m_nAmbientOcclusion]->GetFloatValue();
		if ( bHasFlashlight )
		{
			pShaderAPI->GetFlashlightShaderInfo( &bFlashlightShadows, &bUberlight );
			flAmbientOcclusionStrength *= pShaderAPI->GetFlashlightAmbientOcclusion();
		}

		float vEyePos_AmbientOcclusion[4];
		pShaderAPI->GetWorldSpaceCameraPosition( vEyePos_AmbientOcclusion );
		vEyePos_AmbientOcclusion[3] = clamp( flAmbientOcclusionStrength, 0.0f, 1.0f );
		DynamicCmdsOut.SetPixelShaderConstant( PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_AmbientOcclusion, 1 );

		MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
		int numBones = pShaderAPI->GetCurrentNumBones();

		bool bWriteDepthToAlpha = false;
		bool bWriteWaterFogToAlpha = false;

		if ( bFullyOpaque )
		{
			bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
			bWriteWaterFogToAlpha = ( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			AssertMsg( !( bWriteDepthToAlpha && bWriteWaterFogToAlpha ), "Can't write two values to alpha at the same time." );
		}

		LightState_t lightState = { 0, false, false };
		if ( !bHasFlashlight )
		{
			pShaderAPI->GetDX9LightState( &lightState );
		}

		if ( !g_pHardwareConfig->HasFastVertexTextures() )
		{
			DECLARE_DYNAMIC_VERTEX_SHADER( character_vs20 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, numBones > 0 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, ( int )vertexCompression );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( TESSELLATION, 0 );
			SET_DYNAMIC_VERTEX_SHADER( character_vs20 );

			DECLARE_DYNAMIC_PIXEL_SHADER( character_ps20b );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( NUM_LIGHTS, lightState.m_nNumLights );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( FLASHLIGHTSHADOWS, bFlashlightShadows );
			SET_DYNAMIC_PIXEL_SHADER( character_ps20b );
		}
		else
		{
			pShader->SetHWMorphVertexShaderState( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, VERTEX_SHADER_SHADER_SPECIFIC_CONST_5, SHADER_VERTEXTEXTURE_SAMPLER0 );

			int nLightingPreviewMode = pShaderAPI->GetIntRenderingParameter( INT_RENDERPARM_ENABLE_FIXED_LIGHTING );
			if ( ( nLightingPreviewMode == ENABLE_FIXED_LIGHTING_OUTPUTNORMAL_AND_DEPTH ) && IsPC() )
			{
				float vEyeDir[4];
				pShaderAPI->GetWorldSpaceCameraDirection( vEyeDir );

				float flFarZ = pShaderAPI->GetFarZ();
				vEyeDir[0] /= flFarZ;	// Divide by farZ for SSAO algorithm
				vEyeDir[1] /= flFarZ;
				vEyeDir[2] /= flFarZ;
				pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, vEyeDir );
			}

			TessellationMode_t nTessellationMode = pShaderAPI->GetTessellationMode();
			if ( nTessellationMode != TESSELLATION_MODE_DISABLED )
			{
				pShaderAPI->BindStandardVertexTexture( SHADER_VERTEXTEXTURE_SAMPLER1, TEXTURE_SUBDIVISION_PATCHES );



				float vSubDDimensions[4] = { 1.0f / pShaderAPI->GetSubDHeight(),
											 bHasDisplacement && mat_displacementmap.GetBool() ? 1.0f : 0.0f,
											 bHasDisplacementWrinkles && mat_displacementmap.GetBool() ? 1.0f : 0.0f, 0.0f };

				pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_7, vSubDDimensions );
				if ( bHasDisplacement )
				{
					pShader->BindVertexTexture( SHADER_VERTEXTEXTURE_SAMPLER2, info.m_nDisplacementMap );
				}
				else
				{
					pShaderAPI->BindStandardVertexTexture( SHADER_VERTEXTEXTURE_SAMPLER2, TEXTURE_BLACK );
				}

				// Currently, tessellation is mutually exclusive with any kind of GPU-side skinning, morphing or vertex compression
				Assert( !pShaderAPI->IsHWMorphingEnabled() );
				Assert( numBones == 0 );
				Assert( vertexCompression == 0 );
			}
			DECLARE_DYNAMIC_VERTEX_SHADER( character_vs30 );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, ( numBones > 0 ) && ( nTessellationMode == TESSELLATION_MODE_DISABLED ) );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, ( int )vertexCompression && ( nTessellationMode == TESSELLATION_MODE_DISABLED ) );
			SET_DYNAMIC_VERTEX_SHADER_COMBO( TESSELLATION, nTessellationMode );
			SET_DYNAMIC_VERTEX_SHADER( character_vs30 );

			DECLARE_DYNAMIC_PIXEL_SHADER( character_ps30 );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( NUM_LIGHTS, nLightingPreviewMode ? 0 : lightState.m_nNumLights );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITEWATERFOGTODESTALPHA, nLightingPreviewMode ? false : bWriteWaterFogToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( FLASHLIGHTSHADOWS, nLightingPreviewMode ? false : bFlashlightShadows );
			SET_DYNAMIC_PIXEL_SHADER_COMBO( UBERLIGHT, bUberlight );
			SET_DYNAMIC_PIXEL_SHADER( character_ps30 );

			bool bUnusedTexCoords[3] = { false, false, !pShaderAPI->IsHWMorphingEnabled() || !bIsDecal };
			pShaderAPI->MarkUnusedVertexFields( 0, 3, bUnusedTexCoords );

			// Set constant to enable translation of VPOS to render target coordinates in ps_3_0
			pShaderAPI->SetScreenSizeForVPOS();
		}

		DynamicCmdsOut.End();
		pShaderAPI->ExecuteCommandBuffer( DynamicCmdsOut.Base() );
	}
	pShader->Draw();
}

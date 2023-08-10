//==================================================================================================
//
// Physically Based Rendering shader for brushes and models
//
//==================================================================================================

// Includes for all shaders
#include "BaseVSShader.h"
#include "cpp_shader_constant_register_map.h"

// Includes for PS30
#include "pbr_vs30.inc"
#include "pbr_ps30.inc"

// Includes for PS20b
#include "pbr_vs20.inc"
#include "pbr_ps20b.inc"

// Defining samplers
const Sampler_t SAMPLER_BASETEXTURE = SHADER_SAMPLER0;
const Sampler_t SAMPLER_NORMAL = SHADER_SAMPLER1;
const Sampler_t SAMPLER_ENVMAP = SHADER_SAMPLER2;
const Sampler_t SAMPLER_SHADOWDEPTH = SHADER_SAMPLER4;
const Sampler_t SAMPLER_RANDOMROTATION = SHADER_SAMPLER5;
const Sampler_t SAMPLER_FLASHLIGHT = SHADER_SAMPLER6;
const Sampler_t SAMPLER_LIGHTMAP = SHADER_SAMPLER7;
const Sampler_t SAMPLER_MRAO = SHADER_SAMPLER10;
const Sampler_t SAMPLER_EMISSIVE = SHADER_SAMPLER11;
const Sampler_t SAMPLER_SPECULAR = SHADER_SAMPLER12;

// Convars
static ConVar mat_fullbright( "mat_fullbright", "0", FCVAR_CHEAT );
static ConVar mat_specular( "mat_specular", "1" );
static ConVar mat_pbr_force_20b( "mat_pbr_force_20b", "0", FCVAR_CHEAT );
static ConVar mat_pbr_parallaxmap( "mat_pbr_parallaxmap", "1" );
extern ConVar mat_allow_parallax_cubemaps;

// Variables for this shader
struct PBR_Vars_t
{
	PBR_Vars_t()
	{
		memset( this, 0xFF, sizeof( *this ) );
	}

	int baseTexture;
	int baseColor;
	int normalTexture;
	int bumpMap;
	int envMap;
	int baseTextureFrame;
	int baseTextureTransform;
	int useParallax;
	int parallaxDepth;
	int parallaxCenter;
	int alphaTestReference;
	int flashlightTexture;
	int flashlightTextureFrame;
	int emissionTexture;
	int mraoTexture;
	int useEnvAmbient;
	int specularTexture;

	int m_nEnvmapParallax;
	int m_nEnvmapParallaxObb1;
	int m_nEnvmapParallaxObb2;
	int m_nEnvmapParallaxObb3;
	int m_nEnvmapOrigin;
};

// Beginning the shader
BEGIN_VS_SHADER(PBR, "PBR shader")

	// Setting up vmt parameters
	BEGIN_SHADER_PARAMS;
		SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0", "");
		SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_ENVMAP, "", "Set the cubemap for this material.");
		SHADER_PARAM(MRAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Texture with metalness in R, roughness in G, ambient occlusion in B.");
		SHADER_PARAM(EMISSIONTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Emission texture");
		SHADER_PARAM(NORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture (deprecated, use $bumpmap)");
		SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture");
		SHADER_PARAM(USEENVAMBIENT, SHADER_PARAM_TYPE_BOOL, "0", "Use the cubemaps to compute ambient light.");
		SHADER_PARAM(SPECULARTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Specular F0 RGB map");
		SHADER_PARAM(PARALLAX, SHADER_PARAM_TYPE_BOOL, "0", "Use Parallax Occlusion Mapping.");
		SHADER_PARAM(PARALLAXDEPTH, SHADER_PARAM_TYPE_FLOAT, "0.0030", "Depth of the Parallax Map");
		SHADER_PARAM(PARALLAXCENTER, SHADER_PARAM_TYPE_FLOAT, "0.5", "Center depth of the Parallax Map");

		// Parallax-Corrected Cubemaps (ported from LightmappedGeneric)
		SHADER_PARAM( ENVMAPPARALLAX, SHADER_PARAM_TYPE_BOOL, "0", "Should envmap reflections be parallax-corrected?" )
		SHADER_PARAM( ENVMAPPARALLAXOBB1, SHADER_PARAM_TYPE_VEC4, "[1 0 0 0]", "The first line of the parallax correction OBB matrix" )
		SHADER_PARAM( ENVMAPPARALLAXOBB2, SHADER_PARAM_TYPE_VEC4, "[0 1 0 0]", "The second line of the parallax correction OBB matrix" )
		SHADER_PARAM( ENVMAPPARALLAXOBB3, SHADER_PARAM_TYPE_VEC4, "[0 0 1 0]", "The third line of the parallax correction OBB matrix" )
		SHADER_PARAM( ENVMAPORIGIN, SHADER_PARAM_TYPE_VEC3, "[0 0 0]", "The world space position of the env_cubemap being corrected" )
	END_SHADER_PARAMS;

	// Setting up variables for this shader
	void SetupVars(PBR_Vars_t &info)
	{
		info.baseTexture = BASETEXTURE;
		info.baseColor = COLOR;
		info.normalTexture = NORMALTEXTURE;
		info.bumpMap = BUMPMAP;
		info.baseTextureFrame = FRAME;
		info.baseTextureTransform = BASETEXTURETRANSFORM;
		info.alphaTestReference = ALPHATESTREFERENCE;
		info.flashlightTexture = FLASHLIGHTTEXTURE;
		info.flashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
		info.envMap = ENVMAP;
		info.emissionTexture = EMISSIONTEXTURE;
		info.mraoTexture = MRAOTEXTURE;
		info.useEnvAmbient = USEENVAMBIENT;
		info.specularTexture = SPECULARTEXTURE;
		info.useParallax = PARALLAX;
		info.parallaxDepth = PARALLAXDEPTH;
		info.parallaxCenter = PARALLAXCENTER;

		info.m_nEnvmapParallax = ENVMAPPARALLAX;
		info.m_nEnvmapParallaxObb1 = ENVMAPPARALLAXOBB1;
		info.m_nEnvmapParallaxObb2 = ENVMAPPARALLAXOBB2;
		info.m_nEnvmapParallaxObb3 = ENVMAPPARALLAXOBB3;
		info.m_nEnvmapOrigin = ENVMAPORIGIN;
	};

	// Initializing parameters
	SHADER_INIT_PARAMS()
	{
		// Fallback for changed parameter
		if (params[NORMALTEXTURE]->IsDefined())
			params[BUMPMAP]->SetStringValue(params[NORMALTEXTURE]->GetStringValue());

		// Dynamic lights need a bumpmap
		if (!params[BUMPMAP]->IsDefined())
			params[BUMPMAP]->SetStringValue("dev/flat_normal");

		// Set a good default mrao texture
		if (!params[MRAOTEXTURE]->IsDefined())
			params[MRAOTEXTURE]->SetStringValue("dev/pbr_mraotexture");

		// PBR relies heavily on envmaps
		if (!params[ENVMAP]->IsDefined())
			params[ENVMAP]->SetStringValue("env_cubemap");

		// Check if the hardware supports flashlight border color
		if (g_pHardwareConfig->SupportsBorderColor())
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
		}
		else
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
		}
	};

	// Define shader fallback
	SHADER_FALLBACK
	{
		return 0;
	};

	SHADER_INIT
	{
		PBR_Vars_t info;
		SetupVars(info);

		Assert(info.flashlightTexture >= 0);
		LoadTexture(info.flashlightTexture);

		Assert(info.bumpMap >= 0);
		LoadBumpMap(info.bumpMap);

		Assert(info.envMap >= 0);
		LoadCubeMap(info.envMap);

		if (info.emissionTexture >= 0 && params[EMISSIONTEXTURE]->IsDefined())
			LoadTexture(info.emissionTexture);

		Assert(info.mraoTexture >= 0);
		LoadTexture(info.mraoTexture);

		if (params[info.baseTexture]->IsDefined())
		{
			LoadTexture(info.baseTexture);
		}

		if (params[info.specularTexture]->IsDefined())
		{
			LoadTexture(info.specularTexture);
		}

		if (IS_FLAG_SET(MATERIAL_VAR_MODEL)) // Set material var2 flags specific to models
		{
			SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);             // Required for skinning
			SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);         // Required for dynamic lighting
			SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);             // Required for dynamic lighting
			SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);              // Required for dynamic lighting
			SET_FLAGS2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);   // Required for ambient cube
			SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
			SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight
		}
		else // Set material var2 flags specific to brushes
		{
			SET_FLAGS2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);                // Required for lightmaps
			SET_FLAGS2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);         // Required for lightmaps
			SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
			SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight
		}

		// Cubemap parallax correction requires all 4 lines
		if ( info.m_nEnvmapParallax != -1 && !( mat_allow_parallax_cubemaps.GetBool() && params[info.m_nEnvmapParallax]->IsDefined() && IS_PARAM_DEFINED( info.m_nEnvmapParallaxObb1 ) && IS_PARAM_DEFINED( info.m_nEnvmapParallaxObb2 ) && IS_PARAM_DEFINED( info.m_nEnvmapParallaxObb3 ) && IS_PARAM_DEFINED( info.m_nEnvmapOrigin ) ) )
		{
			params[info.m_nEnvmapParallax]->SetIntValue( 0 );
		}
	};

	// Drawing the shader
	SHADER_DRAW
	{
		PBR_Vars_t info;
		SetupVars(info);

		// Setting up booleans
		bool bHasBaseTexture = (info.baseTexture != -1) && params[info.baseTexture]->IsTexture();
		bool bHasNormalTexture = (info.bumpMap != -1) && params[info.bumpMap]->IsTexture();
		bool bHasMraoTexture = (info.mraoTexture != -1) && params[info.mraoTexture]->IsTexture();
		bool bHasEmissionTexture = (info.emissionTexture != -1) && params[info.emissionTexture]->IsTexture();
		bool bHasEnvTexture = (info.envMap != -1) && params[info.envMap]->IsTexture();
		bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) != 0;
		bool bHasFlashlight = UsingFlashlight(params);
		bool bHasColor = (info.baseColor != -1) && params[info.baseColor]->IsDefined();
		bool bLightMapped = !IS_FLAG_SET(MATERIAL_VAR_MODEL);
		bool bUseEnvAmbient = (info.useEnvAmbient != -1) && (params[info.useEnvAmbient]->GetIntValue() == 1);
		bool bHasSpecularTexture = (info.specularTexture != -1) && params[info.specularTexture]->IsTexture();
		bool hasParallaxCorrection = info.m_nEnvmapParallax != -1 && params[info.m_nEnvmapParallax]->GetIntValue() != 0;

		int useParallax = params[info.useParallax]->GetIntValue();
		if ( !mat_pbr_parallaxmap.GetBool() || !g_pHardwareConfig->HasFastVertexTextures() || mat_pbr_force_20b.GetBool() )
		{
			useParallax = 0;
		}

		// Determining whether we're dealing with a fully opaque material
		BlendType_t nBlendType = EvaluateBlendRequirements(info.baseTexture, true);
		bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested;

		if (IsSnapshotting())
		{
			// If alphatest is on, enable it
			pShaderShadow->EnableAlphaTest(bIsAlphaTested);

			if (info.alphaTestReference != -1 && params[info.alphaTestReference]->GetFloatValue() > 0.0f)
			{
				pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.alphaTestReference]->GetFloatValue());
			}

			if (bHasFlashlight )
			{
				pShaderShadow->EnableBlending(true);
				pShaderShadow->BlendFunc(SHADER_BLEND_ONE, SHADER_BLEND_ONE); // Additive blending
			}
			else
			{
				SetDefaultBlendingShadowState(info.baseTexture, true);
			}

			int nShadowFilterMode = bHasFlashlight ? g_pHardwareConfig->GetShadowFilterMode() : 0;

			// Setting up samplers
			pShaderShadow->EnableTexture(SAMPLER_BASETEXTURE, true);    // Basecolor texture
			pShaderShadow->EnableSRGBRead(SAMPLER_BASETEXTURE, true);   // Basecolor is sRGB
			pShaderShadow->EnableTexture(SAMPLER_EMISSIVE, true);       // Emission texture
			pShaderShadow->EnableSRGBRead(SAMPLER_EMISSIVE, true);      // Emission is sRGB
			pShaderShadow->EnableTexture(SAMPLER_LIGHTMAP, true);       // Lightmap texture
			pShaderShadow->EnableSRGBRead(SAMPLER_LIGHTMAP, false);     // Lightmaps aren't sRGB
			pShaderShadow->EnableTexture(SAMPLER_MRAO, true);           // MRAO texture
			pShaderShadow->EnableSRGBRead(SAMPLER_MRAO, false);         // MRAO isn't sRGB
			pShaderShadow->EnableTexture(SAMPLER_NORMAL, true);         // Normal texture
			pShaderShadow->EnableSRGBRead(SAMPLER_NORMAL, false);       // Normals aren't sRGB
			pShaderShadow->EnableTexture(SAMPLER_SPECULAR, true);       // Specular F0 texture
			pShaderShadow->EnableSRGBRead(SAMPLER_SPECULAR, true);      // Specular F0 is sRGB

			// If the flashlight is on, set up its textures
			if (bHasFlashlight)
			{
				pShaderShadow->EnableTexture(SAMPLER_SHADOWDEPTH, true);        // Shadow depth map
				pShaderShadow->SetShadowDepthFiltering(SAMPLER_SHADOWDEPTH);
				pShaderShadow->EnableSRGBRead(SAMPLER_SHADOWDEPTH, false);
				pShaderShadow->EnableTexture(SAMPLER_RANDOMROTATION, true);     // Noise map
				pShaderShadow->EnableTexture(SAMPLER_FLASHLIGHT, true);         // Flashlight cookie
				pShaderShadow->EnableSRGBRead(SAMPLER_FLASHLIGHT, true);
			}

			// Setting up envmap
			if (bHasEnvTexture)
			{
				pShaderShadow->EnableTexture(SAMPLER_ENVMAP, true); // Envmap
				if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
				{
					pShaderShadow->EnableSRGBRead(SAMPLER_ENVMAP, true); // Envmap is only sRGB with HDR disabled?
				}
			}

			// Enabling sRGB writing
			// See common_ps_fxc.h line 349
			// PS2b shaders and up write sRGB
			pShaderShadow->EnableSRGBWrite(true);

			if (IS_FLAG_SET(MATERIAL_VAR_MODEL))
			{
				// We only need the position and surface normal
				unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
				// We need three texcoords, all in the default float2 size
				pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);
			}
			else
			{
				// We need the position, surface normal, and vertex compression format
				unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
				// We only need one texcoord, in the default float2 size
				pShaderShadow->VertexShaderVertexFormat(flags, 3, 0, 0);
			}

			if (!g_pHardwareConfig->HasFastVertexTextures() || mat_pbr_force_20b.GetBool())
			{
				// Setting up static vertex shader
				DECLARE_STATIC_VERTEX_SHADER(pbr_vs20);
				SET_STATIC_VERTEX_SHADER(pbr_vs20);

				// Setting up static pixel shader
				DECLARE_STATIC_PIXEL_SHADER(pbr_ps20b);
				SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
				SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
				SET_STATIC_PIXEL_SHADER_COMBO(LIGHTMAPPED, bLightMapped);
				SET_STATIC_PIXEL_SHADER_COMBO(EMISSIVE, bHasEmissionTexture);
				SET_STATIC_PIXEL_SHADER_COMBO(SPECULAR, 0);
				SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXCORRECT, hasParallaxCorrection);
				SET_STATIC_PIXEL_SHADER(pbr_ps20b);
			}
			else
			{
				// Setting up static vertex shader
				DECLARE_STATIC_VERTEX_SHADER(pbr_vs30);
				SET_STATIC_VERTEX_SHADER(pbr_vs30);

				// Setting up static pixel shader
				DECLARE_STATIC_PIXEL_SHADER(pbr_ps30);
				SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
				SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
				SET_STATIC_PIXEL_SHADER_COMBO(LIGHTMAPPED, bLightMapped);
				SET_STATIC_PIXEL_SHADER_COMBO(USEENVAMBIENT, bUseEnvAmbient);
				SET_STATIC_PIXEL_SHADER_COMBO(EMISSIVE, bHasEmissionTexture);
				SET_STATIC_PIXEL_SHADER_COMBO(SPECULAR, bHasSpecularTexture);
				SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXOCCLUSION, useParallax);
				SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXCORRECT, hasParallaxCorrection);
				SET_STATIC_PIXEL_SHADER(pbr_ps30);
			}

			// Setting up fog
			DefaultFog(); // I think this is correct

			// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
			pShaderShadow->EnableAlphaWrites(bFullyOpaque);

			if ( hasParallaxCorrection )
			{
				pShaderAPI->SetPixelShaderConstant( 3, params[info.m_nEnvmapOrigin]->GetVecValue() );

				const float *vecs[3];
				vecs[0] = params[info.m_nEnvmapParallaxObb1]->GetVecValue();
				vecs[1] = params[info.m_nEnvmapParallaxObb2]->GetVecValue();
				vecs[2] = params[info.m_nEnvmapParallaxObb3]->GetVecValue();
				float matrix[4][4];
				for ( int i = 0; i < 3; i++ )
				{
					for ( int j = 0; j < 4; j++ )
					{
						matrix[i][j] = vecs[i][j];
					}
				}
				matrix[3][0] = matrix[3][1] = matrix[3][2] = 0;
				matrix[3][3] = 1;
				pShaderAPI->SetPixelShaderConstant( 26, &matrix[0][0], 4 );
			}

			PI_BeginCommandBuffer();

			// Send ambient cube to the pixel shader, force to black if not available
			PI_SetPixelShaderAmbientLightCube( PSREG_AMBIENT_CUBE );

			// Send lighting array to the pixel shader
			PI_SetPixelShaderLocalLighting( PSREG_LIGHT_INFO_ARRAY );

			// Set up shader modulation color
			PI_SetModulationPixelShaderDynamicState( PSREG_DIFFUSE_MODULATION );

			PI_EndCommandBuffer();
		}
		else // Not snapshotting -- begin dynamic state
		{
			bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE);

			// Setting up albedo texture
			if (bHasBaseTexture)
			{
				BindTexture(SAMPLER_BASETEXTURE, info.baseTexture, info.baseTextureFrame);
			}
			else
			{
				pShaderAPI->BindStandardTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
			}

			// Setting up vmt color
			Vector color;
			if (bHasColor)
			{
				params[info.baseColor]->GetVecValue(color.Base(), 3);
			}
			else
			{
				color = Vector{1.f, 1.f, 1.f};
			}
			pShaderAPI->SetPixelShaderConstant(PSREG_SELFILLUMTINT, color.Base());

			// Setting up environment map
			if (bHasEnvTexture)
			{
				BindTexture(SAMPLER_ENVMAP, info.envMap, 0);
			}
			else
			{
				pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK);
			}

			// Setting up emissive texture
			if (bHasEmissionTexture)
			{
				BindTexture(SAMPLER_EMISSIVE, info.emissionTexture, 0);
			}
			else
			{
				pShaderAPI->BindStandardTexture(SAMPLER_EMISSIVE, TEXTURE_BLACK);
			}

			// Setting up normal map
			if (bHasNormalTexture)
			{
				BindTexture(SAMPLER_NORMAL, info.bumpMap, 0);
			}
			else
			{
				pShaderAPI->BindStandardTexture(SAMPLER_NORMAL, TEXTURE_NORMALMAP_FLAT);
			}

			// Setting up mrao map
			if (bHasMraoTexture)
			{
				BindTexture(SAMPLER_MRAO, info.mraoTexture, 0);
			}
			else
			{
				pShaderAPI->BindStandardTexture(SAMPLER_MRAO, TEXTURE_WHITE);
			}

			if (bHasSpecularTexture)
			{
				BindTexture(SAMPLER_SPECULAR, info.specularTexture, 0);
			}
			else
			{
				pShaderAPI->BindStandardTexture(SAMPLER_SPECULAR, TEXTURE_BLACK);
			}

			// Getting the light state
			LightState_t lightState;
			pShaderAPI->GetDX9LightState(&lightState);

			// Brushes don't need ambient cubes or dynamic lights
			if (!IS_FLAG_SET(MATERIAL_VAR_MODEL))
			{
				lightState.m_bAmbientLight = false;
				lightState.m_nNumLights = 0;
			}

			// Setting up the flashlight related textures and variables
			bool bFlashlightShadows = false;
			if (bHasFlashlight)
			{
				Assert(info.flashlightTexture >= 0 && info.flashlightTextureFrame >= 0);
				Assert(params[info.flashlightTexture]->IsTexture());
				BindTexture(SAMPLER_FLASHLIGHT, info.flashlightTexture, info.flashlightTextureFrame);
				VMatrix worldToTexture;
				ITexture *pFlashlightDepthTexture;
				FlashlightState_t state = pShaderAPI->GetFlashlightStateEx(worldToTexture, &pFlashlightDepthTexture);
				bFlashlightShadows = state.m_bEnableShadows && (pFlashlightDepthTexture != NULL);

				SetFlashLightColorFromState(state, pShaderAPI, PSREG_FLASHLIGHT_COLOR);

				if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && state.m_bEnableShadows)
				{
					BindTexture(SAMPLER_SHADOWDEPTH, pFlashlightDepthTexture, 0);
					pShaderAPI->BindStandardTexture(SAMPLER_RANDOMROTATION, TEXTURE_SHADOW_NOISE_2D);
				}
			}

			// Getting fog info
			MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
			int fogIndex = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) ? 1 : 0;

			// Getting skinning info
			int numBones = pShaderAPI->GetCurrentNumBones();

			// Some debugging stuff
			bool bWriteDepthToAlpha = false;
			bool bWriteWaterFogToAlpha = false;
			if (bFullyOpaque)
			{
				bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
				bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
				AssertMsg(!(bWriteDepthToAlpha && bWriteWaterFogToAlpha),
						"Can't write two values to alpha at the same time.");
			}

			float vEyePos_SpecExponent[4];
			pShaderAPI->GetWorldSpaceCameraPosition(vEyePos_SpecExponent);

			// Determining the max level of detail for the envmap
			int iEnvMapLOD = 6;
			auto envTexture = params[info.envMap]->GetTextureValue();
			if (envTexture)
			{
				// Get power of 2 of texture width
				int width = envTexture->GetMappingWidth();
				int mips = 0;
				while (width >>= 1)
					++mips;

				// Cubemap has 4 sides so 2 mips less
				iEnvMapLOD = mips;
			}

			// Dealing with very high and low resolution cubemaps
			if (iEnvMapLOD > 12)
				iEnvMapLOD = 12;
			if (iEnvMapLOD < 4)
				iEnvMapLOD = 4;

			// This has some spare space
			vEyePos_SpecExponent[3] = iEnvMapLOD;
			pShaderAPI->SetPixelShaderConstant(PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1);

			// Setting lightmap texture
			s_pShaderAPI->BindStandardTexture(SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP_BUMPED);

			if (!g_pHardwareConfig->HasFastVertexTextures() || mat_pbr_force_20b.GetBool())
			{
				// Setting up dynamic vertex shader
				DECLARE_DYNAMIC_VERTEX_SHADER(pbr_vs20);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
				SET_DYNAMIC_VERTEX_SHADER(pbr_vs20);

				// Setting up dynamic pixel shader
				DECLARE_DYNAMIC_PIXEL_SHADER(pbr_ps20b);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
				SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
				SET_DYNAMIC_PIXEL_SHADER(pbr_ps20b);
			}
			else
			{
				// Setting up dynamic vertex shader
				DECLARE_DYNAMIC_VERTEX_SHADER(pbr_vs30);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
				SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
				SET_DYNAMIC_VERTEX_SHADER(pbr_vs30);

				// Setting up dynamic pixel shader
				DECLARE_DYNAMIC_PIXEL_SHADER(pbr_ps30);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
				SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
				SET_DYNAMIC_PIXEL_SHADER(pbr_ps30);
			}

			// Setting up base texture transform
			SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.baseTextureTransform);

			// Handle mat_fullbright 2 (diffuse lighting only)
			if (bLightingOnly)
			{
				pShaderAPI->BindStandardTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY); // Basecolor
			}

			// Handle mat_specular 0 (no envmap reflections)
			if (!mat_specular.GetBool())
			{
				pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK); // Envmap
			}

			// Sending fog info to the pixel shader
			pShaderAPI->SetPixelShaderFogParams(PSREG_FOG_PARAMS);

			// More flashlight related stuff
			if (bHasFlashlight)
			{
				VMatrix worldToTexture;
				float atten[4], pos[4], tweaks[4];

				const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);
				SetFlashLightColorFromState(flashlightState, pShaderAPI, PSREG_FLASHLIGHT_COLOR);

				BindTexture(SAMPLER_FLASHLIGHT, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame);

				// Set the flashlight attenuation factors
				atten[0] = flashlightState.m_fConstantAtten;
				atten[1] = flashlightState.m_fLinearAtten;
				atten[2] = flashlightState.m_fQuadraticAtten;
				atten[3] = flashlightState.m_FarZ;
				pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_ATTENUATION, atten, 1);

				// Set the flashlight origin
				pos[0] = flashlightState.m_vecLightOrigin[0];
				pos[1] = flashlightState.m_vecLightOrigin[1];
				pos[2] = flashlightState.m_vecLightOrigin[2];
				pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1);

				pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4);

				// Tweaks associated with a given flashlight
				tweaks[0] = ShadowFilterFromState(flashlightState);
				tweaks[1] = ShadowAttenFromState(flashlightState);
				HashShadow2DJitter(flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3]);
				pShaderAPI->SetPixelShaderConstant(PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1);
			}

			if ( g_pHardwareConfig->HasFastVertexTextures() )
			{
				float flParams[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				// Parallax Depth (the strength of the effect)
				flParams[0] = GetFloatParam( info.parallaxDepth, params, 3.0f );
				// Parallax Center (the height at which it's not moved)
				flParams[1] = GetFloatParam( info.parallaxCenter, params, 3.0f );
				pShaderAPI->SetPixelShaderConstant( 32, flParams, 1 );
			}
		}

		// Actually draw the shader
		Draw();
	};

// Closing it off
END_SHADER;

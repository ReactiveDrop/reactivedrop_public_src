//========= Copyright © 1996-2007, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef CHARACTER_DX9_HELPER_H
#define CHARACTER_DX9_HELPER_H

#include <string.h>


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CBaseVSShader;
class IMaterialVar;
class IShaderDynamicAPI;
class IShaderShadow;


//-----------------------------------------------------------------------------
// Init params/ init/ draw methods
//-----------------------------------------------------------------------------
struct Character_DX9_Vars_t
{
	Character_DX9_Vars_t() { memset( this, 0xFF, sizeof( *this ) ); }

	int m_nBaseTexture;
	int m_nWrinkle;
	int m_nStretch;
	int m_nBaseTextureFrame;
	int m_nBaseTextureTransform;
	int m_nAlbedo;
	int m_nDetail;
	int m_nDetailFrame;
	int m_nDetailScale;
	int m_nEnvmap;
	int m_nEnvmapFrame;
	int m_nEnvmapMask;
	int m_nEnvmapMaskFrame;
	int m_nEnvmapMaskTransform;
	int m_nEnvmapTint;
	int m_nBumpmap;
	int m_nNormalWrinkle;
	int m_nNormalStretch;
	int m_nBumpFrame;
	int m_nBumpTransform;
	int m_nEnvmapContrast;
	int m_nEnvmapSaturation;
	int m_nAlphaTestReference;
	int m_nVertexAlphaTest;
	int m_nFlashlightNoLambert;
	int m_nFlashlightTexture;
	int m_nFlashlightTextureFrame;

	int m_nSelfIllumTint;
	int m_nSelfIllumFresnel;
	int m_nSelfIllumFresnelMinMaxExp;
	int m_nSelfIllumMaskScale;

	int m_nPhongExponent;
	int m_nPhongTint;
	int m_nPhongAlbedoTint;
	int m_nPhongExponentTexture;
	int m_nDiffuseWarpTexture;
	int m_nPhongWarpTexture;	
	int m_nPhongBoost;
	int m_nPhongFresnelRanges;
	int m_nSelfIllumEnvMapMask_Alpha;
	int m_nAmbientOnly;
	int m_nHDRColorScale;
	int m_nBaseMapAlphaPhongMask;
	int m_nBaseMapLuminancePhongMask;
	int m_nEnvmapFresnel;

	int m_nDetailTextureCombineMode;
	int m_nDetailTextureBlendFactor;

	// Rim lighting parameters
	int m_nRimLight;
	int m_nRimLightPower;
	int m_nRimLightBoost;
	int m_nRimMask;

	int m_nTime;

	int m_nSeparateDetailUVs;
	int m_nDetailTextureTransform;

	int m_nLinearWrite;
	int m_nGammaColorRead;

	int m_nDetailTint;
	int m_nInvertPhongMask;

	int m_nSelfIllumMask;
	int m_nReceiveFlashlight;
	int m_nSinglePassFlashlight;

	int m_nShaderSrgbRead360;

	int m_nAmbientOcclusion;

	int m_nBlendTintByBaseAlpha;

	int m_nDesaturateWithBaseAlpha;

	int m_nAllowDiffuseModulation;

	int m_nEnvMapFresnelMinMaxExp;

	int m_nBaseAlphaEnvMapMaskMinMaxExp;
	
	int m_nDisplacementMap;

	int m_nDisplacementWrinkleMap;

	int m_nPhongDisableHalfLambert;

	int m_nTintMaskTexture;

	int m_nCharacterTeamColor;
	int m_nCharacterStatusFx;
};

void InitParamsCharacter_DX9( CBaseVSShader *pShader, IMaterialVar **params, const char *pMaterialName, Character_DX9_Vars_t &info );
void InitCharacter_DX9( CBaseVSShader *pShader, IMaterialVar **params, Character_DX9_Vars_t &info );
void DrawCharacter_DX9( CBaseVSShader *pShader, IMaterialVar **params, IShaderDynamicAPI *pShaderAPI, IShaderShadow *pShaderShadow,
	Character_DX9_Vars_t &info, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr );


#endif // CHARACTER_DX9_HELPER_H

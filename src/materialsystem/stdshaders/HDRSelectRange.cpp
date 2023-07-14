//========= Copyright © 1996-2004, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "BaseVSShader.h"
#include "common_hlsl_cpp_consts.h"

#ifdef RD_SUPPORT_SHADER_MODEL_20
#include "hdrselectrange_ps20.inc"
#endif
#include "hdrselectrange_ps20b.inc"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"


BEGIN_VS_SHADER_FLAGS( HDRSelectRange, "Help for HDRSelectRange", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( SOURCEMRTRENDERTARGET, SHADER_PARAM_TYPE_TEXTURE, "", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
		LoadTexture( SOURCEMRTRENDERTARGET );
	}
	
	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableAlphaWrites( false );
			pShaderShadow->EnableDepthTest( false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );
			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			pShaderShadow->SetVertexShader( "HDRSelectRange_vs20", 0 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER( HDRSelectRange_ps20b );
				SET_STATIC_PIXEL_SHADER( HDRSelectRange_ps20b );
			}
			else
			{
#ifdef RD_SUPPORT_SHADER_MODEL_20
				DECLARE_STATIC_PIXEL_SHADER( HDRSelectRange_ps20 );
				SET_STATIC_PIXEL_SHADER( HDRSelectRange_ps20 );
#else
				RD_SHADER_MODEL_20_CRASH;
#endif
			}
			
		}

		DYNAMIC_STATE
		{
			BindTexture( SHADER_SAMPLER0, SOURCEMRTRENDERTARGET, -1 );
			pShaderAPI->SetVertexShaderIndex( 0 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_DYNAMIC_PIXEL_SHADER( HDRSelectRange_ps20b );
				SET_DYNAMIC_PIXEL_SHADER( HDRSelectRange_ps20b );
			}
			else
			{
#ifdef RD_SUPPORT_SHADER_MODEL_20
				DECLARE_DYNAMIC_PIXEL_SHADER( HDRSelectRange_ps20 );
				SET_DYNAMIC_PIXEL_SHADER( HDRSelectRange_ps20 );
#else
				RD_SHADER_MODEL_20_CRASH;
#endif
			}
		}
		Draw();
	}
END_SHADER

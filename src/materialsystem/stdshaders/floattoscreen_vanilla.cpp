//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"

#include "screenspaceeffect_vs20.inc"
#ifdef RD_SUPPORT_SHADER_MODEL_20
#include "floattoscreen_vanilla_ps20.inc"
#endif
#include "floattoscreen_vanilla_ps20b.inc"
#ifdef RD_SUPPORT_SHADER_MODEL_20
#include "floattoscreen_ps20.inc"
#endif
#include "floattoscreen_ps20b.inc"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"


BEGIN_VS_SHADER_FLAGS( floattoscreen_vanilla, "Help for floattoscreen_vanilla", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( FBTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
		if( params[FBTEXTURE]->IsDefined() )
		{
			LoadTexture( FBTEXTURE );
		}
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

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			// convert from linear to gamma on write.
			pShaderShadow->EnableSRGBWrite( true );

			// Pre-cache shaders
			DECLARE_STATIC_VERTEX_SHADER( screenspaceeffect_vs20 );
			SET_STATIC_VERTEX_SHADER( screenspaceeffect_vs20 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER( floattoscreen_ps20b );
				SET_STATIC_PIXEL_SHADER( floattoscreen_ps20b );
			}
			else
			{
#ifdef RD_SUPPORT_SHADER_MODEL_20
				DECLARE_STATIC_PIXEL_SHADER( floattoscreen_ps20 );
				SET_STATIC_PIXEL_SHADER( floattoscreen_ps20 );
#else
				RD_SHADER_MODEL_20_CRASH;
#endif
			}
		}

		DYNAMIC_STATE
		{
			BindTexture( SHADER_SAMPLER0, FBTEXTURE, -1 );
			DECLARE_DYNAMIC_VERTEX_SHADER( screenspaceeffect_vs20 );
			SET_DYNAMIC_VERTEX_SHADER( screenspaceeffect_vs20 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_DYNAMIC_PIXEL_SHADER( floattoscreen_vanilla_ps20b );
				SET_DYNAMIC_PIXEL_SHADER( floattoscreen_vanilla_ps20b );
			}
			else
			{
#ifdef RD_SUPPORT_SHADER_MODEL_20
				DECLARE_DYNAMIC_PIXEL_SHADER( floattoscreen_vanilla_ps20 );
				SET_DYNAMIC_PIXEL_SHADER( floattoscreen_vanilla_ps20 );
#else
				RD_SHADER_MODEL_20_CRASH;
#endif
			}
		}
		Draw();
	}
END_SHADER

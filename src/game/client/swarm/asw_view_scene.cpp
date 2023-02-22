//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Responsible for drawing the scene
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "asw_view_scene.h"
#include "view_scene.h"
#include "precache_register.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "c_asw_render_targets.h"
#include "materialsystem/IMaterialVar.h"
#include "renderparm.h"
#include "asw_weapon_night_vision.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"
#include "functionproxy.h"
#include "imaterialproxydict.h"
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

float g_fMarinePoisonDuration = 0;
bool g_bBlurredLastTime = false;
static float fNextDrawTime = 0.0f;
ConVar asw_motionblur( "asw_motionblur", "0", FCVAR_NONE, "Motion Blur" );			// motion blur on/off
ConVar asw_motionblur_addalpha( "asw_motionblur_addalpha", "0.3", FCVAR_NONE, "Motion Blur Alpha" );	// The amount of alpha to use when adding the FB to our custom buffer
ConVar asw_motionblur_drawalpha( "asw_motionblur_drawalpha", "1", FCVAR_NONE, "Motion Blur Draw Alpha" );		// The amount of alpha to use when adding our custom buffer to the FB
ConVar asw_motionblur_time( "asw_motionblur_time", "0.05", FCVAR_CHEAT, "The amount of time to wait until updating the FB" );	// Delay to add between capturing the FB
ConVar asw_motionblur_forceupdate( "asw_motionblur_forceupdate", "0", FCVAR_NONE, "update the motion blur buffer even if it's not being displayed" );
ConVar asw_night_vision_self_illum_multiplier( "asw_night_vision_self_illum_multiplier", "25", FCVAR_CHEAT, "For materials that use the NightVision proxy, multiply the result (normally in the [0,1] range) by this value." );
ConVar asw_sniper_scope_self_illum_multiplier( "asw_sniper_scope_self_illum_multiplier", "0.5", FCVAR_CHEAT, "For materials that use the NightVision proxy, multiply the result (normally in the [0,1] range) by this value." );

// @TODO: move this parameter to an entity property rather than convar
ConVar mat_dest_alpha_range( "mat_dest_alpha_range", "1000", 0, "Amount to scale depth values before writing into destination alpha ([0,1] range)." );

PRECACHE_REGISTER_BEGIN( GLOBAL, ASWPrecacheViewRender )
	PRECACHE( MATERIAL, "swarm/effects/frontbuffer" )
	PRECACHE( MATERIAL, "effects/nightvision" )
	PRECACHE( MATERIAL, "effects/nightvision_flash" )
	PRECACHE( MATERIAL, "effects/nightvision_noise" )
	PRECACHE( MATERIAL, "effects/object_motion_blur" )
PRECACHE_REGISTER_END()

static CASWViewRender g_ViewRender;

IViewRender *GetViewRenderInstance()
{
	return &g_ViewRender;
}

CASWViewRender::CASWViewRender()
{
	
}


void CASWViewRender::OnRenderStart()
{
	CViewRender::OnRenderStart();
	CMatRenderContextPtr pRenderContext( materials );

	pRenderContext->SetFloatRenderingParameter( FLOAT_RENDERPARM_DEST_ALPHA_DEPTH_SCALE, mat_dest_alpha_range.GetFloat() );
}

//-----------------------------------------------------------------------------
// Purpose: Renders extra 2D effects in derived classes while the 2D view is on the stack
//-----------------------------------------------------------------------------
void CASWViewRender::Render2DEffectsPreHUD( const CViewSetup &viewsetup )
{
	PerformNightVisionEffect( viewsetup );	// this needs to come before the HUD is drawn, or it will wash the HUD out
#ifndef _X360
	// @TODO: Motion blur not supported on X360 yet due to EDRAM issues
	DoMotionBlur( viewsetup );
#endif
}


void CASWViewRender::DoMotionBlur( const CViewSetup &viewsetup )
{
	bool bShouldDraw = asw_motionblur.GetBool() || g_fMarinePoisonDuration > 0;
	if ( !bShouldDraw && !asw_motionblur_forceupdate.GetBool() )
	{
		g_bBlurredLastTime = false;
		return;
	}

	bool found;
	IMaterialVar *mv = NULL;
	IMaterial *pMatScreen = NULL;
	ITexture *pMotionBlur = NULL;
	ITexture *pOriginalTexture = NULL;

	// Get the front buffer material
	pMatScreen = materials->FindMaterial( "swarm/effects/frontbuffer", TEXTURE_GROUP_OTHER, true );
	Assert( pMatScreen );
	// Get our custom render target
	pMotionBlur = g_pASWRenderTargets->GetASWMotionBlurTexture();
	Assert( pMotionBlur );
	// Store the current render target
	CMatRenderContextPtr pRenderContext( materials );
	ITexture *pOriginalRenderTarget = pRenderContext->GetRenderTarget();
	Assert( !pOriginalRenderTarget );

	// Set the camera up so we can draw the overlay
	int oldX, oldY, oldW, oldH;
	pRenderContext->GetViewport( oldX, oldY, oldW, oldH );

	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->PushMatrix();
	pRenderContext->LoadIdentity();

	// set our blur parameters, based on convars or the poison duration
	float add_alpha = asw_motionblur_addalpha.GetFloat();
	float blur_time = asw_motionblur_time.GetFloat();
	float draw_alpha = asw_motionblur_drawalpha.GetFloat();
	if ( g_fMarinePoisonDuration > 0 )
	{
		if ( g_fMarinePoisonDuration < 1.0f )
		{
			draw_alpha = g_fMarinePoisonDuration;
			add_alpha = 0.3f;
		}
		else
		{
			draw_alpha = 1.0f;
			float over_time = g_fMarinePoisonDuration - 1.0f;
			over_time = -MIN( 4.0f, over_time );
			// map 0 to -4, to 0.3 to 0.05
			add_alpha = ( over_time + 4 ) * 0.0625 + 0.05f;
		}
		blur_time = 0.05f;
	}
	if ( !g_bBlurredLastTime )
		add_alpha = 1.0f;	// add the whole buffer if this is the first time we're blurring after a while, so we don't end up with images from ages ago

	if ( fNextDrawTime - gpGlobals->curtime > 1.0f )
	{
		fNextDrawTime = 0.0f;
	}

	if ( gpGlobals->curtime >= fNextDrawTime )
	{
		UpdateScreenEffectTexture( 0, viewsetup.x, viewsetup.y, viewsetup.width, viewsetup.height );

		// Set the alpha to whatever our console variable is
		mv = pMatScreen->FindVar( "$alpha", &found, true );
		Assert( found );
		if ( found )
		{
			if ( fNextDrawTime == 0 )
			{
				mv->SetFloatValue( 1.0f );
			}
			else
			{
				mv->SetFloatValue( add_alpha );
			}
		}

		pRenderContext->SetRenderTarget( pMotionBlur );
		pRenderContext->DrawScreenSpaceQuad( pMatScreen );

		// Set the next draw time according to the convar
		fNextDrawTime = gpGlobals->curtime + blur_time;
	}

	// Set the alpha
	mv = pMatScreen->FindVar( "$alpha", &found, true );
	Assert( found );
	if ( found )
	{
		mv->SetFloatValue( draw_alpha );
	}

	// Set the texture to our buffer
	mv = pMatScreen->FindVar( "$basetexture", &found, true );
	Assert( found );
	if ( found )
	{
		pOriginalTexture = mv->GetTextureValue();
		AssertMsg1( pOriginalTexture == GetFullFrameFrameBufferTexture( 0 ), "pOriginalTexture is %s", pOriginalTexture->GetName() );
		mv->SetTextureValue( pMotionBlur );
	}

	// Pretend we were never here, set everything back
	pRenderContext->SetRenderTarget( pOriginalRenderTarget );
	if ( bShouldDraw )
	{
		pRenderContext->DrawScreenSpaceQuad( pMatScreen );
	}

	// Set our texture back to _rt_FullFrameFB
	Assert( found );
	if ( found )
	{
		ITexture *pFullFrameFB = GetFullFrameFrameBufferTexture( 0 );
		if ( pOriginalTexture != pFullFrameFB )
		{
			Warning( "Fixing motion blur texture.\n" );
			pOriginalTexture = pFullFrameFB;
		}

		mv->SetTextureValue( pOriginalTexture );
	}

	pRenderContext->DepthRange( 0.0f, 1.0f );
	pRenderContext->MatrixMode( MATERIAL_PROJECTION );
	pRenderContext->PopMatrix();
	pRenderContext->MatrixMode( MATERIAL_VIEW );
	pRenderContext->PopMatrix();

	g_bBlurredLastTime = true;
}

CON_COMMAND( asw_motionblur_check, "check for anomalies in asw_motionblur" )
{
	Msg( "next draw time is in %f sec\n", fNextDrawTime - gpGlobals->curtime );

	IMaterial *pMatScreen = materials->FindMaterial( "swarm/effects/frontbuffer", TEXTURE_GROUP_OTHER, true );
	if ( !pMatScreen )
	{
		Warning( "frontbuffer material missing\n" );
		return;
	}

	ITexture *pMotionBlur = g_pASWRenderTargets->GetASWMotionBlurTexture();
	if ( !pMotionBlur )
	{
		Warning( "motionblur texture missing\n" );
		return;
	}

	CMatRenderContextPtr pRenderContext( materials );
	ITexture *pOriginalRenderTarget = pRenderContext->GetRenderTarget();
	if ( pOriginalRenderTarget )
	{
		Warning( "unexpectedly have render target texture %s\n", pOriginalRenderTarget->GetName() );
		return;
	}

	bool found = false;
	IMaterialVar *mv = pMatScreen->FindVar( "$alpha", &found, true );
	if ( !found )
	{
		Warning( "could not find $alpha in frontbuffer material\n" );
		return;
	}

	mv = pMatScreen->FindVar( "$basetexture", &found, true );
	if ( !found )
	{
		Warning( "could not find $basetexture in frontbuffer material\n" );
		return;
	}

	ITexture *pOriginalTexture = mv->GetTextureValue();
	if ( !pOriginalTexture )
	{
		Warning( "missing original texture in frontbuffer material\n" );
		return;
	}

	if ( pOriginalTexture != GetFullFrameFrameBufferTexture( 0 ) )
	{
		Warning( "original texture in frontbuffer material is %s\n", pOriginalTexture->GetName() );
		return;
	}

	Msg( "no motion blur anomalies found. if motion blur is currently broken, this command needs to be expanded.\n" );
}

inline bool ASW_SetMaterialVarFloat( IMaterial* pMat, const char* pVarName, float flValue )
{
	Assert( pMat != NULL );
	Assert( pVarName != NULL );
	if ( pMat == NULL || pVarName == NULL )
	{
		return false;
	}

	bool bFound = false;
	IMaterialVar* pVar = pMat->FindVar( pVarName, &bFound );
	if ( bFound )
	{
		pVar->SetFloatValue( flValue );
	}

	return bFound;
}

inline bool ASW_SetMaterialVarInt( IMaterial* pMat, const char* pVarName, int iValue )
{
	Assert( pMat != NULL );
	Assert( pVarName != NULL );
	if ( pMat == NULL || pVarName == NULL )
	{
		return false;
	}

	bool bFound = false;
	IMaterialVar* pVar = pMat->FindVar( pVarName, &bFound );
	if ( bFound )
	{
		pVar->SetIntValue( iValue );
	}

	return bFound;
}

inline bool ASW_SetMaterialVarVector4D( IMaterial* pMat, const char* pVarName, const Vector4D &vValue )
{
	Assert( pMat != NULL );
	Assert( pVarName != NULL );
	if ( pMat == NULL || pVarName == NULL )
	{
		return false;
	}

	bool bFound = false;
	IMaterialVar* pVar = pMat->FindVar( pVarName, &bFound );
	if ( bFound )
	{
		pVar->SetVecValue( vValue.Base(), 4 );
	}

	return bFound;
}

// Material proxy for changing a texture based on the user's language preference.
// Example:
// UnlitGeneric {
//     $basetexture "example/translated_texture_english"
//
//     Proxies {
//         LanguagePreference {
//             default "example/translated_texture_english"
//             schinese "example/translated_texture_schinese"
//             french "example/translated_texture_french"
//             resultvar "$basetexture"
//         }
//     }
// }
class CLanguagePreferenceProxy : public CResultProxy
{
public:
	bool Init( IMaterial *pMaterial, KeyValues *pKeyValues ) override;
	void OnBind( void *pC_BaseEntity ) override;

	ITexture *m_pTexture{};
};

bool CLanguagePreferenceProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if ( !CResultProxy::Init( pMaterial, pKeyValues ) )
	{
		return false;
	}

	const char *szTextureName = pKeyValues->GetString( "default", NULL );
	if ( SteamApps() )
	{
		szTextureName = pKeyValues->GetString( SteamApps()->GetCurrentGameLanguage(), szTextureName);
	}

	if ( !szTextureName )
	{
		Warning( "No default or language-specific texture for LanguagePreference proxy in %s\n", pMaterial->GetName() );
		return false;
	}

	m_pTexture = materials->FindTexture( szTextureName, pMaterial->GetTextureGroupName() );

	return !IsErrorTexture( m_pTexture );
}

void CLanguagePreferenceProxy::OnBind( void *pC_BaseEntity )
{
	Assert( m_pResult && m_pTexture );
	Assert( m_pResult->GetType() == MATERIAL_VAR_TYPE_TEXTURE );

	m_pResult->SetTextureValue( m_pTexture );
}

EXPOSE_MATERIAL_PROXY( CLanguagePreferenceProxy, LanguagePreference );

// Set to true by the client mode when rendering glows, false when done
bool g_bRenderingGlows;

//-----------------------------------------------------------------------------
// Material proxy for getting the strength of the self-illum effect to 
// apply to objects when night vision is enabled (value is always 0 when
// the effect is disabled)
//-----------------------------------------------------------------------------
class CASWNightVisionSelfIllumProxy : public CResultProxy
{
public:
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
};


bool CASWNightVisionSelfIllumProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	if ( !CResultProxy::Init( pMaterial, pKeyValues ) )
		return false;

	return true;
}

void CASWNightVisionSelfIllumProxy::OnBind( void *pC_BaseEntity )
{
	Assert( m_pResult );
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pPlayer || !pC_BaseEntity )
	{
		SetFloatResult( 0.0f );
		return;
	}

	C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pPlayer->GetViewNPC() );
	if ( !pMarine )
	{
		SetFloatResult( 0.0f );
		return;
	}

	if ( pMarine->m_flVisionAlpha > 0 )
	{
		SetFloatResult( pMarine->m_flVisionAlpha / 255.0f * asw_night_vision_self_illum_multiplier.GetFloat() );
		return;
	}

	if ( pPlayer->IsSniperScopeActive() && g_bRenderingGlows )
	{
		SetFloatResult( asw_sniper_scope_self_illum_multiplier.GetFloat() );
		return;
	}

	SetFloatResult( 0.0f );
}

EXPOSE_MATERIAL_PROXY( CASWNightVisionSelfIllumProxy, NightVisionSelfIllum );


void CASWViewRender::PerformNightVisionEffect( const CViewSetup &view )
{
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pPlayer )
		return;

	C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pPlayer->GetViewNPC() );
	if ( !pMarine )
		return;

	float flVisionAlpha = pMarine->UpdateVisionAlpha();
	float flFlashAlpha = pMarine->UpdateFlashAlpha();

	if ( flVisionAlpha > 0 )
	{
		IMaterial *pMaterial = materials->FindMaterial( "effects/nightvision", TEXTURE_GROUP_CLIENT_EFFECTS, true );

		if ( pMaterial )
		{
			byte overlaycolor[4] = { 0, 255, 0, 255 };

			UpdateScreenEffectTexture( 0, view.x, view.y, view.width, view.height );

			overlaycolor[3] = flVisionAlpha;

			render->ViewDrawFade( overlaycolor, pMaterial );

			CMatRenderContextPtr pRenderContext( materials );
			pRenderContext->DrawScreenSpaceQuad( pMaterial );
			render->ViewDrawFade( overlaycolor, pMaterial );
			pRenderContext->DrawScreenSpaceQuad( pMaterial );
		}
		IMaterial *pNoiseMaterial = materials->FindMaterial( "effects/nightvision_noise", TEXTURE_GROUP_CLIENT_EFFECTS, true );

		if ( pNoiseMaterial )
		{
			byte overlaycolor[4] = { 255, 255, 255, 255 };
			overlaycolor[3] = MAX( flFlashAlpha, 16.0f );
			CMatRenderContextPtr pRenderContext( materials );
			render->ViewDrawFade( overlaycolor, pNoiseMaterial );
		}
	}
	if ( flFlashAlpha > 0 )
	{
		IMaterial *pMaterial = materials->FindMaterial( "effects/nightvision_flash", TEXTURE_GROUP_CLIENT_EFFECTS, true );

		if ( pMaterial )
		{
			byte overlaycolor[4] = { 255, 255, 255, 255 };
			overlaycolor[3] = flFlashAlpha;
			CMatRenderContextPtr pRenderContext( materials );
			render->ViewDrawFade( overlaycolor, pMaterial );
		}		
	}
}
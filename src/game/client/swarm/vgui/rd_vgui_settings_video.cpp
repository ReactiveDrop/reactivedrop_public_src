#include "cbase.h"
#include "rd_vgui_settings.h"
#include "videocfg/videocfg.h"
#include "materialsystem/materialsystem_config.h"
#include "IGameUIFuncs.h"
#include "modes.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static class VideoConfigVariableListHack_t
{
public:
	VideoConfigVariableListHack_t()
	{
		// videocfg.lib is statically linked, but the variable we need is marked as static, so we can't reference it using an extern.
		// Instead, we have to do this garbage:
		const byte *pUpdateCurrentVideoConfig = reinterpret_cast< const byte * >( &UpdateCurrentVideoConfig );
#ifdef _DEBUG
		// Gotta deal with thunks on debug builds.
		Assert( *pUpdateCurrentVideoConfig == 0xE9 );
		pUpdateCurrentVideoConfig += 5 + *reinterpret_cast< const intptr_t * >( pUpdateCurrentVideoConfig + 1 );
#endif
		struct VideoConfigSetting_t
		{
			const char *m_szName;
			bool m_bUseMin;
			bool m_bSave;
			bool m_bIsConVar;
			bool m_bAuto;
		} *s_pVideoConfigSettingsWhitelist = *reinterpret_cast< VideoConfigSetting_t *const * >( pUpdateCurrentVideoConfig + 51 );

		// We can just barely fit our new variables into the videocfg.lib array due to unused names:
		// [0] setting.cpu_level
		// [1] setting.gpu_level
		// [2] setting.mat_antialias
		// [3] setting.mat_aaquality
		// [4] setting.mat_forceaniso
		// [5] setting.mat_vsync
		// [6] setting.mat_triplebuffered
		// [7] setting.mat_grain_scale_override
		// [8] setting.mat_monitorgamma
		// [9] setting.gpu_mem_level
		// [10] setting.mem_level
		// [11] setting.videoconfig_version
		// [12] setting.mat_queue_mode
		// [13] setting.fullscreen
		// [14] setting.nowindowborder
		// [15] setting.aspectratiomode
		// [16] setting.defaultres
		// [17] setting.defaultresheight
		// [18] setting.dxlevel (UNUSED)
		// [19] setting.mindxlevel (UNUSED)
		// [20] setting.maxdxlevel
		// [21] setting.preferhardwaresync (UNUSED)
		// [22] setting.centroidhack (UNUSED)
		// [23] setting.preferzprepass (UNUSED)
		// [24] setting.prefertexturesinhwmemory (UNUSED)
		// [25] setting.laptop (UNUSED)
		// [26] setting.suppresspixelshadercentroidhackfixup (UNUSED)
		// [27] setting.nouserclipplanes (UNUSED)
		//
		// This is a total of 9 unused keys, and we have:
		// 0. mat_local_contrast_enable
		// 1. rd_func_precipitation_enable
		// 2. mat_bloom_scalefactor_scalar
		// 3. rd_env_projectedtexture_enabled
		// 4. rd_flashlightshadows
		// 5. rd_flashlight_dlight_enable
		// 7. rd_simple_beacons
		// 8. muzzleflash_light
		//
		// This leaves us with three video config settings that are stored per-user rather than per-machine:
		// - asw_alien_shadows
		// - asw_directional_shadows
		// - rd_health_effect

		// But before we get to that: videocfg.lib was built with setting.mat_grain_scale_override set to 1 for enabled, but we use -1.
		Assert( !V_strcmp( s_pVideoConfigSettingsWhitelist[7].m_szName, "setting.mat_grain_scale_override" ) );
		s_pVideoConfigSettingsWhitelist[7].m_bUseMin = false;

		// Now we go through and replace the unused keys with our keys.
		Assert( !V_strcmp( s_pVideoConfigSettingsWhitelist[18].m_szName, "setting.dxlevel" ) );
		s_pVideoConfigSettingsWhitelist[18] = VideoConfigSetting_t{ "setting.mat_local_contrast_enable", true, true, true, false };
		Assert( !V_strcmp( s_pVideoConfigSettingsWhitelist[19].m_szName, "setting.mindxlevel" ) );
		s_pVideoConfigSettingsWhitelist[19] = VideoConfigSetting_t{ "setting.rd_func_precipitation_enable", true, true, true, false };
		Assert( !V_strcmp( s_pVideoConfigSettingsWhitelist[21].m_szName, "setting.preferhardwaresync" ) );
		s_pVideoConfigSettingsWhitelist[21] = VideoConfigSetting_t{ "setting.mat_bloom_scalefactor_scalar", true, true, true, false };
		Assert( !V_strcmp( s_pVideoConfigSettingsWhitelist[22].m_szName, "setting.centroidhack" ) );
		s_pVideoConfigSettingsWhitelist[22] = VideoConfigSetting_t{ "setting.", true, true, true, false };
		Assert( !V_strcmp( s_pVideoConfigSettingsWhitelist[23].m_szName, "setting.preferzprepass" ) );
		s_pVideoConfigSettingsWhitelist[23] = VideoConfigSetting_t{ "setting.rd_env_projectedtexture_enabled", true, true, true, false };
		Assert( !V_strcmp( s_pVideoConfigSettingsWhitelist[24].m_szName, "setting.prefertexturesinhwmemory" ) );
		s_pVideoConfigSettingsWhitelist[24] = VideoConfigSetting_t{ "setting.rd_flashlightshadows", true, true, true, false };
		Assert( !V_strcmp( s_pVideoConfigSettingsWhitelist[25].m_szName, "setting.laptop" ) );
		s_pVideoConfigSettingsWhitelist[25] = VideoConfigSetting_t{ "setting.rd_flashlight_dlight_enable", true, true, true, false };
		Assert( !V_strcmp( s_pVideoConfigSettingsWhitelist[26].m_szName, "setting.suppresspixelshadercentroidhackfixup" ) );
		s_pVideoConfigSettingsWhitelist[26] = VideoConfigSetting_t{ "setting.rd_simple_beacons", false, true, true, false };
		Assert( !V_strcmp( s_pVideoConfigSettingsWhitelist[27].m_szName, "setting.nouserclipplanes" ) );
		s_pVideoConfigSettingsWhitelist[27] = VideoConfigSetting_t{ "setting.muzzleflash_light", true, true, true, false };
	}
} s_VideoConfigVariableListHack;

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Video );

const static struct AAMode_t
{
	const char *m_szLabel;
	uint8_t m_nNumSamples;
	uint8_t m_nQualityLevel;
} s_AAModes[] =
{
	{ "#GameUI_None",      1, 0 },
	{ "#GameUI_2X",        2, 0 },
	{ "#GameUI_4X",        4, 0 },
	{ "#GameUI_6X",        6, 0 },
	{ "#GameUI_8X_CSAA",   4, 2 },
	{ "#GameUI_16X_CSAA",  4, 4 },
	{ "#GameUI_8X",        8, 0 },
	{ "#GameUI_16XQ_CSAA", 8, 2 },
};

const static vmode_t s_pWindowedModes[] =
{
	// NOTE: These must be sorted by ascending width, then ascending height
	{ 640, 480, 32, 60 },
	{ 852, 480, 32, 60 },
	{ 1280, 720, 32, 60 },
	{ 1920, 822, 32, 60 }, // for testing ultra-wide on a 16:9 or 16:10 widescreen monitor
	{ 1920, 1080, 32, 60 },
};

static void GenerateWindowedModes( CUtlVector< vmode_t > &windowedModes, int nCount, vmode_t *pFullscreenModes )
{
	int nFSMode = 0;
	for ( int i = 0; i < ARRAYSIZE( s_pWindowedModes ); ++i )
	{
		while ( true )
		{
			if ( nFSMode >= nCount )
				break;

			if ( pFullscreenModes[nFSMode].width > s_pWindowedModes[i].width )
				break;

			if ( pFullscreenModes[nFSMode].width == s_pWindowedModes[i].width )
			{
				if ( pFullscreenModes[nFSMode].height > s_pWindowedModes[i].height )
					break;

				if ( pFullscreenModes[nFSMode].height == s_pWindowedModes[i].height )
				{
					// Don't add the matching fullscreen mode
					++nFSMode;
					break;
				}
			}

			windowedModes.AddToTail( pFullscreenModes[nFSMode] );
			++nFSMode;
		}

		windowedModes.AddToTail( s_pWindowedModes[i] );
	}

	for ( ; nFSMode < nCount; ++nFSMode )
	{
		windowedModes.AddToTail( pFullscreenModes[nFSMode] );
	}
}

CRD_VGUI_Settings_Video::CRD_VGUI_Settings_Video( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	m_pSettingScreenResolution = new CRD_VGUI_Option( this, "SettingScreenResolution", "#rd_video_screen_resolution", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingDisplayMode = new CRD_VGUI_Option( this, "SettingDisplayMode", "#rd_video_display_mode", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingDisplayMode->AddOption( 0, "#rd_video_display_mode_fullscreen", "#rd_video_display_mode_fullscreen_hint" );
	m_pSettingDisplayMode->AddOption( 1, "#rd_video_display_mode_noborder", "#rd_video_display_mode_noborder_hint" );
	m_pSettingDisplayMode->AddOption( 2, "#rd_video_display_mode_windowed", "#rd_video_display_mode_windowed_hint" );
	m_pSettingRenderingPipeline = new CRD_VGUI_Option( this, "SettingRenderingPipeline", "#rd_video_rendering_pipeline", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingRenderingPipeline->AddOption( 0, "#rd_video_rendering_pipeline_single", "#rd_video_rendering_pipeline_hint" );
	m_pSettingRenderingPipeline->AddOption( -1, "#rd_video_rendering_pipeline_multi", "#rd_video_rendering_pipeline_hint" );
	m_pSettingRenderingPipeline->SetDefaultHint( "#rd_video_rendering_pipeline_hint" );
	m_pSettingVSync = new CRD_VGUI_Option( this, "SettingVSync", "#rd_video_vsync", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingVSync->AddOption( 0, "#rd_video_vsync_single", "#rd_video_vsync_single_hint" );
	m_pSettingVSync->AddOption( 1, "#rd_video_vsync_double", "#rd_video_vsync_double_hint" );
	m_pSettingVSync->AddOption( 2, "#rd_video_vsync_triple", "#rd_video_vsync_triple_hint" );

	m_pSettingEffectDetail = new CRD_VGUI_Option( this, "SettingEffectDetail", "#rd_video_effect_detail" );
	m_pSettingEffectDetail->AddOption( 0, "#rd_video_level_low", "#rd_video_effect_detail_hint" );
	m_pSettingEffectDetail->AddOption( 1, "#rd_video_level_medium", "#rd_video_effect_detail_hint" );
	m_pSettingEffectDetail->AddOption( 2, "#rd_video_level_high", "#rd_video_effect_detail_hint" );
	m_pSettingEffectDetail->SetDefaultHint( "#rd_video_effect_detail_hint" );
	m_pSettingShaderDetail = new CRD_VGUI_Option( this, "SettingShaderDetail", "#rd_video_shader_detail" );
	m_pSettingShaderDetail->AddOption( 0, "#rd_video_level_low", "#rd_video_shader_detail_hint" );
	m_pSettingShaderDetail->AddOption( 1, "#rd_video_level_medium", "#rd_video_shader_detail_hint" );
	m_pSettingShaderDetail->AddOption( 2, "#rd_video_level_high", "#rd_video_shader_detail_hint" );
	m_pSettingShaderDetail->AddOption( 3, "#rd_video_level_ultra", "#rd_video_shader_detail_hint" );
	m_pSettingShaderDetail->SetDefaultHint( "#rd_video_shader_detail_hint" );
	m_pSettingTextureDetail = new CRD_VGUI_Option( this, "SettingTextureDetail", "#rd_video_texture_detail" );
	m_pSettingTextureDetail->AddOption( 0, "#rd_video_level_low", "#rd_video_texture_detail_hint" );
	m_pSettingTextureDetail->AddOption( 1, "#rd_video_level_medium", "#rd_video_texture_detail_hint" );
	m_pSettingTextureDetail->AddOption( 2, "#rd_video_level_high", "#rd_video_texture_detail_hint" );
	m_pSettingTextureDetail->AddOption( 3, "#rd_video_level_ultra", "#rd_video_texture_detail_hint" );
	m_pSettingTextureDetail->SetDefaultHint( "#rd_video_texture_detail_hint" );
	m_pSettingAntiAliasing = new CRD_VGUI_Option( this, "SettingAntiAliasing", "#rd_video_anti_aliasing" );
	m_pSettingAntiAliasing->SetDefaultHint( "#rd_video_anti_aliasing_hint" );
	m_pSettingFiltering = new CRD_VGUI_Option( this, "SettingFiltering", "#rd_video_filtering" );
	m_pSettingFiltering->AddOption( 0, "#GameUI_Bilinear", "#rd_video_filtering_hint" );
	m_pSettingFiltering->AddOption( 1, "#GameUI_Trilinear", "#rd_video_filtering_hint" );
	m_pSettingFiltering->AddOption( 2, "#GameUI_Anisotropic2X", "#rd_video_filtering_hint" );
	m_pSettingFiltering->AddOption( 4, "#GameUI_Anisotropic4X", "#rd_video_filtering_hint" );
	m_pSettingFiltering->AddOption( 8, "#GameUI_Anisotropic8X", "#rd_video_filtering_hint" );
	m_pSettingFiltering->AddOption( 16, "#GameUI_Anisotropic16X", "#rd_video_filtering_hint" );
	m_pSettingFiltering->SetDefaultHint( "#rd_video_filtering_hint" );

	m_pSettingFilmGrain = new CRD_VGUI_Option( this, "SettingFilmGrain", "#rd_video_film_grain" );
	m_pSettingFilmGrain->AddOption( 0, "#rd_video_effect_disabled", "#rd_video_film_grain_hint" );
	m_pSettingFilmGrain->AddOption( -1, "#rd_video_effect_enabled", "#rd_video_film_grain_hint" );
	m_pSettingFilmGrain->SetDefaultHint( "#rd_video_film_grain_hint" );
	m_pSettingLocalContrast = new CRD_VGUI_Option( this, "SettingLocalContrast", "#rd_video_local_contrast" );
	m_pSettingLocalContrast->AddOption( 0, "#rd_video_effect_disabled", "#rd_video_local_contrast_hint" );
	m_pSettingLocalContrast->AddOption( 1, "#rd_video_effect_enabled", "#rd_video_local_contrast_hint" );
	m_pSettingLocalContrast->SetDefaultHint( "#rd_video_local_contrast_hint" );
	m_pSettingDepthBlur = new CRD_VGUI_Option( this, "SettingDepthBlur", "#rd_video_depth_blur" );
	m_pSettingDepthBlur->AddOption( 0, "#rd_video_effect_disabled", "#rd_video_depth_blur_hint" );
	m_pSettingDepthBlur->AddOption( -1, "#rd_video_effect_enabled", "#rd_video_depth_blur_hint" );
	m_pSettingDepthBlur->SetDefaultHint( "#rd_video_depth_blur_hint" );
	m_pSettingWeatherEffects = new CRD_VGUI_Option( this, "SettingWeatherEffects", "#rd_video_weather_effects" );
	m_pSettingWeatherEffects->AddOption( 0, "#rd_video_effect_disabled", "#rd_video_weather_effects_hint" );
	m_pSettingWeatherEffects->AddOption( 1, "#rd_video_effect_enabled", "#rd_video_weather_effects_hint" );
	m_pSettingWeatherEffects->SetDefaultHint( "#rd_video_weather_effects_hint" );
	m_pSettingBloomScale = new CRD_VGUI_Option( this, "SettingBloomScale", "#rd_video_bloom_scale", CRD_VGUI_Option::MODE_SLIDER );
	m_pSettingBloomScale->SetSliderMinMax( 0.0f, 2.0f );
	m_pSettingBloomScale->SetDefaultHint( "#rd_video_bloom_scale_hint" );
	m_pSettingProjectedTextures = new CRD_VGUI_Option( this, "SettingProjectedTextures", "#rd_video_projected_textures" );
	m_pSettingProjectedTextures->AddOption( 0, "#rd_video_effect_disabled", "#rd_video_projected_textures_hint" );
	m_pSettingProjectedTextures->AddOption( 1, "#rd_video_effect_enabled", "#rd_video_projected_textures_hint" );
	m_pSettingProjectedTextures->SetDefaultHint( "#rd_video_projected_textures_hint" );
	m_pSettingFlashlightShadows = new CRD_VGUI_Option( this, "SettingFlashlightShadows", "#rd_video_flashlight_shadows" );
	m_pSettingFlashlightShadows->AddOption( 0, "#rd_video_effect_disabled", "#rd_video_flashlight_shadows_hint" );
	m_pSettingFlashlightShadows->AddOption( 1, "#rd_video_effect_enabled", "#rd_video_flashlight_shadows_hint" );
	m_pSettingFlashlightShadows->SetDefaultHint( "#rd_video_flashlight_shadows_hint" );
	m_pSettingFlashlightLightSpill = new CRD_VGUI_Option( this, "SettingFlashlightLightSpill", "#rd_video_flashlight_light_spill" );
	m_pSettingFlashlightLightSpill->AddOption( 0, "#rd_video_effect_disabled", "#rd_video_flashlight_light_spill_hint" );
	m_pSettingFlashlightLightSpill->AddOption( 1, "#rd_video_effect_enabled", "#rd_video_flashlight_light_spill_hint" );
	m_pSettingFlashlightLightSpill->SetDefaultHint( "#rd_video_flashlight_light_spill_hint" );
	m_pSettingHighQualityBeacons = new CRD_VGUI_Option( this, "SettingHighQualityBeacons", "#rd_video_high_quality_beacons" );
	m_pSettingHighQualityBeacons->AddOption( 1, "#rd_video_effect_disabled", "#rd_video_high_quality_beacons_hint" );
	m_pSettingHighQualityBeacons->AddOption( 0, "#rd_video_effect_enabled", "#rd_video_high_quality_beacons_hint" );
	m_pSettingHighQualityBeacons->SetDefaultHint( "#rd_video_high_quality_beacons_hint" );
	m_pSettingMuzzleFlashLights = new CRD_VGUI_Option( this, "SettingMuzzleFlashLights", "#rd_video_muzzle_flash_lights" );
	m_pSettingMuzzleFlashLights->AddOption( 0, "#rd_video_effect_disabled", "#rd_video_muzzle_flash_lights_hint" );
	m_pSettingMuzzleFlashLights->AddOption( 1, "#rd_video_effect_enabled", "#rd_video_muzzle_flash_lights_hint" );
	m_pSettingMuzzleFlashLights->SetDefaultHint( "#rd_video_muzzle_flash_lights_hint" );
	m_pSettingAlienShadows = new CRD_VGUI_Option( this, "SettingAlienShadows", "#rd_video_alien_shadows" );
	m_pSettingAlienShadows->AddOption( 0, "#rd_video_effect_disabled", "#rd_video_alien_shadows_hint" );
	m_pSettingAlienShadows->AddOption( 1, "#rd_video_effect_flashlight_only", "#rd_video_alien_shadows_hint" );
	m_pSettingAlienShadows->AddOption( 2, "#rd_video_effect_enabled", "#rd_video_alien_shadows_hint" );
	m_pSettingAlienShadows->SetDefaultHint( "#rd_video_alien_shadows_hint" );
	m_pSettingLowHealthEffect = new CRD_VGUI_Option( this, "SettingLowHealthEffect", "#rd_video_low_health_effect" );
	m_pSettingLowHealthEffect->AddOption( 0, "#rd_video_effect_disabled", "#rd_video_low_health_effect_hint" );
	m_pSettingLowHealthEffect->AddOption( 1, "#rd_video_effect_enabled", "#rd_video_low_health_effect_hint" );
	m_pSettingLowHealthEffect->SetDefaultHint( "#rd_video_low_health_effect_hint" );
}

void CRD_VGUI_Settings_Video::Activate()
{
	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

	KeyValues::AutoDelete pRecommended{ "VideoConfig" };
	bool bHaveRecommended = ReadCurrentVideoConfig( pRecommended, true );

	// Screen Resolution [setting.defaultres / setting.defaultresheight] [drop-down]
	vmode_t *pResolutionList = NULL;
	int nScreenResolutions = 0;
	gameuifuncs->GetVideoModes( &pResolutionList, &nScreenResolutions );

	int iDesktopWidth, iDesktopHeight;
	gameuifuncs->GetDesktopResolution( iDesktopWidth, iDesktopHeight );

	// Add some extra modes in if we're running windowed
	CUtlVector< vmode_t > WindowedModes;
	if ( config.Windowed() )
	{
		GenerateWindowedModes( WindowedModes, nScreenResolutions, pResolutionList );
		nScreenResolutions = WindowedModes.Count();
		pResolutionList = WindowedModes.Base();
	}

	int iScreenWidth, iScreenHeight;
	GetHudSize( iScreenWidth, iScreenHeight );

	m_pSettingScreenResolution->RemoveAllOptions();
	int iCurrentScreenResolution = -1, iRecommendedScreenResolution = -1;
	for ( int i = 0; i < nScreenResolutions; i++ )
	{
		int iPackedResolution = ( pResolutionList[i].width << 16 ) | pResolutionList[i].height;
		char szResolution[64];
		V_snprintf( szResolution, sizeof( szResolution ), "%d x %d", pResolutionList[i].width, pResolutionList[i].height );
		m_pSettingScreenResolution->AddOption( iPackedResolution, szResolution, "" );

		if ( iScreenWidth == pResolutionList[i].width && iScreenHeight == pResolutionList[i].height )
			iCurrentScreenResolution = iPackedResolution;
		if ( iDesktopWidth == pResolutionList[i].width && iDesktopHeight == pResolutionList[i].height )
			iRecommendedScreenResolution = iPackedResolution;
	}
	m_pSettingScreenResolution->SetCurrentAndRecommended( iCurrentScreenResolution, iRecommendedScreenResolution );

	// Display Mode [setting.fullscreen / setting.nowindowborder] [drop-down]
	// Exclusive Full Screen - Takes full control of the monitor. Allows you to play the game full-screen at a resolution different than what your desktop is set to. May cause problems when switching applications on some machines.
	// Borderless Window - Sometimes called "desktop friendly full screen". Allows you to switch applications more easily while still covering the entire monitor. Can be set to a smaller resolution than your monitor.
	// Window - Standard operating system window with a title bar. Can be moved and resized. Resizing may cause issues.
	int iRecommendedFullScreen = bHaveRecommended ? pRecommended->GetInt( "setting.fullscreen", 0 ) : -1;
	int iRecommendedNoBorder = bHaveRecommended ? pRecommended->GetInt( "setting.nowindowborder", 1 ) : -1;
	int iRecommendedDisplayMode = iRecommendedFullScreen != -1 ? iRecommendedFullScreen == 0 ? iRecommendedNoBorder ? 1 : 2 : 0 : -1;
	int iCurrentDisplayMode = config.Windowed() ? config.NoWindowBorder() ? 1 : 2 : 0;
	m_pSettingDisplayMode->SetCurrentAndRecommended( iCurrentDisplayMode, iRecommendedDisplayMode );

	// Rendering Pipeline [setting.mat_queue_mode] [radio buttons]
	// Single-Threaded (Compatibility)	Multi-Threaded (Fastest)
	// Does not affect graphical quality. It is recommended that you use Multi-Threaded rendering unless it causes problems on your hardware.
	int iRecommendedRenderingPipeline = bHaveRecommended ? pRecommended->GetInt( "setting.mat_queue_mode", -1 ) : -2;
	if ( GetCPUInformation().m_nPhysicalProcessors <= 1 )
		iRecommendedRenderingPipeline = 0;
	ConVarRef mat_queue_mode{ "mat_queue_mode" };
	m_pSettingRenderingPipeline->SetCurrentAndRecommended( mat_queue_mode.GetInt(), iRecommendedRenderingPipeline );
	m_pSettingRenderingPipeline->SetEnabled( GetCPUInformation().m_nPhysicalProcessors > 1 );

	// V-Sync [setting.mat_vsync / setting.mat_triplebuffered]
	// Unsynchronized - Render frames as fast as possible, limited to your monitor's refresh rate. May cause screen tearing on some graphics set-ups.
	// Double Buffering - Synchronizes frame rate with your monitor's ability to display frames. May reduce frame rate if your computer cannot render frames fast enough.
	// Triple Buffering - Smooth out frame rate by rendering two frames ahead. Slightly increases input latency.
	int iRecommendedVSyncEnabled = bHaveRecommended ? pRecommended->GetInt( "setting.mat_vsync", 1 ) : -1;
	int iRecommendedTripleBuffering = bHaveRecommended ? pRecommended->GetInt( "setting.mat_triplebuffered", 0 ) : -1;
	int iRecommendedVSync = iRecommendedVSyncEnabled != -1 ? iRecommendedVSyncEnabled != 0 ? iRecommendedTripleBuffering ? 2 : 1 : 0 : -1;
	int iCurrentVSync = config.WaitForVSync() ? config.m_bWantTripleBuffered ? 2 : 1 : 0;
	m_pSettingVSync->SetCurrentAndRecommended( iCurrentVSync, iRecommendedVSync );

	// Faster < - - - - > Higher Quality

	// Effect Detail [setting.cpu_level]
	// [0] Low [1] Medium [2] High
	// Affected by CPU (processor) performance. Affects impact effects, physics-controlled debris animations, and precipitation.
	int iRecommendedCPULevel = bHaveRecommended ? pRecommended->GetInt( "setting.cpu_level", -1 ) : -1;
	m_pSettingEffectDetail->SetCurrentAndRecommended( GetCPULevel(), iRecommendedCPULevel );

	// Shader Detail [setting.gpu_level]
	// [0] Low [1] Medium [2] High [3] Ultra
	// Affected by GPU (graphics card) performance. Affects lighting and material detail, fog, stains, and bullet holes.
	int iRecommendedGPULevel = bHaveRecommended ? pRecommended->GetInt( "setting.gpu_level", -1 ) : -1;
	m_pSettingShaderDetail->SetCurrentAndRecommended( GetGPULevel(), iRecommendedGPULevel );

	// Texture Detail [setting.gpu_mem_level]
	// [0] Low [1] Medium [2] High [3] Ultra
	// Affected by GPU (graphics card) memory capacity and speed. Affects sharpness of textures.
	int iRecommendedGPUMemLevel = bHaveRecommended ? pRecommended->GetInt( "setting.gpu_mem_level", -1 ) : -1;
	m_pSettingTextureDetail->SetCurrentAndRecommended( GetGPUMemLevel(), iRecommendedGPUMemLevel );

	// We no longer use mem_level to change any settings. Instead, we always set
	// mem_level to the recommended value so anything that wants an approximate
	// memory size (such as the ConVar material proxy) can access it.
	int iRecommendedMemLevel = bHaveRecommended ? pRecommended->GetInt( "setting.mem_level", -1 ) : -1;
	if ( iRecommendedMemLevel >= 0 )
	{
		ConVarRef mem_level{ "mem_level" };
		// avoid running on-change functions if we're not changing it.
		if ( mem_level.GetInt() != iRecommendedMemLevel )
			mem_level.SetValue( iRecommendedMemLevel );
	}

	// Anti-Aliasing [setting.mat_antialias (MSAA) / setting.mat_aaquality (CSAA)]
	// None	2x	4x	6x	8x	16x	8x	16xQ [visible options determined by GPU]
	// Smooths out jagged edges on objects. Available options depend on your graphics card.
	int iCurrentAAMode = -1, iRecommendedAAMode = -1;
	int iRecommendedAASamples = bHaveRecommended ? pRecommended->GetInt( "setting.mat_antialias", 1 ) : -1;
	int iRecommendedAAQuality = bHaveRecommended ? pRecommended->GetInt( "setting.mat_aaquality", 0 ) : -1;
	m_pSettingAntiAliasing->RemoveAllOptions();
	for ( int i = 0; i < NELEMS( s_AAModes ); i++ )
	{
		bool bSupported = true;
		if ( s_AAModes[i].m_nQualityLevel )
		{
			bSupported = materials->SupportsCSAAMode( s_AAModes[i].m_nNumSamples, s_AAModes[i].m_nQualityLevel );
		}
		else if ( s_AAModes[i].m_nNumSamples > 1 )
		{
			bSupported = materials->SupportsMSAAMode( s_AAModes[i].m_nNumSamples );
		}

		if ( !bSupported )
		{
			continue;
		}

		m_pSettingAntiAliasing->AddOption( i, s_AAModes[i].m_szLabel, "#rd_video_aa_hint" );

		if ( config.m_nAASamples == s_AAModes[i].m_nNumSamples && config.m_nAAQuality == s_AAModes[i].m_nQualityLevel )
		{
			iCurrentAAMode = i;
		}
		if ( iRecommendedAASamples == s_AAModes[i].m_nNumSamples && iRecommendedAAQuality == s_AAModes[i].m_nQualityLevel )
		{
			iRecommendedAAMode = i;
		}
	}
	m_pSettingAntiAliasing->SetCurrentAndRecommended( iCurrentAAMode, iRecommendedAAMode );

	// Texture Filtering [setting.mat_forceaniso]
	// Bi-linear	Tri-linear	Anisotropic 2x	Anisotropic 4x	Anisotropic 8x	Anisotropic 16x
	// Higher quality filtering makes textures viewed from a shallow angle or from far away clearer.
	int iRecommendedFilteringLevel = bHaveRecommended ? pRecommended->GetInt( "setting.mat_forceaniso", 1 ) : -1;
	m_pSettingFiltering->SetCurrentAndRecommended( config.m_nForceAnisotropicLevel, iRecommendedFilteringLevel );

	// Film Grain [setting.mat_grain_scale_override]
	// [0] Disabled [-1] Enabled
	// Adds a simulated film grain (fuzziness) effect to the screen, controlled by the mission.
	// It is recommended to disable this if you are recording gameplay as it can negatively affect video encoding.
	int iRecommendedFilmGrain = bHaveRecommended ? pRecommended->GetInt( "setting.mat_grain_scale_override", 0 ) : -2;
	ConVarRef mat_grain_scale_override{ "mat_grain_scale_override" };
	m_pSettingFilmGrain->SetCurrentAndRecommended( mat_grain_scale_override.GetInt(), iRecommendedFilmGrain );

	// Local Contrast [setting.mat_local_contrast_enable]
	// [0] Disabled [1] Enabled
	// Controls whether missions can change the sharpness of a scene for dramatic effect.
	int iRecommendedLocalContrast = bHaveRecommended ? pRecommended->GetInt( "setting.mat_local_contrast_enable", 0 ) : -1;
	ConVarRef mat_local_contrast_enable{ "mat_local_contrast_enable" };
	m_pSettingLocalContrast->SetCurrentAndRecommended( mat_local_contrast_enable.GetInt(), iRecommendedLocalContrast );

	// Depth Blur [setting.mat_depth_blur_strength_override]
	// [0] Disabled [-1] Enabled
	// Controls whether missions can make objects far below the marines blurry for a simulated camera focus effect.
	int iRecommendedDepthBlur = bHaveRecommended ? pRecommended->GetInt( "setting.mat_depth_blur_strength_override", 0 ) : -2;
	ConVarRef mat_depth_blur_strength_override{ "mat_depth_blur_strength_override" };
	m_pSettingDepthBlur->SetCurrentAndRecommended( mat_depth_blur_strength_override.GetInt(), iRecommendedDepthBlur );

	// Weather Effects [setting.rd_func_precipitation_enable]
	// [0] Disabled [1] Enabled
	// Controls some rain and snow effects.
	int iRecommendedWeatherEffects = bHaveRecommended ? pRecommended->GetInt( "setting.rd_func_precipitation_enable", 0 ) : -1;
	ConVarRef rd_func_precipitation_enable{ "rd_func_precipitation_enable" };
	m_pSettingWeatherEffects->SetCurrentAndRecommended( rd_func_precipitation_enable.GetInt(), iRecommendedWeatherEffects );

	// Light and Specular Blooms [setting.mat_bloom_scalefactor_scalar] [slider]
	// An effect where brightly glowing objects "spill" light into the surrounding area.
	ConVarRef mat_bloom_scalefactor_scalar{ "mat_bloom_scalefactor_scalar" };
	m_pSettingBloomScale->SetCurrentSliderValue( mat_bloom_scalefactor_scalar.GetFloat() );
	if ( bHaveRecommended )
		m_pSettingBloomScale->SetRecommendedSliderValue( pRecommended->GetFloat( "setting.mat_bloom_scalefactor_scalar", 1.0f ) );
	else
		m_pSettingBloomScale->ClearRecommendedSliderValue();

	// High Quality Dynamic Shadows [setting.rd_env_projectedtexture_enabled]
	// [0] Disabled [1] Enabled
	// Some missions contain stationary or moving area lights that produce dramatic shadows of marines and aliens. If this option is disabled, these lights will be turned off entirely.
	int iRecommendedProjectedTextures = bHaveRecommended ? pRecommended->GetInt( "setting.rd_env_projectedtexture_enabled", 0 ) : -1;
	ConVarRef rd_env_projectedtexture_enabled{ "rd_env_projectedtexture_enabled" };
	m_pSettingProjectedTextures->SetCurrentAndRecommended( rd_env_projectedtexture_enabled.GetInt(), iRecommendedProjectedTextures );

	// Flashlight Dynamic Shadows [setting.rd_flashlightshadows]
	// [0] Disabled [1] Enabled
	// Controls whether the Flashlight Attachment equipment casts high quality dynamic shadows.
	int iRecommendedFlashlightShadows = bHaveRecommended ? pRecommended->GetInt( "setting.rd_flashlightshadows", 0 ) : -1;
	ConVarRef rd_flashlightshadows{ "rd_flashlightshadows" };
	m_pSettingFlashlightShadows->SetCurrentAndRecommended( rd_flashlightshadows.GetInt(), iRecommendedFlashlightShadows );

	// Flashlight Light Spill [setting.rd_flashlight_dlight_enable]
	// [0] Disabled [1] Enabled
	// Controls whether the Flashlight Attachment equipment adds light to the area outside its beam.
	int iRecommendedFlashlightLightSpill = bHaveRecommended ? pRecommended->GetInt( "setting.rd_flashlight_dlight_enable", 0 ) : -1;
	ConVarRef rd_flashlight_dlight_enable{ "rd_flashlight_dlight_enable" };
	m_pSettingFlashlightLightSpill->SetCurrentAndRecommended( rd_flashlight_dlight_enable.GetInt(), iRecommendedFlashlightLightSpill );

	// High-Quality Beacons [setting.rd_simple_beacons]
	// [1] Disabled [0] Enabled
	// Controls whether items such as the IAF Heal Beacon and X-33 Damage Amplifier cause light distortion, glowing, and pulsing effects.
	int iRecommendedHighQualityBeacons = bHaveRecommended ? pRecommended->GetInt( "setting.rd_simple_beacons", 1 ) : -1;
	ConVarRef rd_simple_beacons{ "rd_simple_beacons" };
	m_pSettingHighQualityBeacons->SetCurrentAndRecommended( rd_simple_beacons.GetInt(), iRecommendedHighQualityBeacons );

	// Muzzle Flash Lights [setting.muzzleflash_light]
	// [0] Disabled [1] Enabled
	// Controls whether guns light up nearby surfaces and objects when fired.
	int iRecommendedMuzzleFlashLights = bHaveRecommended ? pRecommended->GetInt( "setting.muzzleflash_light", 0 ) : -1;
	ConVarRef muzzleflash_light{ "muzzleflash_light" };
	m_pSettingMuzzleFlashLights->SetCurrentAndRecommended( muzzleflash_light.GetInt(), iRecommendedMuzzleFlashLights );

	// Alien Shadows [convar asw_alien_shadows / convar asw_directional_shadows]
	// [0/0] Disabled [0/1] Flashlight Only [1/1] Enabled
	// Controls whether aliens have shadows. Warning: Setting this to "Enabled" may have a large effect on the frame rate of the game.
	ConVarRef asw_alien_shadows{ "asw_alien_shadows" }, asw_directional_shadows{ "asw_directional_shadows" };
	int iRecommendedAlienShadows = V_atoi( asw_alien_shadows.GetDefault() ) ? 2 : V_atoi( asw_directional_shadows.GetDefault() ) ? 1 : 0;
	int iCurrentAlienShadows = asw_alien_shadows.GetBool() ? 2 : asw_directional_shadows.GetBool() ? 1 : 0;
	m_pSettingAlienShadows->SetCurrentAndRecommended( iCurrentAlienShadows, iRecommendedAlienShadows );

	// Low Health Vignetting [convar rd_health_effect]
	// [0] Disabled [1] Enabled
	// Controls whether the edges of the screen glow red when the marine is at low health.
	ConVarRef rd_health_effect{ "rd_health_effect" };
	m_pSettingLowHealthEffect->SetCurrentAndRecommended( rd_health_effect.GetInt(), V_atoi( rd_health_effect.GetDefault() ) );
}

#include "cbase.h"
#include "rd_vgui_settings.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Video );

CRD_VGUI_Settings_Video::CRD_VGUI_Settings_Video( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	// TODO:
	// Screen Resolution [setting.defaultres / setting.defaultresheight] [drop-down]
	// [auto-filled]
	//
	// [ignoring setting.aspectratiomode]
	//
	// Display Mode [setting.fullscreen / setting.nowindowborder] [drop-down]
	// Exclusive Full Screen - Takes full control of the monitor. Allows you to play the game full-screen at a resolution different than what your desktop is set to. May cause problems when switching applications on some machines.
	// Borderless Window - Sometimes called "desktop friendly full screen". Allows you to switch applications more easily while still covering the entire monitor. Can be set to a smaller resolution than your monitor.
	// Window - Standard operating system window with a title bar. Can be moved and resized. Resizing may cause issues.
	//
	// Rendering Pipeline [setting.mat_queue_mode] [radio buttons]
	// Single-Threaded (Compatibility)	Multi-Threaded (Fastest)
	// Does not affect graphical quality. It is recommended that you use Multi-Threaded rendering unless it causes problems on your hardware.
	//
	// V-Sync [setting.mat_vsync / setting.mat_triplebuffered]
	// Unsynchronized - Render frames as fast as possible, limited to your monitor's refresh rate. May cause screen tearing on some graphics set-ups.
	// Double Buffering - Synchronizes frame rate with your monitor's ability to display frames. May reduce frame rate if your computer cannot render frames fast enough.
	// Triple Buffering - Smooth out frame rate by rendering two frames ahead. Slightly increases input latency.
	//
	// Faster < - - - - > Higher Quality
	//
	// Effect Detail [setting.cpu_level]
	// [0] Low [1] Medium [2] High
	// Affected by CPU (processor) performance. Affects impact effects, physics-controlled debris animations, and precipitation.
	//
	// Shader Detail [setting.gpu_level]
	// [0] Low [1] Medium [2] High [3] Ultra
	// Affected by GPU (graphics card) performance. Affects lighting and material detail, fog, stains, and bullet holes.
	//
	// Texture Detail [setting.gpu_mem_level]
	// [0] Low [1] Medium [2] High [3] Ultra
	// Affected by GPU (graphics card) memory capacity and speed. Affects sharpness of textures.
	//
	// [will remove setting.mem_level]
	// [TODO: revisit what settings are in each EKV file; remove settings that are user-controlled; remove duplicates]
	// [TODO: add gpu_mem_level 3 with mat_picmip -10]
	//
	// Anti-Aliasing [setting.mat_antialias (MSAA) / setting.mat_aaquality (CSAA)]
	// None	2x	4x	6x	8x	16x	8xQ	16xQ [visible options determined by GPU]
	// Smooths out jagged edges on objects. Available options depend on your graphics card.
	//
	// Texture Filtering [setting.mat_forceaniso]
	// Bi-linear	Tri-linear	Anisotropic 2x	Anisotropic 4x	Anisotropic 8x	Anisotropic 16x
	// Higher quality filtering makes textures viewed from a shallow angle or from far away clearer.
	//
	// Film Grain [setting.mat_grain_scale_override]
	// [0] Disabled [-1] Enabled
	// Adds a simulated film grain (fuzziness) effect to the screen, controlled by the mission.
	// It is recommended to disable this if you are recording gameplay as it can negatively affect video encoding.
	//
	// Local Contrast [mat_local_contrast_enable]
	// [0] Disabled [1] Enabled
	// Controls whether missions can change the sharpness of a scene for dramatic effect.
	//
	// Depth Blur [mat_depth_blur_strength_override]
	// [0] Disabled [-1] Enabled
	// Controls whether missions can make objects far below the marines blurry for a simulated camera focus effect.
	//
	// Weather Effects [rd_func_precipitation_enable]
	// [0] Disabled [1] Enabled
	// Controls some rain and snow effects.
	//
	// Light and Specular Blooms [mat_bloom_scalefactor_scalar]
	// [0] Disabled [0.3] Reduced [1] Enabled
	// An effect where brightly glowing objects "spill" light into the surrounding area.
	//
	// High Quality Dynamic Shadows [rd_env_projectedtexture_enabled]
	// [0] Disabled [1] Enabled
	// Some missions contain stationary or moving area lights that produce dramatic shadows of marines and aliens. If this option is disabled, these lights will be turned off entirely.
	//
	// Flashlight Dynamic Shadows [rd_flashlightshadows]
	// [0] Disabled [1] Enabled
	// Controls whether the Flashlight Attachment equipment casts high quality dynamic shadows.
	//
	// Flashlight Light Spill [rd_flashlight_dlight_enable]
	// [0] Disabled [1] Enabled
	// Controls whether the Flashlight Attachment equipment adds light to the area outside its beam.
	//
	// High-Quality Beacons [rd_simple_beacons]
	// [1] Disabled [0] Enabled
	// Controls whether items such as the IAF Heal Beacon and X-33 Damage Amplifier cause light distortion, glowing, and pulsing effects.
	//
	// Muzzle Flash Lights [muzzleflash_light]
	// [0] Disabled [1] Enabled
	// Controls whether guns light up nearby surfaces and objects when fired.
	//
	// Alien Shadows [asw_alien_shadows / asw_directional_shadows]
	// [0/0] Disabled [0/1] Flashlight Only [1/1] Enabled
	// Controls whether aliens have shadows. Warning: Setting this to "Enabled" may have a large affect on the frame rate of the game.
	//
	// Low Health Vignetting [rd_health_effect]
	// [0] Disabled [1] Enabled
	// Controls whether the edges of the screen glow red when the marine is at low health.
}

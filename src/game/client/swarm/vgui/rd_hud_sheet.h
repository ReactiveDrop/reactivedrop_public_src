#pragma once

struct HudSheetTexture_t
{
	float u;
	float v;
	float s;
	float t;
};

// ====================================
//  Macros for defining texture sheets
// ====================================

#define DECLARE_HUD_SHEET( name ) \
	int m_n##name##ID; \
	Vector2D m_vec##name##Size; \
	enum UVs_##name {

#define DECLARE_HUD_SHEET_UV( name ) \
	UV_##name

#define END_HUD_SHEET( name ) \
	NUM_##name##_UVS, \
	}; \
	HudSheetTexture_t m_##name##[ NUM_##name##_UVS ];


class HudSheet_t
{
public:
	HudSheet_t( int *pTextureID, HudSheetTexture_t *pTextureData, int nNumSubTextures, const char *pszTextureFile, Vector2D *pSize ) :
		m_pSheetID( pTextureID ), m_pTextureData( pTextureData ), m_nNumSubTextures( nNumSubTextures ), m_pszTextureFile( pszTextureFile ),
		m_pSize( pSize ) { }

	int *m_pSheetID;
	HudSheetTexture_t *m_pTextureData;
	int m_nNumSubTextures;
	const char *m_pszTextureFile;
	Vector2D *m_pSize;
};

#define ADD_HUD_SHEET( name, textureFile ) \
	m_HudSheets.AddToTail( HudSheet_t( &m_n##name##ID, &m_##name##[0], NUM_##name##_UVS, textureFile, &m_vec##name##Size ) );

#define HUD_UV_COORDS_QUALIFIED( sheet, full_texture_name ) \
	g_RD_HUD_Sheets.m_##sheet[ full_texture_name ].u, \
	g_RD_HUD_Sheets.m_##sheet[ full_texture_name ].v, \
	g_RD_HUD_Sheets.m_##sheet[ full_texture_name ].s, \
	g_RD_HUD_Sheets.m_##sheet[ full_texture_name ].t
#define HUD_UV_COORDS( sheet, full_texture_name ) HUD_UV_COORDS_QUALIFIED( sheet, CRD_HUD_Sheets::UVs_##sheet::full_texture_name )
#define HUD_SHEET_DRAW_RECT_ALPHA( x0, y0, x1, y1, sheet, full_texture_name, alpha ) \
	do \
	{ \
		vgui::surface()->DrawSetColor( 255, 255, 255, ( alpha ) ); \
		vgui::surface()->DrawSetTexture( g_RD_HUD_Sheets.m_n##sheet##ID ); \
		vgui::surface()->DrawTexturedSubRect( ( x0 ), ( y0 ), ( x1 ), ( y1 ), HUD_UV_COORDS( sheet, full_texture_name ) ); \
	} while ( 0 )
#define HUD_SHEET_DRAW_RECT( x0, y0, x1, y1, sheet, full_texture_name ) \
	HUD_SHEET_DRAW_RECT_ALPHA( ( x0 ), ( y0 ), ( x1 ), ( y1 ), sheet, full_texture_name, 255 )
#define HUD_SHEET_DRAW_BOUNDS_ALPHA( sheet, full_texture_name, alpha ) \
	HUD_SHEET_DRAW_RECT_ALPHA( ( x0 ), ( y0 ), ( x0 ) + ( x1 ), ( y0 ) + ( y1 ), sheet, full_texture_name, alpha )
#define HUD_SHEET_DRAW_BOUNDS( sheet, full_texture_name ) \
	HUD_SHEET_DRAW_BOUNDS_ALPHA( sheet, full_texture_name, 255 )
#define HUD_SHEET_DRAW_ABS_ALPHA( sheet, full_texture_name, alpha ) \
	HUD_SHEET_DRAW_RECT_ALPHA( ( x0 ), ( y0 ), ( x1 ), ( y1 ), sheet, full_texture_name, alpha )
#define HUD_SHEET_DRAW_ABS( sheet, full_texture_name ) \
	HUD_SHEET_DRAW_ABS_ALPHA( sheet, full_texture_name, 255 )
#define HUD_SHEET_DRAW_PANEL_ALPHA( pPanel, sheet, full_texture_name, alpha ) \
	do \
	{ \
		if ( ( pPanel )->IsVisible() ) \
		{ \
			int x0, y0, x1, y1; \
			( pPanel )->GetBounds( x0, y0, x1, y1 ); \
			HUD_SHEET_DRAW_BOUNDS_ALPHA( sheet, full_texture_name, alpha ); \
		} \
	} while ( 0 )
#define HUD_SHEET_DRAW_PANEL( pPanel, sheet, full_texture_name ) \
	HUD_SHEET_DRAW_PANEL_ALPHA( ( pPanel ), sheet, full_texture_name, 255 )

extern class CRD_HUD_Sheets : public CAutoGameSystem
{
public:
	CRD_HUD_Sheets() : CAutoGameSystem( "CRD_HUD_Sheets" ) {}

	void PostInit() override
	{
		ADD_HUD_SHEET( Sheet1, "vgui/hud/sheet1/sheet1" );
		ADD_HUD_SHEET( Sheet_Stencil, "vgui/hud/sheet_stencil/sheet_stencil" );
		ADD_HUD_SHEET( MainMenuSheet, "vgui/swarm/main_menu_sheet" );
		ADD_HUD_SHEET( MainMenuAdditive, "vgui/swarm/main_menu_additive_sheet" );
		ADD_HUD_SHEET( CommanderProfileSheet, "vgui/swarm/commander_profile_sheet" );

		FOR_EACH_VEC( m_HudSheets, i )
		{
			*( m_HudSheets[i].m_pSheetID ) = -1;
		}
	}

	void VidInit();

	// HUD Main
	DECLARE_HUD_SHEET( Sheet1 )
		DECLARE_HUD_SHEET_UV( hud_ammo_heal ),
		DECLARE_HUD_SHEET_UV( marine_health_circle_BG ),
		DECLARE_HUD_SHEET_UV( marine_health_circle_FG ),
		DECLARE_HUD_SHEET_UV( marine_health_circle_FG_subtract ),
		DECLARE_HUD_SHEET_UV( marine_health_circle_FG_infested ),
		DECLARE_HUD_SHEET_UV( marine_portrait_BG_cap ),
		DECLARE_HUD_SHEET_UV( marine_portrait_BG_circle ),
		DECLARE_HUD_SHEET_UV( marine_portrait_square ),

		DECLARE_HUD_SHEET_UV( NCOClassIcon ),
		DECLARE_HUD_SHEET_UV( SpecialWeaponsClassIcon ),
		DECLARE_HUD_SHEET_UV( MedicClassIcon ),
		DECLARE_HUD_SHEET_UV( TechClassIcon ),
		DECLARE_HUD_SHEET_UV( DeadIcon ),
		DECLARE_HUD_SHEET_UV( InfestedIcon ),

		DECLARE_HUD_SHEET_UV( team_health_bar_BG ),
		DECLARE_HUD_SHEET_UV( team_health_bar_FG ),
		DECLARE_HUD_SHEET_UV( team_health_bar_FG_subtract ),
		DECLARE_HUD_SHEET_UV( team_health_bar_FG_infested ),
		DECLARE_HUD_SHEET_UV( team_portrait_BG ),
	END_HUD_SHEET( Sheet1 );

	// HUD Marine Ammo
	DECLARE_HUD_SHEET( Sheet_Stencil )
		DECLARE_HUD_SHEET_UV( hud_ammo_bullets ),
		DECLARE_HUD_SHEET_UV( hud_ammo_clip_empty ),
		DECLARE_HUD_SHEET_UV( hud_ammo_clip_full ),
		DECLARE_HUD_SHEET_UV( hud_ammo_grenade ),
		DECLARE_HUD_SHEET_UV( team_ammo_bar ),
		DECLARE_HUD_SHEET_UV( hud_ammo_clip_double ),
	END_HUD_SHEET( Sheet_Stencil );

	// Main Menu
	DECLARE_HUD_SHEET( MainMenuSheet )
		DECLARE_HUD_SHEET_UV( create_lobby ),
		DECLARE_HUD_SHEET_UV( event_timer ),
		DECLARE_HUD_SHEET_UV( hoiaf_timer ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_1 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10 ),
		DECLARE_HUD_SHEET_UV( logo ),
		DECLARE_HUD_SHEET_UV( news ),
		DECLARE_HUD_SHEET_UV( quick_join ),
		DECLARE_HUD_SHEET_UV( quit ),
		DECLARE_HUD_SHEET_UV( settings ),
		DECLARE_HUD_SHEET_UV( singleplayer ),
		DECLARE_HUD_SHEET_UV( ticker_left ),
		DECLARE_HUD_SHEET_UV( ticker_mid ),
		DECLARE_HUD_SHEET_UV( ticker_right ),
		DECLARE_HUD_SHEET_UV( top_bar ),
		DECLARE_HUD_SHEET_UV( top_bar_left ),
		DECLARE_HUD_SHEET_UV( top_bar_right ),
		DECLARE_HUD_SHEET_UV( top_button ),
		DECLARE_HUD_SHEET_UV( update ),
		DECLARE_HUD_SHEET_UV( workshop ),
	END_HUD_SHEET( MainMenuSheet );

	DECLARE_HUD_SHEET( MainMenuAdditive )
		DECLARE_HUD_SHEET_UV( create_lobby_hover ),
		DECLARE_HUD_SHEET_UV( create_lobby_logo_hover ),
		DECLARE_HUD_SHEET_UV( create_lobby_profile_hover ),
		DECLARE_HUD_SHEET_UV( create_lobby_singleplayer_hover ),
		DECLARE_HUD_SHEET_UV( event_timer_above_hover ),
		DECLARE_HUD_SHEET_UV( event_timer_below_hover ),
		DECLARE_HUD_SHEET_UV( event_timer_hoiaf_timer_hover ),
		DECLARE_HUD_SHEET_UV( event_timer_hover ),
		DECLARE_HUD_SHEET_UV( event_timer_news_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_timer_event_timer_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_timer_hoiaf_top_10_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_timer_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_above_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_below_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_hoiaf_timer_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_1 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_2 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_3 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_4 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_5 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_6 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_7 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_10_quit_hover_8 ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_1_below_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_1_hover ),
		DECLARE_HUD_SHEET_UV( hoiaf_top_1_quit_hover ),
		DECLARE_HUD_SHEET_UV( logo_hover ),
		DECLARE_HUD_SHEET_UV( logo_profile_hover ),
		DECLARE_HUD_SHEET_UV( logo_settings_hover ),
		DECLARE_HUD_SHEET_UV( news_event_timer_hover ),
		DECLARE_HUD_SHEET_UV( news_hover ),
		DECLARE_HUD_SHEET_UV( news_update_hover ),
		DECLARE_HUD_SHEET_UV( quick_join_above_hover ),
		DECLARE_HUD_SHEET_UV( quick_join_below_hover ),
		DECLARE_HUD_SHEET_UV( quick_join_hover ),
		DECLARE_HUD_SHEET_UV( quick_join_singleplayer_hover ),
		DECLARE_HUD_SHEET_UV( quit_hover ),
		DECLARE_HUD_SHEET_UV( settings_hover ),
		DECLARE_HUD_SHEET_UV( settings_logo_hover ),
		DECLARE_HUD_SHEET_UV( settings_profile_hover ),
		DECLARE_HUD_SHEET_UV( singleplayer_create_lobby_hover ),
		DECLARE_HUD_SHEET_UV( singleplayer_hover ),
		DECLARE_HUD_SHEET_UV( singleplayer_quick_join_hover ),
		DECLARE_HUD_SHEET_UV( ticker_left_workshop_hover ),
		DECLARE_HUD_SHEET_UV( ticker_right_update_hover ),
		DECLARE_HUD_SHEET_UV( top_bar_button_glow ),
		DECLARE_HUD_SHEET_UV( top_bar_left_logo_glow ),
		DECLARE_HUD_SHEET_UV( top_bar_left_profile_glow ),
		DECLARE_HUD_SHEET_UV( top_bar_left_settings_glow ),
		DECLARE_HUD_SHEET_UV( top_bar_right_hoiaf_glow ),
		DECLARE_HUD_SHEET_UV( top_bar_right_quit_glow ),
		DECLARE_HUD_SHEET_UV( top_button_hover ),
		DECLARE_HUD_SHEET_UV( top_button_left_hover ),
		DECLARE_HUD_SHEET_UV( top_button_profile_hover ),
		DECLARE_HUD_SHEET_UV( top_button_right_hover ),
		DECLARE_HUD_SHEET_UV( update_hover ),
		DECLARE_HUD_SHEET_UV( update_news_hover ),
		DECLARE_HUD_SHEET_UV( workshop_hover ),
		DECLARE_HUD_SHEET_UV( workshop_quick_join_hover ),
	END_HUD_SHEET( MainMenuAdditive );

	// Commander Profile
	DECLARE_HUD_SHEET( CommanderProfileSheet )
		DECLARE_HUD_SHEET_UV( profile ),
		DECLARE_HUD_SHEET_UV( profile_create_lobby_hover ),
		DECLARE_HUD_SHEET_UV( profile_hover ),
		DECLARE_HUD_SHEET_UV( profile_logo_hover ),
		DECLARE_HUD_SHEET_UV( profile_settings_hover ),
	END_HUD_SHEET( CommanderProfileSheet );

	CUtlVector<HudSheet_t> m_HudSheets;
} g_RD_HUD_Sheets;

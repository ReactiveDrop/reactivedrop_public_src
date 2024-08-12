//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "VMainMenu.h"
#include "EngineInterface.h"
#include "VFooterPanel.h"
#include "VHybridButton.h"
#include "VFlyoutMenu.h"
#include "vGenericConfirmation.h"
#include "VQuickJoin.h"
#include "VQuickJoinPublic.h"
#include "basemodpanel.h"
#include "UIGameData.h"
#include "VGameSettings.h"
#include "VSteamCloudConfirmation.h"
#include "vaddonassociation.h"
#include "ConfigManager.h"
#include "rd_vgui_commander_mini_profile.h"
#include "rd_vgui_main_menu_hoiaf_leaderboard_entry.h"
#include "c_asw_steamstats.h"
#include "VSignInDialog.h"
#include "VGuiSystemModuleLoader.h"
#include "VAttractScreen.h"
#include "gamemodes.h"
#include <ctime>

#include "vgui/ILocalize.h"
#include "vgui/ISystem.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Button.h"
#include "vgui_controls/Tooltip.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Image.h"
#include "vgui_controls/TextImage.h"

#include "steam/isteamremotestorage.h"
#include "materialsystem/materialsystem_config.h"
#include "bitmap/psheet.h"

#include "ienginevgui.h"
#include "basepanel.h"
#include "vgui/ISurface.h"
#include "tier0/icommandline.h"
#include "fmtstr.h"
#include "cdll_client_int.h"
#include "inputsystem/iinputsystem.h"
#include "asw_util_shared.h"
#include "matchmaking/swarm/imatchext_swarm.h"
#include "rd_steam_input.h"
#include "rd_workshop.h"
#include "rd_inventory_shared.h"
#include "rd_hud_sheet.h"
#include "rd_vgui_main_menu_top_bar.h"
#include "rd_vgui_stock_ticker.h"
#include "nb_header_footer.h"
#include "briefingtooltip.h"
#include "rd_lobby_utils.h"
#include "rd_hoiaf_utils.h"
#include "rd_vgui_notifications.h"
#include "rd_collections.h"
#include "rd_swarmopedia.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

//=============================================================================
static ConVar connect_lobby( "connect_lobby", "", FCVAR_HIDDEN, "Sets the lobby ID to connect to on start." );
static ConVar ui_old_options_menu( "ui_old_options_menu", "0", FCVAR_HIDDEN, "Brings up the old tabbed options dialog from Keyboard/Mouse when set to 1." );
static ConVar ui_play_online_browser( "ui_play_online_browser", "1", FCVAR_RELEASE, "Whether play online displays a browser or plain search dialog." );
ConVar rd_trending_workshop_tags( "rd_trending_workshop_tags", "Campaign,Bonus,Endless,Challenge", FCVAR_NONE, "Trending addons must have at least one of these tags to appear on the main menu." );
ConVar rd_hoiaf_leaderboard_on_main_menu( "rd_hoiaf_leaderboard_on_main_menu", "1", FCVAR_ARCHIVE, "Should we download HoIAF stats for the current season on the main menu?" );
static void ActivateMainMenuAgain( IConVar *pConVar, const char *szOldValue, float flOldValue )
{
	if ( CBaseModPanel::GetSingleton().GetActiveWindowType() == WT_MAINMENU )
	{
		CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU )->Activate();
	}
}
ConVar rd_hoiaf_leaderboard_friends_only( "rd_hoiaf_leaderboard_friends_only", "0", FCVAR_ARCHIVE, "Only show friends on the main menu HoIAF leaderboard?", ActivateMainMenuAgain );
ConVar rd_main_menu_idle_timeout( "rd_main_menu_idle_timeout", "90", FCVAR_NONE );
ConVar rd_main_menu_slide_in_time( "rd_main_menu_slide_in_time", "0.75", FCVAR_NONE );
ConVar rd_main_menu_slide_out_time( "rd_main_menu_slide_out_time", "2.5", FCVAR_NONE );
extern ConVar rd_reduce_motion;
extern ConVar mm_max_players;
ConVar rd_last_game_access( "rd_last_game_access", "public", FCVAR_ARCHIVE, "Remembers the last game access setting (public or friends) for a lobby created from the main menu." );
ConVar rd_last_game_difficulty( "rd_last_game_difficulty", "normal", FCVAR_ARCHIVE, "Remembers the last game difficulty setting (easy/normal/hard/insane/imba) for a lobby created from the main menu." );
ConVar rd_last_game_challenge( "rd_last_game_challenge", "0", FCVAR_ARCHIVE, "Remembers the last game challenge ID (0 for none) for a lobby created from the main menu." );
ConVar rd_last_game_onslaught( "rd_last_game_onslaught", "0", FCVAR_ARCHIVE, "Remembers the last game onslaught setting for a lobby created from the main menu." );
ConVar rd_last_game_hardcoreff( "rd_last_game_hardcoreff", "0", FCVAR_ARCHIVE, "Remembers the last game hardcore friendly fire setting for a lobby created from the main menu." );
ConVar rd_last_game_maxplayers( "rd_last_game_maxplayers", "4", FCVAR_ARCHIVE, "Remembers the last game max players setting for a lobby created from the main menu." );
ConVar rd_revert_convars( "rd_revert_convars", "1", FCVAR_ARCHIVE, "Resets FCVAR_REPLICATED variables to their default values when opening the main menu." );
#ifdef RD_7A_DROPS
ConVar rd_crafting_material_beta_phase1_show_promo( "rd_crafting_material_beta_phase1_show_promo", "1", FCVAR_ARCHIVE );
#endif

static void OnLegacyUIChanged( IConVar *var, const char *pOldValue, float flOldValue )
{
	// reset the main menu background video
	CBaseModPanel::GetSingleton().InvalidateLayout( true, true );
	CBaseModPanel::GetSingleton().ReleaseBackgroundMusic();
	ASWBackgroundMovie()->Update( true );

	// reset UI scripts
	engine->ClientCmd_Unrestricted( "ui_reloadscheme; hud_reloadscheme\n" );
}
ConVar rd_legacy_ui( "rd_legacy_ui", "", FCVAR_ARCHIVE, "Set to 2004, 2010, or 2017 to use simulated versions of previous user interfaces.", OnLegacyUIChanged );

void Demo_DisableButton( Button *pButton );
void OpenGammaDialog( VPANEL parent );

#ifdef IS_WINDOWS_PC
extern const char *( *const wine_get_version )( void );
const char *( *const wine_get_version )( void ) = static_cast< const char *( * )( void ) >( Plat_GetProcAddress( "ntdll.dll", "wine_get_version" ) );
#endif

static bool s_bMainMenuShown = false;
static uint16_t s_iLastQuickJoinPublicVisible = 0;
static uint16_t s_iLastQuickJoinFriendsVisible = 0;
static int s_nHoIAFCachedEntries = 0;
static LeaderboardEntry_t s_HoIAFLeaderboardEntryCache[10];
static LeaderboardScoreDetails_Points_t s_HoIAFLeaderboardDetailsCache[10];

int g_iSetUnlockedChaptersToValue = 0;

#ifdef RD_7A_DROPS
class CRD_VGUI_Main_Menu_Promo_Model_Viewer : public CRD_Swarmopedia_Model_Panel
{
	DECLARE_CLASS_SIMPLE( CRD_VGUI_Main_Menu_Promo_Model_Viewer, CRD_Swarmopedia_Model_Panel );
public:
	CRD_VGUI_Main_Menu_Promo_Model_Viewer( vgui::Panel *parent, const char *panelName ) : BaseClass{ parent, panelName }
	{
		m_eMode = MODE_FULLSCREEN_MOUSE;
		m_flPitchIntensity = -5.0f;
		m_flYawIntensity = 5.0f;
		m_flPanSpeed = 2.5f;
		m_angPanOrigin.Init( 40.0f, -15.0f, 0.0f );
		m_bShouldDrawGrid = false;

		RD_Swarmopedia::Display display;
		display.Models.SetCount( 1 );
		display.Models[0] = new RD_Swarmopedia::Model;
		display.Models[0]->ModelName = "models/swarm/crafting/ocm_floppy.mdl";
		display.LightingState = SwarmopediaDefaultLightingState();

		SetDisplay( &display );
	}

	void PerformLayout() override
	{
		// hard-coded position. sorry.
		SetBounds( ScreenWidth() - YRES( 315 ), YRES( 355 ), YRES( 100 ), YRES( 100 ) );
		SetZPos( 99 );

		BaseClass::PerformLayout();

		SetVisible( rd_crafting_material_beta_phase1_show_promo.GetBool() );
	}

	void OnCursorEntered()
	{
		for ( int i = 0; i < NELEMS( m_LightingState.m_vecAmbientCube ); i++ )
		{
			m_LightingState.m_vecAmbientCube[i].Init( 0.0f, 5.0f, 6.0f );
		}

		RequestFocus();
	}

	void OnCursorExited()
	{
		for ( int i = 0; i < NELEMS( m_LightingState.m_vecAmbientCube ); i++ )
		{
			m_LightingState.m_vecAmbientCube[i].Init( 24.0f / 255.0f, 24.0f / 255.0f, 24.0f / 255.0f );
		}

		m_bLeftMousePressed = false;
	}

	void OnMousePressed( vgui::MouseCode code ) override
	{
		BaseClass::OnMousePressed( code );

		if ( code == MOUSE_LEFT )
		{
			m_bLeftMousePressed = true;
		}
	}

	void OnMouseReleased( vgui::MouseCode code ) override
	{
		BaseClass::OnMouseReleased( code );

		if ( code == MOUSE_LEFT && m_bLeftMousePressed )
		{
			OnCursorExited();

			CBaseModPanel::GetSingleton().OpenWindow( WT_PROMOOPTIN, CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ), true );
		}
	}

	bool m_bLeftMousePressed{};
};
#endif

//=============================================================================
MainMenu::MainMenu( Panel *parent, const char *panelName ):
	BaseClass( parent, panelName, true, true, false, false )
{
	SetProportional( true );
	SetTitle( "", false );
	SetMoveable( false );
	SetSizeable( false );
	SetPaintBackgroundEnabled( true );

	SetLowerGarnishEnabled( true );

	AddFrameListener( this );

	m_iQuickJoinHelpText = MMQJHT_NONE;
	m_iLastTimerUpdate = 0;

	SetDeleteSelfOnClose( true );

	m_pTopBar = new CRD_VGUI_Main_Menu_Top_Bar( this, "TopBar" );
	m_pTopBar->m_hActiveButton = m_pTopBar->m_pBtnLogo;
	m_pStockTickerHelper = new CRD_VGUI_Stock_Ticker_Helper( this, "StockTickerHelper" );
	m_pCommanderProfile = new CRD_VGUI_Commander_Mini_Profile( this, "CommanderMiniProfile" );
	m_pBtnMultiplayer = new BaseModHybridButton( this, "BtnMultiplayer", "#L4D360UI_FoudGames_CreateNew_campaign", this, "CreateGame" );
	m_pBtnSingleplayer = new BaseModHybridButton( this, "BtnSingleplayer", "#L4D360UI_MainMenu_PlaySolo", this, "SoloPlay" );
	m_pPnlQuickJoin = new QuickJoinPanel( this, "PnlQuickJoin" );
	m_pPnlQuickJoinPublic = new QuickJoinPublicPanel( this, "PnlQuickJoinPublic" );
	m_pBtnWorkshopShowcase = new BaseModHybridButton( this, "BtnWorkshopShowcase", "", this, "WorkshopShowcase" );
	m_pTopLeaderboardEntries[0] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry_Large( this, "HoIAFTop1", this, "HoIAFTop1" );
	m_pTopLeaderboardEntries[1] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop2", this, "HoIAFTop2" );
	m_pTopLeaderboardEntries[2] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop3", this, "HoIAFTop3" );
	m_pTopLeaderboardEntries[3] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop4", this, "HoIAFTop4" );
	m_pTopLeaderboardEntries[4] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop5", this, "HoIAFTop5" );
	m_pTopLeaderboardEntries[5] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop6", this, "HoIAFTop6" );
	m_pTopLeaderboardEntries[6] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop7", this, "HoIAFTop7" );
	m_pTopLeaderboardEntries[7] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop8", this, "HoIAFTop8" );
	m_pTopLeaderboardEntries[8] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop9", this, "HoIAFTop9" );
	m_pTopLeaderboardEntries[9] = new CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry( this, "HoIAFTop10", this, "HoIAFTop10" );
	m_pBtnHoIAFTimer = new BaseModHybridButton( this, "BtnHoIAFTimer", "", this, "ShowHoIAF" );
	m_pBtnEventTimer[0] = new BaseModHybridButton( this, "BtnEventTimer1", "", this, "EventTimer1" );
	m_pBtnEventTimer[1] = new BaseModHybridButton( this, "BtnEventTimer2", "", this, "EventTimer2" );
	m_pBtnEventTimer[2] = new BaseModHybridButton( this, "BtnEventTimer3", "", this, "EventTimer3" );
	m_pBtnNewsShowcase = new BaseModHybridButton( this, "BtnNewsShowcase", "", this, "NewsShowcase" );
	m_pBtnUpdateNotes = new BaseModHybridButton( this, "BtnUpdateNotes", "", this, "UpdateNotes" );
#ifdef RD_7A_DROPS
	m_pCraftingMaterialsBetaPromoButton = new CRD_VGUI_Main_Menu_Promo_Model_Viewer( this, "CraftingMaterialsBetaPromoButton" );
#endif
}

MainMenu::~MainMenu()
{
	RemoveFrameListener( this );

	for ( int i = 0; i < NELEMS( m_pWorkshopTrendingPreview ); i++ )
	{
		delete m_pWorkshopTrendingPreview[i];
	}

	BriefingTooltip::Free();
}

void MainMenu::OpenMainMenuJoinFailed( const char *msg )
{
	// This is called when accepting an invite or joining friends game fails
	CUIGameData::Get()->OpenWaitScreen( msg );
	CUIGameData::Get()->CloseWaitScreen( NULL, NULL );
}

void MainMenu::OnNotifyChildFocus( vgui::Panel* child )
{
}

void MainMenu::OnFlyoutMenuClose( vgui::Panel* flyTo )
{
	SetFooterState();
}

void MainMenu::OnFlyoutMenuCancelled()
{
}

void MainMenu::Activate()
{
	BaseClass::Activate();

	vgui::Panel *firstPanel = FindChildByName( "BtnMultiplayer" );
	if ( firstPanel )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		firstPanel->NavigateTo();
	}

	g_RD_HUD_Sheets.VidInit();

	// don't query group servers over and over; we don't need them
	if ( g_pMatchFramework && g_pMatchFramework->GetMatchSystem() && g_pMatchFramework->GetMatchSystem()->GetUserGroupsServerManager() )
		g_pMatchFramework->GetMatchSystem()->GetUserGroupsServerManager()->EnableServersUpdate( false );

	// reactivedrop: reset the value each time we go in main menu
	// for us to be able to browse lobbies with up to 32 slots
	mm_max_players.Revert();

	// we've left whatever server we were on; get rid of the stuff we borrowed
	g_ReactiveDropWorkshop.UnloadTemporaryAddons();

	static bool s_bRunOnce = true;
	if ( s_bRunOnce )
	{
		// need to know our ping location to show pings in the lobby browser
		UTIL_RD_InitSteamNetworking();

		if ( ConVarRef( "net_steamcnx_allowrelay" ).GetBool() )
		{
			// if relayed connections are enabled, use them by default instead of trying direct IPv4 UDP first
			ConVarRef( "net_steamcnx_enabled" ).SetValue( 2 );
		}

		// see if we earned any goodies since last time we played
		ReactiveDropInventory::RequestGenericPromoItems();

		// update soundcache on initial load
		engine->ClientCmd_Unrestricted( "snd_restart; update_addon_paths; mission_reload; snd_updateaudiocache; snd_restart" );

		// added support for loadout editor, by element109
		engine->ClientCmd( "execifexists loadouts" );

		s_bRunOnce = false;
	}

	int iPatchYear, iPatchMonth, iPatchDay;
	if ( HoIAF()->GetLastUpdateDate( iPatchYear, iPatchMonth, iPatchDay ) )
	{
		char szBranchName[64]{};
		if ( SteamApps() && SteamApps()->GetCurrentBetaName( szBranchName, sizeof(szBranchName) ) && szBranchName[0] != '\0' )
		{
			// we are on a branch. doesn't matter if it's beta, releasecandidate,
			// bugtest, or something else. show the beta notes message.
			m_pBtnUpdateNotes->SetText( "#rd_view_beta_update_notes" );
		}
		else
		{
			wchar_t wszPatchDay1[3];
			wchar_t wszPatchDay2[3];
			wchar_t wszPatchMonth1[3];
			wchar_t wszPatchMonth2[3];
			wchar_t wszPatchYear[5];
			V_snwprintf( wszPatchDay1, NELEMS( wszPatchDay1 ), L"%d", iPatchDay );
			V_snwprintf( wszPatchDay2, NELEMS( wszPatchDay2 ), L"%02d", iPatchDay );
			V_snwprintf( wszPatchMonth1, NELEMS( wszPatchMonth1 ), L"%d", iPatchMonth );
			V_snwprintf( wszPatchMonth2, NELEMS( wszPatchMonth2 ), L"%02d", iPatchMonth );
			V_snwprintf( wszPatchYear, NELEMS( wszPatchYear ), L"%d", iPatchYear );

			wchar_t wszPatchText[1024];
			g_pVGuiLocalize->ConstructString( wszPatchText, sizeof( wszPatchText ), g_pVGuiLocalize->Find( "#rd_latest_update_ymd" ), 5, wszPatchYear, wszPatchMonth2, wszPatchDay2, wszPatchMonth1, wszPatchDay1 );
			m_pBtnUpdateNotes->SetText( wszPatchText );
		}

		m_pBtnUpdateNotes->SetVisible( !m_bIsStub && rd_legacy_ui.GetString()[0] == '\0' );

		// force an update
		m_iLastTimerUpdate = 0;
	}

	ISteamUserStats *pUserStats = SteamUserStats();
	if ( pUserStats && !m_bIsLegacy && rd_hoiaf_leaderboard_on_main_menu.GetBool() )
	{
		for ( int i = 0; i < s_nHoIAFCachedEntries; i++ )
		{
			m_pTopLeaderboardEntries[i]->SetFromEntry( s_HoIAFLeaderboardEntryCache[i], s_HoIAFLeaderboardDetailsCache[i] );
			m_pTopLeaderboardEntries[i]->SetVisible( true );
		}

		for ( int i = s_nHoIAFCachedEntries; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
		{
			m_pTopLeaderboardEntries[i]->SetVisible( false );
		}

		SteamAPICall_t hCall = pUserStats->DownloadLeaderboardEntries( STEAM_LEADERBOARD_HOIAF_CURRENT_SEASON, rd_hoiaf_leaderboard_friends_only.GetBool() ? k_ELeaderboardDataRequestFriends : k_ELeaderboardDataRequestGlobal, 1, 10 );
		m_HoIAFTop10Callback.Set( hCall, this, &MainMenu::OnHoIAFTop10ScoresDownloaded );
	}

	ISteamUGC *pUGC = SteamUGC();
	if ( pUGC && rd_trending_workshop_tags.GetString()[0] != '\0' )
	{
		CSplitString AllowedTags( rd_trending_workshop_tags.GetString(), "," );
		UGCQueryHandle_t hQuery = pUGC->CreateQueryAllUGCRequest( k_EUGCQuery_RankedByTrend, k_EUGCMatchingUGCType_Items_ReadyToUse, 563560, 563560 );
		SteamParamStringArray_t RequiredTags;
		RequiredTags.m_ppStrings = const_cast< const char ** >( AllowedTags.Base() );
		RequiredTags.m_nNumStrings = AllowedTags.Count();
		pUGC->AddRequiredTagGroup( hQuery, &RequiredTags );
		if ( ISteamApps *pApps = SteamApps() )
			pUGC->SetLanguage( hQuery, pApps->GetCurrentGameLanguage() );
		pUGC->SetAllowCachedResponse( hQuery, 3600 );
		SteamAPICall_t hCall = pUGC->SendQueryUGCRequest( hQuery );
		m_WorkshopTrendingItemsCallback.Set( hCall, this, &MainMenu::OnWorkshopTrendingItems );
	}

	BriefingTooltip::EnsureParent( this );

	if ( g_iSetUnlockedChaptersToValue != 0 )
	{
		engine->ClientCmd_Unrestricted( VarArgs( "sv_unlockedchapters %d; host_writeconfig\n", g_iSetUnlockedChaptersToValue ) );
		g_iSetUnlockedChaptersToValue = 0;
	}

	m_flLastActiveTime = Plat_FloatTime();
}

void MainMenu::LoadLayout()
{
	const char *pSettings = "Resource/UI/BaseModUI/MainMenu.res";
	m_bIsLegacy = false;

	if ( !V_strcmp( rd_legacy_ui.GetString(), "2004" ) || !V_strcmp( rd_legacy_ui.GetString(), "2010" ) || !V_strcmp( rd_legacy_ui.GetString(), "2017" ) )
	{
		pSettings = "Resource/UI/BaseModUI/MainMenuLegacy.res";
		m_bIsLegacy = true;
	}

	if ( CommandLine()->FindParm( "-teststeamapiloadfail" ) || !g_pMatchFramework || !g_pMatchFramework->GetMatchSystem() || !g_pMatchFramework->GetMatchSystem()->GetPlayerManager() || !g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetLocalPlayer(0) || !SteamApps() || !SteamFriends() || !SteamHTTP() || !SteamInput() || !SteamMatchmaking() || !SteamMatchmakingServers() || !SteamRemoteStorage() || !SteamUGC() || !SteamUser() || !SteamUserStats() || !SteamUtils())
	{
		pSettings = "Resource/UI/BaseModUI/MainMenuStub.res";
		m_bIsStub = true;

		for ( int i = GetChildCount() - 1; i >= 0; i-- )
		{
			// Clean up all UI elements we made in the constructor
			vgui::Panel *pChild = GetChild( i );
			if ( pChild && V_stricmp( pChild->GetName(), "BtnStub" ) && V_stricmp( pChild->GetName(), "BtnQuit" ) )
			{
				pChild->SetVisible( false );
			}
		}

		// go ahead and just click the button to ask for help on behalf of the user.
		PostMessage( this, new KeyValues( "Command", "command", "BtnStub" ), 0.5f );
	}

	V_strncpy( m_ResourceName, pSettings, sizeof( m_ResourceName ) );

	BaseClass::LoadLayout();
}

void MainMenu::ApplySchemeSettings( IScheme *pScheme )
{
	static bool s_bReloadLocOnce = true;
	if ( s_bReloadLocOnce )
	{
		// BenLubar: load translations again right before the main menu appears for the first time
		UTIL_RD_ReloadLocalizeFiles();
		s_bReloadLocOnce = false;
	}

	BaseClass::ApplySchemeSettings( pScheme );

	BaseModHybridButton *button = dynamic_cast< BaseModHybridButton* >( FindChildByName( "BtnPlaySolo" ) );
	if ( button )
	{
#ifdef _X360
		button->SetText( ( XBX_GetNumGameUsers() > 1 ) ? ( "#L4D360UI_MainMenu_PlaySplitscreen" ) : ( "#L4D360UI_MainMenu_PlaySolo" ) );
		button->SetHelpText( ( XBX_GetNumGameUsers() > 1 ) ? ( "#L4D360UI_MainMenu_OfflineCoOp_Tip" ) : ( "#L4D360UI_MainMenu_PlaySolo_Tip" ) );
#endif
	}

#ifdef _X360
	if ( !XBX_GetPrimaryUserIsGuest() )
	{
		wchar_t wszListText[ 128 ];
		wchar_t wszPlayerName[ 128 ];

		IPlayer *player1 = NULL;
		if ( XBX_GetNumGameUsers() > 0 )
		{
			player1 = g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetLocalPlayer( XBX_GetUserId( 0 ) );
		}

		IPlayer *player2 = NULL;
		if ( XBX_GetNumGameUsers() > 1 )
		{
			player2 = g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetLocalPlayer( XBX_GetUserId( 1 ) );
		}

		if ( player1 )
		{
			Label *pLblPlayer1GamerTag = dynamic_cast< Label* >( FindChildByName( "LblPlayer1GamerTag" ) );
			if ( pLblPlayer1GamerTag )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( player1->GetName(), wszPlayerName, sizeof( wszPlayerName ) );
				g_pVGuiLocalize->ConstructString( wszListText, sizeof( wszListText ), g_pVGuiLocalize->Find( "#L4D360UI_MainMenu_LocalProfilePlayer1" ), 1, wszPlayerName );

				pLblPlayer1GamerTag->SetVisible( true );
				pLblPlayer1GamerTag->SetText( wszListText );
			}
		}

		if ( player2 )
		{
			Label *pLblPlayer2GamerTag = dynamic_cast< Label* >( FindChildByName( "LblPlayer2GamerTag" ) );
			if ( pLblPlayer2GamerTag )
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( player2->GetName(), wszPlayerName, sizeof( wszPlayerName ) );
				g_pVGuiLocalize->ConstructString( wszListText, sizeof( wszListText ), g_pVGuiLocalize->Find( "#L4D360UI_MainMenu_LocalProfilePlayer2" ), 1, wszPlayerName );

				pLblPlayer2GamerTag->SetVisible( true );
				pLblPlayer2GamerTag->SetText( wszListText );

				// in split screen, have player2 gamer tag instead of enable, and disable
				SetControlVisible( "LblPlayer2DisableIcon", true );
				SetControlVisible( "LblPlayer2Disable", true );
				SetControlVisible( "LblPlayer2Enable", false );
			}
		}
		else
		{
			SetControlVisible( "LblPlayer2DisableIcon", false );
			SetControlVisible( "LblPlayer2Disable", false );

			// not in split screen, no player2 gamertag, instead have enable
			SetControlVisible( "LblPlayer2GamerTag", false );
			SetControlVisible( "LblPlayer2Enable", true );
		}
	}
#endif

	if ( IsPC() )
	{
		FlyoutMenu *pFlyout = dynamic_cast< FlyoutMenu* >( FindChildByName( "FlmOptionsFlyout" ) );
		if ( pFlyout )
		{
			bool bUsesCloud = false;

#ifdef IS_WINDOWS_PC
			ISteamRemoteStorage *pRemoteStorage = SteamRemoteStorage();
#else
			ISteamRemoteStorage *pRemoteStorage =  NULL; 
			AssertMsg( false, "This branch run on a PC build without IS_WINDOWS_PC defined." );
#endif

			uint64 availableBytes, totalBytes = 0;
			if ( pRemoteStorage && pRemoteStorage->GetQuota( &totalBytes, &availableBytes ) )
			{
				if ( totalBytes > 0 )
				{
					bUsesCloud = true;
				}
			}

			pFlyout->SetControlEnabled( "BtnCloud", bUsesCloud );
		}
	}

	SetFooterState();

	vgui::Panel *firstPanel = FindChildByName( "BtnMultiplayer" );
	if ( firstPanel )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom( );
		}
		firstPanel->NavigateTo();
	}

#if defined( _X360 ) && defined( _DEMO )
	SetControlVisible( "BtnExtras", !engine->IsDemoHostedFromShell() );
	SetControlVisible( "BtnQuit", engine->IsDemoHostedFromShell() );
#endif

	// CERT CATCH ALL JUST IN CASE!
#ifdef _X360
	bool bAllUsersCorrectlySignedIn = ( XBX_GetNumGameUsers() > 0 );
	for ( int k = 0; k < ( int ) XBX_GetNumGameUsers(); ++ k )
	{
		if ( !g_pMatchFramework->GetMatchSystem()->GetPlayerManager()->GetLocalPlayer( XBX_GetUserId( k ) ) )
			bAllUsersCorrectlySignedIn = false;
	}
	if ( !bAllUsersCorrectlySignedIn )
	{
		Warning( "======= SIGNIN FAIL SIGNIN FAIL SIGNIN FAIL SIGNIN FAIL ==========\n" );
		Assert( 0 );
		CBaseModPanel::GetSingleton().CloseAllWindows( CBaseModPanel::CLOSE_POLICY_EVEN_MSGS );
		CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GAMESTART );
		CBaseModPanel::GetSingleton().OpenWindow( WT_ATTRACTSCREEN, NULL, true );
		Warning( "======= SIGNIN RESET SIGNIN RESET SIGNIN RESET SIGNIN RESET ==========\n" );
	}
#endif

	m_pBranchDisclaimer = dynamic_cast< vgui::Label * >( FindChildByName( "LblBranchDisclaimer" ) );
	ISteamApps *pApps = SteamApps();
	if ( m_pBranchDisclaimer && pApps )
	{
		char szBranch[256]{};
		if ( !pApps->GetCurrentBetaName( szBranch, sizeof( szBranch ) ) )
		{
			m_pBranchDisclaimer->SetVisible( false );
		}
		else
		{
			m_pBranchDisclaimer->SetText( VarArgs( "#rd_branch_disclaimer_%s", szBranch ) );
			m_pBranchDisclaimer->SetVisible( true );
			int w, t;
			m_pBranchDisclaimer->GetTextImage()->GetContentSize( w, t );
			m_pBranchDisclaimer->SetTextInset( YRES( 2 ), YRES( 2 ) );
			m_pBranchDisclaimer->SetSize( w + YRES( 4 ), t + YRES( 4 ) );
		}
	}

	// force an update
	m_iLastTimerUpdate = 0;

	if ( m_iHoIAFTimerOffset >= 0 )
	{
		int x, y, discard;
		m_pBtnHoIAFTimer->GetPos( x, discard );
		m_pTopLeaderboardEntries[0]->GetPos( discard, y );

		for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
		{
			if ( !m_pTopLeaderboardEntries[i]->IsVisible() )
				break;

			m_pTopLeaderboardEntries[i]->GetPos( discard, y );
			y += m_pTopLeaderboardEntries[i]->GetTall() + m_iHoIAFTimerOffset;
		}
		m_pBtnHoIAFTimer->SetPos( x, y );
	}

	m_bGrabPanelLocations = true;
}

void MainMenu::OnCommand( const char *command )
{
	int iUserSlot = CBaseModPanel::GetSingleton().GetLastActiveUserId();

	if ( UI_IsDebug() )
	{
		Msg( "[GAMEUI] Handling main menu command %s from user%d ctrlr%d\n",
			command, iUserSlot, XBX_GetUserId( iUserSlot ) );
	}

	bool bOpeningFlyout = false;

	if ( char const *szQuickMatch = StringAfterPrefix( command, "QuickMatch_" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() ||
			CUIGameData::Get()->CheckAndDisplayErrorIfNotSignedInToLive( this ) )
			return;

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
			" network LIVE "
			" } "
			" game { "
			" mode = "
			" } "
			" options { "
			" action quickmatch "
			" } "
		);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetString( "game/mode", szQuickMatch );

		// TCR: We need to respect the default difficulty
		if ( GameModeHasDifficulty( szQuickMatch ) )
			pSettings->SetString( "game/difficulty", GameModeGetDefaultDifficulty( szQuickMatch ) );

		g_pMatchFramework->MatchSession( pSettings );
	}
	else if ( char const *szCustomMatch = StringAfterPrefix( command, "CustomMatch_" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() ||
			CUIGameData::Get()->CheckAndDisplayErrorIfNotSignedInToLive( this ) )
			return;

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
			" network LIVE "
			" } "
			" game { "
			" mode = "
			" } "
			" options { "
			" action custommatch "
			" } "
		);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetString( "game/mode", szCustomMatch );

		CBaseModPanel::GetSingleton().OpenWindow(
			ui_play_online_browser.GetBool() ? WT_FOUNDPUBLICGAMES : WT_GAMESETTINGS,
			this, true, pSettings );
	}
	else if ( char const *szFriendsMatch = StringAfterPrefix( command, "FriendsMatch_" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() )
			return;

		if ( StringHasPrefix( szFriendsMatch, "team" ) &&
			CUIGameData::Get()->CheckAndDisplayErrorIfNotSignedInToLive( this ) )
			// Team games require to be signed in to LIVE
			return;

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" game { "
			" mode = "
			" } "
		);
		KeyValues::AutoDelete autodelete( pSettings );

		pSettings->SetString( "game/mode", szFriendsMatch );

		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_ALLGAMESEARCHRESULTS, this, true, pSettings );
	}
	else if ( char const *szGroupServer = StringAfterPrefix( command, "GroupServer_" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() )
			return;

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" game { "
			// " mode = "
			" } "
			" options { "
			" action groupserver "
			" } "
		);
		KeyValues::AutoDelete autodelete( pSettings );

		if ( *szGroupServer )
			pSettings->SetString( "game/mode", szGroupServer );

		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_STEAMGROUPSERVERS, this, true, pSettings );
	}
	else if ( !Q_strcmp( command, "BtnStub" ) )
	{
		// clicking No Steam will provide some info
		GenericConfirmation *confirmation =
			static_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
		GenericConfirmation::Data_t data;
		data.pWindowTitle = "#rd_no_steam_service";
		data.pMessageText = "#rd_no_steam_solutions";

		if ( !CommandLine()->FindParm( "-teststeamapiloadfail" ) && SteamApps() && SteamFriends() && SteamHTTP() && SteamInput() && SteamMatchmaking() && SteamMatchmakingServers() && SteamRemoteStorage() && SteamUGC() && SteamUser() && SteamUserStats() && SteamUtils() )
		{
			// The NO STEAM main menu is active, but the Steam API is available. This should never happen. Please contact https://reactivedrop.com/feedback
			data.pMessageText = "#rd_no_steam_solutions_api";
		}
		else if ( !SteamAPI_IsSteamRunning() )
		{
			// Did not detect an instance of the Steam Client. If the Steam Client is running, try selecting Steam->Exit and then restarting Steam.
			data.pMessageText = "#rd_no_steam_solutions_client";
		}
		else if ( !SteamAPI_GetSteamInstallPath() )
		{
			// Could not determine the location of the Steam Client through the registry. Try restarting Steam.
			data.pMessageText = "#rd_no_steam_solutions_path";
		}
		else if ( wine_get_version )
		{
			static wchar_t s_wszWineVersionWarning[1024];
			wchar_t wszWineVersion[128];
			V_UTF8ToUnicode( wine_get_version(), wszWineVersion, sizeof( wszWineVersion ) );
			g_pVGuiLocalize->ConstructString( s_wszWineVersionWarning, sizeof( s_wszWineVersionWarning ), g_pVGuiLocalize->Find( "#rd_no_steam_solutions_wine" ), 1, wszWineVersion );
			data.pMessageTextW = s_wszWineVersionWarning;
		}

		data.bOkButtonEnabled = true;
		confirmation->SetUsageData( data );
	}
	else if ( !Q_strcmp( command, "TrainingPlay" ) )
	{
		// Ensure that tutorial messages will be displayed.
		engine->ClientCmd_Unrestricted( "gameinstructor_enable 1; gameinstructor_reset_counts" );

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
			" network offline "
			" } "
			" game { "
			" mode single_mission "
			" campaign jacob "
			" mission asi-jac1-landingbay_pract "
			" difficulty normal "
			" } "
		);
		KeyValues::AutoDelete autodelete( pSettings );

		g_pMatchFramework->CreateSession( pSettings );

		// Automatically start the tutorial mission, no configuration required
		if ( IMatchSession *pMatchSession = g_pMatchFramework->GetMatchSession() )
		{
			pMatchSession->Command( KeyValues::AutoDeleteInline( new KeyValues( "Start" ) ) );
		}
	}
	else if ( !Q_strcmp( command, "SoloPlay" ) )
	{
		engine->ClientCmd_Unrestricted( "asw_mission_chooser singleplayer" );
	}
	else if ( !Q_strcmp( command, "DeveloperCommentary" ) )
	{
#ifdef _X360
		if ( XBX_GetNumGameUsers() > 1 )
		{
			GenericConfirmation *confirmation =
				static_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

			GenericConfirmation::Data_t data;

			data.pWindowTitle = "#L4D360UI_MainMenu_SplitscreenDisableConf";
			data.pMessageText = "#L4D360UI_Extras_Commentary_ss_Msg";

			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &AcceptSplitscreenDisableCallback;
			data.bCancelButtonEnabled = true;

			confirmation->SetUsageData( data );
			return;
		}
#endif
		// Explain the rules of commentary
		GenericConfirmation *confirmation =
			static_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

		GenericConfirmation::Data_t data;

		data.pWindowTitle = "#GAMEUI_CommentaryDialogTitle";
		data.pMessageText = "#L4D360UI_Commentary_Explanation";

		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &AcceptCommentaryRulesCallback;
		data.bCancelButtonEnabled = true;

		confirmation->SetUsageData( data );
		NavigateFrom();
	}
	else if ( !Q_strcmp( command, "StatsAndAchievements" ) )
	{
		// If PC make sure that the Steam user is logged in
		if ( CheckAndDisplayErrorIfNotLoggedIn() )
			return;

#ifdef _X360
		// If 360 make sure that the user is not a guest
		if ( XBX_GetUserIsGuest( CBaseModPanel::GetSingleton().GetLastActiveUserId() ) )
		{
			GenericConfirmation *confirmation =
				static_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );
			GenericConfirmation::Data_t data;
			data.pWindowTitle = "#L4D360UI_MsgBx_AchievementsDisabled";
			data.pMessageText = "#L4D360UI_MsgBx_GuestsUnavailableToGuests";
			data.bOkButtonEnabled = true;
			confirmation->SetUsageData( data );

			return;
		}
#endif //_X360
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}

		CBaseModPanel::GetSingleton().OpenWindow( WT_ACHIEVEMENTS, this, true );
	}
	else if ( !Q_strcmp( command, "FlmExtrasFlyoutCheck" ) )
	{
		if ( IsX360() && CUIGameData::Get()->SignedInToLive() )
			OnCommand( "FlmExtrasFlyout_Live" );
		else
			OnCommand( "FlmExtrasFlyout_Simple" );
		return;
	}
	else if ( char const *szInviteType = StringAfterPrefix( command, "InviteUI_" ) )
	{
		CUIGameData::Get()->OpenInviteUI( szInviteType );
	}
	else if ( !Q_strcmp( command, "Game" ) )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_GAMEOPTIONS, this, true );
	}
	else if ( !Q_strcmp( command, "AudioVideo" ) )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_AUDIOVIDEO, this, true );
	}
	else if ( !Q_strcmp( command, "Controller" ) )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_CONTROLLER, this, true );
	}
	else if ( !Q_strcmp( command, "Storage" ) )
	{
#ifdef _X360
		if ( XBX_GetUserIsGuest( iUserSlot ) )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_INVALID );
			return;
		}
#endif
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}

		// Trigger storage device selector
		CUIGameData::Get()->SelectStorageDevice( new CChangeStorageDevice( XBX_GetUserId( iUserSlot ) ) );
	}
	else if ( !Q_strcmp( command, "Workshop" ) )
	{
		OpenNewsURL( "https://steamcommunity.com/app/563560/workshop/?l=%s" );
	}
	else if ( !Q_strcmp( command, "WorkshopManage" ) )
	{
		CBaseModPanel::GetSingleton().OpenWindow( WT_WORKSHOP, this );
	}
	else if ( !Q_strcmp( command, "Credits" ) )
	{
#ifdef _X360
		if ( XBX_GetNumGameUsers() > 1 )
		{
			GenericConfirmation *confirmation =
				static_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

			GenericConfirmation::Data_t data;

			data.pWindowTitle = "#L4D360UI_MainMenu_SplitscreenDisableConf";
			data.pMessageText = "#L4D360UI_Extras_Credits_ss_Msg";

			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &AcceptSplitscreenDisableCallback;
			data.bCancelButtonEnabled = true;

			confirmation->SetUsageData( data );
			return;
		}
#endif
		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
			" network offline "
			" } "
			" game { "
			" mode single_mission "
			" } "
			" options { "
			" play credits "
			" } "
		);
		KeyValues::AutoDelete autodelete( pSettings );

		g_pMatchFramework->CreateSession( pSettings );

		// Automatically start the credits session, no configuration required
		if ( IMatchSession *pMatchSession = g_pMatchFramework->GetMatchSession() )
		{
			pMatchSession->Command( KeyValues::AutoDeleteInline( new KeyValues( "Start" ) ) );
		}
	}
	else if ( !Q_strcmp( command, "QuitGame" ) )
	{
		if ( IsPC() )
		{
			GenericConfirmation *confirmation =
				static_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

			GenericConfirmation::Data_t data;

			data.pWindowTitle = "#L4D360UI_MainMenu_Quit_Confirm";
			data.pMessageText = "#L4D360UI_MainMenu_Quit_ConfirmMsg";

			data.bOkButtonEnabled = true;
			data.pfnOkCallback = &AcceptQuitGameCallback;
			data.bCancelButtonEnabled = true;

			confirmation->SetUsageData( data );

			NavigateFrom();
		}

		if ( IsX360() )
		{
			engine->ExecuteClientCmd( "demo_exit" );
		}
	}
	else if ( !Q_stricmp( command, "QuitGame_NoConfirm" ) )
	{
		if ( IsPC() )
		{
			engine->ClientCmd( "quit" );
		}
	}
	else if ( !Q_strcmp( command, "EnableSplitscreen" ) )
	{
		Msg( "Enabling splitscreen from main menu...\n" );

		CBaseModPanel::GetSingleton().CloseAllWindows();
		CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GOSPLITSCREEN );
		CBaseModPanel::GetSingleton().OpenWindow( WT_ATTRACTSCREEN, NULL, true );
	}
	else if ( !Q_strcmp( command, "DisableSplitscreen" ) )
	{
		GenericConfirmation *confirmation =
			static_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, this, false ) );

		GenericConfirmation::Data_t data;

		data.pWindowTitle = "#L4D360UI_MainMenu_SplitscreenDisableConf";
		data.pMessageText = "#L4D360UI_MainMenu_SplitscreenDisableConfMsg";

		data.bOkButtonEnabled = true;
		data.pfnOkCallback = &AcceptSplitscreenDisableCallback;
		data.bCancelButtonEnabled = true;

		confirmation->SetUsageData( data );
	}
	else if ( !Q_strcmp( command, "DisableSplitscreen_NoConfirm" ) )
	{
		Msg( "Disabling splitscreen from main menu...\n" );

		CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GAMESTART );
		OnCommand( "ActivateAttractScreen" );
	}
	else if ( !Q_strcmp( command, "ChangeGamers" ) )	// guest SIGN-IN command
	{
		CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GUESTSIGNIN, XBX_GetUserId( iUserSlot ) );
		OnCommand( "ActivateAttractScreen" );
	}
	else if ( !Q_strcmp( command, "ActivateAttractScreen" ) )
	{
		if ( IsX360() )
		{
			Close();
			CBaseModPanel::GetSingleton().CloseAllWindows();
			CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GAMESTART );
			CBaseModFrame *pWnd = CBaseModPanel::GetSingleton().OpenWindow( WT_ATTRACTSCREEN, NULL, true );
			if ( pWnd )
			{
				pWnd->PostMessage( pWnd, new KeyValues( "ChangeGamers" ) );
			}
		}
	}
	else if ( !Q_strcmp( command, "Audio" ) )
	{
		if ( ui_old_options_menu.GetBool() )
		{
			CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
		}
		else
		{
			// audio options dialog, PC only
			if ( m_ActiveControl )
			{
				m_ActiveControl->NavigateFrom();
			}
			CBaseModPanel::GetSingleton().OpenWindow( WT_AUDIO, this, true );
		}
	}
	else if ( !Q_strcmp( command, "Video" ) )
	{
		if ( ui_old_options_menu.GetBool() )
		{
			CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
		}
		else
		{
			// video options dialog, PC only
			if ( m_ActiveControl )
			{
				m_ActiveControl->NavigateFrom();
			}
			CBaseModPanel::GetSingleton().OpenWindow( WT_VIDEO, this, true );
		}
	}
	else if ( !Q_strcmp( command, "Brightness" ) )
	{
		if ( ui_old_options_menu.GetBool() )
		{
			CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
		}
		else
		{
			// brightness options dialog, PC only
			OpenGammaDialog( GetVParent() );
		}
	}
	else if ( !Q_strcmp( command, "KeyboardMouse" ) )
	{
		if ( ui_old_options_menu.GetBool() )
		{
			CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
		}
		else
		{
			// standalone keyboard/mouse dialog, PC only
			if ( m_ActiveControl )
			{
				m_ActiveControl->NavigateFrom();
			}
			CBaseModPanel::GetSingleton().OpenWindow( WT_KEYBOARDMOUSE, this, true );
		}
	}
	else if ( !Q_strcmp( command, "Gamepad" ) )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_GAMEPAD, this, true );
	}
	else if ( Q_stricmp( "#L4D360UI_Controller_Edit_Keys_Buttons", command ) == 0 )
	{
		FlyoutMenu::CloseActiveMenu();
		CBaseModPanel::GetSingleton().OpenKeyBindingsDialog( this );
	}
	else if ( !Q_strcmp( command, "MultiplayerSettings" ) )
	{
		if ( ui_old_options_menu.GetBool() )
		{
			CBaseModPanel::GetSingleton().OpenOptionsDialog( this );
		}
		else
		{
			// standalone multiplayer settings dialog, PC only
			if ( m_ActiveControl )
			{
				m_ActiveControl->NavigateFrom();
			}
			CBaseModPanel::GetSingleton().OpenWindow( WT_MULTIPLAYER, this, true );
		}
	}
	else if ( !Q_strcmp( command, "AdvancedSettings" ) )
	{
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_ADVANCEDSETTINGS, this, true );
	}
	else if ( !Q_strcmp( command, "CloudSettings" ) )
	{
		// standalone cloud settings dialog, PC only
		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_CLOUD, this, true );
	}
	else if ( !Q_strcmp( command, "SeeAll" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() )
			return;

		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" game { "
			// passing empty game settings to indicate no preference
			" } "
		);
		KeyValues::AutoDelete autodelete( pSettings );

		if ( m_ActiveControl )
		{
			m_ActiveControl->NavigateFrom();
		}
		CBaseModPanel::GetSingleton().OpenWindow( WT_ALLGAMESEARCHRESULTS, this, true, pSettings );
		CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
	}
	else if ( !Q_strcmp( command, "OpenServerBrowser" ) )
	{
		if ( CheckAndDisplayErrorIfNotLoggedIn() )
			return;

		// on PC, bring up the server browser and switch it to the LAN tab (tab #5)
		engine->ClientCmd( "openserverbrowser" );
	}
	else if ( !Q_strcmp( command, "DemoConnect" ) )
	{
		g_pMatchFramework->GetMatchTitle()->PrepareClientForConnect( NULL );
		engine->ClientCmd( CFmtStr( "connect %s", demo_connect_string.GetString() ) );
	}
	else if ( command && command[0] == '#' )
	{
		// Pass it straight to the engine as a command
		engine->ClientCmd( command + 1 );
	}
	else if ( !Q_strcmp( command, "Addons" ) )
	{
		CBaseModPanel::GetSingleton().OpenWindow( WT_ADDONS, this, true );
	}
	else if ( !Q_strcmp( command, "IafRanks" ) )
	{
		OpenNewsURL( "https://stats.reactivedrop.com/heroes?l=%s" );
	}
	else if ( !Q_strcmp( command, "CreateGame" ) )
	{
		engine->ClientCmd_Unrestricted( "asw_mission_chooser createserver" );
	}
	else if ( !V_stricmp( command, "WorkshopShowcase" ) )
	{
		uint32 iCurrentWorkshopShowcase = ( std::time( NULL ) / 60 ) % NELEMS( m_iWorkshopTrendingFileID );
		if ( m_iWorkshopTrendingFileID[iCurrentWorkshopShowcase] == k_PublishedFileIdInvalid )
		{
			OpenNewsURL( "https://steamcommunity.com/app/563560/workshop/?l=%s" );
		}
		else
		{
			OpenNewsURL( CFmtStr{ "https://steamcommunity.com/workshop/filedetails/?id=%llu&l=%%s", m_iWorkshopTrendingFileID[iCurrentWorkshopShowcase] } );
		}
	}
	else if ( const char *szHoIAFPlaceNumber = StringAfterPrefix( command, "HoIAFTop" ) )
	{
		int iPlaceNumber = V_atoi( szHoIAFPlaceNumber );
		Assert( iPlaceNumber >= 1 && iPlaceNumber <= NELEMS( m_pTopLeaderboardEntries ) );
		if ( iPlaceNumber >= 1 && iPlaceNumber <= NELEMS( m_pTopLeaderboardEntries ) )
		{
			Assert( m_pTopLeaderboardEntries[iPlaceNumber - 1]->IsVisible() );
			KeyValues::AutoDelete pSettings{ "settings" };
			pSettings->SetUint64( "steamid", m_pTopLeaderboardEntries[iPlaceNumber - 1]->m_SteamID.ConvertToUint64() );
			CBaseModPanel::GetSingleton().OpenWindow( WT_COMMANDERPROFILE, this, true, pSettings );
		}
	}
	else if ( !V_stricmp( command, "ShowHoIAF" ) )
	{
		OpenNewsURL( "https://stats.reactivedrop.com/heroes?lang=%s" );
	}
	else if ( const char *szEventNumber = StringAfterPrefix( command, "EventTimer" ) )
	{
		OpenNewsURL( HoIAF()->GetEventTimerURL( V_atoi( szEventNumber ) - 1 ) );
	}
	else if ( !V_stricmp( command, "NewsShowcase" ) )
	{
		int iNumNewsShowcases = HoIAF()->CountFeaturedNews();
		if ( iNumNewsShowcases )
		{
			int iCurrentMinute = ( std::time( NULL ) / 60 );
			OpenNewsURL( HoIAF()->GetFeaturedNewsURL( iCurrentMinute %iNumNewsShowcases ) );
		}
	}
	else if ( !V_stricmp( command, "UpdateNotes" ) )
	{
		char szBranchName[64]{};
		if ( SteamApps() && SteamApps()->GetCurrentBetaName( szBranchName, sizeof( szBranchName ) ) && szBranchName[0] != '\0' )
		{
			OpenNewsURL( "https://reactivedrop.com/beta-updates/" );
		}
		else
		{
			OpenNewsURL( "https://steamcommunity.com/app/563560/allnews/?l=%s" );
		}
	}
	else if ( !V_stricmp( command, "Settings" ) || !V_stricmp( command, "Loadout" ) || !V_stricmp( command, "Contracts" ) || !V_stricmp( command, "Recordings" ) || !V_stricmp( command, "Swarmopedia" ) || !V_stricmp( command, "Workshop" ) || !V_stricmp( command, "Inventory" ) )
	{
		// forward to top bar so modded main menus can use these commands
		static bool s_bReentrant = false;
		Assert( !s_bReentrant );
		if ( !s_bReentrant )
		{
			s_bReentrant = true;
			m_pTopBar->OnCommand( command );
			s_bReentrant = false;
		}
	}
	else
	{
		// does this command match a flyout menu?
		BaseModUI::FlyoutMenu *flyout = dynamic_cast< FlyoutMenu * >( FindChildByName( command ) );
		if ( flyout )
		{
			bOpeningFlyout = true;

			// If so, enumerate the buttons on the menu and find the button that issues this command.
			// (No other way to determine which button got pressed; no notion of "current" button on PC.)
			for ( int iChild = 0; iChild < GetChildCount(); iChild++ )
			{
				bool bFound = false;
				GameModes *pGameModes = dynamic_cast< GameModes * >( GetChild( iChild ) );
				if ( pGameModes )
				{
					for ( int iGameMode = 0; iGameMode < pGameModes->GetNumGameInfos(); iGameMode++ )
					{
						BaseModHybridButton *pHybrid = pGameModes->GetHybridButton( iGameMode );
						if ( pHybrid && pHybrid->GetCommand() && !Q_strcmp( pHybrid->GetCommand()->GetString( "command" ), command ) )
						{
							pHybrid->NavigateFrom();
							// open the menu next to the button that got clicked
							flyout->OpenMenu( pHybrid );
							flyout->SetListener( this );
							bFound = true;
							break;
						}
					}
				}

				if ( !bFound )
				{
					BaseModHybridButton *hybrid = dynamic_cast< BaseModHybridButton * >( GetChild( iChild ) );
					if ( hybrid && hybrid->GetCommand() && !Q_strcmp( hybrid->GetCommand()->GetString( "command" ), command ) )
					{
						hybrid->NavigateFrom();
						// open the menu next to the button that got clicked
						flyout->OpenMenu( hybrid );
						flyout->SetListener( this );
						break;
					}
				}
			}
		}
		else
		{
			BaseClass::OnCommand( command );
		}
	}

	if ( !bOpeningFlyout )
	{
		FlyoutMenu::CloseActiveMenu(); //due to unpredictability of mouse navigation over keyboard, we should just close any flyouts that may still be open anywhere.
	}
}

void MainMenu::OnKeyCodePressed( KeyCode code )
{
	int userId = GetJoystickForCode( code );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( userId );

	switch ( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_B:
		// Capture the B key so it doesn't play the cancel sound effect
		break;
	case KEY_XBUTTON_X:
	{
		QuickJoinPanel *pQuickJoin = dynamic_cast< QuickJoinPanel * >( FindChildByName( "PnlQuickJoin" ) );
		if ( pQuickJoin && pQuickJoin->ShouldBeVisible() )
		{
			// X shortcut only works if the panel is showing
			OnCommand( "SeeAll" );
		}

		break;
	}

	case KEY_XBUTTON_BACK:
#ifdef _X360
		if ( XBX_GetNumGameUsers() > 1 )
		{
			OnCommand( "DisableSplitscreen" );
		}
#endif
		break;

	case KEY_XBUTTON_INACTIVE_START:
#ifdef _X360
		if ( !XBX_GetPrimaryUserIsGuest() &&
			userId != ( int )XBX_GetPrimaryUserId() &&
			userId >= 0 &&
			CUIGameData::Get()->CanPlayer2Join() )
		{
			// Pass the index of controller which wanted to join splitscreen
			CBaseModPanel::GetSingleton().CloseAllWindows();
			CAttractScreen::SetAttractMode( CAttractScreen::ATTRACT_GOSPLITSCREEN, userId );
			CBaseModPanel::GetSingleton().OpenWindow( WT_ATTRACTSCREEN, NULL, true );
		}
#endif
		break;

	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

void MainMenu::OnThink()
{
	// don't need any of this if we're not able to run
	if ( m_bIsStub )
		return;

	// need to change state of flyout if user suddenly disconnects
	// while flyout is open
	BaseModUI::FlyoutMenu *flyout = dynamic_cast< FlyoutMenu * >( FindChildByName( "FlmCampaignFlyout" ) );
	if ( flyout )
	{
		BaseModHybridButton *pButton = dynamic_cast< BaseModHybridButton * >( flyout->FindChildButtonByCommand( "QuickMatchCoOp" ) );
		if ( pButton )
		{
			if ( !CUIGameData::Get()->SignedInToLive() )
			{
				pButton->SetText( "#L4D360UI_QuickStart" );
				if ( m_iQuickJoinHelpText != MMQJHT_QUICKSTART )
				{
					pButton->SetHelpText( "#L4D360UI_QuickMatch_Offline_Tip" );
					m_iQuickJoinHelpText = MMQJHT_QUICKSTART;
				}
			}
			else
			{
				pButton->SetText( "#L4D360UI_QuickMatch" );
				if ( m_iQuickJoinHelpText != MMQJHT_QUICKMATCH )
				{
					pButton->SetHelpText( "#L4D360UI_QuickMatch_Tip" );
					m_iQuickJoinHelpText = MMQJHT_QUICKMATCH;
				}
			}
		}
	}

	if ( IsPC() )
	{
		FlyoutMenu *pFlyout = dynamic_cast< FlyoutMenu * >( FindChildByName( "FlmOptionsFlyout" ) );
		if ( pFlyout )
		{
			const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
			pFlyout->SetControlEnabled( "BtnBrightness", !config.Windowed() );
			pFlyout->SetControlEnabled( "BtnController", g_RD_Steam_Input.GetJoystickCount() != 0 );
		}
	}

	// don't need fancy main menu stuff on the legacy version
	if ( m_bIsLegacy )
	{
		for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
			m_pTopLeaderboardEntries[i]->SetVisible( false );
		m_pBtnHoIAFTimer->SetVisible( false );
		for ( int i = 0; i < NELEMS( m_pBtnEventTimer ); i++ )
			m_pBtnEventTimer[i]->SetVisible( false );
		m_pBtnNewsShowcase->SetVisible( false );
		m_pBtnWorkshopShowcase->SetVisible( false );
		return;
	}

	uint32 iCurrentTime = SteamUtils() ? SteamUtils()->GetServerRealTime() : std::time( NULL );
	uint32 iCurrentMinute = iCurrentTime / 60;
	if ( m_iLastTimerUpdate != iCurrentMinute )
	{
		wchar_t wszTimerText[1024];
		for ( int i = NELEMS( m_pBtnEventTimer ) - 1; i >= 0; i-- )
		{
			if ( !HoIAF()->IsEventTimerActive( i ) )
			{
				m_pBtnEventTimer[i]->SetVisible( false );
				continue;
			}

			wchar_t wszEventTitle[1024];
			V_UTF8ToUnicode( HoIAF()->GetEventTimerCaption( i ), wszEventTitle, sizeof( wszEventTitle ) );

			uint32 iTimeLeftHours = ( HoIAF()->GetEventEndTime( i ) - iCurrentTime ) / 3600;
			if ( iTimeLeftHours > 23 )
				g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_event_timer_days" ), 2, wszEventTitle, UTIL_RD_CommaNumber( iTimeLeftHours / 24 ) );
			else if ( iTimeLeftHours > 0 )
				g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_event_timer_hours" ), 2, wszEventTitle, UTIL_RD_CommaNumber( iTimeLeftHours ) );
			else
				g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_event_timer_soon" ), 1, wszEventTitle );

			m_pBtnEventTimer[i]->SetText( wszTimerText );
			m_pBtnEventTimer[i]->SetVisible( true );
		}

		int iDaysRemaining, iHoursRemaining;
		int iSeasonNumber = UTIL_RD_GetCurrentHoIAFSeason( &iDaysRemaining, &iHoursRemaining );

		if ( iDaysRemaining > 0 )
			g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_hoiaf_ends_days_hours" ), 3, UTIL_RD_CommaNumber( iSeasonNumber ), UTIL_RD_CommaNumber( iDaysRemaining ), UTIL_RD_CommaNumber( iHoursRemaining ) );
		else if ( iHoursRemaining > 0 )
			g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_hoiaf_ends_hours" ), 2, UTIL_RD_CommaNumber( iSeasonNumber ), UTIL_RD_CommaNumber( iHoursRemaining ) );
		else
			g_pVGuiLocalize->ConstructString( wszTimerText, sizeof( wszTimerText ), g_pVGuiLocalize->Find( "#rd_hoiaf_ends_soon" ), 1, UTIL_RD_CommaNumber( iSeasonNumber ) );

		m_pBtnHoIAFTimer->SetText( wszTimerText );
		m_pBtnHoIAFTimer->SetVisible( true );

		m_pBtnWorkshopShowcase->SetText( m_wszWorkshopTrendingTitle[iCurrentMinute % NELEMS( m_wszWorkshopTrendingTitle )] );
		m_pBtnWorkshopShowcase->SetVisible( true );

		int iNumNewsShowcases = HoIAF()->CountFeaturedNews();
		if ( iNumNewsShowcases == 0 )
		{
			m_pBtnNewsShowcase->SetVisible( false );
		}
		else
		{
			wchar_t wszNewsTitle[1024];
			V_UTF8ToUnicode( HoIAF()->GetFeaturedNewsCaption( iCurrentMinute % iNumNewsShowcases ), wszNewsTitle, sizeof( wszNewsTitle ) );
			m_pBtnNewsShowcase->SetText( wszNewsTitle );
			m_pBtnNewsShowcase->SetVisible( true );
		}

		m_iLastTimerUpdate = iCurrentMinute;
	}

	if ( m_bGrabPanelLocations )
	{
		int discard;

		m_bHavePanelLocations = true;

		// top
		m_pTopBar->GetPos( discard, m_iTargetYTopBar );

		// left
		m_pCommanderProfile->GetPos( m_iTargetXCommanderProfile, discard );
		m_pBtnMultiplayer->GetPos( m_iTargetXCreateLobby, discard );
		m_pBtnSingleplayer->GetPos( m_iTargetXSingleplayer, discard );
		m_pPnlQuickJoinPublic->GetPos( m_iTargetXQuickJoinPublic, discard );
		m_pPnlQuickJoin->GetPos( m_iTargetXQuickJoinFriends, discard );
		m_pBtnWorkshopShowcase->GetPos( m_iTargetXWorkshopShowcase, discard );

		// right
		for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
			m_pTopLeaderboardEntries[i]->GetPos( m_iTargetXHoIAFLeaderboard[i], discard );
		m_pBtnHoIAFTimer->GetPos( m_iTargetXHoIAFTimer, discard );
		for ( int i = 0; i < NELEMS( m_pBtnEventTimer ); i++ )
			m_pBtnEventTimer[i]->GetPos( m_iTargetXEventTimer[i], discard );
		m_pBtnNewsShowcase->GetPos( m_iTargetXNewsShowcase, discard );
		m_pBtnUpdateNotes->GetPos( m_iTargetXUpdateNotes, discard );
#ifdef RD_7A_DROPS
		m_pCraftingMaterialsBetaPromoButton->GetPos( m_iTargetXCraftingMaterialsBetaPromo, discard );
#endif

		// bottom
		m_pStockTickerHelper->GetPos( discard, m_iTargetYStockTicker );
	}

	if ( g_pInputSystem->GetEventCount() )
		m_flLastActiveTime = Plat_FloatTime();
	m_flLastActiveTime = MAX( m_flLastActiveTime, g_RD_Steam_Input.m_flLastInputTime );

	if ( m_bHavePanelLocations )
	{
		bool bWasMin = m_iInactiveHideMainMenu == 0;
		bool bWasMax = m_iInactiveHideMainMenu == 65535;
		bool bMenuActive = rd_main_menu_idle_timeout.GetFloat() <= 0 || Plat_FloatTime() - m_flLastActiveTime < rd_main_menu_idle_timeout.GetFloat();
		if ( m_pTopBar->m_pBtnNotifications->m_hListPopOut && m_pTopBar->m_pBtnNotifications->m_hListPopOut->IsVisible() )
		{
			bMenuActive = true;
		}
		if ( rd_reduce_motion.GetBool() )
		{
			m_iInactiveHideMainMenu = bMenuActive ? 65535 : 0;
		}
		else
		{
			int iSlide = m_iInactiveHideMainMenu;
			float flRampTime = bMenuActive ? rd_main_menu_slide_in_time.GetFloat() : rd_main_menu_slide_out_time.GetFloat();
			if ( flRampTime <= 0 )
				iSlide = bMenuActive ? 65535 : 0;
			else if ( bMenuActive )
				iSlide += gpGlobals->absoluteframetime / flRampTime * 65535;
			else
				iSlide -= gpGlobals->absoluteframetime / flRampTime * 65535;

			m_iInactiveHideMainMenu = uint16( clamp<int>( iSlide, 0, 65535 ) );
		}

		s_bMainMenuShown = m_iInactiveHideMainMenu == 65535;

		m_InactiveHideQuickJoinPublic.Update( m_pPnlQuickJoinPublic->IsVisible() );
		m_InactiveHideQuickJoinFriends.Update( m_pPnlQuickJoin->IsVisible() );

		s_iLastQuickJoinPublicVisible = m_InactiveHideQuickJoinPublic.m_iGlow;
		s_iLastQuickJoinFriendsVisible = m_InactiveHideQuickJoinFriends.m_iGlow;

		uint16 iSlideState = m_iInactiveHideMainMenu;
		if ( m_pBranchDisclaimer )
			m_pBranchDisclaimer->SetAlpha( RemapValClamped( iSlideState, 32768, 65535, 0, 255 ) );

		int x, y;
		int offx = ScreenWidth() / 2;
		int offy = ScreenHeight() / 8;
#define UPDATE_PANEL_SLIDE( pPanel, iTarget, axis, sign, start, end ) \
			if ( !pPanel ) ; \
			else \
			{ \
				pPanel->GetPos( x, y ); \
				axis = iTarget + RemapValClamped( iSlideState / 65535.0f, start, end, sign off##axis, 0 ); \
				pPanel->SetPos( x, y ); \
			}
		if ( m_bGrabPanelLocations || ( !( bWasMin && iSlideState == 0 ) && !( bWasMax && iSlideState == 65535 ) ) )
		{
			UPDATE_PANEL_SLIDE( m_pTopBar, m_iTargetYTopBar, y, -, 0.65f, 0.95f );
			UPDATE_PANEL_SLIDE( m_pCommanderProfile, m_iTargetXCommanderProfile, x, -, 0.2f, 0.8f );
			UPDATE_PANEL_SLIDE( m_pBtnMultiplayer, m_iTargetXCreateLobby, x, -, 0.25f, 0.85f );
			UPDATE_PANEL_SLIDE( m_pBtnSingleplayer, m_iTargetXSingleplayer, x, -, 0.3f, 0.9f );
			UPDATE_PANEL_SLIDE( m_pBtnWorkshopShowcase, m_iTargetXWorkshopShowcase, x, -, 0.2f, 0.8f );
			UPDATE_PANEL_SLIDE( m_pTopLeaderboardEntries[0], m_iTargetXHoIAFLeaderboard[0], x, +, 0.0f, 0.6f );
			for ( int i = 1; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
				UPDATE_PANEL_SLIDE( m_pTopLeaderboardEntries[i], m_iTargetXHoIAFLeaderboard[i], x, +, 0.1f + 0.025f * i, 0.7f + 0.025f * i );
			UPDATE_PANEL_SLIDE( m_pBtnHoIAFTimer, m_iTargetXHoIAFTimer, x, +, 0.4f, 1.0f );
			for ( int i = 0; i < NELEMS( m_pBtnEventTimer ); i++ )
				UPDATE_PANEL_SLIDE( m_pBtnEventTimer[i], m_iTargetXEventTimer[i], x, +, 0.1f + 0.1f * i, 0.7f + 0.1f * i );
			UPDATE_PANEL_SLIDE( m_pBtnNewsShowcase, m_iTargetXNewsShowcase, x, +, 0.1f, 0.7f );
			UPDATE_PANEL_SLIDE( m_pBtnUpdateNotes, m_iTargetXUpdateNotes, x, +, 0.1f, 0.7f );
#ifdef RD_7A_DROPS
			UPDATE_PANEL_SLIDE( m_pCraftingMaterialsBetaPromoButton, m_iTargetXCraftingMaterialsBetaPromo, x, +, 0.1f, 0.7f );
#endif
			UPDATE_PANEL_SLIDE( m_pStockTickerHelper, m_iTargetYStockTicker, y, +, 0.0f, 0.6f );
		}

		iSlideState = uint16( ( uint32( m_iInactiveHideMainMenu ) * uint32( m_InactiveHideQuickJoinPublic.m_iGlow ) ) >> 16 );
		UPDATE_PANEL_SLIDE( m_pPnlQuickJoinPublic, m_iTargetXQuickJoinPublic, x, -, 0.35f, 0.95f );
		iSlideState = uint16( ( uint32( m_iInactiveHideMainMenu ) * uint32( m_InactiveHideQuickJoinFriends.m_iGlow ) ) >> 16 );
		UPDATE_PANEL_SLIDE( m_pPnlQuickJoin, m_iTargetXQuickJoinFriends, x, -, 0.4f, 1.0f );
	}

	m_bGrabPanelLocations = false;

	if ( g_hBriefingTooltip )
		g_hBriefingTooltip->SetTooltipIgnoresCursor( false );

	if ( m_pTopBar->m_pBtnNotifications->m_hListPopOut && m_pTopBar->m_pBtnNotifications->m_hListPopOut->IsVisible() )
	{
		// no tooltip if we have the notifications menu open
	}
	else if ( m_iInactiveHideMainMenu == 65535 )
	{
		MaybeShowTooltip( m_pTopBar->m_pBtnSettings, "#rd_main_menu_tip_settings_title", "#rd_main_menu_tip_settings", 0.0f, 1.0f, vgui::Label::a_northwest );
		MaybeShowTooltip( m_pTopBar->m_pTopButton[CRD_VGUI_Main_Menu_Top_Bar::BTN_LOADOUTS], "#rd_main_menu_tip_loadout_title", "#rd_main_menu_tip_loadout", 0.5f, 1.0f, vgui::Label::a_north );
#ifdef RD_7A_QUESTS
		MaybeShowTooltip( m_pTopBar->m_pTopButton[CRD_VGUI_Main_Menu_Top_Bar::BTN_CONTRACTS], "#rd_main_menu_tip_contracts_title", "#rd_main_menu_tip_contracts", 0.5f, 1.0f, vgui::Label::a_north );
#endif
		MaybeShowTooltip( m_pTopBar->m_pTopButton[CRD_VGUI_Main_Menu_Top_Bar::BTN_RECORDINGS], "#rd_main_menu_tip_recordings_title", "#rd_main_menu_tip_recordings", 0.5f, 1.0f, vgui::Label::a_north );
		MaybeShowTooltip( m_pTopBar->m_pTopButton[CRD_VGUI_Main_Menu_Top_Bar::BTN_SWARMOPEDIA], "#rd_main_menu_tip_swarmopedia_title", "#rd_main_menu_tip_swarmopedia", 0.5f, 1.0f, vgui::Label::a_north );
		MaybeShowTooltip( m_pTopBar->m_pTopButton[CRD_VGUI_Main_Menu_Top_Bar::BTN_WORKSHOP], "#rd_main_menu_tip_workshop_title", "#rd_main_menu_tip_workshop", 0.5f, 1.0f, vgui::Label::a_north );
		MaybeShowTooltip( m_pTopBar->m_pTopButton[CRD_VGUI_Main_Menu_Top_Bar::BTN_INVENTORY], "#rd_main_menu_tip_inventory_title", "#rd_main_menu_tip_inventory", 0.5f, 1.0f, vgui::Label::a_north );
		MaybeShowTooltip( m_pTopBar->m_pBtnNotifications, "#rd_main_menu_tip_notifications_title", "#rd_main_menu_tip_notifications", 1.0f, 1.0f, vgui::Label::a_northeast );
		MaybeShowTooltip( m_pTopBar->m_pBtnQuit, "#rd_main_menu_tip_quit_title", "#rd_main_menu_tip_quit", 1.0f, 1.0f, vgui::Label::a_northeast );
#ifdef RD_7A_QUESTS
		MaybeShowTooltip( m_pCommanderProfile, "#rd_main_menu_tip_commander_profile_title", "#rd_main_menu_tip_commander_profile", 1.0f, 0.5f, vgui::Label::a_west );
#else
		MaybeShowTooltip( m_pCommanderProfile, "#rd_main_menu_tip_commander_profile_title", "#rd_main_menu_tip_commander_profile_2024", 1.0f, 0.5f, vgui::Label::a_west );
#endif
		MaybeShowTooltip( m_pBtnSingleplayer, "#rd_main_menu_tip_single_player_title", "#rd_main_menu_tip_single_player", 1.0f, 0.5f, vgui::Label::a_west );
		MaybeShowTooltip( m_pPnlQuickJoinPublic, "#rd_main_menu_tip_quick_join_public_title", "#rd_main_menu_tip_quick_join_public", 1.0f, 0.5f, vgui::Label::a_west );
		MaybeShowTooltip( m_pPnlQuickJoin, "#rd_main_menu_tip_quick_join_friends_title", "#rd_main_menu_tip_quick_join_friends", 1.0f, 0.5f, vgui::Label::a_west );
		MaybeShowTooltip( m_pBtnWorkshopShowcase, "#rd_main_menu_tip_workshop_showcase_title", "#rd_main_menu_tip_workshop_showcase", 1.0f, 0.5f, vgui::Label::a_west );
		for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
			MaybeShowTooltip( m_pTopLeaderboardEntries[i], "#rd_main_menu_tip_hoiaf_title", rd_hoiaf_leaderboard_friends_only.GetBool() ? "#rd_main_menu_tip_hoiaf_leaderboard_friends" : "#rd_main_menu_tip_hoiaf_leaderboard_top10", 0.0f, 0.5f, vgui::Label::a_east );
		MaybeShowTooltip( m_pBtnHoIAFTimer, "#rd_main_menu_tip_hoiaf_title", "#rd_main_menu_tip_hoiaf_timer", 0.0f, 0.5f, vgui::Label::a_east );
		for ( int i = 0; i < NELEMS( m_pBtnEventTimer ); i++ )
			MaybeShowTooltip( m_pBtnEventTimer[i], "#rd_main_menu_tip_event_timer_title", "#rd_main_menu_tip_event_timer", 0.0f, 0.5f, vgui::Label::a_east );
		MaybeShowTooltip( m_pBtnNewsShowcase, "#rd_main_menu_tip_news_showcase_title", "#rd_main_menu_tip_news_showcase", 0.0f, 0.5f, vgui::Label::a_east );
		MaybeShowTooltip( m_pBtnUpdateNotes, "#rd_main_menu_tip_update_notes_title", "#rd_main_menu_tip_update_notes", 0.0f, 0.5f, vgui::Label::a_east );
#ifdef RD_7A_DROPS
		MaybeShowTooltip( m_pCraftingMaterialsBetaPromoButton, "vi'ecpe do", "ti xatra do la briju voi'e tugygau\n.i la SENARIOS noi qocre fizde lo nu do spuda", 0.0f, 0.5f, vgui::Label::a_east, true );
#endif
	}

	BaseClass::OnThink();
}

void MainMenu::OnOpen()
{
	extern ConVar *sv_cheats;
	if ( !sv_cheats )
	{
		sv_cheats = cvar->FindVar( "sv_cheats" );
	}

	if ( sv_cheats && !sv_cheats->GetBool() )
	{
		if ( rd_revert_convars.GetBool() )
		{
			g_pCVar->RevertFlaggedConVars( FCVAR_REPLICATED );
		}

		g_pCVar->RevertFlaggedConVars( FCVAR_CHEAT );

		if ( rd_revert_convars.GetBool() )
		{
			engine->ClientCmd_Unrestricted( "execifexists autoexec\n" );

			g_ReactiveDropWorkshop.RerunAutoExecScripts();
		}
	}

	// Apply mixer convars.
	engine->ClientCmd_Unrestricted( "_rd_mixer_init\n" );

	if ( IsPC() && connect_lobby.GetString()[0] )
	{
		// if we were launched with "+connect_lobby <lobbyid>" on the command line, join that lobby immediately
		uint64 nLobbyID = _atoi64( connect_lobby.GetString() );
		if ( nLobbyID != 0 )
		{
			KeyValues *pSettings = KeyValues::FromString(
				"settings",
				" system { "
				" network LIVE "
				" } "
				" options { "
				" action joinsession "
				" } "
			);
			pSettings->SetUint64( "options/sessionid", nLobbyID );
			KeyValues::AutoDelete autodelete( pSettings );

			g_pMatchFramework->MatchSession( pSettings );
		}
		// clear the convar so we don't try to join that lobby every time we return to the main menu
		connect_lobby.SetValue( "" );
	}

	m_iInactiveHideMainMenu = s_bMainMenuShown ? 65535 : 0;
	m_InactiveHideQuickJoinPublic.m_iGlow = s_iLastQuickJoinPublicVisible;
	m_InactiveHideQuickJoinFriends.m_iGlow = s_iLastQuickJoinFriendsVisible;
	if ( s_bMainMenuShown )
		m_flLastActiveTime = Plat_FloatTime();

	BaseClass::OnOpen();

	SetFooterState();

#ifndef _X360
	bool bSteamCloudVisible = false;

	{
		static CGameUIConVarRef cl_cloud_settings( "cl_cloud_settings" );
		if ( cl_cloud_settings.GetInt() == -1 )
		{
#if 0
			CBaseModPanel::GetSingleton().OpenWindow( WT_STEAMCLOUDCONFIRM, this, false );
			bSteamCloudVisible = true;
#else
			// Default to allowing Steam Cloud; if the user doesn't want it, they can disable
			// it in game properties or via the menu after initial game launch.
			cl_cloud_settings.SetValue( STEAMREMOTESTORAGE_CLOUD_ALL );
#endif
		}
	}

	if ( !bSteamCloudVisible )
	{
		if ( AddonAssociation::CheckAndSeeIfShouldShow() )
		{
			CBaseModPanel::GetSingleton().OpenWindow( WT_ADDONASSOCIATION, this, false );
		}
	}
#endif
}

void MainMenu::RunFrame()
{
	BaseClass::RunFrame();
}

void MainMenu::PaintBackground()
{
	if ( m_bIsStub || m_bIsLegacy )
		return;

	HUD_SHEET_DRAW_PANEL( m_pBtnMultiplayer, MainMenuSheet, UV_create_lobby );
	HUD_SHEET_DRAW_PANEL( m_pBtnSingleplayer, MainMenuSheet, UV_singleplayer );
	{
		// always draw quick join panel backgrounds (they are moved off the screen when inactive)
		int x0, y0, x1, y1;
		m_pPnlQuickJoinPublic->GetBounds( x0, y0, x1, y1 );
		HUD_SHEET_DRAW_BOUNDS( MainMenuSheet, UV_quick_join );
		m_pPnlQuickJoin->GetBounds( x0, y0, x1, y1 );
		HUD_SHEET_DRAW_BOUNDS( MainMenuSheet, UV_quick_join );
	}
	HUD_SHEET_DRAW_PANEL( m_pBtnWorkshopShowcase, MainMenuSheet, UV_workshop );
	for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
	{
		if ( !m_pTopLeaderboardEntries[i]->IsVisible() )
			continue;

		int x0, y0, x1, y1;
		m_pTopLeaderboardEntries[i]->GetBounds( x0, y0, x1, y1 );

		// add 1 pixel on each side so these don't look weird with rounding errors
		x0--;
		y0--;
		x1 += 2;
		y1 += 2;

		if ( i == 0 )
			HUD_SHEET_DRAW_BOUNDS( MainMenuSheet, UV_hoiaf_top_1 );
		else
			HUD_SHEET_DRAW_BOUNDS( MainMenuSheet, UV_hoiaf_top_10 );
	}
	HUD_SHEET_DRAW_PANEL( m_pBtnHoIAFTimer, MainMenuSheet, UV_hoiaf_timer );
	for ( int i = 0; i < NELEMS( m_pBtnEventTimer ); i++ )
	{
		HUD_SHEET_DRAW_PANEL( m_pBtnEventTimer[i], MainMenuSheet, UV_event_timer );
	}
	HUD_SHEET_DRAW_PANEL( m_pBtnNewsShowcase, MainMenuSheet, UV_news );
	HUD_SHEET_DRAW_PANEL( m_pBtnUpdateNotes, MainMenuSheet, UV_update );

	int iCurrentMinute = ( ( SteamUtils() ? SteamUtils()->GetServerRealTime() : std::time( NULL ) ) / 60 );

	int iNumNewsShowcases = HoIAF()->CountFeaturedNews();
	if ( iNumNewsShowcases && m_pBtnNewsShowcase->IsVisible() )
	{
		vgui::surface()->DrawSetTexture( HoIAF()->GetFeaturedNewsTexture( iCurrentMinute % iNumNewsShowcases ) );
		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );

		int x0, y0, x1, y1;
		m_pBtnNewsShowcase->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedRect( x0 + YRES( 1 ), y0 + YRES( 1 ), x0 + x1 - YRES( 1 ), y0 + y1 - YRES( 1 ) );
	}

	int iCurrentWorkshopShowcase = iCurrentMinute % NELEMS( m_pWorkshopTrendingPreview );
	if ( m_pBtnWorkshopShowcase->IsVisible() && m_pWorkshopTrendingPreview[iCurrentWorkshopShowcase] )
	{
		vgui::surface()->DrawSetTexture( m_pWorkshopTrendingPreview[iCurrentWorkshopShowcase]->GetID() );
		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );

		int iw, it, cw, ct;
		vgui::surface()->DrawGetTextureSize( m_pWorkshopTrendingPreview[iCurrentWorkshopShowcase]->GetID(), iw, it );
		m_pWorkshopTrendingPreview[iCurrentWorkshopShowcase]->GetContentSize( cw, ct );

		int x0, y0, x1, y1;
		m_pBtnWorkshopShowcase->GetBounds( x0, y0, x1, y1 );
		vgui::surface()->DrawTexturedSubRect( x0 + YRES( 1 ), y0 + YRES( 1 ), x0 + x1 - YRES( 1 ), y0 + y1 - YRES( 1 ), 0, 0, cw / ( float )iw, ct / ( float )it );
	}

	m_GlowCreateLobby.Update( m_pBtnMultiplayer->GetCurrentState() == BaseModHybridButton::Focus );
	m_GlowSingleplayer.Update( m_pBtnSingleplayer->GetCurrentState() == BaseModHybridButton::Focus );
	m_GlowQuickJoinPublic.Update( m_pPnlQuickJoinPublic->HasMouseover() );
	m_GlowQuickJoinFriends.Update( m_pPnlQuickJoin->HasMouseover() );
	m_GlowWorkshopShowcase.Update( m_pBtnWorkshopShowcase->GetCurrentState() == BaseModHybridButton::Focus );

	m_GlowHoIAFTimer.Update( m_pBtnHoIAFTimer->GetCurrentState() == BaseModHybridButton::Focus );
	for ( int i = 0; i < NELEMS( m_pBtnEventTimer ); i++ )
		m_GlowEventTimer[i].Update( m_pBtnEventTimer[i]->GetCurrentState() == BaseModHybridButton::Focus );
	m_GlowNewsShowcase.Update( m_pBtnNewsShowcase->GetCurrentState() == BaseModHybridButton::Focus );
	m_GlowUpdateNotes.Update( m_pBtnUpdateNotes->GetCurrentState() == BaseModHybridButton::Focus );

	float flAlphaAdjust = RemapValClamped( m_iInactiveHideMainMenu, 49152.0f, 65535.0f, 0.0f, 1.0f );

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnMultiplayer, MainMenuAdditive, UV_create_lobby_hover, m_GlowCreateLobby.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnMultiplayer, MainMenuAdditive, UV_create_lobby_logo_hover, m_pTopBar->m_GlowLogo.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnMultiplayer, MainMenuAdditive, UV_create_lobby_singleplayer_hover, m_GlowSingleplayer.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnMultiplayer, MainMenuAdditive, UV_create_lobby_profile_hover, m_pCommanderProfile->m_GlowHover.Get() * flAlphaAdjust );
	if ( m_pTopBar->m_hActiveButton.Get() == m_pTopBar->m_pBtnLogo )
		HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnMultiplayer, MainMenuAdditive, UV_create_lobby_logo_hover, 255 * flAlphaAdjust );

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnSingleplayer, MainMenuAdditive, UV_singleplayer_hover, m_GlowSingleplayer.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnSingleplayer, MainMenuAdditive, UV_singleplayer_create_lobby_hover, m_GlowCreateLobby.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnSingleplayer, MainMenuAdditive, UV_singleplayer_quick_join_hover, m_GlowQuickJoinPublic.Get() * flAlphaAdjust );

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pPnlQuickJoinPublic, MainMenuAdditive, UV_quick_join_hover, m_GlowQuickJoinPublic.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pPnlQuickJoinPublic, MainMenuAdditive, UV_quick_join_below_hover, m_GlowQuickJoinFriends.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pPnlQuickJoinPublic, MainMenuAdditive, UV_quick_join_singleplayer_hover, m_GlowSingleplayer.Get() * flAlphaAdjust );

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pPnlQuickJoin, MainMenuAdditive, UV_quick_join_hover, m_GlowQuickJoinFriends.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pPnlQuickJoin, MainMenuAdditive, UV_quick_join_above_hover, m_GlowQuickJoinPublic.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pPnlQuickJoin, MainMenuAdditive, UV_quick_join_below_hover, m_GlowWorkshopShowcase.Get() * flAlphaAdjust );

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnWorkshopShowcase, MainMenuAdditive, UV_workshop_hover, m_GlowWorkshopShowcase.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnWorkshopShowcase, MainMenuAdditive, UV_workshop_quick_join_hover, m_GlowQuickJoinFriends.Get() * flAlphaAdjust );

	for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
	{
		// add 1 pixel on each side so these don't look weird with rounding errors
		int x0, y0, x1, y1;
		m_pTopLeaderboardEntries[i]->GetBounds( x0, y0, x1, y1 );
		x0--;
		y0--;
		x1 += 2;
		y1 += 2;

		if ( i == 0 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_1_hover, m_pTopLeaderboardEntries[i]->m_GlowHover.Update( m_pTopLeaderboardEntries[i]->GetCurrentState() == BaseModHybridButton::Focus ) * flAlphaAdjust );
		else
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_hover, m_pTopLeaderboardEntries[i]->m_GlowHover.Update( m_pTopLeaderboardEntries[i]->GetCurrentState() == BaseModHybridButton::Focus ) * flAlphaAdjust );

		if ( i == 0 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_1_quit_hover, m_pTopBar->m_GlowQuit.Get() * flAlphaAdjust );
		else if ( i == 1 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_quit_hover_1, m_pTopBar->m_GlowQuit.Get() * flAlphaAdjust );
		else if ( i == 2 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_quit_hover_2, m_pTopBar->m_GlowQuit.Get() * flAlphaAdjust );
		else if ( i == 3 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_quit_hover_3, m_pTopBar->m_GlowQuit.Get() * flAlphaAdjust );
		else if ( i == 4 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_quit_hover_4, m_pTopBar->m_GlowQuit.Get() * flAlphaAdjust );
		else if ( i == 5 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_quit_hover_5, m_pTopBar->m_GlowQuit.Get() * flAlphaAdjust );
		else if ( i == 6 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_quit_hover_6, m_pTopBar->m_GlowQuit.Get() * flAlphaAdjust );
		else if ( i == 7 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_quit_hover_7, m_pTopBar->m_GlowQuit.Get() * flAlphaAdjust );
		else if ( i == 8 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_quit_hover_8, m_pTopBar->m_GlowQuit.Get() * flAlphaAdjust );

		// some of these are drawn one frame late, but that's also true of buttons in other PaintBackground scopes, so no point in adding more complex code for this.

		if ( i == 0 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_1_below_hover, m_pTopLeaderboardEntries[1]->m_GlowHover.Get() * flAlphaAdjust );
		if ( i < NELEMS( m_pTopLeaderboardEntries ) - 1 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_below_hover, m_pTopLeaderboardEntries[i + 1]->m_GlowHover.Get() * flAlphaAdjust );
		if ( i > 0 )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_above_hover, m_pTopLeaderboardEntries[i - 1]->m_GlowHover.Get() * flAlphaAdjust );
		else if ( ( i == NELEMS( m_pTopLeaderboardEntries ) - 1 || !m_pTopLeaderboardEntries[i + 1]->IsVisible() ) )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_hoiaf_top_10_hoiaf_timer_hover, m_GlowHoIAFTimer.Get() * flAlphaAdjust );
	}

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnHoIAFTimer, MainMenuAdditive, UV_hoiaf_timer_hover, m_GlowHoIAFTimer.Get() * flAlphaAdjust );
	if ( m_pTopLeaderboardEntries[NELEMS( m_pTopLeaderboardEntries ) - 1]->IsVisible() )
		HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnHoIAFTimer, MainMenuAdditive, UV_hoiaf_timer_event_timer_hover, m_GlowEventTimer[NELEMS( m_GlowEventTimer ) - 1].Get() * flAlphaAdjust );
	if ( !m_pTopLeaderboardEntries[0]->IsVisible() )
		HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnHoIAFTimer, MainMenuAdditive, UV_hoiaf_top_1_quit_hover, m_pTopBar->m_GlowQuit.Get() * flAlphaAdjust );

	for ( int i = NELEMS( m_pTopLeaderboardEntries ) - 1; i >= 0; i-- )
	{
		if ( m_pTopLeaderboardEntries[i]->IsVisible() )
		{
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnHoIAFTimer, MainMenuAdditive, UV_hoiaf_timer_hoiaf_top_10_hover, m_pTopLeaderboardEntries[i]->m_GlowHover.Get() * flAlphaAdjust );
			break;
		}
	}

	for ( int i = 0; i < NELEMS( m_pBtnEventTimer ); i++ )
	{
		HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnEventTimer[i], MainMenuAdditive, UV_event_timer_hover, m_GlowEventTimer[i].Get() * flAlphaAdjust );
		if ( i == 0 )
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnEventTimer[i], MainMenuAdditive, UV_event_timer_news_hover, m_GlowNewsShowcase.Get() * flAlphaAdjust );
		if ( i == NELEMS( m_pBtnEventTimer ) - 1 && m_pTopLeaderboardEntries[NELEMS( m_pTopLeaderboardEntries ) - 1]->IsVisible() )
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnEventTimer[i], MainMenuAdditive, UV_event_timer_hoiaf_timer_hover, m_GlowHoIAFTimer.Get() * flAlphaAdjust );
		if ( i > 0 )
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnEventTimer[i], MainMenuAdditive, UV_event_timer_below_hover, m_GlowEventTimer[i - 1].Get() * flAlphaAdjust );
		if ( i < NELEMS( m_pBtnEventTimer ) - 1 )
			HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnEventTimer[i], MainMenuAdditive, UV_event_timer_above_hover, m_GlowEventTimer[i + 1].Get() * flAlphaAdjust );
	}

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnNewsShowcase, MainMenuAdditive, UV_news_hover, m_GlowNewsShowcase.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnNewsShowcase, MainMenuAdditive, UV_news_update_hover, m_GlowUpdateNotes.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnNewsShowcase, MainMenuAdditive, UV_news_event_timer_hover, m_GlowEventTimer[0].Get() * flAlphaAdjust );

	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnUpdateNotes, MainMenuAdditive, UV_update_hover, m_GlowUpdateNotes.Get() * flAlphaAdjust );
	HUD_SHEET_DRAW_PANEL_ALPHA( m_pBtnUpdateNotes, MainMenuAdditive, UV_update_news_hover, m_GlowNewsShowcase.Get() * flAlphaAdjust );

	m_pTopBar->m_iLeftGlow = m_pCommanderProfile->m_GlowHover.Get() * flAlphaAdjust;
	m_pTopBar->m_iRightGlow = ( m_pTopLeaderboardEntries[0]->IsVisible() ? m_pTopLeaderboardEntries[0]->m_GlowHover.Get() : m_GlowHoIAFTimer.Get() ) * flAlphaAdjust;
	m_pStockTickerHelper->m_iLeftGlow = m_GlowWorkshopShowcase.Get() * flAlphaAdjust;
	m_pStockTickerHelper->m_iRightGlow = m_GlowUpdateNotes.Get() * flAlphaAdjust;
}

void MainMenu::AcceptCommentaryRulesCallback() 
{
	if ( MainMenu *pMainMenu = static_cast< MainMenu* >( CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ) ) )
	{
		KeyValues *pSettings = KeyValues::FromString(
			"settings",
			" system { "
				" network offline "
			" } "
			" game { "
				" mode single_mission "
			" } "
			" options { "
				" play commentary "
			" } "
			);
		KeyValues::AutoDelete autodelete( pSettings );

		g_pMatchFramework->CreateSession( pSettings );
	}

}

void MainMenu::AcceptSplitscreenDisableCallback()
{
	if ( MainMenu *pMainMenu = static_cast< MainMenu* >( CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ) ) )
	{
		pMainMenu->OnCommand( "DisableSplitscreen_NoConfirm" );
	}
}

void MainMenu::AcceptVersusSoftLockCallback()
{
	if ( MainMenu *pMainMenu = static_cast< MainMenu* >( CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ) ) )
	{
		pMainMenu->OnCommand( "FlmVersusFlyout" );
	}
}

void MainMenu::AcceptQuitGameCallback()
{
	if ( MainMenu *pMainMenu = static_cast< MainMenu * >( CBaseModPanel::GetSingleton().GetWindow( WT_MAINMENU ) ) )
	{
		pMainMenu->OnCommand( "QuitGame_NoConfirm" );
	}
}

void MainMenu::SetFooterState()
{
	CBaseModFooterPanel *footer = BaseModUI::CBaseModPanel::GetSingleton().GetFooterPanel();
	if ( footer )
	{
		CBaseModFooterPanel::FooterButtons_t buttons = FB_ABUTTON;
#if defined( _X360 )
		if ( XBX_GetPrimaryUserIsGuest() == 0 )
		{
			buttons |= FB_XBUTTON;
		}
#endif

		footer->SetButtons( buttons, FF_MAINMENU, false );
		footer->SetButtonText( FB_ABUTTON, "#L4D360UI_Select" );
		footer->SetButtonText( FB_XBUTTON, "#L4D360UI_MainMenu_SeeAll" );
	}
}

void MainMenu::OpenNewsURL( const char *szURL )
{
	if ( !szURL )
		return;

	char szFormattedURL[1024];
	V_snprintf( szFormattedURL, sizeof( szFormattedURL ), szURL, SteamApps() ? SteamApps()->GetCurrentGameLanguage() : "" );
	CUIGameData::Get()->ExecuteOverlayUrl( szFormattedURL );
}

void MainMenu::MaybeShowTooltip( vgui::Panel *pPanel, const char *szTitle, const char *szDescription, float flWidthBias, float flHeightBias, vgui::Label::Alignment iAlignment, bool bZbalermorna )
{
	bool bFocused = pPanel->HasFocus();
	if ( BaseModHybridButton *pHybridButton = dynamic_cast< BaseModHybridButton * >( pPanel ) )
	{
		bFocused = pHybridButton->GetCurrentState() == BaseModHybridButton::Focus;
	}
	else if ( QuickJoinPanel *pQuickJoin = dynamic_cast< QuickJoinPanel * >( pPanel ) )
	{
		bFocused = pQuickJoin->HasMouseover();
	}

	if ( !bFocused )
		return;

	BriefingTooltip::EnsureParent( this );
	if ( g_hBriefingTooltip )
		g_hBriefingTooltip->SetTooltipIgnoresCursor( true );
	if ( g_hBriefingTooltip && g_hBriefingTooltip->GetTooltipPanel() != pPanel )
	{
		int x, y, w, t;

		if ( CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry *pLeaderboardEntry = dynamic_cast< CRD_VGUI_Main_Menu_HoIAF_Leaderboard_Entry * >( pPanel ) )
		{
			m_pTopLeaderboardEntries[0]->GetPos( x, y );
			w = m_pTopLeaderboardEntries[0]->GetWide();
			t = 0;
			for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
				if ( m_pTopLeaderboardEntries[i]->IsVisible() )
					t += m_pTopLeaderboardEntries[i]->GetTall();
		}
		else
		{
			pPanel->GetBounds( x, y, w, t );

			for ( int i = 0; i < NELEMS( m_pBtnEventTimer ); i++ )
			{
				if ( pPanel == m_pBtnEventTimer[i] )
				{
					m_pBtnEventTimer[0]->GetPos( x, y );
					for ( int j = 1; j < NELEMS( m_pBtnEventTimer ); j++ )
					{
						if ( m_pBtnEventTimer[j]->IsVisible() )
						{
							y -= pPanel->GetTall();
							t += pPanel->GetTall();
						}
					}

					break;
				}
			}
		}

		if ( flWidthBias == 0.0f )
			x -= YRES( 5 );
		else if ( flWidthBias == 1.0f )
			x += YRES( 5 );
		if ( flHeightBias == 0.0f )
			y -= YRES( 5 );
		else if ( flHeightBias == 1.0f )
			y += YRES( 5 );

		g_hBriefingTooltip->SetTooltip( pPanel, szTitle, szDescription, x + w * flWidthBias, y + t * flHeightBias, iAlignment, bZbalermorna );
	}
}

void MainMenu::OnHoIAFTop10ScoresDownloaded( LeaderboardScoresDownloaded_t *pParam, bool bIOFailure )
{
	s_nHoIAFCachedEntries = 0;
	for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
	{
		m_pTopLeaderboardEntries[i]->SetVisible( false );
	}

	if ( !bIOFailure && !m_bIsStub )
	{
		s_nHoIAFCachedEntries = MIN( pParam->m_cEntryCount, NELEMS( m_pTopLeaderboardEntries ) );
		for ( int i = 0; i < pParam->m_cEntryCount && i < NELEMS( m_pTopLeaderboardEntries ); i++ )
		{
			bool bOK = SteamUserStats()->GetDownloadedLeaderboardEntry( pParam->m_hSteamLeaderboardEntries, i, &s_HoIAFLeaderboardEntryCache[i], reinterpret_cast< int32 * >( &s_HoIAFLeaderboardDetailsCache[i] ), sizeof( s_HoIAFLeaderboardDetailsCache[i] ) / sizeof( int32 ) );
			Assert( bOK );
			Assert( s_HoIAFLeaderboardEntryCache[i].m_cDetails == sizeof( s_HoIAFLeaderboardDetailsCache[i] ) / sizeof( int32 ) );

			m_pTopLeaderboardEntries[i]->SetFromEntry( s_HoIAFLeaderboardEntryCache[i], s_HoIAFLeaderboardDetailsCache[i] );
			m_pTopLeaderboardEntries[i]->SetVisible( !m_bIsLegacy );
		}
	}

	if ( m_iHoIAFTimerOffset >= 0 )
	{
		int x, y, discard;
		m_pBtnHoIAFTimer->GetPos( x, discard );
		m_pTopLeaderboardEntries[0]->GetPos( discard, y );

		for ( int i = 0; i < NELEMS( m_pTopLeaderboardEntries ); i++ )
		{
			if ( !m_pTopLeaderboardEntries[i]->IsVisible() )
				break;

			m_pTopLeaderboardEntries[i]->GetPos( discard, y );
			y += m_pTopLeaderboardEntries[i]->GetTall() + m_iHoIAFTimerOffset;
		}
		m_pBtnHoIAFTimer->SetPos( x, y );
	}
}

void MainMenu::OnWorkshopTrendingItems( SteamUGCQueryCompleted_t *pParam, bool bIOFailure )
{
	if ( bIOFailure || pParam->m_eResult != k_EResultOK )
		return;

	uint32 iCurrentWorkshopShowcase = ( std::time( NULL ) / 60 ) % NELEMS( m_iWorkshopTrendingFileID );
	for ( uint32 i = 0; i < pParam->m_unNumResultsReturned && i < NELEMS( m_iWorkshopTrendingFileID ); i++ )
	{
		SteamUGCDetails_t details;
		bool bOK = SteamUGC()->GetQueryUGCResult( pParam->m_handle, i, &details );
		Assert( bOK );
		if ( bOK )
		{
			m_iWorkshopTrendingFileID[i] = details.m_nPublishedFileId;
			V_UTF8ToUnicode( details.m_rgchTitle, m_wszWorkshopTrendingTitle[i], sizeof( m_wszWorkshopTrendingTitle[i] ) );
			m_hWorkshopTrendingPreview[i] = details.m_hPreviewFile;

			SteamAPICall_t hCall = SteamRemoteStorage()->UGCDownload( details.m_hPreviewFile, 0 );
			m_WorkshopPreviewImageCallback[i].Set( hCall, this, &MainMenu::OnWorkshopPreviewImage );

			if ( iCurrentWorkshopShowcase == i )
			{
				m_pBtnWorkshopShowcase->SetText( m_wszWorkshopTrendingTitle[i] );
			}
		}
	}
}

void MainMenu::OnWorkshopPreviewImage( RemoteStorageDownloadUGCResult_t *pParam, bool bIOFailure )
{
	if ( bIOFailure || pParam->m_eResult != k_EResultOK )
		return;

	Assert( pParam->m_nAppID == SteamUtils()->GetAppID() );

	for ( int i = 0; i < NELEMS( m_hWorkshopTrendingPreview ); i++ )
	{
		if ( m_hWorkshopTrendingPreview[i] != pParam->m_hFile )
			continue;

		CUtlBuffer buf;
		int32 nBytesRead = SteamRemoteStorage()->UGCRead( pParam->m_hFile, buf.AccessForDirectRead( pParam->m_nSizeInBytes ), pParam->m_nSizeInBytes, 0, k_EUGCRead_Close );
		Assert( nBytesRead == pParam->m_nSizeInBytes );
		buf.SeekPut( CUtlBuffer::SEEK_HEAD, nBytesRead );

		delete m_pWorkshopTrendingPreview[i]; // if we had a previous image, kill it before we put the new one in
		m_pWorkshopTrendingPreview[i] = new CReactiveDropWorkshopPreviewImage{ buf };

		return;
	}

	Assert( !"unexpected workshop preview image handle on main menu" );

	// kill the file handle anyway
	SteamRemoteStorage()->UGCRead( pParam->m_hFile, NULL, 0, 0, k_EUGCRead_Close );
}

#ifndef _X360
CON_COMMAND_F( openserverbrowser, "Opens server browser", 0 )
{
	bool isSteam = IsPC() && SteamFriends() && SteamUtils();
	if ( isSteam )
	{
		// show the server browser
		g_VModuleLoader.ActivateModule("Servers");

		// if an argument was passed, that's the tab index to show, send a message to server browser to switch to that tab
		if ( args.ArgC() > 1 )
		{
			KeyValues *pKV = new KeyValues( "ShowServerBrowserPage" );
			pKV->SetInt( "page", atoi( args[1] ) );
			g_VModuleLoader.PostMessageToAllModules( pKV );
		}

#ifdef INFESTED_DLL
		KeyValues *pSchemeKV = new KeyValues( "SetCustomScheme" );
		pSchemeKV->SetString( "SchemeName", "SwarmServerBrowserScheme" );
		g_VModuleLoader.PostMessageToAllModules( pSchemeKV );
#endif
	}
}
#endif

CON_COMMAND( rd_debug_wine_version, "" )
{
#ifdef IS_WINDOWS_PC
	if ( !wine_get_version )
	{
		Msg( "Cannot find function ntdll.dll!wine_get_version - probably not running Wine.\n" );
		return;
	}

	const char *szVersion = wine_get_version();
	Msg( "Wine Version: %s\n", szVersion );
#else
	Msg( "Not running a Windows build.\n" );
#endif
}

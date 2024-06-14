#include "cbase.h"
#include "rd_vgui_commander_mini_profile.h"
#include "c_asw_steamstats.h"
#include "rd_hud_sheet.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "gameui/swarm/vmainmenu.h"
#include "rd_vgui_main_menu_top_bar.h"
#include "vgui_avatarimage.h"
#include "asw_medal_store.h"
#include "statsbar.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY( CRD_VGUI_Commander_Mini_Profile );

CRD_VGUI_Commander_Mini_Profile::CRD_VGUI_Commander_Mini_Profile( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	SetConsoleStylePanel( true );

	m_pImgAvatar = new CAvatarImagePanel( this, "ImgAvatar" );
	m_pLblPlayerName = new vgui::Label( this, "LblPlayerName", L"" );
	m_pImgPromotionIcon = new vgui::ImagePanel( this, "ImgPromotionIcon" );
	m_pExperienceBar = new StatsBar( this, "ExperienceBar" );
	m_pLblExperience = new vgui::Label( this, "LblExperience", L"" );
	m_pExperienceBar->UseExternalCounter( m_pLblExperience );
	m_pLblLevel = new vgui::Label( this, "LblLevel", L"" );
	for ( int i = 0; i < NELEMS( m_pImgMedal ); i++ )
	{
		m_pImgMedal[i] = new vgui::ImagePanel( this, VarArgs( "ImgMedal%d", i + 1 ) );
	}
	m_pImgPredictedHoIAFMedal = new vgui::ImagePanel( this, "ImgPredictedHoIAFMedal" );

	g_RD_HUD_Sheets.VidInit();
}

CRD_VGUI_Commander_Mini_Profile::~CRD_VGUI_Commander_Mini_Profile()
{
}

void CRD_VGUI_Commander_Mini_Profile::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "resource/ui/miniprofile.res" );
}

void CRD_VGUI_Commander_Mini_Profile::ApplySettings( KeyValues *pSettings )
{
	BaseClass::ApplySettings( pSettings );

	if ( !m_bEmbedded )
	{
		Assert( m_bShowLocalPlayer );
		if ( m_bShowLocalPlayer )
		{
			InitForLocalPlayer();
		}
	}
}

void CRD_VGUI_Commander_Mini_Profile::NavigateTo()
{
	BaseClass::NavigateTo();

	if ( m_bIsButton )
		RequestFocus( 0 );
}

void CRD_VGUI_Commander_Mini_Profile::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	if ( m_bIsButton && GetParent() )
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_FOCUS );

		GetParent()->NavigateToChild( this );
	}
}

void CRD_VGUI_Commander_Mini_Profile::PaintBackground()
{
	int x0, y0, x1, y1;

	x0 = y0 = 0;
	GetSize( x1, y1 );

	float flAlphaAdjust = 1.0f;

	HUD_SHEET_DRAW_BOUNDS( MainMenuSheet, UV_profile );

	CRD_VGUI_Main_Menu_Top_Bar *pTopBar = assert_cast< CRD_VGUI_Main_Menu_Top_Bar * >( FindSiblingByName( "TopBar" ) );
	BaseModUI::MainMenu *pMainMenu = dynamic_cast< BaseModUI::MainMenu * >( GetParent() );

	HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_profile_hover, m_GlowHover.Update( m_bIsButton && HasFocus() ) );

	if ( pMainMenu )
	{
		flAlphaAdjust = RemapValClamped( pMainMenu->m_iInactiveHideMainMenu, 49152.0f, 65535.0f, 0.0f, 1.0f );

		HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_profile_create_lobby_hover, pMainMenu->m_GlowCreateLobby.Get() * flAlphaAdjust );
	}

	if ( pTopBar )
	{
		HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_profile_logo_hover, pTopBar->m_GlowLogo.Get() * flAlphaAdjust );
		HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_profile_settings_hover, pTopBar->m_GlowSettings.Get() * flAlphaAdjust );

		if ( pTopBar->m_hActiveButton.Get() == pTopBar->m_pBtnLogo )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_profile_logo_hover, 255 * flAlphaAdjust );
		if ( pTopBar->m_hActiveButton.Get() == pTopBar->m_pBtnSettings )
			HUD_SHEET_DRAW_BOUNDS_ALPHA( MainMenuAdditive, UV_profile_settings_hover, 255 * flAlphaAdjust );
	}
}

void CRD_VGUI_Commander_Mini_Profile::OnCommand( const char *command )
{
	if ( FStrEq( command, "OpenProfile" ) )
	{
		BaseModUI::CBaseModFrame *pCaller = BaseModUI::CBaseModPanel::GetSingleton().GetWindow( BaseModUI::CBaseModPanel::GetSingleton().GetActiveWindowType() );
		KeyValues::AutoDelete pSettings{ "settings" };
		if ( m_bShowLocalPlayer )
			pSettings->SetBool( "showLocalPlayer", true );
		else
			pSettings->SetUint64( "steamid", m_SteamID.ConvertToUint64() );
		BaseModUI::CBaseModPanel::GetSingleton().OpenWindow( BaseModUI::WT_COMMANDERPROFILE, pCaller, true, pSettings );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_VGUI_Commander_Mini_Profile::OnMousePressed( vgui::MouseCode code )
{
	if ( code == MOUSE_LEFT )
	{
		OnCommand( "OpenProfile" );
	}
	else
	{
		BaseClass::OnMousePressed( code );
	}
}

void CRD_VGUI_Commander_Mini_Profile::OnKeyCodePressed( vgui::KeyCode code )
{
	switch ( GetBaseButtonCode( code ) )
	{
	case KEY_XBUTTON_A:
		OnCommand( "OpenProfile" );
		break;
	default:
		BaseClass::OnKeyCodePressed( code );
		break;
	}
}

void CRD_VGUI_Commander_Mini_Profile::InitForLocalPlayer()
{
	ISteamUser *pSteamUser = SteamUser();
	Assert( pSteamUser );
	if ( !pSteamUser )
		return;

	CSteamID localPlayerID = pSteamUser->GetSteamID();

	InitShared( localPlayerID );

	int iExperience = -1;
	int iPromotion = 0;

	C_ASW_Medal_Store *pMedalStore = GetMedalStore();
	Assert( pMedalStore );
	if ( pMedalStore )
	{
		iExperience = pMedalStore->GetExperience();
		iPromotion = pMedalStore->GetPromotion();
	}

	SetExperienceAndPromotion( iExperience, iPromotion );

	for ( int i = 0; i < RD_STEAM_INVENTORY_NUM_MEDAL_SLOTS; i++ )
	{
		ConVarRef var{ CFmtStr{ "rd_equipped_%s", ReactiveDropInventory::g_PlayerInventorySlotNames[i + RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL] } };
		Assert( var.IsValid() );
		SteamItemInstanceID_t id = strtoull( var.GetString(), NULL, 10 );
		if ( const ReactiveDropInventory::ItemInstance_t *pInstance = ReactiveDropInventory::GetLocalItemCache( id ) )
		{
			m_Medals[i].SetFromInstance( *pInstance );
			m_pImgMedal[i]->SetVisible( true );
			m_pImgMedal[i]->SetImage( m_Medals[i].GetIcon() );
		}
		else
		{
			m_Medals[i].Reset();
			m_pImgMedal[i]->SetVisible( false );
		}
	}
}

void CRD_VGUI_Commander_Mini_Profile::InitShared( CSteamID steamID )
{
	MakeReadyForUse();

	m_SteamID = steamID;

	ISteamFriends *pSteamFriends = SteamFriends();
	Assert( pSteamFriends );
	if ( !pSteamFriends )
		return;

	m_pImgAvatar->SetAvatarBySteamID( &steamID );

	wchar_t wszName[k_cwchPersonaNameMax + 1];
	V_UTF8ToUnicode( pSteamFriends->GetFriendPersonaName( steamID ), wszName, sizeof( wszName ) );
	m_pLblPlayerName->SetText( wszName );

	ISteamUserStats *pUserStats = SteamUserStats();
	Assert( pUserStats );
	if ( pUserStats )
	{
		SteamAPICall_t hCall = pUserStats->DownloadLeaderboardEntriesForUsers( STEAM_LEADERBOARD_HOIAF_CURRENT_SEASON, &steamID, 1 );
		m_LeaderboardScoresDownloaded.Set( hCall, this, &CRD_VGUI_Commander_Mini_Profile::OnLeaderboardScoresDownloaded );
	}
	else
	{
		SetHoIAFError();
	}
}

void CRD_VGUI_Commander_Mini_Profile::SetExperienceAndPromotion( int iExperience, int iPromotion )
{
	if ( iPromotion == 0 )
	{
		m_pImgPromotionIcon->SetVisible( false );
	}
	else
	{
		m_pImgPromotionIcon->SetVisible( true );
		m_pImgPromotionIcon->SetImage( VarArgs( "briefing/promotion_%d_LG", iPromotion ) );
	}

	if ( iExperience < 0 )
	{
		m_pExperienceBar->SetVisible( false );
		m_pLblLevel->SetVisible( false );
	}
	else
	{
		int iLevel = LevelFromXP( iExperience, iPromotion );
		wchar_t wszLevelNumber[8];
		V_snwprintf( wszLevelNumber, NELEMS( wszLevelNumber ), L"%d", iLevel + 1 );
		wchar_t wszLevelText[64];
		g_pVGuiLocalize->ConstructString( wszLevelText, sizeof( wszLevelText ), g_pVGuiLocalize->Find( "#asw_experience_level" ), 1, wszLevelNumber );

		m_pLblLevel->SetVisible( true );
		m_pLblLevel->SetText( wszLevelText );

		m_pExperienceBar->SetVisible( true );
		m_pExperienceBar->Init( iExperience, iExperience, 1.0f, true, false );
		m_pExperienceBar->ClearMinMax();
		m_pExperienceBar->AddMinMax( 0, g_iLevelExperience[0] * g_flPromotionXPScale[iPromotion] );
		for ( int i = 0; i < ASW_NUM_EXPERIENCE_LEVELS - 1; i++ )
		{
			m_pExperienceBar->AddMinMax( g_iLevelExperience[i] * g_flPromotionXPScale[iPromotion], g_iLevelExperience[i + 1] * g_flPromotionXPScale[iPromotion] );
		}
	}
}

void CRD_VGUI_Commander_Mini_Profile::SetHoIAFData( const LeaderboardEntry_t &entry, const LeaderboardScoreDetails_Points_t &details )
{
	int iSeason = UTIL_RD_GetCurrentHoIAFSeason();
	SteamItemDef_t iMedalID = 30; // HoIAF Participant (season 11+)
	int iSeasonColor = ( iSeason / 3 + 1 ) % 4;
	if ( entry.m_nGlobalRank <= 20 )
		iMedalID = 35 + iSeasonColor; // HoIAF Top 10 (season 11+)
	else if ( entry.m_nGlobalRank <= 100 )
		iMedalID = 31 + iSeasonColor; // HoIAF Elite (season 11+)

	// Fetch item def so we start loading the icon.
	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( iMedalID );
	Assert( pDef );
	if ( pDef )
	{
		m_pImgPredictedHoIAFMedal->SetVisible( true );
		m_pImgPredictedHoIAFMedal->SetImage( pDef->Icon );
		m_pImgPredictedHoIAFMedal->SetDrawColor( Color{ 224, 224, 224, 255 } );
	}

#ifdef DBGFLAG_ASSERT
	// HoIAF medal dynamic props are in this order:
	// style;season;score;rank;alien_kills;player_kills;games_won;games_lost
	Assert( pDef->CompressedDynamicProps.Count() == 8 );
	Assert( !V_strcmp( pDef->CompressedDynamicProps[0], "style" ) );
	Assert( !V_strcmp( pDef->CompressedDynamicProps[1], "season" ) );
	Assert( !V_strcmp( pDef->CompressedDynamicProps[2], "score" ) );
	Assert( !V_strcmp( pDef->CompressedDynamicProps[3], "rank" ) );
	Assert( !V_strcmp( pDef->CompressedDynamicProps[4], "alien_kills" ) );
	Assert( !V_strcmp( pDef->CompressedDynamicProps[5], "player_kills" ) );
	Assert( !V_strcmp( pDef->CompressedDynamicProps[6], "games_won" ) );
	Assert( !V_strcmp( pDef->CompressedDynamicProps[7], "games_lost" ) );
#endif

	m_PredictedHoIAFMedal.Reset();
	m_PredictedHoIAFMedal.m_iItemDefID.Set( iMedalID );
	m_PredictedHoIAFMedal.m_nCounter.Set( 0, 0 );
	m_PredictedHoIAFMedal.m_nCounter.Set( 1, iSeason );
	m_PredictedHoIAFMedal.m_nCounter.Set( 2, entry.m_nScore );
	m_PredictedHoIAFMedal.m_nCounter.Set( 3, entry.m_nGlobalRank );
	m_PredictedHoIAFMedal.m_nCounter.Set( 4, details.m_iAlienKills );
	m_PredictedHoIAFMedal.m_nCounter.Set( 5, details.m_iPlayerKills );
	m_PredictedHoIAFMedal.m_nCounter.Set( 6, details.m_iGamesWon );
	m_PredictedHoIAFMedal.m_nCounter.Set( 7, details.m_iGamesLost );
	m_bHoIAFError = false;
}

void CRD_VGUI_Commander_Mini_Profile::SetHoIAFError()
{
	m_PredictedHoIAFMedal.Reset();
	m_bHoIAFError = true;
	m_pImgPredictedHoIAFMedal->SetVisible( false );
}

void CRD_VGUI_Commander_Mini_Profile::ClearHoIAFData()
{
	m_PredictedHoIAFMedal.Reset();
	m_bHoIAFError = false;
	m_pImgPredictedHoIAFMedal->SetVisible( false );

	const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( 30 );
	Assert( pDef );
	if ( pDef )
	{
		m_pImgPredictedHoIAFMedal->SetVisible( true );
		m_pImgPredictedHoIAFMedal->SetImage( pDef->Icon );
		m_pImgPredictedHoIAFMedal->SetDrawColor( Color{ 128, 128, 128, 64 } );
	}
}

void CRD_VGUI_Commander_Mini_Profile::OnPersonaStateChange( PersonaStateChange_t *pParam )
{
	ISteamFriends *pSteamFriends = SteamFriends();
	Assert( pSteamFriends );
	if ( pSteamFriends && m_SteamID.ConvertToUint64() == pParam->m_ulSteamID )
	{
		wchar_t wszName[k_cwchPersonaNameMax + 1];
		V_UTF8ToUnicode( pSteamFriends->GetFriendPersonaName( m_SteamID ), wszName, sizeof( wszName ) );
		m_pLblPlayerName->SetText( wszName );
	}
}

void CRD_VGUI_Commander_Mini_Profile::OnLeaderboardScoresDownloaded( LeaderboardScoresDownloaded_t *pParam, bool bIOFailure )
{
	if ( bIOFailure )
	{
		SetHoIAFError();
		return;
	}

	if ( pParam->m_cEntryCount )
	{
		Assert( pParam->m_cEntryCount == 1 );
		LeaderboardEntry_t entry;
		LeaderboardScoreDetails_Points_t details;
		bool bOK = SteamUserStats()->GetDownloadedLeaderboardEntry( pParam->m_hSteamLeaderboardEntries, 0, &entry, reinterpret_cast< int32 * >( &details ), sizeof( details ) / sizeof( int32 ) );
		Assert( bOK );
		if ( bOK )
		{
			Assert( entry.m_cDetails == sizeof( details ) / sizeof( int32 ) );
			SetHoIAFData( entry, details );
		}
		else
		{
			SetHoIAFError();
		}
	}
	else
	{
		ClearHoIAFData();
	}
}

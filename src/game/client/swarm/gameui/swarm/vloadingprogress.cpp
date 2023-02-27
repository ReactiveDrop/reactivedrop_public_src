//========= Copyright © 1996-2008, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//
#include "cbase.h"
#include "VLoadingProgress.h"
#include "EngineInterface.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/ProgressBar.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include "vgui_controls/Image.h"
#include "vgui_controls/ImagePanel.h"
#include "gameui_util.h"
#include "KeyValues.h"
#include "fmtstr.h"
#include "FileSystem.h"
#include "c_asw_steamstats.h"
#include "asw_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

ConVar rd_show_leaderboard_loading( "rd_show_leaderboard_loading", "1", FCVAR_ARCHIVE, "show friends' leaderboard entries on the loading screen" );
ConVar rd_show_mission_icon_loading( "rd_show_mission_icon_loading", "1", FCVAR_ARCHIVE, "show mission icon on the loading screen" );
ConVar rd_auto_hide_mission_icon_loading( "rd_auto_hide_mission_icon_loading", "1", FCVAR_ARCHIVE, "hide the mission icon if the mission has a custom loading background" );
ConVar rd_loading_image_per_map( "rd_loading_image_per_map", "1", FCVAR_ARCHIVE, "If set to 1 each map can have its own background image during loading screen, 0 means same image for every map" );
ConVar rd_loading_status_text_visible( "rd_loading_status_text_visible", "1", FCVAR_ARCHIVE, "If set to 1 status text is visible on the loading screen." );

extern ConVar asw_skill;

static bool IsAvatarFemale( int iAvatar )
{
	return ( iAvatar == 1 ); // Zoey or Producer
}

//=============================================================================
LoadingProgress::LoadingProgress(Panel *parent, const char *panelName, LoadingWindowType eLoadingType ):
	BaseClass( parent, panelName, false, false, false ),
	m_pDefaultPosterDataKV( NULL ),
	m_LoadingWindowType( eLoadingType )
{
	memset( m_szGameMode, 0, sizeof( m_szGameMode ) );
	memset( m_wszLeaderboardTitle, 0, sizeof( m_wszLeaderboardTitle ) );
	memset( m_szLevelName, 0, sizeof( m_szLevelName ) );

	if ( IsPC() && eLoadingType == LWT_LOADINGPLAQUE )
	{
		MakePopup( false );
	}

	SetDeleteSelfOnClose( true );
	SetProportional( true );

	m_flPeakProgress = 0.0f;
	m_pProTotalProgress = NULL;
	m_pWorkingAnim = NULL;
	m_LoadingType = LT_UNDEFINED;

	m_pBGImage = NULL;
	m_pPoster = NULL;

	m_pFooter = NULL;

	// purposely not pre-caching the poster images
	// as they do not appear in-game, and are 1MB each, we will demand load them and ALWAYS discard them
	m_pMissionInfo = NULL;
	m_pChapterInfo = NULL;
	m_botFlags = 0xFF;
	
	memset( m_PlayerNames, 0, sizeof( m_PlayerNames ) );
	// const char *pPlayerNames[NUM_LOADING_CHARACTERS] = { "#L4D360UI_NamVet", "#L4D360UI_TeenGirl", "#L4D360UI_Biker", "#L4D360UI_Manager" };
	const char *pPlayerNames[NUM_LOADING_CHARACTERS] = { "", "", "", "" };
	for ( int k = 0; k < NUM_LOADING_CHARACTERS; ++ k )
		Q_strncpy( m_PlayerNames[k], pPlayerNames[k], MAX_PLAYER_NAME_LENGTH );

	m_textureID_LoadingBar = -1;
	m_textureID_LoadingBarBG = -1;
	m_textureID_DefaultPosterImage = -1;

	m_bDrawBackground = false;
	m_bDrawPoster = false;
	m_bDrawProgress = false;
	m_bDrawSpinner = false;
	m_bFullscreenPoster = true;

	m_flLastEngineTime = 0;

	// marked to indicate the controls exist
	m_bValid = false;

	MEM_ALLOC_CREDIT();
	m_pDefaultPosterDataKV = new KeyValues( "DefaultPosterData" );
	if ( !m_pDefaultPosterDataKV->LoadFromFile( g_pFullFileSystem, "resource/UI/BaseModUI/LoadingPosterDefault.res", "MOD" ) )
	{
		DevWarning( "Failed to load default poster information!\n" );
		m_pDefaultPosterDataKV->deleteThis();
		m_pDefaultPosterDataKV = NULL;
	}

	m_pTipPanel = NULL;
	if ( IsX360() )
	{
		m_pTipPanel = new CLoadingTipPanel( this );
	}

	m_pLoadingText = new vgui::Label( this, "LoadingText", "#L4D360UI_Loading" );

	m_pLeaderboardBackground = new vgui::Panel( this, "LeaderboardBackground" );
	m_pLeaderboardPanel = new CReactiveDrop_VGUI_Leaderboard_Panel( this, "Leaderboard" );
	m_pMissionPic = new vgui::ImagePanel( this, "MissionPic" );
}

//=============================================================================
LoadingProgress::~LoadingProgress()
{
	if ( m_pDefaultPosterDataKV )
		m_pDefaultPosterDataKV->deleteThis();
	m_pDefaultPosterDataKV = NULL;
}

//=============================================================================
void LoadingProgress::OnThink()
{
	UpdateWorkingAnim();
}

//=============================================================================
void LoadingProgress::OnCommand(const char *command)
{
}

//=============================================================================
void LoadingProgress::ApplySchemeSettings( IScheme *pScheme )
{
	// will cause the controls to be instanced
	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBackgroundEnabled( true );

	m_pLeaderboardPanel->SetVisible( false );

	// now have controls, can now do further initing
	m_bValid = true;

	// find or create pattern
	// purposely not freeing these, not worth the i/o hitch for something so small
	const char *pImageName = "vgui/loadingbar";
	m_textureID_LoadingBar = vgui::surface()->DrawGetTextureId( pImageName );
	if ( m_textureID_LoadingBar == -1 )
	{
		m_textureID_LoadingBar = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_LoadingBar, pImageName, true, false );	
	}

	// find or create pattern
	// purposely not freeing these, not worth the i/o hitch for something so small
	pImageName = "vgui/loadingbar_bg";
	m_textureID_LoadingBarBG = vgui::surface()->DrawGetTextureId( pImageName );
	if ( m_textureID_LoadingBarBG == -1 )
	{
		m_textureID_LoadingBarBG = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile( m_textureID_LoadingBarBG, pImageName, true, false );	
	}

	// need to get the default image loaded now
	// find or create pattern
	// Purposely not freeing these, need this image to be resident always. We flip to
	// this image on a sign out during loading and cannot bring it into	existence then.
#if defined ( SUPPORT_DEFAULT_LOADING_POSTER )
	if ( m_pDefaultPosterDataKV )
	{
		static ConVarRef mat_xbox_iswidescreen( "mat_xbox_iswidescreen" );
		bool bIsWidescreen = mat_xbox_iswidescreen.GetBool();
		bool bFullscreenPoster = m_pDefaultPosterDataKV->GetBool( "fullscreen", false );
		const char *pszPosterImage = ( bFullscreenPoster && bIsWidescreen ) ? m_pDefaultPosterDataKV->GetString( "posterImage_widescreen" ) : m_pDefaultPosterDataKV->GetString( "posterImage" );

		// have to do this to mimic what the bowels of the scheme manager does with bitmaps
		bool bPrependVguiFix = V_strnicmp( pszPosterImage, "vgui", 4 ) != 0;
		CFmtStr sPosterImageFmt( "%s%s", ( bPrependVguiFix ? "vgui/" : "" ), pszPosterImage );
		pszPosterImage = sPosterImageFmt;

		m_textureID_DefaultPosterImage = vgui::surface()->DrawGetTextureId( pszPosterImage );
		if ( m_textureID_DefaultPosterImage == -1 )
		{
			m_textureID_DefaultPosterImage = vgui::surface()->CreateNewTextureID();
			vgui::surface()->DrawSetTextureFile( m_textureID_DefaultPosterImage, pszPosterImage, true, false );	
		}
	}
#endif

	SetupControlStates();
}

void LoadingProgress::Close()
{
	// hint to force any of the movie posters out of memory (purposely ignoring specific map search)
	// if the image is unreferenced (as it should be), it will evict, if already evicted or empty, then harmless
	if ( m_pPoster )
	{
		m_pPoster->EvictImage();
	}

	if ( m_pBGImage && ( m_LoadingType == LT_POSTER ) )
	{
		m_pBGImage->EvictImage();
	}
		
	BaseClass::Close();
}

//=============================================================================
// this is where the spinner gets updated.
void LoadingProgress::UpdateWorkingAnim()
{
	if ( m_pWorkingAnim && ( m_bDrawSpinner || m_LoadingType == LT_TRANSITION ) )
	{
		// clock the anim at 10hz
		float time = Plat_FloatTime();
		if ( ( m_flLastEngineTime + 0.1f ) < time )
		{
			m_flLastEngineTime = time;
			m_pWorkingAnim->SetFrame( m_pWorkingAnim->GetFrame() + 1 );
		}
	}
}

//=============================================================================
void LoadingProgress::SetProgress( float progress )
{
	if ( m_pProTotalProgress && m_bDrawProgress )
	{
		if ( progress > m_flPeakProgress )
		{
			m_flPeakProgress = progress;
		}
		m_pProTotalProgress->SetProgress( m_flPeakProgress );
	}
	
	if ( m_pTipPanel )
	{
		m_pTipPanel->NextTip();
	}

	UpdateWorkingAnim();
}

//=============================================================================
float LoadingProgress::GetProgress()
{
	float retVal = -1.0f;

	if ( m_pProTotalProgress )
	{
		retVal = m_pProTotalProgress->GetProgress();
	}

	return retVal;
}

void LoadingProgress::SetStatusText( const char *statusText )
{
	m_pLoadingText->SetVisible( rd_loading_status_text_visible.GetBool() );

	if ( statusText )
	{
		m_pLoadingText->SetText( statusText );
	}
}

void LoadingProgress::PaintBackground()
{
	int screenWide, screenTall;
	surface()->GetScreenSize( screenWide, screenTall );

	surface()->DrawSetColor( m_PosterReflectionColor );
	surface()->DrawFilledRect( 0, 0, screenWide, screenTall );

	if ( m_bDrawBackground && m_pBGImage )
	{
		int x, y, wide, tall;
		m_pBGImage->GetBounds( x, y, wide, tall );
		surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
		surface()->DrawSetTexture( m_pBGImage->GetImage()->GetID() );
		surface()->DrawTexturedRect( x, y, x+wide, y+tall );
	}

	if ( m_bDrawPoster && m_pPoster && m_pPoster->GetImage() )
	{
		if ( m_bFullscreenPoster )
		{
			int x0 = 0, x1 = screenWide, y0 = 0, y1 = screenTall;

			float aspectRatio = ( float )screenWide / ( float )screenTall;
			if ( aspectRatio >= 16.0f / 9.0f )
			{
				// pillarbox the loading screen
				int posterWide = screenTall * 16 / 9;
				x0 = ( screenWide - posterWide ) / 2;
				x1 -= x0;
			}
			else
			{
				// letterbox the loading screen
				int posterTall = screenWide * 9 / 16;
				y0 = ( screenTall - posterTall ) / 2;
				y1 -= y0;
			}

			surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
			surface()->DrawSetTexture( m_pPoster->GetImage()->GetID() );
			surface()->DrawTexturedRect( x0, y0, x1, y1 );
		}
		else
		{
			// fill black
			surface()->DrawSetColor( Color( 0, 0, 0, 255 ) );
			surface()->DrawFilledRect( 0, 0, screenWide, screenTall );

			// overlay poster
			int x, y, wide, tall;
			m_pPoster->GetBounds( x, y, wide, tall );
			surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
			surface()->DrawSetTexture( m_pPoster->GetImage()->GetID() );
			surface()->DrawTexturedRect( x, y, x+wide, y+tall );
		}		
	}

	if ( m_bDrawPoster && m_pFooter )
	{
		int screenWidth, screenHeight;
		CBaseModPanel::GetSingleton().GetSize( screenWidth, screenHeight );

		int x, y, wide, tall;
		m_pFooter->GetBounds( x, y, wide, tall );
		surface()->DrawSetColor( m_pFooter->GetBgColor() );
		surface()->DrawFilledRect( 0, y, x+screenWidth, y+tall );
	}

	// this is where the spinner draws
	bool bRenderSpinner = ( m_bDrawSpinner && m_pWorkingAnim );
	Panel *pWaitscreen = CBaseModPanel::GetSingleton().GetWindow( WT_GENERICWAITSCREEN );
	if ( pWaitscreen && pWaitscreen->IsVisible() && ( m_LoadingWindowType == LWT_BKGNDSCREEN ) )
		bRenderSpinner = false;	// Don't render spinner if the progress screen is displaying progress
	if ( bRenderSpinner  )
	{
		int x, y, wide, tall;

		wide = tall = scheme()->GetProportionalScaledValue( 45 );
		x = scheme()->GetProportionalScaledValue( 45 ) - wide/2;
		y = screenTall - scheme()->GetProportionalScaledValue( 32 ) - tall/2;

		m_pWorkingAnim->GetImage()->SetFrame( m_pWorkingAnim->GetFrame() );

		surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );
		surface()->DrawSetTexture( m_pWorkingAnim->GetImage()->GetID() );
		surface()->DrawTexturedRect( x, y, x+wide, y+tall );
	}

	if ( m_bDrawProgress && m_pProTotalProgress )
	{
		int x, y, wide, tall;
		m_pProTotalProgress->GetBounds( x, y, wide, tall );

		int iScreenWide, iScreenTall;
		surface()->GetScreenSize( iScreenWide, iScreenTall );

		float f = m_pProTotalProgress->GetProgress();
		f = clamp( f, 0, 1.0f );

		// Textured bar
		surface()->DrawSetColor( Color( 255, 255, 255, 255 ) );

		// Texture BG
		surface()->DrawSetTexture( m_textureID_LoadingBarBG );
		surface()->DrawTexturedRect( x, y, x + wide, y + tall );

		surface()->DrawSetTexture( m_textureID_LoadingBar );

		// YWB 10/13/2009:  If we don't crop the texture coordinate down then we will see a jitter of the texture as the texture coordinate and the rounded width 
		//  alias

		int nIntegerWide = f * wide;		
		float flUsedFrac = (float)nIntegerWide / (float)wide;
		
		DrawTexturedRectParms_t params;
		params.x0 = x;
		params.y0 = y;
		params.x1 = x + nIntegerWide;
		params.y1 = y + tall;
		params.s0 = 0;
		params.s1 = flUsedFrac;
		surface()->DrawTexturedRectEx( &params );		
	}

	// Need to call this periodically to collect sign in and sign out notifications,
	// do NOT dispatch events here in the middle of loading and rendering!
	if ( ThreadInMainThread() )
	{
		XBX_ProcessEvents();
	}
}

//=============================================================================
// Must be called first. Establishes the loading style
//=============================================================================
void LoadingProgress::SetLoadingType( LoadingProgress::LoadingType loadingType )
{
	m_LoadingType = loadingType;

	// the first time initing occurs during ApplySchemeSettings() or if the panel is deleted
	// if the panel is re-used, this is for the second time the panel gets used
	SetupControlStates();
}

//=============================================================================
LoadingProgress::LoadingType LoadingProgress::GetLoadingType()
{
	return m_LoadingType;
}

//=============================================================================
void LoadingProgress::SetupControlStates()
{
	m_flPeakProgress = 0.0f;

	if ( !m_bValid )
	{
		// haven't been functionally initialized yet
		// can't set or query control states until they get established
		return;
	}

	m_bDrawBackground = false;
	m_bDrawPoster = false;
	m_bDrawSpinner = false;
	m_bDrawProgress = false;

	switch( m_LoadingType )
	{
	case LT_MAINMENU:
		m_bDrawBackground = true;
		m_bDrawProgress = true;
		m_bDrawSpinner = true;
		break;
	case LT_TRANSITION:
		// the transition screen shows stats and owns all the drawing
		// spinner needs to be drawn as child of the window that is shown while loading
		break;
	case LT_POSTER:
		m_bDrawBackground = false;
		m_bDrawPoster = true;
		m_bDrawProgress = true;
		m_bDrawSpinner = true;
		break;
	default:
		break;
	}

	m_pFooter = dynamic_cast< vgui::EditablePanel* >( FindChildByName( "CampaignFooter" ) );

	m_pBGImage = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "BGImage" ) );
	if ( m_pBGImage )
	{
		// set the correct background image
		if ( m_LoadingType == LT_POSTER )
		{
			//m_pBGImage->SetImage( "../vgui/LoadingScreen_background" );
		}
		else
		{
			int screenWide, screenTall;
			surface()->GetScreenSize( screenWide, screenTall );
			char filename[MAX_PATH];
			V_snprintf( filename, sizeof( filename ), "console/background01rd_widescreen" ); // TODO: engine->GetStartupImage( filename, sizeof( filename ), screenWide, screenTall );
			m_pBGImage->SetImage( CFmtStr( "../%s", filename ) );
		}

		// we will custom draw
		m_pBGImage->SetVisible( false );
	}

	m_pPoster = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "Poster" ) );
	if ( m_pPoster )
	{
		// we will custom draw
		m_pPoster->SetVisible( false );
	}
	
	m_pProTotalProgress = dynamic_cast< vgui::ProgressBar* >( FindChildByName( "ProTotalProgress" ) );
	if ( m_pProTotalProgress )
	{
		// we will custom draw
		m_pProTotalProgress->SetVisible( false );
	}

	m_pWorkingAnim = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "WorkingAnim" ) );
	if ( m_pWorkingAnim )
	{
		// we will custom draw
		m_pWorkingAnim->SetVisible( false );
	}

	vgui::Label *pLoadingLabel = dynamic_cast< vgui::Label *>( FindChildByName( "LoadingText" ) );
	if ( pLoadingLabel )
	{
		pLoadingLabel->SetVisible( m_bDrawProgress );
	}

	SetControlVisible( "PlayerNames", m_bDrawPoster );
	SetControlVisible( "StarringLabel", m_bDrawPoster );
	SetControlVisible( "GameModeLabel", false );

	if ( m_LoadingType == LT_POSTER )
	{
		SetupPoster();
	}

	// Hold on to start frame slightly
	m_flLastEngineTime = Plat_FloatTime() + 0.2f;
}

void LoadingProgress::SetPosterData( KeyValues *pMissionInfo, KeyValues *pChapterInfo, const char **pPlayerNames, unsigned int botFlags, const char *pszGameMode, const char *levelName )
{
	m_botFlags = botFlags;
	m_pMissionInfo = pMissionInfo;
	m_pChapterInfo = pChapterInfo;

	RearrangeNames( pMissionInfo->GetString( "poster/character_order", NULL ), pPlayerNames );

	Q_snprintf( m_szGameMode, sizeof( m_szGameMode ), "#L4D360UI_Loading_GameMode_%s", pszGameMode );
	if ( levelName )
	{
		Q_strncpy( m_szLevelName, levelName, sizeof( m_szLevelName ) );
	}
}

void LoadingProgress::SetLeaderboardData( const char *pszLevelName, PublishedFileId_t nLevelAddon, const char *pszLevelDisplayName, const char *pszChallengeName, PublishedFileId_t nChallengeAddon, const char *pszChallengeDisplayName )
{
	if ( !rd_show_leaderboard_loading.GetBool() || !SteamUserStats() || !SteamFriends() )
	{
		return;
	}

	wchar_t wszLevelDisplayName[MAX_PATH];
	if ( const wchar_t *pwszLevelDisplayName = g_pVGuiLocalize->Find( pszLevelDisplayName ) )
	{
		Q_wcsncpy( wszLevelDisplayName, pwszLevelDisplayName, sizeof( wszLevelDisplayName ) );
	}
	else
	{
		g_pVGuiLocalize->ConvertANSIToUnicode( pszLevelDisplayName, wszLevelDisplayName, sizeof( wszLevelDisplayName ) );
	}

	wchar_t wszChallengeDisplayName[MAX_PATH];
	if ( const wchar_t *pwszChallengeDisplayName = g_pVGuiLocalize->Find( pszChallengeDisplayName ) )
	{
		Q_wcsncpy( wszChallengeDisplayName, pwszChallengeDisplayName, sizeof( wszChallengeDisplayName ) );
	}
	else
	{
		g_pVGuiLocalize->ConvertANSIToUnicode( pszChallengeDisplayName, wszChallengeDisplayName, sizeof( wszChallengeDisplayName ) );
	}

	if ( Q_strcmp( pszChallengeName, "0" ) )
	{
		Q_snwprintf( m_wszLeaderboardTitle, ARRAYSIZE( m_wszLeaderboardTitle ), L"%s (%s)", wszLevelDisplayName, wszChallengeDisplayName );
	}
	else
	{
		Q_snwprintf( m_wszLeaderboardTitle, ARRAYSIZE( m_wszLeaderboardTitle ), L"%s", wszLevelDisplayName );
	}

	char szLeaderboardName[k_cchLeaderboardNameMax]{};
	g_ASW_Steamstats.SpeedRunLeaderboardName( szLeaderboardName, sizeof( szLeaderboardName ), pszLevelName, nLevelAddon, pszChallengeName, nChallengeAddon );
	if ( !g_ASW_Steamstats.IsLBWhitelisted( szLeaderboardName ) )
	{
		g_ASW_Steamstats.SpeedRunLeaderboardName( szLeaderboardName, sizeof( szLeaderboardName ), pszLevelName, nLevelAddon, "0", k_PublishedFileIdInvalid );
		Q_snwprintf( m_wszLeaderboardTitle, ARRAYSIZE( m_wszLeaderboardTitle ), L"%s", wszLevelDisplayName );
	}

	SteamAPICall_t hCall = SteamUserStats()->FindLeaderboard( szLeaderboardName );
	m_LeaderboardFind.Set( hCall, this, &LoadingProgress::LeaderboardFind );
}

void LoadingProgress::LeaderboardFind( LeaderboardFindResult_t *pResult, bool bIOError )
{
	if ( bIOError || !pResult->m_bLeaderboardFound )
	{
		return;
	}

	m_pLeaderboardPanel->SetDisplayType( SteamUserStats()->GetLeaderboardDisplayType( pResult->m_hSteamLeaderboard ) );

	SteamAPICall_t hCall = SteamUserStats()->DownloadLeaderboardEntries( pResult->m_hSteamLeaderboard, k_ELeaderboardDataRequestFriends, 0, 0 );
	m_LeaderboardDownloaded.Set( hCall, this, &LoadingProgress::LeaderboardDownloaded );
}

void LoadingProgress::LeaderboardDownloaded( LeaderboardScoresDownloaded_t *pResult, bool bIOError )
{
	if ( bIOError || pResult->m_cEntryCount == 0 )
	{
		return;
	}

	CUtlVector<RD_LeaderboardEntry_t> entries;

	g_ASW_Steamstats.ReadDownloadedLeaderboard( entries, pResult->m_hSteamLeaderboardEntries, pResult->m_cEntryCount );

	m_pLeaderboardBackground->SetVisible( true );
	m_pLeaderboardPanel->SetTitle( m_wszLeaderboardTitle );
	m_pLeaderboardPanel->SetEntries( entries );
	m_pLeaderboardPanel->SetVisible( true );
}

//rearrange names to match the poster character order... sigh..
void LoadingProgress::RearrangeNames( const char *pszCharacterOrder, const char **pPlayerNames )
{
	int iNewOrder[NUM_LOADING_CHARACTERS]; 
	char m_SortPlayerNames[NUM_LOADING_CHARACTERS][MAX_PLAYER_NAME_LENGTH];

	for ( int k = 0; k < NUM_LOADING_CHARACTERS; ++ k )
	{
		iNewOrder[k] = k;
		if ( pPlayerNames[k] )
			Q_strncpy( m_PlayerNames[k], pPlayerNames[k], MAX_PLAYER_NAME_LENGTH );

		Q_strncpy( m_SortPlayerNames[k], m_PlayerNames[k], MAX_PLAYER_NAME_LENGTH );
	}

	if ( pszCharacterOrder )
	{
		char szOrder[MAX_PATH];
		Q_strncpy( szOrder, pszCharacterOrder, sizeof( szOrder ) );

		char	*p = szOrder;
		int i = 0;

		p = strtok( p, ";" );

		const char * const arrCharacterNamesInConfig[ NUM_LOADING_CHARACTERS ] = { "gambler", "producer", "coach", "mechanic" };

		while ( p != NULL && *p )
		{
			for ( int k = 0; k < NUM_LOADING_CHARACTERS; ++ k )
			{
				if ( !Q_stricmp( p, arrCharacterNamesInConfig[k] ) )
				{
					iNewOrder[k] = i;
					break;
				}
			}

			i++;

			p = strtok( NULL, ";" );
		}
	}

	for ( int k = 0; k < NUM_LOADING_CHARACTERS; ++ k )
	{
		Q_strncpy( m_PlayerNames[k], m_SortPlayerNames[iNewOrder[k]], MAX_PLAYER_NAME_LENGTH );
	}
}

//=============================================================================
bool LoadingProgress::ShouldShowPosterForLevel( KeyValues *pMissionInfo, KeyValues *pChapterInfo )
{
	// Do not show loading poster for credits
	if ( pMissionInfo && !Q_stricmp( pMissionInfo->GetString( "name" ), "credits" ) )
		return false;

	if ( pMissionInfo )
	{
		char const *szPosterImage = pMissionInfo->GetString( "poster/posterImage", NULL );
		if ( szPosterImage && *szPosterImage )
			// If the campaign specifies a valid poster, then we'll use it
			return true;
	}

	// All other campaigns can fall back to a default poster
	return ( m_pDefaultPosterDataKV != NULL );
}

//=============================================================================
void LoadingProgress::SetupPoster( void )
{
	bool bUsingCustomBackground = false;

	m_PosterReflectionColor = Color( 0, 0, 0, 255 );
	
	vgui::ImagePanel *pPoster = dynamic_cast< vgui::ImagePanel* >( FindChildByName( "Poster" ) );
	if ( pPoster )
	{ 
		int screenWide, screenTall;
		surface()->GetScreenSize( screenWide, screenTall );

		const char *pszPosterImage = "swarm/loading/RD_BGFX04_wide";
		if ( rd_loading_image_per_map.GetInt() == 1 && m_szLevelName[0] != 0 )
		{
			CFmtStr szMapLoadingImageVmt( "materials/vgui/swarm/loading/%s_wide.vmt", m_szLevelName );
			CFmtStr szMapLoadingImage( "swarm/loading/%s_wide", m_szLevelName );

			IMaterial *pMaterial;
			if ( m_bFullscreenPoster && filesystem->FileExists( szMapLoadingImageVmt ) )
			{
				pPoster->SetImage( szMapLoadingImage );
				pMaterial = materials->FindMaterial( CFmtStr( "vgui/%s", szMapLoadingImage.Access() ), TEXTURE_GROUP_VGUI );
				bUsingCustomBackground = true;
			}
			else
			{
				pPoster->SetImage( pszPosterImage );
				pMaterial = materials->FindMaterial( CFmtStr( "vgui/%s", pszPosterImage ), TEXTURE_GROUP_VGUI );
			}

			Vector vecColor{ 0, 0, 0 };
			if ( !IsErrorMaterial( pMaterial ) )
			{
				pMaterial->GetReflectivity( vecColor );
			}

			m_PosterReflectionColor = Color(
				clamp( vecColor.x * 255, 0, 255 ),
				clamp( vecColor.y * 255, 0, 255 ),
				clamp( vecColor.z * 255, 0, 255 ),
				255
			);
		}
		else
		{
			// if the image was cached this will just hook it up, otherwise it will load it
			pPoster->SetImage( pszPosterImage );
		}
	}

	if ( m_pChapterInfo && m_pChapterInfo->GetString( "image", NULL ) )
	{
		m_pMissionPic->SetImage( m_pChapterInfo->GetString( "image" ) );
		m_pMissionPic->SetVisible( rd_show_mission_icon_loading.GetBool() && ( !rd_auto_hide_mission_icon_loading.GetBool() || !bUsingCustomBackground ) );
	}

	bool bIsLocalized = false;
#ifdef _X360
	bIsLocalized = XBX_IsLocalized();
#else
	char uilanguage[ 64 ];
	engine->GetUILanguage( uilanguage, sizeof( uilanguage ) );
	if ( Q_stricmp( uilanguage, "english" ) )
	{
		bIsLocalized = true;
	}
#endif

	SetControlVisible( "LocalizedCampaignName", false );
	SetControlVisible( "LocalizedCampaignTagline", false );

	SetControlString( "GameModeLabel", m_szGameMode );

	SetControlVisible( "PlayerNames", false );

	SetControlVisible( "StarringLabel", false );
	SetControlVisible( "GameModeLabel", false );

	SetControlEnabled( "LoadingTipPanel", false );
}

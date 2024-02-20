#include "cbase.h"
#include "vreportproblem.h"
#include "gameui_interface.h"
#include "vgui_bitmapbutton.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/TextEntry.h"
#include "nb_button.h"
#include "nb_header_footer.h"
#include "rd_vgui_settings.h"
#include "rd_player_reporting.h"
#include "rd_workshop_preview_image.h"
#include "MultiFontRichText.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern ConVar ui_gameui_modal;

// stored outside of frame so we can resume a report from earlier
static struct
{
	const char *Category{ NULL };
	const char *const *SubCategories{ NULL };
	CSteamID Player;
	CUtlString Description;
	CUtlVectorAutoPurge<CReactiveDropWorkshopPreviewImage *> Screenshots;
	CUtlVector<bool> UseScreenshot;
	const ReportingServerSnapshot_t *Snapshot{ NULL };

	void Clear()
	{
		Category = NULL;
		SubCategories = NULL;
		Player = k_steamIDNil;
		Description.Purge();
		Screenshots.PurgeAndDeleteElements();
		UseScreenshot.Purge();
		Snapshot = NULL;
	}

	bool HasData() const
	{
		return Category != NULL || SubCategories != NULL;
	}
} s_RD_Current_Report;

// report categories with subcategories
static const char *const s_ServerReportTypes[] =
{
	"server_technical",
	"server_abuse",
	"server_other",
	NULL,
};

static const char *const s_PlayerReportTypes[] =
{
	"player_cheating",
	"player_abusive_gameplay",
	"player_abusive_communication",
	"player_commend",
	NULL,
};

// report categories that may be forwarded
static const char *const s_NonPrivateReportTypes[] =
{
	"player_commend",
	"server_technical",
	"server_other",
	"game_bug",
	"other",
	// no NULL at the end as this one is used statically
};

ReportProblem::ReportProblem( vgui::Panel *parent, const char *panelName )
	: BaseClass{ parent, panelName }
{
	if ( ui_gameui_modal.GetBool() )
	{
		GameUI().PreventEngineHideGameUI();
	}

	SetProportional( true );
	SetDeleteSelfOnClose( true );
	SetConsoleStylePanel( true );

	// global
	m_pBtnBack = new CNB_Button( this, "BtnBack", "#nb_back", this, "Back" );

	// category selection screen
	m_pHeaderFooter_Category = new CNB_Header_Footer( this, "HeaderFooter_Category" );
	m_pBtnMyAccount = new CBitmapButton( this, "BtnMyAccount", "#rd_reporting_category_my_account" );
	m_pBtnMyAccount->AddActionSignalTarget( this );
	m_pLblMyAccount = new vgui::Label( this, "LblMyAccount", "#rd_reporting_category_my_account_desc" );
	m_pBtnServer = new CBitmapButton( this, "BtnServer", "#rd_reporting_category_server" );
	m_pBtnServer->AddActionSignalTarget( this );
	m_pLblServer = new vgui::Label( this, "LblServer", "#rd_reporting_category_server_desc" );
	m_pBtnPlayer = new CBitmapButton( this, "BtnPlayer", "#rd_reporting_category_player" );
	m_pBtnPlayer->AddActionSignalTarget( this );
	m_pLblPlayer = new vgui::Label( this, "LblPlayer", "#rd_reporting_category_player_desc" );
	m_pBtnGameBug = new CBitmapButton( this, "BtnGameBug", "#rd_reporting_category_game_bug" );
	m_pBtnGameBug->AddActionSignalTarget( this );
	m_pLblGameBug = new vgui::Label( this, "LblGameBug", "#rd_reporting_category_game_bug_desc" );
	m_pBtnOther = new CBitmapButton( this, "BtnOther", "#rd_reporting_category_other" );
	m_pBtnOther->AddActionSignalTarget( this );
	m_pLblOther = new vgui::Label( this, "LblOther", "#rd_reporting_category_other_desc" );
	m_pBtnResume = new CNB_Button( this, "BtnResume", "#rd_reporting_resume", this, "Resume" );
	m_pLblLastProgress = new vgui::Label( this, "LblLastProgress", L"" );

	// wait screen
	m_pHeaderFooter_Wait = new CNB_Header_Footer( this, "HeaderFooter_Wait" );
	m_pLblWait = new vgui::Label( this, "LblWait", L"" );
	m_pLblDontWait = new vgui::Label( this, "LblDontWait", "#rd_reporting_continues_in_background" );
	m_pImgSpinner = new vgui::ImagePanel( this, "ImgSpinner" );

	// player select
	m_pHeaderFooter_Player = new CNB_Header_Footer( this, "HeaderFooter_Player" );

	// report in progress
	m_pHeaderFooter_Report = new CNB_Header_Footer( this, "HeaderFooter_Report" );
	m_pSettingSubCategory = new CRD_VGUI_Option( this, "SettingSubCategory", "", CRD_VGUI_Option::MODE_RADIO );
	m_pLblInstructions = new vgui::Label( this, "LblInstructions", L"" );
	m_pTxtDescription = new vgui::TextEntry( this, "TxtDescription" );
	m_pTxtDescription->AddActionSignalTarget( this );
	m_pTxtDescription->SetMultiline( true );
	COMPILE_TIME_ASSERT( NELEMS( m_pImgScreenshot ) == NELEMS( m_pChkScreenshot ) );
	for ( int i = 0; i < NELEMS( m_pImgScreenshot ); i++ )
	{
		m_pImgScreenshot[i] = new vgui::ImagePanel( this, VarArgs( "ImgScreenshot%d", i ) );
		m_pChkScreenshot[i] = new vgui::CheckButton( this, VarArgs( "ChkScreenshot%d", i ), "#rd_reporting_include_this_screenshot" );
		m_pChkScreenshot[i]->AddActionSignalTarget( this );
	}
	m_pLblReportContents = new vgui::MultiFontRichText( this, "LblReportContents" );
	m_pBtnSubmit = new CNB_Button( this, "BtnSubmit", "#rd_reporting_submit", this, "Submit" );
}

ReportProblem::~ReportProblem()
{
	GameUI().AllowEngineHideGameUI();
}

void ReportProblem::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pLblReportContents->SetFont( pScheme->GetFont( "DefaultVerySmall", true ) );
	m_pLblReportContents->SetPanelInteractive( false );
	m_pLblReportContents->SetDrawTextOnly();
	m_pLblReportContents->SetCursor( dc_arrow );

	if ( g_RD_Player_Reporting.IsInProgress() )
	{
		ShowWaitScreen();
	}
	else
	{
		ShowCategorySelectionScreen();
	}
}

void ReportProblem::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "Back" ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else if ( !V_stricmp( command, "Resume" ) )
	{
		ResumeInProgressReport();
	}
	else if ( !V_stricmp( command, "MyAccount" ) )
	{
		StartNewReport( "my_account" );
	}
	else if ( !V_stricmp( command, "Server" ) )
	{
		if ( !g_RD_Player_Reporting.HasRecentServer() )
		{
			ShowCategorySelectionScreen();
			return;
		}

		StartNewReport( "server_other", s_ServerReportTypes, k_steamIDNil, true );
	}
	else if ( !V_stricmp( command, "Player" ) )
	{
		ShowRecentlyPlayedWith();
	}
	else if ( !V_stricmp( command, "GameBug" ) )
	{
		StartNewReport( "game_bug" );
	}
	else if ( !V_stricmp( command, "Other" ) )
	{
		StartNewReport( "other" );
	}
	else if ( !V_stricmp( command, "Submit" ) )
	{
		if ( g_RD_Player_Reporting.IsInProgress() )
		{
			// Something went wrong and we're already submitting a report. Don't modify the current draft report; just swap to the wait screen.
			ShowWaitScreen();
			return;
		}

		CUtlVector<CUtlBuffer> screenshots;
		FOR_EACH_VEC( s_RD_Current_Report.UseScreenshot, i )
		{
			if ( s_RD_Current_Report.UseScreenshot[i] )
			{
				screenshots[screenshots.AddToTail()].Put( s_RD_Current_Report.Screenshots[i]->m_SavedBuffer.Base(), s_RD_Current_Report.Screenshots[i]->m_SavedBuffer.TellPut() );
			}
		}

		bool bOK = g_RD_Player_Reporting.PrepareReportForSend( s_RD_Current_Report.Category, s_RD_Current_Report.Description.Get(), s_RD_Current_Report.Player, screenshots, s_RD_Current_Report.Snapshot );
		if ( bOK )
		{
			// Report was copied into the request buffer successfully; clear the draft report.
			s_RD_Current_Report.Clear();
		}

		// Regardless, switch to the wait screen.
		ShowWaitScreen();
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void ReportProblem::OnThink()
{
	BaseClass::OnThink();

	if ( m_pLblLastProgress->IsVisible() )
	{
		// small message (so players can know the state of a background report)
		m_pLblLastProgress->SetText( g_RD_Player_Reporting.m_wszLastMessage );
	}
	if ( m_pLblWait->IsVisible() )
	{
		// large message (main focus of screen)
		m_pLblWait->SetText( g_RD_Player_Reporting.m_wszLastMessage );
	}

	if ( !g_RD_Player_Reporting.IsInProgress() )
	{
		m_pLblDontWait->SetVisible( false );
		m_pImgSpinner->SetVisible( false );
	}
}

void ReportProblem::OnTextChanged()
{
	int buflen = m_pTxtDescription->GetTextLength() + 1;
	s_RD_Current_Report.Description.SetLength( buflen - 1 );
	m_pTxtDescription->GetText( s_RD_Current_Report.Description.Get(), buflen );

	UpdateReportContents();

	m_pBtnSubmit->SetEnabled( buflen > 1 && s_RD_Current_Report.Category );
}

void ReportProblem::OnCheckButtonChecked()
{
	FOR_EACH_VEC( s_RD_Current_Report.UseScreenshot, i )
	{
		Assert( i < NELEMS( m_pChkScreenshot ) );
		if ( i >= NELEMS( m_pChkScreenshot ) )
		{
			break;
		}

		s_RD_Current_Report.UseScreenshot[i] = m_pChkScreenshot[i]->IsSelected();
	}

	UpdateReportContents();
}

void ReportProblem::OnCurrentOptionChanged()
{
	if ( !s_RD_Current_Report.SubCategories )
	{
		Assert( m_pSettingSubCategory->GetCurrentOption() == 0 );
		return;
	}

	int iOption = m_pSettingSubCategory->GetCurrentOption();
	if ( iOption == -1 || ( s_RD_Current_Report.Category && !V_strcmp( s_RD_Current_Report.Category, s_RD_Current_Report.SubCategories[iOption] ) ) )
	{
		return;
	}

	s_RD_Current_Report.Category = s_RD_Current_Report.SubCategories[iOption];

	ResumeInProgressReport();
}

void ReportProblem::HideAllControls()
{
	MakeReadyForUse();

	// category selection screen
	m_pHeaderFooter_Category->SetVisible( false );
	m_pBtnMyAccount->SetVisible( false );
	m_pBtnServer->SetVisible( false );
	m_pBtnPlayer->SetVisible( false );
	m_pBtnGameBug->SetVisible( false );
	m_pBtnOther->SetVisible( false );
	m_pLblMyAccount->SetVisible( false );
	m_pLblServer->SetVisible( false );
	m_pLblPlayer->SetVisible( false );
	m_pLblGameBug->SetVisible( false );
	m_pLblOther->SetVisible( false );
	m_pBtnResume->SetVisible( false );
	m_pLblLastProgress->SetVisible( false );

	// wait screen
	m_pHeaderFooter_Wait->SetVisible( false );
	m_pLblWait->SetVisible( false );
	m_pLblDontWait->SetVisible( false );
	m_pImgSpinner->SetVisible( false );

	// player select
	m_pHeaderFooter_Player->SetVisible( false );

	// report in progress
	m_pHeaderFooter_Report->SetVisible( false );
	m_pSettingSubCategory->SetVisible( false );
	m_pLblInstructions->SetVisible( false );
	m_pTxtDescription->SetVisible( false );
	COMPILE_TIME_ASSERT( NELEMS( m_pImgScreenshot ) == NELEMS( m_pChkScreenshot ) );
	for ( int i = 0; i < NELEMS( m_pImgScreenshot ); i++ )
	{
		m_pImgScreenshot[i]->SetVisible( false );
		m_pChkScreenshot[i]->SetVisible( false );
	}
	m_pLblReportContents->SetVisible( false );
	m_pBtnSubmit->SetVisible( false );
}

void ReportProblem::ShowWaitScreen()
{
	HideAllControls();

	m_pHeaderFooter_Wait->SetVisible( true );
	m_pLblWait->SetVisible( true );
	m_pLblDontWait->SetVisible( true );
	m_pImgSpinner->SetVisible( true );
}

void ReportProblem::ShowCategorySelectionScreen()
{
	HideAllControls();

	m_pHeaderFooter_Category->SetVisible( true );
	m_pBtnMyAccount->SetVisible( true );
	m_pBtnServer->SetVisible( true );
	m_pBtnPlayer->SetVisible( true );
	m_pBtnGameBug->SetVisible( true );
	m_pBtnOther->SetVisible( true );
	m_pLblMyAccount->SetVisible( true );
	m_pLblServer->SetVisible( true );
	m_pLblPlayer->SetVisible( true );
	m_pLblGameBug->SetVisible( true );
	m_pLblOther->SetVisible( true );
	m_pLblLastProgress->SetVisible( true );

	m_pBtnServer->SetEnabled( g_RD_Player_Reporting.HasRecentServer() );
	m_pBtnPlayer->SetEnabled( g_RD_Player_Reporting.HasRecentlyPlayedWith() );
	m_pBtnResume->SetVisible( s_RD_Current_Report.HasData() );
}

void ReportProblem::ShowRecentlyPlayedWith()
{
	CUtlVector<CSteamID> players;
	g_RD_Player_Reporting.GetRecentlyPlayedWith( players );

	if ( !players.Count() )
	{
		ShowCategorySelectionScreen();
		return;
	}

	HideAllControls();

	m_pHeaderFooter_Player->SetVisible( true );

	DebuggerBreakIfDebugging(); // TODO
}

void ReportProblem::ResumeInProgressReport()
{
	HideAllControls();

	Assert( s_RD_Current_Report.HasData() );
	Assert( s_RD_Current_Report.Screenshots.Count() <= NELEMS( m_pImgScreenshot ) );

	m_pHeaderFooter_Report->SetVisible( true );

	m_pSettingSubCategory->SetVisible( true );
	m_pSettingSubCategory->RemoveAllOptions();
	if ( s_RD_Current_Report.Category && !s_RD_Current_Report.SubCategories )
	{
		m_pSettingSubCategory->AddOption( 0, VarArgs( "#rd_reporting_category_%s", s_RD_Current_Report.Category ), "", CRD_VGUI_Option::FLAG_DISABLED );
		m_pSettingSubCategory->SetCurrentOption( 0 );
	}
	else if ( s_RD_Current_Report.SubCategories )
	{
		int iCurrentOption = -1;
		for ( int i = 0; s_RD_Current_Report.SubCategories[i]; i++ )
		{
			m_pSettingSubCategory->AddOption( i, VarArgs( "#rd_reporting_subcategory_%s", s_RD_Current_Report.SubCategories[i] ), "" );
			if ( s_RD_Current_Report.Category && !V_strcmp( s_RD_Current_Report.Category, s_RD_Current_Report.SubCategories[i] ) )
			{
				Assert( iCurrentOption == -1 );
				iCurrentOption = i;
			}
			Assert( iCurrentOption != -1 || !s_RD_Current_Report.Category );
			m_pSettingSubCategory->SetCurrentOption( iCurrentOption );
		}
	}
	m_pLblInstructions->SetVisible( true );
	if ( s_RD_Current_Report.Category )
	{
		m_pLblInstructions->SetText( VarArgs( "#rd_reporting_instructions_%s", s_RD_Current_Report.Category ) );
	}
	else
	{
		Assert( s_RD_Current_Report.SubCategories );
		m_pLblInstructions->SetText( "#rd_reporting_instructions_required_subcategory" );
	}
	m_pTxtDescription->SetVisible( true );
	m_pTxtDescription->SetText( s_RD_Current_Report.Description.Get() );
	m_pTxtDescription->SetEnabled( s_RD_Current_Report.Category != NULL );
	COMPILE_TIME_ASSERT( NELEMS( m_pImgScreenshot ) == NELEMS( m_pChkScreenshot ) );
	for ( int i = 0; i < NELEMS( m_pImgScreenshot ); i++ )
	{
		if ( s_RD_Current_Report.Screenshots.Count() > i )
		{
			m_pImgScreenshot[i]->SetVisible( true );
			m_pImgScreenshot[i]->SetImage( s_RD_Current_Report.Screenshots[i] );
			m_pChkScreenshot[i]->SetVisible( true );
			m_pChkScreenshot[i]->SetSelected( s_RD_Current_Report.UseScreenshot[i] );
		}
	}
	m_pLblReportContents->SetVisible( true );
	UpdateReportContents();
	m_pBtnSubmit->SetVisible( true );
	m_pBtnSubmit->SetEnabled( s_RD_Current_Report.Category != NULL && !s_RD_Current_Report.Description.IsEmpty() );
}

void ReportProblem::StartNewReport( const char *szCategory, const char *const *SubCategories, CSteamID Player, bool bRequireDedicatedServer )
{
	s_RD_Current_Report.Clear();

	s_RD_Current_Report.Category = szCategory;
	s_RD_Current_Report.SubCategories = SubCategories;
	s_RD_Current_Report.Player = Player;

	s_RD_Current_Report.Snapshot = g_RD_Player_Reporting.PinRelevantSnapshot( Player, bRequireDedicatedServer );

	if ( g_RD_Player_Reporting.m_LatestScreenshot.TellPut() != 0 )
	{
		Assert( s_RD_Current_Report.Screenshots.Count() == s_RD_Current_Report.UseScreenshot.Count() );
		CReactiveDropWorkshopPreviewImage *pImage = new CReactiveDropWorkshopPreviewImage( g_RD_Player_Reporting.m_LatestScreenshot );
		if ( pImage->GetID() != -1 )
		{
			s_RD_Current_Report.Screenshots.AddToTail( pImage );
			s_RD_Current_Report.UseScreenshot.AddToTail( false );
		}
		else
		{
			delete pImage;
		}
	}

	ResumeInProgressReport();
}

void ReportProblem::UpdateReportContents()
{
	m_pLblReportContents->SetText( "#rd_reporting_includes" );
	m_pLblReportContents->InsertString( L"\n\n" );
	m_pLblReportContents->InsertString( "#rd_reporting_includes_your_account_id" );
	if ( s_RD_Current_Report.Player.IsValid() )
	{
		m_pLblReportContents->InsertChar( L'\n' );
		m_pLblReportContents->InsertString( "#rd_reporting_includes_other_account_id" );
	}
	if ( !s_RD_Current_Report.Description.IsEmpty() )
	{
		m_pLblReportContents->InsertChar( L'\n' );
		m_pLblReportContents->InsertString( "#rd_reporting_includes_message" );
	}
	if ( s_RD_Current_Report.Snapshot && s_RD_Current_Report.Snapshot->Witnesses.Count() > 1 )
	{
		m_pLblReportContents->InsertChar( L'\n' );
		m_pLblReportContents->InsertString( "#rd_reporting_includes_witness_list" );
	}
	if ( s_RD_Current_Report.Snapshot && !s_RD_Current_Report.Snapshot->MissionName.IsEmpty() )
	{
		m_pLblReportContents->InsertChar( L'\n' );
		m_pLblReportContents->InsertString( "#rd_reporting_includes_mission" );
	}
	if ( s_RD_Current_Report.Snapshot && s_RD_Current_Report.Snapshot->IsDedicatedServer )
	{
		m_pLblReportContents->InsertChar( L'\n' );
		m_pLblReportContents->InsertString( "#rd_reporting_includes_server_ip" );
	}
	FOR_EACH_VEC( s_RD_Current_Report.UseScreenshot, i )
	{
		if ( s_RD_Current_Report.UseScreenshot[i] )
		{
			m_pLblReportContents->InsertChar( L'\n' );
			m_pLblReportContents->InsertString( "#rd_reporting_includes_screenshot" );
			break;
		}
	}
	if ( s_RD_Current_Report.Snapshot && s_RD_Current_Report.Snapshot->HaveConnectionQuality )
	{
		m_pLblReportContents->InsertChar( L'\n' );
		m_pLblReportContents->InsertString( "#rd_reporting_includes_connection_quality" );
	}
	if ( s_RD_Current_Report.Category )
	{
		bool bIsPrivate = true;
		for ( int i = 0; i < NELEMS( s_NonPrivateReportTypes ); i++ )
		{
			if ( !V_strcmp( s_RD_Current_Report.Category, s_NonPrivateReportTypes[i] ) )
			{
				bIsPrivate = false;
				break;
			}
		}

		if ( bIsPrivate )
		{
			m_pLblReportContents->InsertString( L"\n\n" );
			m_pLblReportContents->InsertString( "#rd_reporting_only_team" );
		}
	}
}

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

ReportProblem::ReportProblem( vgui::Panel *parent, const char *panelName )
	: BaseClass{ parent, panelName }
{
	if ( ui_gameui_modal.GetBool() )
	{
		GameUI().PreventEngineHideGameUI();
	}
	SetProportional( true );
	SetDeleteSelfOnClose( true );

	// global
	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pBtnBack = new CNB_Button( this, "BtnBack", "#nb_back", this, "Back" );

	// category selection screen
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
	m_pLblGameBug = new vgui::Label( this, "LblBug", "#rd_reporting_category_game_bug_desc" );
	m_pBtnOther = new CBitmapButton( this, "BtnOther", "#rd_reporting_category_other" );
	m_pBtnOther->AddActionSignalTarget( this );
	m_pLblOther = new vgui::Label( this, "LblOther", "#rd_reporting_category_other_desc" );
	m_pBtnResume = new CNB_Button( this, "BtnResume", "#rd_reporting_resume", this, "Resume" );
	m_pLblLastProgress = new vgui::Label( this, "LblLastProgress", L"" );

	// wait screen
	m_pLblWait = new vgui::Label( this, "LblWait", L"" );
	m_pLblDontWait = new vgui::Label( this, "LblDontWait", "#rd_reporting_continues_in_background" );
	m_pImgSpinner = new vgui::ImagePanel( this, "ImgSpinner" );

	// player select

	// report in progress
	m_pSettingSubCategory = new CRD_VGUI_Option( this, "SettingSubCategory", "", CRD_VGUI_Option::MODE_RADIO );
	m_pLblInstructions = new vgui::Label( this, "LblInstructions", L"" );
	m_pTxtDescription = new vgui::TextEntry( this, "TxtDescription" );
	m_pTxtDescription->AddActionSignalTarget( this );
	COMPILE_TIME_ASSERT( NELEMS( m_pImgScreenshot ) == NELEMS( m_pChkScreenshot ) );
	for ( int i = 0; i < NELEMS( m_pImgScreenshot ); i++ )
	{
		m_pImgScreenshot[i] = new vgui::ImagePanel( this, VarArgs( "ImgScreenshot%d", i ) );
		m_pChkScreenshot[i] = new vgui::CheckButton( this, VarArgs( "ChkScreenshot%d", i ), "#rd_reporting_include_this_screenshot" );
		m_pChkScreenshot[i]->AddActionSignalTarget( this );
	}
	m_pLblReportContents = new vgui::Label( this, "LblReportContents", L"" );
	m_pBtnSubmit = new CNB_Button( this, "BtnSubmit", "#rd_reporting_submit", this, "Submit" );
}

ReportProblem::~ReportProblem()
{
	GameUI().AllowEngineHideGameUI();
}

void ReportProblem::Activate()
{
	BaseClass::Activate();

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
	DebuggerBreakIfDebugging(); // TODO
}

void ReportProblem::OnCheckButtonChecked()
{
	DebuggerBreakIfDebugging(); // TODO
}

void ReportProblem::HideAllControls()
{
	// category selection screen
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
	m_pLblWait->SetVisible( false );
	m_pLblDontWait->SetVisible( false );
	m_pImgSpinner->SetVisible( false );

	// player select

	// report in progress
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

	m_pLblWait->SetVisible( true );
	m_pLblDontWait->SetVisible( true );
	m_pImgSpinner->SetVisible( true );
}

void ReportProblem::ShowCategorySelectionScreen()
{
	HideAllControls();

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

	DebuggerBreakIfDebugging(); // TODO
}

void ReportProblem::ResumeInProgressReport()
{
	HideAllControls();

	Assert( s_RD_Current_Report.HasData() );
	Assert( s_RD_Current_Report.Screenshots.Count() <= NELEMS( m_pImgScreenshot ) );

	m_pSettingSubCategory->SetVisible( false );
	m_pLblInstructions->SetVisible( true );
	m_pTxtDescription->SetVisible( true );
	m_pTxtDescription->SetText( s_RD_Current_Report.Description.Get() );
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
	m_pBtnSubmit->SetVisible( true );

	UpdateReportContents();
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
	DebuggerBreakIfDebugging(); // TODO
}

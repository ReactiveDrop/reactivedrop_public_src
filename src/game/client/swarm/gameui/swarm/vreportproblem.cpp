#include "cbase.h"
#include "vreportproblem.h"
#include "vgui_bitmapbutton.h"
#include "nb_button.h"
#include "nb_header_footer.h"
#include "rd_player_reporting.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

// stored outside of frame so we can resume a report from earlier
static struct
{
	const char *Category{ NULL };
	CSteamID Player;
	CUtlString Description;
	CUtlVector<CUtlBuffer> Screenshots;
	CUtlVector<bool> UseScreenshot;
	CUtlVector<vgui::HTexture> ScreenshotThumbs;
	const ReportingServerSnapshot_t *Snapshot{ NULL };
} s_RD_Current_Report;

ReportProblem::ReportProblem( vgui::Panel *parent, const char *panelName )
	: BaseClass{ parent, panelName }
{
	SetProportional( true );
	SetDeleteSelfOnClose( true );

	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );

	m_pBtnMyAccount = new CBitmapButton( this, "BtnMyAccount", "#rd_reporting_category_my_account" );
	m_pBtnMyAccount->AddActionSignalTarget( this );
	m_pLblMyAccount = new vgui::Label( this, "LblMyAccount", "#rd_reporting_category_my_account_desc" );
	m_pBtnServer = new CBitmapButton( this, "BtnServer", "#rd_reporting_category_server" );
	m_pBtnServer->AddActionSignalTarget( this );
	m_pLblServer = new vgui::Label( this, "LblServer", "#rd_reporting_category_server_desc" );
	m_pBtnPlayer = new CBitmapButton( this, "BtnPlayer", "#rd_reporting_category_player" );
	m_pBtnPlayer->AddActionSignalTarget( this );
	m_pLblPlayer = new vgui::Label( this, "LblPlayer", "#rd_reporting_category_player_desc" );
	m_pBtnBug = new CBitmapButton( this, "BtnBug", "#rd_reporting_category_bug" );
	m_pBtnBug->AddActionSignalTarget( this );
	m_pLblBug = new vgui::Label( this, "LblBug", "#rd_reporting_category_bug_desc" );
	m_pBtnOther = new CBitmapButton( this, "BtnOther", "#rd_reporting_category_other" );
	m_pBtnOther->AddActionSignalTarget( this );
	m_pLblOther = new vgui::Label( this, "LblOther", "#rd_reporting_category_other_desc" );
	m_pBtnResume = new CNB_Button( this, "BtnResume", "#rd_reporting_resume", this, "Resume" );
	m_pBtnBack = new CNB_Button( this, "BtnBack", "#nb_back", this, "Back" );
}

ReportProblem::~ReportProblem()
{
}

void ReportProblem::OnCommand( const char *command )
{
	// my_account - for self-reported problems about items or achievements or whatever
	//
	// server_technical - reporting a technical problem with a server
	// server_abuse - reporting an abusive server
	// server_other - reporting a server-specific issue that does not fit into the above catgories
	//
	// player_cheating - reporting a player for cheating
	// player_abusive_gameplay - reporting a player for griefing
	// player_abusive_communication - repoting a player for being abusive in text or voice chat or with another in-game communication tool
	// player_commend - reporting a player for doing something good
	//
	// game_bug - reporting a bug in the game
	//
	// other - any non-quick report that does not fit into the above categories
	DebuggerBreakIfDebugging(); // TODO!

	if ( !V_stricmp( command, "Back" ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else if ( !V_stricmp( command, "Resume" ) )
	{
		DebuggerBreakIfDebugging(); // TODO
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

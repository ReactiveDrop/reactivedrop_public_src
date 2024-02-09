#include "cbase.h"
#include "rd_vgui_quick_report_panel.h"
#include "rd_player_reporting.h"
#include <vgui/ILocalize.h>
#include "vgui_avatarimage.h"
#include "vgui_bitmapbutton.h"
#include "nb_button.h"
#include "nb_header_footer.h"
#include "gameui/basepanel.h"
#include "gameui/swarm/vgenericconfirmation.h"
#include "iclientmode.h"
#include "controller_focus.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_debug_quick_report_local_player( "rd_debug_quick_report_local_player", "0", FCVAR_NONE, "Allow opening the quick-report dialog for the local player for testing UI layout." );

CRD_VGUI_Quick_Report_Panel::CRD_VGUI_Quick_Report_Panel( vgui::Panel *parent, const char *panelName )
	: BaseClass{ parent, panelName }
{
	m_pHeaderFooter = new CNB_Header_Footer( this, "HeaderFooter" );
	m_pHeaderFooter->SetTitle( "" );
	m_pHeaderFooter->SetHeaderEnabled( false );
	m_pHeaderFooter->SetFooterEnabled( true );
	m_pHeaderFooter->SetGradientBarEnabled( true );
	m_pHeaderFooter->SetGradientBarPos( 100, 300 );
	m_pHeaderFooter->SetMovieEnabled( true );
	m_pHeaderFooter->SetBackgroundStyle( NB_BACKGROUND_DARK );

	m_pImgPlayerAvatar = new CAvatarImagePanel( this, "ImgPlayerAvatar" );
	m_pLblPlayerName = new vgui::Label( this, "LblPlayerName", L"" );
	m_pLblProgress = new vgui::Label( this, "LblProgress", L"" );
	m_pBtnBack = new CNB_Button( this, "BtnBack", "#nb_back", this, "Back", true );

	m_pBtnCommendFriendly = new CBitmapButton( this, "BtnCommendFriendly", " " );
	m_pBtnCommendFriendly->AddActionSignalTarget( this );
	m_pBtnCommendLeader = new CBitmapButton( this, "BtnCommendLeader", " " );
	m_pBtnCommendLeader->AddActionSignalTarget( this );
	m_pBtnCommendTeacher = new CBitmapButton( this, "BtnCommendTeacher", " " );
	m_pBtnCommendTeacher->AddActionSignalTarget( this );
	m_pBtnReportCheating = new CBitmapButton( this, "BtnReportCheating", " " );
	m_pBtnReportCheating->AddActionSignalTarget( this );
	m_pBtnReportGriefing = new CBitmapButton( this, "BtnReportGriefing", " " );
	m_pBtnReportGriefing->AddActionSignalTarget( this );
	m_pBtnReportCommunication = new CBitmapButton( this, "BtnReportCommunication", " " );
	m_pBtnReportCommunication->AddActionSignalTarget( this );

	m_pLblCommendFriendly = new vgui::Label( this, "LblCommendFriendly", "#rd_quick_report_friendly" );
	m_pLblCommendLeader = new vgui::Label( this, "LblCommendLeader", "#rd_quick_report_leader" );
	m_pLblCommendTeacher = new vgui::Label( this, "LblCommendTeacher", "#rd_quick_report_teacher" );
	m_pLblReportCheating = new vgui::Label( this, "LblReportCheating", "#rd_quick_report_cheating" );
	m_pLblReportGriefing = new vgui::Label( this, "LblReportGriefing", "#rd_quick_report_gameplay" );
	m_pLblReportCommunication = new vgui::Label( this, "LblReportCommunication", "#rd_quick_report_communication" );

	if ( CControllerFocus *pFocus = GetControllerFocus() )
	{
		pFocus->PushModal();
		pFocus->AddToFocusList( m_pBtnBack, false, true );
		pFocus->AddToFocusList( m_pBtnCommendFriendly, false, true );
		pFocus->AddToFocusList( m_pBtnCommendLeader, false, true );
		pFocus->AddToFocusList( m_pBtnCommendTeacher, false, true );
		pFocus->AddToFocusList( m_pBtnReportCheating, false, true );
		pFocus->AddToFocusList( m_pBtnReportGriefing, false, true );
		pFocus->AddToFocusList( m_pBtnReportCommunication, false, true );
	}
}

CRD_VGUI_Quick_Report_Panel::~CRD_VGUI_Quick_Report_Panel()
{
	if ( CControllerFocus *pFocus = GetControllerFocus() )
	{
		pFocus->RemoveFromFocusList( m_pBtnBack );
		pFocus->RemoveFromFocusList( m_pBtnCommendFriendly );
		pFocus->RemoveFromFocusList( m_pBtnCommendLeader );
		pFocus->RemoveFromFocusList( m_pBtnCommendTeacher );
		pFocus->RemoveFromFocusList( m_pBtnReportCheating );
		pFocus->RemoveFromFocusList( m_pBtnReportGriefing );
		pFocus->RemoveFromFocusList( m_pBtnReportCommunication );
		pFocus->PopModal();
	}
}

void CRD_VGUI_Quick_Report_Panel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( "Resource/UI/CRD_VGUI_Quick_Report_Panel.res" );

	Assert( m_PlayerID.BIndividualAccount() );

	m_pImgPlayerAvatar->SetAvatarBySteamID( &m_PlayerID );

	if ( ISteamFriends *pFriends = SteamFriends() )
	{
		wchar_t wszPlayerName[k_cwchPersonaNameMax];
		V_UTF8ToUnicode( pFriends->GetFriendPersonaName( m_PlayerID ), wszPlayerName, sizeof( wszPlayerName ) );
		m_pLblPlayerName->SetText( wszPlayerName );
	}
}

static void UpdateButtonState( CSteamID playerID, const char *szCategory, CBitmapButton *pButton, vgui::Label *pLabel, const char *szText, const char *szRecentlyDoneText )
{
	if ( !g_RD_Player_Reporting.HasRecentlyPlayedWith( playerID ) )
	{
		pButton->SetEnabled( false );
		pLabel->SetText( szText );
	}
	else if ( g_RD_Player_Reporting.RecentlyReportedPlayer( szCategory, playerID ) )
	{
		pButton->SetEnabled( false );
		pLabel->SetText( szRecentlyDoneText );
	}
	else
	{
		pButton->SetEnabled( true );
		pLabel->SetText( szText );
	}

	pLabel->SetVisible( true );
}

void CRD_VGUI_Quick_Report_Panel::OnThink()
{
	BaseClass::OnThink();

	if ( g_RD_Player_Reporting.IsInProgress() )
	{
		m_pLblProgress->SetVisible( true );

		m_pBtnCommendFriendly->SetEnabled( false );
		m_pBtnCommendLeader->SetEnabled( false );
		m_pBtnCommendTeacher->SetEnabled( false );
		m_pBtnReportCheating->SetEnabled( false );
		m_pBtnReportGriefing->SetEnabled( false );
		m_pBtnReportCommunication->SetEnabled( false );

		m_pLblCommendFriendly->SetVisible( false );
		m_pLblCommendLeader->SetVisible( false );
		m_pLblCommendTeacher->SetVisible( false );
		m_pLblReportCheating->SetVisible( false );
		m_pLblReportGriefing->SetVisible( false );
		m_pLblReportCommunication->SetVisible( false );
	}
	else
	{
		UpdateButtonState( m_PlayerID, "quick_commend_friendly", m_pBtnCommendFriendly, m_pLblCommendFriendly, "#rd_quick_report_friendly", "#rd_quick_report_already_commended");
		UpdateButtonState( m_PlayerID, "quick_commend_leader", m_pBtnCommendLeader, m_pLblCommendLeader, "#rd_quick_report_leader", "#rd_quick_report_already_commended");
		UpdateButtonState( m_PlayerID, "quick_commend_teacher", m_pBtnCommendTeacher, m_pLblCommendTeacher, "#rd_quick_report_teacher", "#rd_quick_report_already_commended");
		UpdateButtonState( m_PlayerID, "quick_abusive_cheating", m_pBtnReportCheating, m_pLblReportCheating, "#rd_quick_report_cheating", "#rd_quick_report_already_reported");
		UpdateButtonState( m_PlayerID, "quick_abusive_gameplay", m_pBtnReportGriefing, m_pLblReportGriefing, "#rd_quick_report_gameplay", "#rd_quick_report_already_reported");
		UpdateButtonState( m_PlayerID, "quick_abusive_communication", m_pBtnReportCommunication, m_pLblReportCommunication, "#rd_quick_report_communication", "#rd_quick_report_already_reported" );
	}

	if ( m_pLblProgress->IsVisible() )
	{
		m_pLblProgress->SetText( g_RD_Player_Reporting.m_wszLastMessage );
	}
}

static const char *s_szQuickReportCategory;
static CSteamID s_QuickReportPlayerID;

static void AcceptSendQuickReport()
{
	if ( g_RD_Player_Reporting.PrepareReportForSend( s_szQuickReportCategory, NULL, s_QuickReportPlayerID, CUtlVector<CUtlBuffer>{}, false ) )
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_ACCEPT );
	}
	else
	{
		BaseModUI::CBaseModPanel::GetSingleton().PlayUISound( BaseModUI::UISOUND_DENY );
	}
}

void CRD_VGUI_Quick_Report_Panel::ConfirmAndSendQuickReport( const char *szTitle, const char *szMessage, const char *szCategory, CSteamID player )
{
	s_szQuickReportCategory = szCategory;
	s_QuickReportPlayerID = player;

	static wchar_t s_wszMessage[1024];
	wchar_t wszPlayerName[k_cwchPersonaNameMax];
	if ( ISteamFriends *pFriends = SteamFriends() )
	{
		V_UTF8ToUnicode( pFriends->GetFriendPersonaName( player ), wszPlayerName, sizeof( wszPlayerName ) );
	}
	else
	{
		V_snwprintf( wszPlayerName, NELEMS( wszPlayerName ), L"?ERR?%llu", player.ConvertToUint64() );
	}
	g_pVGuiLocalize->ConstructString( s_wszMessage, sizeof( s_wszMessage ), g_pVGuiLocalize->Find( szMessage ), 1, wszPlayerName );

	BaseModUI::GenericConfirmation::Data_t data;
	data.pWindowTitle = szTitle;
	data.pMessageTextW = s_wszMessage;
	data.bOkButtonEnabled = true;
	data.bCancelButtonEnabled = true;
	data.pfnOkCallback = &AcceptSendQuickReport;

	// slightly hacky because we don't actually have the gameui activated right now and we don't want to bring up the pause menu
	BaseModUI::GenericConfirmation *pConfirm = new BaseModUI::GenericConfirmation( this, "GenericConfirmation" );
	pConfirm->MakeReadyForUse();
	pConfirm->SetVisible( true );
	pConfirm->SetUsageData( data );
	pConfirm->Activate();
	pConfirm->SetZPos( 1000 );
}

void CRD_VGUI_Quick_Report_Panel::OnCommand( const char *command )
{
	if ( !V_stricmp( command, "Back" ) )
	{
		SetVisible( false );
	}
	else if ( !V_stricmp( command, "CommendFriendly" ) )
	{
		ConfirmAndSendQuickReport( "#rd_quick_report_friendly", "#rd_quick_report_friendly_confirm", "quick_commend_friendly", m_PlayerID );
	}
	else if ( !V_stricmp( command, "CommendLeader" ) )
	{
		ConfirmAndSendQuickReport( "#rd_quick_report_leader", "#rd_quick_report_leader_confirm", "quick_commend_leader", m_PlayerID );
	}
	else if ( !V_stricmp( command, "CommendTeacher" ) )
	{
		ConfirmAndSendQuickReport( "#rd_quick_report_teacher", "#rd_quick_report_teacher_confirm", "quick_commend_teacher", m_PlayerID );
	}
	else if ( !V_stricmp( command, "ReportCheating" ) )
	{
		ConfirmAndSendQuickReport( "#rd_quick_report_cheating", "#rd_quick_report_cheating_confirm", "quick_abusive_cheating", m_PlayerID );
	}
	else if ( !V_stricmp( command, "ReportGriefing" ) )
	{
		ConfirmAndSendQuickReport( "#rd_quick_report_gameplay", "#rd_quick_report_gameplay_confirm", "quick_abusive_gameplay", m_PlayerID );
	}
	else if ( !V_stricmp( command, "ReportCommunication" ) )
	{
		ConfirmAndSendQuickReport( "#rd_quick_report_communication", "#rd_quick_report_communication_confirm", "quick_abusive_communication", m_PlayerID );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_VGUI_Quick_Report_Panel::SetPlayer( CSteamID player )
{
	Assert( player.BIndividualAccount() );

	m_PlayerID = player;

	InvalidateLayout( false, true );
}

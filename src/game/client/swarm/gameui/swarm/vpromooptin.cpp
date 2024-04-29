#include "cbase.h"
#include "vpromooptin.h"
#include "vgenericconfirmation.h"
#include "gameui_interface.h"
#include "steam/steam_api.h"
#include "vgui/ILocalize.h"
#include "vgui/ISurface.h"
#include "vgui_controls/AnimationController.h"
#include "nb_button.h"
#include "rd_inventory_shared.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern ConVar ui_gameui_modal;
#ifdef RD_7A_DROPS
extern ConVar rd_crafting_material_beta_phase1_show_promo;
ConVar rd_crafting_material_beta_phase1_ignore_already_in( "rd_crafting_material_beta_phase1_ignore_already_in", "0" );

static class CRD_PromoOptIn
{
public:
	void BeginRequest()
	{
		ISteamUser *pUser = SteamUser();
		Assert( pUser );
		if ( !pUser )
		{
			Warning( "Missing ISteamUser! Cannot request ticket!\n" );
			ShowMessage( "#rd_redeem_special_error" );
			return;
		}

		CUIGameData::Get()->OpenWaitScreen( "#rd_redeem_special_waiting_steam" );

		m_hAuthTicket = pUser->GetAuthTicketForWebApi( "redeem_special_4030" );
	}

	void ShowMessage( const char *szMessage )
	{
		if ( CBaseModFrame *pWait = CBaseModPanel::GetSingleton().GetWindow( WT_GENERICWAITSCREEN ) )
		{
			pWait->Close();
		}

		if ( !V_strcmp( szMessage, "#rd_redeem_special_already" ) || !V_strcmp( szMessage, "#rd_redeem_special_unavailable" ) || !V_strcmp( szMessage, "#rd_redeem_special_success" ) )
		{
			rd_crafting_material_beta_phase1_show_promo.SetValue( false );
			engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
		}

		GenericConfirmation *pConfirmation = assert_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, CBaseModPanel::GetSingleton().OpenWindow( WT_MAINMENU, NULL ) ) );
		GenericConfirmation::Data_t data;
		data.pWindowTitle = "";
		data.pMessageText = szMessage;
		data.bOkButtonEnabled = true;
		data.bCancelButtonEnabled = false;
		pConfirmation->SetUsageData( data );
	}

	STEAM_CALLBACK( CRD_PromoOptIn, OnTicket, GetTicketForWebApiResponse_t )
	{
		if ( pParam->m_hAuthTicket != m_hAuthTicket )
			return;

		m_hAuthTicket = k_HAuthTicketInvalid;

		if ( pParam->m_eResult != k_EResultOK )
		{
			Warning( "Request for identity verification ticket resulted in code %d: %s\n", pParam->m_eResult, UTIL_RD_EResultToString( pParam->m_eResult ) );
			ShowMessage( "#rd_redeem_special_error" );
			return;
		}

		char szHexTicket[sizeof( pParam->m_rgubTicket ) * 2 + 1];
		// this sucks for performance (O(n^2)) but we're already on a wait screen so whatever
		V_binarytohex( pParam->m_rgubTicket, pParam->m_cubTicket, szHexTicket, sizeof( szHexTicket ) );

		ISteamHTTP *pHTTP = SteamHTTP();
		Assert( pHTTP );
		if ( !pHTTP )
		{
			Warning( "Missing ISteamHTTP! Cannot request opt-in!\n" );
			ShowMessage( "#rd_redeem_special_error" );
			return;
		}

		HTTPRequestHandle hRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodPOST, "https://stats.reactivedrop.com/api/redeem_special" );
		pHTTP->SetHTTPRequestUserAgentInfo( hRequest, "RDPromoOptIn" );
		pHTTP->SetHTTPRequestGetOrPostParameter( hRequest, "itemid", "4030" );
		pHTTP->SetHTTPRequestGetOrPostParameter( hRequest, "ticket", szHexTicket );
		SteamAPICall_t hCall = k_uAPICallInvalid;
		if ( pHTTP->SendHTTPRequest( hRequest, &hCall ) )
		{
			m_HTTPComplete.Set( hCall, this, &CRD_PromoOptIn::OnHTTPComplete );

			CUIGameData::Get()->UpdateWaitPanel( "#rd_redeem_special_waiting" );
		}
		else
		{
			Warning( "HTTP request sending failed! Cannot request opt-in!\n" );
			ShowMessage( "#rd_redeem_special_error" );
		}
	}

	void OnHTTPComplete( HTTPRequestCompleted_t *pParam, bool bIOFailure )
	{
		if ( bIOFailure )
		{
			Warning( "IO failure! Cannot request opt-in!\n" );
			ShowMessage( "#rd_redeem_special_error" );
			return;
		}

		ISteamHTTP *pHTTP = SteamHTTP();
		Assert( pHTTP );
		if ( !pHTTP )
		{
			Warning( "Missing ISteamHTTP! Cannot request opt-in!\n" );
			ShowMessage( "#rd_redeem_special_error" );
			return;
		}

		if ( !pParam->m_bRequestSuccessful )
		{
			Warning( "Network failure! Cannot request opt-in!\n" );
			ShowMessage( "#rd_redeem_special_error" );
			pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
			return;
		}

		if ( pParam->m_eStatusCode != k_EHTTPStatusCode202Accepted )
		{
			Warning( "Server returned wrong status code %d! Cannot request opt-in!\n", pParam->m_eStatusCode );
			ShowMessage( "#rd_redeem_special_error" );
			pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
			return;
		}

		char szMessage[1024];
		if ( pParam->m_unBodySize >= sizeof( szMessage ) )
		{
			Warning( "Body size (%d) too big! Cannot request opt-in!\n", pParam->m_unBodySize );
			ShowMessage( "#rd_redeem_special_error" );
			pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
			return;
		}

		pHTTP->GetHTTPResponseBodyData( pParam->m_hRequest, ( uint8 * )szMessage, pParam->m_unBodySize );
		pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );

		szMessage[pParam->m_unBodySize] = '\0';
		if ( pParam->m_unBodySize && szMessage[pParam->m_unBodySize - 1] == '\n' )
			szMessage[pParam->m_unBodySize - 1] = '\0';

		// server will return a translation key
		ShowMessage( szMessage );
	}

	HAuthTicket m_hAuthTicket{ k_HAuthTicketInvalid };
	CCallResult<CRD_PromoOptIn, HTTPRequestCompleted_t> m_HTTPComplete;
} s_RD_PromoOptIn;
#endif

PromoOptIn::PromoOptIn( Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName, false, true, false, false )
{
	if ( ui_gameui_modal.GetBool() )
	{
		GameUI().PreventEngineHideGameUI();
	}

	CBaseModPanel::GetSingleton().UpdateBackgroundMusicVolume( 0.0f );

	SetProportional( true );
	SetDeleteSelfOnClose( true );
	SetConsoleStylePanel( true );

#ifdef RD_7A_DROPS
	wchar_t wszPlayerName[k_cwchPersonaNameMax + 1];
	V_UTF8ToUnicode( SteamFriends() ? SteamFriends()->GetPersonaName() : "", wszPlayerName, sizeof( wszPlayerName ) );
	wchar_t wszFlavor[4096];
	g_pVGuiLocalize->ConstructString( wszFlavor, sizeof( wszFlavor ), g_pVGuiLocalize->Find( "#rd_crafting_beta1_signup_flavor" ), 1, wszPlayerName );
	m_pLblFlavor = new vgui::Label( this, "LblFlavor", wszFlavor );
	m_pLblExplanationTitle = new vgui::Label( this, "LblExplanationTitle", "#rd_crafting_beta1_signup_title" );
	m_pLblExplanation = new vgui::Label( this, "LblExplanation", "#rd_crafting_beta1_signup_explanation" );
	m_pBtnDecline = new CNB_Button( this, "BtnDecline", "#rd_crafting_beta1_signup_decline", this, "Decline" );
	m_pBtnDecline->SetControllerButton( KEY_XBUTTON_B );
	m_pBtnAccept = new CNB_Button( this, "BtnAccept", "#rd_crafting_beta1_signup_accept", this, "Accept" );
	m_pBtnAccept->SetControllerButton( KEY_XBUTTON_X );
	m_pBtnAlready = new CNB_Button( this, "BtnAlready", "#rd_crafting_beta1_signup_already", this, "Back" );
	m_pBtnAlready->SetControllerButton( KEY_XBUTTON_B );
#endif
}

PromoOptIn::~PromoOptIn()
{
	GameUI().AllowEngineHideGameUI();

	CBaseModPanel::GetSingleton().UpdateBackgroundMusicVolume( 1.0f );
}

void PromoOptIn::Activate()
{
	BaseClass::Activate();

	MakeReadyForUse();

#ifdef RD_7A_DROPS
	surface()->PlaySound( "buttons/button1.wav" );

	m_pLblFlavor->SetAlpha( 0 );
	GetAnimationController()->RunAnimationCommand( m_pLblFlavor, "alpha", 255, 2.0f, 2.0f, AnimationController::INTERPOLATOR_LINEAR );

	m_pLblExplanationTitle->SetAlpha( 0 );
	m_pLblExplanation->SetAlpha( 0 );
	GetAnimationController()->RunAnimationCommand( m_pLblExplanationTitle, "alpha", 255, 5.0f, 1.0f, AnimationController::INTERPOLATOR_LINEAR );
	GetAnimationController()->RunAnimationCommand( m_pLblExplanation, "alpha", 255, 5.0f, 1.0f, AnimationController::INTERPOLATOR_LINEAR );

	CUtlVector<ReactiveDropInventory::ItemInstance_t> optin;
	ReactiveDropInventory::GetItemsForDef( optin, 4029 );

	if ( !rd_crafting_material_beta_phase1_ignore_already_in.GetBool() && optin.Count() )
	{
		m_pBtnDecline->SetVisible( false );
		m_pBtnAccept->SetVisible( false );

		m_pBtnAlready->SetAlpha( 0 );
		GetAnimationController()->RunAnimationCommand( m_pBtnAlready, "alpha", 255, 5.5f, 1.0f, AnimationController::INTERPOLATOR_LINEAR );
	}
	else
	{
		m_pBtnAlready->SetVisible( false );

		m_pBtnAccept->SetAlpha( 0 );
		m_pBtnDecline->SetAlpha( 0 );
		GetAnimationController()->RunAnimationCommand( m_pBtnAccept, "alpha", 255, 5.5f, 1.0f, AnimationController::INTERPOLATOR_LINEAR );
		GetAnimationController()->RunAnimationCommand( m_pBtnDecline, "alpha", 255, 5.5f, 1.0f, AnimationController::INTERPOLATOR_LINEAR );
	}
#endif
}

void PromoOptIn::OnCommand( const char *command )
{
	if ( FStrEq( command, "Back" ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
#ifdef RD_7A_DROPS
	else if ( FStrEq( command, "Decline" ) )
	{
		rd_crafting_material_beta_phase1_show_promo.SetValue( false );
		engine->ClientCmd_Unrestricted( "host_writeconfig\n" );
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_B, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
	else if ( FStrEq( command, "Accept" ) )
	{
		OnKeyCodePressed( ButtonCodeToJoystickButtonCode( KEY_XBUTTON_X, CBaseModPanel::GetSingleton().GetLastActiveUserId() ) );
	}
#endif
	else
	{
		BaseClass::OnCommand( command );
	}
}

void PromoOptIn::OnKeyCodePressed( KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	BaseModUI::CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	if ( code == KEY_XBUTTON_X )
	{
		Close();

#ifdef RD_7A_DROPS
		s_RD_PromoOptIn.BeginRequest();
#endif
	}
	else
	{
		BaseClass::OnKeyCodePressed( keycode );
	}
}

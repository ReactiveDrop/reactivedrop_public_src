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
#include "engine/IEngineSound.h"
#include "soundenvelope.h"
#include "MultiFontRichText.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
using namespace BaseModUI;

extern ConVar ui_gameui_modal;
#if 0
static class CRD_PromoOptIn
{
public:
	void BeginRequest()
	{
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		ISteamUser *pUser = SteamUser();
#else
		ISteamUser *pUser = SteamUser();
#endif
		Assert( pUser );
		if ( !pUser )
		{
			Warning( "Missing ISteamUser! Cannot request ticket!\n" );
			ShowMessage( "#rd_redeem_special_error" );
			return;
		}

		CUIGameData::Get()->OpenWaitScreen( "#rd_redeem_special_waiting_steam" );

#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		m_hAuthTicket = pUser->GetAuthTicketForWebApi( "redeem_special_4034" );
#else
		m_hAuthTicket = pUser->GetAuthTicketForWebApi( "redeem_special_4034" );
#endif
	}

	void ShowMessage( const char *szMessage )
	{
		if ( CBaseModFrame *pWait = CBaseModPanel::GetSingleton().GetWindow( WT_GENERICWAITSCREEN ) )
		{
			pWait->Close();
		}

		if ( !V_strcmp( szMessage, "#rd_redeem_special_already" ) || !V_strcmp( szMessage, "#rd_redeem_special_unavailable" ) || !V_strcmp( szMessage, "#rd_redeem_special_success" ) )
		{
			rd_crafting_material_beta_phase2_show_promo.SetValue( false );
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

#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	STEAM_CALLBACK( CRD_PromoOptIn, OnTicket, GetTicketForWebApiResponse_t )
#else
	STEAM_CALLBACK( CRD_PromoOptIn, OnTicket, GetTicketForWebApiResponse_t )
#endif
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
		UTIL_RD_BinToHex( pParam->m_rgubTicket, pParam->m_cubTicket, szHexTicket, sizeof( szHexTicket ) );

#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		ISteamHTTP *pHTTP = SteamHTTP();
#else
		ISteamHTTP *pHTTP = SteamHTTP();
#endif
		Assert( pHTTP );
		if ( !pHTTP )
		{
			Warning( "Missing ISteamHTTP! Cannot request opt-in!\n" );
			ShowMessage( "#rd_redeem_special_error" );
			return;
		}

		#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		HTTPRequestHandle hRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodPOST, "https://stats.reactivedrop.com/api/redeem_special" );
		#else
		HTTPRequestHandle hRequest = pHTTP->CreateHTTPRequest( k_EHTTPMethodPOST, "https://stats.reactivedrop.com/api/redeem_special" );
		#endif
		#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		pHTTP->SetHTTPRequestUserAgentInfo( hRequest, "RDPromoOptIn" );
		#else
		pHTTP->SetHTTPRequestUserAgentInfo( hRequest, "RDPromoOptIn" );
		#endif
		#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		pHTTP->SetHTTPRequestGetOrPostParameter( hRequest, "itemid", "4034" );
		#else
		pHTTP->SetHTTPRequestGetOrPostParameter( hRequest, "itemid", "4034" );
		#endif
		#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		pHTTP->SetHTTPRequestGetOrPostParameter( hRequest, "ticket", szHexTicket );
		#else
		pHTTP->SetHTTPRequestGetOrPostParameter( hRequest, "ticket", szHexTicket );
		#endif
		SteamAPICall_t hCall = k_uAPICallInvalid;
		#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		if ( pHTTP->SendHTTPRequest( hRequest, &hCall ) )
		#else
		if ( pHTTP->SendHTTPRequest( hRequest, &hCall ) )
		#endif
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

#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		ISteamHTTP *pHTTP = SteamHTTP();
#else
		ISteamHTTP *pHTTP = SteamHTTP();
#endif
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
			#if defined( STEAMAPPS_INTERFACE_VERSION008 )
			pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
			#else
			pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
			#endif
			return;
		}

		if ( pParam->m_eStatusCode != k_EHTTPStatusCode202Accepted )
		{
			Warning( "Server returned wrong status code %d! Cannot request opt-in!\n", pParam->m_eStatusCode );
			ShowMessage( "#rd_redeem_special_error" );
			#if defined( STEAMAPPS_INTERFACE_VERSION008 )
			pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
			#else
			pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
			#endif
			return;
		}

		char szMessage[1024];
		if ( pParam->m_unBodySize >= sizeof( szMessage ) )
		{
			Warning( "Body size (%d) too big! Cannot request opt-in!\n", pParam->m_unBodySize );
			ShowMessage( "#rd_redeem_special_error" );
			#if defined( STEAMAPPS_INTERFACE_VERSION008 )
			pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
			#else
			pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
			#endif
			return;
		}

		#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		pHTTP->GetHTTPResponseBodyData( pParam->m_hRequest, ( uint8 * )szMessage, pParam->m_unBodySize );
		#else
		pHTTP->GetHTTPResponseBodyData( pParam->m_hRequest, ( uint8 * )szMessage, pParam->m_unBodySize );
		#endif
		#if defined( STEAMAPPS_INTERFACE_VERSION008 )
		pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
		#else
		pHTTP->ReleaseHTTPRequest( pParam->m_hRequest );
		#endif

		szMessage[pParam->m_unBodySize] = '\0';
		if ( pParam->m_unBodySize && szMessage[pParam->m_unBodySize - 1] == '\n' )
			szMessage[pParam->m_unBodySize - 1] = '\0';

		ReactiveDropInventory::RequestFullInventoryRefresh();

		// server will return a translation key
		ShowMessage( szMessage );
	}

	HAuthTicket m_hAuthTicket{ k_HAuthTicketInvalid };
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	CCallResult<CRD_PromoOptIn, HTTPRequestCompleted_t> m_HTTPComplete;
#else
	CCallResult<CRD_PromoOptIn, HTTPRequestCompleted_t> m_HTTPComplete;
#endif
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
	m_hBackgroundNoiseLoop = 0;

	SetProportional( true );
	SetDeleteSelfOnClose( true );
	SetConsoleStylePanel( true );

#if 0
	wchar_t wszPlayerName[k_cwchPersonaNameMax + 1];
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	V_UTF8ToUnicode( SteamFriends() ? SteamFriends()->GetPersonaName() : "", wszPlayerName, sizeof( wszPlayerName ) );
#else
	V_UTF8ToUnicode( SteamFriends() ? SteamFriends()->GetPersonaName() : "", wszPlayerName, sizeof( wszPlayerName ) );
#endif
	wchar_t wszFlavor[4096];
	g_pVGuiLocalize->ConstructString( wszFlavor, sizeof( wszFlavor ), g_pVGuiLocalize->Find( "#rd_crafting_beta2_signup_flavor" ), 1, wszPlayerName );
	m_pLblFlavor = new MultiFontRichText( this, "LblFlavor" );
	m_pLblFlavor->SetDrawTextOnly();
	m_pLblFlavor->InsertColorChange( Color{ 0, 255, 0, 255 } );
	m_pLblFlavor->InsertString( wszFlavor );

	m_pLblFlavor->InsertZbalermornaString( "\ndoi li'ai " );
#if defined( STEAMAPPS_INTERFACE_VERSION008 )
	AccountID_t iAccount = SteamUser() ? SteamUser()->GetSteamID().GetAccountID() : 0;
#else
	AccountID_t iAccount = SteamUser() ? SteamUser()->GetSteamID().GetAccountID() : 0;
#endif
	m_pLblFlavor->InsertString( UTIL_RD_ZbalermornaNumberHex( iAccount ) );
	m_pLblFlavor->InsertZbalermornaString( " jatna i xu do sidju i do ba penmi lo derxi" );

	m_pLblExplanationTitle = new vgui::Label( this, "LblExplanationTitle", "#rd_crafting_beta2_signup_title" );
	m_pLblExplanation = new vgui::Label( this, "LblExplanation", "#rd_crafting_beta2_signup_explanation" );
	m_pBtnDecline = new CNB_Button( this, "BtnDecline", "#rd_crafting_beta1_signup_decline", this, "Decline" );
	m_pBtnDecline->SetControllerButton( KEY_XBUTTON_B );
	m_pBtnAccept = new CNB_Button( this, "BtnAccept", "#rd_crafting_beta1_signup_accept", this, "Accept" );
	m_pBtnAccept->SetControllerButton( KEY_XBUTTON_X );
	m_pBtnAlready = new CNB_Button( this, "BtnAlready", "#rd_crafting_beta2_signup_already", this, "Back" );
	m_pBtnAlready->SetControllerButton( KEY_XBUTTON_B );
#endif
}

PromoOptIn::~PromoOptIn()
{
	GameUI().AllowEngineHideGameUI();

	CBaseModPanel::GetSingleton().UpdateBackgroundMusicVolume( 1.0f );

	if ( m_hBackgroundNoiseLoop )
	{
		enginesound->StopSoundByGuid( m_hBackgroundNoiseLoop );
		m_hBackgroundNoiseLoop = 0;
	}
}

void PromoOptIn::Activate()
{
	BaseClass::Activate();

	MakeReadyForUse();

	m_pLblFlavor->SetCursor( dc_arrow );
	m_pLblFlavor->SetVerticalScrollbar( false );

#if 0
	CSoundParameters params;
	if ( g_pSoundEmitterSystem->GetParametersForSound( "swarm.gameeffects.bigelevatorstop", params, GENDER_NONE, true ) )
	{
		enginesound->EmitAmbientSound( params.soundname, params.volume, params.pitch );
	}

	if ( g_pSoundEmitterSystem->GetParametersForSound( "ambient.atmosphere.indoor2", params, GENDER_NONE, true ) )
	{
		enginesound->EmitAmbientSound( params.soundname, params.volume * 0.25f, params.pitch );
		m_hBackgroundNoiseLoop = enginesound->GetGuidForLastSoundEmitted();
	}

	m_pLblFlavor->SetAlpha( 0 );
	GetAnimationController()->RunAnimationCommand( m_pLblFlavor, "alpha", 255, 2.0f, 2.0f, AnimationController::INTERPOLATOR_LINEAR );

	m_pLblExplanationTitle->SetAlpha( 0 );
	m_pLblExplanation->SetAlpha( 0 );
	GetAnimationController()->RunAnimationCommand( m_pLblExplanationTitle, "alpha", 255, 5.0f, 1.0f, AnimationController::INTERPOLATOR_LINEAR );
	GetAnimationController()->RunAnimationCommand( m_pLblExplanation, "alpha", 255, 5.0f, 1.0f, AnimationController::INTERPOLATOR_LINEAR );

	CUtlVector<ReactiveDropInventory::ItemInstance_t> optin;
	ReactiveDropInventory::GetItemsForDef( optin, 4033 );

	if ( !rd_crafting_material_beta_phase2_ignore_already_in.GetBool() && optin.Count() )
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
#if 0
	else if ( FStrEq( command, "Decline" ) )
	{
		rd_crafting_material_beta_phase2_show_promo.SetValue( false );
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

#if 0
		s_RD_PromoOptIn.BeginRequest();
#endif
	}
	else
	{
		BaseClass::OnKeyCodePressed( keycode );
	}
}

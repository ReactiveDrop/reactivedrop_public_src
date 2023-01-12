#include "cbase.h"
#include "rd_steam_input.h"
#include "steam/steam_api.h"
#include "filesystem.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/TextEntry.h"
#include "vgui/IInput.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"
#include "lodepng.h"
#include "asw_gamerules.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"
#include "asw_marine_profile.h"
#include "inputsystem/iinputsystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define RD_INPUT_GLYPH_SIZE k_ESteamInputGlyphSize_Medium
#define RD_INPUT_GLYPH_STYLE ESteamInputGlyphStyle_Light

ConVar rd_gamepad_soft_keyboard( "rd_gamepad_soft_keyboard", "1", FCVAR_NONE, "Automatically open Steam Deck style keyboard when focusing a text field." );
ConVar rd_force_power_of_two_controller_glyphs( "rd_force_power_of_two_controller_glyphs", "0", FCVAR_NONE, "Shrink controller glyphs until they are a power-of-two size to avoid scaling artifacts." );
ConVar rd_force_controller_glyph_set( "rd_force_controller_glyph_set", "-1", FCVAR_ARCHIVE, "Use a specific controller button set for UI hints. 3=xbox, 10=switch, 13=ps5, 14=steam deck", true, -1, true, k_ESteamInputType_Count - 1 );

CRD_Steam_Input g_RD_Steam_Input;

const static struct
{
	EXboxOrigin XBoxOrigin;
	ButtonCode_t ButtonCode;
	const char *BindingName;
	const wchar_t *BitmapFontText;
} s_XInputTable[] =
{
	{ k_EXboxOrigin_A, KEY_XBUTTON_A, "A_BUTTON", L"A" },
	{ k_EXboxOrigin_B, KEY_XBUTTON_B, "B_BUTTON", L"B" },
	{ k_EXboxOrigin_X, KEY_XBUTTON_X, "X_BUTTON", L"X" },
	{ k_EXboxOrigin_Y, KEY_XBUTTON_Y, "Y_BUTTON", L"Y" },
	{ k_EXboxOrigin_LeftBumper, KEY_XBUTTON_LEFT_SHOULDER, "L_SHOULDER", L"2" },
	{ k_EXboxOrigin_RightBumper, KEY_XBUTTON_RIGHT_SHOULDER, "R_SHOULDER", L"3" },
	{ k_EXboxOrigin_Menu, KEY_XBUTTON_START, "START", L"5" },
	{ k_EXboxOrigin_View, KEY_XBUTTON_BACK, "BACK", L"4" },
	{ k_EXboxOrigin_LeftTrigger_Click, KEY_XBUTTON_LTRIGGER, "L_TRIGGER", L"0" },
	{ k_EXboxOrigin_RightTrigger_Click, KEY_XBUTTON_RTRIGGER, "R_TRIGGER", L"1" },
	{ k_EXboxOrigin_LeftStick_Click, KEY_XBUTTON_STICK1, "STICK1", L"6" },
	{ k_EXboxOrigin_LeftStick_Move, KEY_XSTICK1_UP, "S1_UP", L"6" },
	{ k_EXboxOrigin_RightStick_Click, KEY_XBUTTON_STICK2, "STICK2", L"7" },
	{ k_EXboxOrigin_RightStick_Move, KEY_XSTICK2_UP, "S2_UP", L"7" },
	{ k_EXboxOrigin_DPad_North, KEY_XBUTTON_UP, "UP", L"U" },
	{ k_EXboxOrigin_DPad_South, KEY_XBUTTON_DOWN, "DOWN", L"D" },
	{ k_EXboxOrigin_DPad_West, KEY_XBUTTON_LEFT, "LEFT", L"L" },
	{ k_EXboxOrigin_DPad_East, KEY_XBUTTON_RIGHT, "RIGHT", L"R" },
};

CRD_Steam_Input::CRD_Steam_Input() :
	CAutoGameSystemPerFrame{ "CRD_Steam_Input" },
	m_bInitialized{ false },
	m_GlyphTextures{ DefLessFunc( EInputActionOrigin ) }
{
}

void CRD_Steam_Input::PostInit()
{
	Assert( !m_bInitialized );
	if ( m_bInitialized )
	{
		Warning( "Steam Input is already initialized.\n" );
		return;
	}

	ISteamInput *pSteamInput = SteamInput();
	Assert( pSteamInput );
	if ( !pSteamInput )
		return;

	bool bSuccess = pSteamInput->Init( false );
	Assert( bSuccess );
	if ( !bSuccess )
		Warning( "ISteamInput::Init returned failure status\n" );

#ifdef RD_STEAM_INPUT_ACTIONS
	char szCWD[MAX_PATH];
	V_GetCurrentDirectory( szCWD, sizeof( szCWD ) );
	CUtlString szInputActionManifest = CUtlString::PathJoin( szCWD, "steam_input/steam_input_manifest.vdf" );
	bSuccess = pSteamInput->SetInputActionManifestFilePath( szInputActionManifest );
	Assert( bSuccess );
	if ( !bSuccess )
		Warning( "ISteamInput::SetInputActionManifestFilePath returned failure status\n" );
#endif

	pSteamInput->EnableDeviceCallbacks();
	pSteamInput->EnableActionEventCallbacks( &OnActionEvent );

#ifdef RD_STEAM_INPUT_ACTIONS
#define INIT_ACTION_SET( name ) { m_ActionSets.name = pSteamInput->GetActionSetHandle( #name ); Assert( m_ActionSets.name ); if ( !m_ActionSets.name ) Warning( "Could not find Steam Input action set '%s'\n", #name ); }
#define INIT_ACTION_SET_LAYER( name ) { m_ActionSetLayers.name = pSteamInput->GetActionSetHandle( #name ); Assert( m_ActionSetLayers.name ); if ( !m_ActionSetLayers.name ) Warning( "Could not find Steam Input action layer '%s'\n", #name ); }
#define INIT_ANALOG_ACTION( name ) { m_AnalogActions.name = pSteamInput->GetAnalogActionHandle( #name ); Assert( m_AnalogActions.name ); if ( !m_AnalogActions.name ) Warning( "Could not find Steam Input analog action '%s'\n", #name ); }

	// Action Sets
	INIT_ACTION_SET( InGame );

	// Action Set Layers

	// Digital Actions
	for ( CRD_Steam_Input_Bind *pBind = CRD_Steam_Input_Bind::s_pBinds; pBind; pBind = pBind->m_pNext )
	{
		pBind->m_hAction = pSteamInput->GetDigitalActionHandle( pBind->m_szActionName );
		AssertMsg1( pBind->m_hAction, "could not find digital action '%s'", pBind->m_szActionName );
		if ( !pBind->m_hAction )
			Warning( "Could not find Steam Input digital action '%s'\n", pBind->m_szActionName );
	}

	// Analog Actions
	INIT_ANALOG_ACTION( Move );
	INIT_ANALOG_ACTION( Look );
#endif

	m_bInitialized = true;
}

void CRD_Steam_Input::Shutdown()
{
	if ( !m_bInitialized )
		return;

	ISteamInput *pSteamInput = SteamInput();
	Assert( pSteamInput );
	if ( !pSteamInput )
		return;

	bool bSuccess = pSteamInput->Shutdown();
	Assert( bSuccess );
	if ( !bSuccess )
		Warning( "ISteamInput::Shutdown returned failure status\n" );

	m_Controllers.PurgeAndDeleteElements();

	m_bInitialized = false;
}

void CRD_Steam_Input::Update( float frametime )
{
	if ( rd_gamepad_soft_keyboard.GetBool() )
	{
		// Check for text input focus even if we're not using Steam Input.
		static vgui::DHANDLE<vgui::TextEntry> s_hTextEntryFocus;
		vgui::TextEntry *pTextEntry = dynamic_cast< vgui::TextEntry * >( vgui::ipanel()->GetPanel( vgui::input()->GetFocus(), vgui::GetControlsModuleName() ) );
		if ( s_hTextEntryFocus != pTextEntry )
		{
			s_hTextEntryFocus = pTextEntry;

			ISteamUtils *pSteamUtils = SteamUtils();
			Assert( pSteamUtils );
			if ( pSteamUtils )
			{
				if ( pTextEntry )
				{
					int x, y, w, t;
					vgui::ipanel()->GetAbsPos( pTextEntry->GetVPanel(), x, y );
					pTextEntry->GetSize( w, t );
					pSteamUtils->ShowFloatingGamepadTextInput( pTextEntry->IsMultiline() ? k_EFloatingGamepadTextInputModeModeMultipleLines : k_EFloatingGamepadTextInputModeModeSingleLine, x, y, w, t );
				}
				else
				{
					pSteamUtils->DismissFloatingGamepadTextInput();
				}
			}
		}
	}

	if ( !m_bInitialized )
		return;

	ISteamInput *pSteamInput = SteamInput();
	Assert( pSteamInput );
	if ( !pSteamInput )
		return;

	pSteamInput->RunFrame();

	FOR_EACH_VEC( m_Controllers, i )
	{
		m_Controllers[i]->OnFrame( pSteamInput );
	}
}

CRD_Steam_Controller *CRD_Steam_Input::FindController( InputHandle_t hController ) const
{
	// The size of the controller vector is limited by the number of physical controllers
	// the user has connected during the current play session. We can use linear search here
	// unless we need to handle a user plugging in millions of different controllers efficiently.

	FOR_EACH_VEC( m_Controllers, i )
	{
		if ( m_Controllers[i]->m_hController == hController )
		{
			return m_Controllers[i];
		}
	}

	return NULL;
}

CRD_Steam_Controller *CRD_Steam_Input::FindOrAddController( InputHandle_t hController )
{
	CRD_Steam_Controller *pController = FindController( hController );
	if ( !pController )
	{
		pController = new CRD_Steam_Controller( hController );
		m_Controllers.AddToTail( pController );
	}

	return pController;
}

int CRD_Steam_Input::GetJoystickCount()
{
	int iCount = 0;
	FOR_EACH_VEC( m_Controllers, i )
	{
		if ( m_Controllers[i]->m_bConnected )
		{
			iCount++;
		}
	}

	return iCount ? iCount : inputsystem->GetJoystickCount();
}

vgui::HTexture CRD_Steam_Input::GlyphForOrigin( EInputActionOrigin eOrigin )
{
	ISteamInput *pSteamInput = SteamInput();
	Assert( pSteamInput );
	if ( !pSteamInput )
		return NULL;

	if ( rd_force_controller_glyph_set.GetInt() >= 0 )
	{
		eOrigin = pSteamInput->TranslateActionOrigin( static_cast< ESteamInputType >( rd_force_controller_glyph_set.GetInt() ), eOrigin );
	}

	ushort index = m_GlyphTextures.Find( eOrigin );
	if ( m_GlyphTextures.IsValidIndex( index ) )
		return m_GlyphTextures[index];

	const char *szGlyphPath = pSteamInput->GetGlyphPNGForActionOrigin( eOrigin, RD_INPUT_GLYPH_SIZE, RD_INPUT_GLYPH_STYLE );
	Assert( szGlyphPath );
	if ( !szGlyphPath )
	{
		Warning( "No Steam Input glyph for input action origin %d\n", eOrigin );
		return NULL;
	}

	CUtlBuffer buf;
	if ( !g_pFullFileSystem->ReadFile( szGlyphPath, NULL, buf ) )
	{
		Warning( "Failed to read Steam Input glyph from %s\n", szGlyphPath );
		return NULL;
	}

	unsigned char *pixels = NULL;
	unsigned w = 0, h = 0;
	unsigned error = lodepng_decode32( &pixels, &w, &h, static_cast< const unsigned char * >( buf.Base() ), buf.Size() );
	if ( error )
	{
		Warning( "Failed to load Steam Input glyph from %s: error %d: %s\n", szGlyphPath, error, lodepng_error_text( error ) );
		free( pixels );
		return NULL;
	}

	Assert( vgui::surface() );
	vgui::HTexture hTexture = vgui::surface()->CreateNewTextureID( true );
	m_GlyphTextures.Insert( eOrigin, hTexture );
	vgui::surface()->DrawSetTextureRGBA( hTexture, pixels, w, h );
	free( pixels );

	return hTexture;
}

static const char *OriginPlaceholderString( EInputActionOrigin eOrigin )
{
	static CStringPool s_PlaceholderStrings;

	char szPlaceholder[255];
	V_snprintf( szPlaceholder, sizeof( szPlaceholder ), "STEAM_INPUT_ORIGIN_%d", eOrigin );

	Assert( eOrigin != k_EInputActionOrigin_None );

	return s_PlaceholderStrings.Allocate( szPlaceholder );
}

static EInputActionOrigin OriginFromPlaceholderString( const char *szKey )
{
	if ( V_strncmp( szKey, "STEAM_INPUT_ORIGIN_", strlen( "STEAM_INPUT_ORIGIN_" ) ) )
		return k_EInputActionOrigin_None;

	EInputActionOrigin eOrigin = static_cast< EInputActionOrigin >( atoi( szKey + strlen( "STEAM_INPUT_ORIGIN_" ) ) );
	Assert( eOrigin != k_EInputActionOrigin_None );

	return eOrigin;
}

const char *CRD_Steam_Input::Key_LookupBindingEx( const char *pBinding, int iUserId, int iStartCount, int iAllowJoystick )
{
	int iRealUserId = iUserId == -1 ? GET_ACTIVE_SPLITSCREEN_SLOT() : iUserId;

	ISteamInput *pSteamInput = SteamInput();
	Assert( pSteamInput );
	if ( iAllowJoystick != 0 && pSteamInput )
	{
		bool bAny = false;

		for ( CRD_Steam_Input_Bind *pBind = CRD_Steam_Input_Bind::s_pBinds; pBind; pBind = pBind->m_pNext )
		{
			if ( V_stricmp( pBinding, pBind->m_szBind ) && ( pBind->m_szBind[0] != '+' || V_stricmp( pBinding, pBind->m_szBind + 1 ) ) )
				continue;

			FOR_EACH_VEC( m_Controllers, i )
			{
				if ( !m_Controllers[i]->m_bConnected || m_Controllers[i]->m_SplitScreenPlayerIndex != iRealUserId )
					continue;

				EInputActionOrigin origins[STEAM_INPUT_MAX_ORIGINS]{};
				InputActionSetHandle_t hActionSet = pSteamInput->GetCurrentActionSet( m_Controllers[i]->m_hController );
				int count = pSteamInput->GetDigitalActionOrigins( m_Controllers[i]->m_hController, hActionSet, pBind->m_hAction, origins );
				if ( count > iStartCount )
				{
					return OriginPlaceholderString( origins[iStartCount] );
				}

				iStartCount -= count;
				bAny = true;
			}
		}

		// if we have Steam Input binds, don't show Source Engine controller binds for the same command.
		if ( bAny )
		{
			if ( iAllowJoystick > 0 )
				return NULL;

			iAllowJoystick = 0;
		}
	}

	const char *szEngineBind = engine->Key_LookupBindingEx( pBinding, iUserId, iStartCount, iAllowJoystick );
	if ( szEngineBind )
	{
		for ( int i = 0; i < NELEMS( s_XInputTable ); i++ )
		{
			if ( V_strcmp( s_XInputTable[i].BindingName, szEngineBind ) )
				continue;


			FOR_EACH_VEC( m_Controllers, j )
			{
				if ( !m_Controllers[j]->m_bConnected || m_Controllers[j]->m_SplitScreenPlayerIndex != iRealUserId )
					continue;

				return OriginPlaceholderString( pSteamInput->GetActionOriginFromXboxOrigin( m_Controllers[j]->m_hController, s_XInputTable[i].XBoxOrigin ) );
			}
		}
	}

	return szEngineBind;
}

bool CRD_Steam_Input::IsOriginPlaceholderString( const char *szKey )
{
	return OriginFromPlaceholderString( szKey ) != k_EInputActionOrigin_None;
}

const char *CRD_Steam_Input::NameForOrigin( EInputActionOrigin eOrigin )
{
	ISteamInput *pSteamInput = SteamInput();
	Assert( pSteamInput );
	return pSteamInput ? pSteamInput->GetStringForActionOrigin( eOrigin ) : NULL;
}

const char *CRD_Steam_Input::NameForOrigin( const char *szKey )
{
	EInputActionOrigin eOrigin = OriginFromPlaceholderString( szKey );
	return eOrigin ? NameForOrigin( eOrigin ) : NULL;
}

void CRD_Steam_Input::DrawLegacyControllerGlyph( const char *szKey, int x, int y, int iCenterX, int iCenterY, vgui::HFont hFont, int nSlot )
{
	EInputActionOrigin eOrigin = OriginFromPlaceholderString( szKey );
	EXboxOrigin eXboxOrigin = k_EXboxOrigin_Count;
	const wchar_t *wszLegacy = L"";
	for ( int i = 0; i < NELEMS( s_XInputTable ); i++ )
	{
		if ( !V_strcmp( szKey, s_XInputTable[i].BindingName ) )
		{
			eXboxOrigin = s_XInputTable[i].XBoxOrigin;
			wszLegacy = s_XInputTable[i].BitmapFontText;
			break;
		}
	}

	int tall = vgui::surface()->GetFontTall( hFont );

	if ( rd_force_power_of_two_controller_glyphs.GetBool() )
	{
		// hack!
		int scaledTall = LargestPowerOfTwoLessThanOrEqual( tall );
		x += ( tall - scaledTall ) / 2;
		y += ( tall - scaledTall ) / 2;
		tall = scaledTall;
	}

	if ( iCenterX )
		x -= tall * iCenterX / 2;

	if ( iCenterY )
		y -= tall * iCenterY / 2;

	ISteamInput *pSteamInput = SteamInput();
	Assert( pSteamInput );
	if ( pSteamInput && eOrigin != k_EInputActionOrigin_None )
	{
		vgui::HTexture hTexture = GlyphForOrigin( eOrigin );

		vgui::surface()->DrawSetTexture( hTexture );
		vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
		vgui::surface()->DrawTexturedRect( x, y, x + tall, y + tall );

		return;
	}

	Assert( eXboxOrigin != k_EXboxOrigin_Count );
	if ( pSteamInput && eXboxOrigin != k_EXboxOrigin_Count )
	{
		FOR_EACH_VEC( m_Controllers, i )
		{
			if ( !m_Controllers[i]->m_bConnected || m_Controllers[i]->m_SplitScreenPlayerIndex != nSlot )
			{
				continue;
			}

			eOrigin = pSteamInput->GetActionOriginFromXboxOrigin( m_Controllers[i]->m_hController, eXboxOrigin );
			vgui::HTexture hTexture = GlyphForOrigin( eOrigin );

			vgui::surface()->DrawSetTexture( hTexture );
			vgui::surface()->DrawSetColor( 255, 255, 255, 255 );
			vgui::surface()->DrawTexturedRect( x, y, x + tall, y + tall );

			return;
		}
	}

	vgui::surface()->DrawSetTextFont( hFont );
	vgui::surface()->DrawSetTextColor( 255, 255, 255, 255 );
	vgui::surface()->DrawSetTextPos( x, y );
	vgui::surface()->DrawPrintText( wszLegacy, V_wcslen( wszLegacy ) );
}

bool CRD_Steam_Input::GetGameAxes( int nSlot, float *flMoveX, float *flMoveY, float *flLookX, float *flLookY )
{
	ISteamInput *pSteamInput = SteamInput();
	Assert( pSteamInput );
	if ( !pSteamInput )
		return false;

	bool bFoundMove = false, bFoundLook = false;

#ifdef RD_STEAM_INPUT_ACTIONS
	FOR_EACH_VEC( m_Controllers, i )
	{
		if ( !m_Controllers[i]->m_bConnected || m_Controllers[i]->m_SplitScreenPlayerIndex != nSlot )
		{
			continue;
		}

		InputAnalogActionData_t data = pSteamInput->GetAnalogActionData( m_Controllers[i]->m_hController, m_AnalogActions.Move );
		if ( data.bActive )
		{
			if ( !bFoundMove )
			{
				bFoundMove = true;
				*flMoveX = 0;
				*flMoveY = 0;
			}

			*flMoveX += data.x * MAX_BUTTONSAMPLE;
			*flMoveY -= data.y * MAX_BUTTONSAMPLE;
		}

		data = pSteamInput->GetAnalogActionData( m_Controllers[i]->m_hController, m_AnalogActions.Look );
		if ( data.bActive )
		{
			if ( !bFoundLook )
			{
				bFoundLook = true;
				*flLookX = 0;
				*flLookY = 0;
			}

			*flLookX += data.x * MAX_BUTTONSAMPLE;
			*flLookY -= data.y * MAX_BUTTONSAMPLE;
		}
	}
#endif

	// ensure added values from multiple controllers are within expected range
	if ( bFoundMove )
	{
		*flMoveX = clamp<float>( *flMoveX, -MAX_BUTTONSAMPLE, MAX_BUTTONSAMPLE );
		*flMoveY = clamp<float>( *flMoveY, -MAX_BUTTONSAMPLE, MAX_BUTTONSAMPLE );
	}

	if ( bFoundLook )
	{
		*flLookX = clamp<float>( *flLookX, -MAX_BUTTONSAMPLE, MAX_BUTTONSAMPLE );
		*flLookY = clamp<float>( *flLookY, -MAX_BUTTONSAMPLE, MAX_BUTTONSAMPLE );
	}

	return bFoundMove || bFoundLook;
}

void CRD_Steam_Input::OnSteamInputDeviceConnected( SteamInputDeviceConnected_t *pParam )
{
	CRD_Steam_Controller *pController = FindOrAddController( pParam->m_ulConnectedDeviceHandle );
	Assert( pController );

	pController->OnConnected();
}

void CRD_Steam_Input::OnSteamInputDeviceDisconnected( SteamInputDeviceDisconnected_t *pParam )
{
	CRD_Steam_Controller *pController = FindController( pParam->m_ulDisconnectedDeviceHandle );
	if ( !pController )
	{
		return;
	}

	pController->OnDisconnected();
}
void CRD_Steam_Input::OnActionEvent( SteamInputActionEvent_t *pEvent )
{
	CRD_Steam_Controller *pController = g_RD_Steam_Input.FindController( pEvent->controllerHandle );
	Assert( pController );
	if ( !pController )
	{
		DevWarning( "Ignoring SteamInputActionEvent for unknown controller %llu\n", pEvent->controllerHandle );
		return;
	}

	switch ( pEvent->eEventType )
	{
	case ESteamInputActionEventType_DigitalAction:
		pController->OnDigitalAction( pEvent->digitalAction.actionHandle, pEvent->digitalAction.digitalActionData.bState );
		break;
	case ESteamInputActionEventType_AnalogAction:
		pController->OnAnalogAction( pEvent->analogAction.actionHandle, pEvent->analogAction.analogActionData.eMode, pEvent->analogAction.analogActionData.x, pEvent->analogAction.analogActionData.y );
		break;
	default:
		Assert( !"unexpected input action event type" );
		break;
	}
}

CRD_Steam_Controller::CRD_Steam_Controller( InputHandle_t hController ) :
	m_hController{ hController },
	m_bConnected{ false },
	m_SplitScreenPlayerIndex{ -1 }
{
	// controllers are always the first splitscreen player until we have actual splitscreen
	m_SplitScreenPlayerIndex = 0;
}

CRD_Steam_Controller::~CRD_Steam_Controller()
{
}

void CRD_Steam_Controller::OnConnected()
{
	Assert( !m_bConnected );

	m_bConnected = true;
}

void CRD_Steam_Controller::OnDisconnected()
{
	Assert( m_bConnected );

	m_bConnected = false;
}

void CRD_Steam_Controller::OnFrame( ISteamInput *pSteamInput )
{
#ifdef RD_STEAM_INPUT_ACTIONS
	if ( !m_bConnected )
		return;

	// action set layers override what came before them, so we need to activate the most specific/useful layer LAST.
	pSteamInput->DeactivateAllActionSetLayers( m_hController );

#define ACTIVATE_SET( name ) pSteamInput->ActivateActionSet( m_hController, g_RD_Steam_Input.m_ActionSets.name )
#define ACTIVATE_LAYER( name ) pSteamInput->ActivateActionSetLayer( m_hController, g_RD_Steam_Input.m_ActionSetLayers.name )

	C_ASW_Player *pPlayer = m_SplitScreenPlayerIndex == -1 ? NULL : C_ASW_Player::GetLocalASWPlayer( m_SplitScreenPlayerIndex );
	NOTE_UNUSED( pPlayer );

	// for now, we only have one mode
	ACTIVATE_SET( InGame );
#endif
}

void CRD_Steam_Controller::OnDigitalAction( InputDigitalActionHandle_t hAction, bool bState )
{
	Assert( m_bConnected );

	C_ASW_Player *pPlayer = m_SplitScreenPlayerIndex == -1 ? NULL : C_ASW_Player::GetLocalASWPlayer( m_SplitScreenPlayerIndex );

	for ( CRD_Steam_Input_Bind *pBind = CRD_Steam_Input_Bind::s_pBinds; pBind; pBind = pBind->m_pNext )
	{
		if ( pBind->m_hAction != hAction )
			continue;

		if ( !pPlayer )
			return;

		char szCommand[1024];

		bool bIsToggle = pBind->m_szBind[0] == '+';
		if ( bState && bIsToggle )
			V_snprintf( szCommand, sizeof( szCommand ), "cmd%d %s %d\n", m_SplitScreenPlayerIndex + 1, pBind->m_szBind, KEY_STEAM_INPUT );
		else if ( bIsToggle )
			V_snprintf( szCommand, sizeof( szCommand ), "cmd%d -%s %d\n", m_SplitScreenPlayerIndex + 1, pBind->m_szBind + 1, KEY_STEAM_INPUT );
		else if ( bState )
			V_snprintf( szCommand, sizeof( szCommand ), "cmd%d %s\n", m_SplitScreenPlayerIndex + 1, pBind->m_szBind );
		else
			return;

		engine->ClientCmd_Unrestricted( szCommand );

		return;
	}

	//Assert( !"unexpected digital action event type" );
}

void CRD_Steam_Controller::OnAnalogAction( InputAnalogActionHandle_t hAction, EInputSourceMode mode, float x, float y )
{
	Assert( m_bConnected );

	//Assert( !"unexpected analog action event type" );
}

CRD_Steam_Input_Bind *CRD_Steam_Input_Bind::s_pBinds = NULL;

CRD_Steam_Input_Bind::CRD_Steam_Input_Bind( const char *szActionName, const char *szBind ) :
	m_szActionName{ szActionName },
	m_szBind{ szBind }
{
#ifdef RD_STEAM_INPUT_ACTIONS
	m_pNext = s_pBinds;
	s_pBinds = this;
#endif
}

CON_COMMAND_F( rd_reload_controller_glyphs, "force all controller glyphs to be reloaded on next use. probably only useful in a debugger.", FCVAR_HIDDEN )
{
	FOR_EACH_MAP_FAST( g_RD_Steam_Input.m_GlyphTextures, i )
	{
		vgui::surface()->DestroyTextureID( g_RD_Steam_Input.m_GlyphTextures[i] );
	}

	g_RD_Steam_Input.m_GlyphTextures.Purge();
}

CON_COMMAND( rd_steam_input_enable, "" )
{
	g_RD_Steam_Input.PostInit();
}

CON_COMMAND( rd_steam_input_disable, "" )
{
	g_RD_Steam_Input.Shutdown();
}

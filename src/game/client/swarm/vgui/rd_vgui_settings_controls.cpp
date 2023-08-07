#include "cbase.h"
#include "rd_vgui_settings.h"
#include <vgui/IInput.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "gameui/swarm/vgenericconfirmation.h"
#include "gameui/swarm/vhybridbutton.h"
#include "rd_steam_input.h"
#include "inputsystem/iinputsystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace BaseModUI;

DECLARE_BUILD_FACTORY( CRD_VGUI_Settings_Controls );

static void ResetControlsToDefaults()
{
	engine->ClientCmd_Unrestricted( "unbindall; exec config_default\n" );
	CRD_VGUI_Settings::s_bWantSave = true;
	CBaseModPanel::GetSingleton().PlayUISound( UISOUND_ACCEPT );
}

// When binding an action, we capture the mouse pointer so we can bind it to a mouse button or wheel direction.
// When the binding capture state ends, we release the mouse pointer and move it back to where it was to avoid confusing the player.
int CRD_VGUI_Bind::s_iCursorX;
int CRD_VGUI_Bind::s_iCursorY;

CRD_VGUI_Bind::CRD_VGUI_Bind( vgui::Panel *parent, const char *panelName, const char *szLabel, const char *szBind, bool bUseRowLayout ) :
	BaseClass( parent, panelName )
{
	Assert( g_RD_Steam_Input.IsSteamInputBind( szBind ) );

	SetConsoleStylePanel( true );

	V_strncpy( m_szLabel, szLabel, sizeof( m_szLabel ) );
	V_strncpy( m_szBind, szBind, sizeof( m_szBind ) );
	m_bUseRowLayout = bUseRowLayout;
	m_bCapturing = false;

	m_pLblKeyboardIcon = new vgui::Label( this, "LblKeyboardIcon", "" );
	m_pLblKeyboardIconLong = new vgui::Label( this, "LblKeyboardIconLong", "" );
	m_pPnlControllerIcon = new vgui::Panel( this, "PnlControllerIcon" );
	m_pImgClearBind = new vgui::ImagePanel( this, "ImgClearBind" );
	m_pLblDescription = new vgui::Label( this, "LblDescription", szLabel );
	m_pLblNotBound = new vgui::Label( this, "LblNotBound", "" );
}

void CRD_VGUI_Bind::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	LoadControlSettings( m_bUseRowLayout ? "Resource/UI/BaseModUI/CRD_VGUI_Bind_Row.res" : "Resource/UI/BaseModUI/CRD_VGUI_Bind_Box.res" );

	m_pLblDescription->SetText( m_szLabel );

	m_pLblKeyboardIcon->SetMouseInputEnabled( false );
	m_pLblKeyboardIconLong->SetMouseInputEnabled( false );
	m_pPnlControllerIcon->SetMouseInputEnabled( false );
	m_pImgClearBind->SetMouseInputEnabled( false );
	m_pLblDescription->SetMouseInputEnabled( false );
	m_pLblNotBound->SetMouseInputEnabled( false );
}

void CRD_VGUI_Bind::OnKeyCodePressed( vgui::KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_DELETE:
		if ( m_pImgClearBind->IsVisible() )
		{
			ClearKeyboardBind();
			break;
		}

		break;
	case KEY_SPACE:
	case KEY_ENTER:
	case KEY_PAD_ENTER:
		StartKeyboardCapture();

		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

void CRD_VGUI_Bind::OnKeyCodeTyped( vgui::KeyCode keycode )
{
	int lastUser = GetJoystickForCode( keycode );
	CBaseModPanel::GetSingleton().SetLastActiveUserId( lastUser );

	vgui::KeyCode code = GetBaseButtonCode( keycode );

	switch ( code )
	{
	case KEY_XBUTTON_A:
		if ( !SteamInput() || !SteamInput()->ShowBindingPanel( g_RD_Steam_Input.m_hLastControllerWithEvent ) )
		{
			CBaseModPanel::GetSingleton().PlayUISound( UISOUND_INVALID );
		}

		break;
	default:
		BaseClass::OnKeyCodePressed( keycode );
		break;
	}
}

void CRD_VGUI_Bind::OnMouseReleased( vgui::MouseCode code )
{
	switch ( code )
	{
	case MOUSE_LEFT:
		if ( m_pImgClearBind->IsVisible() && m_pImgClearBind->IsCursorOver() )
		{
			ClearKeyboardBind();
			break;
		}

		if ( m_pPnlControllerIcon->IsCursorOver() && g_RD_Steam_Input.Key_LookupBindingEx( m_szBind, -1, 0, 1 ) )
		{
			if ( !SteamInput() || !SteamInput()->ShowBindingPanel( g_RD_Steam_Input.m_hLastControllerWithEvent ) )
			{
				CBaseModPanel::GetSingleton().PlayUISound( UISOUND_INVALID );
			}

			break;
		}

		StartKeyboardCapture();

		break;
	default:
		BaseClass::OnMouseReleased( code );
		break;
	}
}

void CRD_VGUI_Bind::OnCursorEntered()
{
	BaseClass::OnCursorEntered();

	if ( GetParent() )
		NavigateToChild( this );
}

void CRD_VGUI_Bind::NavigateTo()
{
	BaseClass::NavigateTo();

	RequestFocus();
}

void CRD_VGUI_Bind::OnThink()
{
	BaseClass::OnThink();

	const char *szKeyBind = g_RD_Steam_Input.Key_LookupBindingEx( m_szBind, -1, 0, 0 );
	for ( int i = 0; i < m_AlternateBind.Count() && !szKeyBind; i++ )
		szKeyBind = g_RD_Steam_Input.Key_LookupBindingEx( m_AlternateBind[i], -1, 0, 0 );

	if ( m_bCapturing )
	{
		m_pLblKeyboardIcon->SetText( "" );
		m_pLblKeyboardIconLong->SetText( "" );
		m_pLblNotBound->SetVisible( false );
		m_pImgClearBind->SetVisible( false );

		ButtonCode_t code = BUTTON_CODE_INVALID;
		if ( engine->CheckDoneKeyTrapping( code ) )
		{
			m_bCapturing = false;
			vgui::input()->SetMouseCapture( NULL );
			vgui::input()->SetCursorPos( s_iCursorX, s_iCursorY );

			if ( code != BUTTON_CODE_NONE && code != BUTTON_CODE_INVALID && code != KEY_ESCAPE )
			{
				if ( szKeyBind )
					ClearKeyboardBind();

				int iSlot = GET_ACTIVE_SPLITSCREEN_SLOT();
				engine->ClientCmd_Unrestricted( VarArgs( "cmd%d bind \"%s\" \"%s\"", iSlot + 1, g_pInputSystem->ButtonCodeToString( code ), m_szBind ) );
				CRD_VGUI_Settings::s_bWantSave = true;
			}

			RequestFocus();
		}

		return;
	}

	m_pImgClearBind->SetVisible( HasFocus() && szKeyBind );

	if ( szKeyBind )
	{
		if ( const wchar_t *wszTranslation = g_pVGuiLocalize->Find( szKeyBind ) )
		{
			if ( V_wcslen( wszTranslation ) > 2 )
			{
				m_pLblKeyboardIcon->SetText( "" );
				m_pLblKeyboardIconLong->SetText( wszTranslation );
			}
			else
			{
				m_pLblKeyboardIcon->SetText( wszTranslation );
				m_pLblKeyboardIconLong->SetText( "" );
			}
		}
		else
		{
			if ( V_strlen( szKeyBind ) > 2 )
			{
				m_pLblKeyboardIcon->SetText( "" );
				m_pLblKeyboardIconLong->SetText( szKeyBind );
			}
			else
			{
				m_pLblKeyboardIcon->SetText( szKeyBind );
				m_pLblKeyboardIconLong->SetText( "" );
			}
		}
		m_pLblNotBound->SetVisible( false );
	}
	else
	{
		m_pLblKeyboardIcon->SetText( "" );
		m_pLblKeyboardIconLong->SetText( "" );
		m_pLblNotBound->SetVisible( true );
	}
}

void CRD_VGUI_Bind::Paint()
{
	BaseClass::Paint();

	int x, y, w, t;
	if ( m_bUseRowLayout )
	{
		const int nHighlight = 24;

		Color c = m_pLblKeyboardIcon->GetBgColor();
		if ( m_bCapturing )
			c.SetColor( c.r() + nHighlight * 2, c.g() + nHighlight * 2, c.b() + nHighlight * 2, c.a() );
		else if ( HasFocus() )
			c.SetColor( c.r() + nHighlight, c.g() + nHighlight, c.b() + nHighlight, c.a() );
		m_pLblKeyboardIcon->GetBounds( x, y, w, t );
		vgui::surface()->DrawSetColor( c );
		vgui::surface()->DrawFilledRect( YRES( 1 ), y - YRES( 1 ), x + w + YRES( 1 ), y + t + YRES( 1 ) );

		c = m_pLblDescription->GetBgColor();
		if ( HasFocus() && !m_bCapturing )
			c.SetColor( c.r() + nHighlight, c.g() + nHighlight, c.b() + nHighlight, c.a() );
		m_pLblDescription->GetBounds( x, y, w, t );
		vgui::surface()->DrawSetColor( c );
		vgui::surface()->DrawFilledRect( x - YRES( 1 ), y - YRES( 1 ), x + w - YRES( 3 ), y + t + YRES( 1 ) );
		vgui::surface()->DrawFilledRectFade( x + w - YRES( 3 ), y - YRES( 1 ), x + w + YRES( 1 ), y + t + YRES( 1 ), 255, 0, true );
	}
	else
	{
		m_pLblKeyboardIcon->GetBounds( x, y, w, t );
		vgui::surface()->DrawSetColor( 12, 16, 20, 255 );
		vgui::surface()->DrawFilledRect( x, y, x + w, y + t );
	}

	int nBindCount = 0;
	while ( g_RD_Steam_Input.Key_LookupBindingEx( m_szBind, -1, nBindCount, 1 ) )
		nBindCount++;

	if ( nBindCount )
	{
		const char *szKeyBind = g_RD_Steam_Input.Key_LookupBindingEx( m_szBind, -1, int( Plat_FloatTime() ) % nBindCount, 1 );
		Assert( szKeyBind );
		if ( szKeyBind )
		{
			m_pPnlControllerIcon->GetBounds( x, y, w, t );
			g_RD_Steam_Input.DrawLegacyControllerGlyph( szKeyBind, x + w / 2, y + t / 2, 1, 1, m_hButtonFont );
		}
	}
}

void CRD_VGUI_Bind::AddFallbackBind( const char *szBind )
{
	m_AlternateBind.CopyAndAddToTail( szBind );
}

void CRD_VGUI_Bind::StartKeyboardCapture()
{
	m_bCapturing = true;
	vgui::input()->GetCursorPos( s_iCursorX, s_iCursorY );
	vgui::input()->SetMouseFocus( GetVPanel() );
	vgui::input()->SetMouseCapture( GetVPanel() );
	engine->StartKeyTrapMode();
}

void CRD_VGUI_Bind::ClearKeyboardBind()
{
	const char *szKeyBind = g_RD_Steam_Input.Key_LookupBindingEx( m_szBind, -1, 0, 0 );
	for ( int i = 0; i < m_AlternateBind.Count() && !szKeyBind; i++ )
		szKeyBind = g_RD_Steam_Input.Key_LookupBindingEx( m_AlternateBind[i], -1, 0, 0 );

	Assert( szKeyBind && *szKeyBind );
	if ( !szKeyBind )
		return;

	int iSlot = CBaseModPanel::GetSingleton().GetLastActiveUserId();
	engine->ClientCmd_Unrestricted( VarArgs( "cmd%d unbind \"%s\"", iSlot + 1, szKeyBind ) );
	CRD_VGUI_Settings::s_bWantSave = true;
}

CRD_VGUI_Settings_Controls::CRD_VGUI_Settings_Controls( vgui::Panel *parent, const char *panelName ) :
	BaseClass( parent, panelName )
{
	// Movement
	m_pBindMoveForward = new CRD_VGUI_Bind( this, "BindMoveForward", "#ASW_GameUI_MoveForward", "+forward", false );
	SetNavDown( m_pBindMoveForward );
	m_pBindMoveLeft = new CRD_VGUI_Bind( this, "BindMoveLeft", "#ASW_GameUI_MoveLeft", "+moveleft", false );
	m_pBindMoveBack = new CRD_VGUI_Bind( this, "BindMoveBack", "#ASW_GameUI_MoveBack", "+back", false );
	m_pBindMoveRight = new CRD_VGUI_Bind( this, "BindMoveRight", "#ASW_GameUI_MoveRight", "+moveright", false );
	m_pBindWalk = new CRD_VGUI_Bind( this, "BindWalk", "#L4D360UI_Controller_Crouch", "+walk", false );
	m_pBindJump = new CRD_VGUI_Bind( this, "BindJump", "#ASW_GameUI_MoveRoll", "+jump", false );
	m_pLblLeftStickAction = new vgui::Label( this, "LblLeftStickAction", "" );
	m_pLblRightStickAction = new vgui::Label( this, "LblRightStickAction", "" );

	// Controller
	m_pSettingAutoWalk = new CRD_VGUI_Option( this, "SettingAutoWalk", "#rd_controls_auto_walk", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAutoWalk->LinkToConVar( "joy_autowalk", false );
	m_pSettingAutoAttack = new CRD_VGUI_Option( this, "SettingAutoAttack", "#rd_controls_auto_attack", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAutoAttack->LinkToConVar( "joy_autoattack", false );
	m_pSettingAimToMovement = new CRD_VGUI_Option( this, "SettingAimToMovement", "#rd_controls_aim_to_movement", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingAimToMovement->LinkToConVar( "joy_aim_to_movement", false );
	m_pSettingControllerGlyphs = new CRD_VGUI_Option( this, "SettingControllerGlyphs", "#RD_AdvancedSettings_HUD_ControllerIcons", CRD_VGUI_Option::MODE_DROPDOWN );
	m_pSettingControllerGlyphs->AddOption( -1, "#RD_AdvancedSettings_HUD_ControllerIcons_Default", "" );
	m_pSettingControllerGlyphs->AddOption( k_ESteamInputType_XBoxOneController, "#RD_AdvancedSettings_HUD_ControllerIcons_XboxOne", "" );
	m_pSettingControllerGlyphs->AddOption( k_ESteamInputType_SwitchProController, "#RD_AdvancedSettings_HUD_ControllerIcons_Switch", "" );
	m_pSettingControllerGlyphs->AddOption( k_ESteamInputType_PS5Controller, "#RD_AdvancedSettings_HUD_ControllerIcons_PS5", "" );
	m_pSettingControllerGlyphs->AddOption( k_ESteamInputType_SteamDeckController, "#RD_AdvancedSettings_HUD_ControllerIcons_SteamDeck", "" );
	m_pSettingControllerGlyphs->LinkToConVar( "rd_force_controller_glyph_set", false );

	// Actions
	m_pBindPrimaryAttack = new CRD_VGUI_Bind( this, "BindPrimaryAttack", "#Valve_Primary_Attack", "+attack", false );
	m_pBindSecondaryAttack = new CRD_VGUI_Bind( this, "BindSecondaryAttack", "#Valve_Secondary_Attack", "+attack2", false );
	m_pBindUse = new CRD_VGUI_Bind( this, "BindUse", "#asw_use", "+use", false );
	m_pBindSelectPrimary = new CRD_VGUI_Bind( this, "BindSelectPrimary", "#rd_str_switch_to_primary_weapon", "ASW_SelectPrimary", false );
	m_pBindDropWeapon = new CRD_VGUI_Bind( this, "BindDropWeapon", "#asw_drop", "ASW_Drop", false );
	m_pBindSwapWeapons = new CRD_VGUI_Bind( this, "BindSwapWeapons", "#asw_switch_primary_secondary", "ASW_InvLast", false );
	m_pBindSwapWeapons->AddFallbackBind( "ASW_InvNext" );
	m_pBindSwapWeapons->AddFallbackBind( "ASW_InvPrev" );
	m_pBindMeleeAttack = new CRD_VGUI_Bind( this, "BindMeleeAttack", "#asw_melee_attack", "+alt1", false );
	m_pBindReload = new CRD_VGUI_Bind( this, "BindReload", "#Valve_Reload_Weapon", "+reload", false );
	m_pBindSelectSecondary = new CRD_VGUI_Bind( this, "BindSelectSecondary", "#rd_str_switch_to_secondary_weapon", "ASW_SelectSecondary", false );
	m_pBindDropEquipment = new CRD_VGUI_Bind( this, "BindDropEquipment", "#rd_str_drop_extra", "ASW_DropExtra", false );

	// Social / Extras
	m_pBindTextChat = new CRD_VGUI_Bind( this, "BindTextChat", "#Valve_Chat_Message", "say", true );
	m_pBindVoiceChat = new CRD_VGUI_Bind( this, "BindVoiceChat", "#Valve_Use_Voice_Communication", "+voicerecord", true );
	m_pBindWheelDefault = new CRD_VGUI_Bind( this, "BindWheelDefault", "#rd_wheel_default", "+mouse_menu", true );
	m_pBindEmoteGo = new CRD_VGUI_Bind( this, "BindEmoteGo", "#asw_order_marines_follow", "asw_orderMarinesFollow", true );
	m_pBindEmoteStop = new CRD_VGUI_Bind( this, "BindEmoteStop", "#asw_order_marines_hold", "asw_orderMarinesHold", true );
	m_pBindMarinePosition = new CRD_VGUI_Bind( this, "BindMarinePosition", "#asw_order_specific_marine", "+holdorder", true );
	m_pBindEmoteMedic = new CRD_VGUI_Bind( this, "BindEmoteMedic", "#asw_emote_medic", "cl_emote 0", true );
	m_pBindEmoteAmmo = new CRD_VGUI_Bind( this, "BindEmoteAmmo", "#asw_emote_ammo", "cl_emote 1", true );
	m_pBindEmoteQuestion = new CRD_VGUI_Bind( this, "BindEmoteQuestion", "#asw_emote_question", "cl_emote 7", true );
	m_pBindEmoteExclaim = new CRD_VGUI_Bind( this, "BindEmoteExclaim", "#asw_emote_watch_out", "cl_emote 5", true );
	m_pBindVoteYes = new CRD_VGUI_Bind( this, "BindVoteYes", "#asw_vote_yes_key", "vote_yes", true );
	m_pBindVoteNo = new CRD_VGUI_Bind( this, "BindVoteNo", "#asw_vote_no_key", "vote_no", true );
	m_pBindMissionOverview = new CRD_VGUI_Bind( this, "BindMissionOverview", "#asw_ingame_map", "ingamebriefing", true );
	m_pBindPlayerList = new CRD_VGUI_Bind( this, "BindPlayerList", "#asw_playerlist", "playerlist", true );
	m_pBindRotateCameraLeft = new CRD_VGUI_Bind( this, "BindRotateCameraLeft", "#rd_str_rotatecameraleft", "rotatecameraleft", true );
	m_pBindRotateCameraRight = new CRD_VGUI_Bind( this, "BindRotateCameraRight", "#rd_str_rotatecameraright", "rotatecameraright", true );
	m_pBindSecondaryAttackAlt = new CRD_VGUI_Bind( this, "BindSecondaryAttackAlt", "#Valve_Secondary_Attack", "+secondary", true );
	m_pBindChooseMarine = new CRD_VGUI_Bind( this, "BindChooseMarine", "#rd_str_selectloadout", "cl_select_loadout", true );

	// Use Equipment
	m_pBindActivatePrimary = new CRD_VGUI_Bind( this, "BindActivatePrimary", "#rd_bind_ActivatePrimary", "ASW_ActivatePrimary", true );
	m_pBindActivateSecondary = new CRD_VGUI_Bind( this, "BindActivateSecondary", "#rd_bind_ActivateSecondary", "ASW_ActivateSecondary", true );
	m_pBindActivateEquipment[0] = new CRD_VGUI_Bind( this, "BindActivateEquipment0", "#asw_activate_extra", "+grenade1", true );
	for ( int i = 1; i < NELEMS( m_pBindActivateEquipment ); i++ )
	{
		m_pBindActivateEquipment[i] = new CRD_VGUI_Bind( this, CFmtStr{ "BindActivateEquipment%d", i }, CFmtStr{ "#asw_order_marine_%d_equip", i + 1 }, CFmtStr{ "asw_squad_hotbar %d", i }, true );
	}
	m_pBindWheelEquipment = new CRD_VGUI_Bind( this, "BindWheelEquipment", "#rd_wheel_equipment", "+mouse_menu ASW_UseExtra", true );
	m_pBindWheelEquipment1 = new CRD_VGUI_Bind( this, "BindWheelEquipment1", "#rd_wheel_equipment1", "+mouse_menu ASW_UseExtra1", true );
	m_pBindWheelEquipment2 = new CRD_VGUI_Bind( this, "BindWheelEquipment2", "#rd_wheel_equipment2", "+mouse_menu ASW_UseExtra2", true );

	// Select Marine
	for ( int i = 0; i < NELEMS( m_pBindSelectMarine ); i++ )
	{
		m_pBindSelectMarine[i] = new CRD_VGUI_Bind( this, CFmtStr{ "BindSelectMarine%d", i }, CFmtStr{ "#asw_select_marine_%d", i + 1 }, CFmtStr{ "+selectmarine%d", i + 1 }, true );
	}
	m_pBindWheelMarine = new CRD_VGUI_Bind( this, "BindWheelMarine", "#rd_wheel_marine", "+mouse_menu ASW_SelectMarine", true );

	m_pBtnCustomWheels = new BaseModHybridButton( this, "BtnCustomWheels", "#rd_manage_custom_chat_wheels", this, "ManageWheels" );
	m_pBtnCustomWheels->SetEnabled( false ); // TODO!
	m_pBtnResetDefaults = new BaseModHybridButton( this, "BtnResetDefaults", "#L4D360UI_Controller_Default", this, "ResetDefaults" );
	m_pSettingDeveloperConsole = new CRD_VGUI_Option( this, "SettingDeveloperConsole", "#GameUI_DeveloperConsoleCheck", CRD_VGUI_Option::MODE_CHECKBOX );
	m_pSettingDeveloperConsole->LinkToConVar( "con_enable", false );
}

void CRD_VGUI_Settings_Controls::Activate()
{
	NavigateToChild( m_pBindMoveForward );
}

void CRD_VGUI_Settings_Controls::OnThink()
{
	BaseClass::OnThink();

	EInputActionOrigin eMove = g_RD_Steam_Input.OriginFromPlaceholderString( g_RD_Steam_Input.Key_LookupBindingEx( "xmove", -1, 0, 1 ) );
	EInputActionOrigin eLook = g_RD_Steam_Input.OriginFromPlaceholderString( g_RD_Steam_Input.Key_LookupBindingEx( "xlook", -1, 0, 1 ) );

	if ( !SteamInput() || ( eMove == k_EInputActionOrigin_None && eLook == k_EInputActionOrigin_None ) )
	{
		m_pLblLeftStickAction->SetVisible( false );
		m_pLblRightStickAction->SetVisible( false );
	}
	else
	{
		EInputActionOrigin eDeckMove = SteamInput()->TranslateActionOrigin( k_ESteamInputType_SteamDeckController, eMove );
		EInputActionOrigin eDeckLook = SteamInput()->TranslateActionOrigin( k_ESteamInputType_SteamDeckController, eLook );

		m_bMoveStickLeft = eDeckMove == k_EInputActionOrigin_SteamDeck_LeftStick_Move || eDeckMove == k_EInputActionOrigin_SteamDeck_LeftPad_Swipe;
		m_bMoveStickRight = eDeckMove == k_EInputActionOrigin_SteamDeck_RightStick_Move || eDeckMove == k_EInputActionOrigin_SteamDeck_RightPad_Swipe;
		m_bLookStickLeft = eDeckLook == k_EInputActionOrigin_SteamDeck_LeftStick_Move || eDeckLook == k_EInputActionOrigin_SteamDeck_LeftPad_Swipe;
		m_bLookStickRight = eDeckLook == k_EInputActionOrigin_SteamDeck_RightStick_Move || eDeckLook == k_EInputActionOrigin_SteamDeck_RightPad_Swipe;

		if ( m_bMoveStickLeft == m_bLookStickLeft || m_bMoveStickRight == m_bLookStickRight )
		{
			m_bMoveStickLeft = m_bLookStickRight = true;
			m_bMoveStickRight = m_bLookStickLeft = false;
		}
		else if ( !m_bMoveStickLeft && !m_bMoveStickRight )
		{
			m_bMoveStickLeft = !m_bLookStickLeft;
			m_bMoveStickRight = !m_bLookStickRight;
		}
		else if ( !m_bLookStickLeft && !m_bLookStickRight )
		{
			m_bLookStickLeft = !m_bMoveStickLeft;
			m_bLookStickRight = !m_bMoveStickRight;
		}

		m_pLblLeftStickAction->SetVisible( ( m_bMoveStickLeft ? eMove : eLook ) != k_EInputActionOrigin_None );
		m_pLblRightStickAction->SetVisible( ( m_bMoveStickRight ? eMove : eLook ) != k_EInputActionOrigin_None );
		m_pLblLeftStickAction->SetText( m_bMoveStickLeft ? "#rd_stick_move" : "#rd_stick_look" );
		m_pLblRightStickAction->SetText( m_bMoveStickRight ? "#rd_stick_move" : "#rd_stick_look" );
	}
}

void CRD_VGUI_Settings_Controls::OnCommand( const char *command )
{
	if ( FStrEq( command, "ResetDefaults" ) )
	{
		GenericConfirmation *confirmation = assert_cast< GenericConfirmation * >( CBaseModPanel::GetSingleton().OpenWindow( WT_GENERICCONFIRMATION, assert_cast< CRD_VGUI_Settings * >( GetParent() ), false ) );
		Assert( confirmation );
		if ( confirmation )
		{
			GenericConfirmation::Data_t data;
			data.pWindowTitle = "#GameUI_KeyboardSettings";
			data.pMessageText = "#GameUI_KeyboardSettingsText";
			data.bOkButtonEnabled = true;
			data.bCancelButtonEnabled = true;
			data.pfnOkCallback = &ResetControlsToDefaults;
			confirmation->SetUsageData( data );
		}
	}
	else if ( FStrEq( command, "ManageWheels" ) )
	{
		Assert( !"TODO: ManageWheels" );
	}
	else
	{
		BaseClass::OnCommand( command );
	}
}

void CRD_VGUI_Settings_Controls::Paint()
{
	BaseClass::Paint();

	int iSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

	float flMoveX{}, flMoveY{}, flLookX{}, flLookY{};
	g_RD_Steam_Input.GetGameAxes( iSlot, &flMoveX, &flMoveY, &flLookX, &flLookY );

	int x, y, w, t;
	if ( m_pLblLeftStickAction->IsVisible() )
	{
		Assert( m_bMoveStickLeft != m_bLookStickLeft );
		const char *szBind = g_RD_Steam_Input.Key_LookupBindingEx( m_bMoveStickLeft ? "xmove" : "xlook", -1, 0, 1 );

		m_pLblLeftStickAction->GetBounds( x, y, w, t );

		int dx = ( m_bMoveStickLeft ? flMoveX : flLookX ) / MAX_BUTTONSAMPLE * m_flStickTestDistance;
		int dy = ( m_bMoveStickLeft ? flMoveY : flLookY ) / MAX_BUTTONSAMPLE * m_flStickTestDistance;

		g_RD_Steam_Input.DrawLegacyControllerGlyph( szBind, x + w / 2 + dx, y + dy, 1, 0, m_hButtonFont, 0, m_pLblLeftStickAction->GetFgColor() );
	}

	if ( m_pLblRightStickAction->IsVisible() )
	{
		Assert( m_bMoveStickRight != m_bLookStickRight );
		const char *szBind = g_RD_Steam_Input.Key_LookupBindingEx( m_bMoveStickRight ? "xmove" : "xlook", -1, 0, 1 );

		m_pLblRightStickAction->GetBounds( x, y, w, t );

		int dx = ( m_bMoveStickRight ? flMoveX : flLookX ) / MAX_BUTTONSAMPLE * m_flStickTestDistance;
		int dy = ( m_bMoveStickRight ? flMoveY : flLookY ) / MAX_BUTTONSAMPLE * m_flStickTestDistance;

		g_RD_Steam_Input.DrawLegacyControllerGlyph( szBind, x + w / 2 + dx, y + dy, 1, 0, m_hButtonFont, 0, m_pLblRightStickAction->GetFgColor() );
	}
}

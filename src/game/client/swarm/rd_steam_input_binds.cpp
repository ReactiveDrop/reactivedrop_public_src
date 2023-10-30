#include "cbase.h"
#include "rd_steam_input.h"
#include "asw_shareddefs.h"
#include "asw_input.h"
#include "controller_focus.h"
#include "inputsystem/iinputsystem.h"
#include "vgui/IInputInternal.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// attacks
RD_STEAM_INPUT_BIND( PrimaryAttack, "+attack", "InGame" );
RD_STEAM_INPUT_BIND( SecondaryAttack, "+attack2", "InGame" );
RD_STEAM_INPUT_BIND( SecondaryAttackAlt, "+secondary", "InGame" ); // does the same thing as SecondaryAttack; included for consistency with keyboard, which only allows binding one key to each action through the UI.
RD_STEAM_INPUT_BIND( MeleeAttack, "+alt1", "InGame" );
RD_STEAM_INPUT_BIND( OffHandAttack, "+grenade1", "InGame" );

// actions
RD_STEAM_INPUT_BIND( Reload, "+reload", "InGame" );
RD_STEAM_INPUT_BIND( Jump, "+jump", "InGame" );
RD_STEAM_INPUT_BIND( Use, "+use", "InGame" );
RD_STEAM_INPUT_BIND( DropWeapon, "ASW_Drop", "InGame" );
RD_STEAM_INPUT_BIND( DropEquipment, "ASW_DropExtra", "InGame" );

// weapon swap
RD_STEAM_INPUT_BIND( SwapWeapons, "ASW_InvLast", "InGame" );
RD_STEAM_INPUT_BIND( SelectPrimary, "ASW_SelectPrimary", "InGame" );
RD_STEAM_INPUT_BIND( SelectSecondary, "ASW_SelectSecondary", "InGame" );
RD_STEAM_INPUT_BIND( ActivatePrimary, "ASW_ActivatePrimary", "InGame" );
RD_STEAM_INPUT_BIND( ActivateSecondary, "ASW_ActivateSecondary", "InGame" );

// manual movement
RD_STEAM_INPUT_BIND( MoveForward, "+forward", "InGame" );
RD_STEAM_INPUT_BIND( MoveLeft, "+moveleft", "InGame" );
RD_STEAM_INPUT_BIND( MoveBack, "+back", "InGame" );
RD_STEAM_INPUT_BIND( MoveRight, "+moveright", "InGame" );
RD_STEAM_INPUT_BIND( MoveSlow, "+walk", "InGame" );

// manually position a bot marine
RD_STEAM_INPUT_BIND( HoldOrder, "+holdorder", "InGame" );

// emotes
RD_STEAM_INPUT_BIND( EmoteMedic, "cl_emote 0", "InGame" );
RD_STEAM_INPUT_BIND( EmoteAmmo, "cl_emote 1", "InGame" );
RD_STEAM_INPUT_BIND( EmoteSmile, "cl_emote 2", "InGame" );
RD_STEAM_INPUT_BIND( EmoteGo, "asw_OrderMarinesFollow", "InGame" );
RD_STEAM_INPUT_BIND( EmoteStop, "asw_OrderMarinesHold", "InGame" );
RD_STEAM_INPUT_BIND( EmoteExclaim, "cl_emote 5", "InGame" );
RD_STEAM_INPUT_BIND( EmoteSmile2, "cl_emote 6", "InGame" );
RD_STEAM_INPUT_BIND( EmoteQuestion, "cl_emote 7", "InGame" );

// communication
RD_STEAM_INPUT_BIND( TextChat, "say", "InGame" );
RD_STEAM_INPUT_BIND( VoiceChat, "+voicerecord", "InGame" );
RD_STEAM_INPUT_BIND( WheelDefault, "+mouse_menu", "InGame" );
RD_STEAM_INPUT_BIND( WheelMarine, "+mouse_menu ASW_SelectMarine", "InGame" );
RD_STEAM_INPUT_BIND( WheelEquipment, "+mouse_menu ASW_UseExtra", "InGame" );
RD_STEAM_INPUT_BIND( WheelEquipment1, "+mouse_menu ASW_UseExtra1", "InGame" );
RD_STEAM_INPUT_BIND( WheelEquipment2, "+mouse_menu ASW_UseExtra2", "InGame" );
RD_STEAM_INPUT_BIND( VoteYes, "vote_yes", "InGame" );
RD_STEAM_INPUT_BIND( VoteNo, "vote_no", "InGame" );

// UI
RD_STEAM_INPUT_BIND( PlayerList, "playerlist", "InGame" );
RD_STEAM_INPUT_BIND( InGameBriefing, "ingamebriefing", "InGame" );
RD_STEAM_INPUT_BIND( Menu, "gameui_activate", "InGame", true );
RD_STEAM_INPUT_BIND( RotateCameraLeft, "rotatecameraleft", "InGame" );
RD_STEAM_INPUT_BIND( RotateCameraRight, "rotatecameraright", "InGame" );
RD_STEAM_INPUT_BIND( SelectMarineDeathmatch, "cl_select_loadout", "InGame" );

// per-marine
RD_STEAM_INPUT_BIND( SquadOffhand2, "asw_squad_hotbar 1", "InGame" );
RD_STEAM_INPUT_BIND( SquadOffhand3, "asw_squad_hotbar 2", "InGame" );
RD_STEAM_INPUT_BIND( SquadOffhand4, "asw_squad_hotbar 3", "InGame" );
RD_STEAM_INPUT_BIND( SquadOffhand5, "asw_squad_hotbar 4", "InGame" );
RD_STEAM_INPUT_BIND( SquadOffhand6, "asw_squad_hotbar 5", "InGame" );
RD_STEAM_INPUT_BIND( SquadOffhand7, "asw_squad_hotbar 6", "InGame" );
RD_STEAM_INPUT_BIND( SquadOffhand8, "asw_squad_hotbar 7", "InGame" );

RD_STEAM_INPUT_BIND( SelectMarine1, "+selectmarine1", "InGame" );
RD_STEAM_INPUT_BIND( SelectMarine2, "+selectmarine2", "InGame" );
RD_STEAM_INPUT_BIND( SelectMarine3, "+selectmarine3", "InGame" );
RD_STEAM_INPUT_BIND( SelectMarine4, "+selectmarine4", "InGame" );
RD_STEAM_INPUT_BIND( SelectMarine5, "+selectmarine5", "InGame" );
RD_STEAM_INPUT_BIND( SelectMarine6, "+selectmarine6", "InGame" );
RD_STEAM_INPUT_BIND( SelectMarine7, "+selectmarine7", "InGame" );
RD_STEAM_INPUT_BIND( SelectMarine8, "+selectmarine8", "InGame" );

extern vgui::IInputInternal *g_InputInternal;
extern ConVar rd_gamepad_ignore_menus;

static bool GamepadIgnoreMenus()
{
	if ( rd_gamepad_ignore_menus.GetInt() == -1 )
	{
		DevMsg( "detected xbox gamepad count: %d; updating rd_gamepad_ignore_menus\n", g_pInputSystem->GetJoystickCount() );
		rd_gamepad_ignore_menus.SetValue( g_pInputSystem->GetJoystickCount() != 0 );
	}

	return rd_gamepad_ignore_menus.GetBool();
}

static void ButtonPressHelper( ButtonCode_t eButton )
{
	if ( !GamepadIgnoreMenus() )
		g_InputInternal->InternalKeyCodePressed( eButton );
	//GetControllerFocus()->OnControllerButtonPressed( eButton );
}

static void ButtonReleaseHelper( ButtonCode_t eButton )
{
	if ( !GamepadIgnoreMenus() )
		g_InputInternal->InternalKeyCodeReleased( eButton );
	//GetControllerFocus()->OnControllerButtonReleased( eButton );
}

#define RD_STEAM_INPUT_MENU_BIND( ActionName, ConCommandName, eButton, ... ) \
	static void ConCommandName##_press( const CCommand &args ) \
	{ \
		ButtonPressHelper( eButton ); \
	} \
	ConCommand ConCommandName##_press_command( "+" #ConCommandName, &ConCommandName##_press, "helper command for " #eButton, FCVAR_HIDDEN ); \
	static void ConCommandName##_release( const CCommand &args ) \
	{ \
		ButtonReleaseHelper( eButton ); \
	} \
	ConCommand ConCommandName##_release_command( "-" #ConCommandName, &ConCommandName##_release, "helper command for " #eButton, FCVAR_HIDDEN ); \
	RD_STEAM_INPUT_BIND( ActionName, "+" #ConCommandName, __VA_ARGS__ )

RD_STEAM_INPUT_MENU_BIND( MenuConfirm, rd_menu_confirm, KEY_XBUTTON_A, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuBack, rd_menu_back, KEY_XBUTTON_B, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuSpecial1, rd_menu_special_1, KEY_XBUTTON_X, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuSpecial2, rd_menu_special_2, KEY_XBUTTON_Y, "Menus" );

RD_STEAM_INPUT_MENU_BIND( MenuL1, rd_menu_left_bumper, KEY_XBUTTON_LEFT_SHOULDER, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuR1, rd_menu_right_bumper, KEY_XBUTTON_RIGHT_SHOULDER, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuL2, rd_menu_left_trigger, KEY_XBUTTON_LTRIGGER, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuR2, rd_menu_right_trigger, KEY_XBUTTON_RTRIGGER, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuL3, rd_menu_left_stick, KEY_XBUTTON_STICK1, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuR3, rd_menu_right_stick, KEY_XBUTTON_STICK2, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuSelect, rd_menu_select, KEY_XBUTTON_BACK, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuStart, rd_menu_start, KEY_XBUTTON_START, "Menus", true );

RD_STEAM_INPUT_MENU_BIND( MenuLeft, rd_menu_left, KEY_XBUTTON_LEFT, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuRight, rd_menu_right, KEY_XBUTTON_RIGHT, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuUp, rd_menu_up, KEY_XBUTTON_UP, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuDown, rd_menu_down, KEY_XBUTTON_DOWN, "Menus" );

RD_STEAM_INPUT_MENU_BIND( MenuLeftStickLeft, rd_menu_left_stick_left, KEY_XSTICK1_LEFT, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuLeftStickRight, rd_menu_left_stick_right, KEY_XSTICK1_RIGHT, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuLeftStickUp, rd_menu_left_stick_up, KEY_XSTICK1_UP, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuLeftStickDown, rd_menu_left_stick_down, KEY_XSTICK1_DOWN, "Menus" );

RD_STEAM_INPUT_MENU_BIND( MenuRightStickLeft, rd_menu_right_stick_left, KEY_XSTICK2_LEFT, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuRightStickRight, rd_menu_right_stick_right, KEY_XSTICK2_RIGHT, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuRightStickUp, rd_menu_right_stick_up, KEY_XSTICK2_UP, "Menus" );
RD_STEAM_INPUT_MENU_BIND( MenuRightStickDown, rd_menu_right_stick_down, KEY_XSTICK2_DOWN, "Menus" );

#include "cbase.h"
#include "rd_steam_input.h"
#include "asw_shareddefs.h"

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
RD_STEAM_INPUT_BIND( Menu, "gameui_activate", "InGame" );
RD_STEAM_INPUT_BIND( RotateCameraLeft, "rotatecameraleft", "InGame" );
RD_STEAM_INPUT_BIND( RotateCameraRight, "rotatecameraright", "InGame" );
RD_STEAM_INPUT_BIND( SelectMarineDeathmatch, "cl_select_loadout", "InGame" );

// per-marine
COMPILE_TIME_ASSERT( ASW_NUM_MARINE_PROFILES == 8 );

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

#include "cbase.h"
#include "rd_steam_input.h"
#include "asw_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// attacks
RD_STEAM_INPUT_BIND( PrimaryAttack, "+attack" );
RD_STEAM_INPUT_BIND( SecondaryAttack, "+attack2" );
RD_STEAM_INPUT_BIND( SecondaryAttackAlt, "+secondary" ); // does the same thing as SecondaryAttack; included for consistency with keyboard, which only allows binding one key to each action through the UI.
RD_STEAM_INPUT_BIND( MeleeAttack, "+alt1" );
RD_STEAM_INPUT_BIND( OffHandAttack, "+grenade1" );

// actions
RD_STEAM_INPUT_BIND( Reload, "+reload" );
RD_STEAM_INPUT_BIND( Jump, "+jump" );
RD_STEAM_INPUT_BIND( Use, "+use" );
RD_STEAM_INPUT_BIND( DropWeapon, "ASW_Drop" );
RD_STEAM_INPUT_BIND( DropEquipment, "ASW_DropExtra" );

// weapon swap
RD_STEAM_INPUT_BIND( SwapWeapons, "ASW_InvLast" );
RD_STEAM_INPUT_BIND( SelectPrimary, "ASW_SelectPrimary" );
RD_STEAM_INPUT_BIND( SelectSecondary, "ASW_SelectSecondary" );
RD_STEAM_INPUT_BIND( ActivatePrimary, "ASW_ActivatePrimary" );
RD_STEAM_INPUT_BIND( ActivateSecondary, "ASW_ActivateSecondary" );

// manual movement
RD_STEAM_INPUT_BIND( MoveForward, "+forward" );
RD_STEAM_INPUT_BIND( MoveLeft, "+moveleft" );
RD_STEAM_INPUT_BIND( MoveBack, "+back" );
RD_STEAM_INPUT_BIND( MoveRight, "+moveright" );
RD_STEAM_INPUT_BIND( MoveSlow, "+walk" );

// emotes
RD_STEAM_INPUT_BIND( EmoteMedic, "cl_emote 0" );
RD_STEAM_INPUT_BIND( EmoteAmmo, "cl_emote 1" );
RD_STEAM_INPUT_BIND( EmoteSmile, "cl_emote 2" );
RD_STEAM_INPUT_BIND( EmoteGo, "asw_OrderMarinesFollow" );
RD_STEAM_INPUT_BIND( EmoteStop, "asw_OrderMarinesHold" );
RD_STEAM_INPUT_BIND( EmoteExclaim, "cl_emote 5" );
RD_STEAM_INPUT_BIND( EmoteSmile2, "cl_emote 6" );
RD_STEAM_INPUT_BIND( EmoteQuestion, "cl_emote 7" );

// communication
RD_STEAM_INPUT_BIND( TextChat, "say" );
RD_STEAM_INPUT_BIND( VoiceChat, "+voicerecord" );
RD_STEAM_INPUT_BIND( WheelDefault, "+mouse_menu" );
RD_STEAM_INPUT_BIND( WheelMarine, "+mouse_menu ASW_SelectMarine" );
RD_STEAM_INPUT_BIND( WheelEquipment, "+mouse_menu ASW_UseExtra" );
RD_STEAM_INPUT_BIND( WheelEquipment1, "+mouse_menu ASW_UseExtra1" );
RD_STEAM_INPUT_BIND( WheelEquipment2, "+mouse_menu ASW_UseExtra2" );
RD_STEAM_INPUT_BIND( VoteYes, "vote_yes" );
RD_STEAM_INPUT_BIND( VoteNo, "vote_no" );

// UI
RD_STEAM_INPUT_BIND( PlayerList, "playerlist" );
RD_STEAM_INPUT_BIND( InGameBriefing, "ingamebriefing" );
RD_STEAM_INPUT_BIND( Menu, "gameui_activate" );
RD_STEAM_INPUT_BIND( RotateCameraLeft, "rotatecameraleft" );
RD_STEAM_INPUT_BIND( RotateCameraRight, "rotatecameraright" );
RD_STEAM_INPUT_BIND( SelectMarineDeathmatch, "cl_select_loadout" );

// per-marine
COMPILE_TIME_ASSERT( ASW_NUM_MARINE_PROFILES == 8 );

RD_STEAM_INPUT_BIND( SquadOffhand2, "asw_squad_hotbar 1" );
RD_STEAM_INPUT_BIND( SquadOffhand3, "asw_squad_hotbar 2" );
RD_STEAM_INPUT_BIND( SquadOffhand4, "asw_squad_hotbar 3" );
RD_STEAM_INPUT_BIND( SquadOffhand5, "asw_squad_hotbar 4" );
RD_STEAM_INPUT_BIND( SquadOffhand6, "asw_squad_hotbar 5" );
RD_STEAM_INPUT_BIND( SquadOffhand7, "asw_squad_hotbar 6" );
RD_STEAM_INPUT_BIND( SquadOffhand8, "asw_squad_hotbar 7" );

RD_STEAM_INPUT_BIND( SelectMarine1, "+selectmarine1" );
RD_STEAM_INPUT_BIND( SelectMarine2, "+selectmarine2" );
RD_STEAM_INPUT_BIND( SelectMarine3, "+selectmarine3" );
RD_STEAM_INPUT_BIND( SelectMarine4, "+selectmarine4" );
RD_STEAM_INPUT_BIND( SelectMarine5, "+selectmarine5" );
RD_STEAM_INPUT_BIND( SelectMarine6, "+selectmarine6" );
RD_STEAM_INPUT_BIND( SelectMarine7, "+selectmarine7" );
RD_STEAM_INPUT_BIND( SelectMarine8, "+selectmarine8" );

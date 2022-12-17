#include "cbase.h"
#include "rd_steam_input.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


RD_STEAM_INPUT_BIND( PrimaryAttack, "+attack" );
RD_STEAM_INPUT_BIND( SecondaryAttack, "+attack2" );
RD_STEAM_INPUT_BIND( MeleeAttack, "+alt1" );
RD_STEAM_INPUT_BIND( OffHandAttack, "+grenade1" );
RD_STEAM_INPUT_BIND( Reload, "+reload" );
RD_STEAM_INPUT_BIND( Jump, "+jump" );
RD_STEAM_INPUT_BIND( Use, "+use" );
RD_STEAM_INPUT_BIND( SwapWeapons, "ASW_InvLast" );
RD_STEAM_INPUT_BIND( SelectPrimary, "ASW_SelectPrimary" );
RD_STEAM_INPUT_BIND( SelectSecondary, "ASW_SelectSecondary" );
RD_STEAM_INPUT_BIND( ActivatePrimary, "ASW_ActivatePrimary" );
RD_STEAM_INPUT_BIND( ActivateSecondary, "ASW_ActivateSecondary" );

// emotes
RD_STEAM_INPUT_BIND( EmoteMedic, "cl_emote 0" );
RD_STEAM_INPUT_BIND( EmoteAmmo, "cl_emote 1" );
RD_STEAM_INPUT_BIND( EmoteSmile, "cl_emote 2" );
RD_STEAM_INPUT_BIND( EmoteGo, "asw_OrderMarinesFollow" );
RD_STEAM_INPUT_BIND( EmoteStop, "asw_OrderMarinesHold" );
RD_STEAM_INPUT_BIND( EmoteExclaim, "cl_emote 5" );
RD_STEAM_INPUT_BIND( EmoteSmile2, "cl_emote 6" );
RD_STEAM_INPUT_BIND( EmoteQuestion, "cl_emote 7" );

// UI
RD_STEAM_INPUT_BIND( PlayerList, "playerlist" );
RD_STEAM_INPUT_BIND( InGameBriefing, "ingamebriefing" );
RD_STEAM_INPUT_BIND( Menu, "gameui_activate" );
RD_STEAM_INPUT_BIND( VoteYes, "vote_yes" );
RD_STEAM_INPUT_BIND( VoteNo, "vote_no" );

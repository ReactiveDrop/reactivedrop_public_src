//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:		Player for Swarm.  This is an invisible entity that doesn't move, representing the commander.
//                  The player drives movement of CASW_Marine NPC entities
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "asw_player.h"
#include "in_buttons.h"
#include "asw_gamerules.h"
#include "asw_marine_resource.h"
#include "asw_marine.h"
#include "asw_marine_speech.h"
#include "asw_marine_profile.h"
#include "asw_spawner.h"
#include "asw_alien.h"
#include "asw_simple_alien.h"
#include "asw_pickup.h"
#include "asw_use_area.h"
#include "asw_button_area.h"
#include "asw_weapon.h"
#include "asw_ammo.h"
#include "asw_weapon_parse.h"
#include "asw_computer_area.h"
#include "asw_hack.h"
#include "asw_point_camera.h"
#include "ai_waypoint.h"
#include "inetchannelinfo.h"
#include "asw_sentry_base.h"
#include "asw_shareddefs.h"
#include "iasw_vehicle.h"
#include "obstacle_pushaway.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "asw_door.h"
#include "asw_remote_turret_shared.h"
#include "asw_weapon_medical_satchel_shared.h"
#include "Sprite.h"
#include "physics_prop_ragdoll.h"
#include "asw_util_shared.h"
#include "asw_campaign_save.h"
#include "gib.h"
#include "asw_intro_control.h"
#include "asw_weapon_ammo_bag_shared.h"
#include "ai_network.h"
#include "ai_navigator.h"
#include "ai_node.h"
#include "datacache/imdlcache.h"
#include "asw_spawn_manager.h"
#include "sendprop_priorities.h"
#include "asw_deathmatch_mode.h"
#include "asw_trace_filter.h"
#include "env_tonemap_controller.h"
#include "fogvolume.h"
#include "missionchooser/iasw_mission_chooser.h"
#include "missionchooser/iasw_mission_chooser_source.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ASW_PLAYER_MODEL "models/swarm/marine/Marine.mdl"

//#define MARINE_ORDER_DISTANCE 500
#define MARINE_ORDER_DISTANCE 32000

ConVar asw_client_chatter_enabled("asw_client_chatter_enabled", "1", FCVAR_NONE, "If zero cl_chatter is played only for the player who issued the command"); 
ConVar asw_blend_test_scale("asw_blend_test_scale", "0.1f", FCVAR_CHEAT);
ConVar asw_debug_pvs( "asw_debug_pvs", "0", FCVAR_CHEAT );
extern ConVar asw_rts_controls;
extern ConVar asw_DebugAutoAim;
extern ConVar asw_debug_marine_damage;
extern ConVar rd_respawn_time;
extern ConVar asw_default_campaign;

ConVar rm_welcome_message("rm_welcome_message", "", FCVAR_NONE, "This message is displayed to a player after they join the game");
ConVar rm_welcome_message_delay("rm_welcome_message_delay", "10", FCVAR_NONE, "The number of seconds the welcome message is delayed.", true, 0, true, 30);
ConVar rd_kick_inactive_players( "rd_kick_inactive_players", "0", FCVAR_NONE, "If positive, kick players who are inactive for this many seconds." );
ConVar rd_kick_inactive_players_warning( "rd_kick_inactive_players_warning", "0.8", FCVAR_NONE, "Warn players that they will be kicked after this fraction of the inactive time.", true, 0, true, 1 );
ConVar rd_force_all_marines_in_pvs( "rd_force_all_marines_in_pvs", "3", FCVAR_NONE, "Send information about objects near all marines to all players. Helps record more complete demos, but increases memory and bandwidth usage. 2=only for spectators, 3=only for players with rd_auto_record_lobbies enabled" );

static const char* s_pWelcomeMessageContext = "WelcomeMessageDelayedContext";

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEPlayerAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

					CTEPlayerAnimEvent( const char *name ) : CBaseTempEntity( name )
					{
					}

	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent( "PlayerAnimEvent" );

void TE_PlayerAnimEvent( CBasePlayer *pPlayer, PlayerAnimEvent_t event )
{
	CPVSFilter filter( (const Vector&) pPlayer->EyePosition() );
	
	// The player himself doesn't need to be sent his animation events 
	// unless cs_showanimstate wants to show them.
	//if ( asw_showanimstate.GetInt() == pPlayer->entindex() )
	//{
		//filter.RemoveRecipient( pPlayer );
	//}

	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}

// -------------------------------------------------------------------------------- //
// Marine animation event.
// -------------------------------------------------------------------------------- //

class CTEMarineAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEMarineAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

	CTEMarineAnimEvent( const char *name ) : CBaseTempEntity( name )
	{
	}

	CNetworkHandle( CBasePlayer, m_hExcludePlayer );
	CNetworkHandle( CASW_Marine, m_hMarine );
	CNetworkVar( int, m_iEvent );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEMarineAnimEvent, DT_TEMarineAnimEvent )
	SendPropEHandle( SENDINFO( m_hMarine ) ),
	SendPropEHandle( SENDINFO( m_hExcludePlayer ) ),
	SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
END_SEND_TABLE()

static CTEMarineAnimEvent g_TEMarineAnimEvent( "MarineAnimEvent" );

void TE_MarineAnimEvent( CASW_Marine *pMarine, PlayerAnimEvent_t event )
{
	CPVSFilter filter( (const Vector&) pMarine->EyePosition() );

	g_TEMarineAnimEvent.m_hMarine = pMarine;
	g_TEMarineAnimEvent.m_hExcludePlayer = NULL;
	g_TEMarineAnimEvent.m_iEvent = event;
	g_TEMarineAnimEvent.Create( filter, 0 );
}

void TE_MarineAnimEventExceptCommander( CASW_Marine *pMarine, PlayerAnimEvent_t event )
{
	if (!pMarine)
		return;
	CPVSFilter filter( (const Vector&) pMarine->EyePosition() );
	
	if (pMarine->GetCommander() && pMarine->IsInhabited())
		g_TEMarineAnimEvent.m_hExcludePlayer = pMarine->GetCommander();
	else
		g_TEMarineAnimEvent.m_hExcludePlayer = NULL;
		//filter.RemoveRecipient(pMarine->GetCommander());
	g_TEMarineAnimEvent.m_hMarine = pMarine;
	g_TEMarineAnimEvent.m_iEvent = event;
	g_TEMarineAnimEvent.Create( filter, 0 );
}

// NOTE: This animevent won't get recorded in demos properly, since it's not sent to everyone!
void TE_MarineAnimEventJustCommander( CASW_Marine *pMarine, PlayerAnimEvent_t event )
{	
	if (!pMarine || !pMarine->IsInhabited())
		return;
	
	if (!pMarine->GetCommander())
		return;

	CRecipientFilter filter;
	filter.RemoveAllRecipients();
	filter.AddRecipient(pMarine->GetCommander());

	g_TEMarineAnimEvent.m_hExcludePlayer = NULL;
	g_TEMarineAnimEvent.m_hMarine = pMarine;
	g_TEMarineAnimEvent.m_iEvent = event;
	g_TEMarineAnimEvent.Create( filter, 0 );
}

// -------------------------------------------------------------------------------- //
// Tables.
// -------------------------------------------------------------------------------- //

LINK_ENTITY_TO_CLASS( player, CASW_Player );
PRECACHE_REGISTER(player);

IMPLEMENT_SERVERCLASS_ST( CASW_Player, DT_ASW_Player )
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
	
	// cs_playeranimstate and clientside animation takes care of these on the client
	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),

	//SendPropInt		(SENDINFO(m_iHealth), 10 ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 10, 0, SendProxy_AngleToFloat, SENDPROP_PLAYER_EYE_ANGLES_PRIORITY ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 10, 0, SendProxy_AngleToFloat, SENDPROP_PLAYER_EYE_ANGLES_PRIORITY ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 2), 10, 0, SendProxy_AngleToFloat, SENDPROP_PLAYER_EYE_ANGLES_PRIORITY ),
	SendPropEHandle( SENDINFO( m_hInhabiting ) ),
	SendPropEHandle( SENDINFO( m_hSpectating ) ),
	SendPropFloat(   SENDINFO( m_fMarineDeathTime) ),
	SendPropEHandle( SENDINFO( m_hOrderingMarine ) ),
	SendPropEHandle( SENDINFO ( m_pCurrentInfoMessage ) ),

	SendPropInt(SENDINFO(m_iLeaderVoteIndex) ),
	SendPropInt(SENDINFO(m_iKickVoteIndex) ),
	SendPropFloat( SENDINFO( m_fMapGenerationProgress ) ),

	SendPropTime( SENDINFO( m_flUseKeyDownTime ) ),
	SendPropEHandle( SENDINFO ( m_hUseKeyDownEnt ) ),
	SendPropFloat	(SENDINFO( m_flMovementAxisYaw)),
	SendPropInt		(SENDINFO(m_nChangingMR)),
	SendPropInt		(SENDINFO(m_nChangingSlot)),
	SendPropInt	(SENDINFO( m_iMapVoted ) ),
	SendPropInt		(SENDINFO( m_iNetworkedXP ) ),
	SendPropInt		(SENDINFO( m_iNetworkedPromotion ) ),

	// BenLubar(spectator-mouse)
	SendPropInt( SENDINFO( m_iScreenWidth ) ),
	SendPropInt( SENDINFO( m_iScreenHeight ) ),
	SendPropInt( SENDINFO( m_iMouseX ), -1, SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO( m_iMouseY ), -1, SPROP_CHANGES_OFTEN ),
	//

	SendPropBool( SENDINFO( m_bSentJoinedMessage ) ),
	SendPropQAngles( SENDINFO( m_angMarineAutoAimFromClient ), 10, SPROP_CHANGES_OFTEN ),
	SendPropBool( SENDINFO( m_bWantsSpectatorOnly ) ),
	SendPropFloat( SENDINFO( m_flInactiveKickWarning ) ),
	SendPropDataTable( SENDINFO_DT_NAME( m_EquippedItemData[RD_STEAM_INVENTORY_EQUIP_SLOT_FIRST_MEDAL + 0], m_EquippedMedal ), &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ) ),
END_SEND_TABLE()

BEGIN_DATADESC( CASW_Player )
	DEFINE_FIELD( m_fIsWalking, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecLastMarineOrigin, FIELD_VECTOR ),
	DEFINE_FIELD( m_hInhabiting, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hSpectating, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecStoredPosition, FIELD_VECTOR ),
	DEFINE_FIELD( m_pCurrentInfoMessage, FIELD_EHANDLE ),
	//DEFINE_FIELD( m_bLastAttackButton, FIELD_BOOLEAN ),		// keep this at no click after restore
	DEFINE_FIELD( m_iUseEntities, FIELD_INTEGER ),
	DEFINE_AUTO_ARRAY( m_hUseEntities, FIELD_EHANDLE ),
	DEFINE_FIELD( m_fBlendAmount, FIELD_FLOAT ),
	DEFINE_FIELD( m_angEyeAngles, FIELD_VECTOR ),
	DEFINE_FIELD( m_iLeaderVoteIndex, FIELD_INTEGER ),
	DEFINE_FIELD( m_iKickVoteIndex, FIELD_INTEGER ),	
	DEFINE_FIELD( m_iKLVotesStarted, FIELD_INTEGER ),
	DEFINE_FIELD( m_fLastKLVoteTime, FIELD_TIME ),
	DEFINE_FIELD( m_hOrderingMarine, FIELD_EHANDLE ),
	DEFINE_FIELD( m_vecFreeCamOrigin, FIELD_VECTOR ),
	DEFINE_FIELD( m_bUsedFreeCam, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bSentJoinedMessage, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_iMapVoted, FIELD_INTEGER ),
	DEFINE_FIELD( m_fLastControlledMarineTime, FIELD_TIME ),
	DEFINE_FIELD( m_vecCrosshairTracePos, FIELD_VECTOR ),

	// BenLubar(spectator-mouse)
	DEFINE_FIELD( m_iScreenWidth, FIELD_SHORT ),
	DEFINE_FIELD( m_iScreenHeight, FIELD_SHORT ),
	DEFINE_FIELD( m_iMouseX, FIELD_SHORT ),
	DEFINE_FIELD( m_iMouseY, FIELD_SHORT ),
	//

	DEFINE_FIELD( m_angMarineAutoAimFromClient, FIELD_VECTOR ),
	DEFINE_FIELD( m_bWantsSpectatorOnly, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flLastActiveTime, FIELD_TIME ),
	DEFINE_FIELD( m_flInactiveKickWarning, FIELD_TIME ),
END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CASW_Player, CBasePlayer, "The player entity." )
	DEFINE_SCRIPTFUNC( ResurrectMarine, "Resurrect the marine" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetNPC, "GetNPC", "Returns entity the player is inhabiting" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetSpectatingNPC, "GetSpectatingNPC", "Returns entity the player is spectating" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetViewNPC, "GetViewNPC", "Returns entity the player is spectating, else will return inhabiting entity" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptGetMarine, "GetMarine", "Returns the marine the player is commanding" )
	DEFINE_SCRIPTFUNC_NAMED( ScriptFindPickerEntity, "FindPickerEntity", "Finds the nearest entity in front of the player" )
	DEFINE_SCRIPTFUNC( GetCrosshairTracePos, "Returns the world location directly beneath the player's crosshair" )
END_SCRIPTDESC()

// -------------------------------------------------------------------------------- //

void ASW_GetNumAIAwake(const char* szClass, int &iAwake, int &iAsleep, int &iNormal, int &iEfficient, int &iVEfficient, int &iSEfficient, int &iDormant)
{
	CBaseEntity* pEntity = NULL;
	iAwake = 0;
	iAsleep = 0;
	iNormal = iEfficient = iVEfficient = iSEfficient = iDormant = 0;
	while ((pEntity = gEntList.FindEntityByClassname( pEntity, szClass )) != NULL)
	{
		CAI_BaseNPC* pAI = pEntity->MyNPCPointer();			
		if (pAI)
		{
			if (pAI->GetEfficiency() == AIE_NORMAL)
				iNormal++;
			else if (pAI->GetEfficiency() == AIE_EFFICIENT)
				iEfficient++;
			else if (pAI->GetEfficiency() == AIE_VERY_EFFICIENT)
				iVEfficient++;
			else if (pAI->GetEfficiency() == AIE_SUPER_EFFICIENT)
				iSEfficient++;
			else if (pAI->GetEfficiency() == AIE_DORMANT)
				iDormant++;

			if (pAI->GetSleepState() == AISS_AWAKE)
				iAwake++;
			else
				iAsleep++;
		}
		else
		{
			CASW_Simple_Alien *pSimple = dynamic_cast<CASW_Simple_Alien*>(pEntity);
			if (pSimple)
			{
				if (!pSimple->m_bSleeping)
					iAwake++;
				else
					iAsleep++;
			}
		}
	}
}

void ASW_DrawAwakeAI()
{
	int iAwake = 0;
	int iAsleep = 0;
	int iDormant = 0;
	int iEfficient = 0;
	int iVEfficient = 0;
	int iSEfficient = 0;
	int iNormal = 0;
	int nprintIndex = 18;
	engine->Con_NPrintf( nprintIndex, "AI (awake/asleep) (normal/efficient/very efficient/super efficient/dormant)");
	nprintIndex++;
	engine->Con_NPrintf( nprintIndex, "================================");
	nprintIndex++;
	for ( int i = 0; i < ASWSpawnManager()->GetNumAlienClasses(); i++ )
	{
		ASW_GetNumAIAwake( ASWSpawnManager()->GetAlienClass( i )->m_pszAlienClass, iAwake, iAsleep, iNormal, iEfficient, iVEfficient, iSEfficient, iDormant);
		engine->Con_NPrintf( nprintIndex, "%s: (%d / %d) (%d / %d / %d / %d / %d)\n", ASWSpawnManager()->GetAlienClass( i )->m_pszAlienClass, iAwake, iAsleep, iNormal, iEfficient, iVEfficient, iSEfficient, iDormant );
		nprintIndex++;
	}
}
ConVar asw_draw_awake_ai( "asw_draw_awake_ai", "0", FCVAR_CHEAT, "Lists how many of each AI are awake");

CBaseEntity *CASW_Player::spawn_point = NULL;

CASW_Player::CASW_Player()
{
	m_PlayerAnimState = CreatePlayerAnimState(this, this, LEGANIM_9WAY, false);
	UseClientSideAnimation();
	m_angEyeAngles.Init();

	SetViewOffset( ASW_PLAYER_VIEW_OFFSET );

	m_hInhabiting = NULL;
	m_hSpectating = NULL;
	m_pCurrentInfoMessage = NULL;
	m_vecLastMarineOrigin = vec3_origin;

	m_bLastAttackButton= false;
	m_bLastAttack2Button= false;
	m_fLastAICountTime = 0;
	m_bAutoReload = true;
	m_hUseKeyDownEnt = NULL;
	m_flUseKeyDownTime = 0.0f;
	m_flMovementAxisYaw = 90.0f;
	m_fMarineDeathTime = 0.0f;

	m_Local.m_vecPunchAngle.Set( ROLL, 15 );
	m_Local.m_vecPunchAngle.Set( PITCH, 8 );
	m_Local.m_vecPunchAngle.Set( YAW, 0 );
	m_angMarineAutoAim = vec3_angle;
	m_angMarineAutoAimFromClient = vec3_angle;

	m_bPendingSteamStats = false;
	m_flPendingSteamStatsStart = 0.0f;

	m_bWelcomed = false;

	if ( ASWGameRules() )
	{
		ASWGameRules()->ClearLeaderKickVotes( this );

		if ( ASWGameRules()->GetGameState() == ASW_GS_INGAME )
		{
			SpectateNextMarine();
		}
	}
	m_fLastFragTime = FLT_MIN;
	m_iKillingSpree = 0;

	// BenLubar(spectator-mouse)
	m_iScreenWidth = 0;
	m_iScreenHeight = 0;
	m_iMouseX = 0;
	m_iMouseY = 0;
	// 

	m_bLeaderboardReady = false;
	m_bWantsSpectatorOnly = false;

	m_flLastActiveTime = 0.0f;
	m_flInactiveKickWarning = 0.0f;

	for ( int i = 0; i < RD_NUM_STEAM_INVENTORY_EQUIP_SLOTS; i++ )
	{
		m_EquippedItemData[i].Reset();
	}
	m_EquippedItemsResult = k_SteamInventoryResultInvalid;
	m_EquippedItemsReceiving.Purge();
	m_iEquippedItemsReceivingOffset = 0;
	m_iEquippedItemsParity = 0;
}


CASW_Player::~CASW_Player()
{
	m_PlayerAnimState->Release();
	if (ASWGameRules())
		ASWGameRules()->SetMaxMarines(this);

	// free inventory handle
	ReactiveDropInventory::DecodeItemData( m_EquippedItemsResult, "" );
}

//------------------------------------------------------------------------------
// Purpose : Send even though we don't have a model
//------------------------------------------------------------------------------
int CASW_Player::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_ALWAYS );
}


CASW_Player *CASW_Player::CreatePlayer( const char *className, edict_t *ed )
{
	CASW_Player::s_PlayerEdict = ed;
	return (CASW_Player*)CreateEntityByName( className );
}

void CASW_Player::PreThink( void )
{
	BaseClass::PreThink();
	HandleSpeedChanges();
}


void CASW_Player::PostThink()
{
	BaseClass::PostThink();

	QAngle angles = GetLocalAngles();
	if ( GetNPC() )
		angles[PITCH] = 0;
	SetLocalAngles( angles );
	
	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

    m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );

	// find nearby usable items
	FindUseEntities();

	if ( CanBeKicked() && rd_kick_inactive_players.GetFloat() > 0 )
	{
		float flInactiveFor = gpGlobals->curtime - m_flLastActiveTime;
		float flInactiveRatio = flInactiveFor / rd_kick_inactive_players.GetFloat();
		m_flInactiveKickWarning = flInactiveRatio >= rd_kick_inactive_players_warning.GetFloat() ? m_flLastActiveTime + rd_kick_inactive_players.GetFloat() : 0.0f;

		if ( flInactiveRatio >= 1.0f )
		{
			engine->ServerCommand( CFmtStr( "kickid %d Disconnected due to inactivity\n", GetUserID() ) );
		}
	}

	// clicking while ingame on mission with no marine makes us spectate the next marine
	if ((!GetNPC() || GetNPC()->GetHealth()<=0)
		/*&& !HasLiveMarines()*/ && ASWGameRules() && ASWGameRules()->GetGameState() == ASW_GS_INGAME)
	{
		//Msg("m_nButtons & IN_ATTACK = %d (m_Local.m_nOldButtons & IN_ATTACK) = %d\n", (m_nButtons & IN_ATTACK), (m_Local.m_nOldButtons & IN_ATTACK));
		bool bClicked = (!m_bLastAttackButton && (m_nButtons & IN_ATTACK));
		bool bRightClicked = (!m_bLastAttack2Button && (m_nButtons & IN_ALT1));
		bool bJumpPressed = (!m_bLastAttackButton && (m_nButtons & IN_JUMP));

		// for deathmatch we spectate after 1.5 sec and respawn marine on click 
		// after 5 seconds
		if ( ASWDeathmatchMode() )
		{
			if ( bJumpPressed && gpGlobals->curtime > m_fMarineDeathTime + rd_respawn_time.GetFloat() )
			{
				ASWDeathmatchMode()->SpawnMarine( this );
			}
			else if ( !GetSpectatingNPC() && gpGlobals->curtime > m_fLastControlledMarineTime + 6.0f )
			{
				SpectateNextMarine();
				m_fLastControlledMarineTime = gpGlobals->curtime - 5.0f; // set this again so we don't spam SpectateNextMarine
			}
			else if ( ( bClicked || bRightClicked ) && gpGlobals->curtime > m_fMarineDeathTime + 1.5f )
			{
				SpectateNextMarine();
			}
		}
		else if ( bClicked && gpGlobals->curtime > m_fLastControlledMarineTime + 6.0f )
		{
			// riflemod: allow drop in 
			CASW_Game_Resource *pGameResource = ASWGameResource();
			if ( !this->HasLiveMarines() && pGameResource )
			{
				bool found_available_marine = false;
				CASW_Marine *pBotMarine = NULL;

				if ( CASW_Marine::AsMarine( GetSpectatingNPC() ) && !GetSpectatingNPC()->IsInhabited() )
				{
					found_available_marine = true;
					pBotMarine = CASW_Marine::AsMarine( GetSpectatingNPC() );
				}
				else 
				{
					for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
					{
						CASW_Marine_Resource* pMR = pGameResource->GetMarineResource( i );
						if ( !pMR )
							continue;
						CASW_Marine *pMarine = pMR->GetMarineEntity();

						if ( pMarine && !pMarine->IsInhabited() )
						{
							found_available_marine = true;
							pBotMarine = pMarine;
							break;
						}
					}
				}
				if (found_available_marine)
				{
					DevMsg(" Riflemod Drop-In. Switching player to marine 0\n");
					pBotMarine->SetCommander( this );
					pBotMarine->GetMarineResource()->SetCommander( this );
					SetSpectatingNPC( NULL );
					SwitchMarine( 0, false );
					// reactivedrop: when player took marine under control
					// delay his primary attack to prevent immediate shooting at
					// unknown posisition which can kill teammates
					pBotMarine->SetNextAttack( gpGlobals->curtime + 1.0f );

					// reactivedrop: additionally check bots. If they have no 
					// commander then assign to this player
					//
					// This happens when the only one player on server plays with bots
					// and goes asw_afk. When this player takes control over a bot
					// all other bots are now assigned to this player. Previously
					// they were assigned to nobody
					for (int i = 0; i < pGameResource->GetMaxMarineResources(); ++i)
					{
						CASW_Marine_Resource* pMR = pGameResource->GetMarineResource( i );
						if (!pMR || pMR->GetHealthPercent() <= 0)
							continue;
						if (pMR->GetCommander() == NULL)
							pMR->SetCommander( this );
					}
				}
				else 
				{
					SpectateNextMarine();
				}
			}
		}
		else if ( bRightClicked || ( !GetSpectatingNPC() && gpGlobals->curtime > m_fLastControlledMarineTime + 6.0f ) )
		{
			// riflemod: right click when spectating cycles through marines 
			SpectateNextMarine();
		}
	}
	else
	{
		m_fLastControlledMarineTime = gpGlobals->curtime;
	}

	m_bLastAttackButton = (m_nButtons & IN_ATTACK) != 0;
	m_bLastAttack2Button= (m_nButtons & IN_ALT1) != 0;

	RagdollBlendTest();

	if (asw_draw_awake_ai.GetBool() && gpGlobals->curtime - m_fLastAICountTime > 2.0f)
	{
		ASW_DrawAwakeAI();
		m_fLastAICountTime = gpGlobals->curtime;
	}
}

CBaseEntity* CASW_Player::GetUseEntity(int i) 
{
	return m_hUseEntities[i];
}

void CASW_Player::Precache()
{
	PrecacheModel( ASW_PLAYER_MODEL );
	PrecacheModel( "models/swarm/OrderArrow/OrderArrow.mdl" );
	// ASWTODO - precache the actual model set in asw_client_corpse.cpp rather than assuming this one
	PrecacheModel( "models/swarm/colonist/male/malecolonist.mdl" );

	PrecacheScriptSound( "noslow.BulletTimeIn" );
	PrecacheScriptSound( "noslow.BulletTimeOut" );
	PrecacheScriptSound( "noslow.SingleBreath" );
	PrecacheScriptSound( "Game.ObjectiveComplete" );
	PrecacheScriptSound( "Game.MissionWon" );
	PrecacheScriptSound( "Game.MissionLost" );
	PrecacheScriptSound( "asw_song.stims" );
	//PrecacheScriptSound( "Holdout.GetReadySlide" );
	//PrecacheScriptSound( "Holdout.GetReadySlam" );
	PrecacheScriptSound( "asw_song.statsSuccess" );
	PrecacheScriptSound( "asw_song.statsFail" );

	if (MarineProfileList())
	{
		MarineProfileList()->PrecacheSpeech(this);
	}
	else
	{
		Msg("Couldn't precache marine speech as profile list isn't created yet\n");
	}

	BaseClass::Precache();
}

void CASW_Player::Spawn()
{
	SetModel( ASW_PLAYER_MODEL );

	BaseClass::Spawn();

	m_vecLastMarineOrigin = vec3_origin;
	m_flMovementAxisYaw = 90.0f;

	SetMoveType( MOVETYPE_WALK );
	m_takedamage = DAMAGE_NO;
	m_iKickVoteIndex = -1;
	m_iLeaderVoteIndex = -1;
	BecomeNonSolid();

	m_bFirstInhabit = false;
	m_bRequestedSpectator = false;
	m_bPrintedWantStartMessage = false;
	m_bPrintedWantsContinueMessage = false;
	m_nChangingMR = -1;
	m_nChangingSlot = 0;
	m_bHasAwardedXP = false;
	m_bSentPromotedMessage = false;

	m_flLastActiveTime = gpGlobals->curtime;

	if (ASWGameRules())
	{
		ASWGameRules()->SetMaxMarines();
	}
}

void CASW_Player::DoAnimationEvent( PlayerAnimEvent_t event )
{
	TE_PlayerAnimEvent( this, event );	// Send to any clients who can see this guy.
}

CBaseCombatWeapon* CASW_Player::ASWAnim_GetActiveWeapon()
{
	return GetActiveWeapon();
}

void CASW_Player::EmitPrivateSound( const char *soundName, bool bFromNPC )
{
	CSoundParameters params;
	if ( !GetParametersForSound( soundName, params, NULL ) )
		return;

	CSingleUserRecipientFilter filter( this );
	if ( bFromNPC && GetNPC() )
	{
		EmitSound( filter, GetNPC()->entindex(), soundName );
	}
	else
	{
		EmitSound( filter, entindex(), soundName );
	}
}

bool CASW_Player::ASWAnim_CanMove()
{
	return true;
}

void CASW_Player::WelcomeMessageThink()
{
	m_bWelcomed = true;

	char buffer[512];
	Q_snprintf(buffer, sizeof(buffer), rm_welcome_message.GetString());
	ClientPrint(this, HUD_PRINTTALK, buffer);
	SetContextThink(NULL, gpGlobals->curtime, s_pWelcomeMessageContext);
}

bool CASW_Player::ClientCommand( const CCommand &args )
{
	const char *pcmd = args[0];

	m_flLastActiveTime = gpGlobals->curtime;

	switch ( ASWGameRules()->GetGameState() )
	{
		case ASW_GS_BRIEFING:
		{
			if ( FStrEq( pcmd, "cl_selectm" ) )			// selecting a marine from the roster
			{
				CASW_Game_Resource *pGameResource = ASWGameResource();
				if (!pGameResource)
					return false;

				if ( args.ArgC() < 3 )
				{
					Warning( "Player sent bad cl_selectm command\n" );
					return false;
				}

				int iRosterIndex = clamp(atoi( args[1] ), 0, ASW_NUM_MARINE_PROFILES-1);
				int nPreferredSlot = atoi( args[2] );
				if (ASWGameRules()->RosterSelect( this, iRosterIndex, nPreferredSlot ) )
				{
					// did they specify a previous inventory selection too?
					if ( args.ArgC() == 6 )
					{
						int iMarineResource = -1;
						for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
						{
							CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
							if (pMR && pMR->GetProfileIndex() == iRosterIndex)
							{
								iMarineResource = i;
								break;
							}
						}
						if (iMarineResource == -1)
							return true;

						m_bRequestedSpectator = false;
						int primary = atoi( args[3] );
						int secondary = atoi( args[4] );
						int extra = atoi( args[5] );

						if (primary != -1)
							ASWGameRules()->LoadoutSelect(this, iRosterIndex, 0, primary);
						if (secondary != -1)
							ASWGameRules()->LoadoutSelect(this, iRosterIndex, 1, secondary);
						if (extra != -1)
							ASWGameRules()->LoadoutSelect(this, iRosterIndex, 2, extra);
					}
				}

				return true;
			}
			else if ( FStrEq( pcmd, "cl_dselectm" ) )			// selecting a marine from the roster
			{
				if ( args.ArgC() < 2 )
				{
					Warning( "Player sent bad cl_dselectm command\n" );
					return false;
				}

				if (!ASWGameRules())
					return false;

				int iRosterIndex = clamp(atoi( args[1] ), 0, ASW_NUM_MARINE_PROFILES-1);
				ASWGameRules()->RosterDeselect(this, iRosterIndex);

				return true;
			}
			else if ( FStrEq( pcmd, "cl_revive" ) )			// revive all dead marines, leader only
			{
				if (ASWGameResource() && ASWGameResource()->m_iLeaderIndex == entindex() && ASWGameRules())
				{
					ASWGameRules()->ReviveDeadMarines();
				}

				return true;
			}
			else if ( FStrEq( pcmd, "cl_wants_start" ) )			// print a message telling the other players that you want to start
			{
				if (ASWGameResource() && ASWGameResource()->m_iLeaderIndex == entindex() && ASWGameRules())
				{
					if (ASWGameRules()->GetGameState() == ASW_GS_BRIEFING)
					{
						if (!m_bPrintedWantStartMessage)
						{
							UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_wants_start", GetPlayerName() );
							m_bPrintedWantStartMessage = true;
						}
					}
				}
				return true;
			}
			else if ( FStrEq( pcmd, "cl_loadouta" ) )			// selecting equipment
			{
				if ( args.ArgC() < 5 )
				{
					Warning( "Player sent bad loadouta command\n" );
					return false;
				}

				int iProfileIndex = clamp(atoi( args[1] ), 0, ASW_NUM_MARINE_PROFILES-1);
				int iPrimary = atoi( args[2] );
				int iSecondary = atoi( args[3] );
				int iExtra = atoi( args[4] );

				if (iPrimary >=0)
					ASWGameRules()->LoadoutSelect(this, iProfileIndex, 0, iPrimary);
				if (iSecondary >=0)
					ASWGameRules()->LoadoutSelect(this, iProfileIndex, 1, iSecondary);
				if (iExtra >=0)
					ASWGameRules()->LoadoutSelect(this, iProfileIndex, 2, iExtra);

				return true;

			}
			else if ( FStrEq( pcmd, "cl_start" ) )			// done selecting, go ingame
			{
				// check if all players are ready, etc
				BecomeNonSolid();
				// send a message to client telling him to close the briefing
				if (ASWGameRules())
				{			
					ASWGameRules()->RequestStartMission(this);
				}		

				return true;
			}
			else if ( FStrEq( pcmd, "cl_spendskill") )
			{	
				if ( args.ArgC() < 3 )
				{
					Warning("Player sent a bad cl_spendskill command\n");
					return false;
				}

				int iProfileIndex = atoi(args[1]);
				int nSkillSlot = atoi(args[2]);

				if (iProfileIndex < 0 || iProfileIndex >= ASW_NUM_MARINE_PROFILES )
					return false;

				if (nSkillSlot < 0 || nSkillSlot >= ASW_SKILL_SLOT_SPARE )
					return false;

				if (ASWGameRules() && ASWGameRules()->CanSpendPoint(this, iProfileIndex, nSkillSlot))
					ASWGameRules()->SpendSkill(iProfileIndex, nSkillSlot);
				return true;
			}
			else if ( FStrEq( pcmd, "cl_undoskill" ) )
			{
				if ( args.ArgC() < 2 )
				{
					Warning("Player sent a bad cl_undoskill command\n");
					return false;
				}
				int iProfileIndex = atoi(args[1]);
				if (iProfileIndex < 0 || iProfileIndex >= ASW_NUM_MARINE_PROFILES )
					return false;
				if (ASWGameRules())
					ASWGameRules()->SkillsUndo(this, iProfileIndex);
				return true;
			}
			else if ( FStrEq( pcmd, "cl_hardcore_ff") )
			{
				if ( args.ArgC() < 2 )
				{
					Warning("Player sent a bad cl_hardcore_ff command\n");
					return false;
				}

				if ( ASWGameResource() && ASWGameResource()->GetLeader() == this )
				{
					bool bOldHardcoreMode = CAlienSwarm::IsHardcoreFF();
					int nHardcore = atoi( args[1] );
					nHardcore = clamp<int>( nHardcore, 0, 1 );

					extern ConVar asw_sentry_friendly_fire_scale;
					extern ConVar asw_marine_ff_absorption;
					asw_sentry_friendly_fire_scale.SetValue( nHardcore );
					asw_marine_ff_absorption.SetValue( 1 - nHardcore );

					if ( CAlienSwarm::IsHardcoreFF() != bOldHardcoreMode )
					{
						CReliableBroadcastRecipientFilter filter;
						filter.RemoveRecipient( this );		// notify everyone except the player changing the setting
						if ( nHardcore > 0 )
						{
							UTIL_ClientPrintFilter( filter, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_enabled_hardcoreff", GetPlayerName() );
						}
						else
						{
							UTIL_ClientPrintFilter( filter, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_disabled_hardcoreff", GetPlayerName() );
						}
					}
				}
				return true;
			}
			else if (FStrEq(pcmd, "rd_set_challenge"))
			{
				if (args.ArgC() < 2)
				{
					Warning("Player sent a bad rd_set_challenge command\n");
					return false;
				}

				if (ASWGameResource() && ASWGameResource()->GetLeader() == this)
				{
					const char *szChallengeName = args[1];

					if (ASWGameRules())
					{
						ASWGameRules()->EnableChallenge(szChallengeName);
					}
				}
				return true;
			}
			else if ( FStrEq( pcmd, "cl_fixedskills") )
			{
				/*
				if ( args.ArgC() < 2 )
				{
					Warning("Player sent a bad cl_fixedskills command\n");
					return false;
				}
				if ( ASWGameRules() && ASWGameRules()->IsCampaignGame() && ASWGameRules()->GetGameState() == ASW_GS_BRIEFING
					&& ASWGameResource()->m_Leader.Get() == this && ASWGameResource() && ASWGameRules()->GetCampaignSave() )
				{
					ASWGameRules()->GetCampaignSave()->m_bFixedSkillPoints = ( atoi( args[1] ) == 1 );
					ASWGameResource()->UpdateMarineSkills( ASWGameRules()->GetCampaignSave() );
					ASWGameRules()->GetCampaignSave()->SaveGameToFile();
				}
				*/
				return true;
			}
			else if ( FStrEq( pcmd, "cl_needtech") )
			{
				if (ASWGameResource()->GetLeader() != this)
					return true;
				if (ASWGameRules() && ASWGameRules()->GetGameState() == ASW_GS_BRIEFING)
					UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_need_tech" );
				return true;
			}
			else if ( FStrEq( pcmd, "cl_needequip") )
			{
				if (ASWGameResource()->GetLeader() != this)
					return true;
				if (ASWGameRules() && ASWGameRules()->GetGameState() == ASW_GS_BRIEFING)
					ASWGameRules()->ReportMissingEquipment();
				return true;
			}
			else if ( FStrEq( pcmd, "cl_needtwoplayers") )
			{
				if (ASWGameRules() && ASWGameRules()->GetGameState() == ASW_GS_BRIEFING)
					ASWGameRules()->ReportNeedTwoPlayers();
				return true;
			}
			else if ( FStrEq( pcmd, "cl_editing_slot") )
			{
				if ( args.ArgC() < 3 )
				{
					Warning("Player sent a bad cl_editing_slot command\n");
					return false;
				}
				m_nChangingMR = atoi( args[1] );
				m_nChangingSlot = atoi( args[2] );

				return true;
			}
			else if ( FStrEq( pcmd, "cl_promoted" ) )
			{
				if ( args.ArgC() < 2 )
				{
					Warning("Player sent a bad cl_promoted command\n");
					return false;
				}
				if ( !m_bSentPromotedMessage )
				{
					m_bSentPromotedMessage = true;
					int nPromote = atoi( args[1] );
					switch ( nPromote )
					{
						case 1: UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_promoted_1", GetPlayerName() ); break;
						case 2: UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_promoted_2", GetPlayerName() ); break;
						case 3: UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_promoted_3", GetPlayerName() ); break;
						case 4: UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_promoted_4", GetPlayerName() ); break;
						case 5: UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_promoted_5", GetPlayerName() ); break;
						case 6: UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_promoted_6", GetPlayerName() ); break;
						case 7: UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_promoted_7", GetPlayerName() ); break;
					}

					CReliableBroadcastRecipientFilter filter;
					UserMessageBegin( filter, "RDPlayerPromoted" );
						WRITE_BYTE( GetClientIndex() );
					MessageEnd();
				}
				return true;
			}
			else if ( FStrEq( pcmd, "cl_lobby_invalid_request" ) )			// check if server is dedicated, if yes, then disconnect this client
			{
				ConVarRef sv_allow_lobby_connect_only_ref("sv_allow_lobby_connect_only");
				ConVarRef sv_lan_ref( "sv_lan" );
				if ( sv_allow_lobby_connect_only_ref.IsValid() && sv_lan_ref.IsValid() )
				{
					if ( sv_allow_lobby_connect_only_ref.GetBool() && !sv_lan_ref.GetBool() && engine->IsDedicatedServer() )
					{
						ClientPrint( this, HUD_PRINTTALK, "Disconnecting, lost connection to Steam" );
						engine->ClientCommand( edict(), "disconnect" );
					}
				}
				return true;	
			}

			break;
		}

		case ASW_GS_INGAME:
		{
			if ( FStrEq( pcmd, "cl_marineface" ) )			// ordering a marine to a certain spot + yaw
			{
				if ( args.ArgC() < 6 )
				{
					Warning( "Player sent bad marine face command\n" );
					return false;
				}

				int iMarineEntIndex = atoi( args[1] );		
				float fYaw = atof( args[2] );
				float x = atof(args[3]);
				float y = atof(args[4]);
				float z = atof(args[5]);
				//Msg("cl_marineface %f, %f, %f\n", x, y, z);
				Vector vecOrderPos(x, y, z);

				OrderMarineFace(iMarineEntIndex, fYaw, vecOrderPos);

				return true;

			}
			else if ( FStrEq( pcmd, "cl_orderfollow" ) )			// ordering a marine to a follow your current marine
			{
				if ( args.ArgC() < 1 )
				{
					Warning( "Player sent bad cl_orderfollow command\n" );
				}

				OrderNearbyMarines(this, ASW_ORDER_FOLLOW);

				return true;
			}
			else if ( FStrEq( pcmd, "cl_orderhold" ) )
			{
				if ( args.ArgC() < 1 )
				{
					Warning( "Player sent bad cl_orderhold command\n" );
				}

				OrderNearbyMarines(this, ASW_ORDER_HOLD_POSITION);

				return true;
			}
			else if ( FStrEq( pcmd, "cl_mread" ) )			// telling the server you've read an info message
			{
				if ( args.ArgC() < 2 )
				{
					Warning( "Player sent bad cl_mread command\n" );
					return false;
				}

				int iMessageEntIndex = atoi( args[1] );
				CASW_Info_Message *pMessage = dynamic_cast<CASW_Info_Message*>(CBaseEntity::Instance(iMessageEntIndex));
				if (pMessage)
				{
					pMessage->OnMessageRead(this);
				}

				return true;

			}
			else if ( FStrEq( pcmd, "cl_emote" ) )			// activating special speech+icon emote
			{
				if ( args.ArgC() < 2 )
				{
					Warning("Player sent a bad cl_emote command\n");
					return false;
				}
				int iEmote = atoi( args[1] );
				CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC() );
				if ( pMarine && pMarine->GetHealth() > 0 )
				{
					pMarine->DoEmote( iEmote );
				}
				return true;
			}
			else if ( FStrEq( pcmd, "cl_chatter" ) )			// activate a specific chatter
			{
				if ( args.ArgC() < 2 )
				{
					Warning("Player sent a bad cl_chatter command\n");
					return false;
				}
				int iChatter = atoi( args[1] );
				int iSubChatter = -1;
				if (args.ArgC() == 3)
				{
					iSubChatter = atoi( args[2] );
				}
				CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC() );
				if (pMarine && pMarine->GetMarineSpeech())
				{
					pMarine->GetMarineSpeech()->ClientRequestChatter(iChatter, iSubChatter, asw_client_chatter_enabled.GetBool() ? NULL : this);
				}
				return true;
			}
			else if ( FStrEq( pcmd, "cl_freecam") )		// player is telling server where his freecam is
			{
				if ( args.ArgC() < 4 )
				{
					Warning("Player sent a bad cl_freecam command\n");
					return false;
				}

				m_bUsedFreeCam = true;
				m_vecFreeCamOrigin.x = atoi( args[1] );
				m_vecFreeCamOrigin.y = atoi( args[2] );
				m_vecFreeCamOrigin.z = atoi( args[3] );

				return true;
			}
			else if ( FStrEq( pcmd, "cl_selecthack") )
			{
				if ( args.ArgC() < 2 )
				{
					Warning("Player sent a bad cl_selecthack command\n");
					return false;
				}

				int iHackOption = atoi(args[1]);
				CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC());
				if ( pMarine && pMarine->m_hCurrentHack.Get() )
				{
					CASW_Hack *pHack = pMarine->m_hCurrentHack.Get();
					if ( pHack )
					{
						pHack->SelectHackOption( iHackOption );
						return true;
					}
				}
				return false;
			}
			else if ( FStrEq( pcmd, "cl_stopusing") )
			{
				if ( GetNPC() && GetNPC()->m_hUsingEntity.Get() )
					GetNPC()->StopUsing();
				return true;
			}
			else if ( FStrEq( pcmd, "cl_blipspeech") )
			{
				if ( args.ArgC() < 2 )
				{
					Warning("Player sent a bad cl_blipspeech command\n");
					return false;
				}
				int iTargetMarine = atoi( args[1] );
				if (ASWGameRules())
					ASWGameRules()->BlipSpeech(iTargetMarine);
				return true;
			}
			else if ( FStrEq( pcmd, "cl_viewmail") )
			{
				if ( args.ArgC() < 2 )
				{
					Warning("Player sent a bad cl_viewmail command\n");
					return false;
				}
				CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC() );
				if (pMarine)
				{
					CBaseEntity* pUsing = pMarine->m_hUsingEntity.Get();
					if ( pUsing && pUsing->Classify() == CLASS_ASW_COMPUTER_AREA )
					{
						CASW_Computer_Area* pComputer = assert_cast<CASW_Computer_Area*>(pUsing);
						int iMail = clamp(atoi(args[1]), 0, 3);
						pComputer->OnViewMail(pMarine, iMail);
					}
				}
				return true;
			}
			else if ( FStrEq( pcmd, "cl_offhand") )
			{			
				if ( args.ArgC() < 2 )
				{
					Warning("Player sent a bad cl_offhand command\n");
					return false;
				}
				int slot = clamp(atoi(args[1]), 0, 3);
				CASW_Marine* pMarine = CASW_Marine::AsMarine( GetNPC() );
				if (pMarine && pMarine->GetHealth()>0 && !(pMarine->GetFlags() & FL_FROZEN))
				{
					// check we have an item in that slot
					CASW_Weapon* pWeapon = pMarine->GetASWWeapon(slot);
					if (pWeapon)
					{
						const CASW_WeaponInfo* pWpnInfo = pWeapon->GetWeaponInfo();
						if (pWpnInfo && pWpnInfo->m_bOffhandActivate)
						{
							pWeapon->OffhandActivate();

							// Fire event when a player uses an offhand item
							IGameEvent* event = gameeventmanager->CreateEvent( "weapon_offhand_activate" );
							if (event)
							{
								event->SetInt( "userid", GetUserID() );
								event->SetInt( "marine", pMarine->entindex() );
								event->SetInt( "weapon", pWeapon->entindex() );

								gameeventmanager->FireEvent(event);
							}
						}
					}
				}
				return true;
			}
			else if ( FStrEq( pcmd, "cl_ai_offhand") )
			{			
				if ( args.ArgC() < 3 )
				{
					Warning("Player sent a bad cl_ai_offhand command\n");
					return false;
				}
				int slot = clamp( atoi( args[1] ), 0, 2 );
				int marine_index = atoi( args[2] );
				CBaseEntity* pEnt = CBaseEntity::Instance(marine_index);

				if (pEnt && pEnt->Classify() == CLASS_ASW_MARINE )
				{
					CASW_Marine* pMarine = assert_cast<CASW_Marine*>(pEnt);
					if (pMarine->GetCommander() == this)
					{
						pMarine->OrderUseOffhandItem(slot, GetCrosshairTracePos());
					}
				}
				return true;
			}
			else if ( FStrEq( pcmd, "rd_set_challenge" ) && ASWDeathmatchMode() )
			{
				if ( args.ArgC() < 2 )
				{
					Warning( "Player sent a bad rd_set_challenge command\n" );
					return false;
				}

				if ( ASWGameResource() && ASWGameResource()->GetLeader() == this )
				{
					const char *szChallengeName = args[1];

					if ( ASWGameRules() )
					{
						ASWGameRules()->EnableChallenge( szChallengeName );
						ASWGameRules()->ApplyChallenge();
					}
				}
				return true;
			}

			break;
		}

		case ASW_GS_DEBRIEF:
		{
			if ( FStrEq( pcmd, "cl_wants_continue" ) )			// print a message telling the other players that you want to start
			{
				if (ASWGameResource() && ASWGameResource()->m_iLeaderIndex == entindex() && ASWGameRules())
				{
					if (ASWGameRules()->GetGameState() == ASW_GS_DEBRIEF)
					{
						if (!m_bPrintedWantsContinueMessage)
						{
							UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_wants_continue", GetPlayerName() );
							m_bPrintedWantsContinueMessage = true;
						}
					}
				}
				return true;
			}
			else if ( FStrEq( pcmd, "cl_wants_returnmap" ) )			// print a message telling the other players that you want to start
			{
				if (ASWGameResource() && ASWGameResource()->m_iLeaderIndex == entindex() && ASWGameRules())
				{
					if (ASWGameRules()->GetGameState() == ASW_GS_DEBRIEF)
					{
						if (!m_bPrintedWantsContinueMessage)
						{
							UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_wants_returnmap", GetPlayerName() );
							m_bPrintedWantsContinueMessage = true;
						}
					}
				}
				return true;
			}
			else if ( FStrEq( pcmd, "cl_leaderboard_ready" ) )
			{
				if ( !m_bLeaderboardReady && ASWGameRules() )
				{
					m_bLeaderboardReady = true;
					ASWGameRules()->CheckLeaderboardReady();
				}
				return true;
			}

			break;
		}
	}

	if ( FStrEq( pcmd, "cl_leaderboard_ready" ) )
	{
		// ignore if received at any time outside of debriefing
		return true;
	}

	if ( FStrEq( pcmd, "cl_selectsinglem" ) )			// selecting a marine from the roster, deselecting our current
	{
		// allow to select marine only in BREIFING for coop
		if ( !ASWDeathmatchMode() && ASWGameRules()->GetGameState() != ASW_GS_BRIEFING)
			return false;

		CASW_Game_Resource *pGameResource = ASWGameResource();
		if (!pGameResource)
			return false;

		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad cl_selectsinglem command\n" );
			return false;
		}

		// deselect any current marines
		for ( int i = 0; i < ASWGameResource()->GetMaxMarineResources(); i++ )
		{
			CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
			if ( !pMR )
				break;
			// BenLubar: Don't deselect bots when chaning marine
			if ( pMR->GetCommander() == this && pMR->IsInhabited() )
			{
				ASWGameRules()->RosterDeselect( this, pMR->GetProfileIndex() );
			}
		}

		// now select the new marine
		int iRosterIndex = clamp(atoi( args[1] ), 0, ASW_NUM_MARINE_PROFILES-1);
		if ( ASWGameRules()->RosterSelect( this, iRosterIndex, -2 ) )
		{
			// did they specify a previous inventory selection too?
			if (args.ArgC() == 5)
			{
				int iMarineResource = -1;
				for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
				{
					CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
					if (pMR && pMR->GetProfileIndex() == iRosterIndex)
					{
						iMarineResource = i;
						break;
					}
				}
				if (iMarineResource == -1)
					return true;

				m_bRequestedSpectator = false;
				int primary = atoi( args[2] );
				int secondary = atoi( args[3] );
				int extra = atoi( args[4] );

				if (primary != -1)
					ASWGameRules()->LoadoutSelect(this, iRosterIndex, 0, primary);
				if (secondary != -1)
					ASWGameRules()->LoadoutSelect(this, iRosterIndex, 1, secondary);
				if (extra != -1)
					ASWGameRules()->LoadoutSelect(this, iRosterIndex, 2, extra);
			}
		}

//         if ( ASWDeathmatchMode() )
//         {
//             ASWDeathmatchMode()->SpawnMarine( this );
//         }

		return true;
	}
	else if ( FStrEq( pcmd, "cl_loadout" ) )			// selecting equipment
	{
		if ( ASWDeathmatchMode() || ASWGameRules()->GetGameState() == ASW_GS_BRIEFING )
		{
			if ( args.ArgC() < 4 )
			{
				Warning( "Player sent bad loadout command\n" );
				return false;
			}

			int iProfileIndex = clamp(atoi( args[1] ), 0, ASW_NUM_MARINE_PROFILES-1);
			int iInvSlot = atoi( args[2] );
			int iEquipIndex = atoi( args[3] );

			ASWGameRules()->LoadoutSelect(this, iProfileIndex, iInvSlot, iEquipIndex);
			return true;
		}
		else 
			return false;
	}
	// reactivedrop: moved here from case ASW_GS_BRIEFING
	// to allow onlsaught toggling in PvP
	else if ( FStrEq( pcmd, "cl_onslaught") )
	{
		// works only in deathmatch in game or co-op in briefing
		if ( ( ASWDeathmatchMode() && ASWGameRules()->GetGameState() == ASW_GS_INGAME ) 
			|| ASWGameRules()->GetGameState() == ASW_GS_BRIEFING )
		{
			if ( args.ArgC() < 2 )
			{
				Warning("Player sent a bad cl_onslaught command\n");
				return false;
			}

			if ( ASWGameResource() && ASWGameResource()->GetLeader() == this )
			{
				bool bOldOnslaughtMode = CAlienSwarm::IsOnslaught();
				int nOnslaught = atoi( args[1] );
				nOnslaught = clamp<int>( nOnslaught, 0, 1 );

				extern ConVar asw_horde_override;
				extern ConVar asw_wanderer_override;
				asw_horde_override.SetValue( nOnslaught );
				asw_wanderer_override.SetValue( nOnslaught );

				if ( CAlienSwarm::IsOnslaught() != bOldOnslaughtMode )
				{
					CReliableBroadcastRecipientFilter filter;
					filter.RemoveRecipient( this );		// notify everyone except the player changing the setting
					if ( nOnslaught > 0 )
					{
						UTIL_ClientPrintFilter( filter, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_enabled_onslaught", GetPlayerName() );
					}
					else
					{
						UTIL_ClientPrintFilter( filter, ASW_HUD_PRINTTALKANDCONSOLE, "#asw_disabled_onslaught", GetPlayerName() );
					}
				}
			}
			return true;
		}
		else 
			return false;
	}
	else if ( FStrEq( pcmd, "cl_wants_restart" ) )			// print a message telling the other players that you want to start
	{
		if (ASWGameResource() && ASWGameResource()->m_iLeaderIndex == entindex() && ASWGameRules())
		{
			if (ASWGameRules()->GetGameState() == ASW_GS_DEBRIEF)
			{
				if (!m_bPrintedWantsContinueMessage)
				{
					UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_wants_restart", GetPlayerName() );
					m_bPrintedWantsContinueMessage = true;
				}
			}
		}
		return true;
	}
	else if ( FStrEq( pcmd, "cl_autoreload" ) )			// telling the server you've read an info message
	{
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad cl_autoreload command\n" );
			return false;
		}

		m_bAutoReload = (atoi(args[1]) == 1);
		return true;

	}
	else if ( FStrEq( pcmd, "cl_switchm" ) )			// selecting a marine from the roster
	{
		if ( args.ArgC() < 2 )
		{
			Warning( "Player sent bad switch marine command\n" );
			return false;
		}

		// BenLubar: Don't allow taking over bots in PvP
		if ( ASWDeathmatchMode() )
			return false;

		int iMarineIndex = atoi( args[1] ) - 1;
		SwitchMarine(iMarineIndex);

		return true;

	}
	else if ( FStrEq( pcmd, "cl_mapline") )		// player is drawing a line on the minimap
	{
		if ( args.ArgC() < 4 )
		{
			Warning("Player sent a bad cl_mapline command\n");
			return false;
		}
		int linetype = clamp(atoi( args[1] ), 0, 1);
		int world_x = atoi( args[2] );
		int world_y = atoi( args[3] );

		// todo: throttle messages here
		// send user messages to all other clients
		if (ASWGameRules())
			ASWGameRules()->BroadcastMapLine(this, linetype, world_x, world_y);
		return true;
	}
	else if ( FStrEq( pcmd, "cl_campaignnext") )
	{
		if ( args.ArgC() < 2 )
		{
			Warning("Player sent a bad cl_campaignnext command\n");
			return false;
		}
		// make sure we're leader
		if ( ASWGameResource() && ASWGameResource()->m_iLeaderIndex == entindex() )
		{
			int iTargetMission = atoi( args[1] );
			const RD_Campaign_t *pCampaign = ASWGameRules()->GetCampaignInfo();
			if ( pCampaign && pCampaign->GetMission( iTargetMission ) )
			{
				ASWGameResource()->m_iNextCampaignMission = iTargetMission;
			}
// 			if (ASWGameRules() && ASWGameRules()->GetCampaignSave())
// 			{
// 				ASWGameRules()->GetCampaignSave()->PlayerVote(this, iTargetMission);
// 			}
		}
		return true;
	}
	else if ( FStrEq( pcmd, "cl_skill") )
	{
		if ( args.ArgC() < 2 )
		{
			Warning("Player sent a bad cl_skill command\n");
			return false;
		}
		if ( ASWGameRules() )
		{
			ASWGameRules()->RequestSkill( this, atoi(args[1]) );
		}
		return true;
	}
	else if ( FStrEq( pcmd, "cl_forcelaunch") )
	{
		// make sure we're leader
		if (ASWGameResource() && ASWGameResource()->m_iLeaderIndex == entindex())
		{
			if ( ASWGameRules() && ASWGameRules()->GetCampaignSave() )
			{
				ASWGameRules()->RequestCampaignMove( ASWGameResource()->m_iNextCampaignMission.Get() );
				//ASWGameRules()->GetCampaignSave()->ForceNextMissionLaunch();
			}
		}
		return true;
	}
	else if ( FStrEq( pcmd, "cl_campaignsas") )
	{
		if (ASWGameRules() && ASWGameRules()->GetGameState() >= ASW_GS_DEBRIEF
			&& ASWGameResource() && ASWGameResource()->GetLeader() == this)
		{
			ASWGameRules()->CampaignSaveAndShowCampaignMap(this, false);
		}
		return true;
	}
	else if ( FStrEq( pcmd, "cl_forceready") )
	{
		if ( args.ArgC() < 2 || !ASWGameResource() || ASWGameResource()->GetLeader() != this || !ASWGameRules())
		{
			Warning("Player sent a bad cl_forceready command\n");
			return false;
		}

		int iReadyType = atoi(args[1]);		
		if (iReadyType == ASW_FR_BRIEFING && !ASWGameResource()->AtLeastOneMarine())	// don't allow force ready start in briefing if no marines are selected
			return false;

		ASWGameRules()->SetForceReady(iReadyType);
		return true;
	}
	else if ( FStrEq( pcmd, "cl_restart_mission") )
	{
		// check we're leader and request the restart
		if (ASWGameResource() && ASWGameResource()->GetLeader() == this && ASWGameRules())
			ASWGameRules()->RestartMission(this);
		return true;
	}
	else if ( FStrEq( pcmd, "cl_ready") )
	{	
		if ( args.ArgC() < 1 )
		{
			Warning("Player sent a bad cl_ready command\n");
			return false;
		}

		if (!ASWGameResource() || !ASWGameRules())
			return false;

		// only allow readiness in briefing and debrief
		if (ASWGameRules()->GetGameState() != ASW_GS_BRIEFING && ASWGameRules()->GetGameState() != ASW_GS_DEBRIEF)
			return false;

		int iPlayerIndex = entindex() - 1;

		// player index is out of range
		if (iPlayerIndex < 0 || iPlayerIndex >= ASW_MAX_READY_PLAYERS)
			return false;

		// mark us as ready or not
		bool bReady = ASWGameResource()->m_bPlayerReady[iPlayerIndex];
		// toggle our ready status
		ASWGameResource()->m_bPlayerReady.Set(iPlayerIndex, !bReady);

		
		ASWGameRules()->SetMaxMarines();

		// if we have no marines, that means we want to be a spectator
		if (ASWGameRules()->GetGameState() == ASW_GS_BRIEFING && ASWGameResource()->GetNumMarines(this) <= 0)
			m_bRequestedSpectator = true;
		
		return true;
	}
	else if ( FStrEq( pcmd, "cl_spectating") )
	{	
		if ( args.ArgC() < 1 )
		{
			Warning("Player sent a bad cl_spectating command\n");
			return false;
		}
		Msg("cl_spectating get game resource=%d\n", ASWGameResource());
		Msg(" gamerules = %d\n", ASWGameRules());
		Msg(" this is leader = %d\n", ASWGameResource() && ASWGameResource()->GetLeader() == this);

		// remember that this guy wants to spectate (so we can try not to make him leader, ready him up after leaving leader, etc)
		m_bRequestedSpectator = true;

		if (!ASWGameResource() || !ASWGameRules())
		{
			Msg("  cl_spectating returning false as we don't have game resource or gamerules\n");
			return false;
		}

		int iPlayerIndex = entindex() - 1;
		Msg(" playerindex = %d\n", iPlayerIndex);

		// player index is out of range
		if (iPlayerIndex < 0 || iPlayerIndex >= ASW_MAX_READY_PLAYERS)
		{
			Warning("Spectating player index out of range!");
			Msg("  cl_spectating returning false as it's out of range\n");
			return false;
		}

		Msg(" gamestate = %d (briefing=%d debrief=%d map=%d\n", ASWGameRules()->GetGameState(), ASW_GS_BRIEFING, ASW_GS_DEBRIEF, ASW_GS_CAMPAIGNMAP);

		if (ASWGameResource()->GetLeader() == this)		// leader can't be made ready here (but spectator will be made autoready when he's removed from being leader, later)
		{
			Msg("Spectator is leader, so we're deferring auto ready until he stops.  returning true\n");
			if (ASWGameRules()->GetGameState() == ASW_GS_BRIEFING
				|| ASWGameRules()->GetGameState() == ASW_GS_DEBRIEF)
			{
				ASWGameRules()->SetMaxMarines();
			}
			return true;
		}
		
		// flag us as ready in the briefing/debrief
		if (ASWGameRules()->GetGameState() == ASW_GS_BRIEFING
			|| ASWGameRules()->GetGameState() == ASW_GS_DEBRIEF)
		{
			Msg(" Setting player %d ready\n", iPlayerIndex);
			ASWGameResource()->m_bPlayerReady.Set(iPlayerIndex, true);

			ASWGameRules()->SetMaxMarines();
		}
		else if (ASWGameRules()->GetGameState() == ASW_GS_CAMPAIGNMAP && ASWGameRules()->GetCampaignSave())
		{
			Msg(" telling campaign save that we're spectating\n", iPlayerIndex);
			ASWGameRules()->GetCampaignSave()->PlayerSpectating(this);
		}

		Msg("  cl_spectating returning true\n");		
		return true;
	}
	else if ( FStrEq( pcmd, "cl_leadervote") )
	{
		if ( args.ArgC() < 2 )
		{
			Warning("Player sent a bad cl_leadervote command\n");
			return false;
		}
		int iTargetPlayer = clamp(atoi(args[1]), -1, gpGlobals->maxClients);
		ASWGameRules()->SetLeaderVote(this, iTargetPlayer);
		return true;
	}
	else if ( FStrEq( pcmd, "cl_kickvote") )
	{
		if ( args.ArgC() < 2 )
		{
			Warning("Player sent a bad cl_kickvote command\n");
			return false;
		}
		int iTargetPlayer = clamp(atoi(args[1]), -1, gpGlobals->maxClients);
		ASWGameRules()->SetKickVote(this, iTargetPlayer);
		return true;
	}
	else if ( FStrEq( pcmd, "asw_vote_saved_campaign") )
	{
		return 0;
#if 0
		if ( args.ArgC() < 2 )
		{
			Warning("Player sent a bad asw_vote_saved_campaign command\n");
			return false;
		}		
		
		if (ASWGameRules())
			ASWGameRules()->StartVote(this, ASW_VOTE_SAVED_CAMPAIGN, args[1]);
		return true;
#endif
	}
	else if ( FStrEq( pcmd, "asw_vote_campaign") )
	{
		if ( args.ArgC() < 3 )
		{
			Warning("Player sent a bad asw_vote_campaign command\n");
			return false;
		}	
		int nCampaignIndex = atoi( args[1] );
		if (ASWGameRules())
			ASWGameRules()->StartVote(this, ASW_VOTE_CHANGE_MISSION, args[2], nCampaignIndex );
		return true;
	}
	else if ( FStrEq( pcmd, "asw_vote_mission") )
	{
		if ( args.ArgC() < 2 )
		{
			Warning("Player sent a bad asw_vote_mission command\n");
			return false;
		}		
		if (ASWGameRules())
			ASWGameRules()->StartVote(this, ASW_VOTE_CHANGE_MISSION, args[1]);
		return true;
	}
	else if ( FStrEq( pcmd, "vote_yes") )
	{
		if (ASWGameRules())
			ASWGameRules()->CastVote(this, true);
		return true;
	}
	else if ( FStrEq( pcmd, "vote_no") )
	{
		if (ASWGameRules())
			ASWGameRules()->CastVote(this, false);
		return true;
	}
	else if ( FStrEq( pcmd, "cl_ccounts") )
	{
		if ( args.ArgC() < 4 )
		{
			Warning("Player sent a bad cl_ccounts command\n");
			return false;
		}
		m_iClientMissionsCompleted = atoi(args[1]);
		m_iClientCampaignsCompleted = atoi(args[2]);
		m_iClientKills = atoi(args[3]);
		return true;
	}
	else if ( FStrEq( pcmd, "cl_fullyjoined") )
	{
		OnFullyJoined( true );

		return true;
	}
	else if ( FStrEq( pcmd, "cl_gen_progress") )
	{
		if ( args.ArgC() < 2 )
		{
			Warning("Player sent a bad cl_gen_progress command\n");
			return false;
		}
		m_fMapGenerationProgress = clamp(atof(args[1]), 0.0f, 1.0f);
		return true;
	}
	
	return BaseClass::ClientCommand( args );
}

void CASW_Player::UpdateTonemapController()
{
	CASW_Inhabitable_NPC *pNPC = GetViewNPC();
	if ( pNPC )
	{
		if ( CBaseEntity *pController = pNPC->m_hTonemapController )
		{
			m_hTonemapController = pController;
			return;
		}

		FOR_EACH_VEC_BACK( pNPC->m_hTriggerTonemapList, i )
		{
			CTonemapTrigger *pTrigger = pNPC->m_hTriggerTonemapList[i];
			CBaseEntity *pController = pTrigger ? pTrigger->GetTonemapController() : NULL;
			if ( pController )
			{
				m_hTonemapController = pController;
				return;
			}

			// missing trigger or controller; remove to save loop iterations on future frames
			pNPC->m_hTriggerTonemapList.Remove( i );
		}
	}

	BaseClass::UpdateTonemapController();
}

void CASW_Player::UpdateFXVolume()
{
	CFogController *pFogController = NULL;
	CPostProcessController *pPostProcessController = NULL;
	CColorCorrection *pColorCorrectionEnt = NULL;

	Vector eyePos;
	CASW_Inhabitable_NPC *pViewNPC = GetViewNPC();
	if ( pViewNPC )
	{
		eyePos = pViewNPC->EyePosition();
	}
	else if ( CBaseEntity *pViewEntity = GetViewEntity() )
	{
		eyePos = pViewEntity->GetAbsOrigin();
	}
	else
	{
		eyePos = EyePosition();
	}

	CFogVolume *pFogVolume = CFogVolume::FindFogVolumeForPosition( eyePos );
	if ( pFogVolume )
	{
		pFogController = pFogVolume->GetFogController();
		pPostProcessController = pFogVolume->GetPostProcessController();
		pColorCorrectionEnt = pFogVolume->GetColorCorrectionController();

		if ( !pFogController )
		{
			pFogController = FogSystem()->GetMasterFogController();
		}

		if ( !pPostProcessController )
		{
			pPostProcessController = PostProcessSystem()->GetMasterPostProcessController();
		}

		if ( !pColorCorrectionEnt )
		{
			pColorCorrectionEnt = ColorCorrectionSystem()->GetMasterColorCorrection();
		}
	}
	else if ( TheFogVolumes.Count() > 0 )
	{
		pFogController = FogSystem()->GetMasterFogController();
		pPostProcessController = PostProcessSystem()->GetMasterPostProcessController();
		pColorCorrectionEnt = ColorCorrectionSystem()->GetMasterColorCorrection();
	}

	if ( pViewNPC )
	{
		if ( CFogController *pNPCFogController = pViewNPC->m_hFogController )
		{
			pFogController = pNPCFogController;
		}
		if ( CPostProcessController *pNPCPostProcessController = pViewNPC->m_hPostProcessController )
		{
			pPostProcessController = pNPCPostProcessController;
		}
		if ( CColorCorrection *pNPCColorCorrection = pViewNPC->m_hColorCorrection )
		{
			pColorCorrectionEnt = pNPCColorCorrection;
		}
	}

	if ( pFogController && m_PlayerFog.m_hCtrl.Get() != pFogController )
	{
		m_PlayerFog.m_hCtrl.Set( pFogController );
	}

	if ( pPostProcessController )
	{
		m_hPostProcessCtrl.Set( pPostProcessController );
	}

	if ( pColorCorrectionEnt )
	{
		m_hColorCorrectionCtrl.Set( pColorCorrectionEnt );
	}
}

void CASW_Player::BecomeNonSolid()
{
	m_afPhysicsFlags |= PFLAG_OBSERVER;

	SetGroundEntity( (CBaseEntity *)NULL );

    AddSolidFlags( FSOLID_NOT_SOLID );
	RemoveFlag( FL_AIMTARGET ); // don't attract autoaim
	AddFlag( FL_DONTTOUCH );	// stop it touching anything
	AddFlag( FL_NOTARGET );	// stop NPCs noticing it

	SetMoveType( MOVETYPE_NOCLIP );

	return;
}

void CASW_Player::OnNPCCommanded( CASW_Inhabitable_NPC *pNPC )
{
	if ( !ASWGameResource() )
	{
		return;
	}

	if ( pNPC->Classify() == CLASS_ASW_MARINE )
	{
		int nNumMarines = 0;
		int nNewMarine = 0;

		const int max_marines = ASWGameResource()->GetMaxMarineResources();
		for ( int i = 0; i < max_marines; i++ )
		{
			CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
			if ( pMR )
			{
				if ( pMR->m_Commander.Get() == this )
				{
					if ( pMR->GetMarineEntity() == pNPC )
					{
						nNewMarine = nNumMarines;
					}

					nNumMarines++;
				}
			}
		}

		IGameEvent *event = gameeventmanager->CreateEvent( "player_commanding" );
		if ( event )
		{
			event->SetInt( "userid", GetUserID() );
			event->SetInt( "new_marine", pNPC->entindex() );
			event->SetInt( "new_index", nNewMarine );
			event->SetInt( "count", nNumMarines );
			gameeventmanager->FireEvent( event );
		}
	}
}

void CASW_Player::SetNPC( CASW_Inhabitable_NPC *pNPC )
{
	Assert( !pNPC || !IsSpectatorOnly() );

	if ( pNPC && pNPC != GetNPC() )
	{
		if ( !ASWGameResource() )
		{
			return;
		}

		if ( pNPC->Classify() == CLASS_ASW_MARINE )
		{
			int nNumMarines = 0;
			int nOldMarine = 0;

			const int max_marines = ASWGameResource()->GetMaxMarineResources();
			for ( int i = 0; i < max_marines; ++i )
			{
				CASW_Marine_Resource *pMR = ASWGameResource()->GetMarineResource( i );
				if ( pMR )
				{
					if ( pMR->m_Commander.Get() == this )
					{
						if ( pMR->GetMarineEntity() == GetNPC() )
						{
							nOldMarine = nNumMarines;
						}

						nNumMarines++;
					}
				}
			}

			IGameEvent *event = gameeventmanager->CreateEvent( "marine_selected" );
			if ( event )
			{
				event->SetInt( "userid", GetUserID() );
				event->SetInt( "new_marine", pNPC->entindex() );
				event->SetInt( "old_marine", ( GetNPC() ? GetNPC()->entindex() : -1 ) );
				event->SetInt( "old_index", nOldMarine );
				event->SetInt( "count", nNumMarines );
				gameeventmanager->FireEvent( event );
			}
		}

		if ( GetASWControls() != ASWC_TOPDOWN )
		{
			// make sure we're facing the right way when we start controlling a character in first/third person
			SnapEyeAngles( pNPC->GetLocalAngles() );
		}

		m_hInhabiting = pNPC;
		// make sure our list of usable entities is refreshed
		FindUseEntities();
	}
}

CASW_Inhabitable_NPC *CASW_Player::GetNPC() const
{
	return m_hInhabiting.Get();
}

HSCRIPT CASW_Player::ScriptGetNPC() const
{
	return ToHScript( GetNPC() );
}

void CASW_Player::SpectateNextMarine()
{
	CASW_Game_Resource* pGameResource = ASWGameResource();
	if (!pGameResource)
		return;
	CASW_Marine *pFirst = NULL;
	//Msg("CASW_Player::SpectateNextMarine\n");

	// loop through all marines
	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		//Msg("Checking pMR %d\n", i);
		CASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
		if (!pMR)
			continue;
		CASW_Marine *pMarine = pMR->GetMarineEntity();
		if (!pMarine || !pMarine->IsAlive() || pMarine->GetHealth() <= 0)
		{
			//Msg(" but he's dead\n");
			continue;
		}
		if (!pFirst)
		{
			pFirst = pMarine;
			//Msg("  set this guy as our first\n");
		}
		if (GetSpectatingNPC() == NULL)		// if we're not spectating anything yet, then spectate the first one we find
		{
			//Msg("  We're not spectating anyone, so we're gonna spec this dude\n");
			SetSpectatingNPC(pMarine);
			break;
		}
		if (GetSpectatingNPC() == pMarine)	// if we're spectating this one, then clear it, so the next one we find will get set
		{
			//Msg("  we're spectating this dude, so clearing our current spectator\n");
			SetSpectatingNPC(NULL);
		}				
	}
	//Msg("end\n");
	// if we're still not spectating anything but we found at least marine, then that means we were spectating the last one in the list and need to set this
	if (GetSpectatingNPC() == NULL && pFirst)
	{
		//Msg("  but we're still not speccing anyone and we have a first set, so speccing that dude\n");
		SetSpectatingNPC(pFirst);
	}
}

void CASW_Player::SetSpectatingNPC( CASW_Inhabitable_NPC *pSpectating )
{
	m_hSpectating = pSpectating;
}

CASW_Inhabitable_NPC *CASW_Player::GetSpectatingNPC() const
{
	return m_hSpectating.Get();
}

HSCRIPT CASW_Player::ScriptGetSpectatingNPC() const
{
	return ToHScript( GetSpectatingNPC() );
}

CASW_Inhabitable_NPC *CASW_Player::GetViewNPC() const
{
	CASW_Inhabitable_NPC *pNPC = GetSpectatingNPC();
	if ( !pNPC )
		pNPC = GetNPC();

	return pNPC;
}

HSCRIPT CASW_Player::ScriptGetViewNPC() const
{
	return ToHScript( GetViewNPC() );
}

void CASW_Player::SelectNextMarine(bool bReverse)
{
	// find index of current marine and our total number of live marines
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return;

	Msg("CASW_Player::SelectNextMarine reverse=%d\n", bReverse);

	int iMarines = 0;
	int iCurrent = -1;
	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource(i);
		if (pMR && pMR->GetCommander() == this)
		{
			iMarines++;
			if (pMR->GetMarineEntity() && pMR->GetMarineEntity() == GetNPC())
				iCurrent = iMarines;
		}
	}
	Msg("Marines = %d Current = %d\n", iMarines, iCurrent);
	// if we don't have any of our marines selected (maybe we died), then put us in at the start of the list
	if (iCurrent == -1 && iMarines > 1)
		iCurrent = 1;
	if (iCurrent != -1 && iMarines > 1)
	{
		int iTarget = iCurrent + (bReverse ? -1 : 1);
		if (iTarget <= 0)
			iTarget = iMarines;
		if (iTarget > iMarines)
			iTarget = 1;

		int k = 6;
		while (!CanSwitchToMarine(iTarget-1) && k >= 0)
		{
			iTarget += (bReverse ? -1 : 1);
			if (iTarget <= 0)
				iTarget = iMarines;
			if (iTarget > iMarines)
				iTarget = 1;
			k--;
		}		
		Msg(" wrapped to %d and sent as \n", iTarget, iTarget-1);
		SwitchMarine(iTarget-1);
		// todo: this currently doesn't work properly if we have dead marines! (it'll try to select a dead one and just end up doing nothing)
		//&& pMR->GetHealthPercent() > 0 
	}
}

bool CASW_Player::CanSwitchToMarine(int num)
{
	if (!ASWGameResource())
		return false;
	int max_marines = ASWGameResource()->GetMaxMarineResources();
	for (int i=0;i<max_marines;i++)
	{		
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource(i);
		if (pMR)
		{
			if ((CASW_Player*) pMR->m_Commander == this)
			{
				num--;
				if (num < 0 && pMR->GetMarineEntity() )
				{
					// abort if we're trying to switch to a dead marine
					if (pMR->GetMarineEntity()->GetHealth() <= 0)
					{
						return false;
					}
					return true;
				}
			}
		}
	}
	return false;
}

void CASW_Player::SwitchInhabiting( CASW_Inhabitable_NPC *pNPC )
{
	Assert( !IsSpectatorOnly() );

	CASW_Inhabitable_NPC *pOld = GetNPC();

	// abort if we're trying to switch to a dead NPC
	if ( !pNPC || pNPC->GetHealth() <= 0 )
	{
		return;
	}

	if ( pOld )
	{
		if ( pNPC == pOld )
			return;
		pOld->UninhabitedBy( this );
	}
	else
	{
		// old marine is dead and deleted, uninhabit marine resource
		int max_marines = ASWGameResource()->GetMaxMarineResources();
		for ( int i = 0; i < max_marines; i++ )
		{
			CASW_Marine_Resource* pRes = ASWGameResource()->GetMarineResource( i );
			if ( pRes && pRes->GetCommander() == this && pRes->IsInhabited() )
			{
				pRes->SetInhabited( false );
			}
		}
	}

	if ( asw_rts_controls.GetBool() )
	{
		DevMsg("Marine is at: %f, %f, %f\n", pNPC->GetAbsOrigin().x, pNPC->GetAbsOrigin().y, pNPC->GetAbsOrigin().z);
		Vector vecNewOrigin = pNPC->GetAbsOrigin() + Vector(0, -200, 400);
		SetAbsOrigin( vecNewOrigin );
		DevMsg("Moved cam to: %f, %f, %f\n", vecNewOrigin.x, vecNewOrigin.y, vecNewOrigin.z);
		return;
	}

	m_ASWLocal.m_hAutoAimTarget.Set( NULL );

	SetNPC( pNPC );
	SetSpectatingNPC( NULL );
	pNPC->SetCommander( this );
	pNPC->InhabitedBy( this );

	if ( !m_bFirstInhabit )
	{
		m_bFirstInhabit = true;
	}
}

void CASW_Player::SwitchMarine( CASW_Marine_Resource *pMR, bool set_squad_leader )
{
	CASW_Marine *pOldMarine = CASW_Marine::AsMarine( GetNPC() );
	CASW_Marine *pNewMarine = pMR->GetMarineEntity();

	SwitchInhabiting( pNewMarine );

	if ( gpGlobals->curtime > ASWGameRules()->m_fMissionStartedTime + 5.0f )
	{
		// If it's not the very beginning of the level... go ahead and say it
		pNewMarine->GetMarineSpeech()->Chatter( CHATTER_SELECTION );
	}

	CASW_SquadFormation *pSquad = pNewMarine->GetSquadFormation();
	if ( pSquad && set_squad_leader )
	{
		pSquad->ChangeLeader( pNewMarine, false );
	}

	if ( pOldMarine )
	{
		if ( pOldMarine->GetASWOrders() == ASW_ORDER_HOLD_POSITION )
		{
			pOldMarine->OrdersFromPlayer( this, ASW_ORDER_HOLD_POSITION, pNewMarine, false, GetLocalAngles().y );
		}
		else
		{
			pOldMarine->OrdersFromPlayer( this, ASW_ORDER_FOLLOW, pNewMarine, false );
		}
	}
}

// select the nth marine in the marine info list owned by this player
void CASW_Player::SwitchMarine(int num, bool set_squad_leader/* = true*/)
{
	if ( !ASWGameResource() )
		return;

	int max_marines = ASWGameResource()->GetMaxMarineResources();
	for ( int i = 0; i < max_marines; i++ )
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource( i );
		if ( pMR )
		{
			if ( pMR->GetCommander() == this )
			{
				num--;
				if ( num < 0 && pMR->GetMarineEntity() )
				{
					SwitchMarine( pMR, set_squad_leader );
					return;
				}
			}
		}
	}

	// if we got here, it means we pushed a marine number greater than the number of marines we have
	// check again, this time counting up other player's marines, to see if we're trying to shout out to them (or trying to spectate them, if we're all dead)
	CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC() );
	bool bSpectating = ( !pMarine || pMarine->GetHealth() <= 0 ) && !HasLiveMarines();
	for (int i = 0; i < max_marines; i++ )
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource( i );
		if ( pMR )
		{
			if ((CASW_Player*) pMR->m_Commander != this)
			{
				num--;
				if (num < 0)
				{		
					// do a chatter call to this marine
					CASW_Marine_Profile *pProfile = pMR->GetProfile();
					if (pProfile)
					{
						if ( bSpectating && pMR->GetMarineEntity() )
						{
							SetSpectatingNPC( pMR->GetMarineEntity() );
							return;
						}
						else if ( pMarine )
						{
							if (!Q_stricmp(pProfile->m_ShortName, "#asw_name_sarge"))
								pMarine->GetMarineSpeech()->Chatter(CHATTER_SARGE);
							else if (!Q_stricmp(pProfile->m_ShortName, "#asw_name_jaeger"))
								pMarine->GetMarineSpeech()->Chatter(CHATTER_JAEGER);
							else if (!Q_stricmp(pProfile->m_ShortName, "#asw_name_wildcat"))
								pMarine->GetMarineSpeech()->Chatter(CHATTER_WILDCAT);
							else if (!Q_stricmp(pProfile->m_ShortName, "#asw_name_wolfe"))
								pMarine->GetMarineSpeech()->Chatter(CHATTER_WOLFE);
							else if (!Q_stricmp(pProfile->m_ShortName, "#asw_name_faith"))
								pMarine->GetMarineSpeech()->Chatter(CHATTER_FAITH);
							else if (!Q_stricmp(pProfile->m_ShortName, "#asw_name_bastille"))
								pMarine->GetMarineSpeech()->Chatter(CHATTER_BASTILLE);
							else if (!Q_stricmp(pProfile->m_ShortName, "#asw_name_crash"))
								pMarine->GetMarineSpeech()->Chatter(CHATTER_CRASH);
							else if (!Q_stricmp(pProfile->m_ShortName, "#asw_name_flynn"))
								pMarine->GetMarineSpeech()->Chatter(CHATTER_FLYNN);
							else if (!Q_stricmp(pProfile->m_ShortName, "#asw_name_vegas"))
								pMarine->GetMarineSpeech()->Chatter(CHATTER_VEGAS);
							return;
						}
					}
				}
			}
		}
	}
	
}

void CASW_Player::OrderMarineFace( int iMarine, float fYaw, Vector &vecOrderPos )
{
	//Msg("Ordering marine %d ", iMarine);
	// check if we were passed an ent index of the marine we're ordering

	CASW_Marine *pTarget = NULL;
	CBaseEntity *pEnt = CBaseEntity::Instance( iMarine );
	if ( iMarine != -1 && pEnt && pEnt->Classify() == CLASS_ASW_MARINE )
		pTarget = assert_cast< CASW_Marine * >( pEnt );

	CASW_Marine *pMyMarine = CASW_Marine::AsMarine( GetNPC() );
	if ( !pMyMarine )
		return;

	// if we don't have a specific marine to order, find the best one
	if ( !pTarget )
	{
		CASW_Game_Resource *pGameResource = ASWGameResource();
		if ( !pGameResource )
			return;

		// if we didn't specify a marine, we'll order the one nearest to the order pos

		// check if we preselected a specific marine to order
		pTarget = m_hOrderingMarine.Get();

		// find the nearest marine
		if ( !pTarget )
		{
			float nearest_dist = 9999;
			for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
				if ( !pMR )
					continue;

				CASW_Marine *pMarine = pMR->GetMarineEntity();
				if ( !pMarine || pMarine == pMyMarine || pMarine->GetHealth() <= 0		// skip if dead
					|| pMarine->GetCommander() != this )
					continue;

				float distance = vecOrderPos.DistTo( pMarine->GetAbsOrigin() );
				if ( pMarine->GetASWOrders() != ASW_ORDER_FOLLOW )		// bias against marines that are already holding position somewhere
					distance += 5000;
				if ( distance < nearest_dist )
				{
					nearest_dist = distance;
					pTarget = pMarine;
				}
			}
		}
	}

	// do an emote
	//pMyMarine->DoEmote(3);	// stop

	if ( !pTarget )
		return;

	pTarget->OrdersFromPlayer( this, ASW_ORDER_MOVE_TO, pMyMarine, true, fYaw, &vecOrderPos );
}

// makes the player uninhabit marines and become free in control of his
//   player entity as normal (this is just for debugging)
void CASW_Player::LeaveMarines()
{
	if (GetNPC())
	{
		GetNPC()->UninhabitedBy( this );
	}
	m_hInhabiting = NULL;
}

void CASW_Player::ChangeName( const char *pszNewName )
{
	// make sure name is not too long
	char trimmedName[MAX_PLAYER_NAME_LENGTH];
	Q_strncpy( trimmedName, pszNewName, sizeof( trimmedName ) );

	const char *pszOldName = GetPlayerName();

	//char text[256];
	//Q_snprintf( text,sizeof(text), "%s changed name (CASW_Player::ChangeName) to %s\n", pszOldName, trimmedName );
	//UTIL_ClientPrintAll( HUD_PRINTTALK, text );

	// broadcast event
	IGameEvent * event = gameeventmanager->CreateEvent( "player_changename" );
	if ( event )
	{
		event->SetInt( "userid", GetUserID() );
		event->SetString( "oldname", pszOldName );
		event->SetString( "newname", trimmedName );
		gameeventmanager->FireEvent( event );
	}

	// change shared player name
	SetPlayerName( trimmedName );

	// tell engine to use new name
	engine->ClientCommand( edict(), "name \"%s\"", trimmedName );
}

void CASW_Player::OnFullyJoined( bool bSendGameEvent )
{
	if ( !m_bSentJoinedMessage )
	{
		if ( gpGlobals->maxClients > 1 )
		{
			if ( bSendGameEvent )
			{
				IGameEvent *event = gameeventmanager->CreateEvent( "player_fullyjoined" );
				if ( event )
				{
					event->SetInt( "userid", GetUserID() );
					event->SetString( "name", GetPlayerName() );

					gameeventmanager->FireEvent( event );
				}
			}

			// if we're meant to be leader then make it so
			if ( ASWGameResource() && !Q_strcmp( GetASWNetworkID(), ASWGameResource()->GetLastLeaderNetworkID() ) && ASWGameResource()->GetLeader() != this )
			{
				ASWGameResource()->SetLeader( this );
				if ( bSendGameEvent )
				{
					UTIL_ClientPrintAll( ASW_HUD_PRINTTALKANDCONSOLE, "#asw_player_made_leader", GetPlayerName() );
				}
				DevMsg( "Network ID=%s LastLeaderNetworkID=%s\n", GetASWNetworkID(), ASWGameResource()->GetLastLeaderNetworkID() );
			}

			UTIL_RestartAmbientSounds();
		}
		m_bSentJoinedMessage = true;
	}
	// check for getting back any marines we lost
	bool bReturnedMarines = false;
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( pGameResource )
	{
		const char *szNetworkID = GetASWNetworkID();
		for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
		{
			CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
			if ( !pMR )
				continue;
			CASW_Marine *pMarine = pMR->GetMarineEntity();

			if ( pMarine && !Q_strcmp( pMarine->m_szInitialCommanderNetworkID, szNetworkID ) )
			{
				bool bWasInhabited = pMarine->IsInhabited();
				CASW_Player *pTempCommander = pMarine->GetCommander();
				if ( bWasInhabited && pTempCommander )
				{
					// if another player is currently controlling one of our old marines, have him switch out
					//  this will likely be confusing for them!  need some message?
					pTempCommander->LeaveMarines();
				}
				DevMsg( "Fully joined, marine %d previous ID = %s my ID = %s\n", i, pMarine->m_szInitialCommanderNetworkID, szNetworkID );
				pMarine->SetCommander( this );
				pMR->SetCommander( this );
				bReturnedMarines = true;

				// if another player was controlling one of our marines, make sure he switches to one of his own if he can
				if ( bWasInhabited && pTempCommander )
				{
					pTempCommander->SwitchMarine( 0 );
				}
			}
		}
	}
	if ( bReturnedMarines )
	{
		DevMsg( " Fully joined player switching to marine 0\n" );
		SwitchMarine( 0 );
	}
	ASWGameRules()->OnPlayerFullyJoined( this );

	if ( !m_bWelcomed && rm_welcome_message.GetString()[0] != 0 )
	{
		SetContextThink( &CASW_Player::WelcomeMessageThink, gpGlobals->curtime + rm_welcome_message_delay.GetFloat(), s_pWelcomeMessageContext );
	}
}

#define ASW_NO_SERVERSIDE_AUTOAIM

Vector CASW_Player::GetAutoaimVectorForMarine(CASW_Marine* marine, float flDelta, float flNearMissDelta)
{
#ifdef ASW_NO_SERVERSIDE_AUTOAIM
	// test of no serverside autoaim
	Vector	forward;
	if ( GetNPC() == marine )
		AngleVectors( EyeAngles() + m_angMarineAutoAimFromClient, &forward );
	else
		AngleVectors( EyeAngles(), &forward );	//  + m_Local.m_vecPunchAngle
	return	forward;
#else
	//if ( ( ShouldAutoaim() == false ) || ( flDelta == 0 ) )	
	if (GetMarine() == NULL)
	{
		Vector	forward;
		AngleVectors( EyeAngles(), &forward );	//  + m_Local.m_vecPunchAngle
		//Msg("Not autoaiming\n");
		return	forward;	
	}

	Vector vecSrc	= GetMarine()->Weapon_ShootPosition( );
	float flDist	= MAX_COORD_RANGE;

	QAngle angles = MarineAutoaimDeflection( vecSrc, flDist, flDelta, flNearMissDelta );

	// update ontarget if changed
	if ( !g_pGameRules->AllowAutoTargetCrosshair() )
		m_fOnTarget = false;

	if (angles.x > 180)
		angles.x -= 360;
	if (angles.x < -180)
		angles.x += 360;
	if (angles.y > 180)
		angles.y -= 360;
	if (angles.y < -180)
		angles.y += 360;

	Vector	forward;
	AngleVectors( EyeAngles() + angles, &forward );	// + m_Local.m_vecPunchAngle

	m_angMarineAutoAim = angles;
	
	return forward;
#endif
}

QAngle CASW_Player::MarineAutoaimDeflection( Vector &vecSrc, float flDist, float flDelta, float flNearMissDelta )
{
	float		bestdot;
	Vector		bestdir;
	CBaseEntity	*bestent;
	trace_t		tr;
	Vector		v_forward, v_right, v_up;

	//if ( ShouldAutoaim() == false )
	//{
		//m_fOnTarget = false;
		//return vec3_angle;
	//}

	AngleVectors( EyeAngles() + m_angMarineAutoAim, &v_forward, &v_right, &v_up );  //  + m_Local.m_vecPunchAngle

	// try all possible entities
	bestdir = v_forward;
	bestdot = flDelta; // +- 10 degrees
	bestent = NULL;

	// near misses
	Vector nearmissdir = v_forward;
	float nearmissdot = flNearMissDelta;
	CBaseEntity *nearmissent = NULL;

	//Reset this data
	m_fOnTarget			= false;
	CASW_Weapon *pWeapon = GetNPC() ? GetNPC()->GetActiveASWWeapon() : NULL;
	bool bDoAutoaimEnt = pWeapon && pWeapon->GetAutoAimAmount() >= 0.25f;	// only the prifle + autogun show up their autoaim target

	if (bDoAutoaimEnt || m_ASWLocal.m_hAutoAimTarget.Get())
		m_ASWLocal.m_hAutoAimTarget.Set(NULL);
	//Msg("marine aa def clearing aa ent\n");

	UTIL_TraceLine( vecSrc, vecSrc + bestdir * flDist, MASK_SHOT, GetNPC(), COLLISION_GROUP_NONE, &tr );

	CBaseEntity *pEntHit = tr.m_pEnt;
	CBaseEntity* pFlareEnt = NULL;

	if ( pEntHit && pEntHit->m_takedamage != DAMAGE_NO)
	{
		//m_hAutoAimTarget = pEntHit;

		// don't look through water
		if (!((GetWaterLevel() != 3 && pEntHit->GetWaterLevel() == 3) || (GetWaterLevel() == 3 && pEntHit->GetWaterLevel() == 0)))
		{
			if ( pEntHit->GetFlags() & FL_AIMTARGET )
			{
				m_fOnTarget = true;
				if (bDoAutoaimEnt)
					m_ASWLocal.m_hAutoAimTarget.Set(pEntHit);
			}

			//Already on target, don't autoaim
			//Msg("Already on target (%s), not autoaiming\n", pEntHit->GetClassname());
			
			return vec3_angle;
		}
	}

	int count = AimTarget_ListCount();
	//Msg("checking autoaim over %d targets\n", count);
	if ( count )
	{
		CBaseEntity **pList = (CBaseEntity **)stackalloc( sizeof(CBaseEntity *) * count );
		AimTarget_ListCopy( pList, count );

		for ( int i = 0; i < count; i++ )
		{
			Vector center, center_flat;
			Vector dir, dir_flat;
			float dot, dot_flat;
			CBaseEntity *pEntity = pList[i];

			//Msg("Checking autoaim vs %d:%s", pEntity->entindex, pEntity->GetClassname());

			// Don't shoot yourself
			if ( pEntity == this || pEntity == GetNPC())
			{
				//Msg("this is you!, skipping\n");
				continue;
			}

			if (!pEntity->IsAlive() || !pEntity->edict() )
			{
				//Msg("not alive or not an edict, skipping\n");
				continue;
			}
	
 			// don't autoaim onto marines for coop
 			if ( !ASWDeathmatchMode() && pEntity->Classify() == CLASS_ASW_MARINE)
 				continue;

			//if ( !g_pGameRules->ShouldAutoAim( this, pEntity->edict() ) )
				//continue;

			// don't look through water
			if ((GetWaterLevel() != 3 && pEntity->GetWaterLevel() == 3) || (GetWaterLevel() == 3 && pEntity->GetWaterLevel() == 0))
			{
				//Msg("not looking through water, skipping\n");
				continue;
			}

			// Only shoot enemies!
			//if ( IRelationType( pEntity ) != D_HT )
			//{
				//if ( !pEntity->IsPlayer() && !g_pGameRules->IsDeathmatch())
					// Msg( "friend\n");
					//continue;
			//}

			center = pEntity->BodyTarget( vecSrc );
			center_flat = center;
			center_flat.z = vecSrc.z;

			dir = (center - vecSrc);
			VectorNormalize( dir );

			dir_flat = (center_flat - vecSrc);
			VectorNormalize( dir_flat );

			// make sure it's in front of the player
			if (DotProduct (dir, v_forward ) < 0)
			{
				//Msg("not in front of you, skipping\n");
				continue;
			}

			dot = fabs( DotProduct (dir, v_right ) ) 
				+ fabs( DotProduct (dir, v_up ) ) * 0.5;

			dot_flat = fabs( DotProduct (dir_flat, v_right ) ) 
				+ fabs( DotProduct (dir_flat, v_up ) ) * 0.5;

			// tweak for distance
			dot *= 1.0 + 0.2 * ((center - vecSrc).Length() / flDist);
			dot_flat *= 1.0 + 0.2 * ((center - vecSrc).Length() / flDist);

			// asw: we're only using the flat dot here
			//  should really fix this to give priority to aliens that are nearer your z, instead of just nearer on the x and y

			// asw temp lose the z autoaim
			dot_flat = dot;

			// check for 'near misses' that change the Z aim only
			if (dot_flat < nearmissdot)
			{
				// todo: check if we have LOS trace?
				nearmissdot = dot_flat;
				nearmissent = pEntity;
				nearmissdir = dir;
			}
			
			// check for actual autoaim
			if (dot_flat > bestdot)
			{
				//Msg("too far to turn, skipping\n");
				if (bestent==NULL && dot_flat <= ASW_FLARE_AUTOAIM_DOT
					&& ASWGameRules()->CanFlareAutoaimAt(CASW_Marine::AsMarine( GetNPC() ), pEntity))
				{
					pFlareEnt = pEntity;
					// allow autoaim at this entity because it's inside a flare radius
					if (asw_debug_marine_damage.GetBool())
						Msg("Flare autoaiming!");
				}
				else
				{
					continue;	// too far to turn
				}
			}

			UTIL_TraceLine( vecSrc, center, MASK_SHOT, GetNPC(), COLLISION_GROUP_NONE, &tr );

			if (tr.fraction != 1.0 && tr.m_pEnt != pEntity )
			{
				//Msg( "hit %s, can't see %s, skipping\n", STRING( tr.u.ent->classname ), STRING( pEdict->classname ) );
				//Msg("Can't see, skipping\n");
				continue;
			}

			// can shoot at this one
			bestdot = dot_flat;
			bestent = pEntity;
			bestdir = dir;
			//Msg("can see, storing!\n");
		}
		if ( bestent )
		{
			if (asw_DebugAutoAim.GetBool())
			{
				Msg("Autoaiming at a %s dot=%f\n", bestent->GetClassname(), bestdot);
				NDebugOverlay::EntityBounds(bestent, 255,0,0, 255, 1.0f);
			}
			QAngle bestang;
			VectorAngles( bestdir, bestang );

			bestang -= EyeAngles();	 // - m_Local.m_vecPunchAngle

			if (bDoAutoaimEnt || (pFlareEnt == bestent))
				m_ASWLocal.m_hAutoAimTarget.Set(bestent);
			//Msg("marine aa def setting aa ent to %d\n", bestent->entindex());

			m_fOnTarget = true;

			return bestang;
		}
		/*
		else if (nearmissent)
		{
			if (asw_DebugAutoAim.GetBool())
			{
				Msg("Nearmiss aiming at a %s dot=%f\n", nearmissent->GetClassname(), nearmissdot);
				NDebugOverlay::EntityBounds(nearmissent, 0,255,0, 255, 1.0f);
			}
			QAngle bestang;
			Vector verticalonlydir = v_forward;
			verticalonlydir.z = nearmissdir.z;
			VectorAngles( verticalonlydir, bestang );

			bestang -= EyeAngles();	 //  - m_Local.m_vecPunchAngle

			//m_hAutoAimTarget = bestent;
			//m_fOnTarget = true;
			return bestang;
		}
		*/
	}

	return QAngle( 0, 0, 0 );
}

void CASW_Player::FlashlightTurnOn()
{
	/*
	if (GetMarine() && !(GetMarine()->GetFlags() & FL_FROZEN))	// don't allow this if the marine is frozen
		GetMarine()->FlashlightToggle();
	else
		BaseClass::FlashlightTurnOn();
	*/
}

void CASW_Player::FlashlightTurnOff()
{
	/*if (GetMarine())
		GetMarine()->FlashlightTurnOff();
	else
		BaseClass::FlashlightTurnOff();
		*/
}

void OrderNearestHoldingMarineToFollow()
{
	CASW_Player *pPlayer = ToASW_Player(UTIL_GetCommandClient());
	if (pPlayer)
	{
		CASW_Marine *pMyMarine = CASW_Marine::AsMarine( pPlayer->GetNPC() );
		if (pMyMarine)
		{
			if (pMyMarine->GetFlags() & FL_FROZEN)	// don't allow this if the marine is frozen
				return;

			CASW_Game_Resource *pGameResource = ASWGameResource();
			if (!pGameResource)
				return;

			// check if we preselected a specific marine to order
			CASW_Marine *pTarget = pPlayer->m_hOrderingMarine.Get();

			// find the nearest holding marine
			if (!pTarget)
			{
				float nearest_dist = 9999;
				for (int i = 0; i < pGameResource->GetMaxMarineResources(); i++)
				{
					CASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
					if (!pMR)
						continue;

					CASW_Marine* pMarine = pMR->GetMarineEntity();
					if (!pMarine || pMarine == pMyMarine || pMarine->GetHealth() <= 0		// skip if dead
						|| pMarine->GetASWOrders() == ASW_ORDER_FOLLOW	// skip if already following
						|| pMarine->GetCommander() != pPlayer)
						continue;

					float distance = pMyMarine->GetAbsOrigin().DistTo(pMarine->GetAbsOrigin());
					if (pMarine->GetASWOrders() != ASW_ORDER_HOLD_POSITION)		// bias against marines that are already moving somewhere
						distance += 5000;
					if (distance < nearest_dist)
					{
						nearest_dist = distance;
						pTarget = pMarine;
					}
				}
			}

			// do an emote
			pMyMarine->DoEmote(4);	// stop


			// abort if we couldn't find another marine to order
			if (!pTarget)
				return;

			pTarget->OrdersFromPlayer(pPlayer, ASW_ORDER_FOLLOW, pPlayer->GetNPC(), true);
		}
	}
}

void OrderNearbyMarines(CASW_Player *pPlayer, ASW_Orders NewOrders, bool bAcknowledge)
{
	if ( !pPlayer )
		return;

	CASW_Marine *pMyMarine = CASW_Marine::AsMarine( pPlayer->GetNPC() );
	if ( pMyMarine )
	{
		if ( pMyMarine->GetFlags() & FL_FROZEN )	// don't allow this if the marine is frozen
			return;

		// BenLubar: if player gave follow command, bots will follow not 
		// using hints 
		if ( NewOrders == ASW_ORDER_FOLLOW && bAcknowledge && pMyMarine->GetSquadFormation() && pMyMarine->GetSquadFormation()->Leader() == pMyMarine )
			pMyMarine->GetSquadFormation()->FollowCommandUsed();

		// go through all marines and tell them to follow our marine
		CASW_Game_Resource *pGameResource = ASWGameResource();
		if ( !pGameResource )
			return;

		// do an emote
		if ( NewOrders == ASW_ORDER_HOLD_POSITION && bAcknowledge )
		{
			pMyMarine->DoEmote( 3 );	// stop
		}

		else if ( NewOrders == ASW_ORDER_FOLLOW && bAcknowledge )
		{
			pMyMarine->DoEmote( 4 );	// go
		}

		// first count how many marines are in range
		int iNearby = 0;
		for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
		{
			CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
			if ( !pMR )
				continue;

			CASW_Marine* pMarine = pMR->GetMarineEntity();
			if ( !pMarine || pMarine == pMyMarine || pMarine->GetCommander() != pPlayer )
				continue;
		
			float distance = pMyMarine->GetAbsOrigin().DistTo( pMarine->GetAbsOrigin() );
			if ( distance < MARINE_ORDER_DISTANCE )
			{
				iNearby++;
			}
		}

		// pick one to chatter
		int iChatter = random->RandomInt( 0, iNearby - 1 ) + 1;

		// give the orders
		for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
		{
			CASW_Marine_Resource* pMR = pGameResource->GetMarineResource( i );
			if ( !pMR )
				continue;

			CASW_Marine* pMarine = pMR->GetMarineEntity();
			if ( !pMarine || pMarine == pMyMarine || pMarine->GetHealth() <= 0 || pMarine->GetCommander() != pPlayer )
				continue;
		
			float distance = pMyMarine->GetAbsOrigin().DistTo( pMarine->GetAbsOrigin() );
			if ( distance < MARINE_ORDER_DISTANCE )
			{
				iChatter--;
				pMarine->OrdersFromPlayer( pPlayer, NewOrders, pPlayer->GetNPC(), bAcknowledge && iChatter == 0 );
			}
		}

		if ( iNearby >= 1 )
		{
			if ( NewOrders == ASW_ORDER_FOLLOW )
			{
				IGameEvent * event = gameeventmanager->CreateEvent( "player_command_follow" );
				if ( event )
				{
					event->SetInt( "userid", pPlayer->GetUserID() );
					gameeventmanager->FireEvent( event );
				}
			}
			else if ( NewOrders == ASW_ORDER_HOLD_POSITION )
			{
				IGameEvent * event = gameeventmanager->CreateEvent( "player_command_hold" );
				if ( event )
				{
					event->SetInt( "userid", pPlayer->GetUserID() );
					gameeventmanager->FireEvent( event );
				}
			}
		}
	}
}

void CASW_Player::ShowInfoMessage(CASW_Info_Message* pMessage)
{
	if (!pMessage)
		return;

	m_pCurrentInfoMessage = pMessage;
}

void CASW_Player::HideInfoMessage()
{
	m_pCurrentInfoMessage = NULL;
}

void CASW_Player::MoveMarineToPredictedPosition()
{
	CASW_Inhabitable_NPC *pNPC = GetNPC();
	if ( !pNPC )
		return;

	m_vecStoredPosition = pNPC->GetAbsOrigin();

	// sweep a bounding box ahead in the current velocity direction
	Vector vel = pNPC->GetAbsVelocity();
	INetChannelInfo *nci = engine->GetPlayerNetInfo( entindex() ); 
	if (!nci)
		return;
	float fVelScale = nci->GetLatency( FLOW_OUTGOING );
	Vector dest = m_vecStoredPosition + vel * fVelScale;

	Ray_t ray;
	trace_t pm;
	ray.Init( m_vecStoredPosition, dest, pNPC->CollisionProp()->OBBMins(), pNPC->CollisionProp()->OBBMaxs() );
	UTIL_TraceRay( ray, MASK_PLAYERSOLID, pNPC, ASW_COLLISION_GROUP_MARINE_POSITION_PREDICTION, &pm );
	
	dest = m_vecStoredPosition + vel * fVelScale * pm.fraction;
	pNPC->SetAbsOrigin(dest);
}

void CASW_Player::RestoreMarinePosition()
{
	CASW_Inhabitable_NPC *pNPC = GetNPC();
	if ( !pNPC )
		return;

	pNPC->SetAbsOrigin(m_vecStoredPosition);
}

void CASW_Player::ActivateUseIcon( int iUseEntityIndex, int nHoldType )
{	
	// no using when you're dead
	if ( !GetNPC() || GetNPC()->GetHealth() <= 0 )
		return;

	if ( GetNPC()->GetFlags() & FL_FROZEN )	// don't allow this if the marine is frozen
		return;

	CBaseEntity *pEnt = CBaseEntity::Instance(iUseEntityIndex);
	if (!pEnt)
	{
		return;
	}

	// check this item is in our usable entities list
	bool bFound = false;
	for (int i=0;i<m_iUseEntities;i++)
	{
		if (pEnt == m_hUseEntities[i].Get())
		{
			bFound = true;
			break;
		}
	}
	if (!bFound)
		return;

	IASW_Server_Usable_Entity *pUsable = dynamic_cast<IASW_Server_Usable_Entity*>(pEnt);
	if (!pUsable)
		return;

	pUsable->ActivateUseIcon( GetNPC(), nHoldType );
}

void CASW_Player::SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize )
{
	if ( asw_debug_pvs.GetBool() )
	{
		Msg( "Player:%d SetupVis\n", entindex() );
	}

	if ( m_bUsedFreeCam )
	{
		if ( asw_debug_pvs.GetBool() )
		{
			Msg( "  freecam %s\n", VecToString( m_vecFreeCamOrigin ) );
		}
		engine->AddOriginToPVS( m_vecFreeCamOrigin );
	}

	CAlienSwarm *pGameRules = ASWGameRules();
	if ( pGameRules && pGameRules->GetGameState() < ASW_GS_INGAME && pGameRules->m_hBriefingCamera )
	{
		engine->AddOriginToPVS( pGameRules->m_hBriefingCamera->GetAbsOrigin() );
	}

	CASW_Inhabitable_NPC *pNPC = GetViewNPC();
	if ( pNPC )
	{
		SetupVisibilityForNPC( pViewEntity, pvs, pvssize, pNPC );
	}
	else
	{
		//if (asw_debug_pvs.GetBool()) Msg(" Base\n");
		BaseClass::SetupVisibility( pViewEntity, pvs, pvssize );
	}

	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( rd_force_all_marines_in_pvs.GetBool() && pGameResource )
	{
		if ( ( rd_force_all_marines_in_pvs.GetInt() != 2 || !GetNPC() ) &&
			( rd_force_all_marines_in_pvs.GetInt() != 3 || V_atoi( engine->GetClientConVarValue( entindex(), "rd_auto_record_lobbies" ) ) ) )
		{
			for ( int i = 0; i < ASW_MAX_MARINE_RESOURCES; i++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
				if ( pMR && pMR->GetMarineEntity() )
				{
					SetupVisibilityForNPC( pViewEntity, pvs, pvssize, pMR->GetMarineEntity() );
				}
			}
		}
	}
}

void CASW_Player::SetupVisibilityForNPC( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize, CASW_Inhabitable_NPC *pNPC )
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	// asw - add the marine as our PVS position (since we're using radius based, this will do the job)
	if ( pMarine && pMarine->IsInVehicle() )
	{
		if ( pMarine->GetASWVehicle() && pMarine->GetASWVehicle()->GetEntity() )
			engine->AddOriginToPVS( pMarine->GetASWVehicle()->GetEntity()->GetAbsOrigin() );
	}
	else
	{
		if ( asw_debug_pvs.GetBool() )
		{
			const Vector pos = pNPC->GetAbsOrigin();
			Msg( "  Marine %f,%f,%f\n", pos.x, pos.y, pos.z );
		}
		engine->AddOriginToPVS( pNPC->GetAbsOrigin() );
	}

	// Check for mapper cameras
	CPointCamera *pMapperCamera = GetPointCameraList();
	bool bMapperCam = false;
	for ( int cameraNum = 0; pMapperCamera != NULL; pMapperCamera = pMapperCamera->m_pNext )
	{
		if ( pMapperCamera->IsActive() && !ASW_IsSecurityCam( pMapperCamera ) )
		{
			engine->AddOriginToPVS( pMapperCamera->GetAbsOrigin() );
			bMapperCam = true;
			break;
		}

		++cameraNum;
	}

	CBaseEntity *pUsing = pNPC->m_hUsingEntity.Get();
	if ( pUsing )
	{
		if ( pUsing->Classify() == CLASS_ASW_COMPUTER_AREA )
		{
			CASW_Computer_Area *pComputer = assert_cast< CASW_Computer_Area * >( pUsing );
			CASW_PointCamera *pComputerCam = pComputer->GetActiveCam();

			// check if any mapper set cameras are active, we shouldn't be on if they are
			if ( pComputer->m_hSecurityCam1 && ( !bMapperCam || pComputer->m_hSecurityCam1 != pMapperCamera ) && pComputer->m_hSecurityCam1.Get() != pComputerCam )
			{
				pComputer->m_hSecurityCam1->SetActive( false );
			}

			if ( pComputer->m_hSecurityCam2 && ( !bMapperCam || pComputer->m_hSecurityCam2 != pMapperCamera ) && pComputer->m_hSecurityCam2.Get() != pComputerCam )
			{
				pComputer->m_hSecurityCam2->SetActive( false );
			}

			if ( pComputer->m_hSecurityCam3 && ( !bMapperCam || pComputer->m_hSecurityCam3 != pMapperCamera ) && pComputer->m_hSecurityCam3.Get() != pComputerCam )
			{
				pComputer->m_hSecurityCam3->SetActive( false );
			}

			if ( pComputerCam && ( !bMapperCam || pComputerCam != pMapperCamera ) )
			{
				// if we're here, a computer camera is active
				engine->AddOriginToPVS( pComputerCam->GetAbsOrigin() );
				pComputerCam->SetActive( true );
			}
		}
	}

	if ( pMarine && pMarine->IsControllingTurret() )
	{
		engine->AddOriginToPVS( pMarine->GetRemoteTurret()->GetAbsOrigin() );
	}
}

#define ASW_PUSHAWAY_THINK_CONTEXT	"CSPushawayThink"

void CASW_Player::PushawayThink()
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC() );
	if ( pMarine && !pMarine->IsInVehicle() )
	{
		// Push physics props out of our way.
		PerformObstaclePushaway( pMarine );
	}

	SetNextThink( gpGlobals->curtime + PUSHAWAY_THINK_INTERVAL, ASW_PUSHAWAY_THINK_CONTEXT );
}

void CASW_Player::RagdollBlendTest()
{
	if (!m_pBlendRagdoll)
		return;

	m_fBlendAmount += gpGlobals->frametime * 0.25f;
	while (m_fBlendAmount > 1.0f)
		m_fBlendAmount -= 1.0f;

	// sin wave it in and out
	//float fRealBlend = sin(m_fBlendAmount * 6.284);
	float fRealBlend = m_fBlendAmount * 2;
	if (fRealBlend > 1.0f)
		fRealBlend = 2.0f - fRealBlend;
	m_pBlendRagdoll->SetBlendWeight(abs(fRealBlend) * asw_blend_test_scale.GetFloat());
}

const char* CASW_Player::GetASWNetworkID()
{
	const char *pszNetworkID = GetNetworkIDString();	
	if (!Q_strcmp(pszNetworkID, "UNKNOWN") || !Q_strcmp(pszNetworkID, "HLTV") ||
		!Q_strcmp(pszNetworkID, "STEAM_ID_LAN") || !Q_strcmp(pszNetworkID, "STEAM_ID_PENDING") || 
		!Q_strcmp(pszNetworkID, "STEAM_1:0:0"))
	{
		// player has no valid steam ID, so let's use his IP address instead		
		INetChannelInfo *nci = engine->GetPlayerNetInfo( entindex() );
		if (nci)
		{
			pszNetworkID = nci->GetAddress();
		}
		else
		{
			pszNetworkID = GetPlayerName();
		}
	}
	// chop off the steam ID part...
	if (!Q_strncmp("STEAM_ID", pszNetworkID, 8))
	{
		pszNetworkID+=8;
	}
	return pszNetworkID;
}

//-----------------------------------------------------------------------------
// Purpose: Finds the nearest entity in front of the player, preferring
//			collidable entities, but allows selection of enities that are
//			on the other side of walls or objects
// Input  :
// Output :
//-----------------------------------------------------------------------------
CBaseEntity* CASW_Player::FindPickerEntity()
{
	MDLCACHE_CRITICAL_SECTION();

	trace_t tr;
	// BenLubar(sd2-ceiling-ents): use CASW_Trace_Filter to handle *_asw_fade properly
	CASW_Trace_Filter filter( this, COLLISION_GROUP_NONE );
	UTIL_TraceLine( GetCrosshairTracePos(),
		GetCrosshairTracePos() + Vector( 0, 0, 10 ),
		MASK_SOLID, &filter, &tr );
	if ( tr.fraction != 1.0 && tr.DidHitNonWorldEntity() )
	{
		return tr.m_pEnt;
	}
	
	// If trace fails, look for the nearest entity
	CBaseEntity *pNearestEntity = NULL;
	float fNearestDistance = -1;
	CBaseEntity *pCurrentEntity = gEntList.FirstEnt();
	while ( pCurrentEntity )
	{
		if ( !pCurrentEntity->IsWorld() )
		{
			float fDistance = pCurrentEntity->WorldSpaceCenter().DistTo( GetCrosshairTracePos() );
			if ( fNearestDistance == -1 || fDistance < fNearestDistance )
			{
				pNearestEntity = pCurrentEntity;
				fNearestDistance = fDistance;
			}
		}
		pCurrentEntity = gEntList.NextEnt( pCurrentEntity );
	}
	return pNearestEntity;
}

HSCRIPT CASW_Player::ScriptFindPickerEntity()
{
	return ToHScript( FindPickerEntity() );
}

const Vector& CASW_Player::GetCrosshairTracePos()
{
	if ( GetASWControls() != ASWC_TOPDOWN && GetNPC() )
	{
		trace_t tr;
		Vector forward;
		Vector shootposition = GetNPC()->Weapon_ShootPosition();
		EyeVectors( &forward );
		UTIL_TraceLine( shootposition,
			shootposition + forward * MAX_COORD_RANGE,
			MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );
		if ( tr.DidHit() )
		{
			m_vecCrosshairTracePos = tr.endpos;
		}
	}

	return m_vecCrosshairTracePos;
}

HSCRIPT CASW_Player::ResurrectMarine( const Vector position, bool bEffect )
{
	CASW_Marine* pMarine = NULL;
	const int numMarineResources = ASWGameResource()->GetMaxMarineResources();

	for ( int i = 0; i < numMarineResources; i++ )
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource( i );
		if ( pMR && pMR->GetHealthPercent() <= 0 ) // if marine exists, is dead
		{
			if ( this == pMR->GetCommander() )
			{
				pMarine = ASWGameRules()->ScriptResurrect( pMR, position, bEffect );
				return ToHScript( pMarine ); // don't do two in a frame
			}
		}
	}

	return ToHScript( pMarine );
}

HSCRIPT CASW_Player::ScriptGetMarine()
{
	const int numMarineResources = ASWGameResource()->GetMaxMarineResources();

	for ( int i = 0; i < numMarineResources; i++ )
	{
		CASW_Marine_Resource* pMR = ASWGameResource()->GetMarineResource( i );
		if ( pMR && pMR->IsInhabited() && pMR->GetCommander() == this )
		{
			return ToHScript( pMR->GetMarineEntity() );
		}
	}

	return NULL;
}

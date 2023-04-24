#include "cbase.h"

#ifdef CLIENT_DLL
	#include "c_asw_player.h"
	#include "c_asw_marine.h"
	#include "c_asw_game_resource.h"
	#include "c_asw_marine_resource.h"
	#include "asw_marine_command.h"
	#include "asw_imarinegamemovement.h"
	#include "asw_marine_gamemovement.h"
	#include "engine/IEngineSound.h"
	#include "c_asw_pickup.h"
	#include "c_asw_pickup_weapon.h"
	#include "c_asw_use_area.h"
	#include "c_asw_button_area.h"
	#include "c_asw_computer_area.h"	
	#include "c_asw_sentry_base.h"
	#include "iinput.h"
	#include "asw_input.h"
	#include "iclientvehicle.h"
	#include "iasw_client_vehicle.h"
	#include "c_asw_weapon.h"
	#include "c_asw_game_resource.h"
	#include "c_asw_hack.h"
	#include "iasw_client_usable_entity.h"
	#include "asw_vgui_info_message.h"
	#include "prediction.h"
	#include "igameevents.h"
	#include "c_asw_ammo_drop.h"
	#include "matchmaking/imatchframework.h"
	#define CASW_Button_Area C_ASW_Button_Area
	#define CASW_Computer_Area C_ASW_Computer_Area
	#define CASW_Use_Area C_ASW_Use_Area
	#define CASW_Pickup C_ASW_Pickup
	#define CASW_Ammo_Drop C_ASW_Ammo_Drop	
	#define CASW_Marine C_ASW_Marine
	#define CASW_Marine_Resource C_ASW_Marine_Resource
	#define CASW_Sentry_Base C_ASW_Sentry_Base
	#define CASW_Weapon C_ASW_Weapon
	#define CASW_Hack C_ASW_Hack
	#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
#else
	#include "player.h"
	#include "asw_player.h"
	#include "asw_marine.h"
	#include "asw_marine_resource.h"
	#include "usercmd.h"
	#include "igamemovement.h"
	#include "mathlib/mathlib.h"
	#include "client.h"
	#include "player_command.h"
	#include "asw_marine_command.h"
	#include "asw_imarinegamemovement.h"
	#include "asw_marine_gamemovement.h"
	#include "movehelper_server.h"
	#include "iservervehicle.h"
	#include "ilagcompensationmanager.h"
	#include "tier0/vprof.h"
	#include "asw_pickup.h"
	#include "asw_use_area.h"
	#include "asw_button_area.h"
	#include "asw_computer_area.h"
	#include "asw_sentry_base.h"
	#include "iservervehicle.h"
	#include "iasw_vehicle.h"
	#include "asw_weapon.h"
	#include "asw_hack.h"
	#include "iasw_server_usable_entity.h"
	#include "asw_lag_compensation.h"
	#include "asw_ammo_drop.h"
	extern ConVar asw_move_marine;
#endif
#include "asw_gamerules.h"
#include "asw_remote_turret_shared.h"
#include "asw_shareddefs.h"
#include "asw_usableobjectsenumerator.h"
#include "in_buttons.h"
#include "asw_weapon_parse.h"
#include "collisionutils.h"
#include "particle_parse.h"
#include "cdll_int.h"
#include "asw_equipment_list.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IGameMovement *g_pGameMovement;
extern IMarineGameMovement *g_pMarineGameMovement;
extern CMoveData *g_pMoveData;	// This is a global because it is subclassed by each game.
extern ConVar sv_noclipduringpause;
extern ConVar rd_revive_duration;
extern ConVar rd_revive_tombstone_hold_duration;

static void ASWControlsChanged( IConVar *var, const char *pOldValue, float flOldValue );

ConVar asw_allow_detach( "asw_allow_detach", "0", FCVAR_REPLICATED | FCVAR_CHEAT, "Allow the camera to detach from the marine." );
ConVar asw_DebugAutoAim( "asw_DebugAutoAim", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_marine_nearby_angle( "asw_marine_nearby_angle", "-75", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_vehicle_cam_shift_2_enable( "asw_vehicle_cam_shift_2_enable", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_rts_controls( "asw_rts_controls", "0", FCVAR_REPLICATED | FCVAR_CHEAT );
ConVar asw_controls( "asw_controls", "1", FCVAR_REPLICATED | FCVAR_CHEAT | FCVAR_DEMO, "Disable to get normal FPS controls (affects all players on the server)", ASWControlsChanged );
ConVar asw_controls_vehicle( "asw_controls_vehicle", "2", FCVAR_REPLICATED | FCVAR_CHEAT, "Disable to get normal FPS controls (affects all players on the server)", ASWControlsChanged );
ConVar asw_hl2_camera( "asw_hl2_camera", "0", FCVAR_REPLICATED | FCVAR_DONTRECORD | FCVAR_CHEAT );
#ifdef CLIENT_DLL
ConVar asw_controls_spectator_override( "asw_controls_spectator_override", "-1", FCVAR_DONTRECORD, "Force a value for asw_controls while spectating.", ASWControlsChanged );
#endif


static void ASWControlsChanged( IConVar *var, const char *pOldValue, float flOldValue )
{
#ifdef CLIENT_DLL
	if ( engine && engine->IsInGame() )
	{
		FOR_EACH_VALID_SPLITSCREEN_PLAYER( i )
		{
			ACTIVE_SPLITSCREEN_PLAYER_GUARD( i );

			ASWInput()->UpdateASWControls();
		}
	}
#else
	if ( CAlienSwarm *pGameRules = ASWGameRules() )
	{
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );
			if ( pPlayer )
			{
				pGameRules->ClientSettingsChanged( pPlayer );
			}
		}
	}
#endif
}

#ifdef CLIENT_DLL		
	extern ConVar asw_vehicle_cam_height;
	extern ConVar asw_vehicle_cam_pitch;
	extern ConVar asw_vehicle_cam_dist;
	extern ConVar asw_cam_marine_shift_z_death;
#endif

extern void DiffPrint( bool bServer, int nCommandNumber, char const *fmt, ... );

void CASW_Player::DriveNPCMovement( CUserCmd *ucmd, IMoveHelper *moveHelper )
{
	CASW_Inhabitable_NPC *pNPC = GetNPC();
	if ( pNPC )
	{
		MoveHelper()->SetHost( pNPC );
	}

	m_iScreenWidthHeight = ucmd->screenwh;
	m_iMouseXY = ucmd->mousexy;

	m_angMarineAutoAimFromClient = ucmd->aimangleoffset;

	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	// process turret movement
	if ( pMarine && pMarine->IsControllingTurret() )
	{
		CASW_Remote_Turret *pTurret = pMarine->GetRemoteTurret();
		if ( pTurret )
		{
			pTurret->SetupMove( this, ucmd, moveHelper, g_pMoveData );
			pTurret->ProcessMovement( this, g_pMoveData );
		}

		pMarine->PostThink();
		return;
	}

	// process vehicle movement
	if ( pMarine && pMarine->IsDriving() )
	{
#ifdef GAME_DLL
		IASW_Vehicle *pVehicle = pMarine->GetASWVehicle();
#else
		IASW_Client_Vehicle *pVehicle = pMarine->GetASWVehicle();
#endif
		if ( pVehicle )
		{
			pVehicle->SetupMove( this, ucmd, moveHelper, g_pMoveData );
			pVehicle->ProcessMovement( this, g_pMoveData );
		}
	}


#ifdef GAME_DLL	
	if ( asw_move_marine.GetBool() )
#endif
	{
		if ( pNPC && ( !pMarine || !pMarine->IsInVehicle() ) && ASWGameResource() )
		{
			// don't apply commands meant for another marine
			if ( !pMarine || ( pMarine->GetMarineResource() && ASWGameResource()->GetMarineResourceIndex( pMarine->GetMarineResource() ) == ucmd->weaponsubtype ) )
			{
				// check if we should be stopped
				if ( pMarine && ( gpGlobals->curtime < pMarine->GetStopTime() || pMarine->m_bPreventMovement )
#ifdef CLIENT_DLL
					|| CASW_VGUI_Info_Message::HasInfoMessageOpen()
#endif
					)
				{
					ucmd->forwardmove = 0;
					ucmd->sidemove = 0;
					ucmd->buttons &= ~IN_JUMP;
					// no firing when picking up stuff - does this do anything here?
					if ( gpGlobals->curtime < pMarine->GetStopTime() )
					{
						ucmd->buttons &= ~IN_ATTACK;
						ucmd->buttons &= ~IN_ATTACK2;
					}

					//ucmd->buttons |= IN_ASW_STOP;
				}

				//m_hMarine->SetMoveType( MOVETYPE_WALK ); // commented out in order for marines to use other movetypes
				MarineMove()->SetupMarineMove( this, pNPC, ucmd, moveHelper, g_pMoveData );
				g_pMarineGameMovement->ProcessMovement( this, pNPC, g_pMoveData );
				MarineMove()->FinishMarineMove( this, pNPC, ucmd, g_pMoveData );
				moveHelper->ProcessImpacts();

				// Call this from within predicted code on both client & server
				if ( pMarine )
				{
					pMarine->PostThink();

#ifdef GAME_DLL
					pMarine->PostThinkVPhysics( g_pMoveData );
					pMarine->UpdateVPhysicsAfterMove();
#endif
				}
			}

			if ( pMarine && pMarine->m_hCurrentHack.Get() )
			{
				pMarine->m_hCurrentHack->ASWPostThink( this, pMarine, ucmd, gpGlobals->frametime );
			}
			// move the player into the same position as the marine
			//SetAbsOrigin( g_pMoveData->m_vecAbsOrigin );
			//SetAbsVelocity( g_pMoveData->m_vecVelocity );
			//SetLocalAngles( g_pMoveData->m_vecAngles );


			//m_pMarine->m_nSimulationTick = gpGlobals->tickcount;  // stop the entity doing any subsequence physicssimulate
		}
	}

	MoveHelper()->SetHost( this );
}

bool CASW_Player::IsSpectatorOnly()
{
	player_info_t playerinfo{};

	bool bGotPlayerInfo = engine->GetPlayerInfo( entindex(), &playerinfo );
	Assert( bGotPlayerInfo );
	return bGotPlayerInfo && ( playerinfo.ishltv || playerinfo.isreplay );
}

bool CASW_Player::IsAnyBot()
{
	player_info_t playerinfo{};

	bool bGotPlayerInfo = engine->GetPlayerInfo( entindex(), &playerinfo );
	Assert( bGotPlayerInfo );
	return bGotPlayerInfo && ( playerinfo.fakeplayer || playerinfo.ishltv || playerinfo.isreplay );
}

bool CASW_Player::CanVote()
{
	if ( IsAnyBot() )
	{
		return false;
	}

	return true;
}

// Can't use UTIL_GetListenServerHost because SourceTV joins before the host.
static bool IsFirstNonBotPlayer( int entindex )
{
	for ( int i = 1; i < entindex; i++ )
	{
		CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
		if ( !pPlayer || !pPlayer->IsAnyBot() )
		{
			return false;
		}
	}

	return true;
}

bool CASW_Player::CanBeKicked()
{
	if ( IsAnyBot() )
	{
		return false;
	}

	if ( IsFirstNonBotPlayer( entindex() ) )
	{
#ifdef CLIENT_DLL
		KeyValues *pSettings = g_pMatchFramework->GetMatchSession() ? g_pMatchFramework->GetMatchSession()->GetSessionSettings() : NULL;

		if ( engine->IsClientLocalToActiveServer() || ( pSettings && !V_strcmp( pSettings->GetString( "server/server" ), "listen" ) ) )
#else
		if ( !engine->IsDedicatedServer() )
#endif
		{
			// can't kick listen server host from their own listen server
			return false;
		}
	}

	return true;
}

bool CASW_Player::CanBeLeader()
{
	if ( IsAnyBot() )
	{
		return false;
	}

	if ( IsSpectatorOnly() )
	{
		return false;
	}

	return true;
}

void CASW_Player::ItemPostFrame()
{
	VPROF( "CASW_Player::ItemPostFrame" );

	// Put viewmodels into basically correct place based on new polayer origin
	CalcViewModelView( EyePosition(), EyeAngles() );

#if !defined( CLIENT_DLL )
	// check if the player is using something
	if ( m_hUseEntity != NULL )
	{
		Assert( !IsInAVehicle() );
		ImpulseCommands();// this will call playerUse
		return;
	}
#endif

	CASW_Weapon* pWeapon = NULL;
	CASW_Inhabitable_NPC *pNPC = GetNPC();
	float next_attack = m_flNextAttack;
	if ( pNPC )
	{
		pWeapon = pNPC->GetActiveASWWeapon();
		next_attack = pNPC->GetNextAttack();
	}
	else
	{
		pWeapon = assert_cast< CASW_Weapon * >( GetActiveWeapon() );
	}

	// CASW_Lag_Compensation NOTE:  From this point on, weapons can turn on lag compensation.
	//  This function must call FinishLagCompensation before returning.
#ifdef GAME_DLL
	CASW_Lag_Compensation::AllowLagCompensation( this );
#endif

	if ( gpGlobals->curtime < next_attack )
	{
		if ( pWeapon )
		{
			pWeapon->ItemBusyFrame();
		}
	}
	else if (pWeapon)
	{
#ifdef CLIENT_DLL
		pWeapon->ClientPostFrame();
	
		if ( pWeapon->IsPredicted() )
			pWeapon->ItemPostFrame();
#else
		pWeapon->ItemPostFrame();
#endif
	}

	// check if offhand weapon needs postframe
	CASW_Weapon *pExtra = pNPC ? pNPC->GetASWWeapon( ASW_INVENTORY_SLOT_EXTRA ) : NULL;
	if ( pExtra && pExtra != pWeapon && pExtra->WantsOffhandPostFrame() )
	{
		pExtra->ItemPostFrame();
	}

	CASW_Weapon *pTempExtra = pNPC ? pNPC->GetASWWeapon( ASW_TEMPORARY_WEAPON_SLOT ) : NULL;
	if ( pTempExtra && pTempExtra->GetEquipItem() && pTempExtra->GetEquipItem()->m_bIsExtra )
	{
		pExtra = pTempExtra;
		if ( pExtra && pExtra != pWeapon && pExtra->WantsOffhandPostFrame() )
		{
			pExtra->ItemPostFrame();
		}
	}


	// check for offhand activation
	if ( pExtra )
	{
		const CASW_WeaponInfo* pWpnInfo = pExtra->GetWeaponInfo();
		if ( pWpnInfo && ( m_afButtonPressed & IN_GRENADE1 ) )
		{
			if ( pWpnInfo->m_bOffhandActivate )
			{
				if ( pExtra->OffhandActivate() )
				{
					IGameEvent * event = gameeventmanager->CreateEvent( "weapon_offhanded" );
					if ( event )
					{		
						event->SetInt( "userid", GetUserID() );
						gameeventmanager->FireEvent( event );
					}

#ifndef CLIENT_DLL
					// Fire event when a player uses an offhand item
					IGameEvent * event2 = gameeventmanager->CreateEvent( "weapon_offhand_activate" );
					if ( event2 )
					{
						event2->SetInt( "userid", GetUserID() );
						event2->SetInt( "marine", pNPC->entindex() );
						event2->SetInt( "weapon", pExtra->entindex() );

						gameeventmanager->FireEvent( event2 );
					}
#endif
				}
			}
			else if ( pWpnInfo->m_bOffhandSwitch )
			{
#ifdef CLIENT_DLL
				::input->MakeWeaponSelection( pExtra );
#else
				if ( gpGlobals->maxClients <= 1 )
				{
					engine->ClientCommand( edict(), "ASW_ActivateExtra" );
				}
#endif
			}
		}
	}

#if !defined( CLIENT_DLL )
	ImpulseCommands();
#else
	m_nImpulse = 0;
#endif

#ifdef GAME_DLL
	CASW_Lag_Compensation::FinishLagCompensation();	// undo lag compensation if we need to
#endif
}

// the player's eyes go above the marine he's spectating/controlling
Vector CASW_Player::EyePosition( )
{
	// revert to hl2 camera
#ifdef CLIENT_DLL
	if (asw_hl2_camera.GetBool() && engine->IsPlayingDemo())
#else
	if (asw_hl2_camera.GetBool())
#endif
	{
		return BaseClass::EyePosition();
	}

	CAlienSwarm *pGameRules = ASWGameRules();
	if ( pGameRules && pGameRules->GetGameState() < ASW_GS_INGAME && pGameRules->m_hBriefingCamera )
		return pGameRules->m_hBriefingCamera->EyePosition();

	CASW_Inhabitable_NPC *pNPC = GetSpectatingNPC();
	bool bSpectating = true;
	if ( !pNPC )
	{
		pNPC = GetNPC();
		bSpectating = false;
	}
	if ( pNPC && pNPC->GetHealth() > 0 )
	{
		m_vecLastMarineOrigin = pNPC->EyePosition();
	}

	if ( !pNPC && asw_rts_controls.GetBool() )
	{
		return GetAbsOrigin();
	}
	
	if ( m_vecLastMarineOrigin != vec3_origin )
	{
#ifdef CLIENT_DLL
		if ( asw_allow_detach.GetBool() )
		{
			return BaseClass::EyePosition();
		}

		bool bIsThirdPerson = GetASWControls() != ASWC_FIRSTPERSON;

		Vector org = vec3_origin;
		QAngle ang;
		CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
		if ( pMarine && pMarine->IsInVehicle() && GetASWControls() != ASWC_THIRDPERSONSHOULDER )
		{
			float flPitch = asw_vehicle_cam_pitch.GetFloat();
			float flDist = asw_vehicle_cam_dist.GetFloat();
			float flHeight = asw_vehicle_cam_height.GetFloat();

#ifdef GAME_DLL
			if ( IASW_Vehicle *pVehicle = pMarine->GetASWVehicle() )
#else
			if ( IASW_Client_Vehicle *pVehicle = pMarine->GetASWVehicle() )
#endif
			{
				pVehicle->ASWGetCameraOverrides( NULL, &flPitch, &flDist, &flHeight );
			}

			ang[PITCH] = flPitch;
			ang[YAW] = m_flMovementAxisYaw;
			ang[ROLL] = 0;
			AngleVectors( ang, &org );
			if ( bIsThirdPerson )
				org *= -flDist;
			org += m_vecLastMarineOrigin;
			org.z += flHeight;
		}
		else if ( pMarine && pMarine->IsControllingTurret() )
		{
			org = pMarine->GetRemoteTurret()->GetTurretCamPosition();
		}
		else
		{
			float fDeathCamInterp = ( ASWGameRules() ? ASWGameRules()->GetMarineDeathCamInterp() : 0.0f );
			if ( fDeathCamInterp <= 0.0f )
			{
				// Not doing the death cam!
				Vector vCamOffset;
				if ( bIsThirdPerson )
				{
					ang[PITCH] = ASWInput()->GetPerUser().m_vecCameraOffset[PITCH];
					ang[YAW] = ASWInput()->GetPerUser().m_vecCameraOffset[YAW];
					ang[ROLL] = 0;
					AngleVectors( ang, &vCamOffset );
					vCamOffset *= -ASWInput()->GetPerUser().m_vecCameraOffset[2];
				}
				else
				{
					vCamOffset.Init();
				}

				org = m_vecLastMarineOrigin + vCamOffset;
			}
			else
			{
				// Do the death cam!
				Vector vCamOffset;
				float fOffsetScale = 1.0f;
				Vector vDeathPos = ASWGameRules()->m_vMarineDeathPos;
				if ( ASWGameRules()->m_hMarineDeathRagdoll.Get() )
				{
					vDeathPos = ASWGameRules()->m_hMarineDeathRagdoll->WorldSpaceCenter();
				}
				vDeathPos.z += 50.0f;

				// Prevent final death cam pos from clipping through walls
				const float flMaxDeathCamInterp = 1.0f;
				float fBaseYaw = ASWInput()->ASW_GetCameraYaw( &flMaxDeathCamInterp );
				ang[PITCH] = ASWInput()->ASW_GetCameraPitch( &flMaxDeathCamInterp );
				ang[ROLL] = 0;

				float fMaxDeathCamDist = -ASWInput()->ASW_GetCameraDist( &flMaxDeathCamInterp );

				trace_t tr;
				tr.fraction = -1.0f;
				float fBestYawOffset = 0.0f;

				// Forward look at a bunch of angles to see if we can find a better one
				const int nNumAngleTests = 12;
				for ( int nAngleTest = 0; nAngleTest < nNumAngleTests; ++nAngleTest )
				{
					float fYawAngleOffset = nAngleTest * ( 360.0f / nNumAngleTests );
					ang[ YAW ] = fBaseYaw + fYawAngleOffset;

					AngleVectors( ang, &vCamOffset );
					if ( bIsThirdPerson )
					{
						vCamOffset *= fMaxDeathCamDist;
					}

					vCamOffset.z += asw_cam_marine_shift_z_death.GetFloat();

					// See if the new angle is clipping
					trace_t trTemp;
					UTIL_TraceLine( vDeathPos, vDeathPos + vCamOffset, MASK_OPAQUE, NULL, COLLISION_GROUP_DEBRIS, &trTemp );

					if ( !trTemp.DidHit() )
					{
						// Not cliping at all
						tr = trTemp;
						fBestYawOffset = fYawAngleOffset;
						break;
					}
					else if ( tr.fraction + 0.15f < trTemp.fraction )
					{
						// It's quite a bit better than what we're currently using
						tr = trTemp;
						fBestYawOffset = fYawAngleOffset;
					}
				}

				ASWGameRules()->m_fDeathCamYawAngleOffset += fBestYawOffset;
				fOffsetScale = tr.fraction;

				// Blend the death cam position with the regular game view
				ang[PITCH] = ASWInput()->ASW_GetCameraPitch( &fDeathCamInterp );
				ang[YAW] = ASWInput()->ASW_GetCameraYaw( &fDeathCamInterp );
				ang[ROLL] = 0;

				AngleVectors( ang, &vCamOffset );
				if ( bIsThirdPerson )
				{
					vCamOffset *= -ASWInput()->ASW_GetCameraDist( &fDeathCamInterp );
				}

				vCamOffset.z += fDeathCamInterp * asw_cam_marine_shift_z_death.GetFloat();

				org = ( 1.0f - fDeathCamInterp ) * m_vecLastMarineOrigin + fDeathCamInterp * vDeathPos;
				org += vCamOffset * fOffsetScale;
			}
		}

		return org;
#else
		return m_vecLastMarineOrigin + Vector(0,0,405);	// todo: make this take into account mouse location, based on the pitch coming in from player commands
#endif

	}
	
	return BaseClass::EyePosition();
}

// tries to find up to 3 nearby usable items and fills in our array (the array is inspected by the HUD to present use icons)
void CASW_Player::FindUseEntities()
{
	CBaseEntity *pOldUseEntity = NULL;
	pOldUseEntity = m_hUseEntities[ 0 ].Get();

	// Clear out all the old use ents
	m_iUseEntities = 0;
	for ( int i = 0; i < ASW_PLAYER_MAX_USE_ENTS; ++i )
	{
		m_hUseEntities[ i ] = NULL;
	}

	CASW_Inhabitable_NPC *pNPC = GetNPC();
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( !pMarine )
	{
		return;
	}

	// if we're in a vehicle, only interact with the vehicle (to get out)
	if ( pMarine && pMarine->IsInVehicle() )
	{
		if ( pMarine->GetASWVehicle() && pMarine->GetASWVehicle()->IsUsable( pMarine ) && pMarine->GetASWVehicle()->GetEntity() )
		{
			m_hUseEntities[0] = pMarine->GetASWVehicle()->GetEntity();
			m_iUseEntities = 1;
		}
		return;
	}

	CASW_UsableObjectsEnumerator items( ASW_MARINE_USE_RADIUS, this );
#ifdef CLIENT_DLL
	partition->EnumerateElementsInSphere( PARTITION_ALL_CLIENT_EDICTS, pNPC->GetAbsOrigin(), ASW_MARINE_USE_RADIUS, false, &items );
#else
	
	partition->EnumerateElementsInSphere( ASW_PARTITION_ALL_SERVER_EDICTS , pNPC->GetAbsOrigin(), ASW_MARINE_USE_RADIUS, false, &items );
#endif
	

	int c = items.GetObjectCount();
	#ifndef CLIENT_DLL
		//Msg("[S] Enumerator returned %d items.\n", c);
	#endif
	if ( c <= 0 )
	{
		return;
	}

	CBaseEntity *(pUseEntities[ 32 ]);

	for ( int i = 0; i < c && i < 32; i++ )
	{
#ifdef CLIENT_DLL
		IASW_Client_Usable_Entity *pEnt = dynamic_cast<IASW_Client_Usable_Entity*>(items.GetObject(i));
#else
		IASW_Server_Usable_Entity *pEnt = dynamic_cast<IASW_Server_Usable_Entity*>(items.GetObject(i));
#endif
		if ( pEnt )
		{
			if ( !pMarine->m_hUsingEntity.Get() || pMarine->m_hUsingEntity.Get() == items.GetObject( i ) )	// if we're in the middle of using an entity, only allow interaction with that entity
			{
				pUseEntities[ m_iUseEntities ] = items.GetObject( i );
				m_iUseEntities++;
			}
		}
	}

	// Store off use priorities first because sometimes it takes expensive traces to decide
	int nUsePriorities[ 32 ];
	for ( int i = 0; i < m_iUseEntities; ++i )
	{
		nUsePriorities[ i ] = GetUsePriority( pUseEntities[ i ] );
	}
	
	// sort the use entities
	for ( int i = 0; i < m_iUseEntities - 1; ++i )
	{
		for ( int j = 0; j < m_iUseEntities - i - 1; ++j )
		{
			SortUsePair( &pUseEntities[ j ], &pUseEntities[ j + 1 ], &nUsePriorities[ j ], &nUsePriorities[ j + 1 ] );
		}
	}

	m_iUseEntities = MIN( 3, m_iUseEntities );

	for ( int i = 0; i < m_iUseEntities; ++i )
	{
		m_hUseEntities[ i ] = pUseEntities[ i ];
	}

#ifdef CLIENT_DLL
	C_ASW_Pickup_Weapon *pWeapon = dynamic_cast<C_ASW_Pickup_Weapon*>( m_hUseEntities[ 0 ].Get() );
	if ( pWeapon && pWeapon != pOldUseEntity )
	{
		IGameEvent * event = gameeventmanager->CreateEvent( "pickup_selected" );
		if ( event )
		{
			event->SetInt( "entindex", pWeapon->entindex() );
			event->SetString( "classname", pWeapon->GetWeaponClass() );

			gameeventmanager->FireEventClientSide( event );
		}
	}
	else
	{
		C_ASW_Sentry_Base *pSentry = dynamic_cast<C_ASW_Sentry_Base*>( m_hUseEntities[ 0 ].Get() );
		if ( pSentry && pSentry != pOldUseEntity && pSentry->IsAssembled() )
		{
			IGameEvent * event = gameeventmanager->CreateEvent( "sentry_selected" );
			if ( event )
			{
				event->SetInt( "entindex", pSentry->entindex() );

				gameeventmanager->FireEventClientSide( event );
			}
		}
	}
#endif
}

// finds the priority of the pair of use entities at the specified index
//  and swaps them so the highest priority one is first
void CASW_Player::SortUsePair( CBaseEntity **pEnt1, CBaseEntity **pEnt2, int *pnFirstPriority, int *pnSecondPriority )
{
	if ( !pEnt1 || !pEnt2 || !*pEnt1 || !*pEnt2 || !pnFirstPriority || !pnSecondPriority )
		return;

	bool bSwap = false;
	if ( *pnFirstPriority == *pnSecondPriority )	// if items are the same type, put the closest first
	{
		CASW_Inhabitable_NPC *pNPC = GetNPC();
		if ( pNPC != NULL )
		{
			float fFirstDist = pNPC->GetAbsOrigin().DistToSqr( ( *pEnt1 )->WorldSpaceCenter() );
			float fSecondDist = pNPC->GetAbsOrigin().DistToSqr( ( *pEnt2 )->WorldSpaceCenter() );
			if ( fSecondDist < fFirstDist)
			{
				bSwap = true;
			}
		}
	}
	else if ( *pnSecondPriority > *pnFirstPriority )
	{
		bSwap = true;
	}

	if ( bSwap )
	{
		CBaseEntity *pTempEnt = *pEnt1;
		*pEnt1 = *pEnt2;
		*pEnt2 = pTempEnt;

		int nTemp = *pnFirstPriority;
		*pnFirstPriority = *pnSecondPriority;
		*pnSecondPriority = nTemp;
	}
}

// returns the priority of a usable entity
int CASW_Player::GetUsePriority( CBaseEntity* pEnt )
{
	if ( !pEnt )
		return 0;

	if ( !pEnt->IsEffectActive( EF_NODRAW ) )
	{
		Vector vTracePos =
#ifdef GAME_DLL
			GetCrosshairTracePos();
#else
			ASWInput()->GetCrosshairTracePos();
#endif

		CCollisionProperty *pMyProp = pEnt->CollisionProp();
		Ray_t ray;
		ray.Init( vTracePos + Vector( 0.0f, 0.0f, -10.0f ), vTracePos + Vector( 0.0f, 0.0f, 10.0f ) );

#ifdef GAME_DLL
		//NDebugOverlay::Line( vTracePos, vTracePos + Vector( 0.0f, 0.0f, 1.0f ), 255, 0, 0, true, 0.0f );
		//NDebugOverlay::BoxAngles( pMyProp->GetCollisionOrigin(), pMyProp->OBBMins(), pMyProp->OBBMaxs(), pMyProp->GetCollisionAngles(), 255, 255, 0, true, 0.0f );
#endif

		if ( IsRayIntersectingOBB( ray, pMyProp->GetCollisionOrigin(), pMyProp->GetCollisionAngles(), pMyProp->OBBMins(), pMyProp->OBBMaxs() ) )
		{
			// Under the crosshair! Give it a high number, but subtract distance from the score in case multiple things are under the crosshair.
			return ( 10000 - vTracePos.DistTo( pMyProp->GetCollisionOrigin() ) );
		}
	}

	if ( pEnt->Classify() == CLASS_ASW_BUTTON_PANEL )
	{
		CASW_Button_Area *pButton = static_cast< CASW_Button_Area* >( pEnt );
		if ( pButton->IsWaitingForInput() )
		{
			// Button wants to be pushed oh so badly
			return 2;
		}

		return 0;
	}

	if ( pEnt->Classify() == CLASS_ASW_COMPUTER_AREA )
	{
		CASW_Computer_Area *pComputer = static_cast< CASW_Computer_Area* >( pEnt );
		if ( pComputer->IsWaitingForInput() )
		{
			// Button wants to be pushed oh so badly
			return 2;
		}

		return 0;
	}

	CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC() );
	
	// check if this item is usable by a marine
	CASW_Weapon *pWeapon = dynamic_cast< CASW_Weapon* >( pEnt );
	if ( pWeapon )
	{
		if ( pWeapon->AllowedToPickup( pMarine ) )
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}

	// check if this item is usable by a marine
	CASW_Pickup *pPickup = dynamic_cast< CASW_Pickup* >( pEnt );
	if ( pPickup )
	{
		if ( pPickup->AllowedToPickup( pMarine ) )
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}

	// check if this item is usable by a marine
	CASW_Ammo_Drop *pAmmoDrop = dynamic_cast< CASW_Ammo_Drop* >( pEnt );
	if ( pAmmoDrop )
	{
		if ( pAmmoDrop->AllowedToPickup( pMarine ) )
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}

	// if it's not usable, give it a priority of 0
	return 0;
}

CBaseCombatCharacter *CASW_Player::ActivePlayerCombatCharacter( void )
{
	CASW_Inhabitable_NPC *pNPC = GetNPC();
	if ( !pNPC )
	{
		return BaseClass::ActivePlayerCombatCharacter();
	}

	return pNPC;
}

// for switching weapons on the current marine
void CASW_Player::ASWSelectWeapon(CBaseCombatWeapon* pWeapon, int subtype)
{
	if ( !pWeapon )
		return;

	CASW_Marine* pMarine = CASW_Marine::AsMarine( GetNPC() );
	if ( !pMarine )
	{
		SelectItem(pWeapon->GetName(), subtype);
		return;
	}

#ifndef CLIENT_DLL
	m_ASWLocal.m_hAutoAimTarget.Set(NULL);
#endif

	pMarine->Weapon_Switch(pWeapon, subtype);
}

bool CASW_Player::Weapon_CanUse( CBaseCombatWeapon *pWeapon )
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC() );
	if ( !pMarine )
	{
		return false;
	}

	CASW_Weapon *pASWWeapon = static_cast< CASW_Weapon* >( pWeapon );

	return pASWWeapon->AllowedToPickup( pMarine );
}

CBaseCombatWeapon *CASW_Player::Weapon_OwnsThisType( const char *pszWeapon, int iSubType ) const
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC() );
	if ( !pMarine )
	{
		return NULL;
	}

	return pMarine->Weapon_OwnsThisType( pszWeapon, iSubType );
}

int CASW_Player::Weapon_GetSlot( const char *pszWeapon, int iSubType ) const
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC() );
	if ( !pMarine )
	{
		return -1;
	}

	return pMarine->Weapon_GetSlot( pszWeapon, iSubType );
}

// always return horizontal pitch (pitch was used for finding distance of crosshair from the centre of screen - now it's stored in roll (can return pitch safely now?))

const QAngle& CASW_Player::EyeAngles( )
{
	// revert to hl2 camera
#ifdef CLIENT_DLL
	if (asw_hl2_camera.GetBool() && engine->IsPlayingDemo())
#else
	if (asw_hl2_camera.GetBool())
#endif
	{
		return BaseClass::EyeAngles();
	}

	CAlienSwarm *pGameRules = ASWGameRules();
	if ( pGameRules && pGameRules->GetGameState() < ASW_GS_INGAME && pGameRules->m_hBriefingCamera )
		return pGameRules->m_hBriefingCamera->EyeAngles();

	static QAngle angAdjustedEyes;

#ifdef CLIENT_DLL
	if ( IsLocalPlayer ( this ) )
	{
		angAdjustedEyes = BaseClass::EyeAngles();
	}
	else
	{
		angAdjustedEyes = m_angEyeAngles;
	}
#else
	angAdjustedEyes = BaseClass::EyeAngles();
#endif

#ifdef CLIENT_DLL
	if ( asw_allow_detach.GetBool() && GetNPC() )
	{
		return m_angEyeAngles;
	}
#endif

	CASW_Marine *pMarine = CASW_Marine::AsMarine( GetViewNPC() );
	if ( pMarine )
	{
		angAdjustedEyes.z = 0;

		// if we're spectating a turret, use the turret's eye angles
		if ( ( pMarine->GetCommander() != this || !pMarine->IsInhabited() ) && pMarine->IsControllingTurret() )
		{
			return pMarine->GetRemoteTurret()->EyeAngles();
		}

#ifdef GAME_DLL
		IASW_Vehicle *pVehicle = pMarine->GetASWVehicle();
#else
		IASW_Client_Vehicle *pVehicle = pMarine->GetASWVehicle();
#endif
		if ( !asw_allow_detach.GetBool() && pVehicle && !asw_vehicle_cam_shift_2_enable.GetBool() && GetASWControls() == ASWC_THIRDPERSONSHOULDER )
		{
			Vector vehicleOrigin;
			QAngle vehicleAngle;
			pVehicle->ASWGetSeatPosition( pMarine->m_iVehicleSeat, vehicleOrigin, vehicleAngle );
			angAdjustedEyes[YAW] = vehicleAngle[YAW];
		}
	}

	return angAdjustedEyes;
}

const QAngle& CASW_Player::EyeAnglesWithCursorRoll( )
{
	// cursor roll for distance only makes sense in top-down view
	Assert( GetASWControls() == ASWC_TOPDOWN );

	static QAngle angCursorEyes;
#ifdef CLIENT_DLL
	if ( IsLocalPlayer ( this ) )
	{
		angCursorEyes = BaseClass::EyeAngles();
	}
	else
	{
		angCursorEyes = m_angEyeAngles;
	}	
#else
	angCursorEyes = BaseClass::EyeAngles();
#endif
	return angCursorEyes;
}

void CASW_Player::AvoidPhysicsProps(CUserCmd *pCmd)
{
	CASW_Marine *pMarine = CASW_Marine::AsMarine( GetNPC() );
	if ( pMarine && !pMarine->IsInVehicle() )
		pMarine->AvoidPhysicsProps( pCmd );
}

void CASW_Player::SetAnimation( PLAYER_ANIM playerAnim )
{
	// playeranim state manages the marine animations
	return;
}

Vector CASW_Player::EarPosition( void )
{
	// revert to hl2 camera
#ifdef CLIENT_DLL
	if (asw_hl2_camera.GetBool() && engine->IsPlayingDemo())
#else
	if (asw_hl2_camera.GetBool())
#endif
	{
		return BaseClass::EarPosition();
	}

	CBaseEntity *pViewEnt = GetViewEntity();
	if ( pViewEnt )
		return pViewEnt->EarPosition();

	CAlienSwarm *pGameRules = ASWGameRules();
	if ( pGameRules && pGameRules->GetGameState() < ASW_GS_INGAME && pGameRules->m_hBriefingCamera )
		return pGameRules->m_hBriefingCamera->EarPosition();

	CASW_Inhabitable_NPC *pNPC = GetViewNPC();
	if ( pNPC )
		return pNPC->EarPosition();

	if ( asw_rts_controls.GetBool() )
	{
		return GetAbsOrigin();
	}

	return BaseClass::EarPosition();
}

bool CASW_Player::HasLiveMarines()
{
	if (!ASWGameRules())
		return false;
	CASW_Game_Resource* pGameResource = ASWGameResource();
	if (!pGameResource)
		return false;

	// loop through all marines, if we find a live one, bail
	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
		if (!pMR || pMR->GetHealthPercent() <= 0)
			continue;
		if (pMR->GetCommander() == this)
			return true;
	}
	return false;
}

bool CASW_Player::IsAlive( void )
{
	return HasLiveMarines();
}

void CASW_Player::PlayerUse()
{
	// Was use pressed or released?
	if ( ! ((m_nButtons | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
		return;

	CASW_Inhabitable_NPC *pNPC = GetNPC();
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );

	if ( !pNPC || pNPC->GetHealth() <= 0 )
		return;

	if ( pMarine && pMarine->m_bKnockedOut )
		return;

	if ( GetNumUseEntities() > 0 )
	{		
		CBaseEntity *pEnt = GetUseEntity( 0 );
		if ( pEnt )
		{
			if ( m_afButtonPressed & IN_USE )
			{
				m_flUseKeyDownTime = gpGlobals->curtime;
				m_hUseKeyDownEnt = pEnt;
			}

			CBaseEntity *pActivateEnt = NULL;
			int nHoldType = ASW_USE_RELEASE_QUICK;
	
			if ( m_nButtons & IN_USE )
			{
				float flUseHoldTime = ASW_USE_KEY_HOLD_SENTRY_TIME;

				if ( pEnt->Classify() == CLASS_ASW_MARINE )
				{
					CASW_Marine *pUsableMarine = assert_cast< CASW_Marine * >( pEnt );
					if ( pUsableMarine->m_bKnockedOut )
						flUseHoldTime = rd_revive_duration.GetFloat();
				}

				if ( pEnt->Classify() == CLASS_ASW_BUTTON_PANEL )
				{
					CASW_Button_Area *pButtonArea = assert_cast< CASW_Button_Area * >( pEnt );
					if ( pButtonArea->m_flHoldTime > 0 )
						flUseHoldTime = pButtonArea->m_flHoldTime;
				}

#ifdef RD_7A_WEAPONS
				if ( pEnt->Classify() == CLASS_ASW_REVIVE_TOOL_MARKER )
				{
					flUseHoldTime = rd_revive_tombstone_hold_duration.GetFloat();
				}
#endif

				if ( ( gpGlobals->curtime - m_flUseKeyDownTime ) >= flUseHoldTime )
				{
					if ( pEnt == m_hUseKeyDownEnt.Get() )
					{
						pActivateEnt = m_hUseKeyDownEnt.Get();
						nHoldType = ASW_USE_HOLD_RELEASE_FULL;
					}
				}
				else if ( ( gpGlobals->curtime - m_flUseKeyDownTime ) >= 0.2f )
				{
					pActivateEnt = m_hUseKeyDownEnt.Get();
					nHoldType = ASW_USE_HOLD_START;
				}
			}

			if ( m_afButtonReleased & IN_USE )
			{
				if ( m_hUseKeyDownEnt.Get() == pEnt && nHoldType != ASW_USE_HOLD_RELEASE_FULL )
				{
					pActivateEnt = pEnt;
					nHoldType = ASW_USE_RELEASE_QUICK;
				}
			}

			if ( pMarine && pActivateEnt )
			{
				bool bCanTake = true;

				CASW_Pickup *pItem = dynamic_cast<CASW_Pickup*>( pActivateEnt );
				if ( pItem && nHoldType != ASW_USE_HOLD_START )
				{
					bCanTake = pItem->AllowedToPickup( pMarine );
					if ( bCanTake )
					{
						pMarine->SetStopTime( gpGlobals->curtime + 1.0f );
						pMarine->DoAnimationEvent( PLAYERANIMEVENT_PICKUP );
					}
				}
				else
				{
					CASW_Weapon *pWeapon = dynamic_cast< CASW_Weapon * >( pActivateEnt );
					if ( pWeapon && nHoldType != ASW_USE_HOLD_START )
					{
						bCanTake = pWeapon->AllowedToPickup( pMarine );
						if ( bCanTake )
						{
							pMarine->SetStopTime( gpGlobals->curtime + 1.0f );
							pMarine->DoAnimationEvent( PLAYERANIMEVENT_PICKUP );
						}
					}
					else
					{
						CASW_Ammo_Drop *pAmmoDrop = dynamic_cast< CASW_Ammo_Drop * >( pActivateEnt );
						if ( pAmmoDrop && nHoldType != ASW_USE_HOLD_START )
						{
							bCanTake = pAmmoDrop->AllowedToPickup( pMarine );
							if ( bCanTake )
							{
								pMarine->SetStopTime( gpGlobals->curtime + 1.0f );
								pMarine->DoAnimationEvent( PLAYERANIMEVENT_PICKUP );
								CASW_Weapon *pAmmoWeapon = pAmmoDrop->GetAmmoUseUnits( pMarine );

								if ( pAmmoWeapon )
								{
									int iAmmoType = pAmmoWeapon->GetPrimaryAmmoType();
									int iAmmoCost = pAmmoDrop->GetAmmoUnitCost( iAmmoType );
									if ( iAmmoCost < 20 )
									{
										EmitSound( "ASW_Ammobag.Pickup_sml" );
										DispatchParticleEffect( "ammo_satchel_take_sml", pAmmoDrop->GetAbsOrigin() + Vector( 0, 0, 4 ), vec3_angle );
									}
									else if ( iAmmoCost < 75 )
									{
										EmitSound( "ASW_Ammobag.Pickup_med" );
										DispatchParticleEffect( "ammo_satchel_take_med", pAmmoDrop->GetAbsOrigin() + Vector( 0, 0, 4 ), vec3_angle );
									}
									else
									{
										EmitSound( "ASW_Ammobag.Pickup_lrg" );
										DispatchParticleEffect( "ammo_satchel_take_lrg", pAmmoDrop->GetAbsOrigin() + Vector( 0, 0, 4 ), vec3_angle );
									}
								}
							}
						}
					}
				}
			}

#ifdef GAME_DLL
			IASW_Server_Usable_Entity *pUsable = dynamic_cast< IASW_Server_Usable_Entity * >( pActivateEnt );
			if ( pUsable )
			{
				if ( !pNPC->m_hUsingEntity.Get() || pNPC->m_hUsingEntity.Get() == pActivateEnt )		// if we're in the middle of using an entity, only allow reusing that same entity
				{
					pUsable->ActivateUseIcon( pNPC, nHoldType );
				}
				//Mad Orange. This is a litte hacky but fixes ghost weapons may appear in inventory and other use area related issues like fail revives with marines when u press use button for a long time with more than 1 use item nearby.
				if ( nHoldType == ASW_USE_HOLD_RELEASE_FULL )
				{
					m_hUseKeyDownEnt = NULL;
					m_flUseKeyDownTime = 0.0f;
				}
			}
#endif
		}
	}

	if ( m_afButtonReleased & IN_USE )
	{
		m_hUseKeyDownEnt = NULL;
		m_flUseKeyDownTime = 0.0f;
	}
	
	if ( m_hUseKeyDownEnt.Get() && GetNumUseEntities() < 1 )
	{
		m_hUseKeyDownEnt = NULL;
		m_flUseKeyDownTime = 0.0f;
	}
}

void CASW_Player::StartWalking( void )
{
	SetMaxSpeed( ASW_MOVEMENT_WALK_SPEED );
	m_fIsWalking = true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CASW_Player::StopWalking( void )
{
	SetMaxSpeed( ASW_MOVEMENT_NORM_SPEED );
	m_fIsWalking = false;
}

// as HL2, but no sprinting
void CASW_Player::HandleSpeedChanges( void )
{
	int buttonsChanged = m_afButtonPressed | m_afButtonReleased;

	if( buttonsChanged & IN_WALK )
	{
		// The state of the WALK button has changed. 
		if( IsWalking() && !(m_afButtonPressed & IN_WALK) )
		{
			StopWalking();
		}
		else if( !IsWalking() && (m_afButtonPressed & IN_WALK) && !(m_nButtons & IN_DUCK) )
		{
			StartWalking();
		}
	}

	if ( IsWalking() && !(m_nButtons & IN_WALK)  ) 
		StopWalking();
}

CBaseEntity* CASW_Player::GetSoundscapeListener()
{
	if ( GetViewEntity() )
		return GetViewEntity();

	if ( GetViewNPC() )
		return GetViewNPC();

	return this;
}

// BenLubar: for code ported from Swarm Director 2, include this method just in case Reactive Drop ever has per-player asw_controls support.
ASW_Controls_t CASW_Player::GetASWControls()
{
	// if a camera is controlling our vision, we're in first person.
	if ( GetViewEntity() )
		return ASWC_FIRSTPERSON;

	// briefing camera controls vision during briefing.
	CAlienSwarm *pGameRules = ASWGameRules();
	if ( pGameRules && pGameRules->GetGameState() < ASW_GS_INGAME && pGameRules->m_hBriefingCamera )
		return ASWC_FIRSTPERSON;

#ifdef CLIENT_DLL
	// if we're in a death cam, switch to third person temporarily.
	if ( pGameRules && pGameRules->GetMarineDeathCamInterp() )
		return ASWC_TOPDOWN;

	// if we're forcing a spectator override, everyone's controls change client-side while spectating.
	C_ASW_Player *pLocal = C_ASW_Player::GetLocalASWPlayer();
	if ( asw_controls_spectator_override.GetInt() >= 0 && ( ( pLocal && pLocal->GetSpectatingNPC() ) || engine->IsPlayingDemo() ) )
		return ( ASW_Controls_t )asw_controls_spectator_override.GetInt();
#endif

	CASW_Inhabitable_NPC *pNPC = GetViewNPC();

	// if we're in a vehicle, see if the vehicle wants to change the asw_controls setting.
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( pMarine && pMarine->IsInVehicle() )
	{
		int nControls = asw_controls_vehicle.GetInt();
#ifdef GAME_DLL
		if ( IASW_Vehicle *pVehicle = pMarine->GetASWVehicle() )
#else
		if ( IASW_Client_Vehicle *pVehicle = pMarine->GetASWVehicle() )
#endif
		{
			pVehicle->ASWGetCameraOverrides( &nControls, NULL, NULL, NULL );
		}

		if ( nControls >= 0 )
		{
			return ( ASW_Controls_t )nControls;
		}
	}

	// if we have a character, use their controls.
	if ( pNPC )
	{
		return pNPC->GetASWControls();
	}

	// otherwise, use the global controls setting.
	return ( ASW_Controls_t )asw_controls.GetInt();
}

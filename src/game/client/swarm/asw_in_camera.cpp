#include "cbase.h"
#include "asw_util_shared.h"
#include "c_asw_camera_volume.h"
#include "c_asw_marine.h"
#include "c_asw_player.h"
#include "asw_gamerules.h"
#include "asw_input.h"
#include "missionchooser/iasw_random_missions.h"
#include "holdout_resupply_frame.h"
#include "igameevents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static bool s_bDontSendDesiredYaw = false;
static void SetDesiredCameraYaw( IConVar *pVar, const char *szOldValue, float flOldValue );

// Marine Camera ConVars.
extern ConVar asw_cam_marine_pitch;
extern ConVar asw_cam_marine_dist;
ConVar asw_cam_marine_pitch_rate( "asw_cam_marine_pitch_rate", "1000", FCVAR_CHEAT ); // asw setting
ConVar asw_cam_marine_yaw( "asw_cam_marine_yaw", "90", FCVAR_CHEAT, "Marine Camera: yaw." );
ConVar rd_cam_marine_yaw_desired( "rd_cam_marine_yaw_desired", "90", FCVAR_CHEAT, "The desired value of yaw. Camera yaw will linearly blend to the desired value", SetDesiredCameraYaw );
ConVar rd_cam_marine_yaw_rate( "rd_cam_marine_yaw_rate", "0.1", FCVAR_CHEAT, "Time in seconds needed to change camera yaw" );
ConVar asw_cam_marine_dist_rate( "asw_cam_marine_dist_rate", "50", FCVAR_CHEAT, "Marine Camera: Distance from marine." );

ConVar asw_cam_marine_dist_death( "asw_cam_marine_dist_death", "200", FCVAR_CHEAT, "Marine Camera: Distance from marine as he dies." );
ConVar asw_cam_marine_pitch_death( "asw_cam_marine_pitch_death", "50", FCVAR_CHEAT, "Marine Camera: pitch when he dies." );
ConVar asw_cam_marine_yaw_death_rate( "asw_cam_marine_yaw_death_rate", "35.0", FCVAR_CHEAT, "Marine Camera: yaw rotate rate when he dies." );
ConVar asw_cam_marine_shift_z_death( "asw_cam_marine_shift_z_death", "-30.0", FCVAR_CHEAT, "Marine Camera: Shift camera vertically when he dies." );

ConVar asw_cam_marine_shift_ratex( "asw_cam_marine_shift_ratex", "1000", FCVAR_CHEAT, "Marine Camera: How far the camera pans east/west as you move the mouse." );
ConVar asw_cam_marine_shift_ratey( "asw_cam_marine_shift_ratey", "650", FCVAR_CHEAT, "Marine Camera: How far the camera pans north as you move the mouse." );
ConVar asw_cam_marine_shift_ratey_south( "asw_cam_marine_shift_ratey_south", "2000", FCVAR_CHEAT, "Marine Camera: How far the camera pans south as you move the mouse." );
ConVar asw_cam_marine_shift_maxx( "asw_cam_marine_shift_maxx", "300", FCVAR_CHEAT, "Marine Camera: How far the camera pans east/west as you move the mouse." );
ConVar asw_cam_marine_shift_maxy( "asw_cam_marine_shift_maxy", "200", FCVAR_CHEAT, "Marine Camera: How far the camera pans north as you move the mouse." );
ConVar asw_cam_marine_shift_maxy_south( "asw_cam_marine_shift_maxy_south", "380", FCVAR_CHEAT, "Marine Camera: How far the camera pans south as you move the mouse." );
ConVar asw_cam_marine_shift_deadspace( "asw_cam_marine_shift_deadspace", "64", FCVAR_CHEAT, "Marine Camera: Deadspace around the marine before camera shifting starts." );
ConVar asw_cam_marine_blend( "asw_cam_marine_blend", "1", FCVAR_CHEAT, "Marine Camera: Whether camera should blend Z movement changes.");

ConVar asw_cam_marine_test( "asw_cam_marine_test", "1", FCVAR_CHEAT, "Camera Test." );
ConVar asw_cam_marine_sphere_min( "asw_cam_marine_sphere_min", "32", FCVAR_CHEAT, "Test" );
ConVar asw_cam_marine_sphere_max( "asw_cam_marine_sphere_max", "400", FCVAR_CHEAT, "Test" );

ConVar asw_cam_marine_spring_vel_max( "asw_cam_marine_spring_vel_max", "35.0", FCVAR_CHEAT, "Camera max velocity." );
ConVar asw_cam_marine_spring_const( "asw_cam_marine_spring_const", "0.75", FCVAR_CHEAT, "Camera spring constant." );
ConVar asw_cam_marine_spring_dampening( "asw_cam_marine_spring_dampening", "3.0", FCVAR_CHEAT, "Camera spring dampening." );

ConVar asw_cam_marine_yshift_static( "asw_cam_marine_yshift_static", "75.0f", FCVAR_CHEAT, "Camera y-shift value." );

ConVar asw_cam_marine_shift_enable( "asw_cam_marine_shift_enable", "1", FCVAR_CHEAT, "Camera shifting enable/disable." );

// Vehicle Camera ConVars.
ConVar asw_vehicle_cam_height( "asw_vehicle_cam_height", "0", FCVAR_CHEAT );
ConVar asw_vehicle_cam_pitch( "asw_vehicle_cam_pitch", "45", FCVAR_CHEAT );
ConVar asw_vehicle_cam_dist( "asw_vehicle_cam_dist", "412", FCVAR_CHEAT );

// ASWTODO - allow thirdperson but cheat protect first person
//static ConCommand thirdperson( "thirdperson", ::CAM_ToThirdPerson, "Switch to thirdperson camera." );
//static ConCommand firstperson( "firstperson", ::CAM_ToFirstPerson, "Switch to firstperson camera.", FCVAR_CHEAT );

extern kbutton_t in_zoom;
extern ConVar asw_hide_local_marine;
extern ConVar cam_command;
extern ConVar joy_pan_camera;

#define	DIST	 2

// Camera height distances!
float s_flCameraHeights[ASW_TILETYPE_COUNT] = 
{ 
  500.0f,			// Unknown
  600.0f,			// Outdoor1
  800.0f,			// Outdoor2
  550.0f,			// Arena1
  650.0f,			// Arena2
  750.0f,			// Arena3
  450.0f,			// Room1
  550.0f,			// Room2
  300.0f,			// Corridor1
  400.0f,			// Corridor1
  250.0f			// Vents
};

float s_flCameraHeights55[ASW_TILETYPE_COUNT] = 
{
	900.0f,			// Unknown
	1080.0f,		// Outdoor1
	1440.0f,		// Outdoor2
	990.0f,			// Arena1
	1170.0f,		// Arena2
	1350.0f,		// Arena3
	810.0f,			// Room1
	990.0f,			// Room2
	540.0f,			// Corridor1
	720.0f,			// Corridor1
	450.0f			// Vents
};
// Panning to much when pulled further out



//-----------------------------------------------------------------------------
// Purpose: Alien Swarm camera pitch.
//-----------------------------------------------------------------------------
float CASWInput::ASW_GetCameraPitch( const float *pfDeathCamInterp /*= NULL*/ )
{
	// Get the given pitch.
	float flPitch = asw_cam_marine_pitch.GetFloat();

	float fDeathCamInterp;
	if ( pfDeathCamInterp )
	{
		fDeathCamInterp = *pfDeathCamInterp;
	}
	else
	{
		fDeathCamInterp = ( ASWGameRules() ? ASWGameRules()->GetMarineDeathCamInterp() : 0.0f );
	}

	if ( fDeathCamInterp > 0.0f )
	{
		flPitch = ( 1.0f - fDeathCamInterp ) * flPitch + fDeathCamInterp * asw_cam_marine_pitch_death.GetFloat();
	}

	// Check to see if we are in a camera volume.
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	C_ASW_Inhabitable_NPC *pNPC = pPlayer ? pPlayer->GetViewNPC() : NULL;
	if ( pNPC )
	{
		float fCameraVolumePitch = C_ASW_Camera_Volume::IsPointInCameraVolume( pNPC->GetAbsOrigin() );
		if ( fCameraVolumePitch != -1 )
		{
			flPitch = fCameraVolumePitch;
		}
	}

	if ( m_fCurrentCameraPitch != flPitch )
	{
		float flDelta = MIN( 0.2f, gpGlobals->frametime );
		m_fCurrentCameraPitch = ASW_ClampYaw( asw_cam_marine_pitch_rate.GetFloat(), m_fCurrentCameraPitch, flPitch, flDelta );
	}

	return m_fCurrentCameraPitch;
}


//-----------------------------------------------------------------------------
// Purpose: Alien Swarm camera yaw.
//-----------------------------------------------------------------------------
float CASWInput::ASW_GetCameraYaw( const float *pfDeathCamInterp /*= NULL*/ )
{
	float fDeathCamInterp;
	if ( pfDeathCamInterp )
	{
		fDeathCamInterp = *pfDeathCamInterp;
	}
	else
	{
		fDeathCamInterp = ( ASWGameRules() ? ASWGameRules()->GetMarineDeathCamInterp() : 0.0f );
	}

	if ( fDeathCamInterp > 0.0f )
	{
		float fRotate = gpGlobals->curtime * asw_cam_marine_yaw_death_rate.GetFloat() + ASWGameRules()->m_fDeathCamYawAngleOffset;
		float fFullRotations = static_cast< int >( fRotate / 360.0f );
		fRotate -= fFullRotations * 360.0f;

		fRotate = AngleNormalize( fRotate );

		return ( 1.0f - fDeathCamInterp ) * asw_cam_marine_yaw.GetFloat() + fDeathCamInterp * ( asw_cam_marine_yaw.GetFloat() + fRotate );
	}

	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	// BenLubar: When spectating a player, use their camera yaw.
	C_ASW_Inhabitable_NPC *pSpectatingNPC = pPlayer ? pPlayer->GetSpectatingNPC() : NULL;
	if ( pSpectatingNPC && pSpectatingNPC->IsInhabited() )
	{
		pPlayer = pSpectatingNPC->GetCommander();
	}

	if ( pPlayer && pPlayer->GetASWControls() != ASWC_TOPDOWN )
	{
		if ( pSpectatingNPC && !pSpectatingNPC->IsInhabited() )
		{
			return pSpectatingNPC->EyeAngles().y;
		}

		return pPlayer->EyeAngles().y;
	}

	if ( pPlayer && asw_cam_marine_yaw.GetFloat() != pPlayer->m_flMovementAxisYaw )
	{
		s_bDontSendDesiredYaw = true;
		rd_cam_marine_yaw_desired.SetValue( pPlayer->m_flMovementAxisYaw );
		s_bDontSendDesiredYaw = false;

		if ( !m_fCamYawRotStartTime )
			m_fCamYawRotStartTime = gpGlobals->realtime;

		float fCamYawRotEndTime = m_fCamYawRotStartTime + rd_cam_marine_yaw_rate.GetFloat();

		if ( gpGlobals->realtime >= fCamYawRotEndTime ) 
		{
			// finish rotation
			asw_cam_marine_yaw.SetValue(pPlayer->m_flMovementAxisYaw);
			m_fCamYawRotStartTime = 0.0f;
		}
		else
		{
			// a value from 0 to 1
			float fRotProgress = (gpGlobals->realtime - m_fCamYawRotStartTime) / (fCamYawRotEndTime - m_fCamYawRotStartTime);
			float fTotalDegreesToChange = 0.0f;

			// handle special situations for 0 and 360 angles
			if ( asw_cam_marine_yaw.GetFloat() == 270.0f && pPlayer->m_flMovementAxisYaw == 0.0f )
			{
				fTotalDegreesToChange = 90.0f;
			}
			else if ( asw_cam_marine_yaw.GetFloat() == 0.0f && pPlayer->m_flMovementAxisYaw == 270.0f )
			{
				fTotalDegreesToChange = -90.0f;
			}
			else
			{
				fTotalDegreesToChange = pPlayer->m_flMovementAxisYaw - asw_cam_marine_yaw.GetFloat();
			}

			float fNewYaw = asw_cam_marine_yaw.GetFloat() + fTotalDegreesToChange * fRotProgress;

			return fNewYaw;
		}
	}

	return asw_cam_marine_yaw.GetFloat();
}


extern ConVar fov_desired;
//-----------------------------------------------------------------------------
// Purpose: Alien Swarm camera distance.
//-----------------------------------------------------------------------------
float CASWInput::ASW_GetCameraDist( const float *pfDeathCamInterp /*= NULL*/ )
{
#ifdef VARIABLE_CAMERA
	// Are we using a valid FOV, if not use the default distance.
	float flFOV = fov_desired.GetFloat();
	if ( !( flFOV == 75.0f || flFOV == 50.0f  ) )
		return asw_cam_marine_dist.GetFloat();

	// Do we have a missionchooser, if not use the default distance.
	if ( !missionchooser || !missionchooser->RandomMissions() )
		return asw_cam_marine_dist.GetFloat();

	// Do we have a valid player and marine, if not use the default distance.
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pPlayer || !pPlayer->GetViewMarine() )
		return asw_cam_marine_dist.GetFloat();

	// Do we have a valid room, if not use the default distance.
	IASW_Room_Details *pRoom = missionchooser->RandomMissions()->GetRoomDetails( pPlayer->GetViewMarine()->GetAbsOrigin() );
	if ( !pRoom )
		return asw_cam_marine_dist.GetFloat();
	
	// Get the desired distance for the room.
	float flDesiredDist = s_flCameraHeights[pRoom->GetTileType()];
	if ( flFOV == 50 )
	{
		flDesiredDist = s_flCameraHeights55[pRoom->GetTileType()];
	}

	float flCameraDelta = fabs( m_flCurrentCameraDist - flDesiredDist );
	// Check against a tolerance so we don't oscillate forever.
	if ( flCameraDelta > 0.2f )
	{
		// Get frametime = delta time
		float flFrameTime = gpGlobals->frametime;

		float flAccel = fabs( flDesiredDist - m_flCurrentCameraDist ) * asw_cam_marine_spring_const.GetFloat();
		float flZDampening = asw_cam_marine_spring_dampening.GetFloat() * m_vecCameraVelocity.z;
		flAccel -= flZDampening;
		float flZVelocity = flAccel * flFrameTime;
		m_vecCameraVelocity.z += flZVelocity;
		m_vecCameraVelocity.z = clamp( m_vecCameraVelocity.z, 0.1f, asw_cam_marine_spring_vel_max.GetFloat() );
		if ( m_flCurrentCameraDist < flDesiredDist )
		{
			m_flCurrentCameraDist += m_vecCameraVelocity.z * flFrameTime;
		}
		else
		{
			m_flCurrentCameraDist -= m_vecCameraVelocity.z * flFrameTime;
		}
	}
	else
	{
		m_vecCameraVelocity.z = 0.0f;
		m_flCurrentCameraDist = flDesiredDist;
	}

	return m_flCurrentCameraDist;
#else
	float fDeathCamInterp;
	if ( pfDeathCamInterp )
	{
		fDeathCamInterp = *pfDeathCamInterp;
	}
	else
	{
		fDeathCamInterp = ( ASWGameRules() ? ASWGameRules()->GetMarineDeathCamInterp() : 0.0f );
	}

	if ( fDeathCamInterp > 0.0f )
	{
		return ( 1.0f - fDeathCamInterp ) * asw_cam_marine_dist.GetFloat() + fDeathCamInterp * asw_cam_marine_dist_death.GetFloat();
	}

	return asw_cam_marine_dist.GetFloat();
#endif
}


extern void UpdateOrderArrow();

void CASWInput::CAM_Think( void )
{
	Assert( engine->IsLocalPlayerResolvable() );

	UpdateOrderArrow();	// update the arrow direction if we're in the middle of ordering a marine (see in_main.cpp)

	switch( GetPerUser().m_nCamCommand )
	{
	case CAM_COMMAND_TOTHIRDPERSON:
		CAM_ToThirdPerson();
		break;

	case CAM_COMMAND_TOFIRSTPERSON:
		CAM_ToFirstPerson();
		break;

	case CAM_COMMAND_NONE:
	default:
		break;
	}

	if( !GetPerUser().m_fCameraInThirdPerson )
		return;

	GetPerUser().m_vecCameraOffset[ PITCH ] = ASW_GetCameraPitch();
	GetPerUser().m_vecCameraOffset[ YAW ]   = ASW_GetCameraYaw();
	GetPerUser().m_vecCameraOffset[ DIST ]  = 0;
}

void CASWInput::CAM_ToThirdPerson(void)
{
	asw_hide_local_marine.SetValue(0);
	CInput::CAM_ToThirdPerson();
}

void CASWInput::CAM_ToFirstPerson(void)
{
	asw_hide_local_marine.SetValue(1);
	CInput::CAM_ToFirstPerson();
}

extern int g_asw_iPlayerListOpen;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CASWInput::CalculateCameraShift( C_ASW_Player *pPlayer, float flDeltaX, float flDeltaY, float &flShiftX, float &flShiftY )
{
	// Init.
	flShiftX = 0.0f;
	flShiftY = 0.0f;

	if ( !asw_cam_marine_shift_enable.GetBool() )
		return;

	if ( m_bCameraFixed || Holdout_Resupply_Frame::HasResupplyFrameOpen() || g_asw_iPlayerListOpen > 0 || ( pPlayer && pPlayer->GetSpectatingNPC() && !pPlayer->GetSpectatingNPC()->IsInhabited() ) )
	{
		m_fShiftFraction = Approach( 0.0f, m_fShiftFraction, gpGlobals->frametime );
	}
	else
	{
		m_fShiftFraction = Approach( 1.0f, m_fShiftFraction, gpGlobals->frametime );
	}

	if ( ASWGameRules() )
	{
		m_fShiftFraction = m_fShiftFraction * ( 1.0f - ASWGameRules()->GetMarineDeathCamInterp() );
	}

	flShiftX = flDeltaX * asw_cam_marine_shift_maxx.GetFloat() * m_fShiftFraction;
	float camshifty = (flDeltaY < 0) ? asw_cam_marine_shift_maxy.GetFloat() : asw_cam_marine_shift_maxy_south.GetFloat();
	flShiftY = flDeltaY * camshifty * m_fShiftFraction;


	return;

	// Calculate the shift, spherically, based on the cursor distance from the player.
	float flDistance = FastSqrt( flDeltaX * flDeltaX + flDeltaY * flDeltaY );
	if ( flDistance > asw_cam_marine_sphere_min.GetFloat() )
	{
		flDistance -= asw_cam_marine_sphere_min.GetFloat();

		float flRatio = 1.0f;
		if ( m_flCurrentCameraDist < asw_cam_marine_dist.GetFloat() )
		{
			flRatio = ( m_flCurrentCameraDist / asw_cam_marine_dist.GetFloat() ) * 0.8f;
		}

		float flTemp = flDistance / ( asw_cam_marine_sphere_max.GetFloat() * flRatio );
		flTemp = clamp( flTemp, 0.0f, 1.0f );

		float flAngle = atan2( (float)flDeltaY, (float)flDeltaX );
		flShiftX = cos( flAngle ) * flTemp * ( asw_cam_marine_shift_maxx.GetFloat() * flRatio );
		if ( flDeltaY < 0 )
		{
			flShiftY = sin( flAngle ) * flTemp * ( asw_cam_marine_shift_maxy.GetFloat() * flRatio );
		}
		else
		{
			flShiftY = sin( flAngle ) * flTemp * ( asw_cam_marine_shift_maxy_south.GetFloat() * flRatio );
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CASWInput::SmoothCamera( C_ASW_Player *pPlayer, Vector &vecCameraLocation )
{
	// Override smoothing enabled.
	if ( !asw_cam_marine_blend.GetBool() )
		return;

	// Apply smoothing from the previous position if we did change marine
	if ( !pPlayer->SmoothMarineChangeCamera( vecCameraLocation ) )
	{
		// Apply Z smoothing to the camera if we didn't change marine.
		pPlayer->SmoothCameraZ( vecCameraLocation );
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CASWInput::ASW_GetCameraLocation( C_ASW_Player *pPlayer, Vector &vecCameraLocation, QAngle &angCamera, int &nMouseX, int &nMouseY, bool bApplySmoothing )
{
	// Verify data.
	Assert( pPlayer != NULL );
	if ( !pPlayer )
		return;

	Assert( ASWInput() != NULL );
	if ( !ASWInput() )
		return;

	// If we've already calculated the camera position on this frame, then just return the previous result.
// 	if ( pPlayer->m_nLastCameraFrame == gpGlobals->framecount )
// 	{
// 		vecCameraLocation = pPlayer->m_vecLastCameraPosition;
// 		angCamera = pPlayer->m_angLastCamera;
// 		return;
// 	}

	// Get the current camera position.
	vecCameraLocation = pPlayer->EyePosition();

	// Get the camera angles and calculate the camera view directions.
	Vector vecCameraDirection;
	::input->CAM_GetCameraOffset( vecCameraDirection );

	angCamera[PITCH] = vecCameraDirection[PITCH];
	angCamera[YAW] = vecCameraDirection[YAW];
	angCamera[ROLL] = 0;

	Vector vecCamForward, vecCamRight, vecCamUp;
	AngleVectors( angCamera, &vecCamForward, &vecCamRight, &vecCamUp );

	// Get the window center.
	int nCenterX, nCenterY;
	ASWInput()->ASW_GetWindowCenter( nCenterX, nCenterY );

	// Get the position change.
	ASWInput()->GetSimulatedFullscreenMousePos( &nMouseX, &nMouseY );

	// Calculate the movement delta - only needed for mouse control or controller with pan enabled.
	int nDeltaX = 0;
	int nDeltaY = 0;
	if ( !ASWInput()->ControllerModeActive() || joy_pan_camera.GetBool() )
	{
		nDeltaX = nMouseX - nCenterX;
		nDeltaY = nMouseY - nCenterY;

		// Calculate the camera shift and move the camera.
		float flShiftX, flShiftY;
		CalculateCameraShift( pPlayer, (float) nDeltaX / ( nCenterX * 2.0f ), (float) nDeltaY / ( nCenterY * 2.0f ), flShiftX, flShiftY );

		VectorMA( vecCameraLocation, flShiftX, vecCamRight, vecCameraLocation );
		vecCamUp.z = 0;	// don't want the camera changing z
		vecCamUp.NormalizeInPlace();
		VectorMA( vecCameraLocation, -flShiftY, vecCamUp, vecCameraLocation );
	}

	bool bDeathcam = ASWGameRules() && ( ASWGameRules()->GetMarineDeathCamInterp() > 0.0f );
	
	// Smooth the camera movement.
	if ( bApplySmoothing && !bDeathcam )
	{
		SmoothCamera( pPlayer, vecCameraLocation );
		pPlayer->m_vecLastCameraPosition = vecCameraLocation;
	}

	// Update the player camera data.
	pPlayer->m_nLastCameraFrame = gpGlobals->framecount;
	pPlayer->m_angLastCamera = angCamera;

	if ( ASWDeathmatchMode() && pPlayer->m_hLastNPC == NULL && !pPlayer->GetSpectatingNPC() && pPlayer->GetNPC() )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( "general_hint" );
		if ( event )
		{
			gameeventmanager->FireEventClientSide( event );
		}
	}
	pPlayer->m_hLastNPC = pPlayer->GetViewNPC();
}



/*
==============================
CAM_StartMouseMove

==============================
*/
void CASWInput::CAM_StartMouseMove(void)
{
	GetPerUser().m_fCameraMovingWithMouse=false;
	GetPerUser().m_fCameraInterceptingMouse=false;
}

/*
==============================
CAM_StartDistance

routines to start the process of moving the cam in or out 
using the mouse
==============================
*/
void CASWInput::CAM_StartDistance(void)
{
	// asw
	GetPerUser().m_fCameraDistanceMove=false;
	GetPerUser().m_fCameraMovingWithMouse=false;
	GetPerUser().m_fCameraInterceptingMouse=false;
}


/*
==============================
Init_Camera

==============================
*/
void CASWInput::Init_Camera( void )
{
	for ( int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i )
	{
		m_PerUser[ i ].m_fCameraInThirdPerson = true;
		m_PerUser[ i ].m_CameraIsOrthographic = false;
		// TODO: make this part of the per user data
		m_fCurrentCameraPitch = 0.0f;
		m_flCurrentCameraDist = asw_cam_marine_dist.GetFloat();
		m_vecCameraVelocity.Init();

		m_fShiftFraction = 1.0f;
		m_bCameraFixed = false;
	}
}

void CASWInput::UpdateASWControls()
{
	ASSERT_LOCAL_PLAYER_RESOLVABLE();

	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( !pPlayer || pPlayer->GetASWControls() == ASWC_TOPDOWN )
	{
		CAM_ToThirdPerson();
	}
	else
	{
		CAM_ToFirstPerson();
	}
}

// asw - sets us up for moving the camera around in demos
void ASWDemoCamera_f()
{
	if (!engine->IsPlayingDemo())
		return;

	engine->ClientCmd("firstperson");
	engine->ClientCmd("asw_hl2_camera 1");
	engine->ClientCmd("asw_controls 0");
}
ConCommand asw_demo_camera("asw_demo_camera", ASWDemoCamera_f);

static void SetDesiredCameraYaw( IConVar *pVar, const char *szOldValue, float flOldValue )
{
	if ( s_bDontSendDesiredYaw )
	{
		return;
	}

	engine->ServerCmd( VarArgs( "rotatecameraexact %f\n", rd_cam_marine_yaw_desired.GetFloat() ) );
}

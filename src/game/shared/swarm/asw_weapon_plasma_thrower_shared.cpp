#include "cbase.h"
#include "asw_weapon_plasma_thrower_shared.h"
#ifdef CLIENT_DLL
#include "c_asw_marine.h"
#include "c_asw_player.h"
#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
#else
#include "asw_fail_advice.h"
#include "asw_lag_compensation.h"
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_player.h"
#include "EntityFlame.h"
#endif
#include "asw_marine_skills.h"
#include "asw_trace_filter.h"
#include "asw_util_shared.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
ConVar rd_plasma_thrower_airblast_push( "rd_plasma_thrower_airblast_push", "1000", FCVAR_CHEAT | FCVAR_REPLICATED, "Amount of force (actually instantaneous velocity) that the airblast applies to each target" );
ConVar rd_plasma_thrower_airblast_push_ally( "rd_plasma_thrower_airblast_push_ally", "300", FCVAR_CHEAT | FCVAR_REPLICATED, "Amount of force (actually instantaneous velocity) that the airblast applies to each allied target" );
ConVar rd_plasma_thrower_airblast_push_up( "rd_plasma_thrower_airblast_push_up", "100", FCVAR_CHEAT | FCVAR_REPLICATED, "Minimum upwards velocity impulse from the airblast" );
ConVar rd_plasma_thrower_airblast_torque( "rd_plasma_thrower_airblast_torque", "10", FCVAR_CHEAT | FCVAR_REPLICATED, "Amount of spin force applied by the airblast to physics props/buzzers" );
ConVar rd_plasma_thrower_airblast_dot( "rd_plasma_thrower_airlast_dot", "0.866025403784", FCVAR_CHEAT | FCVAR_REPLICATED, "Minimum dot product for an object's position if that object gets hit.\n1.0 would be a zero degree arc, and 0.0 would be a 180 degree (90 to both sides) arc.\nDefault value is about 30 degrees to each side (60 degrees total).", true, -1, true, 1 );
ConVar rd_plasma_thrower_airblast_dot_near( "rd_plasma_thrower_airlast_dot_near", "-0.70710678119", FCVAR_CHEAT | FCVAR_REPLICATED, "Minimum dot product for an object's position if that object gets hit within the near range.", true, -1, true, 1 );
ConVar rd_plasma_thrower_airblast_range( "rd_plasma_thrower_airblast_range", "400", FCVAR_CHEAT | FCVAR_REPLICATED, "Distance from the center of the marine that the airblast can affect targets.", true, 0, false, 0 );
ConVar rd_plasma_thrower_airblast_range_near( "rd_plasma_thrower_airblast_range_near", "75", FCVAR_CHEAT | FCVAR_REPLICATED, "Hit targets within this range using the near angle.", true, 0, false, 0 );
ConVar rd_plasma_thrower_airblast_falloff( "rd_plasma_thrower_airblast_falloff", "0.1", FCVAR_CHEAT | FCVAR_REPLICATED, "Adjustment for airblast force near the maximum distance for the airblast. 1 would mean the airblast doesn't work. 0.5 means the airblast falls off linearly.", true, 0, true, 1 );
ConVar rd_plasma_thrower_airblast_ammo( "rd_plasma_thrower_airblast_ammo", "20", FCVAR_CHEAT | FCVAR_REPLICATED, "Amount of ammo required for plasma thrower alt fire", true, 0, false, 0 );
ConVar rd_plasma_thrower_cooldown_primary( "rd_plasma_thrower_cooldown_primary", "1.5", FCVAR_CHEAT | FCVAR_REPLICATED, "How long (in seconds) the plasma thrower must cool down after stopping firing before it can fire again.", true, 0, false, 0 );
ConVar rd_plasma_thrower_cooldown_secondary( "rd_plasma_thrower_cooldown_secondary", "0.75", FCVAR_CHEAT | FCVAR_REPLICATED, "How long (in seconds) the plasma thrower must wait after an airblast to fire again (overrides primary cooldown).", true, 0, false, 0 );
ConVar rd_plasma_thrower_cooldown_airblast( "rd_plasma_thrower_cooldown_airblast", "2.0", FCVAR_CHEAT | FCVAR_REPLICATED, "How long (in seconds) the plasma thrower must wait after an airblast to perform another airblast.", true, 0, false, 0 );
// Because Alien Swarm: Reactive Drop has multiple camera modes and we want muscle memory to transfer between them, recoil is handled in screen space.
// xy = screen coordinate offset (to be scaled by YRES(1))
static Vector2D s_RD_Plasma_Thrower_Recoil[128] = { vec2_invalid };
static void ResetPlasmaThrowerRecoil( IConVar *pConVar, const char *szOldValue, float flOldValue )
{
	s_RD_Plasma_Thrower_Recoil[0] = vec2_invalid;
}
ConVar rd_plasma_thrower_recoil_seed( "rd_plasma_thrower_recoil_seed", "20170420", FCVAR_CHEAT | FCVAR_REPLICATED, "RNG seed for plasma thrower recoil", ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_rampup_min( "rd_plasma_thrower_recoil_rampup_min", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "Recoil scale for the first shot. Linearly scales to 1 over the course of rampup.", true, 0, true, 1, ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_rampup_shots( "rd_plasma_thrower_recoil_rampup_shots", "0.1", FCVAR_CHEAT | FCVAR_REPLICATED, "Portion of the magazine before plasma thrower recoil is in full effect", true, 0, true, 1, ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_variance( "rd_plasma_thrower_recoil_variance", "0.6", FCVAR_CHEAT | FCVAR_REPLICATED, "Amount each recoil value is allowed to change from the previous", true, 0, true, 1, ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_angle_base( "rd_plasma_thrower_recoil_angle_base", "90 360 100 -720 60", FCVAR_CHEAT | FCVAR_REPLICATED, "Coefficients (a + bx + cx^2, etc.) for the angle of the plasma thrower's recoil after x portion of the magazine is expended in one burst", ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_angle_variance( "rd_plasma_thrower_recoil_angle_variance", "2 5 15", FCVAR_CHEAT | FCVAR_REPLICATED, "Coefficients (a + bx + cx^2, etc.) for the angle variance of the plasma thrower's recoil after x portion of the magazine is expended in one burst", ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_magnitude_base( "rd_plasma_thrower_recoil_magnitude_base", "10 100 -20 200 -400 50", FCVAR_CHEAT | FCVAR_REPLICATED, "Coefficients (a + bx + cx^2, etc.) for the magnitude of the plasma thrower's recoil after x portion of the magazine is expended in one burst", ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_magnitude_variance( "rd_plasma_thrower_recoil_magnitude_variance", "0.1 10", FCVAR_CHEAT | FCVAR_REPLICATED, "Coefficients (a + bx + cx^2, etc.) for the magnitude variance of the plasma thrower's recoil after x portion of the magazine is expended in one burst", ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_inaccuracy_bias( "rd_plasma_thrower_recoil_inaccuracy_bias", "0.4", FCVAR_CHEAT | FCVAR_REPLICATED, "Values below 0.5 push inaccuracy down for longer; values above 0.5 push inaccuracy up sooner.", true, 0, true, 1 );
ConVar rd_plasma_thrower_recoil_inaccuracy_base( "rd_plasma_thrower_recoil_inaccuracy_base", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "Number of degrees of inaccuracy for the plasma thrower by default", true, 0, true, 180.0f );
ConVar rd_plasma_thrower_recoil_inaccuracy_per_magazine( "rd_plasma_thrower_recoil_inaccuracy_per_magazine", "23", FCVAR_CHEAT | FCVAR_REPLICATED, "Number of degrees of inaccuracy added to the plasma thrower's aim per magazine.", true, -180.0f, true, 180.0f );
ConVar rd_plasma_thrower_recoil_pull_start( "rd_plasma_thrower_recoil_pull_start", "0.1", FCVAR_CHEAT | FCVAR_REPLICATED, "Portion of magazine before 'pull' recoil begins", true, 0, true, 1, ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_pull_end( "rd_plasma_thrower_recoil_pull_end", "0.25", FCVAR_CHEAT | FCVAR_REPLICATED, "Portion of magazine before 'pull' recoil ends", true, 0, true, 1, ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_pull_angle( "rd_plasma_thrower_recoil_pull_angle", "90", FCVAR_CHEAT | FCVAR_REPLICATED, "Degrees clockwise from the right to where the 'pull' recoil goes", true, 0, true, 360, ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_pull_distance( "rd_plasma_thrower_recoil_pull_distance", "50", FCVAR_CHEAT | FCVAR_REPLICATED, "Distance from the start to the end of the 'pull' recoil", true, 0, false, 0, ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_pull_bias( "rd_plasma_thrower_recoil_pull_bias", "0.8", FCVAR_CHEAT | FCVAR_REPLICATED, "Bias factor for the 'pull' recoil", true, 0, true, 1, ResetPlasmaThrowerRecoil );
ConVar rd_plasma_thrower_recoil_moving( "rd_plasma_thrower_recoil_moving", "2.0", FCVAR_CHEAT | FCVAR_REPLICATED, "Scale plasma thrower inaccuracy while moving" );
ConVar rd_plasma_thrower_recoil_crouch( "rd_plasma_thrower_recoil_crouch", "0.5", FCVAR_CHEAT | FCVAR_REPLICATED, "Scale plasma thrower inaccuracy while crouching and not moving" );
ConVar rd_plasma_thrower_recoil_bot( "rd_plasma_thrower_recoil_bot", "0.25", FCVAR_CHEAT | FCVAR_REPLICATED, "Scale plasma thrower recoil (but not spread) for bots (because they can't compensate for it)" );
static void InitPlasmaThrowerRecoil()
{
	constexpr float flOneOverCount = 1.0f / float( NELEMS( s_RD_Plasma_Thrower_Recoil ) );

	CUniformRandomStream rng;
	rng.SetSeed( rd_plasma_thrower_recoil_seed.GetInt() );

	// cache convars in local variables to save some pointer dereferences
	float flRampUpMin = rd_plasma_thrower_recoil_rampup_min.GetFloat();
	float flRampUpShots = rd_plasma_thrower_recoil_rampup_shots.GetFloat() * NELEMS( s_RD_Plasma_Thrower_Recoil );
	float flVariance = rd_plasma_thrower_recoil_variance.GetFloat();
	CDynamicPolynomial AngleBase( rd_plasma_thrower_recoil_angle_base.GetString() );
	CDynamicPolynomial AngleVariance( rd_plasma_thrower_recoil_angle_variance.GetString() );
	CDynamicPolynomial MagnitudeBase( rd_plasma_thrower_recoil_magnitude_base.GetString() );
	CDynamicPolynomial MagnitudeVariance( rd_plasma_thrower_recoil_magnitude_variance.GetString() );
	int iPullStart = rd_plasma_thrower_recoil_pull_start.GetFloat() * NELEMS( s_RD_Plasma_Thrower_Recoil );
	int iPullEnd = rd_plasma_thrower_recoil_pull_end.GetFloat() * NELEMS( s_RD_Plasma_Thrower_Recoil );
	if ( iPullStart > iPullEnd )
		iPullStart = iPullEnd;
	float flPullSin, flPullCos;
	SinCos( DEG2RAD( rd_plasma_thrower_recoil_pull_angle.GetFloat() ), &flPullSin, &flPullCos );
	flPullSin *= -rd_plasma_thrower_recoil_pull_distance.GetFloat();
	flPullCos *= rd_plasma_thrower_recoil_pull_distance.GetFloat();
	float flPullBias = rd_plasma_thrower_recoil_pull_bias.GetFloat();

	// first compute as angle/magnitude
	for ( int i = 0; i < NELEMS( s_RD_Plasma_Thrower_Recoil ); i++ )
	{
		float flProgress = float( i ) * flOneOverCount;
		float flAngleVariance = AngleVariance( flProgress );
		float flMagnitudeVariance = MagnitudeVariance( flProgress );

		s_RD_Plasma_Thrower_Recoil[i].x = AngleBase( flProgress ) + rng.RandomFloat( -flAngleVariance, flAngleVariance );
		s_RD_Plasma_Thrower_Recoil[i].y = MagnitudeBase( flProgress ) + rng.RandomFloat( -flMagnitudeVariance, flMagnitudeVariance );

		if ( i < flRampUpShots )
		{
			float flScale = RemapVal( i, 0, flRampUpShots, flRampUpMin, 1 );
			s_RD_Plasma_Thrower_Recoil[i].y *= flScale;
		}

		if ( i > 0 && flVariance < 1.0f )
		{
			s_RD_Plasma_Thrower_Recoil[i].x = Lerp( flVariance, s_RD_Plasma_Thrower_Recoil[i - 1].x, s_RD_Plasma_Thrower_Recoil[i].x );
			s_RD_Plasma_Thrower_Recoil[i].y = Lerp( flVariance, s_RD_Plasma_Thrower_Recoil[i - 1].y, s_RD_Plasma_Thrower_Recoil[i].y );
		}
	}

	// now convert the angle/magnitude to x/y
	for ( int i = 0; i < NELEMS( s_RD_Plasma_Thrower_Recoil ); i++ )
	{
		float s, c;
		SinCos( DEG2RAD( s_RD_Plasma_Thrower_Recoil[i].x ), &s, &c );
		s_RD_Plasma_Thrower_Recoil[i].x = c * s_RD_Plasma_Thrower_Recoil[i].y;
		s_RD_Plasma_Thrower_Recoil[i].y = -s * s_RD_Plasma_Thrower_Recoil[i].y;
	}

	// last, apply the "pull"
	for ( int i = iPullStart; i < iPullEnd; i++ )
	{
		float flScale = Bias( float( i - iPullStart ) / float( iPullEnd - iPullStart ), flPullBias );
		s_RD_Plasma_Thrower_Recoil[i].x += flPullCos * flScale;
		s_RD_Plasma_Thrower_Recoil[i].y += flPullSin * flScale;
	}
	for ( int i = iPullEnd; i < NELEMS( s_RD_Plasma_Thrower_Recoil ); i++ )
	{
		s_RD_Plasma_Thrower_Recoil[i].x += flPullCos;
		s_RD_Plasma_Thrower_Recoil[i].y += flPullSin;
	}
}
extern ConVar asw_weapon_max_shooting_distance;
extern ConVar asw_weapon_force_scale;
extern ConVar asw_cam_marine_pitch;
extern ConVar asw_cam_marine_dist;
#ifdef CLIENT_DLL
extern ConVar asw_cam_marine_shift_enable;
#endif
extern ConVar asw_cam_marine_shift_maxx;
extern ConVar asw_cam_marine_shift_maxy;
extern ConVar asw_cam_marine_shift_maxy_south;
extern ConVar rd_ray_trace_distance;

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Plasma_Thrower, DT_ASW_Weapon_Plasma_Thrower );

BEGIN_NETWORK_TABLE( CASW_Weapon_Plasma_Thrower, DT_ASW_Weapon_Plasma_Thrower )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_iShotsInCurrentBurst ) ),
#else
	SendPropInt( SENDINFO( m_iShotsInCurrentBurst ), 8, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
#endif
END_NETWORK_TABLE()

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CASW_Weapon_Plasma_Thrower )
	DEFINE_PRED_FIELD( m_iShotsInCurrentBurst, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#endif

LINK_ENTITY_TO_CLASS( asw_weapon_plasma_thrower, CASW_Weapon_Plasma_Thrower );
PRECACHE_WEAPON_REGISTER( asw_weapon_plasma_thrower );

#ifndef CLIENT_DLL
BEGIN_DATADESC( CASW_Weapon_Plasma_Thrower )
	DEFINE_FIELD( m_iShotsInCurrentBurst, FIELD_INTEGER ),
END_DATADESC()
#endif

CASW_Weapon_Plasma_Thrower::CASW_Weapon_Plasma_Thrower()
{
	m_iShotsInCurrentBurst = 0;
}

Vector CASW_Weapon_Plasma_Thrower::ComputeRecoil( float flMagazineProgress, bool bMoving, bool bCrouching )
{
	if ( s_RD_Plasma_Thrower_Recoil[0] == vec2_invalid )
	{
		InitPlasmaThrowerRecoil();
	}

	Vector2D xy;
	Assert( flMagazineProgress >= 0 && flMagazineProgress <= 1.0f );
	if ( flMagazineProgress <= 0 )
	{
		xy = s_RD_Plasma_Thrower_Recoil[0];
		flMagazineProgress = 0.0f;
	}
	else if ( flMagazineProgress >= 1.0f )
	{
		xy = s_RD_Plasma_Thrower_Recoil[NELEMS( s_RD_Plasma_Thrower_Recoil ) - 1];
		flMagazineProgress = 1.0f;
	}
	else
	{
		float flProgress = flMagazineProgress * ( NELEMS( s_RD_Plasma_Thrower_Recoil ) - 1 );
		int iMinProgress = Floor2Int( flProgress );
		int iMaxProgress = Ceil2Int( flProgress );

		xy = Lerp( SimpleSpline( flProgress - iMinProgress ), s_RD_Plasma_Thrower_Recoil[iMinProgress], s_RD_Plasma_Thrower_Recoil[iMaxProgress] );
	}

	float flScale = 1.0f;
	if ( bMoving )
		flScale = rd_plasma_thrower_recoil_moving.GetFloat();
	else if ( bCrouching )
		flScale = rd_plasma_thrower_recoil_crouch.GetFloat();

	const float flInaccuracyBias = rd_plasma_thrower_recoil_inaccuracy_bias.GetFloat();
	const float flInaccuracyBase = rd_plasma_thrower_recoil_inaccuracy_base.GetFloat();
	const float flInaccuracyPerMagazine = rd_plasma_thrower_recoil_inaccuracy_per_magazine.GetFloat();

	return Vector( xy.x * flScale, xy.y * flScale, sinf( DEG2RAD( flInaccuracyBase + Bias( flMagazineProgress, flInaccuracyBias ) * flInaccuracyPerMagazine ) * flScale / 2 ) );
}

#ifdef CLIENT_DLL
void CASW_Weapon_Plasma_Thrower::ModifyCrosshairPos( int &x, int &y )
{
	if ( m_iShotsInCurrentBurst == 0 )
		return;

	C_ASW_Marine *pOwner = GetMarine();
	if ( !pOwner )
		return;

	Vector recoil = ComputeRecoil(
		float( m_iShotsInCurrentBurst ) / float( GetMaxClip1() ),
		!pOwner->GetLocalVelocity().IsZero(),
		pOwner->m_bWalking || pOwner->m_bForceWalking || ( !pOwner->IsInhabited() && pOwner->m_bAICrouch )
	);
	x += YRES( recoil.x );
	y += YRES( recoil.y );
}

bool CASW_Weapon_Plasma_Thrower::ShouldShowLaserPointer()
{
	if ( gpGlobals->curtime < m_flNextPrimaryAttack )
	{
		return false;
	}

	return BaseClass::ShouldShowLaserPointer();
}
#endif

void CASW_Weapon_Plasma_Thrower::Precache()
{
	BaseClass::Precache();

	if ( s_RD_Plasma_Thrower_Recoil[0] == vec2_invalid )
	{
		InitPlasmaThrowerRecoil();
	}

	PrecacheParticleSystem( "tracer_egon2" );
	PrecacheParticleSystem( "muzzle_egon2" );
	PrecacheParticleSystem( "egon2_airblast" );

	PrecacheScriptSound( "ASW_Weapon_Plasma_Thrower.Loop" );
	PrecacheScriptSound( "ASW_Weapon_Plasma_Thrower.Stop" );
}

void CASW_Weapon_Plasma_Thrower::PrimaryAttack()
{
	// If my clip is empty (and I use clips) start reload
	if ( UsesClipsForAmmo1() && !m_iClip1 )
	{
		Reload();
		return;
	}

	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner || !pOwner->IsInhabitableNPC() )
		return;
	CASW_Inhabitable_NPC *pNPC = assert_cast< CASW_Inhabitable_NPC * >( pOwner );
	CASW_Player *pPlayer = GetCommander();
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );

	m_bIsFiring = true;

	if ( m_iClip1 <= AmmoClickPoint() )
	{
		LowAmmoSound();
	}

#ifdef GAME_DLL	// check for turning on lag compensation
	if ( pPlayer && pNPC->IsInhabited() )
	{
		CASW_Lag_Compensation::RequestLagCompensation( pPlayer, pPlayer->GetCurrentUserCommand() );
	}
#endif

	FireBulletsInfo_t info;
	info.m_vecSrc = pNPC->Weapon_ShootPosition();
	if ( pPlayer && pMarine && pMarine->IsInhabited() )
	{
		info.m_vecDirShooting = pPlayer->GetAutoaimVectorForMarine( pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount() );	// 45 degrees = 0.707106781187
	}
	else
	{
#ifdef CLIENT_DLL
		Msg( "Error, clientside firing of a weapon that's being controlled by an AI marine\n" );
#else
		info.m_vecDirShooting = pNPC->GetActualShootTrajectory( info.m_vecSrc );
#endif
	}

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	info.m_iShots = 0;
	float fireRate = GetFireRate();
	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		m_flNextPrimaryAttack += fireRate;
		info.m_iShots++;
		if ( !fireRate )
			break;
	}

	// Make sure we don't fire more than the amount in the clip
	if ( UsesClipsForAmmo1() )
	{
		info.m_iShots = MIN( info.m_iShots, m_iClip1 );
		m_iClip1 -= info.m_iShots;

#ifdef GAME_DLL
		if ( m_iClip1 <= 0 && pMarine && pMarine->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
		{
			pMarine->OnWeaponOutOfAmmo( true );
		}
#endif
	}
	else
	{
		info.m_iShots = MIN( info.m_iShots, pNPC->GetAmmoCount( m_iPrimaryAmmoType ) );
		pNPC->RemoveAmmo( info.m_iShots, m_iPrimaryAmmoType );
	}

	if ( m_iShotsInCurrentBurst == 0 )
		EmitSound( "ASW_Weapon_Plasma_Thrower.Loop" );

	// computing the recoil requires a bunch of transforms, so only do it once per tick at most even if we're shooting multiple bullets
	m_iShotsInCurrentBurst += info.m_iShots;
	Vector recoil = ComputeRecoil(
		float( m_iShotsInCurrentBurst ) / float( GetMaxClip1() ),
		!pOwner->GetLocalVelocity().IsZero(),
		pMarine && ( pMarine->m_bWalking || pMarine->m_bForceWalking || ( !pMarine->IsInhabited() && pMarine->m_bAICrouch ) )
	);

	ASW_Controls_t eControls = pNPC->GetASWControls();
	if ( !pNPC->IsInhabited() )
	{
		recoil.x *= rd_plasma_thrower_recoil_bot.GetFloat();
		recoil.y *= rd_plasma_thrower_recoil_bot.GetFloat();
		eControls = ASWC_FIRSTPERSON;
	}

	QAngle angCamera;
	Vector vecCamera = pNPC->EyePosition();

	// server doesn't have these unions in CASW_Player, so do them on the stack.
	union
	{
		unsigned WidthHeight;
		struct
		{
			short Width;
			short Height;
		} S;
	} Screen;
	Screen.WidthHeight = pPlayer ? pPlayer->m_iScreenWidthHeight : 0x28001E0; // default to 640x480 for unowned bots
	union
	{
		unsigned XY;
		struct
		{
			short X;
			short Y;
		} M;
	} Mouse;
	Mouse.XY = pPlayer ? pPlayer->m_iMouseXY : 0x14000F0; // default to middle of screen

	if ( eControls == ASWC_TOPDOWN && pPlayer )
	{
		// just ignore death cam and in-progress camera rotation entirely for now.
		angCamera = QAngle( asw_cam_marine_pitch.GetFloat(), pPlayer->m_flMovementAxisYaw, 0 );
		Vector vecCameraForward, vecCameraRight, vecCameraUp;
		AngleVectors( angCamera, &vecCameraForward, &vecCameraRight, &vecCameraUp );
		vecCamera -= vecCameraForward * asw_cam_marine_dist.GetFloat();

#ifdef CLIENT_DLL
		if ( asw_cam_marine_shift_enable.GetBool() )
#else
		if ( V_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "asw_cam_marine_shift_enable" ) ) != 0 )
#endif
		{
			vecCamera += vecCameraRight * ( Mouse.M.X - Screen.S.Width / 2.0f ) / Screen.S.Width * asw_cam_marine_shift_maxx.GetFloat();

			vecCameraUp.z = 0;
			vecCameraUp.NormalizeInPlace();
			float flDeltaY = ( Screen.S.Height / 2.0f - Mouse.M.Y ) / Screen.S.Height;
			vecCamera += vecCameraUp * flDeltaY * ( flDeltaY < 0.0f ? asw_cam_marine_shift_maxy_south.GetFloat() : asw_cam_marine_shift_maxy.GetFloat() );
		}
	}
	else if ( eControls == ASWC_FIRSTPERSON )
	{
		VectorAngles( info.m_vecDirShooting, angCamera );
		Assert( Mouse.M.X == Screen.S.Width / 2 );
		Assert( Mouse.M.Y == Screen.S.Height / 2 );
		Mouse.M.X = Screen.S.Width / 2;
		Mouse.M.Y = Screen.S.Height / 2;
	}
	else
	{
		Assert( eControls == ASWC_THIRDPERSONSHOULDER );

		VectorAngles( info.m_vecDirShooting, angCamera );
		Assert( Mouse.M.X == Screen.S.Width / 2 );
		Assert( Mouse.M.Y == Screen.S.Height / 2 );
		Mouse.M.X = Screen.S.Width / 2;
		Mouse.M.Y = Screen.S.Height / 2;

		Assert( !"TODO: third person camera!!!!!" );
	}

	float flRatio = float( Screen.S.Width ) / float( Screen.S.Height );
	Vector2D vecMouse( ( Screen.S.Width / 2.0f - Mouse.M.X ) / Screen.S.Width, ( Screen.S.Height / 2.0f - Mouse.M.Y ) / Screen.S.Height );
	vecMouse.x -= recoil.x / 480.0f / flRatio;
	vecMouse.y -= recoil.y / 480.0f;

	Vector X, Y, Z;
	AngleVectors( angCamera, &X, &Y, &Z );

	float flHalfFOV = pPlayer->GetFOV() / 2.0f;
	Vector vecTraceDirection = X
		- tanf( DEG2RAD( flHalfFOV ) ) * 2 * Y * ( vecMouse.x ) * 0.75f * flRatio
		+ tanf( DEG2RAD( flHalfFOV ) ) * 2 * Z * ( vecMouse.y ) * 0.75f;
	vecTraceDirection.NormalizeInPlace();

	Vector vecTraceEnd = vecCamera + vecTraceDirection * rd_ray_trace_distance.GetFloat();
	trace_t tr;
	CASW_Trace_Filter filter( pPlayer, COLLISION_GROUP_NONE );
	UTIL_TraceLine( vecCamera, vecTraceEnd, MASK_SOLID | CONTENTS_IGNORE_NODRAW_OPAQUE, &filter, &tr );

	info.m_vecDirShooting = tr.endpos - info.m_vecSrc;
	if ( !tr.m_pEnt || !( tr.m_pEnt->GetFlags() & FL_AIMTARGET ) )
		info.m_vecDirShooting.z = 0;
	info.m_vecDirShooting.NormalizeInPlace();

	info.m_flDistance = asw_weapon_max_shooting_distance.GetFloat();
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 1;
	info.m_flDamageForceScale = asw_weapon_force_scale.GetFloat();
	info.m_nFlags = FIRE_BULLETS_DONT_HIT_UNDERWATER;

	info.m_vecSpread = Vector( recoil.z, recoil.z, recoil.z );
	info.m_flDamage = GetWeaponDamage();

	pNPC->FireBullets( info );

	// increment shooting stats
#ifndef CLIENT_DLL
	if ( pMarine && pMarine->GetMarineResource() )
	{
		pMarine->GetMarineResource()->UsedWeapon( this, info.m_iShots );
		pMarine->OnWeaponFired( this, info.m_iShots );
	}
#endif
}

void CASW_Weapon_Plasma_Thrower::SecondaryAttack()
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if ( !pOwner || !pOwner->IsInhabitableNPC() )
		return;
	CASW_Inhabitable_NPC *pNPC = assert_cast< CASW_Inhabitable_NPC * >( pOwner );
	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );

	if ( Clip1() < rd_plasma_thrower_airblast_ammo.GetInt() )
	{
		if ( !Reload() )
		{
			SendWeaponAnim( ACT_VM_DRYFIRE );
			WeaponSound( EMPTY );
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		}

		return;
	}

	m_iClip1 -= rd_plasma_thrower_airblast_ammo.GetInt();
#ifdef GAME_DLL
	if ( m_iClip1 <= 0 && pMarine && pMarine->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		pMarine->OnWeaponOutOfAmmo( true );
	}
#endif

	SendWeaponAnim( GetSecondaryAttackActivity() );
	if ( pMarine )
	{
		pMarine->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_SECONDARY );

#ifndef CLIENT_DLL
		if ( pMarine->GetMarineResource() )
		{
			pMarine->OnWeaponFired( this, 1, true );
		}

		ASWFailAdvice()->OnMarineUsedSecondary();
#endif
	}

	WeaponSound( SPECIAL1 );
	DispatchParticleEffect( "egon2_airblast", PATTACH_POINT_FOLLOW, this, "muzzle" );

#ifndef CLIENT_DLL
	CASW_Player *pPlayer = GetCommander();
	const Vector vecOrigin = pNPC->Weapon_ShootPosition();
	const float flRadius = rd_plasma_thrower_airblast_range.GetFloat();
	const float flMinDot = rd_plasma_thrower_airblast_dot.GetFloat();
	const float flMinNearDot = rd_plasma_thrower_airblast_dot_near.GetFloat();
	const Vector vecForward = pPlayer && pMarine && pMarine->IsInhabited() ? pPlayer->GetAutoaimVectorForMarine( pMarine, GetAutoAimAmount(), GetVerticalAdjustOnlyAutoAimAmount() ) : pNPC->GetActualShootTrajectory( vecOrigin );

	CBaseEntity *pTarget = NULL;
	int nHits = 0;
	while ( ( pTarget = gEntList.FindEntityInSphere( pTarget, vecOrigin, flRadius ) ) )
	{
		if ( pTarget == pNPC )
		{
			if ( pNPC->IsOnFire() )
			{
				// extinguish self but don't push self
				nHits++;
				if ( pMarine )
				{
					pMarine->Extinguish( pMarine, this );
					if ( pMarine->GetMarineResource() )
					{
						pMarine->GetMarineResource()->m_iPlasmaThrowerExtinguishMarine++;
					}
				}
				else
				{
					pNPC->Extinguish();
				}
			}
			continue;
		}

		if ( pTarget->m_takedamage == DAMAGE_NO )
			continue;

		if ( pTarget->GetMoveType() == MOVETYPE_NONE || pTarget->GetMoveType() == MOVETYPE_PUSH )
			continue;

		Vector vecDiff;
		VectorSubtract( pTarget->WorldSpaceCenter(), vecOrigin, vecDiff );
		float flDist = vecDiff.NormalizeInPlace();
		float flDot = vecDiff.Dot( vecForward );
		if ( flDot < ( flDist > rd_plasma_thrower_airblast_range_near.GetFloat() ? flMinDot : flMinNearDot ) )
			continue;

		float flPushForce = rd_plasma_thrower_airblast_push.GetFloat();
		bool bFriendly = pNPC->IRelationType( pTarget ) == D_LI;
		if ( bFriendly )
		{
			flPushForce = rd_plasma_thrower_airblast_push_ally.GetFloat();
		}

		// put out fires if we're friends
		if ( bFriendly )
		{
			CASW_Marine *pTargetMarine = CASW_Marine::AsMarine( pTarget );
			if ( pTargetMarine )
			{
				if ( pTargetMarine->IsOnFire() )
				{
					pTargetMarine->Extinguish( pNPC, this );
					if ( pMarine->GetMarineResource() )
					{
						pMarine->GetMarineResource()->m_iPlasmaThrowerExtinguishMarine++;
					}
				}
			}
			else
			{
				CBaseAnimating *pAnim = pTarget->GetBaseAnimating();
				if ( pAnim && pAnim->IsOnFire() )
				{
					CEntityFlame *pFireChild = dynamic_cast< CEntityFlame * >( pAnim->GetEffectEntity() );
					if ( pFireChild )
					{
						pAnim->SetEffectEntity( NULL );
						UTIL_Remove( pFireChild );
					}

					pAnim->Extinguish();
				}
			}
		}

		float flDistanceFactor = 1.0f - Bias( clamp( flDist / rd_plasma_thrower_airblast_range.GetFloat(), 0.0f, 1.0f ), rd_plasma_thrower_airblast_falloff.GetFloat() );

		Vector vecImpulse = vecDiff * flPushForce;
		vecImpulse.z = MAX( vecImpulse.z, rd_plasma_thrower_airblast_push_up.GetFloat() );
		pTarget->SetGroundEntity( NULL );
		if ( pTarget->GetMoveType() == MOVETYPE_VPHYSICS && pTarget->VPhysicsGetObject() )
		{
			vecImpulse *= flDistanceFactor;
			QAngle angles;
			VectorAngles( vecDiff.ProjectOnto( vecForward ), angles );
			AngularImpulse angularImpulse( VectorExpand( angles ) );
			angularImpulse *= rd_plasma_thrower_airblast_torque.GetFloat();
			pTarget->VPhysicsGetObject()->AddVelocity( &vecImpulse, &angularImpulse );
		}
		else
		{
			pTarget->SetAbsVelocity( Lerp( flDistanceFactor, pTarget->GetAbsVelocity(), vecImpulse ) );
		}

		nHits++;

		if ( !bFriendly && pTarget->GetHealth() > 0 )
		{
			if ( CAI_BaseNPC *pTargetNPC = pTarget->MyNPCPointer() )
			{
				pTargetNPC->TaskFail( "airblast stunned" );
				pTargetNPC->SetSchedule( SCHED_BIG_FLINCH );
			}
		}
	}

	if ( nHits && pMarine && pMarine->GetMarineResource() )
	{
		pMarine->GetMarineResource()->IncrementWeaponStats( ( Class_T )CLASS_ASW_PLASMA_THROWER_AIRBLAST, 0, 0, 0, nHits, 0 );
	}
#endif

	if ( m_iShotsInCurrentBurst )
	{
		StopSound( "ASW_Weapon_Plasma_Thrower.Loop" );
		EmitSound( "ASW_Weapon_Plasma_Thrower.Stop" );
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + rd_plasma_thrower_cooldown_secondary.GetFloat(); // this may make the cooldown shorter and that's intended
	m_iShotsInCurrentBurst = 0; // after airblast, gun is cool

	Assert( m_flNextSecondaryAttack <= gpGlobals->curtime + rd_plasma_thrower_cooldown_airblast.GetFloat() );
	m_flNextSecondaryAttack = MAX( m_flNextSecondaryAttack, gpGlobals->curtime + rd_plasma_thrower_cooldown_airblast.GetFloat() );
}

void CASW_Weapon_Plasma_Thrower::OnStoppedFiring()
{
	if ( m_iShotsInCurrentBurst == 0 )
		return;

	StopSound( "ASW_Weapon_Plasma_Thrower.Loop" );
	EmitSound( "ASW_Weapon_Plasma_Thrower.Stop" );

	Assert( m_flNextPrimaryAttack <= gpGlobals->curtime + rd_plasma_thrower_cooldown_primary.GetFloat() );
	m_flNextPrimaryAttack = MAX( m_flNextPrimaryAttack, gpGlobals->curtime + rd_plasma_thrower_cooldown_primary.GetFloat() );
	m_iShotsInCurrentBurst = 0;
}

float CASW_Weapon_Plasma_Thrower::GetWeaponBaseDamageOverride()
{
	extern ConVar rd_plasma_thrower_dmg_base;
	return rd_plasma_thrower_dmg_base.GetFloat();
}

int CASW_Weapon_Plasma_Thrower::GetWeaponSkillId()
{
	return ASW_MARINE_SKILL_ACCURACY;
}

int CASW_Weapon_Plasma_Thrower::GetWeaponSubSkillId()
{
	return ASW_MARINE_SUBSKILL_ACCURACY_PLASMA_THROWER_DMG;
}
#endif

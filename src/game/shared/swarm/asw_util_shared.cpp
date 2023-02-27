#include "cbase.h"
#include "asw_shareddefs.h"
#include "gamestringpool.h"
#ifdef GAME_DLL
	#include "asw_player.h"
	#include "asw_marine.h"
	#include "asw_marine_resource.h"
	#include "asw_gamerules.h"
	#include "asw_game_resource.h"
	#include "props.h"
	#include "vphysics_interface.h"
	#include "physics.h"
	#include "vphysics/friction.h"
	#include "asw_computer_area.h"
	#include "point_camera.h"
	#include "asw_remote_turret_shared.h"
	#include "asw_computer_area.h"
	#include "asw_button_area.h"
	#include "fogcontroller.h"
	#include "asw_point_camera.h"
	#include "asw_deathmatch_mode.h"
#else
	#include "asw_gamerules.h"
	#include "c_asw_drone_advanced.h"
	#include "c_asw_fx.h"
	#include "c_asw_marine.h"
	#include "c_asw_game_resource.h"
	#include "c_asw_marine_resource.h"
	#include "c_asw_computer_area.h"
	#include "c_point_camera.h"
	#include "asw_remote_turret_shared.h"
    #include "c_asw_player.h"
	#include "c_playerresource.h"
	#include "c_asw_computer_area.h"
	#include "c_asw_button_area.h"
	#include "vgui/cursor.h"
	#include "iinput.h"
	#include <vgui/ISurface.h>
	#include "vguimatsurface/imatsystemsurface.h"
	#include "vgui_controls\Controls.h"
	#include <vgui/IVGUI.h>
	#include "ivieweffects.h"
	#include "asw_input.h"
	#include "c_asw_point_camera.h"
	#include "baseparticleentity.h"
	#include "asw_hud_floating_number.h"
	#include "takedamageinfo.h"
	#include "clientmode_asw.h"
	#include "engine/IVDebugOverlay.h"
	#include "c_user_message_register.h"
	#include "prediction.h"
	#include "asw_medal_store.h"
	#define CASW_Marine C_ASW_Marine
	#define CASW_Game_Resource C_ASW_Game_Resource
	#define CASW_Marine_Resource C_ASW_Marine_Resource
	#define CASW_Computer_Area C_ASW_Computer_Area
	#define CPointCamera C_PointCamera
	#define CASW_PointCamera C_ASW_PointCamera
	#define CASW_Remote_Turret C_ASW_Remote_Turret
	#define CASW_Button_Area C_ASW_Button_Area
	#define CASW_Computer_Area C_ASW_Computer_Area
#endif
#include "shake.h"
#include "asw_util_shared.h"
#include "tier2/fileutils.h"
#include "vpklib/packedstore.h"
#include "vgui/ILocalize.h"
#include "iregistry.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rd_override_commander_promotion( "rd_override_commander_promotion", "-1", FCVAR_REPLICATED );
ConVar rd_override_commander_level( "rd_override_commander_level", "-1", FCVAR_REPLICATED );

#ifndef CLIENT_DLL
ConVar asw_debug_marine_can_see("asw_debug_marine_can_see", "0", FCVAR_CHEAT, "Display lines for waking up aliens");
#else
ConVar rd_load_all_localization_files( "rd_load_all_localization_files", "1", FCVAR_DEVELOPMENTONLY, "Load reactivedrop_english.txt, etc. from all addons rather than just the last one." );
extern int g_asw_iGUIWindowsOpen;
#endif

ConVar asw_marine_view_cone_dist("asw_marine_view_cone_dist", "700", FCVAR_REPLICATED, "Distance for marine view cone checks");
ConVar asw_marine_view_cone_dot("asw_marine_view_cone_dot", "0.5", FCVAR_REPLICATED, "Dot for marine view cone checks");
extern ConVar asw_rts_controls;


ConVar asw_shake_test_punch_dirx("asw_shake_test_punch_dirx","0", FCVAR_REPLICATED|FCVAR_HIDDEN );
ConVar asw_shake_test_punch_diry("asw_shake_test_punch_diry","0", FCVAR_REPLICATED|FCVAR_HIDDEN );
ConVar asw_shake_test_punch_dirz("asw_shake_test_punch_dirz","1", FCVAR_REPLICATED|FCVAR_HIDDEN );
ConVar asw_shake_test_punch_freq("asw_shake_test_punch_freq","1.5", FCVAR_REPLICATED|FCVAR_HIDDEN );
ConVar asw_shake_test_punch_amp("asw_shake_test_punch_amp","60", FCVAR_REPLICATED|FCVAR_HIDDEN );
ConVar asw_shake_test_punch_dura("asw_shake_test_punch_dura","0.75", FCVAR_REPLICATED|FCVAR_HIDDEN );

// rotates one angle towards another, with a fixed turning rate over the time
float ASW_ClampYaw( float yawSpeedPerSec, float current, float target, float time )
{
	if (current != target)
	{
		float speed = yawSpeedPerSec * time;
		float move = target - current;

		if (target > current)
		{
			if (move >= 180)
				move = move - 360;
		}
		else
		{
			if (move <= -180)
				move = move + 360;
		}

		if (move > 0)
		{// turning to the npc's left
			if (move > speed)
				move = speed;
		}
		else
		{// turning to the npc's right
			if (move < -speed)
				move = -speed;
		}
		
		return anglemod(current + move);
	}
	
	return target;
}

float ASW_Linear_Approach( float current, float target, float delta)
{
	if (current < target)
		current = MIN(current + delta, target);
	else if (current > target)
		current = MAX(current - delta, target);

	return current;
}

// time independent movement of one angle to a fraction of the desired
float ASW_ClampYaw_Fraction( float fraction, float current, float target, float time )
{
	if (current != target)
	{		
		float move = target - current;

		if (target > current)
		{
			if (move >= 180)
				move = move - 360;
		}
		else
		{
			if (move <= -180)
				move = move + 360;
		}
		move = move * pow(fraction, time);
		float r = anglemod(target - move);
		
		return r;
	}
	
	return target;
}

bool ASW_LineCircleIntersection(
	const Vector2D &center,
	const float radius,
	const Vector2D &vLinePt,
	const Vector2D &vLineDir,
	float *fIntersection1,
	float *fIntersection2)
{
	// Line = P + Vt
	// Sphere = r (assume we've translated to origin)
	// (P + Vt)^2 = r^2
	// VVt^2 + 2PVt + (PP - r^2)
	// Solve as quadratic:  (-b  +/-  sqrt(b^2 - 4ac)) / 2a
	// If (b^2 - 4ac) is < 0 there is no solution.
	// If (b^2 - 4ac) is = 0 there is one solution (a case this function doesn't support).
	// If (b^2 - 4ac) is > 0 there are two solutions.
	Vector2D P;
	float a, b, c, sqr, insideSqr;


	// Translate circle to origin.
	P[0] = vLinePt[0] - center[0];
	P[1] = vLinePt[1] - center[1];
	
	a = vLineDir.Dot(vLineDir);
	b = 2.0f * P.Dot(vLineDir);
	c = P.Dot(P) - (radius * radius);

	insideSqr = b*b - 4*a*c;
	if(insideSqr <= 0.000001f)
		return false;

	// Ok, two solutions.
	sqr = (float)FastSqrt(insideSqr);

	float denom = 1.0 / (2.0f * a);
	
	*fIntersection1 = (-b - sqr) * denom;
	*fIntersection2 = (-b + sqr) * denom;

	return true;
}

#ifdef GAME_DLL
// a local helper to normalize some code below -- gets inlined
static void ASW_WriteScreenShakeToMessage( CASW_Inhabitable_NPC *pNPC, ShakeCommand_t eCommand, float amplitude, float frequency, float duration, const Vector &direction )
{
	CASW_ViewNPCRecipientFilter user( pNPC );
	user.MakeReliable();
	if ( direction.IsZeroFast() ) // nondirectional shake
	{
		UserMessageBegin( user, "Shake" );
			WRITE_BYTE( eCommand );				// shake command (SHAKE_START, STOP, FREQUENCY, AMPLITUDE)
			WRITE_FLOAT( amplitude );			// shake magnitude/amplitude
			WRITE_FLOAT( frequency );				// shake noise frequency
			WRITE_FLOAT( duration );				// shake lasts this long
			WRITE_ENTITY( pNPC->entindex() );
		MessageEnd();
	}
	else // directional shake
	{
		UserMessageBegin( user, "ShakeDir" );
			WRITE_BYTE( eCommand );				// shake command (SHAKE_START, STOP, FREQUENCY, AMPLITUDE)
			WRITE_FLOAT( amplitude );			// shake magnitude/amplitude
			WRITE_FLOAT( frequency );				// shake noise frequency
			WRITE_FLOAT( duration );				// shake lasts this long
			WRITE_VEC3NORMAL( direction );
			WRITE_ENTITY( pNPC->entindex() );
		MessageEnd();
	}
}
#endif

//-----------------------------------------------------------------------------
// Transmits the actual shake event
//-----------------------------------------------------------------------------
 void ASW_TransmitShakeEvent( CASW_Inhabitable_NPC *pNPC, float localAmplitude, float frequency, float duration, ShakeCommand_t eCommand, const Vector &direction )
{
	if (( localAmplitude > 0 ) || ( eCommand == SHAKE_STOP ))
	{
		if ( eCommand == SHAKE_STOP )
			localAmplitude = 0;

#ifdef GAME_DLL
		ASW_WriteScreenShakeToMessage( pNPC, eCommand, localAmplitude, frequency, duration, direction );
#else
		ScreenShake_t shake;

		shake.command	= eCommand;
		shake.amplitude = localAmplitude;
		shake.frequency = frequency;
		shake.duration	= duration;
		shake.direction = direction;

		ASW_TransmitShakeEvent( pNPC, shake);
#endif
	}
}

void ASW_TransmitShakeEvent( CASW_Inhabitable_NPC *pNPC, const ScreenShake_t &shake )
{
	if ( shake.command == SHAKE_STOP && shake.amplitude != 0 )
	{
		// create a corrected screenshake and recursively call myself
		AssertMsg1( false, "A ScreenShake_t had a SHAKE_STOP command but a nonzero amplitude %.1f; this is meaningless.\n", shake.amplitude);
		ScreenShake_t localShake = shake;
		localShake.amplitude = 0;
		ASW_TransmitShakeEvent( pNPC, localShake );
	}
#ifdef GAME_DLL
	ASW_WriteScreenShakeToMessage( pNPC, shake.command, shake.amplitude, shake.frequency, shake.duration, shake.direction );
#else
	if ( !( prediction && prediction->InPrediction() && !prediction->IsFirstTimePredicted() ) )
	{
		GetViewEffects()->Shake( shake );
	}
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Compute shake amplitude
//-----------------------------------------------------------------------------
inline float ASW_ComputeShakeAmplitude( const Vector &center, const Vector &shakePt, float amplitude, float radius ) 
{
	if ( radius <= 0 )
		return amplitude;

	float localAmplitude = -1;
	Vector delta = center - shakePt;
	float distance = delta.Length();

	if ( distance <= radius )
	{
		// Make the amplitude fall off over distance
		float flPerc = 1.0 - (distance / radius);
		localAmplitude = amplitude * flPerc;
	}

	return localAmplitude;
}

//-----------------------------------------------------------------------------
// Purpose: Shake the screen of all clients within radius.
//			radius == 0, shake all clients
// UNDONE: Fix falloff model (disabled)?
// UNDONE: Affect user controls?
// Input  : center - Center of screen shake, radius is measured from here.
//			amplitude - Amplitude of shake
//			frequency - 
//			duration - duration of shake in seconds.
//			radius - Radius of effect, 0 shakes all clients.
//			command - One of the following values:
//				SHAKE_START - starts the screen shake for all players within the radius
//				SHAKE_STOP - stops the screen shake for all players within the radius
//				SHAKE_AMPLITUDE - modifies the amplitude of the screen shake
//									for all players within the radius
//				SHAKE_FREQUENCY - modifies the frequency of the screen shake
//									for all players within the radius
//			bAirShake - completely ignored
//-----------------------------------------------------------------------------
const float ASW_MAX_SHAKE_AMPLITUDE = 16.0f;
void UTIL_ASW_ScreenShake( const Vector &center, float amplitude, float frequency, float duration, float radius, ShakeCommand_t eCommand, bool bAirShake, CASW_Marine *pOnlyMarine )
{
	int			i;
	float		localAmplitude;

	if ( amplitude > ASW_MAX_SHAKE_AMPLITUDE )
	{
		amplitude = ASW_MAX_SHAKE_AMPLITUDE;
	}

	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( !pGameResource )
	{
		return;
	}

	for ( i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
		if ( !pMR )
		{
			continue;
		}

		CASW_Marine *pMarine = pMR->GetMarineEntity();
		if ( !pMarine )
		{
			continue;
		}

		// Only start shakes for players that are on the ground unless doing an air shake.
		if ( !bAirShake && !pMarine->m_bOnGround )
			continue;

		if ( pOnlyMarine && pMarine != pOnlyMarine )
			continue;

		Vector vecMarinePos = pMarine->WorldSpaceCenter();
		if ( pMarine->IsControllingTurret() && pMarine->GetRemoteTurret() )
			vecMarinePos = pMarine->GetRemoteTurret()->GetAbsOrigin();

		localAmplitude = ASW_ComputeShakeAmplitude( center, vecMarinePos, amplitude, radius );

		// This happens if the player is outside the radius, in which case we should ignore 
		// all commands
		if (localAmplitude < 0)
			continue;

		ASW_TransmitShakeEvent( pMarine, localAmplitude, frequency, duration, eCommand );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Perform a directional "punch" on the screen of all clients within radius.
//			radius == 0, shake all clients
// Input  : center - Center of screen shake, radius is measured from here.
//          direction - (world space) direction in which to punch camera. punching down makes it look like the world is moving up. must be normal.
//			amplitude - Amplitude of shake, in world units for the camera
//			frequency - controls number of bounces before shake settles; a frequency of 1 means three peaks (forward, back, little forward, settle)
//			duration - duration of shake in seconds.
//			radius - Radius of effect, 0 shakes all clients.
//-----------------------------------------------------------------------------
void UTIL_ASW_ScreenPunch( const Vector &center, const Vector &direction, float amplitude, float frequency, float duration, float radius )
{
	ScreenShake_t shake;
	shake.command = SHAKE_START;
	shake.direction = direction;
	shake.amplitude = amplitude;
	shake.frequency = frequency;
	shake.duration = duration;

	UTIL_ASW_ScreenPunch( center, radius, shake );
}

void UTIL_ASW_ScreenPunch( const Vector &center, float radius, const ScreenShake_t &shake )
{
	int			i;
	const float radiusSqr = radius * radius;

	AssertMsg( CloseEnough(shake.direction.LengthSqr(), 1), "Direction param to ASW_ScreenPunch is abnormal\n" );

	CASW_Game_Resource *pGameResource = ASWGameResource();
	if ( !pGameResource )
	{
		return;
	}

	for ( i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
	{
		CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
		if ( !pMR )
		{
			continue;
		}

		CASW_Marine *pMarine = pMR->GetMarineEntity();
		if ( !pMarine )
		{
			continue;
		}

		Vector vecMarinePos = pMarine->WorldSpaceCenter();
		if ( pMarine->IsControllingTurret() && pMarine->GetRemoteTurret() )
			vecMarinePos = pMarine->GetRemoteTurret()->GetAbsOrigin();

		if ( vecMarinePos.DistToSqr(center) > radiusSqr )
			continue;

		ASW_TransmitShakeEvent( pMarine, shake );
	}
}


// returns the nearest marine to this point
CASW_Marine* UTIL_ASW_NearestMarine( const Vector &pos, float &marine_distance, ASW_Marine_Class marineClass, bool bAIOnly )
{
	// check through all marines, finding the closest that we're aware of
	CASW_Game_Resource* pGameResource = ASWGameResource();
	float distance = 0.0f;
	marine_distance = -1.0f;
	CASW_Marine *pNearest = NULL;
	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
		if (pMR!=NULL && pMR->GetMarineEntity()!=NULL && pMR->GetMarineEntity()->GetHealth() > 0)
		{
			if ( bAIOnly && pMR->IsInhabited() )
				continue;
			if ( marineClass != MARINE_CLASS_UNDEFINED && pMR->GetProfile() && pMR->GetProfile()->GetMarineClass() != marineClass )
				continue;

			distance = pMR->GetMarineEntity()->GetAbsOrigin().DistTo(pos);
			if (marine_distance == -1.0f || distance < marine_distance)
			{
				marine_distance = distance;
				pNearest = pMR->GetMarineEntity();
			}
		}
	}
	return pNearest;
}

CASW_Marine* UTIL_ASW_NearestMarine( const CASW_Marine *pMarine, float &marine_distance )
{
	// check through all marines, finding the closest that we're aware of
	CASW_Game_Resource* pGameResource = ASWGameResource();
	float distance = 0;
	marine_distance = -1.0f;
	CASW_Marine *pNearest = NULL;
	for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
	{
		CASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
		if ( pMR != NULL && pMR->GetMarineEntity() != NULL && pMR->GetMarineEntity() != pMarine && pMR->GetMarineEntity()->GetHealth() > 0 )
		{
			distance = pMR->GetMarineEntity()->GetAbsOrigin().DistTo( pMarine->GetAbsOrigin() );
			if ( marine_distance == -1.0f || distance < marine_distance )
			{
				marine_distance = distance;
				pNearest = pMR->GetMarineEntity();
			}
		}
	}
	return pNearest;
}

int UTIL_ASW_NumCommandedMarines( const CASW_Player *pPlayer )
{
	int nNumMarines = 0;

	// check through all marines
	CASW_Game_Resource* pGameResource = ASWGameResource();
	for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
	{
		CASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
		if ( pMR && pMR->GetMarineEntity() && pMR->GetMarineEntity()->GetHealth() > 0 )
		{
			if ( pMR->GetMarineEntity()->GetCommander() == pPlayer )
			{
				nNumMarines++;
			}
		}
	}
	return nNumMarines;
}

#else
	// make a specific clientside entity gib
	bool UTIL_ASW_ClientsideGib(C_BaseAnimating* pEnt)
	{
		if (!pEnt)
			return false;
		C_BaseAnimating* pAnimating = dynamic_cast<C_BaseAnimating*>(pEnt);
		if (!pAnimating)
			return false;
		if (!stricmp(STRING(pAnimating->GetModelName()), SWARM_DRONE_MODEL))
		{
			Vector vMins, vMaxs, vGibOrigin, vGibVelocity(0,0,1);
			if ( pEnt->m_pRagdoll )
			{
				pEnt->m_pRagdoll->GetRagdollBounds( vMins, vMaxs );
				vGibOrigin =pEnt->m_pRagdoll->GetRagdollOrigin() + ( ( vMins + vMaxs ) / 2.0f );
				pEnt->m_pRagdoll->GetElement(0)->GetVelocity( &vGibVelocity, NULL );
			}
			else
			{
				vGibOrigin = pEnt->WorldSpaceCenter();
			}

			FX_DroneGib( vGibOrigin, Vector(0,0,1), 0.5f, pAnimating->GetSkin(), pEnt->IsOnFire() );
			return true;
		}
		else if (!stricmp(STRING(pAnimating->GetModelName()), SWARM_HARVESTER_MODEL))
		{
			FX_HarvesterGib( pEnt->WorldSpaceCenter(), Vector(0,0,1), 0.5f, pAnimating->GetSkin(), pEnt->IsOnFire() );
			return true;
		}
		else if (!stricmp(STRING(pAnimating->GetModelName()), SWARM_SHIELDBUG_MODEL))
		{
			FX_HarvesterGib( pEnt->WorldSpaceCenter(), Vector(0,0,1), 0.5f, 1, pEnt->IsOnFire() );
			return true;
		}
		// todo: code to gib other types of things clientside?
		return false;
	}
#endif

//void UTIL_ASW_ValidateSoundName( string_t &name, const char *defaultStr )
void UTIL_ASW_ValidateSoundName( char *szString, int stringlength, const char *defaultStr )
{
	Assert(szString);
	if (szString[0] == '\0')
	{
		Q_snprintf(szString, stringlength, "%s", defaultStr);
	}
}

#ifdef GAME_DLL
void UTIL_ASW_PoisonBlur( CASW_Marine *pMarine, float duration )
{
	if ( !pMarine )
		return;

	CASW_ViewNPCRecipientFilter user( pMarine );
	user.MakeReliable();

	UserMessageBegin( user, "ASWBlur" );
		WRITE_ENTITY( pMarine->entindex() );
		WRITE_SHORT( (int) (duration * 10.0f) );		// blur lasts this long / 10
	MessageEnd();
}

// tests if a particular entity is blocking any marines (used by phys props to see if they should leave pushaway mode)
bool UTIL_ASW_BlockingMarine( CBaseEntity *pEntity )
{
	CASW_Game_Resource* pGameResource = ASWGameResource();
	if (!pGameResource)
		return false;

	int iCurrentGroup = pEntity->GetCollisionGroup();
	pEntity->SetCollisionGroup(COLLISION_GROUP_NONE);
	
	bool bBlockedMarine = false;
	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource* pMR = pGameResource->GetMarineResource(i);
		if (pMR!=NULL && pMR->GetMarineEntity()!=NULL && pMR->GetMarineEntity()->GetHealth() > 0)
		{
			CASW_Marine *pMarine = pMR->GetMarineEntity();
			// check if this marine's bounding box trace collides with the specified entity
			Ray_t ray;
			trace_t tr;
			ray.Init( pMarine->GetAbsOrigin(), pMarine->GetAbsOrigin() - Vector(0,0,1),
					pMarine->CollisionProp()->OBBMins(), pMarine->CollisionProp()->OBBMaxs() );
			if (pEntity->TestCollision( ray, MASK_PLAYERSOLID, tr ))
			{
				bBlockedMarine = true;
				break;
			}
		}
	}

	pEntity->SetCollisionGroup(iCurrentGroup);

	return bBlockedMarine;
}

CASW_Marine* UTIL_ASW_Marine_Can_Chatter_Spot(CBaseEntity *pEntity, float fDist)
{	
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return NULL;

	// find how many marines can see us
	int iFound = 0;
	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource* pMarineResource = pGameResource->GetMarineResource(i);
		if (!pMarineResource)
			continue;

		CASW_Marine *pMarine = pMarineResource->GetMarineEntity();
		if (!pMarine)
			continue;

		if (pMarine->GetAbsOrigin().DistTo(pEntity->GetAbsOrigin()) < fDist)
		{
			Vector vecFacing;
			AngleVectors(pMarine->GetAbsAngles(), &vecFacing);
			Vector vecDir = pEntity->GetAbsOrigin() - pMarine->GetAbsOrigin();
			vecDir.NormalizeInPlace();
			if (vecFacing.Dot(vecDir) > 0.5f)
				iFound++;				
		}
	}
	if (iFound <= 0)
		return NULL;

	// randomly pick one
	int iChosen = random->RandomInt(0, iFound-1);
	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource* pMarineResource = pGameResource->GetMarineResource(i);
		if (!pMarineResource)
			continue;

		CASW_Marine *pMarine = pMarineResource->GetMarineEntity();
		if (!pMarine)
			continue;

		if (pMarine->GetAbsOrigin().DistTo(pEntity->GetAbsOrigin()) < 600.0f)
		{
			Vector vecFacing;
			AngleVectors(pMarine->GetAbsAngles(), &vecFacing);
			Vector vecDir = pEntity->GetAbsOrigin() - pMarine->GetAbsOrigin();
			vecDir.NormalizeInPlace();
			if (vecFacing.Dot(vecDir) > 0.5f)
			{
				if (iChosen <= 0)
					return pMarine;
				iChosen--;
			}
		}
	}
	return NULL;
}

CASW_ViewNPCRecipientFilter::CASW_ViewNPCRecipientFilter( CASW_Inhabitable_NPC *pNPC, bool bSendToRecorders )
{
	for ( int i = 1; i <= MAX_PLAYERS; i++ )
	{
		CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
		if ( pPlayer && ( pPlayer->GetViewNPC() == pNPC || ( bSendToRecorders && ( pPlayer->IsAnyBot() || V_atoi( engine->GetClientConVarValue( pPlayer->entindex(), "rd_auto_record_lobbies" ) ) ) ) ) )
		{
			AddRecipient( pPlayer );
		}
	}
}

#endif

//-----------------------------------------------------------------------------
// Purpose: Trace filter that only hits aliens (all NPCS but the marines, eggs, goo)
//-----------------------------------------------------------------------------
bool CTraceFilterAliensEggsGoo::ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
{
	if ( CTraceFilterSimple::ShouldHitEntity(pServerEntity, contentsMask) )
	{		
#ifndef CLIENT_DLL
		CBaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );
		if ( pEntity->Classify() == CLASS_ASW_MARINE )		// we dont hit marines in coop
			return ASWDeathmatchMode() != NULL;				// but hit them for deathmatch

		if ( IsAlienClass( pEntity->Classify() ) )
			return true;
#endif // !CLIENT_DLL
	}
	return false;
}

// NOTE: This function assumes 75 fov and 4:3 ratio (todo: support widescreen all the time?)
bool CanFrustumSee(const Vector &vecCameraCenter, const QAngle &angCameraFacing, 
				   const Vector &pos, const int padding, const int forward_limit, float fov=75.0f)
{	
	Vector vForward, vRight, vUp;
	AngleVectors(angCameraFacing, &vForward, &vRight, &vUp);

	Vector vecTestPos = pos;
	// bring in the x coord by the padding
	if (vecTestPos.x < vecCameraCenter.x)
		vecTestPos.x = MIN(vecCameraCenter.x, vecTestPos.x + padding);
	else if (vecTestPos.x > vecCameraCenter.x)
		vecTestPos.x = MAX(vecCameraCenter.x, vecTestPos.x - padding);
	// bring in the y coord by the padding
	if (vecTestPos.y < vecCameraCenter.y)
		vecTestPos.y = MIN(vecCameraCenter.y, vecTestPos.y + padding);
	else if (vecTestPos.y > vecCameraCenter.y)
		vecTestPos.y = MAX(vecCameraCenter.y, vecTestPos.y - padding);

	float ratio = 4.0f / 3.0f;	// assume 4:3 res	
	float fov_tangent = tan(DEG2RAD(fov) * 0.5f);

	Vector vDiff = vecTestPos - vecCameraCenter;	// vector from camera to testing position
	
	float forward_diff = vDiff.Dot(vForward);
	if (forward_diff < 0)	// behind the camera
		return false;
	if (forward_limit > 0 && forward_diff > forward_limit)
		return false;	// too far away

	//int padding_at_this_distance = padding * (forward_diff / 405.0f);	// adjust padding by ratio of distance to default camera height
	float up_diff = vDiff.Dot(vUp);	
	float max_up_diff = forward_diff * fov_tangent;

	if (up_diff > max_up_diff || up_diff < -max_up_diff)
		return false;
	
	float right_diff = vDiff.Dot(vRight);	
	float max_right_diff = max_up_diff * ratio;

	if (right_diff > max_right_diff || right_diff < -max_right_diff)
		return false;
			
	return true;
}

CASW_Marine* UTIL_ASW_MarineCanSee(CASW_Marine_Resource* pMarineResource, const Vector &pos, const int padding, bool &bCorpseCanSee, const int forward_limit)
{
	if (!pMarineResource)
		return NULL;

	Vector vecMarinePos;
	CASW_Marine* pMarine = NULL;
#ifndef CLIENT_DLL
	if (pMarineResource->GetHealthPercent() <=0 || !pMarineResource->IsAlive())	// if we're dead, take the corpse position
	{
		vecMarinePos = pMarineResource->m_vecDeathPosition;
		// reactivedrop: this makes sure that dead marine is not considered as the one
		// who sees aliens, thus preventing them from going into sleep state.
		// This fixes the bug: When marine dies to a horde and other marines are far
		// enough for horde to see them, the horde gets stuck in non-sleeping state
		// and thus not decreasing the ammount of non-sleeping aliens. This leads to
		// no hordes and wanderers being spawned at all and can be exploited on hard
		// challenges and maps like Survival Desert. 
		if ( gpGlobals->curtime > pMarineResource->m_fDeathTime + 7.0f )	// After 6 seconds of marine's death player is switched to spectating alive marines, so after 7 seconds it is safe to make aliens that killed this marine enter the sleep state
			return NULL;
	}
	else
#endif
	{
		pMarine = pMarineResource->GetMarineEntity();
		if (!pMarine)
			return NULL;

		vecMarinePos = pMarine->GetAbsOrigin();
	}
	
	// note: assumes 60 degree pitch camera and 405 dist (actual convars for these are on the client...)
	QAngle angCameraFacing(60, 90, 0);
	Vector vForward, vRight, vUp;
	AngleVectors(angCameraFacing, &vForward, &vRight, &vUp);
	Vector vecCameraCenter = vecMarinePos - vForward * 405;

	// see if they're beyond the fog plane	
#ifdef CLIENT_DLL
	C_ASW_Player *pPlayer = C_ASW_Player::GetLocalASWPlayer();
	if (pPlayer)
	{		
		if (pPlayer->GetPlayerFog().m_hCtrl->m_fog.enable)
		{
			float dist = (vecCameraCenter - pos).Length();
			if (dist > pPlayer->GetPlayerFog().m_hCtrl->m_fog.end)
				return NULL;
		}
	}
#else
	// ASWTODO - no worldfogparams anymore?
	/*
	fogparams_t fog;
	GetWorldFogParams(fog);	
	if (fog.enable.Get())
	{
		float dist = (vecCameraCenter - pos).Length();
		if (dist > fog.end.Get())
			return NULL;
	}
	*/
#endif	

	// check for plain near the marine
	bool bNearby = CanFrustumSee(vecCameraCenter, angCameraFacing, pos, padding, forward_limit);
	// check if he's looking through a remote turret and can possibly see us from that
	if (!bNearby && pMarine && pMarine->IsControllingTurret() && pMarine->GetRemoteTurret())
	{
		// a turret can look in any direction, so let's just do a radius check (would match up with the fog anyways)
		bNearby = (pMarine->GetRemoteTurret()->GetAbsOrigin().DistTo(pos) <= 1024);	// assume fog distance of 1024 when in first person
	}
	// check if he's looking through a security cam
	if (!bNearby && pMarine)
	{
		CBaseEntity* pUsing = pMarine->m_hUsingEntity.Get();
		if ( pUsing && pUsing->Classify() == CLASS_ASW_COMPUTER_AREA )
		{
			CASW_Computer_Area* pComputer = assert_cast<CASW_Computer_Area*>(pUsing);
			CPointCamera *pCam = pComputer->GetActiveCam();
			if (pCam)
			{
				Vector vecCamFacing;
				AngleVectors(pCam->GetAbsAngles(), &vecCamFacing);
				bNearby = (vecCamFacing.Dot(pos - pCam->GetAbsOrigin()) > 0) &&		// check facing
					(pCam->GetAbsOrigin().DistTo(pos) <= 1024);	// assume fog distance of 1024 when in first person
			}
		}
	}
#ifndef CLIENT_DLL
	if (asw_debug_marine_can_see.GetBool())
	{
		if (bNearby)
		{
			NDebugOverlay::Line(pos, vecMarinePos + Vector(0,0,10), 255, 255, 0, true, 0.1f);
		}
		else
		{
			NDebugOverlay::Line(pos, vecMarinePos + Vector(0,0,10), 255, 0, 0, true, 0.1f);
		}
	}
#endif
	if (bNearby)
	{
		if (!pMarine)
			bCorpseCanSee = true;
		return pMarine;
	}

	return NULL;
}

CASW_Marine* UTIL_ASW_AnyMarineCanSee(const Vector &pos, const int padding, bool &bCorpseCanSee, const int forward_limit)
{
	// find the closest marine
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return NULL;

	bCorpseCanSee = false;
	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource* pMarineResource = pGameResource->GetMarineResource(i);
		bool bCorpse = false;
		CASW_Marine *pMarine = (UTIL_ASW_MarineCanSee(pMarineResource, pos, padding, bCorpse, forward_limit));
		bCorpseCanSee |= bCorpse;
		if (pMarine)
			return pMarine;
	}
	return NULL;
}

bool UTIL_ASW_MarineViewCone(const Vector &pos)
{
	// find the closest marine
	CASW_Game_Resource *pGameResource = ASWGameResource();
	if (!pGameResource)
		return false;

	for (int i=0;i<pGameResource->GetMaxMarineResources();i++)
	{
		CASW_Marine_Resource* pMarineResource = pGameResource->GetMarineResource(i);
		if (!pMarineResource)
			continue;

		Vector vecMarinePos;
		CASW_Marine* pMarine = pMarineResource->GetMarineEntity();
		if (!pMarine)
			continue;

		// check it's not too far away
		Vector vecDiff = pos - pMarine->GetAbsOrigin();
		if (vecDiff.LengthSqr() > asw_marine_view_cone_dist.GetFloat())		
			continue;

		// check dot
		Vector vecFacing;
		AngleVectors(pMarine->EyeAngles(), &vecFacing);
		float dot = vecDiff.Dot(vecFacing);
		if (dot < asw_marine_view_cone_dot.GetFloat())
			continue;

		return true;
	}
	return false;
}

#ifdef CLIENT_DLL
extern ConVar asw_cam_mode;
extern ConVar joy_pan_camera;
#else
extern ConVar asw_debug_medals;
extern ConVar asw_wire_full_random;
#endif

float UTIL_ASW_CalcFastDoorHackTime(int iNumRows, int iNumColumns, int iNumWires, int iHackLevel, float fSpeedScale)
{		
	float ideal_time = 1.0f;
	// assume 0.5 seconds per row
	float seconds_per_column = 0.5f;
	if (iNumRows == 2)
		seconds_per_column = 1.0f;
	else if (iNumRows == 3)
		seconds_per_column = 1.5f;
	
	float time_to_assemble_wire = seconds_per_column * iNumColumns;		
#ifndef CLIENT_DLL
	if (!asw_wire_full_random.GetBool())
	{
		time_to_assemble_wire = 3.0f;	// assumes 5 mistakes per wire
	}
	if (asw_debug_medals.GetBool())
		Msg("time_to_assemble_wire = %f\n", time_to_assemble_wire);
#endif
	// ok so after this amount of time, the wire would be charging	
	if (iNumWires <= 0)
		iNumWires = 1;
	float speed_per_wire = 1.0f / iNumWires;
	speed_per_wire *= fSpeedScale;
	
	float charge_before_assembling_wire_2 = speed_per_wire * time_to_assemble_wire;
	if (charge_before_assembling_wire_2 >= iHackLevel || iNumWires < 2)
	{
		// if we're here, it means we would have finished the hack before wire two was assembled
		// so the ideal time is just how long it takes to charge up with 1 wire, plus the time it took us to assemble
		ideal_time = (float(iHackLevel) / speed_per_wire) + time_to_assemble_wire;
	}
	else
	{
		// if we're in here, then wire 1 and 2 will be charging
		float charge_before_assembling_wire_3 = charge_before_assembling_wire_2
												+ speed_per_wire * time_to_assemble_wire * 2;	// wire 2's contribution
		if (charge_before_assembling_wire_3 >= iHackLevel || iNumWires < 3)
		{
			// if we're here, it means we would have finished the hack before wire three was assembled
			float first_wire_time = time_to_assemble_wire + time_to_assemble_wire;
			float duo_charge = float(iHackLevel) - charge_before_assembling_wire_2;	// how much charge up to do with both wires
			ideal_time = (duo_charge / (speed_per_wire * 2)) + first_wire_time;
		}
		else
		{
			// if we're in here, then wires 1, 2 and 3 will be charging
			float charge_before_assembling_wire_4 = charge_before_assembling_wire_3
												+ speed_per_wire * time_to_assemble_wire * 3;	// wire 3's contribution
			if (charge_before_assembling_wire_4 >= iHackLevel || iNumWires < 4)
			{
				// if we're here, it means we would have finished the hack before wire 4 was assembled
				float first_wire_time = time_to_assemble_wire + time_to_assemble_wire;
				float two_wire_time = time_to_assemble_wire + first_wire_time;
				float triple_charge = float(iHackLevel) - charge_before_assembling_wire_3;	// how much charge do we with 3 wires
				ideal_time = (triple_charge / (speed_per_wire * 3)) + two_wire_time;
			}
			else
			{
				// if we're in here, then wires 1,2,3 and 4 will be charging
				float first_wire_time = time_to_assemble_wire + time_to_assemble_wire;
				float two_wire_time = time_to_assemble_wire + first_wire_time;
				float three_wire_time = time_to_assemble_wire + two_wire_time;
				float quad_charge = float(iHackLevel) - charge_before_assembling_wire_4;	// how much charge do we with 3 wires
				ideal_time = (quad_charge / (speed_per_wire * 4)) + three_wire_time;
			}
		}
	}

	int iSkill = ASWGameRules() ? ASWGameRules()->GetSkillLevel() : 2;
	if (iSkill == 1)
		ideal_time *= 1.05f;	// 5% slower on easy mode
	else if (iSkill == 3)
		ideal_time *= 0.95f;	// 5% faster on hard mode
	else if (iSkill == 4 || iSkill == 5)
		ideal_time *= 0.90f;	// 10% faster on insane mode
	
	return ideal_time;
}

int UTIL_ASW_GetNumPlayers()
{
	int count = 0;
	for (int i=0;i<MAX_PLAYERS;i++)
	{
		// found a connected player who isn't ready?
#ifdef CLIENT_DLL
		if (g_PR && g_PR->IsConnected(i+1))
			count++;
#else
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(i + 1);
		// if they're not connected, skip them
		if (pPlayer && pPlayer->IsConnected())
			count++;
#endif
	}

	return count;
}

bool UTIL_ASW_MissionHasBriefing(const char* mapname)
{
	bool bSpecialMap = (!Q_strnicmp(mapname, "intro_", 6) ||
			!Q_strnicmp(mapname, "outro_", 6) ||
			!Q_strnicmp(mapname, "tutorial", 8) ||
			!Q_strnicmp(mapname, "rdselectionscreen", 20));

	return !bSpecialMap;
}

bool ASW_IsSecurityCam(CPointCamera *pCameraEnt)
{
	CASW_PointCamera *pASW_Cam = dynamic_cast<CASW_PointCamera*>(pCameraEnt);
	return pASW_Cam && pASW_Cam->m_bSecurityCam;
}

// copies a string
char* ASW_AllocString( const char *szString )
{
	if ( !szString )
		return NULL;

	int len = Q_strlen( szString ) + 1;
	if ( len <= 1 )
		return NULL;

	char *text = new char[ len ];
	Q_strncpy( text, szString, len );
	return text;
}


#ifdef CLIENT_DLL
CNewParticleEffect *UTIL_ASW_CreateFireEffect( C_BaseEntity *pEntity )
{
	CNewParticleEffect	*pBurningEffect = pEntity->ParticleProp()->Create( "ent_on_fire", PATTACH_ABSORIGIN_FOLLOW );
	if (pBurningEffect)
	{
		Vector vecOffest1 = (pEntity->WorldSpaceCenter() - pEntity->GetAbsOrigin()) + Vector( 0, 0, 16 );
		pEntity->ParticleProp()->AddControlPoint( pBurningEffect, 1, pEntity, PATTACH_ABSORIGIN_FOLLOW, NULL, vecOffest1 );

		// all bounding boxes are the same, skip this for now
		Vector vecSurroundMins, vecSurroundMaxs;
		vecSurroundMins = pEntity->CollisionProp()->OBBMins();
		vecSurroundMaxs = pEntity->CollisionProp()->OBBMaxs();

		// this sets the maximum bounds for scaling up or down the fire
		float flMaxBounds = 34.0;
		flMaxBounds = MAX( flMaxBounds, vecSurroundMaxs.x - vecSurroundMins.x );
		flMaxBounds = MAX( flMaxBounds, vecSurroundMaxs.y - vecSurroundMins.y );
		flMaxBounds = MAX( flMaxBounds, vecSurroundMaxs.z - vecSurroundMins.z );

		float flScalar = 1.0f;
		flMaxBounds /= 115.0f;
		flMaxBounds = clamp( flMaxBounds, 0.75f, 1.75f );

		// position 0 of CP2 controls the scale of the flames, we want to scale them a bit based on how big the creature is
		// position 1 is the number generated
		if ( flMaxBounds > 220 )
			flScalar = 2.0f;

		pBurningEffect->SetControlPoint( 2, Vector( flMaxBounds, flMaxBounds * flScalar, 0 ) );
	}
	return pBurningEffect;
}

ConVar asw_floating_number_type( "asw_floating_number_type", "0", FCVAR_ARCHIVE, "1 = vgui, 2 = particles" );

void UTIL_ASW_ClientFloatingDamageNumber( const CTakeDamageInfo &info )
{
	// TODO: Move this to some rendering step?
	if ( asw_floating_number_type.GetInt() == 1 )
	{
		Vector screenPos;

		Vector vecPos = info.GetDamagePosition(); // WorldSpaceCenter()
		//debugoverlay->ScreenPosition( vecPos, screenPos );

		floating_number_params_t params;
		params.x = 0;//screenPos.x;
		params.y = 0;//screenPos.y;
		params.bShowPlus = false;
		//params.hFont = m_fontLargeFloatingText;
		params.flMoveDuration = 0.9f;
		params.flFadeStart = 0.6f;
		params.flFadeDuration = 0.3f;
		params.rgbColor = Color( 200, 200, 200, 255 );
		if ( info.GetDamageCustom() & DAMAGE_FLAG_WEAKSPOT )
		{
			params.rgbColor = Color( 255, 170, 150, 255 );
			params.flFadeStart = 0.65f;
			params.flFadeDuration = 0.4f;
			params.flMoveDuration = 0.95f;
		}
		params.alignment = vgui::Label::a_center;
		params.bWorldSpace = true;
		params.vecPos = vecPos;

		new CFloatingNumber( (int) info.GetDamage(), params, GetClientMode()->GetViewport() );
	}
	else if ( asw_floating_number_type.GetInt() == 2 )
	{
		if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_MARINE )
		{
			C_ASW_Marine* pMarine = assert_cast<C_ASW_Marine*>(info.GetAttacker());

			C_ASW_Player* pAttackingPlayer = pMarine->GetCommander();
			if (!pAttackingPlayer)
				return;

			if (pAttackingPlayer != C_BasePlayer::GetLocalPlayer())
				return;

			UTIL_ASW_ParticleDamageNumber(info.GetAttacker(), info.GetDamagePosition(), int(info.GetDamage()), info.GetDamageCustom(), 1.0f, false);
		}
	}
}

PRECACHE_REGISTER_BEGIN( GLOBAL, ParticleDamageNumbers )
	PRECACHE( PARTICLE_SYSTEM, "damage_numbers" )
	PRECACHE( PARTICLE_SYSTEM, "floating_numbers" )
PRECACHE_REGISTER_END()

HPARTICLEFFECT UTIL_ASW_ParticleDamageNumber( C_BaseEntity *pEnt, Vector vecPos, int iDamage, int iDmgCustom, float flScale, bool bRandomVelocity )
{
	if ( asw_floating_number_type.GetInt() != 2 )
		return NULL;

	if ( !pEnt )
		return NULL;

	QAngle vecAngles;
	vecAngles[PITCH] = 0.0f;
	vecAngles[YAW] = ASWInput()->ASW_GetCameraYaw();
	vecAngles[ROLL] = ASWInput()->ASW_GetCameraPitch();

	//Msg( "DMG # angles ( %f, %f, %f )\n", vecAngles[PITCH], vecAngles[YAW], vecAngles[ROLL] );
	Vector vecForward, vecRight, vecUp;
	AngleVectors( vecAngles, &vecForward, &vecRight, &vecUp );

	Color cNumber = Color( 255, 240, 240 );

	int iCrit = 0;
	float flNewScale = MAX( flScale, 1.0f );
	float flLifetime = 1.0f;
	int r, g, b;
	if ( iDmgCustom & DAMAGE_FLAG_CRITICAL )
	{
		flNewScale *= 1.8f;
		flLifetime = 3.0f;
		iCrit = 1;
		cNumber = Color( 255, 0, 0 );
	}
	else if ( iDmgCustom & DAMAGE_FLAG_WEAKSPOT )
	{
		flNewScale *= 1.3f;
		flLifetime = 1.25f;
		cNumber = Color( 255, 128, 128 );
	}
	else if ( iDmgCustom & DAMAGE_FLAG_T75 )
	{
		flNewScale = 1.3f;
		cNumber = Color( 255, 0, 0 );
		// TODO: Stop these numbers from moving randomly
	}

	r = cNumber.r();
	g = cNumber.g();
	b = cNumber.b();

	CUtlReference<CNewParticleEffect> pEffect;
	if ( bRandomVelocity )
	{
		pEffect = pEnt->ParticleProp()->Create( "damage_numbers", PATTACH_CUSTOMORIGIN );
	}
	else
	{
		pEffect = pEnt->ParticleProp()->Create( "floating_numbers", PATTACH_CUSTOMORIGIN );
	}
	pEffect->SetControlPoint( 0, vecPos );
	pEffect->SetControlPoint( 1, Vector( 0, iDamage, iCrit ) );
	pEffect->SetControlPoint( 2, Vector( r, g, b ) );
	pEffect->SetControlPoint( 3, Vector( flNewScale, flLifetime, 0 ) );
	pEffect->SetControlPointOrientation( 5, vecForward, vecRight, vecUp );
	return pEffect;
}

void __MsgFunc_ASWDamageNumber( bf_read &msg )
{
	int iAmount = msg.ReadShort();
	int iFlags = msg.ReadShort();
	int iEntIndex = msg.ReadShort();		
	C_BaseEntity *pEnt = iEntIndex > 0 ? ClientEntityList().GetEnt( iEntIndex ) : NULL;
	if ( !pEnt )
		return;

	if ( asw_floating_number_type.GetInt() == 1 )
	{
		Vector vecPos;
		vecPos.x = msg.ReadFloat();
		vecPos.y = msg.ReadFloat();
		vecPos.z = msg.ReadFloat();

		if ( pEnt )
		{
			vecPos = pEnt->WorldSpaceCenter();
		}

		Vector screenPos;
		debugoverlay->ScreenPosition( vecPos, screenPos );

		floating_number_params_t params;
		params.x = 0;//screenPos.x;
		params.y = 0;// screenPos.y;
		params.bShowPlus = false;
		//params.hFont = m_fontLargeFloatingText;
		params.flMoveDuration = 0.85f;
		params.flFadeStart = 0.6f;
		params.flFadeDuration = 0.3f;
		params.rgbColor = Color( 200, 200, 200, 255 );
		if ( iFlags & DAMAGE_FLAG_WEAKSPOT )
		{
			params.rgbColor = Color( 255, 170, 150, 255 );
			params.flFadeStart = 0.65f;
			params.flFadeDuration = 0.4f;
			params.flMoveDuration = 0.95f;
		}

		params.alignment = vgui::Label::a_center;
		params.bWorldSpace = true;
		params.vecPos = vecPos;

		new CFloatingNumber( iAmount, params, GetClientMode()->GetViewport() );
	}
	else if ( asw_floating_number_type.GetInt() == 2 )
	{
		UTIL_ASW_ParticleDamageNumber( pEnt, pEnt->WorldSpaceCenter(), iAmount, iFlags, 1.25f, false );
	}
}
USER_MESSAGE_REGISTER( ASWDamageNumber );
#endif

// attempts to localize a string
//  if it fails, it just fills the destination with the token name
void TryLocalize( const char *token, wchar_t *unicode, int unicodeBufferSizeInBytes )
{
	if ( token[0] == '#' )
	{
		wchar_t *pLocalized = g_pVGuiLocalize->Find( token );
		if ( pLocalized )
		{
			V_snwprintf( unicode, unicodeBufferSizeInBytes / sizeof( wchar_t ), L"%s", pLocalized );
			return;
		}
	}
	g_pVGuiLocalize->ConvertANSIToUnicode( token, unicode, unicodeBufferSizeInBytes );
}

/// @desc This function can be used as a convenience for when you want to
/// rapidly experiment with different screenshakes for a gameplay feature.
/// You have a single "scratchpad" screen shake which you can fill out with 
/// the concommand asw_shake_setscratch . 
/// Then you can read it in code with the ASW_DefaultScreenShake. 
/// So, the way you use it is,
/// if you have a function Kaboom() that needs to do a screenpunch,
/// but you don't know what numbers you want for that punch yet, 
/// you write the function to use the default screen shake:
///
/// void Kaboom() {  
///   ASW_TransmitShakeEvent( player, ASW_DefaultScreenShake() );
/// }
/// 
/// and then, while the game is running, you can fiddle the numbers around 
/// with asw_shake_setscratch  and try the Kaboom() function over and over
/// again to see the results without having to recompile.
/// Once you have numbers you are happy with, you can go back and hardcode
/// them into Kaboom(), freeing up the "Default" shake to be used somewhere
/// else.
ScreenShake_t ASW_DefaultScreenShake( void )
{
	return ScreenShake_t( SHAKE_START,
		asw_shake_test_punch_amp.GetFloat(),
		asw_shake_test_punch_freq.GetFloat(),
		asw_shake_test_punch_dura.GetFloat(),
		Vector( asw_shake_test_punch_dirx.GetFloat(), asw_shake_test_punch_diry.GetFloat(), asw_shake_test_punch_dirz.GetFloat() ) 
		);
}

static void ASW_PrintDefaultScreenShake( void )
{
	// x y z f a d
	Msg( "< %.3f,%.3f,%.3f > %.3f %.3f %.3f\n",
		asw_shake_test_punch_dirx.GetFloat(), asw_shake_test_punch_diry.GetFloat(), asw_shake_test_punch_dirz.GetFloat(),
		asw_shake_test_punch_freq.GetFloat(),
		asw_shake_test_punch_amp.GetFloat(),
		asw_shake_test_punch_dura.GetFloat()
		);
}

#ifndef CLIENT_DLL
/// convenient console command for setting the default screen shake parameters

//-----------------------------------------------------------------------------
// Purpose: Test a punch-type screen shake
//-----------------------------------------------------------------------------
static void CC_ASW_Shake_SetScratch( const CCommand &args )
{
	if ( args.ArgC() < 7 )
	{
		Msg("Usage: %s x y z f a d\n"
			"where x,y,z are direction of screen punch\n"
			"      f     is  frequency (1 means three bounces before settling)\n"
			"      a     is  amplitude\n"
			"      d     is  duration\n"
			"you can specify a direction 0 0 0 to mean a classic 'vibrating' shake rather than a directional punch.\n"
			"The current default screen shake is:\n\t",
			args[0]
			);
		ASW_PrintDefaultScreenShake();
	}

	const float x = atof( args[1] );
	const float y = atof( args[2] );
	const float z = atof( args[3] );
	const float f = atof( args[4] );
	const float a = atof( args[5] );
	const float d = atof( args[6] );


	asw_shake_test_punch_dirx.SetValue( x ); 
	asw_shake_test_punch_diry.SetValue( y ); 
	asw_shake_test_punch_dirz.SetValue( z );
	asw_shake_test_punch_freq.SetValue( f );
	asw_shake_test_punch_amp.SetValue( a );
	asw_shake_test_punch_dura.SetValue( d );
}
static ConCommand asw_shake_setscratch("asw_shake_setscratch", CC_ASW_Shake_SetScratch, "Set values for the \"default\" screenshake used for rapid iteration.\n", FCVAR_CHEAT | FCVAR_DEVELOPMENTONLY );

#endif //#ifndef CLIENT_DLL


/// get a parabola that goes from source to destination in specified time
Vector UTIL_LaunchVector( const Vector &src, const Vector &dest, float gravity, float flightTime )
{
	Assert( gravity > 0 );
	Assert( !AlmostEqual(src,dest) );

	if ( flightTime == 0.0f )
	{
		flightTime = MAX( 0.8f, sqrt( ( (dest - src).Length2D() * 1.5f ) / gravity ) );
	}

	// delta high from start to end
	float H = dest.z - src.z ; 
	// azimuth vector
	Vector azimuth = dest-src;
	azimuth.z = 0;
	// get horizontal distance start to end
	float D = azimuth.Length2D();
	// normalize azimuth
	azimuth /= D;

	float Vy = ( H / flightTime + 0.5 * gravity * flightTime );
	float Vx = ( D / flightTime );
	Vector ret = azimuth * Vx;
	ret.z = Vy;
	return ret;
}

extern ConVar sv_maxvelocity;

void UTIL_Bound_Velocity( Vector &vec )
{
	for ( int i=0 ; i<3 ; i++ )
	{
		if ( IS_NAN(vec[i]) )
		{
			vec[i] = 0;
		}

		if ( vec[i] > sv_maxvelocity.GetFloat() ) 
		{
			vec[i] = sv_maxvelocity.GetFloat();
		}
		else if ( vec[i] < -sv_maxvelocity.GetFloat() )
		{
			vec[i] = -sv_maxvelocity.GetFloat();
		}
	}
}

extern ConVar sv_gravity;

Vector UTIL_Check_Throw( const Vector &vecSrc, const Vector &vecThrowVelocity, float flGravity, const Vector &vecHullMins, const Vector &vecHullMaxs,
							int iCollisionMask, int iCollisionGroup, CBaseEntity *pIgnoreEnt, bool bDrawArc )
{
	Vector vecVelocity = vecThrowVelocity;
	const int iMaxSteps = 200;
	Vector vecPos = vecSrc;
	float flInterval = 0.016667f;
	float flActualGravity = sv_gravity.GetFloat() * flGravity;
	for ( int i = 0; i < iMaxSteps; i++ )
	{
		// add gravity
		Vector vecAbsVelocity = vecVelocity;

		Vector vecMove;
		vecMove.x = (vecVelocity.x ) * flInterval;
		vecMove.y = (vecVelocity.y ) * flInterval;

		// linear acceleration due to gravity
		float newZVelocity = vecVelocity.z - flActualGravity * flInterval;

		vecMove.z = ((vecVelocity.z + newZVelocity) / 2.0 ) * flInterval;

		vecVelocity.z = newZVelocity;

		UTIL_Bound_Velocity( vecVelocity );

		// trace to new pos
		trace_t tr;
		Vector vecNewPos = vecPos + vecVelocity * flInterval;		
		UTIL_TraceHull( vecPos, vecNewPos, vecHullMins, vecHullMaxs, iCollisionMask, pIgnoreEnt, iCollisionGroup, &tr );

		if ( bDrawArc )
		{
			debugoverlay->AddLineOverlay( vecPos, vecNewPos, 65, 65, 255, true, 3.0f );
		}

		if ( tr.fraction < 1.0f || tr.startsolid )
			break;

		vecPos = tr.endpos;
	}

	return vecPos;
}

bool UTIL_ASW_CommanderLevelAtLeast( CASW_Player *pPlayer, int iLevel, int iPromotion )
{
	int iActualPromotion, iExperience;

	if ( !pPlayer )
	{
#ifdef CLIENT_DLL
		C_ASW_Medal_Store *pMedals = GetMedalStore();
		if ( pMedals )
		{
			iActualPromotion = pMedals->GetPromotion();
			iExperience = pMedals->GetExperience();
		}
		else
#endif
		{
			return false;
		}
	}
	else
	{
		iActualPromotion = pPlayer->GetPromotion();
		iExperience = pPlayer->GetExperience();
	}

	int iActualLevel = LevelFromXP( iExperience, iActualPromotion );
	if ( rd_override_commander_promotion.GetInt() >= 0 )
	{
		iActualPromotion = rd_override_commander_promotion.GetInt();
	}
	if ( rd_override_commander_level.GetInt() >= 0 )
	{
		iActualLevel = rd_override_commander_level.GetInt();
	}

	if ( iPromotion != -1 && iPromotion != iActualPromotion )
	{
		return iPromotion < iActualPromotion;
	}

	return iLevel <= iActualLevel;
}

struct KeyValuesFilePos
{
	explicit KeyValuesFilePos( const char *szName ) :
		szName{ szName },
		nLine{ 1 },
		nColumn{ 1 },
		nOffset{ 0 }
	{
	}

	void Advance( const CUtlBuffer &buf )
	{
		const char *sz = static_cast< const char * >( buf.Base() );
		int nTarget = buf.TellGet();
		while ( nOffset < nTarget )
		{
			char ch = sz[nOffset];
			if ( ch == '\0' )
			{
				return;
			}

			nOffset++;
			if ( ch == '\n' )
			{
				nLine++;
				nColumn = 1;
			}
			else
			{
				// This is wrong for non-ASCII UTF-8 characters, but most errors are between the start of a line and the first non-ASCII character.
				nColumn++;
			}
		}
	}

	const char *szName;
	int nLine;
	int nColumn;
	int nOffset;
};

static void ReportKeyValuesError( const KeyValuesFilePos &pos, const CUtlVector<KeyValuesFilePos> &tokenStack, const char *szMessage )
{
	Warning( "KeyValues error: %s\n\tin %s:%d:%d (byte offset %d)\n", szMessage, pos.szName, pos.nLine, pos.nColumn, pos.nOffset );

	FOR_EACH_VEC( tokenStack, i )
	{
		DevWarning( "\t%s (line %d col %d)\n", tokenStack[i].szName, tokenStack[i].nLine, tokenStack[i].nColumn );
	}
}

static bool ReadToken( CUtlBuffer &buf, KeyValuesFilePos &pos, const CUtlVector<KeyValuesFilePos> &tokenStack, CUtlCharConversion *pConv, char *szBuf, int nBufLength, bool & bWasQuoted, bool & bWasConditional )
{
	bWasQuoted = false;
	bWasConditional = false;

	do
	{
		buf.EatWhiteSpace();
		if ( !buf.IsValid() )
		{
			pos.Advance( buf );
			return false;
		}
	} while ( buf.EatCPPComment() );
	pos.Advance( buf );

	const char *c = static_cast< const char * >( buf.PeekGet( sizeof( char ), 0 ) );
	if ( !c || *c == '\0' )
	{
		return false;
	}

	if ( *c == '"' )
	{
		int len = buf.PeekDelimitedStringLength( pConv, false );
		if ( len >= nBufLength )
		{
			ReportKeyValuesError( pos, tokenStack, "token too long" );
		}

		bWasQuoted = true;
		// This fails on strings like "vgui\white" where the escape code is unknown.
		// What we want is "vgui\\white", but we parse it as "vgui".
		//buf.GetDelimitedString( pConv, szBuf, nBufLength );

		buf.SeekGet( CUtlBuffer::SEEK_CURRENT, pConv->GetDelimiterLength() );
		len -= pConv->GetDelimiterLength() + 1;
		buf.Get( szBuf, len );
		Assert( !V_strncmp( &szBuf[len - pConv->GetDelimiterLength()], pConv->GetDelimiter(), pConv->GetDelimiterLength() ) );
		szBuf[len - pConv->GetDelimiterLength()] = '\0';
		len -= pConv->GetDelimiterLength();

		for ( int i = 0; i < len; i++ )
		{
			if ( szBuf[i] == pConv->GetEscapeChar() )
			{
				int iEscapeLen = 0;
				char ch = pConv->FindConversion( &szBuf[i + 1], &iEscapeLen );
				// here's where the original algorithm goes wrong - the nonexistent
				// \w is replaced with \0w instead of leaving it alone.
				if ( ch )
				{
					V_memcpy( &szBuf[i + 1], &szBuf[i + 1 + iEscapeLen], len - i - iEscapeLen );
					len -= iEscapeLen;
					szBuf[i] = ch;
				}
			}
		}

		return true;
	}

	if ( *c == '{' || *c == '}' )
	{
		Assert( nBufLength >= 2 );
		szBuf[0] = *c;
		szBuf[1] = '\0';
		buf.SeekGet( CUtlBuffer::SEEK_CURRENT, 1 );
		return szBuf;
	}

	bool bReportedError = false;
	bool bConditionalStart = false;
	int nCount = 0;
	while ( ( c = static_cast< const char * >( buf.PeekGet( sizeof( char ), 0 ) ) ) != NULL )
	{
		// end of file
		if ( *c == '\0' )
		{
			break;
		}

		// break if any control character appears in non quoted tokens
		if ( *c == '"' || *c == '{' || *c == '}' )
		{
			break;
		}

		if ( *c == '[' )
		{
			bConditionalStart = true;
		}

		if ( *c == ']' && bConditionalStart )
		{
			bWasConditional = true;
		}

		// break on whitespace
		if ( *c == ' ' || *c == '\t' || *c == '\n' || *c == '\v' || *c == '\f' || *c == '\r' )
		{
			break;
		}

		if ( nCount < ( nBufLength - 1 ) )
		{
			szBuf[nCount++] = *c;	// add char to buffer
		}
		else if ( !bReportedError )
		{
			bReportedError = true;

			ReportKeyValuesError( pos, tokenStack, "token too long" );
		}

		buf.SeekGet( CUtlBuffer::SEEK_CURRENT, 1 );
	}

	szBuf[nCount] = '\0';

	// non-quoted token cannot be both valid and empty
	return nCount != 0;
}

static bool LoadKeyValuesRecursive( CUtlBuffer &buf, KeyValuesFilePos &pos, CUtlVector<KeyValuesFilePos> &tokenStack, CUtlCharConversion *pConv, KeyValues *pKV, char *szBuf, int nBufLength )
{
	if ( tokenStack.Count() > 100 )
	{
		ReportKeyValuesError( pos, tokenStack, "recursion overflow" );

		return false;
	}

	// Locate the last child.  (Almost always, we will not have any children.)
	// We maintain the pointer to the last child here, so we don't have to re-locate
	// it each time we append the next subkey, which causes O(N^2) time
	KeyValues *pLastChild = pKV->GetFirstSubKey();
	for ( KeyValues *pNext = pLastChild; pNext; pNext = pNext->GetNextKey() )
	{
		pLastChild = pNext;
	}

#ifdef DBGFLAG_ASSERT
	int nTokensAtStart = tokenStack.Count();
#endif

	bool bWasQuoted, bWasConditional;
	int nLastKeyLine = 0;

	while ( true )
	{
		if ( !ReadToken( buf, pos, tokenStack, pConv, szBuf, nBufLength, bWasQuoted, bWasConditional ) )
		{
			ReportKeyValuesError( pos, tokenStack, "unexpected end of file" );

			return false;
		}

		if ( szBuf[0] == '\0' )
		{
			ReportKeyValuesError( pos, tokenStack, "key cannot be empty string" );

			return false;
		}

		if ( bWasConditional )
		{
			ReportKeyValuesError( pos, tokenStack, "unexpected conditional" );

			return false;
		}

		if ( !bWasQuoted && !V_strcmp( szBuf, "{" ) )
		{
			ReportKeyValuesError( pos, tokenStack, "unexpected {" );

			return false;
		}

		if ( !bWasQuoted && !V_strcmp( szBuf, "}" ) )
		{
			return true;
		}

		if ( nLastKeyLine == pos.nLine )
		{
			DevWarning( "KeyValues warning: last key started on the same line - %s:%d\n", pos.szName, pos.nLine );
		}

		nLastKeyLine = pos.nLine;

		KeyValues *pChild = new KeyValues{ szBuf };
		pChild->UsesEscapeSequences( true );

		if ( pLastChild )
		{
			Assert( !pLastChild->GetNextKey() );
			pLastChild->SetNextKey( pChild );
		}
		else
		{
			Assert( !pKV->GetFirstSubKey() );
			pKV->AddSubKey( pChild );
		}
		pLastChild = pChild;

		tokenStack[tokenStack.AddToTail( pos )].szName = pChild->GetName();

		if ( !ReadToken( buf, pos, tokenStack, pConv, szBuf, nBufLength, bWasQuoted, bWasConditional ) )
		{
			ReportKeyValuesError( pos, tokenStack, "unexpected end of file" );

			return false;
		}

		if ( bWasConditional )
		{
			ReportKeyValuesError( pos, tokenStack, "unimplemented KeyValues feature: conditionals" );

			return false;
		}

		if ( !bWasQuoted && !V_strcmp( szBuf, "{" ) )
		{
			if ( !LoadKeyValuesRecursive( buf, pos, tokenStack, pConv, pChild, szBuf, nBufLength ) )
			{
				return false;
			}

			Assert( tokenStack.Count() == nTokensAtStart + 1 );
			tokenStack.RemoveMultipleFromTail( 1 );

			continue;
		}

		if ( !bWasQuoted && !V_strcmp( szBuf, "}" ) )
		{
			ReportKeyValuesError( pos, tokenStack, "unexpected }" );

			return false;
		}

		// Valve's KeyValues parser determines whether the value is some kind of number at this point,
		// but we don't care - just throw it in as a string.
		pChild->SetStringValue( szBuf );

		Assert( tokenStack.Count() == nTokensAtStart + 1 );
		tokenStack.RemoveMultipleFromTail( 1 );
	}
}

static bool LoadKeyValuesFromBuffer( KeyValues *pKV, const char *resourceName, CUtlBuffer &buf )
{
	// This truncates values that are more than 1023 bytes long.
	//return pKV->LoadFromBuffer( resourceName, buf );

	// We have translations that are longer than that, so we need to parse the file ourself.
	// Luckily, Source SDK 2013 has a public copy of KeyValues.cpp, and it supports values up to 4095 bytes.
	// Additionally, we don't use #base, #include, or conditionals, so our code can be simpler.
	// And while we're at it, why not give a little bit better error messages than what KeyValues does by default:

	KeyValues *pCurrentKey = pKV;
	KeyValues *pPreviousKey = NULL;
	KeyValuesFilePos pos{ resourceName };
	CUtlCharConversion *pConv = GetCStringCharConversion();
	char szBuf[4096]{};
	CUtlVector<KeyValuesFilePos> tokenStack{};
	bool bWasQuoted, bWasConditional;

	while ( true )
	{
		if ( !ReadToken( buf, pos, tokenStack, pConv, szBuf, sizeof( szBuf ), bWasQuoted, bWasConditional ) )
		{
			if ( pPreviousKey )
			{
				return true;
			}

			ReportKeyValuesError( pos, tokenStack, "empty file" );

			return false;
		}

		if ( szBuf[0] == '\0' )
		{
			ReportKeyValuesError( pos, tokenStack, "key cannot be empty string" );

			return false;
		}

		if ( bWasConditional )
		{
			ReportKeyValuesError( pos, tokenStack, "unexpected conditional" );

			return false;
		}

		if ( !bWasQuoted && ( !V_strcmp( szBuf, "{" ) || !V_strcmp( szBuf, "}" ) ) )
		{
			ReportKeyValuesError( pos, tokenStack, "file starts with control character" );

			return false;
		}

		if ( !V_stricmp( szBuf, "#include" ) || !V_stricmp( szBuf, "#base" ) )
		{
			ReportKeyValuesError( pos, tokenStack, "unimplemented KeyValues feature: #include/#base" );

			return false;
		}

		if ( pCurrentKey )
		{
			pCurrentKey->SetName( szBuf );
		}
		else
		{
			pCurrentKey = new KeyValues{ szBuf };
			pCurrentKey->UsesEscapeSequences( true );

			if ( pPreviousKey )
			{
				pPreviousKey->SetNextKey( pCurrentKey );
			}
		}

		Assert( tokenStack.Count() == 0 );
		tokenStack[tokenStack.AddToTail( pos )].szName = pCurrentKey->GetName();

		if ( !ReadToken( buf, pos, tokenStack, pConv, szBuf, sizeof( szBuf ), bWasQuoted, bWasConditional ) )
		{
			ReportKeyValuesError( pos, tokenStack, "unexpected end of file" );

			return false;
		}

		bool bAccepted = true;
		if ( bWasConditional )
		{
			ReportKeyValuesError( pos, tokenStack, "unimplemented KeyValues feature: conditionals" );

			return false;
		}

		if ( bWasQuoted || V_strcmp( szBuf, "{" ) )
		{
			ReportKeyValuesError( pos, tokenStack, "expected {" );

			return false;
		}

		if ( !LoadKeyValuesRecursive( buf, pos, tokenStack, pConv, pCurrentKey, szBuf, sizeof( szBuf ) ) )
		{
			return false;
		}

		Assert( tokenStack.Count() == 1 );

		if ( bAccepted )
		{
			pPreviousKey = pCurrentKey;
			pCurrentKey = NULL;
		}
		else
		{
			if ( pPreviousKey )
			{
				pPreviousKey->SetNextKey( NULL );
			}

			pCurrentKey->Clear();
		}

		tokenStack.RemoveMultipleFromTail( 1 );
	}
}

bool UTIL_RD_LoadKeyValues( KeyValues *pKV, const char *resourceName, const CUtlBuffer &buf )
{
	const wchar_t *pwszBuf = static_cast< const wchar_t * >( buf.Base() );
	if ( buf.TellPut() >= sizeof( wchar_t ) && *pwszBuf == 0xFEFF )
	{
		// File starts with a byte order mark. Convert it from UTF-16LE to UTF-8.

		// We don't have a function in this version of the Source Engine to tell us how
		// many bytes of UTF8 data there are in a UTF16 string, so just assume the worst.
		//
		// Additionally, we don't know that the buffer we received is null-terminated
		// (it likely isn't), so we need to copy the buffer multiple times to get this right.
		CUtlBuffer buf2{};
		buf2.EnsureCapacity( buf.TellPut() );
		buf2.Put( pwszBuf + 1, buf.TellPut() - sizeof( wchar_t ) );
		buf2.Put( L"", sizeof( wchar_t ) );

		// Now that we have a null-terminated UTF-16LE buffer, convert it to UTF-8.
		CUtlBuffer buf3{ 0, buf.TellPut() * 2, CUtlBuffer::TEXT_BUFFER };
		char *pszBuf = static_cast< char * >( buf3.Base() );
		V_UnicodeToUTF8( static_cast< const wchar_t * >( buf2.Base() ), pszBuf, buf3.Size() );
		buf3.SeekPut( CUtlBuffer::SEEK_HEAD, V_strlen( pszBuf ) );

		return LoadKeyValuesFromBuffer( pKV, resourceName, buf3 );
	}

	const char *pszBuf = static_cast< const char * >( buf.Base() );
	if ( buf.TellPut() >= 3 && !V_strncmp( pszBuf, "\xEF\xBB\xBF", 3 ) )
	{
		// We've got a byte order mark in UTF-8. KeyValues will get confused by this.
		CUtlBuffer buf2{ pszBuf + 3, buf.TellPut() - 3, CUtlBuffer::READ_ONLY | CUtlBuffer::TEXT_BUFFER };
		return LoadKeyValuesFromBuffer( pKV, resourceName, buf2 );
	}

	// Use the buffer as-is.
	CUtlBuffer buf2{ buf.Base(), buf.TellPut(), CUtlBuffer::READ_ONLY | CUtlBuffer::TEXT_BUFFER };
	return LoadKeyValuesFromBuffer( pKV, resourceName, buf2 );
}

bool UTIL_RD_LoadKeyValuesFromFile( KeyValues *pKV, IFileSystem *pFileSystem, const char *szFileName, const char *szPath )
{
	// g_pFullFileSystem->ReadFile doesn't move the PUT pointer to the right place,
	// so we're implementing our own.

	if ( FileHandle_t hFile = pFileSystem->Open( szFileName, "rb", szPath ) )
	{
		int nBytes = pFileSystem->Size( hFile );

		CUtlBuffer buf{ 0, nBytes, CUtlBuffer::TEXT_BUFFER };
		g_pFullFileSystem->Read( buf.Base(), nBytes, hFile );
		buf.SeekPut( CUtlBuffer::SEEK_HEAD, nBytes );

		g_pFullFileSystem->Close( hFile );

		// This mission is included in HoIAF but its overview file is missing a closing brace.
		if ( !V_strcmp( szFileName, "resource/overviews/researchlab2.txt" ) && CRC32_ProcessSingleBuffer( buf.Base(), buf.TellPut() ) == 0x5a28bbce )
		{
			buf.PutChar( '}' );
		}

		return UTIL_RD_LoadKeyValues( pKV, szFileName, buf );
	}

	return false;
}

void UTIL_RD_LoadAllKeyValues( const char *fileName, const char *pPathID, const char *pKVName, UTIL_RD_LoadAllKeyValuesCallback callback, void *pUserData )
{
	KeyValues::AutoDelete pKV( pKVName );
	pKV->UsesEscapeSequences( true );

	{
		CUtlVector<CUtlString> paths;
		GetSearchPath( paths, pPathID );

		FOR_EACH_VEC( paths, i )
		{
			CUtlString path = CUtlString::PathJoin( paths[i], fileName );

			pKV->Clear();
			if ( UTIL_RD_LoadKeyValuesFromFile( pKV, g_pFullFileSystem, path, NULL ) )
			{
				callback( paths[i], pKV, pUserData );
			}
		}
	}

	{
		CUtlVector<CUtlString> vpks;
		g_pFullFileSystem->GetVPKFileNames( vpks );

		FOR_EACH_VEC( vpks, i )
		{
			CPackedStore vpk( vpks[i], g_pFullFileSystem );

			if ( CPackedStoreFileHandle hFile = vpk.OpenFile( fileName ) )
			{
				CUtlBuffer buf( 0, hFile.m_nFileSize + 2, CUtlBuffer::TEXT_BUFFER );
				hFile.Read( buf.Base(), buf.Size() );
				buf.SeekPut( CUtlBuffer::SEEK_HEAD, hFile.m_nFileSize );

				CUtlString fullFileName = CUtlString::PathJoin( vpks[i], fileName );
				pKV->Clear();
				if ( UTIL_RD_LoadKeyValues( pKV, fullFileName, buf ) )
				{
					callback( vpks[i], pKV, pUserData );
				}
			}
		}
	}
}

struct AddLocalizeFileCallbackData_t
{
	bool bAny : 1;
	bool bIsCaptions : 1;
};

static CUtlStringMap<CRC32_t> s_CaptionHashLookup{ true };
static CUtlMap<CRC32_t, UtlSymId_t> s_CaptionHashRevLookup{ DefLessFunc( CRC32_t ) };

static void AddLocalizeFileCallback( const char *pszPath, KeyValues *pKV, void *pUserData )
{
	AddLocalizeFileCallbackData_t *pData = static_cast< AddLocalizeFileCallbackData_t * >( pUserData );

	KeyValues *pTokens = pKV->FindKey( "Tokens" );
	if ( !pTokens )
	{
		return;
	}

	char szLowerKey[256]{};

	FOR_EACH_VALUE( pTokens, pValue )
	{
		const char *pszKey = pValue->GetName();
		if ( StringHasPrefix( pszKey, "[english]" ) )
		{
			continue;
		}

		if ( *pszKey == '#' )
		{
			pszKey++;
		}

		g_pVGuiLocalize->AddString( pszKey, const_cast<wchar_t *>( pValue->GetWString() ), NULL );

		pData->bAny = true;

		if ( pData->bIsCaptions )
		{
			int len = V_strlen( pszKey );
			Assert( len < sizeof( szLowerKey ) );
			V_strncpy( szLowerKey, pszKey, sizeof( szLowerKey ) );
			V_strlower( szLowerKey );

			CRC32_t hash;
			CRC32_Init( &hash );
			CRC32_ProcessBuffer( &hash, szLowerKey, len );
			CRC32_Final( &hash );

			UtlSymId_t sym = s_CaptionHashLookup.AddString( pszKey );
			s_CaptionHashLookup[sym] = hash;
			s_CaptionHashRevLookup.InsertOrReplace( hash, sym );
		}
	}
}

#ifdef GAME_DLL
ConVar rd_dedicated_server_language( "rd_dedicated_server_language", "english", FCVAR_NONE, "translation language to load on dedicated server" );
#endif

bool UTIL_RD_AddLocalizeFile( const char *fileName, const char *pPathID, bool bIncludeFallbackSearchPaths, bool bIsCaptions )
{
	char szPath[MAX_PATH];
	if ( const char *pszLanguageToken = V_strstr( fileName, "%language%" ) )
	{
		// localize.dll by default reads Steam's language setting from the Windows registry.
		// This is not ideal as it means the user's game language setting is ignored.
		// Replace the language name here so we have control over what it is.

		const char *pszLanguageReplacement = "%language%";
		if ( CommandLine()->FindParm( "-language" ) )
		{
			pszLanguageReplacement = CommandLine()->ParmValue( "-language", "english" );
		}
		else if ( SteamApps() )
		{
			pszLanguageReplacement = SteamApps()->GetCurrentGameLanguage();
		}
#ifdef GAME_DLL
		else if ( engine->IsDedicatedServer() )
		{
			pszLanguageReplacement = rd_dedicated_server_language.GetString();
		}
#endif
		else if ( registry )
		{
			// If we failed to load the Steam API and we're not a dedicated server, attempt to grab the language for Steam itself from the Windows registry, or fall back to English.
			pszLanguageReplacement = registry->ReadString( "HKEY_CURRENT_USER\\SOFTWARE\\Valve\\Steam", "Language", "english" );
		}

		strncpy_s( szPath, fileName, pszLanguageToken - fileName );
		strcat_s( szPath, pszLanguageReplacement );
		strcat_s( szPath, pszLanguageToken + 10 );
	}
	else
	{
		strcpy_s( szPath, fileName );
	}

#ifdef CLIENT_DLL
	if ( rd_load_all_localization_files.GetBool() )
#endif
	{
		AddLocalizeFileCallbackData_t data{};
		data.bAny = false;
		data.bIsCaptions = bIsCaptions;

		UTIL_RD_LoadAllKeyValues( szPath, pPathID, "lang", &AddLocalizeFileCallback, &data );

		return data.bAny;
	}

	return g_pVGuiLocalize->AddFile( szPath, pPathID, bIncludeFallbackSearchPaths );
}

#ifdef CLIENT_DLL
CON_COMMAND_F( rd_loc_reload, "reload localization files", FCVAR_HIDDEN )
#else
CON_COMMAND_F( rd_loc_reload_server, "reload localization files (dedicated server)", FCVAR_HIDDEN )
#endif
{
#ifndef CLIENT_DLL
	if ( engine->IsDedicatedServer() && !UTIL_IsCommandIssuedByServerAdmin() )
	{
		return;
	}
#endif

	UTIL_RD_ReloadLocalizeFiles();
}

void UTIL_RD_ReloadLocalizeFiles()
{
	// load english first just in case an addon is not localized
	UTIL_RD_AddLocalizeFile( "resource/gameui_english.txt", "GAME", true, false );
	UTIL_RD_AddLocalizeFile( "resource/valve_english.txt", "GAME", true, false );
	UTIL_RD_AddLocalizeFile( "resource/platform_english.txt", "GAME", true, false );
	UTIL_RD_AddLocalizeFile( "resource/vgui_english.txt", "GAME", true, false );
	UTIL_RD_AddLocalizeFile( "resource/basemodui_english.txt", "GAME", true, false );
	UTIL_RD_AddLocalizeFile( "resource/closecaption_english.txt", "GAME", true, true );
	UTIL_RD_AddLocalizeFile( "resource/reactivedrop_english.txt", "GAME", true, false );

	// load actual localization files
	UTIL_RD_AddLocalizeFile( "resource/gameui_%language%.txt", "GAME", true, false );
	UTIL_RD_AddLocalizeFile( "resource/valve_%language%.txt", "GAME", true, false );
	UTIL_RD_AddLocalizeFile( "resource/platform_%language%.txt", "GAME", true, false );
	UTIL_RD_AddLocalizeFile( "resource/vgui_%language%.txt", "GAME", true, false );
	UTIL_RD_AddLocalizeFile( "resource/basemodui_%language%.txt", "GAME", true, false );
	UTIL_RD_AddLocalizeFile( "resource/closecaption_%language%.txt", "GAME", true, true );
	UTIL_RD_AddLocalizeFile( "resource/reactivedrop_%language%.txt", "GAME", true, false );

	DevMsg( 2, "Reloaded localization files.\n" );
}

CRC32_t UTIL_RD_CaptionToHash( const char *szToken )
{
	Assert( s_CaptionHashLookup.GetNumStrings() );
	UtlSymId_t sym = s_CaptionHashLookup.Find( szToken );
	if ( sym == UTL_INVAL_SYMBOL )
	{
		return 0;
	}

	return s_CaptionHashLookup[sym];
}

const char *UTIL_RD_HashToCaption( CRC32_t hash )
{
	Assert( s_CaptionHashRevLookup.Count() );
	unsigned short index = s_CaptionHashRevLookup.Find( hash );
	Assert( s_CaptionHashRevLookup.IsValidIndex( index ) );
	if ( !s_CaptionHashRevLookup.IsValidIndex( index ) )
	{
		return NULL;
	}

	UtlSymId_t sym = s_CaptionHashRevLookup[index];
	Assert( sym != UTL_INVAL_SYMBOL );
	return s_CaptionHashLookup.String( sym );
}

const char *UTIL_RD_EResultToString( EResult eResult )
{
	switch ( eResult )
	{
	default:
		return "unknown EResult value";
	case k_EResultNone:
		return "k_EResultNone";
	case k_EResultOK:
		return "k_EResultOK";
	case k_EResultFail:
		return "k_EResultFail";
	case k_EResultNoConnection:
		return "k_EResultNoConnection";
	case k_EResultInvalidPassword:
		return "k_EResultInvalidPassword";
	case k_EResultLoggedInElsewhere:
		return "k_EResultLoggedInElsewhere";
	case k_EResultInvalidProtocolVer:
		return "k_EResultInvalidProtocolVer";
	case k_EResultInvalidParam:
		return "k_EResultInvalidParam";
	case k_EResultFileNotFound:
		return "k_EResultFileNotFound";
	case k_EResultBusy:
		return "k_EResultBusy";
	case k_EResultInvalidState:
		return "k_EResultInvalidState";
	case k_EResultInvalidName:
		return "k_EResultInvalidName";
	case k_EResultInvalidEmail:
		return "k_EResultInvalidEmail";
	case k_EResultDuplicateName:
		return "k_EResultDuplicateName";
	case k_EResultAccessDenied:
		return "k_EResultAccessDenied";
	case k_EResultTimeout:
		return "k_EResultTimeout";
	case k_EResultBanned:
		return "k_EResultBanned";
	case k_EResultAccountNotFound:
		return "k_EResultAccountNotFound";
	case k_EResultInvalidSteamID:
		return "k_EResultInvalidSteamID";
	case k_EResultServiceUnavailable:
		return "k_EResultServiceUnavailable";
	case k_EResultNotLoggedOn:
		return "k_EResultNotLoggedOn";
	case k_EResultPending:
		return "k_EResultPending";
	case k_EResultEncryptionFailure:
		return "k_EResultEncryptionFailure";
	case k_EResultInsufficientPrivilege:
		return "k_EResultInsufficientPrivilege";
	case k_EResultLimitExceeded:
		return "k_EResultLimitExceeded";
	case k_EResultRevoked:
		return "k_EResultRevoked";
	case k_EResultExpired:
		return "k_EResultExpired";
	case k_EResultAlreadyRedeemed:
		return "k_EResultAlreadyRedeemed";
	case k_EResultDuplicateRequest:
		return "k_EResultDuplicateRequest";
	case k_EResultAlreadyOwned:
		return "k_EResultAlreadyOwned";
	case k_EResultIPNotFound:
		return "k_EResultIPNotFound";
	case k_EResultPersistFailed:
		return "k_EResultPersistFailed";
	case k_EResultLockingFailed:
		return "k_EResultLockingFailed";
	case k_EResultLogonSessionReplaced:
		return "k_EResultLogonSessionReplaced";
	case k_EResultConnectFailed:
		return "k_EResultConnectFailed";
	case k_EResultHandshakeFailed:
		return "k_EResultHandshakeFailed";
	case k_EResultIOFailure:
		return "k_EResultIOFailure";
	case k_EResultRemoteDisconnect:
		return "k_EResultRemoteDisconnect";
	case k_EResultShoppingCartNotFound:
		return "k_EResultShoppingCartNotFound";
	case k_EResultBlocked:
		return "k_EResultBlocked";
	case k_EResultIgnored:
		return "k_EResultIgnored";
	case k_EResultNoMatch:
		return "k_EResultNoMatch";
	case k_EResultAccountDisabled:
		return "k_EResultAccountDisabled";
	case k_EResultServiceReadOnly:
		return "k_EResultServiceReadOnly";
	case k_EResultAccountNotFeatured:
		return "k_EResultAccountNotFeatured";
	case k_EResultAdministratorOK:
		return "k_EResultAdministratorOK";
	case k_EResultContentVersion:
		return "k_EResultContentVersion";
	case k_EResultTryAnotherCM:
		return "k_EResultTryAnotherCM";
	case k_EResultPasswordRequiredToKickSession:
		return "k_EResultPasswordRequiredToKickSession";
	case k_EResultAlreadyLoggedInElsewhere:
		return "k_EResultAlreadyLoggedInElsewhere";
	case k_EResultSuspended:
		return "k_EResultSuspended";
	case k_EResultCancelled:
		return "k_EResultCancelled";
	case k_EResultDataCorruption:
		return "k_EResultDataCorruption";
	case k_EResultDiskFull:
		return "k_EResultDiskFull";
	case k_EResultRemoteCallFailed:
		return "k_EResultRemoteCallFailed";
	case k_EResultPasswordUnset:
		return "k_EResultPasswordUnset";
	case k_EResultExternalAccountUnlinked:
		return "k_EResultExternalAccountUnlinked";
	case k_EResultPSNTicketInvalid:
		return "k_EResultPSNTicketInvalid";
	case k_EResultExternalAccountAlreadyLinked:
		return "k_EResultExternalAccountAlreadyLinked";
	case k_EResultRemoteFileConflict:
		return "k_EResultRemoteFileConflict";
	case k_EResultIllegalPassword:
		return "k_EResultIllegalPassword";
	case k_EResultSameAsPreviousValue:
		return "k_EResultSameAsPreviousValue";
	case k_EResultAccountLogonDenied:
		return "k_EResultAccountLogonDenied";
	case k_EResultCannotUseOldPassword:
		return "k_EResultCannotUseOldPassword";
	case k_EResultInvalidLoginAuthCode:
		return "k_EResultInvalidLoginAuthCode";
	case k_EResultAccountLogonDeniedNoMail:
		return "k_EResultAccountLogonDeniedNoMail";
	case k_EResultHardwareNotCapableOfIPT:
		return "k_EResultHardwareNotCapableOfIPT";
	case k_EResultIPTInitError:
		return "k_EResultIPTInitError";
	case k_EResultParentalControlRestricted:
		return "k_EResultParentalControlRestricted";
	case k_EResultFacebookQueryError:
		return "k_EResultFacebookQueryError";
	case k_EResultExpiredLoginAuthCode:
		return "k_EResultExpiredLoginAuthCode";
	case k_EResultIPLoginRestrictionFailed:
		return "k_EResultIPLoginRestrictionFailed";
	case k_EResultAccountLockedDown:
		return "k_EResultAccountLockedDown";
	case k_EResultAccountLogonDeniedVerifiedEmailRequired:
		return "k_EResultAccountLogonDeniedVerifiedEmailRequired";
	case k_EResultNoMatchingURL:
		return "k_EResultNoMatchingURL";
	case k_EResultBadResponse:
		return "k_EResultBadResponse";
	case k_EResultRequirePasswordReEntry:
		return "k_EResultRequirePasswordReEntry";
	case k_EResultValueOutOfRange:
		return "k_EResultValueOutOfRange";
	case k_EResultUnexpectedError:
		return "k_EResultUnexpectedError";
	case k_EResultDisabled:
		return "k_EResultDisabled";
	case k_EResultInvalidCEGSubmission:
		return "k_EResultInvalidCEGSubmission";
	case k_EResultRestrictedDevice:
		return "k_EResultRestrictedDevice";
	case k_EResultRegionLocked:
		return "k_EResultRegionLocked";
	case k_EResultRateLimitExceeded:
		return "k_EResultRateLimitExceeded";
	case k_EResultAccountLoginDeniedNeedTwoFactor:
		return "k_EResultAccountLoginDeniedNeedTwoFactor";
	case k_EResultItemDeleted:
		return "k_EResultItemDeleted";
	case k_EResultAccountLoginDeniedThrottle:
		return "k_EResultAccountLoginDeniedThrottle";
	case k_EResultTwoFactorCodeMismatch:
		return "k_EResultTwoFactorCodeMismatch";
	case k_EResultTwoFactorActivationCodeMismatch:
		return "k_EResultTwoFactorActivationCodeMismatch";
	case k_EResultAccountAssociatedToMultiplePartners:
		return "k_EResultAccountAssociatedToMultiplePartners";
	case k_EResultNotModified:
		return "k_EResultNotModified";
	case k_EResultNoMobileDevice:
		return "k_EResultNoMobileDevice";
	case k_EResultTimeNotSynced:
		return "k_EResultTimeNotSynced";
	case k_EResultSmsCodeFailed:
		return "k_EResultSmsCodeFailed";
	case k_EResultAccountLimitExceeded:
		return "k_EResultAccountLimitExceeded";
	case k_EResultAccountActivityLimitExceeded:
		return "k_EResultAccountActivityLimitExceeded";
	case k_EResultPhoneActivityLimitExceeded:
		return "k_EResultPhoneActivityLimitExceeded";
	case k_EResultRefundToWallet:
		return "k_EResultRefundToWallet";
	case k_EResultEmailSendFailure:
		return "k_EResultEmailSendFailure";
	case k_EResultNotSettled:
		return "k_EResultNotSettled";
	case k_EResultNeedCaptcha:
		return "k_EResultNeedCaptcha";
	case k_EResultGSLTDenied:
		return "k_EResultGSLTDenied";
	case k_EResultGSOwnerDenied:
		return "k_EResultGSOwnerDenied";
	case k_EResultInvalidItemType:
		return "k_EResultInvalidItemType";
	case k_EResultIPBanned:
		return "k_EResultIPBanned";
	case k_EResultGSLTExpired:
		return "k_EResultGSLTExpired";
	case k_EResultInsufficientFunds:
		return "k_EResultInsufficientFunds";
	case k_EResultTooManyPending:
		return "k_EResultTooManyPending";
	case k_EResultNoSiteLicensesFound:
		return "k_EResultNoSiteLicensesFound";
	case k_EResultWGNetworkSendExceeded:
		return "k_EResultWGNetworkSendExceeded";
	case k_EResultAccountNotFriends:
		return "k_EResultAccountNotFriends";
	case k_EResultLimitedUserAccount:
		return "k_EResultLimitedUserAccount";
	case k_EResultCantRemoveItem:
		return "k_EResultCantRemoveItem";
	case k_EResultAccountDeleted:
		return "k_EResultAccountDeleted";
	case k_EResultExistingUserCancelledLicense:
		return "k_EResultExistingUserCancelledLicense";
	case k_EResultCommunityCooldown:
		return "k_EResultCommunityCooldown";
	case k_EResultNoLauncherSpecified:
		return "k_EResultNoLauncherSpecified";
	case k_EResultMustAgreeToSSA:
		return "k_EResultMustAgreeToSSA";
	case k_EResultLauncherMigrated:
		return "k_EResultLauncherMigrated";
	case k_EResultSteamRealmMismatch:
		return "k_EResultSteamRealmMismatch";
	case k_EResultInvalidSignature:
		return "k_EResultInvalidSignature";
	case k_EResultParseFailure:
		return "k_EResultParseFailure";
	case k_EResultNoVerifiedPhone:
		return "k_EResultNoVerifiedPhone";
	case k_EResultInsufficientBattery:
		return "k_EResultInsufficientBattery";
	case k_EResultChargerRequired:
		return "k_EResultChargerRequired";
	case k_EResultCachedCredentialInvalid:
		return "k_EResultCachedCredentialInvalid";
	case K_EResultPhoneNumberIsVOIP:
		return "k_EResultPhoneNumberIsVOIP"; // using lower-case k for consistency
	}
}

const wchar_t *UTIL_RD_CommaNumber( int64_t num )
{
	static wchar_t s_wszBuf[16][28];
	static int s_iSlot = 0;

	// Special case: can't negate this number.
	if ( num == INT64_MIN )
	{
		return L"-9,223,372,036,854,775,808";
	}

	// Special case 2:
	if ( num == 0 )
	{
		return L"0";
	}

	wchar_t *pBuf = &s_wszBuf[s_iSlot][27];
	*pBuf-- = L'\0';

	s_iSlot = ( s_iSlot + 1 ) % NELEMS( s_wszBuf );

	bool bNegative = num < 0;
	if ( bNegative )
	{
		num = -num;
	}

	for ( int64_t next = num / 1000ll; next; num = next, next = next / 1000ll )
	{
		*pBuf-- = L'0' + ( num % 10 );
		*pBuf-- = L'0' + ( ( num / 10 ) % 10 );
		*pBuf-- = L'0' + ( ( num / 100 ) % 10 );
		*pBuf-- = L',';
	}

	while ( num )
	{
		*pBuf-- = L'0' + ( num % 10 );
		num /= 10;
	}

	if ( bNegative )
	{
		*pBuf = L'-';
	}
	else
	{
		pBuf++;
	}

	return pBuf;
}

// get the index of the nth bit that is set to 1
int UTIL_RD_IndexToBit( unsigned bits, int n )
{
	int i = -1;
	while ( i < n )
	{
		Assert( bits );
		if ( !bits )
		{
			// return a bad value that probably won't cause crashes
			return 0;
		}

		if ( bits & 1 )
		{
			i++;
		}

		bits >>= 1;
	}

	return i;
}

// the reverse of UTIL_RD_IndexToBit
int UTIL_RD_BitToIndex( unsigned bits, int n )
{
	return UTIL_CountNumBitsSet( bits & ( ( 1 << n ) - 1 ) );
}

void CmdMsg( _Printf_format_string_ const char *pszFormat, ... )
{
	char szString[1024];

	va_list args;
	va_start( args, pszFormat );
	Q_vsnprintf( szString, sizeof( szString ), pszFormat, args );
	va_end( args );

#ifdef GAME_DLL
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer )
	{
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "%s", szString);
	}
	else
#endif
	{
		Msg( "%s", szString );
	}
}

#include "cbase.h"
#include "asw_door.h"
#include "asw_door_padding.h"
#include "doors.h"
#include "asw_trace_filter_door_crush.h"
#include "eventqueue.h"
#include "func_asw_fade.h"
#include "prop_asw_fade.h"
#include "asw_marine.h"
#include "asw_marine_speech.h"
#include "asw_marine_resource.h"
#include "asw_player.h"
#include "asw_weapon_sniper_rifle.h"
#include "asw_fail_advice.h"
#include "asw_gamerules.h"
#include "asw_generic_emitter_entity.h"
#include "vcollide_parse.h"
#include "cvisibilitymonitor.h"
#include "soundent.h"
#include "asw_util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CASW_Door )
	DEFINE_KEYFIELD( m_eSpawnPosition, FIELD_INTEGER, "spawnpos" ),
	DEFINE_KEYFIELD( m_flDistance, FIELD_FLOAT, "distance" ),
	DEFINE_KEYFIELD( m_angSlideAngle, FIELD_VECTOR, "slideangle" ),
	DEFINE_KEYFIELD( m_flTotalSealTime, FIELD_FLOAT, "totalsealtime" ),
	DEFINE_KEYFIELD( m_flCurrentSealTime, FIELD_FLOAT, "currentsealtime" ),
	DEFINE_KEYFIELD( m_iDoorType, FIELD_INTEGER, "doortype" ),
	DEFINE_KEYFIELD( m_iCustomMaxHealth, FIELD_INTEGER, "CustomMaxHealth" ),
	DEFINE_KEYFIELD( m_DentAmount, FIELD_INTEGER, "dentamount" ),
	DEFINE_KEYFIELD( m_flCustomDentPercentage, FIELD_FLOAT, "CustomDentPercentage" ),
	DEFINE_KEYFIELD( m_bShowsOnScanner, FIELD_BOOLEAN, "showsonscanner" ),
	DEFINE_KEYFIELD( m_bAutoOpen, FIELD_BOOLEAN, "autoopen" ),
	DEFINE_KEYFIELD( m_bCanCloseToWeld, FIELD_BOOLEAN, "CanCloseToWeld" ),
	DEFINE_KEYFIELD( m_bCanPlayerWeld, FIELD_BOOLEAN, "CanPlayerWeld" ),
	DEFINE_KEYFIELD( m_bDoCutShout, FIELD_BOOLEAN, "DoCutShout" ),
	DEFINE_KEYFIELD( m_bDoBreachedShout, FIELD_BOOLEAN, "DoBreachedShout" ),
	DEFINE_KEYFIELD( m_bDoAutoShootChatter, FIELD_BOOLEAN, "DoAutoShootChatter" ),

	DEFINE_FIELD( m_bBashable, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bShootable, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecGoal, FIELD_VECTOR ),
	DEFINE_FIELD( m_hDoorBlocker, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hDoorPadding, FIELD_EHANDLE ),
	DEFINE_FIELD( m_fClosingToWeldTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_bHasBeenWelded, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bFlipped, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bSetSide, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vecOpenPosition, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecClosedPosition, FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vecBoundsMin, FIELD_VECTOR ),
	DEFINE_FIELD( m_vecBoundsMax, FIELD_VECTOR ),

	DEFINE_FIELD( m_fLastMarineShootTime, FIELD_TIME ),
	DEFINE_FIELD( m_fMarineShootCounter, FIELD_FLOAT ),
	DEFINE_FIELD( m_bDoneDoorShout, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_fLastMomentFlipDamage, FIELD_FLOAT ),

	DEFINE_FIELD( m_fSkillMarineHelping, FIELD_TIME ),
	DEFINE_FIELD( m_bSkillMarineHelping, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bDoorFallen, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_bRecommendedSeal, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bWasWeldedByMarine, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_iszDoorHitSound, FIELD_SOUNDNAME ),
	DEFINE_FIELD( m_iszDoorDentSound, FIELD_SOUNDNAME ),

	DEFINE_THINKFUNC( RunAnimation ),

	DEFINE_INPUTFUNC( FIELD_VOID, "NPCNear", InputNPCNear ),

	DEFINE_INPUTFUNC( FIELD_VOID, "EnableAutoOpen", InputEnableAutoOpen ),
	DEFINE_INPUTFUNC( FIELD_VOID, "DisableAutoOpen", InputDisableAutoOpen ),
	DEFINE_INPUTFUNC( FIELD_VOID, "RecommendWeld", InputRecommendWeld ),

	DEFINE_OUTPUT( m_OnFullySealed, "OnFullySealed" ),
	DEFINE_OUTPUT( m_OnFullyCut, "OnFullyCut" ),
	DEFINE_OUTPUT( m_OnDestroyed, "OnDestroyed" ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CASW_Door, DT_ASW_Door )
	SendPropFloat		( SENDINFO( m_flTotalSealTime ) ),
	SendPropFloat		( SENDINFO( m_flCurrentSealTime ) ),
	SendPropInt			( SENDINFO( m_iDoorStrength ) ),
	SendPropInt			( SENDINFO( m_iDoorType ) ),
	SendPropInt			( SENDINFO( m_iHealth ) ),
	SendPropInt			( SENDINFO( m_lifeState ) ),
	SendPropBool		( SENDINFO( m_bAutoOpen ) ),
	SendPropBool		( SENDINFO( m_bBashable ) ),
	SendPropBool		( SENDINFO( m_bShootable ) ),
	SendPropBool		( SENDINFO( m_bCanCloseToWeld ) ),
	SendPropBool		( SENDINFO( m_bCanPlayerWeld ) ),
	SendPropBool		( SENDINFO( m_bRecommendedSeal ) ),
	SendPropBool		( SENDINFO( m_bWasWeldedByMarine ) ),
	SendPropFloat		( SENDINFO( m_fLastMomentFlipDamage ) ),
	SendPropVector		( SENDINFO( m_vecClosedPosition ), -1, SPROP_COORD ),
	SendPropBool		( SENDINFO( m_bSkillMarineHelping ) ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( asw_door, CASW_Door );

ConVar asw_door_drone_damage_scale( "asw_door_drone_damage_scale", "2.0f", FCVAR_CHEAT, "Damage scale for drones hitting doors" );
ConVar asw_door_seal_damage_reduction( "asw_door_seal_damage_reduction", "0.6f", FCVAR_CHEAT, "Alien damage scale when door is fully sealed" );
ConVar asw_door_physics( "asw_door_physics", "0", FCVAR_CHEAT, "If set, doors will turn into vphysics objects upon death." );
ConVar rd_door_melee_damage( "rd_door_melee_damage", "0", FCVAR_CHEAT, "Allow doors to take melee damage from marines." );
ConVar asw_door_damage_base( "asw_door_damage_base", "1000", FCVAR_CHEAT, "Damage being hit by a door does on Normal." );
ConVar asw_door_damage_step( "asw_door_damage_step", "0", FCVAR_CHEAT, "Increase/decrease in damage from being hit by a door for every mission difficulty above/below 5." );
ConVar asw_door_normal_health_base( "asw_door_normal_health_base", "1800", FCVAR_CHEAT | FCVAR_REPLICATED, "Base health for a non-reinforced door on Normal." );
ConVar asw_door_normal_health_step( "asw_door_normal_health_step", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "Increase/decrease in health for a non-reinforced door on for every mission difficulty above/below 5." );
ConVar asw_door_reinforced_health_base( "asw_door_reinforced_health_base", "2400", FCVAR_CHEAT, "Base health for a reinforced door on Normal." );
ConVar asw_door_reinforced_health_step( "asw_door_reinforced_health_step", "0", FCVAR_CHEAT, "Increase/decrease in health for a reinforced door on for every mission difficulty above/below 5." );
extern ConVar asw_debug_marine_chatter;
extern ConVar asw_difficulty_alien_damage_step;

static const char *s_pDoorAnimThink = "DoorAnimThink";

namespace
{
	void UTIL_ComputeAABBForBounds( const Vector &mins1, const Vector &maxs1, const Vector &mins2, const Vector &maxs2, Vector *destMins, Vector *destMaxs )
	{
		// Find the minimum extents
		( *destMins )[0] = MIN( mins1[0], mins2[0] );
		( *destMins )[1] = MIN( mins1[1], mins2[1] );
		( *destMins )[2] = MIN( mins1[2], mins2[2] );

		// Find the maximum extents
		( *destMaxs )[0] = MAX( maxs1[0], maxs2[0] );
		( *destMaxs )[1] = MAX( maxs1[1], maxs2[1] );
		( *destMaxs )[2] = MAX( maxs1[2], maxs2[2] );
	}
}

CASW_Door::CASW_Door()
{
	// compat for maps built before this key was added to hammer
	m_bCanPlayerWeld = true;
}

CASW_Door::~CASW_Door( void )
{
	// Remove our door blocker entity
	if ( m_hDoorBlocker != NULL )
	{
		UTIL_Remove( m_hDoorBlocker );
	}
	if ( m_hDoorPadding != NULL )
	{
		UTIL_Remove( m_hDoorPadding );
	}
}

void CASW_Door::Precache()
{
	PrecacheScriptSound( "ASW_Welder.WeldDeny" );

	BaseClass::Precache();

	int iModelIndex = modelinfo->GetModelIndex( STRING( GetModelName() ) );
	const model_t *pModel = modelinfo->GetModel( iModelIndex );

	KeyValues::AutoDelete pKV( "" );
	CUtlBuffer buf( 1024, 0, CUtlBuffer::TEXT_BUFFER );
	if ( modelinfo->GetModelKeyValue( pModel, buf ) )
	{
		pKV->LoadFromBuffer( modelinfo->GetModelName( pModel ), buf );
	}

	m_iszDoorHitSound = AllocPooledString( pKV->GetString( "asw_door/hit_sound", "ASW_Door.MeleeHit" ) );
	m_iszDoorDentSound = AllocPooledString( pKV->GetString( "asw_door/dent_sound", "ASW_Door.Dented" ) );

	PrecacheScriptSound( STRING( m_iszDoorHitSound ) );
	PrecacheScriptSound( STRING( m_iszDoorDentSound ) );
}

void CASW_Door::Spawn()
{
	if ( m_flDistance == 0 )
	{
		m_flDistance = 90;
	}
	m_fLastMomentFlipDamage = 0;
	m_iFallingStage = 0;
	m_flDistance = fabs( m_flDistance );

	// Calculate our closed and open positions
	m_vecClosedPosition = GetAbsOrigin();
	QAngle door_angle = GetLocalAngles();
	door_angle += m_angSlideAngle;
	Vector v( m_flDistance, 0, 0 );
	matrix3x4_t door_angle_matrix;
	Vector offset;
	AngleMatrix( door_angle, door_angle_matrix );
	VectorRotate( v, door_angle_matrix, offset );
	m_vecOpenPosition = GetAbsOrigin() + offset;
	m_fClosingToWeldTime = 0;
	m_nHardwareType = -1;	// Suppress base class warnings
	m_nPhysicsMaterial = physprops->GetSurfaceIndex( "metal" );

	// Call this last! It relies on stuff we calculated above.
	BaseClass::Spawn();

	// Figure out our volumes of movement as this door opens
	CalculateDoorVolume( m_vecOpenPosition, m_vecClosedPosition, &m_vecBoundsMin, &m_vecBoundsMax );

	if ( m_flTotalSealTime <= 0 )
		m_flTotalSealTime = 10.0f;

	// set door strength according to breakable or not:
	switch ( m_iDoorType )
	{
	case ASWDT_NORMAL:
		{
			m_bBashable = true;
			m_bShootable = true;
			m_iDoorStrength = asw_door_normal_health_base.GetInt();

			break;
		}
	case ASWDT_REINFORCED:
		{
			m_bBashable = true;
			m_bShootable = true;
			m_iDoorStrength = asw_door_reinforced_health_base.GetInt();

			break;
		}
	case ASWDT_INDESTRUCTABLE:
		{
			m_bBashable = false;
			m_bShootable = false;
			m_iDoorStrength = 0;

			break;
		}
	case ASWDT_CUSTOM:
		{
			m_bBashable = true;
			m_bShootable = true;
			m_iDoorStrength = m_iCustomMaxHealth <= 0 ? asw_door_normal_health_base.GetInt() : m_iCustomMaxHealth;

			break;
		}
	}

	m_bSetSide = false;
	m_bFlipped = false;
	m_bDoneChatter = false;
	m_fChatterCounter = 0;

	if ( m_DentAmount == ASWDD_PARTIAL_PREFLIP )
	{
		m_DentAmount = ASWDD_PARTIAL;
		FlipDoor();
	}
	else if ( m_DentAmount == ASWDD_COMPLETE_PREFLIP )
	{
		m_DentAmount = ASWDD_COMPLETE;
		FlipDoor();
	}

	if ( m_iDoorStrength > 0 )
	{
		m_takedamage = DAMAGE_YES;
		m_iHealth = m_iDoorStrength;

		if ( m_flCustomDentPercentage > 0.001f && m_flCustomDentPercentage < 1.0f )
		{
			m_iHealth = m_flCustomDentPercentage * m_iDoorStrength;

			if ( m_iHealth == 0 )
				m_iHealth = 1;
		}
		else
		{
			if ( m_DentAmount == ASWDD_PARTIAL )
			{
				m_iHealth = ASW_DOOR_PARTIAL_DENT_HEALTH * m_iDoorStrength;
			}
			else if ( m_DentAmount == ASWDD_COMPLETE )
			{
				m_iHealth = ASW_DOOR_COMPLETE_DENT_HEALTH * m_iDoorStrength;
			}
		}

		SetDentSequence();
		SetMaxHealth( m_iHealth );
	}
	else
	{
		m_takedamage = DAMAGE_YES; // has to receive damage events so marines can be informed they're shooting a junk item
		m_iHealth = 1;
	}

	// if this door isn't sealed, we don't do cut shouting
	if ( m_flCurrentSealTime <= 0 )
	{
		m_bDoCutShout = false;
	}

	m_fLastFullyWeldedSound = 0;

	// create our padding:
	m_hDoorPadding = CASW_Door_Padding::CreateDoorPadding( this );

	if ( VPhysicsGetObject() )
	{
		VPhysicsGetObject()->SetMaterialIndex( physprops->GetSurfaceIndex( "metal" ) );
	}

	SetContextThink( &CASW_Door::RunAnimation, gpGlobals->curtime + 0.1f, s_pDoorAnimThink );

	if ( m_flCurrentSealTime > 0.0f )
	{
		VisibilityMonitor_AddEntity( this, asw_visrange_generic.GetFloat() * 0.9f, &CASW_Door::WeldedVismonCallback, NULL );
	}
	else
	{
		VisibilityMonitor_AddEntity( this, asw_visrange_generic.GetFloat() * 0.9f, &CASW_Door::DestroyVismonCallback, &CASW_Door::DestroyVismonEvaluator );
	}
}

bool CASW_Door::DestroyVismonEvaluator( CBaseEntity *pVisibleEntity, CBasePlayer *pViewingPlayer )
{
	CASW_Door *pDoor = assert_cast< CASW_Door * >( pVisibleEntity );

	if ( !pDoor->m_bShootable )
		return false;

	if ( pDoor->m_bDoAutoShootChatter )
		return true;

	if ( pDoor->m_iHealth <= 0 )
		return false;

	if ( float( pDoor->m_iHealth ) / pDoor->GetMaxHealth() > 0.4f )
		return false;

	return true;
}

bool CASW_Door::DestroyVismonCallback( CBaseEntity *pVisibleEntity, CBasePlayer *pViewingPlayer )
{
	IGameEvent *event = gameeventmanager->CreateEvent( "door_recommend_destroy" );
	if ( event )
	{
		event->SetInt( "userid", pViewingPlayer->GetUserID() );
		event->SetInt( "entindex", pVisibleEntity->entindex() );
		gameeventmanager->FireEvent( event );
	}

	return false;
}

bool CASW_Door::WeldedVismonCallback( CBaseEntity *pVisibleEntity, CBasePlayer *pViewingPlayer )
{
	IGameEvent *event = gameeventmanager->CreateEvent( "door_welded_visible" );
	if ( event )
	{
		event->SetInt( "userid", pViewingPlayer->GetUserID() );
		event->SetInt( "subject", pVisibleEntity->entindex() );
		event->SetString( "entityname", STRING( pVisibleEntity->GetEntityName() ) );
		gameeventmanager->FireEvent( event );
	}

	return false;
}

void CASW_Door::RunAnimation()
{
	m_flPlaybackRate = 1.0f;
	StudioFrameAdvance();
	DispatchAnimEvents( this );
	SetContextThink( &CASW_Door::RunAnimation, gpGlobals->curtime + 0.1f, s_pDoorAnimThink );

	m_bSkillMarineHelping = ( m_fSkillMarineHelping >= gpGlobals->curtime - 0.2f );
}

// is this door open or not?
bool CASW_Door::IsOpen( void )
{
	//return ( m_vecGoal == m_vecOpenPosition );
	Vector diff = GetAbsOrigin() - m_vecClosedPosition;
	float dist = diff.LengthSqr();
	return ( dist > 2 );
}

bool CASW_Door::IsMoving()
{
	return GetLocalVelocity().LengthSqr() > 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Door::OnDoorOpened( void )
{
	if ( m_hDoorBlocker != NULL )
	{
		// Allow passage through this blocker while open
		m_hDoorBlocker->AddSolidFlags( FSOLID_NOT_SOLID );

		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( GetAbsOrigin(), m_hDoorBlocker->CollisionProp()->OBBMins(), m_hDoorBlocker->CollisionProp()->OBBMaxs(), 0, 255, 0, true, 1.0f );
		}
	}
	if ( m_hDoorPadding != NULL )
	{
		m_hDoorPadding->AddSolidFlags( FSOLID_NOT_SOLID );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Door::OnDoorClosed( void )
{
	if ( m_hDoorBlocker != NULL )
	{
		// Destroy the blocker that was preventing NPCs from getting in our way.
		UTIL_Remove( m_hDoorBlocker );

		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( GetAbsOrigin(), m_hDoorBlocker->CollisionProp()->OBBMins(), m_hDoorBlocker->CollisionProp()->OBBMaxs(), 0, 255, 0, true, 1.0f );
		}
	}
	if ( m_hDoorPadding != NULL )
	{
		m_hDoorPadding->RemoveSolidFlags( FSOLID_NOT_SOLID );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Returns whether the way is clear for the door to close.
// Input  : state - Which sides to check, forward, backward, or both.
// Output : Returns true if the door can close, false if the way is blocked.
//-----------------------------------------------------------------------------
bool CASW_Door::DoorCanClose( bool bAutoClose )
{
	if ( GetMaster() != NULL )
		return GetMaster()->DoorCanClose( bAutoClose );

	// Check all slaves
	if ( HasSlaves() )
	{
		int	numDoors = m_hDoorList.Count();

		CASW_Door *pLinkedDoor = NULL;

		// Check all links as well
		CBasePropDoor *pBasePD;
		for ( int i = 0; i < numDoors; i++ )
		{
			pBasePD = m_hDoorList[i].Get();
			if ( pBasePD && pBasePD->Classify() == CLASS_ASW_DOOR )
			{
				pLinkedDoor = assert_cast< CASW_Door * >( pBasePD );
				if ( !pLinkedDoor->CheckDoorClear() )
					return false;
			}
		}
	}

	// See if our path of movement is clear to allow us to shut
	return CheckDoorClear();
}

void CASW_Door::CalculateDoorVolume( Vector OpenPosition, Vector ClosedPosition, Vector *destMins, Vector *destMaxs )
{
	// Save our current position and move to our start position
	Vector	savePosition = GetAbsOrigin();
	SetAbsOrigin( ClosedPosition );

	// Find our AABB at the closed state
	Vector	closedMins, closedMaxs;
	CollisionProp()->WorldSpaceAABB( &closedMins, &closedMaxs );

	SetAbsOrigin( OpenPosition );

	// Find our AABB at the open state
	Vector	openMins, openMaxs;
	CollisionProp()->WorldSpaceAABB( &openMins, &openMaxs );

	// Reset our position to our starting position
	SetAbsOrigin( savePosition );

	// Find the minimum extents
	UTIL_ComputeAABBForBounds( closedMins, closedMaxs, openMins, openMaxs, destMins, destMaxs );

	// Move this back into local space
	*destMins -= GetAbsOrigin();
	*destMaxs -= GetAbsOrigin();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Door::OnRestore( void )
{
	BaseClass::OnRestore();

	// Figure out our volumes of movement as this door opens
	CalculateDoorVolume( m_vecOpenPosition, m_vecClosedPosition, &m_vecBoundsMin, &m_vecBoundsMax );
}

class CASWTraceFilterDoor : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CASWTraceFilterDoor );

	CASWTraceFilterDoor( const IHandleEntity *pDoor, const IHandleEntity *passentity, int collisionGroup )
		: m_pDoor( pDoor ), m_pPassEnt( passentity ), m_collisionGroup( collisionGroup )
	{
	}

	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !StandardFilterRules( pHandleEntity, contentsMask ) )
			return false;

		if ( !PassServerEntityFilter( pHandleEntity, m_pDoor ) )
			return false;

		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;

		// Don't test if the game code tells us we should ignore this collision...
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

		if ( pEntity )
		{
			if ( dynamic_cast< CFunc_ASW_Fade * >( pEntity ) || dynamic_cast< CProp_ASW_Fade * >( pEntity ) )
				return false;

			if ( !pEntity->ShouldCollide( m_collisionGroup, contentsMask ) )
				return false;

			if ( !g_pGameRules->ShouldCollide( m_collisionGroup, pEntity->GetCollisionGroup() ) )
				return false;

			// If objects are small enough and can move, close on them
			if ( pEntity->GetMoveType() == MOVETYPE_VPHYSICS )
			{
				IPhysicsObject *pPhysics = pEntity->VPhysicsGetObject();
				Assert( pPhysics );

				// Must either be squashable or very light
				if ( pPhysics->IsMoveable() && pPhysics->GetMass() < 32 )
					return false;
			}
		}

		return true;
	}

private:

	const IHandleEntity *m_pDoor;
	const IHandleEntity *m_pPassEnt;
	int m_collisionGroup;
};

inline void TraceHull_Door( const CBasePropDoor *pDoor, const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &hullMin,
	const Vector &hullMax, unsigned int mask, const CBaseEntity *ignore,
	int collisionGroup, trace_t *ptr )
{
	Ray_t ray;
	ray.Init( vecAbsStart, vecAbsEnd, hullMin, hullMax );
	CASWTraceFilterDoor traceFilter( pDoor, ignore, collisionGroup );
	enginetrace->TraceRay( ray, mask, &traceFilter, ptr );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : forward - 
//			mask - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CASW_Door::CheckDoorClear()
{
	// Look for blocking entities, ignoring ourselves and the entity that opened us.
	Vector vecClosed = m_vecClosedPosition;
	trace_t	tr;
	TraceHull_Door( this, vecClosed, vecClosed, m_vecBoundsMin, m_vecBoundsMax, MASK_SOLID, GetActivator(), COLLISION_GROUP_NONE, &tr );
	if ( tr.allsolid || tr.startsolid )
	{
		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( vecClosed, m_vecBoundsMin, m_vecBoundsMax, 255, 0, 0, true, 10.0f );

			if ( tr.m_pEnt )
			{
				NDebugOverlay::Box( tr.m_pEnt->GetAbsOrigin(), tr.m_pEnt->CollisionProp()->OBBMins(), tr.m_pEnt->CollisionProp()->OBBMaxs(), 220, 220, 0, true, 10.0f );
			}
		}

		return false;
	}

	if ( g_debug_doors.GetBool() )
	{
		NDebugOverlay::Box( vecClosed, m_vecBoundsMin, m_vecBoundsMax, 0, 255, 0, true, 10.0f );
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Puts the door in its appropriate position for spawning.
//-----------------------------------------------------------------------------
void CASW_Door::DoorTeleportToSpawnPosition()
{
	Vector vecSpawn;

	// The Start Open spawnflag trumps the choices field
	if ( HasSpawnFlags( SF_DOOR_START_OPEN_OBSOLETE ) || m_eSpawnPosition == DOOR_SPAWN_OPEN )
	{
		vecSpawn = m_vecOpenPosition;
		SetDoorState( DOOR_STATE_OPEN );
	}
	else if ( m_eSpawnPosition == DOOR_SPAWN_CLOSED )
	{
		vecSpawn = m_vecClosedPosition;
		SetDoorState( DOOR_STATE_CLOSED );
	}
	else
	{
		// Bogus spawn position setting!
		Assert( false );
		vecSpawn = m_vecClosedPosition;
		SetDoorState( DOOR_STATE_CLOSED );
	}

	SetAbsOrigin( vecSpawn );

	// Doesn't relink; that's done in Spawn.
}


//-----------------------------------------------------------------------------
// Purpose: After moving, set position to the exact final position, call "move done" function.
//-----------------------------------------------------------------------------
void CASW_Door::MoveDone()
{
	SetAbsOrigin( m_vecGoal );
	SetLocalVelocity( vec3_origin );
	SetMoveDoneTime( -1 );
	BaseClass::MoveDone();
}


//-----------------------------------------------------------------------------
// Purpose: Calculate m_vecVelocity and m_flNextThink to reach vecDest from
//			GetLocalOrigin() traveling at flSpeed. Just like LinearMove, but rotational.
// Input  : vecDestPosition - 
//			flSpeed - 
//-----------------------------------------------------------------------------
void CASW_Door::SlideMove( const Vector &vecDestPosition, float flSpeed )
{
	ASSERTSZ( flSpeed != 0, "SlideMove:  no speed is defined!" );

	// no moving if our door is dead
	//if (m_lifeState = LIFE_DEAD)
	//{
		//MoveDone();
		//return;
	//}

	m_vecGoal = vecDestPosition;

	// Already there?
	if ( vecDestPosition == GetAbsOrigin() )
	{
		MoveDone();
		return;
	}

	// Set destdelta to the vector needed to move.
	Vector vecDestDelta = vecDestPosition - GetAbsOrigin();

	// Divide by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// Call MoveDone when destination position is reached.
	SetMoveDoneTime( flTravelTime );

	// Scale the destdelta vector by the time spent traveling to get velocity.
	SetLocalVelocity( vecDestDelta * ( 1.0 / flTravelTime ) );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Door::BeginOpening( CBaseEntity *pOwner )
{
	Vector vecOpen = m_vecOpenPosition;

	// turn off our padding, so it doesn't drag marines along
	if ( m_hDoorPadding != NULL )
	{
		m_hDoorPadding->AddSolidFlags( FSOLID_NOT_SOLID );
	}

	// Create the door blocker
	Vector mins, maxs;
	mins = m_vecBoundsMin;
	maxs = m_vecBoundsMax;

	if ( m_hDoorBlocker != NULL )
	{
		UTIL_Remove( m_hDoorBlocker );
	}

	// Create a blocking entity to keep random entities out of our movement path
	m_hDoorBlocker = CEntityBlocker::Create( GetAbsOrigin(), mins, maxs, pOwner, false );

	Vector	volumeCenter = ( ( mins + maxs ) * 0.5f ) + GetAbsOrigin();

	// Ignoring the Z
	float volumeRadius = MAX( fabs( mins.x ), maxs.x );
	volumeRadius = MAX( volumeRadius, MAX( fabs( mins.y ), maxs.y ) );

	// Debug
	if ( g_debug_doors.GetBool() )
	{
		NDebugOverlay::Cross3D( volumeCenter, -Vector( volumeRadius, volumeRadius, volumeRadius ), Vector( volumeRadius, volumeRadius, volumeRadius ), 255, 0, 0, true, 1.0f );
	}

	// Make respectful entities move away from our path
	CSoundEnt::InsertSound( SOUND_MOVE_AWAY, volumeCenter, volumeRadius, 0.5f, pOwner );

	// Do final setup
	if ( m_hDoorBlocker != NULL )
	{
		// Only block NPCs
		m_hDoorBlocker->SetCollisionGroup( COLLISION_GROUP_DOOR_BLOCKER );

		// If we hit something while opening, just stay unsolid until we try again
		if ( CheckDoorClear() == false )
		{
			m_hDoorBlocker->AddSolidFlags( FSOLID_NOT_SOLID );
		}

		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( GetAbsOrigin(), m_hDoorBlocker->CollisionProp()->OBBMins(), m_hDoorBlocker->CollisionProp()->OBBMaxs(), 255, 0, 0, true, 1.0f );
		}
	}

	SlideMove( vecOpen, m_flSpeed );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Door::BeginClosing( void )
{
	if ( m_hDoorBlocker != NULL )
	{
		// Become solid again unless we're already being blocked
		if ( CheckDoorClear() )
		{
			m_hDoorBlocker->RemoveSolidFlags( FSOLID_NOT_SOLID );
		}

		if ( g_debug_doors.GetBool() )
		{
			NDebugOverlay::Box( GetAbsOrigin(), m_hDoorBlocker->CollisionProp()->OBBMins(), m_hDoorBlocker->CollisionProp()->OBBMaxs(), 255, 0, 0, true, 1.0f );
		}
	}

	SlideMove( m_vecClosedPosition, m_flSpeed );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CASW_Door::DoorStop( void )
{
	SetLocalVelocity( vec3_origin );
	SetMoveDoneTime( -1 );
}

//-----------------------------------------------------------------------------
// Purpose: Restart a door moving that was temporarily paused
//-----------------------------------------------------------------------------
void CASW_Door::DoorResume( void )
{
	// Restart our linear movement
	SlideMove( m_vecGoal, m_flSpeed );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : vecMoveDir - 
//			opendata - 
//-----------------------------------------------------------------------------
void CASW_Door::GetNPCOpenData( CAI_BaseNPC *pNPC, opendata_t &opendata )
{
	// dvs: TODO: finalize open position, direction, activity
	Vector vecForward;
	Vector vecRight;
	AngleVectors( GetAbsAngles(), &vecForward, &vecRight, NULL );

	//
	// Figure out where the NPC should stand to open this door,
	// and what direction they should face.
	//
	opendata.vecStandPos = GetAbsOrigin() - ( vecRight * 24 );
	opendata.vecStandPos.z -= 54;

	//Vector vecNPCOrigin = pNPC->GetAbsOrigin();

	if ( pNPC->GetAbsOrigin().Dot( vecForward ) > GetAbsOrigin().Dot( vecForward ) )
	{
		// In front of the door relative to the door's forward vector.
		opendata.vecStandPos += vecForward * 64;
		opendata.vecFaceDir = -vecForward;
	}
	else
	{
		// Behind the door relative to the door's forward vector.
		opendata.vecStandPos -= vecForward * 64;
		opendata.vecFaceDir = vecForward;
	}

	opendata.eActivity = ACT_OPEN_DOOR;
}


//-----------------------------------------------------------------------------
// Purpose: Returns how long it will take this door to open.
//-----------------------------------------------------------------------------
float CASW_Door::GetOpenInterval()
{
	// set destdelta to the vector needed to move
	Vector vecDestDelta = m_vecOpenPosition - GetAbsOrigin();

	// divide by speed to get time to reach dest
	return vecDestDelta.Length() / m_flSpeed;
}


//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CASW_Door::DrawDebugTextOverlays( void )
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if ( m_debugOverlays & OVERLAY_TEXT_BIT )
	{
		char tempstr[512];
		Q_snprintf( tempstr, sizeof( tempstr ), "Avelocity: %.2f %.2f %.2f", GetLocalVelocity().x, GetLocalVelocity().y, GetLocalVelocity().z );
		NDebugOverlay::EntityText( entindex(), text_offset, tempstr, 0 );
		text_offset++;

		if ( IsDoorOpen() )
		{
			Q_strncpy( tempstr, "DOOR STATE: OPEN", sizeof( tempstr ) );
		}
		else if ( IsDoorClosed() )
		{
			Q_strncpy( tempstr, "DOOR STATE: CLOSED", sizeof( tempstr ) );
		}
		else if ( IsDoorOpening() )
		{
			Q_strncpy( tempstr, "DOOR STATE: OPENING", sizeof( tempstr ) );
		}
		else if ( IsDoorClosing() )
		{
			Q_strncpy( tempstr, "DOOR STATE: CLOSING", sizeof( tempstr ) );
		}
		else if ( IsDoorAjar() )
		{
			Q_strncpy( tempstr, "DOOR STATE: AJAR", sizeof( tempstr ) );
		}
		NDebugOverlay::EntityText( entindex(), text_offset, tempstr, 0 );
		text_offset++;
	}

	return text_offset;
}


void CASW_Door::InputNPCNear( inputdata_t &inputdata )
{
	// a marine or an alien has come near the door
	Msg( "[S] NPC near door\n" );
}

// returns how sealed this door is, from 0 to 1.0
float CASW_Door::GetSealAmount()
{
	if ( m_flTotalSealTime <= 0 )
		return 0;

	return ( m_flCurrentSealTime / m_flTotalSealTime );
}

void CASW_Door::SetCurrentSealTime( float fTime )
{
	m_flCurrentSealTime = fTime;
}

void CASW_Door::SetTotalSealTime( float fTime )
{
	m_flTotalSealTime = fTime;
}

bool CASW_Door::IsDoorLocked()
{
	bool bDented = ( m_DentAmount > ASWDD_PARTIAL );
	if ( !bDented && HasSlaves() )
	{
		int	numDoors = m_hDoorList.Count();
		CASW_Door *pSlave = NULL;
		CBasePropDoor *pBasePD;
		for ( int i = 0; i < numDoors; i++ )
		{
			pBasePD = m_hDoorList[i].Get();
			if ( pBasePD && pBasePD->Classify() == CLASS_ASW_DOOR )
			{
				pSlave = assert_cast< CASW_Door * >( pBasePD );
				if ( pSlave->m_DentAmount > ASWDD_PARTIAL )
				{
					bDented = true;
					break;
				}
			}
		}
	}
	return BaseClass::IsDoorLocked() || bDented || ( GetCurrentSealTime() > 0 );
}

void CASW_Door::WeldDoor( bool bSeal, float fAmount, CASW_Marine *pMarine )
{
	if ( !pMarine )
		return;

	float fCurrentSealTime = GetCurrentSealTime();

	if ( !bSeal )
	{
		fAmount = -fAmount;
	}
	else
	{
		m_bWasWeldedByMarine = true;
	}

	// once a door has been welded a bit, don't have anyone complain about it being sealed
	m_bDoCutShout = false;

	if ( m_fChatterCounter * fAmount < 0 )	// reset the chatter counter if we switch weld direction
	{
		m_fChatterCounter = 0;
		m_bDoneChatter = false;
	}
	if ( !m_bDoneChatter )
	{
		m_fChatterCounter += fAmount;
		if ( m_fChatterCounter > 1.0f )
		{
			pMarine->GetMarineSpeech()->Chatter( CHATTER_SEALING_DOOR );
			m_bDoneChatter = true;
		}
		else if ( m_fChatterCounter < -1.0f )
		{
			pMarine->GetMarineSpeech()->Chatter( CHATTER_CUTTING_DOOR );
			m_bDoneChatter = true;
		}
	}

	float fNewTime = fCurrentSealTime + fAmount;
	if ( fNewTime <= 0 )
	{
		if ( fCurrentSealTime > 0 )
		{
			m_OnFullyCut.FireOutput( pMarine, this );
			// door completely opened
			if ( IsAutoOpen() )
			{
				variant_t emptyVariant;
				AcceptInput( "Open", pMarine, this, emptyVariant, 0 );
			}
		}
		fNewTime = 0;
	}

	if ( fNewTime >= GetTotalSealTime() )
	{
		if ( fCurrentSealTime < fNewTime )
		{
			// door is completely sealed	
			m_OnFullySealed.FireOutput( pMarine, this );
		}
		//Msg(" Fully sealed, cur=%f last=%f inh=%d commander=%d\n", gpGlobals->curtime, m_fLastFullyWeldedSound,
				//pMarine->IsInhabited(), pMarine->GetCommander() != NULL );
		if ( gpGlobals->curtime > ( m_fLastFullyWeldedSound + 1.0f ) && pMarine->IsInhabited() && pMarine->GetCommander() )
		{
			m_fLastFullyWeldedSound = gpGlobals->curtime;
			//Msg("Playing deny sound time=%f!\n", m_fLastFullyWeldedSound);
			// play a deny sound to the marine's owner
			pMarine->GetCommander()->EmitPrivateSound( "ASW_Welder.WeldDeny", true );
		}
		fNewTime = GetTotalSealTime();
	}

	if ( fCurrentSealTime != fNewTime )
	{
		// sparks now spawn from the welder weapon

		if ( fCurrentSealTime > 0.0f && fNewTime <= 0.0f )
		{
			// Door is now unsealed!
			IGameEvent *event = gameeventmanager->CreateEvent( "door_unwelded" );
			if ( event )
			{
				CASW_Player *pPlayer = NULL;
				pPlayer = pMarine->GetCommander();

				event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
				event->SetInt( "entindex", entindex() );
				gameeventmanager->FireEvent( event );
			}
		}
		else if ( fCurrentSealTime < 1.0f && fNewTime >= 1.0f )
		{
			// Door is now sealed!
			IGameEvent *event = gameeventmanager->CreateEvent( "door_welded" );
			if ( event )
			{
				CASW_Player *pPlayer = NULL;
				pPlayer = pMarine->GetCommander();

				event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
				event->SetInt( "entindex", entindex() );
				event->SetInt( "inhabited", pMarine->IsInhabited() );
				gameeventmanager->FireEvent( event );
			}
		}

		SetCurrentSealTime( fNewTime );

		if ( !m_bHasBeenWelded && bSeal && m_flCurrentSealTime > 0.0f )
		{
			m_bHasBeenWelded = true;
			ASWFailAdvice()->OnMarineWeldedDoor();
		}
	}
}

bool CASW_Door::CloseForWeld( CASW_Marine *pMarine )
{
	if ( m_bCanCloseToWeld )
	{
		variant_t emptyVariant;
		AcceptInput( "Close", pMarine, this, emptyVariant, 0 );
		m_fClosingToWeldTime = gpGlobals->curtime + 1.0f;
		return true;
	}
	return false;
}

void CASW_Door::AutoOpen( CBaseEntity *pOther )
{
	if ( gpGlobals->curtime > m_fClosingToWeldTime )
	{
		variant_t emptyVariant;
		AcceptInput( "Open", pOther, this, emptyVariant, 0 );
	}
}

// the point a marine should look to weld this door
// sparks come from here also

#define DOOR_CORNER_DISTANCE 62.0f
#define DOOR_HEIGHT 135.0f
Vector CASW_Door::GetWeldFacingPoint( CBaseEntity *pOther )
{
	// work out which side of the door the marine is on
	Vector diff = pOther->GetAbsOrigin() - GetAbsOrigin();
	VectorNormalize( diff );
	QAngle angDoorFacing = GetAbsAngles();
	Vector vecDoorFacing = vec3_origin;
	AngleVectors( angDoorFacing, &vecDoorFacing );
	bool bFrontSide = ( DotProduct( vecDoorFacing, diff ) > 0 );

	// depending on the side, get one of the corners
	angDoorFacing.y -= bFrontSide ? 81 : 102;
	AngleVectors( angDoorFacing, &vecDoorFacing );
	Vector result = GetAbsOrigin() + vecDoorFacing * DOOR_CORNER_DISTANCE;

	// correct by height
	result.z += DOOR_HEIGHT * ( 1.0f - GetSealAmount() );

	return result;
}

bool CASW_Door::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( FStrEq( szKeyName, "health" ) )
	{
		// skip over the CBaseProp KeyValue which discards health setting
		return CBaseAnimating::KeyValue( szKeyName, szValue );
	}

	return BaseClass::KeyValue( szKeyName, szValue );
}

int CASW_Door::OnTakeDamage( const CTakeDamageInfo &info )
{
	if ( !edict() || !m_takedamage )
		return 0;

	CBaseEntity *pAttacker = info.GetAttacker();
	if ( ( m_takedamage != DAMAGE_EVENTS_ONLY ) && pAttacker && pAttacker->Classify() == CLASS_ASW_MARINE )
	{
		CASW_Marine *pMarine = assert_cast< CASW_Marine * >( pAttacker );
		pMarine->HurtJunkItem( this, info );
	}

	if ( info.GetDamageType() == DMG_SLASH || info.GetDamageType() == DMG_CLUB )			// if an alien claw attack, then check if it's bashable
	{
		EmitSound( STRING( m_iszDoorHitSound ) );

		if ( !m_bBashable )
			return 0;
	}
	else if ( !m_bShootable )			// otherwise, check if it's shootable
		return 0;

	// door doesn't take damage when it's open
	// todo: take damage, but not below a dent boundary?
	//if (IsDoorOpen() || IsDoorOpening())
	if ( !IsDoorClosed() )
		return 0;

	// stop buzzers from knocking down the door in 1 swoop (their physics??)
	if ( pAttacker && pAttacker->Classify() == CLASS_ASW_BUZZER )
		return 0;

	Vector vecTemp;
	if ( info.GetInflictor() )
	{
		vecTemp = info.GetInflictor()->WorldSpaceCenter() - ( WorldSpaceCenter() );
	}
	else
	{
		vecTemp.Init( 1, 0, 0 );
	}

	// this global is still used for glass and other non-NPC killables, along with decals.
	g_vecAttackDir = vecTemp;
	VectorNormalize( g_vecAttackDir );

	m_vLastDamageDir = g_vecAttackDir;

	if ( m_takedamage != DAMAGE_EVENTS_ONLY )
	{
		CTakeDamageInfo newInfo( info );

		float damage = info.GetDamage();
		// reduce damage from shotguns and mining laser
		if ( info.GetDamageType() & DMG_ENERGYBEAM )
		{
			damage *= 0.4f;	// still makes the mining laser the best weapon for taking down doors
		}
		if ( info.GetDamageType() & DMG_BUCKSHOT )
		{
			damage *= 0.42f;	// makes level 3 skill vindicator take around 16 seconds, normal shotgun around 26 (worse than rifle :/)
		}

		if ( info.GetInflictor() && info.GetInflictor()->Classify() == CLASS_ASW_T75 )
		{
			damage *= 7.0f;		// take extra damage from T75
		}

		if ( info.GetDamageType() & DMG_BULLET )
		{
			if ( pAttacker && pAttacker->Classify() == CLASS_ASW_MARINE )
			{
				CASW_Weapon *pWeapon = assert_cast< CASW_Marine * >( pAttacker )->GetActiveASWWeapon();
				if ( pWeapon )
				{
					if ( pWeapon->Classify() == CLASS_ASW_DEAGLE )
					{
						damage *= 0.4f; // deagle isn't a door killer gun
					}
					else if ( pWeapon->Classify() == CLASS_ASW_SNIPER_RIFLE )
					{
						CASW_Weapon_Sniper_Rifle *pSniper = assert_cast< CASW_Weapon_Sniper_Rifle * >( pWeapon );
						if ( pSniper->IsZoomed() ) //zoomed sniper bonus damage does not affect doors
							damage -= pSniper->GetZoomedDamageBonus();
					}
				}
			}
		}

		if ( pAttacker )
		{
			if ( pAttacker->Classify() == CLASS_ASW_DRONE )	// scale alien damage
			{
				// Undo difficulty damage adjust
				float fDiff = ASWGameRules()->GetMissionDifficulty() - 5;
				float f = 1.0 + fDiff * asw_difficulty_alien_damage_step.GetFloat();
				damage /= f;

				damage *= asw_door_drone_damage_scale.GetFloat();
			}
			else if ( pAttacker->Classify() == CLASS_ASW_MARINE && info.GetDamageType() & DMG_CLUB )	// make doors immune to kick damage
			{
				if ( !rd_door_melee_damage.GetBool() )
					damage *= 0;
			}

			// reduce damage from welding
			if ( GetTotalSealTime() > 0 )
			{
				float fSeal = GetCurrentSealTime() / GetTotalSealTime();
				if ( fSeal > 0 && fSeal <= 1.0f )
				{
					// up to X% damage reduction
					fSeal *= asw_door_seal_damage_reduction.GetFloat();
					damage *= ( 1.0f - fSeal );
				}
			}
		}

		newInfo.SetDamage( damage );

		CheckForDoorShootChatter( newInfo );

		int iHealthBefore = m_iHealth;
		// do the damage
		m_iHealth -= damage;

		//Msg("Door health now %d (%d) seq %d frame %f DoorOpen=%d DoorOpening=%d DoorClosed=%d DoorClosing=%d\n",
			//m_iHealth, (int) m_DentAmount, GetSequence(), GetCycle(),
			//IsDoorOpen(), IsDoorOpening(), IsDoorClosed(), IsDoorClosing() );

		if ( m_iHealth <= 0 )
		{
			const int needed_flip_damage = 50;
			// can't kill the door if it's not facing the right way

			//change this to check if the door is denting the right way
			//Msg( "Door dead. needs flip = %d  flipped = %d\n", DoorNeedsFlip(), m_bFlipped );

			if ( m_DentAmount == ASWDD_NONE )
			{
				if ( DoorNeedsFlip() )
				{
					FlipDoor();
				}

				m_DentAmount = ASWDD_COMPLETE;
				SetDentSequence();

				Event_Killed( newInfo );
			}
			else if ( DoorNeedsFlip() )
			{
				//Msg( "  keeping door alive\n" );
				m_iHealth = 1;
				m_fLastMomentFlipDamage += newInfo.GetDamage();
				if ( m_fLastMomentFlipDamage >= needed_flip_damage )
				{
					m_fLastMomentFlipDamage = 0;
					// put it in the middle and the run setdentsequence to actually flip it
					m_DentAmount = ASWDD_NONE;
					SetDentSequence();

					// now make sure it's completely dented in this new direction (looks weird if the door straightens itself out)
					m_DentAmount = ASWDD_COMPLETE;
					SetDentSequence();
					m_fLastMomentFlipDamage = -needed_flip_damage;
				}
			}
			else	// door is facing the right way and is fully dented, let's make it fall over
			{
				if ( m_fLastMomentFlipDamage < 0 )		// count up a small amount of damage so it doesn't fall immediately after changing facing
				{
					m_iHealth = 1;
					m_fLastMomentFlipDamage += newInfo.GetDamage();
				}
				else
				{
					Event_Killed( newInfo );
				}
			}
		}
		else	// door is still alive
		{
			// figure out how dented the door is		
			if ( m_iHealth > ASW_DOOR_PARTIAL_DENT_HEALTH * m_iDoorStrength )
			{
				m_DentAmount = ASWDD_NONE;
			}
			else if ( m_iHealth > ASW_DOOR_COMPLETE_DENT_HEALTH * m_iDoorStrength )
			{
				m_DentAmount = ASWDD_PARTIAL;
			}
			else
			{
				m_DentAmount = ASWDD_COMPLETE;
			}

			SetDentSequence();
		}

		UTIL_RD_HitConfirm( this, iHealthBefore, newInfo );
	}

	return 1;
}

bool CASW_Door::DoorNeedsFlip( void )
{
	// check if we need to rotate the prop to make the right side anim
	Vector vecFacing;
	AngleVectors( GetAbsAngles(), &vecFacing );
	float dot = DotProduct( vecFacing, m_vLastDamageDir );
	bool bNeedFlip = ( dot > 0 );
	if ( bNeedFlip != m_bFlipped )
	{
		return true;
	}
	return false;
}

void CASW_Door::SetDentSequence()
{
	if ( m_lifeState == LIFE_DEAD )
		return;

	if ( m_DentAmount == ASWDD_NONE )	// always check for flipping if the door is in the 'middle' position
	{
		if ( DoorNeedsFlip() )
			FlipDoor();

		m_bSetSide = true;
	}

	// old (non-body group) doors use sequences for denting
	int iSeq;
	switch ( m_DentAmount )
	{
	case ASWDD_COMPLETE:
		iSeq = LookupSequence( "dent2" );
		break;
	case ASWDD_PARTIAL:
		iSeq = LookupSequence( "dent1" );
		break;
	default:
		iSeq = LookupSequence( "still" );
		break;
	}

	if ( iSeq == ACT_INVALID )
	{
		// we're using a door that supports body groups (or is missing animations, whoops)
		SetDoorDamage();
		return;
	}

	// only do denting when the door is shut
	if ( iSeq != GetSequence() && !IsDoorOpen() && !IsDoorOpening() )
	{
		//Msg("Setting door sequence to %d and cycle to 0\n", iSeq);
		ResetSequence( iSeq );
		ResetClientsideFrame();

		// spawn some smoke and stuff, since the door just got dented a bit
		DoorSmoke();
	}
}

void CASW_Door::SetDoorDamage()
{
	// Body groups for doors (in order):
	// - Undamaged
	// - Partially dented from front (toward +x)
	// - Completely dented from front (toward +x) (door_rear_fall)
	// - Partially dented from rear (toward -x)
	// - Completely dented from rear (toward -x) (door_front_fall)

	int iDamageGroup;
	switch ( m_DentAmount )
	{
	case ASWDD_COMPLETE:
		iDamageGroup = 4;
		break;
	case ASWDD_PARTIAL:
		iDamageGroup = 3;
		break;
	default:
		iDamageGroup = 0;
		break;
	}

	// apply flip
	if ( m_bFlipped )
	{
		if ( iDamageGroup == 3 )
			iDamageGroup = 1;
		else if ( iDamageGroup == 4 )
			iDamageGroup = 2;
	}

	// only do denting when the door is shut
	if ( iDamageGroup != GetSequence() && !IsDoorOpen() && !IsDoorOpening() )
	{
		SetBodygroup( 0, iDamageGroup );

		// spawn some smoke and stuff, since the door just got dented a bit
		DoorSmoke();
	}
}


void CASW_Door::DoorSmoke()
{
	CASW_Emitter *pEmitter = ( CASW_Emitter * )CreateEntityByName( "asw_emitter" );
	if ( pEmitter )
	{
		pEmitter->UseTemplate( "doorsmoke1" );
		UTIL_SetOrigin( pEmitter, GetAbsOrigin() );
		pEmitter->SetAbsAngles( GetAbsAngles() );
		pEmitter->Spawn();
		g_EventQueue.AddEvent( pEmitter, "Kill", 2.0f, pEmitter, pEmitter );
	}
	EmitSound( STRING( m_iszDoorDentSound ) );
}

void CASW_Door::Event_Killed( const CTakeDamageInfo &info )
{
	m_OnDestroyed.FireOutput( info.GetInflictor(), this );

	CBaseEntity *pAttacker = info.GetAttacker();
	IGameEvent *event = gameeventmanager->CreateEvent( "door_destroyed" );
	if ( event )
	{
		CBasePlayer *pPlayer = NULL;
		if ( pAttacker && pAttacker->Classify() == CLASS_ASW_MARINE )
		{
			CASW_Marine *pMarine = assert_cast< CASW_Marine * >( pAttacker );
			pPlayer = pMarine->GetCommander();
		}

		event->SetInt( "userid", ( pPlayer ? pPlayer->GetUserID() : 0 ) );
		event->SetInt( "entindex", entindex() );
		gameeventmanager->FireEvent( event );
	}

	if ( pAttacker )
	{
		pAttacker->Event_KilledOther( this, info );
	}

	// check if marines should shout about this door being bashed down
	// check there's another marine nearby
	if ( m_bDoBreachedShout && pAttacker && pAttacker->Classify() == CLASS_ASW_DRONE
		&& ASWGameResource() )
	{
		CASW_Game_Resource *pGameResource = ASWGameResource();
		// count how many marines are nearby
		int iNearby = 0;
		for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
		{
			CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
			CASW_Marine *pMarine = pMR ? pMR->GetMarineEntity() : NULL;
			if ( pMarine && ( GetAbsOrigin().DistTo( pMarine->GetAbsOrigin() ) < 800 )
				&& pMarine->GetHealth() > 0 )
				iNearby++;
		}

		if ( iNearby >= 0 )
		{
			if ( asw_debug_marine_chatter.GetBool() )
				Msg( "Doing door breached chatter\n" );
			int iChatter = random->RandomInt( 0, iNearby - 1 );
			for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
			{
				CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
				CASW_Marine *pMarine = pMR ? pMR->GetMarineEntity() : NULL;
				if ( pMarine && ( GetAbsOrigin().DistTo( pMarine->GetAbsOrigin() ) < 600 )
					&& pMarine->GetHealth() > 0 )
				{
					if ( iChatter <= 0 )
					{
						pMarine->GetMarineSpeech()->QueueChatter( CHATTER_BREACHED_DOOR, gpGlobals->curtime + 0.5f, gpGlobals->curtime + 1.5f );
						break;
					}
					iChatter--;
				}
			}
		}
		else	// only one marine nearby, pretend we did the shout so we don't waste time checking again
		{
			if ( asw_debug_marine_chatter.GetBool() )
				Msg( "Skipping door breached chatter, no marines nearby\n" );
		}
	}

	m_bDoorFallen = true;
	m_takedamage = DAMAGE_NO;
	m_lifeState = LIFE_DEAD;

	// remove any padding childs
	for ( CBaseEntity *child = FirstMoveChild(); child != NULL; child = child->NextMovePeer() )
	{
		if ( FClassnameIs( child, "asw_door_padding" ) )
		{
			child->SetParent( NULL );
			child->SetSolid( SOLID_NONE );
			UTIL_Remove( child );
		}
	}
	m_hDoorPadding = NULL;

	DoorSmoke();

	// turn into a physics prop
	if ( asw_door_physics.GetBool() )
	{
		VPhysicsDestroyObject();
		ASWCreateVPhysics();
		if ( VPhysicsGetObject() )
		{
			Vector vecFacing;
			QAngle angFacing = GetAbsAngles();
			AngleVectors( angFacing, &vecFacing );
			VPhysicsGetObject()->ApplyForceOffset( vecFacing * 50000, WorldSpaceCenter() );
		}
		else
		{
			DevMsg( "Error, failed to create door physics object\n" );
		}
	}
	else
	{
		int iSeq = LookupSequence( m_bFlipped ? "door_rear_fall" : "door_front_fall" );
		if ( iSeq == ACT_INVALID )
		{
			// fallback for old door models
			iSeq = LookupSequence( "fall" );
		}

		CollisionProp()->SetSurroundingBoundsType( USE_HITBOXES );
		if ( iSeq != GetSequence() )
		{
			ResetSequence( iSeq );
			ResetClientsideFrame();
			DoorSmoke();
		}
	}
}

bool CASW_Door::ASWCreateVPhysics()
{
	// Create the object in the physics system
	bool asleep = HasSpawnFlags( SF_PHYSPROP_START_ASLEEP ) ? true : false;

	solid_t tmpSolid;
	PhysModelParseSolid( tmpSolid, this, GetModelIndex() );

	PhysGetMassCenterOverride( this, modelinfo->GetVCollide( GetModelIndex() ), tmpSolid );

	IPhysicsObject *pPhysicsObject = VPhysicsInitNormal( SOLID_VPHYSICS, 0, asleep, &tmpSolid );
	if ( !pPhysicsObject )
	{
		SetSolid( SOLID_NONE );
		SetMoveType( MOVETYPE_NONE );
		Warning( "ERROR!: Can't create physics object for %s\n", STRING( GetModelName() ) );
	}
	else
	{
		if ( HasSpawnFlags( SF_PHYSPROP_MOTIONDISABLED ) ) //|| m_damageToEnableMotion > 0 || m_flForceToEnableMotion > 0 )
		{
			pPhysicsObject->EnableMotion( false );
		}
	}

	// fix up any noncompliant blades.
	if ( HasInteraction( PROPINTER_PHYSGUN_LAUNCH_SPIN_Z ) )
	{
		if ( !( VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_DMG_SLICE ) )
		{
			PhysSetGameFlags( pPhysicsObject, FVPHYSICS_DMG_SLICE );
		}
	}

	if ( HasInteraction( PROPINTER_PHYSGUN_DAMAGE_NONE ) )
	{
		PhysSetGameFlags( pPhysicsObject, FVPHYSICS_NO_IMPACT_DMG );
	}

	if ( HasSpawnFlags( SF_PHYSPROP_PREVENT_PICKUP ) )
	{
		PhysSetGameFlags( pPhysicsObject, FVPHYSICS_NO_PLAYER_PICKUP );
	}

	return true;
}

void CASW_Door::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );
	NetworkStateChanged();
}

//-----------------------------------------------------------------------------
// Mass / mass center
//-----------------------------------------------------------------------------
void CASW_Door::GetMassCenter( Vector *pMassCenter )
{
	if ( !VPhysicsGetObject() )
	{
		pMassCenter->Init();
		return;
	}

	Vector vecLocal = VPhysicsGetObject()->GetMassCenterLocalSpace();
	VectorTransform( vecLocal, EntityToWorldTransform(), *pMassCenter );
}

float CASW_Door::GetMass() const
{
	return VPhysicsGetObject() ? VPhysicsGetObject()->GetMass() : 1.0f;
}

void CASW_Door::FlipDoor()
{
	m_bFlipped = !m_bFlipped;
}

// always send this entity to players (until we have a radius based network cull thing...)
int CASW_Door::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

void CASW_Door::InputEnableAutoOpen( inputdata_t &inputdata )
{
	m_bAutoOpen = true;
}

void CASW_Door::InputDisableAutoOpen( inputdata_t &inputdata )
{
	m_bAutoOpen = false;
}

void CASW_Door::InputRecommendWeld( inputdata_t &inputdata )
{
	if ( m_lifeState == LIFE_DEAD || m_DentAmount != ASWDD_NONE )
	{
		// Can't weld a broken door.
		return;
	}

	m_bRecommendedSeal = true;

	IGameEvent *event = gameeventmanager->CreateEvent( "door_recommend_weld" );
	if ( event )
	{
		event->SetInt( "entindex", entindex() );
		gameeventmanager->FireEvent( event );
	}
}

// called by door areas that want automatic door shoot or weld chatter
void CASW_Door::DoAutoDoorShootChatter( CASW_Marine *pMarine )
{
	// check there's another marine nearby
	if ( !m_bDoneDoorShout && ASWGameResource() && pMarine && pMarine->GetHealth() > 0 )
	{
		CASW_Game_Resource *pGameResource = ASWGameResource();
		// count how many are nearby and see if any have a welder
		int iNearby = 0;
		for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
		{
			CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
			CASW_Marine *pOtherMarine = pMR ? pMR->GetMarineEntity() : NULL;
			if ( pOtherMarine && ( GetAbsOrigin().DistTo( pOtherMarine->GetAbsOrigin() ) < 600 )
				&& pOtherMarine->GetHealth() > 0 )
			{
				iNearby++;
			}
		}

		if ( iNearby >= 2 )	// changed to only need the 1 marine...
		{
			if ( asw_debug_marine_chatter.GetBool() )
				Msg( "Doing door shout hint\n" );

			if ( pMarine->GetMarineSpeech()->Chatter( CHATTER_REQUEST_SHOOT_DOOR ) )
			{
				m_bDoneDoorShout = true;
			}
		}
		else	// no other marines nearby, do a shoot shout or a cut shown depending on if we have a welder
		{
			if ( asw_debug_marine_chatter.GetBool() )
				Msg( "Skipping door shout, only 1 marine nearby\n" );

			CASW_Weapon *pWeapon2 = pMarine->GetASWWeapon( 2 );
			if ( pWeapon2 && pWeapon2->Classify() == CLASS_ASW_WELDER && pMarine->GetMarineSpeech()->Chatter( CHATTER_CUTTING_DOOR ) )
			{
				m_bDoCutShout = false;
				m_bDoneDoorShout = true;
			}
			else if ( pMarine->GetMarineSpeech()->Chatter( CHATTER_REQUEST_SHOOT_DOOR ) )
			{
				m_bDoneDoorShout = true;
			}
		}
	}
}

void CASW_Door::CheckForDoorShootChatter( const CTakeDamageInfo &info )
{
	if ( m_bDoneDoorShout )
		return;

	if ( m_takedamage != DAMAGE_YES || GetHealth() <= 0 || !m_bShootable )
		return;

	if ( info.GetAttacker() && info.GetAttacker()->Classify() == CLASS_ASW_MARINE )
	{
		// reduce our block counter by the number of seconds that have passed since a marine last shot us in the front
		if ( m_fLastMarineShootTime != 0 && m_fMarineShootCounter > 0 )
		{
			if ( asw_debug_marine_chatter.GetBool() )
				Msg( "Reducing shoot counter by %f\n", ( gpGlobals->curtime - m_fLastMarineShootTime ) * 20.0f );
			m_fMarineShootCounter -= ( gpGlobals->curtime - m_fLastMarineShootTime ) * 20.0f;	// counter ticks completely down after 30 seconds
			if ( m_fMarineShootCounter < 0 )
				m_fMarineShootCounter = 0;
		}

		m_fMarineShootCounter += info.GetDamage();

		if ( asw_debug_marine_chatter.GetBool() )
			Msg( "m_fMarineShootCounter = %f\n", m_fMarineShootCounter );

		if ( m_fMarineShootCounter > 100 )		// roughly 1 second of rifle firing on normal
		{
			CASW_Marine *pMarine = assert_cast< CASW_Marine * >( info.GetAttacker() );

			// check there's another marine nearby
			if ( ASWGameResource() )
			{
				CASW_Game_Resource *pGameResource = ASWGameResource();
				// count how many are nearby
				int iNearby = 0;
				for ( int i = 0; i < pGameResource->GetMaxMarineResources(); i++ )
				{
					CASW_Marine_Resource *pMR = pGameResource->GetMarineResource( i );
					CASW_Marine *pOtherMarine = pMR ? pMR->GetMarineEntity() : NULL;
					if ( pOtherMarine && ( GetAbsOrigin().DistTo( pOtherMarine->GetAbsOrigin() ) < 600 )
						&& pOtherMarine->GetHealth() > 0 )
						iNearby++;
				}

				if ( iNearby >= 2 )
				{
					if ( asw_debug_marine_chatter.GetBool() )
						Msg( "Doing door shout hint\n" );
					if ( pMarine->GetMarineSpeech()->Chatter( CHATTER_REQUEST_SHOOT_DOOR ) )
						m_bDoneDoorShout = true;
				}
				else	// only one marine nearby, pretend we did the shout so we don't waste time checking again
				{
					if ( asw_debug_marine_chatter.GetBool() )
						Msg( "Skipping door shout, only 1 marine nearby\n" );
					m_bDoneDoorShout = true;
				}
			}
			m_fMarineShootCounter = 0;
		}
		m_fLastMarineShootTime = gpGlobals->curtime;
	}
}

void CASW_Door::HandleAnimEvent( animevent_t *pEvent )
{
	int nEvent = pEvent->Event();

	if ( nEvent == AE_START_SCRIPTED_EFFECT )
	{
		m_iFallingStage++;
		//Msg("Door fallen!\n");
		VPhysicsDestroyObject();

		// crush anyone in the way for this stage
		FallCrush();

		// spawn a blocker to cover the door's fallen position
		if ( m_iFallingStage == 2 )
			CASW_Fallen_Door_Padding::CreateFallenDoorPadding( this );

		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

// attempt to create a vphysics object on the floor in the place the door fall animation goes - doesn't work though
IPhysicsObject *CASW_Door::VPhysicsInitFallenShadow( bool allowPhysicsMovement, bool allowPhysicsRotation, solid_t *pSolid )
{
	if ( !edict() || IsMarkedForDeletion() )
		return NULL;

	// No physics
	if ( GetSolid() == SOLID_NONE )
		return NULL;

	const Vector &origin = GetAbsOrigin();
	QAngle angles = GetAbsAngles();
	Vector vecMaxs = Vector( 60, 185, 21 );
	Vector vecMins = Vector( -60, 42, 0 );
	Vector absMin = Vector( -300, -300, -300 );
	Vector absMax = Vector( 300, 300, 300 );
	CollisionProp()->SetSurroundingBoundsType( USE_SPECIFIED_BOUNDS, &absMin, &absMax );


	IPhysicsObject *pPhysicsObject = PhysModelCreate( this, GetModelIndex(), origin, angles, pSolid );
	if ( !pPhysicsObject )
		return NULL;

	VPhysicsSetObject( pPhysicsObject );
	pPhysicsObject->UpdateShadow( origin, angles, false, 0 );
	pPhysicsObject->SetShadow( 1e4, 1e4, allowPhysicsMovement, allowPhysicsRotation );
	pPhysicsObject->UpdateShadow( origin, angles, false, 0 );
	return pPhysicsObject;
}

extern ConVar ai_show_hull_attacks;
void CASW_Door::FallCrush()
{
	Vector vForward;
	QAngle ang = GetAbsAngles();
	ang[PITCH] = 0;
	ang[ROLL] = 0;

	// 	if ( DoorNeedsFlip() )
	// 	{
	// 	   ang[YAW] += 180;
	// 	}

	if ( m_bFlipped )
	{
		ang[YAW] += 180;
	}

	AngleVectors( ang, &vForward );

	KeyValues::AutoDelete pKV( "" );
	CUtlBuffer buf( 1024, 0, CUtlBuffer::TEXT_BUFFER );
	if ( modelinfo->GetModelKeyValue( GetModel(), buf ) )
	{
		pKV->LoadFromBuffer( modelinfo->GetModelName( GetModel() ), buf );
	}

	// adjust the sweep of destruction by how far fallen over the door is
	float length, start_offset;
	switch ( m_iFallingStage )
	{
	case 1:
		length = pKV->GetFloat( "asw_door/crush_length_1", 26.64f );
		start_offset = pKV->GetFloat( "asw_door/crush_offset_1", 40 );
		break;
	case 2:
		length = pKV->GetFloat( "asw_door/crush_length_2", 53.28f );
		start_offset = pKV->GetFloat( "asw_door/crush_offset_2", 49 );
		break;
	default:
		length = pKV->GetFloat( "asw_door/crush_length_3", 80.0f );
		start_offset = pKV->GetFloat( "asw_door/crush_offset_3", 99 );
		break;
	}

	Vector vStart = GetAbsOrigin() + vForward * start_offset;
	Vector vEnd = vStart + vForward * length;

	// compute a bounding box that encompasses the door, no matter its angle
	float wide = pKV->GetFloat( "asw_door/wide", 60 );
	float deep = pKV->GetFloat( "asw_door/deep", 10 );
	float tall = pKV->GetFloat( "asw_door/tall", 140 );
	Vector pre_top_left( -deep, -wide, 0 );
	Vector pre_top_right( deep, -wide, 0 );
	Vector pre_bottom_right( deep, wide, tall );
	Vector pre_bottom_left( -deep, wide, tall );
	Vector tl, tr, br, bl;

	matrix3x4_t fRotateMatrix;
	AngleMatrix( ang, fRotateMatrix );
	VectorRotate( pre_top_left, fRotateMatrix, tl );
	VectorRotate( pre_top_right, fRotateMatrix, tr );
	VectorRotate( pre_bottom_right, fRotateMatrix, br );
	VectorRotate( pre_bottom_left, fRotateMatrix, bl );

	Vector mins, maxs;
	mins.x = MIN( MIN( tl.x, tr.x ), MIN( br.x, bl.x ) );
	mins.y = MIN( MIN( tl.y, tr.y ), MIN( br.y, bl.y ) );
	mins.z = 0;
	maxs.x = MAX( MAX( tl.x, tr.x ), MAX( br.x, bl.x ) );
	maxs.y = MAX( MAX( tl.y, tr.y ), MAX( br.y, bl.y ) );
	maxs.z = tall;

	CTakeDamageInfo	dmgInfo( this, this, asw_door_damage_base.GetFloat() + ( ASWGameRules()->GetMissionDifficulty() - 5 ) * asw_door_damage_step.GetFloat(), DMG_SLASH );
	dmgInfo.SetDamageForce( vForward * 100.0f );
	dmgInfo.SetDamagePosition( vEnd );
	CASW_Trace_Filter_Door_Crush traceFilter( this, COLLISION_GROUP_NONE, &dmgInfo, 1.0f, true );

	trace_t trace;
	UTIL_TraceHull( vStart, vEnd, mins, maxs, MASK_SHOT_HULL, &traceFilter, &trace );

	if ( ai_show_hull_attacks.GetBool() )
	{
		NDebugOverlay::SweptBox( vStart, vEnd, mins, maxs, QAngle( 0, 0, 0 ), 255, 255, 0, 255, 4.0f );
		NDebugOverlay::Line( vStart + Vector( 0, 0, 30 ), vEnd + Vector( 0, 0, 30 ), 255, 255, 0, true, 3.9f );
	}
}

// extent contains the volume encompassing open + closed states
void CASW_Door::ComputeDoorExtent( Extent *extent, unsigned int extentType )
{
	if ( !extent )
		return;

	Extent closedExtent;
	CalculateDoorVolume( vec3_origin, vec3_origin, &extent->lo, &extent->hi );

	extent->lo += GetAbsOrigin();
	extent->hi += GetAbsOrigin();
}

void CASW_Door::UpdateDoorHealthOnMissionStart( int iDifficulty )
{
	if ( m_iDoorType == ASWDT_INDESTRUCTABLE || m_iDoorType == ASWDT_CUSTOM || m_flCustomDentPercentage > 0.001f )
		return;

	Assert( ( m_iDoorType == ASWDT_NORMAL && m_iDoorStrength == asw_door_normal_health_base.GetInt() ) || ( m_iDoorType == ASWDT_REINFORCED && m_iDoorStrength == asw_door_reinforced_health_base.GetInt() ) );
	Assert( m_DentAmount == ASWDD_NONE || m_DentAmount == ASWDD_PARTIAL || m_DentAmount == ASWDD_COMPLETE );

	float flScale = 1.0f;
	if ( m_DentAmount == ASWDD_PARTIAL )
	{
		flScale = ASW_DOOR_PARTIAL_DENT_HEALTH;
	}
	else if ( m_DentAmount == ASWDD_COMPLETE )
	{
		flScale = ASW_DOOR_COMPLETE_DENT_HEALTH;
	}

	Assert( m_iHealth == m_iMaxHealth );
	Assert( m_iHealth == int( m_iDoorStrength * flScale ) );

	if ( m_iDoorType == ASWDT_NORMAL )
	{
		m_iDoorStrength = asw_door_normal_health_base.GetInt() + ( iDifficulty - 5 ) * asw_door_normal_health_step.GetInt();
	}
	else
	{
		m_iDoorStrength = asw_door_reinforced_health_base.GetInt() + ( iDifficulty - 5 ) * asw_door_reinforced_health_step.GetInt();
	}

	m_iHealth = MAX( m_iDoorStrength * flScale, 1 );
	SetMaxHealth( m_iHealth );
}

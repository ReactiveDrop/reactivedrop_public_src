#include "cbase.h"
#include "asw_marine.h"
#include "asw_marine_profile.h"
#include "ai_hint.h"
#include "asw_marine_hint.h"
#include "asw_weapon_healgrenade_shared.h"
#include "asw_shieldbug.h"
#include "asw_boomer_blob.h"
#include "asw_mortarbug_shell_shared.h"
#include "asw_radiation_volume.h"
#include "ai_network.h"
#include "triggers.h"
#include "asw_path_utils.h"
#include "ai_pathfinder.h"
#include "ai_waypoint.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// TODO: these will become cheats once versus mode is available
ConVar asw_debug_marine_ai_followspot( "asw_debug_marine_ai_followspot", "0", FCVAR_NONE );
ConVar asw_debug_marine_hints( "asw_debug_marine_hints", "0", FCVAR_NONE );
ConVar asw_debug_squad_movement( "asw_debug_squad_movement", "0", FCVAR_NONE, "Draw debug overlays for squad movement" );

ConVar asw_follow_hint_max_range( "asw_follow_hint_max_range", "300", FCVAR_NONE, "If bot is this far from leader it starts to move closer" );
ConVar rd_follow_hint_max_path_bloat( "rd_follow_hint_max_path_bloat", "450", FCVAR_CHEAT, "Maximum additional pathing distance where a bot can try to pick a hint (due to non-optimal node connection choices)" );
ConVar rd_follow_hint_max_search_range( "rd_follow_hint_max_search_range", "300", FCVAR_CHEAT, "A range around leader used to search for a hint to move bot to" );
ConVar rd_follow_hint_max_search_range_danger( "rd_follow_hint_max_search_range_danger", "400", FCVAR_CHEAT, "A range around leader used to search for a hint to move bot to" );
ConVar asw_follow_hint_max_z_dist( "asw_follow_hint_max_z_dist", "120", FCVAR_CHEAT );
ConVar asw_follow_use_hints( "asw_follow_use_hints", "2", FCVAR_NONE, "0 = follow formation, 1 = use hints when in combat, 2 = always use hints" );
ConVar rd_follow_hint_delay( "rd_follow_hint_delay", "5", FCVAR_NONE, "The number of seconds marines will ignore follow hints after being told to follow" );
ConVar asw_follow_velocity_predict( "asw_follow_velocity_predict", "0.3", FCVAR_CHEAT, "Marines travelling in diamond follow formation will predict their leader's movement ahead by this many seconds" );
ConVar asw_follow_threshold( "asw_follow_threshold", "40", FCVAR_CHEAT, "Marines in diamond formation will move after leader has moved this much" );
ConVar rd_bots_flank_shieldbug( "rd_bots_flank_shieldbug", "1", FCVAR_NONE, "If 1 AI marines will try to get behind shieldbugs" );
ConVar rd_bots_avoid_bombs( "rd_bots_avoid_bombs", "1", FCVAR_NONE, "If 1 AI marines will try to find a safe place when they see alien explosives or some types of live grenades" );
ConVar rd_bots_avoid_gas( "rd_bots_avoid_gas", "1", FCVAR_NONE, "If 1 AI marines will try to find a safe place when they see gas grenades or ruptured radioactive barrels" );
ConVar rd_bots_avoid_fire( "rd_bots_avoid_fire", "1", FCVAR_NONE, "If 1 AI marines will try to avoid standing near fires" );
ConVar rd_use_info_nodes( "rd_use_info_nodes", "0", FCVAR_NONE, "If there are no info_marine_hint nodes, info_node will be used instead" );
ConVar rd_follow_hint_pathfind( "rd_follow_hint_pathfind", "2.5", FCVAR_NONE, "Avoid directing marines to stand in places that cannot be walked to by the squad leader. Remember bad nodes for this many seconds." );

bool FireSystem_IsValidFirePosition( const Vector &position, float testRadius );

static const int s_SquadDebugColors[7][3] =
{
	{ 225, 60, 60 },
	{ 200, 200, 60 },
	{ 60, 225, 60 },
	{ 30, 90, 225 },
	{ 225, 150, 30 },
	{ 225, 60, 150 },
	{ 120, 80, 250 },
};

void CASW_SquadFormation::LevelInitPostEntity()
{
	BaseClass::LevelInitPostEntity();

	m_bLevelHasFollowHints = ( MarineHintManager()->GetHintCount() > 0 );

	DevMsg( "Level has follow hints %d\n", m_bLevelHasFollowHints );

	if ( !m_bLevelHasFollowHints && rd_use_info_nodes.GetBool() )
	{
		for ( int i = 0; i < g_pBigAINet->NumNodes(); ++i )
		{
			MarineHintManager()->AddInfoNode(g_pBigAINet->GetNode(i));
		}
		m_bLevelHasFollowHints = (MarineHintManager()->GetHintCount() > 0);
		DevMsg( "Using nodes if there is no follow hints %d\n", m_bLevelHasFollowHints );
	}
}

unsigned int CASW_SquadFormation::Add( CASW_Marine *pMarine )
{
	AssertMsg( !!m_hLeader.Get(), "A CASW_SquadFormation has no leader!\n" );
	if ( pMarine == Leader() )
	{
		AssertMsg1( false, "Tried to set %s to follow itself!\n", pMarine->GetMarineProfile()->GetShortName() );
		return INVALID_SQUADDIE;
	}

	unsigned slot = Find(pMarine);
	if ( IsValid( slot ) )
	{
		AssertMsg2( false, "Tried to double-add %s to squad (already in slot %d)\n",
			pMarine->GetMarineProfile()->GetShortName(),
			slot );
		return slot;
	}
	else
	{
		for ( slot = 0 ; slot < MAX_SQUAD_SIZE; ++slot )
		{
			if ( !Squaddie(slot) )
			{
				m_hSquad[slot] = pMarine;
				return slot;
			}
		}

		// if we're down here, the squad is full!
		AssertMsg2( false, "Tried to add %s to %s's squad, but that's full! (How?)\n",
			pMarine->GetMarineProfile()->GetShortName(), m_hLeader->GetMarineProfile()->GetShortName()	);
		return INVALID_SQUADDIE;
	}
}

bool CASW_SquadFormation::Remove( unsigned int slotnum )
{
	Assert(IsValid(slotnum));
	if ( Squaddie(slotnum) )
	{
		m_hSquad[slotnum] = NULL;
		return true;
	}
	else
	{
		return false;
	}
}

bool CASW_SquadFormation::Remove( CASW_Marine *pMarine, bool bIgnoreAssert )
{
	unsigned slot = Find(pMarine);
	if ( IsValid(slot) )
	{
		m_hSquad[slot] = NULL;
		return true;
	}
	else
	{
		AssertMsg1( bIgnoreAssert, "Tried to remove marine %s from squad, but wasn't a member.\n",
			pMarine->GetMarineProfile()->GetShortName() );
		return false;
	}
}

const  Vector CASW_SquadFormation::s_MarineFollowOffset[MAX_SQUAD_SIZE]=
{
	Vector(-60, -70, 0),
	Vector(-120, 0, 0),
	Vector(-60, 70, 0),
	Vector(-100, 50, 0),
	Vector(-100, -50, 0),
	Vector(-20, 50, 0),
	Vector(-20, -50, 0)
};

const  float CASW_SquadFormation::s_MarineFollowDirection[MAX_SQUAD_SIZE]=
{
	-70,
	180,
	70,
	125,
	-125,
	35,
	-35
};

// position offsets when standing around a heal beacon
const  Vector CASW_SquadFormation::s_MarineBeaconOffset[MAX_SQUAD_SIZE]=
{
	Vector(30, -52, 0),
	Vector(-52, 0, 0),
	Vector(30, 52, 0),
	Vector(-30, 40, 0),
	Vector(-30, -40, 0),
	Vector(-20, 40, 0),
	Vector(-20, -40, 0)
};

const  float CASW_SquadFormation::s_MarineBeaconDirection[MAX_SQUAD_SIZE]=
{
	-70,
	180,
	70,
	125,
	-125,
	35,
	-35
};

float CASW_SquadFormation::GetYaw( unsigned slotnum )
{
	if ( m_bStandingInBeacon[slotnum] )
	{
		return s_MarineBeaconDirection[slotnum];
	}
	else if ( m_bLevelHasFollowHints && m_flUseHintsAfter < gpGlobals->curtime && asw_follow_use_hints.GetBool() && Leader() && ( Leader()->IsInCombat() || asw_follow_use_hints.GetInt() == 2 ) )
	{
		Assert( MarineHintManager() );
		if ( m_nMarineHintIndex[ slotnum ] != INVALID_HINT_INDEX && MarineHintManager())
		{
			return MarineHintManager()->GetHintYaw( m_nMarineHintIndex[ slotnum ] );
		}
		else
		{
			return anglemod( m_flCurrentForwardAbsoluteEulerYaw + s_MarineFollowDirection[ slotnum ] );
		}
	}
	// face our formation direction
	return anglemod( m_flCurrentForwardAbsoluteEulerYaw + s_MarineFollowDirection[ slotnum ] );
}

void CASW_SquadFormation::RecomputeFollowerOrder(  const Vector &vProjectedLeaderPos, QAngle qLeaderAim )  ///< reorganize the follower slots so that each follower has the least distance to move
{
	VPROF("CASW_Marine::RecomputeFollowerOrder");

//#pragma message("TODO: this algorithm should be SIMD optimized.")
	// all the possible orderings of three followers ( 3! == 6 )
	const static uint8 sFollowerOrderings[6][MAX_SQUAD_SIZE] =
	{
		{ 0, 1, 2 },
		{ 0, 2, 1 },
		{ 1, 0, 2 },
		{ 1, 2, 0 },
		{ 2, 0, 1 },
		{ 2, 1, 0 }
	};
	// keep track of how well each order scored
	float fOrderingScores[6] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX }; 

	// score each permutation in turn. (This is an experimental dirty algo,
	// and will be SIMDied if we actually go this route)
	qLeaderAim[ PITCH ] = 0;
	matrix3x4_t matLeader;

	Vector vecLeaderAim = GetLdrAnglMatrix( vProjectedLeaderPos, qLeaderAim, &matLeader );
	Vector vProjectedFollowPos[ MAX_SQUAD_SIZE ];
	for ( int i = 0 ; i < MAX_SQUAD_SIZE ; ++i )
	{
		if ( m_bLevelHasFollowHints && m_flUseHintsAfter < gpGlobals->curtime && asw_follow_use_hints.GetBool() && m_hLeader.Get() && ( m_hLeader->IsInCombat() || asw_follow_use_hints.GetInt() == 2 ) )
		{
			if ( m_nMarineHintIndex[i] != INVALID_HINT_INDEX )
			{
				// in combat, follow positions come from nearby hint nodes
				vProjectedFollowPos[i] = MarineHintManager()->GetHintPosition( m_nMarineHintIndex[i] );
			}
			else
			{
				CASW_Marine *pMarine = Squaddie( i );
				if ( pMarine )
				{
					vProjectedFollowPos[i] = pMarine->GetAbsOrigin();
				}
			}
		}
		else
		{
			// when not in combat, use our fixed follow offsets
			VectorTransform( s_MarineFollowOffset[i], matLeader, vProjectedFollowPos[i] );
		}
	}

	for ( int permute = 0 ; permute < 6 ; ++permute )
	{
		// each marine is scored by how far he'd have to move to get to his follow position.
		// movement perpendicular to the leader's aim vector is weighed more heavily than 
		// movement along it. lowest score wins.
		float score = 0;
		for ( int follower = 0 ; follower < MAX_SQUAD_SIZE ; ++follower )
		{
			const int iFollowSlot = sFollowerOrderings[permute][follower];
			const CASW_Marine * RESTRICT pFollower = Squaddie(follower);
			if ( pFollower )
			{
				/*
				Vector vecTransformedOffset;
				
				VectorTransform( s_MarineFollowOffset[iFollowSlot], matLeader, vecTransformedOffset );
				Vector traverse = vecTransformedOffset - pFollower->GetAbsOrigin();
				*/
				Vector traverse = vProjectedFollowPos[iFollowSlot] - pFollower->GetAbsOrigin();
				// score += traverse.Length2D();
				traverse.z = 0;
				score += ( traverse + traverse - traverse.ProjectOnto(vecLeaderAim) ).Length();
			}
		}
		fOrderingScores[ permute ] = score;
	}

	// once done, figure out the best scoring alternative,
	int iBestPermute = 0; 
	for ( int ii = 1 ; ii < 6 ; ++ii )
	{
		iBestPermute = ( fOrderingScores[ ii ] >= fOrderingScores[ iBestPermute ] ) ? iBestPermute : ii;
	}
	// and use it
	if ( iBestPermute != 0 )
	{	// (some change is needed)
		CASW_Marine * RESTRICT pOldFollowers[ MAX_SQUAD_SIZE ] = {0};
		for ( int ii = 0 ; ii < MAX_SQUAD_SIZE ; ++ii ) // copy off the current array as we're about to permute it
		{
			pOldFollowers[ii] = m_hSquad[ii];
		}
		for ( int ii = 0 ; ii < MAX_SQUAD_SIZE ; ++ii )
		{
			m_hSquad[ii] = pOldFollowers[ sFollowerOrderings[iBestPermute][ii] ];
		}
		// Msg( "\n" );

		if ( asw_debug_marine_ai_followspot.GetBool() )
		{
			for ( int ii = 0 ; ii < MAX_SQUAD_SIZE ; ++ii )
			{
				NDebugOverlay::HorzArrow( m_hSquad[ii]->GetAbsOrigin(), m_vFollowPositions[ii], 3, 
					s_SquadDebugColors[ii][0], s_SquadDebugColors[ii][1], s_SquadDebugColors[ii][2], 196, true, 0.35f );
			}
		}
	}
}

Vector CASW_SquadFormation::GetLdrAnglMatrix( const Vector &origin, const QAngle &ang, matrix3x4_t * RESTRICT pout ) RESTRICT
{
	Vector vecLeaderAim;
	{
		// "forward" is actual movement velocity if available, facing otherwise
		float leaderVelSq = m_vLastLeaderVelocity.LengthSqr();
		if ( leaderVelSq > 1.0f )
		{
			vecLeaderAim = m_vLastLeaderVelocity * FastRSqrtFast(leaderVelSq);
			VectorMatrix( vecLeaderAim, *pout );
			pout->SetOrigin( origin );
		}
		else
		{
			AngleMatrix( ang, origin, *pout );
			MatrixGetColumn( *pout, 0, vecLeaderAim );
		}
	}

	m_vCachedForward = vecLeaderAim;
	if (m_vCachedForward[1] == 0 && m_vCachedForward[0] == 0)
	{
		m_flCurrentForwardAbsoluteEulerYaw = 0;
	}
	else
	{
		m_flCurrentForwardAbsoluteEulerYaw = (atan2(vecLeaderAim[1], vecLeaderAim[0]) * (180.0 / M_PI));
		m_flCurrentForwardAbsoluteEulerYaw = 
			fsel( m_flCurrentForwardAbsoluteEulerYaw, m_flCurrentForwardAbsoluteEulerYaw, m_flCurrentForwardAbsoluteEulerYaw + 360 );
	}
	

	return vecLeaderAim;
}

static bool WithinDangerRadius( const Vector &vecPosition )
{
	// marine hull is 26x26; using 15.0f as half hull size plus some padding

	if ( rd_bots_avoid_bombs.GetBool() )
	{
		FOR_EACH_VEC( g_aExplosiveProjectiles, i )
		{
			float flRadius = 240.0f;
			if ( g_aExplosiveProjectiles[i]->Classify() == CLASS_ASW_MORTAR_SHELL )
			{
				flRadius = assert_cast< CASW_Mortarbug_Shell * >( g_aExplosiveProjectiles[i] )->m_DmgRadius;
			}
			else if ( CASW_Grenade_Vindicator *pGrenade = assert_cast< CASW_Grenade_Vindicator * >( g_aExplosiveProjectiles[i] ) )
			{
				// this includes boomer blobs, cluster grenades, and vindicator alt fire
				flRadius = pGrenade->m_DmgRadius;
			}

			float flDistSqr = g_aExplosiveProjectiles[i]->GetAbsOrigin().DistToSqr(vecPosition);
			if ( flDistSqr < Square( flRadius + 15.0f ) )
			{
				return true;
			}
		}
	}

	if ( rd_bots_avoid_gas.GetBool() )
	{
		FOR_EACH_VEC( g_aRadiationVolumes, i )
		{
			CASW_Radiation_Volume *pVolume = g_aRadiationVolumes[i];

			float flDistSqr = pVolume->GetAbsOrigin().DistToSqr( vecPosition );
			// multiply width by sqrt(2) to get radius of square
			if ( flDistSqr < Square( pVolume->m_flBoxWidth * 1.41421356237f + 15.0f ) )
			{
				return true;
			}
		}
	}

	if ( rd_bots_avoid_fire.GetBool() && !FireSystem_IsValidFirePosition( vecPosition, 100.0f ) )
	{
		return true;
	}

	return false;
}

void CASW_SquadFormation::UpdateFollowPositions()
{
	VPROF( "CASW_SquadFormation::UpdateFollowPositions" );
	CASW_Marine *RESTRICT pLeader = Leader();
	if ( !pLeader )
	{
		AssertMsg1( false, "Tried to update positions for a squad with no leader and %d followers.\n",
			Count() );
		return;
	}
	m_flLastSquadUpdateTime = gpGlobals->curtime;

	if ( m_bLevelHasFollowHints && m_flUseHintsAfter < gpGlobals->curtime && asw_follow_use_hints.GetBool() && ( pLeader->IsInCombat() || asw_follow_use_hints.GetInt() ) )
	{
		FindFollowHintNodes();
	}

	QAngle angLeaderFacing = pLeader->EyeAngles();
	angLeaderFacing[PITCH] = 0;
	matrix3x4_t matLeaderFacing;
	Vector vProjectedLeaderPos = pLeader->GetAbsOrigin() + pLeader->GetAbsVelocity() * asw_follow_velocity_predict.GetFloat();
	GetLdrAnglMatrix( vProjectedLeaderPos, angLeaderFacing, &matLeaderFacing );

	for ( int i = 0; i < MAX_SQUAD_SIZE; ++i )
	{
		CASW_Marine *pMarine = Squaddie( i );
		if ( !pMarine )
			continue;

		m_bStandingInBeacon[i] = false;
		CBaseEntity *pBeaconToStandIn = NULL;
		// check for nearby heal beacons			
		if ( IHealGrenadeAutoList::AutoList().Count() > 0 && pMarine->GetHealth() < pMarine->GetMaxHealth() )
		{
			const float flHealGrenadeDetectionRadius = 600.0f;
			for ( int g = 0; g < IHealGrenadeAutoList::AutoList().Count(); ++g )
			{
				const CUtlVector< IHealGrenadeAutoList * > &grenades = IHealGrenadeAutoList::AutoList();
				CBaseEntity *pBeacon = grenades[g]->GetEntity();
				if ( pBeacon && pBeacon->GetAbsOrigin().DistTo( pMarine->GetAbsOrigin() ) < flHealGrenadeDetectionRadius )
				{
					pBeaconToStandIn = pBeacon;
					break;
				}
			}
		}

		if ( pBeaconToStandIn )
		{
			m_vFollowPositions[i] = pBeaconToStandIn->GetAbsOrigin() + s_MarineBeaconOffset[i];
			m_bStandingInBeacon[i] = true;
		}
		else if ( m_bLevelHasFollowHints && m_flUseHintsAfter < gpGlobals->curtime && asw_follow_use_hints.GetBool() && ( pLeader->IsInCombat() || asw_follow_use_hints.GetInt() == 2 ) )
		{
			if ( m_nMarineHintIndex[i] != INVALID_HINT_INDEX )
			{
				m_vFollowPositions[i] = MarineHintManager()->GetHintPosition( m_nMarineHintIndex[i] );
			}
			else
			{

				if ( pMarine )
				{
					m_vFollowPositions[i] = pMarine->GetAbsOrigin();
				}
			}
		}
		else
		{
			VectorTransform( s_MarineFollowOffset[i], matLeaderFacing, m_vFollowPositions[i] );
		}
		if ( asw_debug_marine_ai_followspot.GetBool() )
		{
			NDebugOverlay::HorzArrow( pLeader->GetAbsOrigin(), m_vFollowPositions[i], 3,
				s_SquadDebugColors[i][0], s_SquadDebugColors[i][1], s_SquadDebugColors[i][2], 255, true, 0.35f );
		}
	}

	m_flLastLeaderYaw = pLeader->EyeAngles()[YAW];
	m_vLastLeaderPos = pLeader->GetAbsOrigin();
	m_vLastLeaderVelocity = pLeader->GetAbsVelocity();
}

bool CASW_SquadFormation::SanityCheck() const
{
	CASW_Marine *pLeader = Leader();
	Assert( pLeader );
	if ( !pLeader ) 
		return false;

	// for each slot, make sure no dups, and that leader is not following self
	for ( int testee = 0 ; testee < MAX_SQUAD_SIZE ; ++testee )
	{
		CASW_Marine *pTest = m_hSquad[testee];
		Assert( pLeader != pTest );  // am not leader
		if ( pLeader == pTest ) 
			return false;

		if ( pTest )
		{
			for ( int i = (testee+1)%MAX_SQUAD_SIZE ; i != testee ; i = (i + 1)%MAX_SQUAD_SIZE )
			{
				Assert( m_hSquad[i] != pTest ); // am not in array twice
				if ( m_hSquad[i] == pTest ) 
					return false;
			}
		}
	}

	return true;
}

// For such a tiny array, a linear search is actually the fastest
// way to go
unsigned int CASW_SquadFormation::Find( CASW_Marine *pMarine ) const
{
	//Assert( pMarine != Leader() );
	for ( int i = 0 ; i < MAX_SQUAD_SIZE ; ++i )
	{
		if ( m_hSquad[i] == pMarine )
			return i;
	}
	return INVALID_SQUADDIE;
}

void CASW_SquadFormation::ChangeLeader( CASW_Marine *pNewLeader, bool bUpdateLeaderPos )
{
	// if the squad has no leader, become the leader
	CASW_Marine *pOldLeader = Leader();
	if ( !pOldLeader )
	{
		Leader( pNewLeader );
		return;
	}

	// if we're trying to wipe out the leader, do so if there are no followers
	if ( !pNewLeader )
	{
		//AssertMsg2( Count() == 0, "Tried to unset squad leader %s, but squad has %d followers\n", pNewLeader->GetMarineProfile()->GetShortName(), Count() );
		Leader(NULL);
		return;
	}

	if ( pOldLeader == pNewLeader )
	{
		//AssertMsg1( false, "Tried to reset squad leader to its current value (%s)\n", pNewLeader->GetMarineProfile()->GetShortName() );
		return;
	}

	// if the new leader was previously a follower, swap with the old leader
	int slot = Find( pNewLeader );
	if ( IsValid(slot) )
	{
		m_hSquad[slot] = pOldLeader;
		Leader( pNewLeader );
	}
	else
	{
		// make the old leader a follower 
		Leader( pNewLeader );
		Add( pOldLeader );
	}
	if ( bUpdateLeaderPos )
	{
		m_flLastLeaderYaw = pNewLeader->EyeAngles()[ YAW ];
		m_vLastLeaderPos = pNewLeader->GetAbsOrigin();
		m_vLastLeaderVelocity = pNewLeader->GetAbsVelocity();
		m_flLastSquadUpdateTime = gpGlobals->curtime;
	}
	else
	{
		m_flLastSquadUpdateTime = 0;
	}
}

void CASW_SquadFormation::LevelInitPreEntity()
{
	Reset();
}

void CASW_SquadFormation::Reset()
{
	m_hLeader = NULL;
	for ( int i = 0 ; i < MAX_SQUAD_SIZE ; ++i )
	{
		m_hSquad[i] = NULL;
		m_bRearGuard[i] = false;
		m_bStandingInBeacon[i] = false;
		m_nMarineHintIndex[i] = INVALID_HINT_INDEX;
	}
	m_flUseHintsAfter = -1;
	m_flLastSquadUpdateTime = 0;
	m_bLevelHasFollowHints = false;
	m_vLastLeaderVelocity.Zero();
	m_flLastDangerTime = 0;
}

void CASW_SquadFormation::FollowCommandUsed()
{
	m_flUseHintsAfter = gpGlobals->curtime + rd_follow_hint_delay.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Sorts AI nodes by proximity to leader
//-----------------------------------------------------------------------------
CASW_Marine *g_pSortLeader = NULL;
int CASW_SquadFormation::FollowHintSortFunc( HintData_t* const *pHint1, HintData_t* const *pHint2 )
{
	int nDist1 = (int) (*pHint1)->GetAbsOrigin().DistToSqr( g_pSortLeader->GetAbsOrigin() );
	int nDist2 = (int) (*pHint2)->GetAbsOrigin().DistToSqr( g_pSortLeader->GetAbsOrigin() );

	return ( nDist1 - nDist2 );
}

float GetWaypointDistToEnd( const Vector &vStartPos, AI_Waypoint_t *way );

//-----------------------------------------------------------------------------
// Purpose: Finds the set of hint nodes to use when following during combat
//-----------------------------------------------------------------------------
void CASW_SquadFormation::FindFollowHintNodes()
{
	CASW_Marine *pLeader = Leader();
	if ( !pLeader )
		return;

	// evaluate each squaddie individually to see if his node should be updated
	for ( int slotnum = 0; slotnum < MAX_SQUAD_SIZE; slotnum++ )
	{
		CASW_Marine *pMarine = Squaddie( slotnum );
		if ( !pMarine )
		{
			m_nMarineHintIndex[slotnum] = INVALID_HINT_INDEX;
			continue;
		}

		// reactivedrop: increase the hint max range if we have more then 3 bots
		// this fixes the issue where bots get stuck and don't follow leader
		// because they can't find a hint to stand on
		float fHintRangeFactor = Count() > 3 ? float( Count() ) / 3.0f : 1.0f;
		//DevMsg("fHintRangeFactor=%f, hint_max_range=%f, hint_search_range=%f\n", fHintRangeFactor, asw_follow_hint_max_range.GetFloat() * fHintRangeFactor, rd_follow_hint_max_search_range.GetFloat() * fHintRangeFactor);
		bool bNeedNewNode = ( pMarine->GetAbsOrigin().DistTo( pLeader->GetAbsOrigin() ) > asw_follow_hint_max_range.GetFloat() * fHintRangeFactor );
		bNeedNewNode = bNeedNewNode || !pMarine->FVisible( pLeader );
		bNeedNewNode = bNeedNewNode || WithinDangerRadius( pMarine->GetAbsOrigin() );

		// reactivedrop: if leader is in escape area then bots will follow him
		// immediately. This fixes a bug when player is at the map finish and 
		// bots don't follow him
		CBaseTrigger *pEscapeVolume = pLeader->IsInEscapeVolume();
		if ( !bNeedNewNode && pEscapeVolume )
		{
			if ( !pMarine->IsInEscapeVolume() )
				bNeedNewNode = true;
		}

		CBaseTrigger *pStickTogetherVolume = pLeader->IsInStickTogetherVolume();
		if ( !bNeedNewNode && pStickTogetherVolume )
		{
			if ( !pMarine->IsInStickTogetherVolume() )
				bNeedNewNode = true;
		}
		if ( !pEscapeVolume && pStickTogetherVolume )
		{
			// a hack, just a hack 
			pEscapeVolume = pStickTogetherVolume;
		}

		if ( !bNeedNewNode && m_bLevelHasFollowHints && m_nMarineHintIndex[slotnum] != INVALID_HINT_INDEX && ( MarineHintManager()->GetHintFlags( m_nMarineHintIndex[slotnum] ) & HintData_t::HINT_DELETED ) )
			bNeedNewNode = true;

		// find shield bug (if any) nearest each marine
		const float flShieldbugScanRangeSqr = Square( rd_follow_hint_max_search_range_danger.GetFloat() );
		CASW_Shieldbug *pClosestShieldbug = NULL;
		float flClosestShieldBugDistSqr = flShieldbugScanRangeSqr;

		if ( rd_bots_flank_shieldbug.GetBool() && pMarine->IsAlive() )
		{
			for ( int iShieldbug = 0; iShieldbug < IShieldbugAutoList::AutoList().Count(); iShieldbug++ )
			{
				CASW_Shieldbug *pShieldbug = static_cast< CASW_Shieldbug * >( IShieldbugAutoList::AutoList()[iShieldbug] );
				if ( pShieldbug && pShieldbug->IsAlive() )
				{
					float flDistSqr = pMarine->GetAbsOrigin().DistToSqr( pShieldbug->GetAbsOrigin() );
					if ( flDistSqr < flClosestShieldBugDistSqr )
					{
						flClosestShieldBugDistSqr = flDistSqr;
						pClosestShieldbug = pShieldbug;
					}
				}
			}
		}

		if ( !bNeedNewNode && !pClosestShieldbug )
			continue;

		// find a new node
		CUtlVector< HintData_t * > hints;
		// reactivedrop: for pEscapeVolume find only hints inside pEscapeVolume
		if ( pEscapeVolume )
		{
			MarineHintManager()->FindHints( *pEscapeVolume, &hints );
		}
		else
		{
			MarineHintManager()->FindHints( pLeader->GetAbsOrigin(), 80.0f, ( m_flLastDangerTime > gpGlobals->curtime ? rd_follow_hint_max_search_range_danger.GetFloat() : rd_follow_hint_max_search_range.GetFloat() ) * fHintRangeFactor, &hints );
		}

		int nCount = hints.Count();

		float flMovementYaw = pLeader->GetOverallMovementDirection();

		int iClosestFlankingNode = INVALID_HINT_INDEX;
		float flClosestFlankingNodeDistSqr = FLT_MAX;

		if ( pEscapeVolume && pEscapeVolume->CollisionProp() )
		{
			// remove hints that aren't in the escape volume bounds
			for ( int i = nCount - 1; i >= 0; i-- )
			{
				if ( !pEscapeVolume->CollisionProp()->IsPointInBounds( hints[i]->GetAbsOrigin() ) )
				{
					if ( asw_debug_marine_hints.GetBool() )
					{
						NDebugOverlay::Box( hints[i]->GetAbsOrigin(), -Vector( 5, 5, 0 ), Vector( 5, 5, 5 ), 255, 0, 0, 64, 0.35f );
					}
					hints.Remove( i );
					nCount--;
				}
				else if ( asw_debug_marine_hints.GetBool() )
				{
					NDebugOverlay::Box( hints[i]->GetAbsOrigin(), -Vector( 5, 5, 0 ), Vector( 5, 5, 5 ), 0, 255, 0, 64, 0.35f );
				}
			}
		}
		else
		{
			// remove hints that are in front of the leader's overall direction of movement
			// TODO: turn this into a hint filter
			for ( int i = nCount - 1; i >= 0; i-- )
			{
				Vector vecDir = ( hints[i]->GetAbsOrigin() - pLeader->GetAbsOrigin() ).Normalized();
				float flYaw = UTIL_VecToYaw( vecDir );
				flYaw = AngleDiff( flYaw, flMovementYaw );
				bool bRemoveNode = false;

				if ( flYaw < 85.0f && flYaw > -85.0f )
				{
					bRemoveNode = true;

					// remove hints that are in front of the leader's overall direction of movement,
					// unless we need to use them to get the AI to flank a shieldbug
					if ( pClosestShieldbug )
					{
						// if any of the marines are close, don't delete nodes behind the shieldbug
						float flShieldbugDistSqr = hints[i]->GetAbsOrigin().DistToSqr( pClosestShieldbug->GetAbsOrigin() );
						if ( flShieldbugDistSqr < flShieldbugScanRangeSqr )
						{
							// preserve the node if it's behind the shieldbug
							Vector vecShieldBugToNode, vecShieldbugFacing;

							vecShieldBugToNode = hints[i]->GetAbsOrigin() - pClosestShieldbug->GetAbsOrigin();
							QAngle angFacing = pClosestShieldbug->GetAbsAngles();
							AngleVectors( angFacing, &vecShieldbugFacing );
							vecShieldbugFacing.z = 0;
							vecShieldBugToNode.z = 0;

							VectorNormalize( vecShieldbugFacing );
							VectorNormalize( vecShieldBugToNode );

							float flForwardDot = vecShieldbugFacing.Dot( vecShieldBugToNode );
							if ( flForwardDot < 0.5f )	// if node is 60 degrees or more away from shieldbug's facing...
							{
								float flDistSqr = hints[i]->GetAbsOrigin().DistToSqr( pMarine->GetAbsOrigin() );
								bool bHasLOS = pMarine->TestShootPosition( pMarine->GetAbsOrigin(), hints[i]->GetAbsOrigin() );

								// if closer than the previous closest node, and the current node isn't taken, reserve it
								if ( flDistSqr < flClosestFlankingNodeDistSqr && bHasLOS )
								{
									bool flNodeTaken = false;
									for ( int iSlot = 0; iSlot < MAX_SQUAD_SIZE; iSlot++ )
									{
										if ( iSlot != slotnum && hints[i]->m_nHintIndex == m_nMarineHintIndex[iSlot] )
										{
											flNodeTaken = true;
											break;
										}
									}

									if ( !flNodeTaken )
									{
										iClosestFlankingNode = hints[i]->m_nHintIndex;
										flClosestFlankingNodeDistSqr = flDistSqr;
										bRemoveNode = false;
									}
								}
							}
						}
					}
				}

				// remove unsafe hint locations
				if ( WithinDangerRadius( hints[i]->GetAbsOrigin() ) )
				{
					bRemoveNode = true;
				}
				else if ( bRemoveNode && m_flLastDangerTime > gpGlobals->curtime && !pClosestShieldbug )
				{
					// if we're not flanking a shieldbug and we're in danger mode, allow running in front of the leader.
					// this is useful because when danger happens, players usually back away from it, moving our cone of
					// acceptable nodes into the area we need to get out of.
					bRemoveNode = false;
					if ( asw_debug_marine_hints.GetBool() )
					{
						NDebugOverlay::Box( hints[i]->GetAbsOrigin(), -Vector( 5, 5, 0 ), Vector( 5, 5, 5 ), 255, 255, 0, 64, 0.35f );
					}
				}

				// if zdiff is too great, remove
				float flZDiff = fabs( hints[i]->GetAbsOrigin().z - pLeader->GetAbsOrigin().z );
				if ( flZDiff > asw_follow_hint_max_z_dist.GetFloat() )
				{
					bRemoveNode = true;
				}

				if ( bRemoveNode )
				{
					if ( asw_debug_marine_hints.GetBool() )
					{
						NDebugOverlay::Box( hints[i]->GetAbsOrigin(), -Vector( 5, 5, 0 ), Vector( 5, 5, 5 ), 255, 0, 0, 64, 0.35f );
					}
					hints.Remove( i );
					nCount--;
				}
			}
		}

		g_pSortLeader = pLeader;
		hints.Sort( CASW_SquadFormation::FollowHintSortFunc );

		if ( rd_follow_hint_pathfind.GetFloat() != 0 )
		{
			FOR_EACH_VEC_BACK( hints, i )
			{
				AI_Waypoint_t *pRoute = pMarine->GetPathfinder()->BuildRoute( pLeader->GetAbsOrigin(), hints[i]->GetAbsOrigin(),
					NULL, 100, NAV_GROUND );
				if ( pRoute )
				{
					float flLength = GetWaypointDistToEnd( pLeader->GetAbsOrigin(), pRoute );
					bool bShortEnough = flLength < ( m_flLastDangerTime > gpGlobals->curtime ? rd_follow_hint_max_search_range_danger.GetFloat() : rd_follow_hint_max_search_range.GetFloat() ) + rd_follow_hint_max_path_bloat.GetFloat();

					if ( !bShortEnough && asw_debug_marine_hints.GetBool() )
						ASWPathUtils()->DebugDrawRoute( pLeader->GetAbsOrigin(), pRoute );

					ASWPathUtils()->DeleteRoute( pRoute );

					if ( bShortEnough )
						continue;
				}

				if ( asw_debug_marine_hints.GetBool() )
				{
					NDebugOverlay::Box( hints[i]->GetAbsOrigin(), -Vector( 5, 5, 0 ), Vector( 5, 5, 5 ), 255, 127, 127, 64, 0.35f );
				}

				hints[i]->m_flIgnoreUntil = gpGlobals->curtime + rd_follow_hint_pathfind.GetFloat();
				hints.Remove( i );
				nCount--;
			}
		}

		// if this marine is close to a shield bug, grab a flanking node
		if ( !pEscapeVolume && pClosestShieldbug && iClosestFlankingNode != INVALID_HINT_INDEX )
		{
			m_nMarineHintIndex[slotnum] = iClosestFlankingNode;
			continue;
		}

		// find the first node not used by another other squaddie
		int nNode = 0;
		bool bValidNodeFound = false;
		while ( nNode < hints.Count() )
		{
			bool bValidNode = true;
			for ( int k = 0; k < MAX_SQUAD_SIZE; k++ )
			{
				if ( k == slotnum )
					continue;
				if ( hints[nNode]->m_nHintIndex == m_nMarineHintIndex[k] )
				{
					bValidNode = false;
					break;
				}
			}
			if ( bValidNode )
			{
				m_nMarineHintIndex[slotnum] = hints[nNode]->m_nHintIndex;
				nNode++;
				bValidNodeFound = true;
				break;
			}
			nNode++;
		}

		// a workaround to handle escape volumes with not enough hint nodes
		// use the first hint in escape volume 
		if ( pEscapeVolume && !bValidNodeFound && hints.Count() > 0 )
		{
			DevMsg( "Failed to find free node from %i available using first one\n", hints.Count() );
			m_nMarineHintIndex[slotnum] = hints[0]->m_nHintIndex;
		}
	}
}

template<typename E1, typename E2 = CBaseEntity>
bool AnyInDangerRange( const CUtlVector<E1 *> &entities, const Vector &vecOrigin, float flDist )
{
	float flDistSqr = Square( flDist );
	FOR_EACH_VEC( entities, i )
	{
		if ( assert_cast< E2 * >( entities[i] )->GetAbsOrigin().DistToSqr( vecOrigin ) < flDistSqr )
		{
			return true;
		}
	}

	return false;
}

bool CASW_SquadFormation::ShouldUpdateFollowPositions()
{
	// update if a heal beacon was placed/removed
	if ( m_iLastHealBeaconCount != IHealGrenadeAutoList::AutoList().Count() )
		return true;

	if ( !Leader() )
		return false;

	if ( m_flLastDangerTime > gpGlobals->curtime )
		return true;

	Vector vecLeaderPosition = Leader()->GetAbsOrigin();
	Vector vecLeaderVelocity = Leader()->GetAbsVelocity();

	float flDangerRadius = rd_follow_hint_max_search_range_danger.GetFloat() * MAX( Count(), 3 ) / 3.0f;

	bool bShieldbugHazard = rd_bots_flank_shieldbug.GetBool() && AnyInDangerRange<IShieldbugAutoList, CASW_Shieldbug>( IShieldbugAutoList::AutoList(), vecLeaderPosition, flDangerRadius );
	bool bBombHazard = rd_bots_avoid_bombs.GetBool() && AnyInDangerRange( g_aExplosiveProjectiles, vecLeaderPosition, flDangerRadius );
	bool bGasHazard = rd_bots_avoid_gas.GetBool() && AnyInDangerRange( g_aRadiationVolumes, vecLeaderPosition, flDangerRadius );
	bool bFireHazard = rd_bots_avoid_fire.GetBool() && !FireSystem_IsValidFirePosition( vecLeaderPosition, flDangerRadius );
	if ( bShieldbugHazard || bBombHazard || bGasHazard || bFireHazard )
	{
		m_flLastDangerTime = gpGlobals->curtime + 4.0f;
		return true;
	}

	// leader's more than epsilon from previous position, 
	// and we haven't updated in a quarter second.
	// force a reupdate if leader's velocity has changed by 50% or more or boomer bombs are deployed
	return ( gpGlobals->curtime > m_flLastSquadUpdateTime + 0.33f ||
		( vecLeaderVelocity - m_vLastLeaderVelocity ).LengthSqr() * FastRecip( m_vLastLeaderVelocity.LengthSqr() ) > ( 0.5f * 0.5f ) ||
		( vecLeaderVelocity.IsZeroFast() && !m_vLastLeaderVelocity.IsZeroFast() ) ) &&
		( vecLeaderPosition.DistToSqr( m_vLastLeaderPos ) > Square( asw_follow_threshold.GetFloat() ) );
}

void CASW_SquadFormation::DrawDebugGeometryOverlays()
{
	if ( !asw_debug_squad_movement.GetBool() )
		return;

	CASW_Marine *pLeader = Leader();
	if ( !pLeader )
		return;

	for ( int i = 0; i < MAX_SQUAD_SIZE; i++ )
	{
		CASW_Marine *pMarine = Squaddie( i );
		if ( pMarine )
		{
			NDebugOverlay::Line( pMarine->WorldSpaceCenter(), pLeader->WorldSpaceCenter(), 63, 63, 63, false, 0.35f );
		}
	}
}


CASW_SquadFormation g_ASWSquadFormation;
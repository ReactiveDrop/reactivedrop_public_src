//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "c_physicsprop.h"
#include "c_physbox.h"
#include "c_props.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"

#define CPhysBox C_PhysBox
#define CPhysicsProp C_PhysicsProp


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( DynamicProp, DT_DynamicProp )

BEGIN_NETWORK_TABLE( CDynamicProp, DT_DynamicProp )
	RecvPropBool( RECVINFO(m_bUseHitboxesForRenderBox) ),
	RecvPropFloat( RECVINFO(m_flGlowMaxDist) ),
	RecvPropBool( RECVINFO(m_bShouldGlow) ),
	RecvPropInt( RECVINFO(m_clrGlow), 0, RecvProxy_Int32ToColor32 ),
END_NETWORK_TABLE()

C_DynamicProp::C_DynamicProp( void ) : 
m_GlowObject( this, Vector( 1.0f, 1.0f, 1.0f ), 0.0f, false, false )
{
	m_iCachedFrameCount = -1;
}

C_DynamicProp::~C_DynamicProp( void )
{
}

bool C_DynamicProp::TestBoneFollowers( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr )
{
	// UNDONE: There is no list of the bone followers that is networked to the client
	// so instead we do a search for solid stuff here.  This is not really great - a list would be
	// preferable.
	CBaseEntity	*pList[128];
	Vector mins, maxs;
	CollisionProp()->WorldSpaceAABB( &mins, &maxs );
	int count = UTIL_EntitiesInBox( pList, ARRAYSIZE(pList), mins, maxs, 0, PARTITION_CLIENT_SOLID_EDICTS );
	for ( int i = 0; i < count; i++ )
	{
		if ( pList[i]->GetOwnerEntity() == this )
		{
			if ( pList[i]->TestCollision(ray, fContentsMask, tr) )
			{
				return true;
			}
		}
	}
	return false;
}

bool C_DynamicProp::TestCollision( const Ray_t &ray, unsigned int fContentsMask, trace_t& tr )
{
	if ( IsSolidFlagSet(FSOLID_NOT_SOLID) )
	{
		// if this entity is marked non-solid and custom test it must have bone followers
		if ( IsSolidFlagSet( FSOLID_CUSTOMBOXTEST ) && IsSolidFlagSet( FSOLID_CUSTOMRAYTEST ))
		{
			return TestBoneFollowers( ray, fContentsMask, tr );
		}
	}
	return BaseClass::TestCollision( ray, fContentsMask, tr );
}

void C_DynamicProp::ClientThink( void )
{
	BaseClass::ClientThink();

	UpdateGlow();
}

//-----------------------------------------------------------------------------
// implements these so ragdolls can handle frustum culling & leaf visibility
//-----------------------------------------------------------------------------
void C_DynamicProp::GetRenderBounds( Vector& theMins, Vector& theMaxs )
{
	if ( m_bUseHitboxesForRenderBox )
	{
		if ( GetModel() )
		{
			studiohdr_t *pStudioHdr = modelinfo->GetStudiomodel( GetModel() );
			if ( !pStudioHdr || GetSequence() == -1 )
			{
				theMins = vec3_origin;
				theMaxs = vec3_origin;
				return;
			}

			// Only recompute if it's a new frame
			if ( gpGlobals->framecount != m_iCachedFrameCount )
			{
				ComputeEntitySpaceHitboxSurroundingBox( &m_vecCachedRenderMins, &m_vecCachedRenderMaxs );
				m_iCachedFrameCount = gpGlobals->framecount;
			}

			theMins = m_vecCachedRenderMins;
			theMaxs = m_vecCachedRenderMaxs;
			return;
		}
	}

	BaseClass::GetRenderBounds( theMins, theMaxs );
}

unsigned int C_DynamicProp::ComputeClientSideAnimationFlags()
{
	if ( GetSequence() != -1 )
	{
		CStudioHdr *pStudioHdr = GetModelPtr();
		if ( GetSequenceCycleRate(pStudioHdr, GetSequence()) != 0.0f )
		{
			return BaseClass::ComputeClientSideAnimationFlags();
		}
	}

	// no sequence or no cycle rate, don't do any per-frame calcs
	return 0;
}

void C_DynamicProp::UpdateGlow( void )
{
	if ( m_bShouldGlow == false )
	{
		if ( m_GlowObject.IsRendering() )
		{
			m_GlowObject.SetRenderFlags( false, false );
			m_GlowObject.SetAlpha( 0.0f );
		}
		return;
	}

	Vector glowColor;
	glowColor.x = ( m_clrGlow.r/255.0f );
	glowColor.y = ( m_clrGlow.g/255.0f );
	glowColor.z = ( m_clrGlow.b/255.0f );

	float flAlpha = 0.9f;

	// fade the alpha based on distace
	C_ASW_Player *pLocalPlayer = C_ASW_Player::GetLocalASWPlayer();
	if ( pLocalPlayer && pLocalPlayer->GetViewNPC() && m_bShouldGlow )
	{
		float flDistanceToMarine = 0.0f;
		flDistanceToMarine = ( pLocalPlayer->GetViewNPC()->GetAbsOrigin() - WorldSpaceCenter() ).Length();
		flAlpha = clamp( 1.0 - ( flDistanceToMarine / m_flGlowMaxDist ), 0.0, 0.9 );
	}

	// Start glowing
	m_GlowObject.SetRenderFlags( m_bShouldGlow, false );
	m_GlowObject.SetColor( glowColor );
	m_GlowObject.SetAlpha( m_bShouldGlow ? flAlpha : 0.0f );

	SetNextClientThink( gpGlobals->curtime + 0.1f );
}

void C_DynamicProp::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	UpdateGlow();
}

// ------------------------------------------------------------------------------------------ //
// ------------------------------------------------------------------------------------------ //
IMPLEMENT_CLIENTCLASS_DT(C_BasePropDoor, DT_BasePropDoor, CBasePropDoor)
END_RECV_TABLE()

C_BasePropDoor::C_BasePropDoor( void )
{
	m_modelChanged = false;
}

C_BasePropDoor::~C_BasePropDoor( void )
{
}

void C_BasePropDoor::PostDataUpdate( DataUpdateType_t updateType )
{
	if ( updateType == DATA_UPDATE_CREATED )
	{
		BaseClass::PostDataUpdate( updateType );
	}
	else
	{
		const model_t *oldModel = GetModel();
		BaseClass::PostDataUpdate( updateType );
		const model_t *newModel = GetModel();

		if ( oldModel != newModel )
		{
			m_modelChanged = true;
		}
	}
}

void C_BasePropDoor::OnDataChanged( DataUpdateType_t type )
{
	BaseClass::OnDataChanged( type );

	bool bCreate = (type == DATA_UPDATE_CREATED) ? true : false;
	if ( VPhysicsGetObject() && m_modelChanged )
	{
		VPhysicsDestroyObject();
		m_modelChanged = false;
		bCreate = true;
	}
	VPhysicsShadowDataChanged(bCreate, this);
}

bool C_BasePropDoor::TestCollision( const Ray_t &ray, unsigned int mask, trace_t& trace )
{
	if ( !VPhysicsGetObject() )
		return false;

	MDLCACHE_CRITICAL_SECTION();
	CStudioHdr *pStudioHdr = GetModelPtr( );
	if (!pStudioHdr)
		return false;

	physcollision->TraceBox( ray, VPhysicsGetObject()->GetCollide(), GetAbsOrigin(), GetAbsAngles(), &trace );

	if ( trace.DidHit() )
	{
		trace.contents = pStudioHdr->contents();
		// use the default surface properties
		trace.surface.name = "**studio**";
		trace.surface.flags = 0;
		trace.surface.surfaceProps = pStudioHdr->GetSurfaceProp();

		return true;
	}

	return false;
}

//just need to reference by classname in portal
class C_PropDoorRotating : public C_BasePropDoor
{
public:
	DECLARE_CLASS( C_PropDoorRotating, C_BasePropDoor );
	DECLARE_CLIENTCLASS();
};

IMPLEMENT_CLIENTCLASS_DT(C_PropDoorRotating, DT_PropDoorRotating, CPropDoorRotating)
END_RECV_TABLE()

// ------------------------------------------------------------------------------------------ //
// Special version of func_physbox.
// ------------------------------------------------------------------------------------------ //
#ifndef _XBOX
class CPhysBoxMultiplayer : public CPhysBox, public IMultiplayerPhysics
{
public:
	DECLARE_CLASS( CPhysBoxMultiplayer, CPhysBox );

	virtual int GetMultiplayerPhysicsMode()
	{
		return m_iPhysicsMode;
	}

	virtual float GetMass()
	{
		return m_fMass;
	}

	virtual bool IsAsleep()
	{
		Assert ( 0 );
		return true;
	}

	CNetworkVar( int, m_iPhysicsMode );	// One of the PHYSICS_MULTIPLAYER_ defines.	
	CNetworkVar( float, m_fMass );

	DECLARE_CLIENTCLASS();
};

IMPLEMENT_CLIENTCLASS_DT( CPhysBoxMultiplayer, DT_PhysBoxMultiplayer, CPhysBoxMultiplayer )
	RecvPropInt( RECVINFO( m_iPhysicsMode ) ),
	RecvPropFloat( RECVINFO( m_fMass ) ),
END_RECV_TABLE()


class CPhysicsPropMultiplayer : public CPhysicsProp, public IMultiplayerPhysics
{
	DECLARE_CLASS( CPhysicsPropMultiplayer, CPhysicsProp );

	virtual int GetMultiplayerPhysicsMode()
	{
		Assert( m_iPhysicsMode != PHYSICS_MULTIPLAYER_CLIENTSIDE );
		Assert( m_iPhysicsMode != PHYSICS_MULTIPLAYER_AUTODETECT );
		return m_iPhysicsMode;
	}

	virtual float GetMass()
	{
		return m_fMass;
	}

	virtual bool IsAsleep()
	{
		return !m_bAwake;
	}

	virtual void ComputeWorldSpaceSurroundingBox( Vector *mins, Vector *maxs )
	{
		Assert( mins != NULL && maxs != NULL );
		if ( !mins || !maxs )
			return;

		// Take our saved collision bounds, and transform into world space
		TransformAABB( EntityToWorldTransform(), m_collisionMins, m_collisionMaxs, *mins, *maxs );
	}

	CNetworkVar( int, m_iPhysicsMode );	// One of the PHYSICS_MULTIPLAYER_ defines.	
	CNetworkVar( float, m_fMass );
	CNetworkVector( m_collisionMins );
	CNetworkVector( m_collisionMaxs );

	DECLARE_CLIENTCLASS();
};

IMPLEMENT_CLIENTCLASS_DT( CPhysicsPropMultiplayer, DT_PhysicsPropMultiplayer, CPhysicsPropMultiplayer )
	RecvPropInt( RECVINFO( m_iPhysicsMode ) ),
	RecvPropFloat( RECVINFO( m_fMass ) ),
	RecvPropVector( RECVINFO( m_collisionMins ) ),
	RecvPropVector( RECVINFO( m_collisionMaxs ) ),
END_RECV_TABLE()
#endif

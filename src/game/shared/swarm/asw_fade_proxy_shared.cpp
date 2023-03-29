#include "cbase.h"
#include "asw_fade_proxy_shared.h"
#ifdef CLIENT_DLL
#include "c_asw_inhabitable_npc.h"
#else
#include "func_asw_fade.h"
#include "prop_asw_fade.h"
#include "asw_inhabitable_npc.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_NETWORKCLASS_ALIASED( Point_ASW_Fade_Proxy, DT_Point_ASW_Fade_Proxy );
IMPLEMENT_NETWORKCLASS_ALIASED( Trigger_ASW_Fade_Proxy, DT_Trigger_ASW_Fade_Proxy );

BEGIN_NETWORK_TABLE( CPoint_ASW_Fade_Proxy, DT_Point_ASW_Fade_Proxy )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bDisabled ) ),
	RecvPropBool( RECVINFO( m_bBrushOnly ) ),
	RecvPropFloat( RECVINFO( m_flFreeRadius ) ),
	RecvPropFloat( RECVINFO( m_flMaxRadius ) ),
#else
	SendPropBool( SENDINFO( m_bDisabled ) ),
	SendPropBool( SENDINFO( m_bBrushOnly ) ),
	SendPropFloat( SENDINFO( m_flFreeRadius ) ),
	SendPropFloat( SENDINFO( m_flMaxRadius ) ),
#endif
END_NETWORK_TABLE();

BEGIN_NETWORK_TABLE( CTrigger_ASW_Fade_Proxy, DT_Trigger_ASW_Fade_Proxy )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_nModelIndex ) ),
	RecvPropDataTable( RECVINFO_DT( m_Collision ), 0, &REFERENCE_RECV_TABLE( DT_CollisionProperty ) ),
#else
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropDataTable( SENDINFO_DT( m_Collision ), &REFERENCE_SEND_TABLE( DT_CollisionProperty ) ),
#endif
END_NETWORK_TABLE();

#ifndef CLIENT_DLL
LINK_ENTITY_TO_CLASS( point_asw_fade_proxy, CPoint_ASW_Fade_Proxy );
LINK_ENTITY_TO_CLASS( trigger_asw_fade_proxy, CTrigger_ASW_Fade_Proxy );

BEGIN_DATADESC( CPoint_ASW_Fade_Proxy )
	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "StartDisabled" ),
	DEFINE_KEYFIELD( m_bBrushOnly, FIELD_BOOLEAN, "BrushOnly" ),
	DEFINE_KEYFIELD( m_flFreeRadius, FIELD_FLOAT, "FreeRadius" ),
	DEFINE_KEYFIELD( m_flMaxRadius, FIELD_FLOAT, "MaxRadius" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
END_DATADESC();

BEGIN_DATADESC( CTrigger_ASW_Fade_Proxy )
END_DATADESC();
#endif

CPoint_ASW_Fade_Proxy::CPoint_ASW_Fade_Proxy()
{
	m_bDisabled = false;
	m_bBrushOnly = false;
	m_flFreeRadius = 0;
	m_flMaxRadius = MAX_COORD_RANGE;
}

#ifdef CLIENT_DLL
extern ConVar asw_debug_fade;
#else
void CPoint_ASW_Fade_Proxy::Activate()
{
	BaseClass::Activate();

	CFunc_ASW_Fade *pBrush = dynamic_cast< CFunc_ASW_Fade * >( GetMoveParent() );
	CProp_ASW_Fade *pProp = dynamic_cast< CProp_ASW_Fade * >( GetMoveParent() );
	Assert( pBrush || pProp );

	if ( !pBrush && !pProp )
	{
		Warning( "%s at %f %f %f does not have a func_asw_fade or prop_asw_fade parent. Deleting.\n", GetClassname(), VectorExpand( GetAbsOrigin() ) );
		UTIL_Remove( this );

		return;
	}

	SetOwnerEntity( GetMoveParent() );

	if ( pBrush )
	{
		pBrush->m_bHasProxies = true;
	}
	else if ( pProp )
	{
		pProp->m_bHasProxies = true;
	}
}

int	CPoint_ASW_Fade_Proxy::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

int CPoint_ASW_Fade_Proxy::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

void CPoint_ASW_Fade_Proxy::InputEnable( inputdata_t &data )
{
	m_bDisabled = false;
}

void CPoint_ASW_Fade_Proxy::InputDisable( inputdata_t &data )
{
	m_bDisabled = true;
}

void CPoint_ASW_Fade_Proxy::InputToggle( inputdata_t &data )
{
	m_bDisabled = !m_bDisabled;
}
#endif

class CTraceFilterFadeProxy : public ITraceFilter
{
public:
	CTraceFilterFadeProxy( bool bBrushOnly )
	{
		m_bBrushOnly = bBrushOnly;
	}

	bool ShouldHitEntity( IHandleEntity *pEntity, int contentsMask ) override
	{
		CBaseEntity *pBaseEntity = EntityFromEntityHandle( pEntity );
		if ( !pBaseEntity )
		{
			Assert( m_bBrushOnly && staticpropmgr->IsStaticProp( pEntity ) );
			return !m_bBrushOnly;
		}

		if ( m_bBrushOnly && !pBaseEntity->IsBSPModel() )
		{
			return false;
		}

		return pBaseEntity->GetMoveType() == MOVETYPE_NONE || pBaseEntity->GetMoveType() == MOVETYPE_PUSH;
	}

	TraceType_t	GetTraceType() const override
	{
		return m_bBrushOnly ? TRACE_EVERYTHING_FILTER_PROPS : TRACE_EVERYTHING;
	}

	bool m_bBrushOnly;
};

bool CPoint_ASW_Fade_Proxy::ShouldFade( const Vector & vecEyePosition )
{
	if ( m_bDisabled )
	{
		return false;
	}

	float flDistSqr = vecEyePosition.DistToSqr( GetAbsOrigin() );

#ifdef CLIENT_DLL
	if ( asw_debug_fade.GetBool() )
	{
		NDebugOverlay::Sphere( GetAbsOrigin(), m_flMaxRadius, flDistSqr > Square( m_flMaxRadius.Get() ) ? 255 : 0, 255, 0, false, 0.01f );
		if ( m_flFreeRadius > 0 && m_flFreeRadius < m_flMaxRadius )
		{
			NDebugOverlay::Sphere( GetAbsOrigin(), m_flFreeRadius, flDistSqr < Square( m_flFreeRadius.Get() ) ? 0 : 255, flDistSqr < Square( m_flFreeRadius.Get() ) ? 255 : 0, 0, false, 0.01f );
		}
	}
#endif

	if ( flDistSqr > Square( m_flMaxRadius.Get() ) )
	{
		return false;
	}

	if ( flDistSqr < Square( m_flFreeRadius.Get() ) )
	{
		return true;
	}

	trace_t tr;
	CTraceFilterFadeProxy filter( m_bBrushOnly );
	UTIL_TraceLine( vecEyePosition, GetAbsOrigin(), MASK_OPAQUE, &filter, &tr );

#ifdef CLIENT_DLL
	if ( asw_debug_fade.GetBool() )
	{
		NDebugOverlay::Line( vecEyePosition, tr.endpos, tr.fraction >= 1.0f ? 0 : 255, tr.fraction >= 1.0f ? 255 : 0, 0, false, 0.01f );
	}
#endif

	return tr.fraction >= 1.0f;
}

#ifdef CLIENT_DLL
int CTrigger_ASW_Fade_Proxy::DrawModel( int flags, const RenderableInstance_t &instance )
{
	if ( !asw_debug_fade.GetBool() )
	{
		return 0;
	}

	// BenLubar: There doesn't seem to be an easy way to draw the actual trigger as a wireframe,
	// so we're going to just draw a red or green bounding box and call it a day.
	debugoverlay->AddBoxOverlay2( CollisionProp()->GetCollisionOrigin(), CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(),
		CollisionProp()->GetCollisionAngles(), Color{ 0, 0, 0, 0 },
		Color{ m_bLastTouchingTrigger ? 0 : 255, m_bLastTouchingTrigger ? 255 : 0, 0, 255 }, 0.01f );

	return 1;
}
#else
void CTrigger_ASW_Fade_Proxy::Spawn()
{
	BaseClass::Spawn();

	SetModel( STRING( GetModelName() ) );
	AddEffects( EF_NODRAW );
	AddSolidFlags( FSOLID_VOLUME_CONTENTS );
}
#endif

bool CTrigger_ASW_Fade_Proxy::ShouldFade( const Vector &vecEyePosition )
{
	if ( m_bDisabled )
	{
		return false;
	}

	bool bIsTouchingTrigger = enginetrace->GetPointContents_Collideable( GetCollideable(), vecEyePosition ) != CONTENTS_EMPTY;

#ifdef CLIENT_DLL
	m_bLastTouchingTrigger = bIsTouchingTrigger;
#endif

	return bIsTouchingTrigger;
}

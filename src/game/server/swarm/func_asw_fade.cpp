#include "cbase.h"
#include "func_asw_fade.h"
#include "asw_shareddefs.h"
#include "asw_fade_proxy_shared.h"
#include "asw_inhabitable_npc.h"
#include "asw_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( func_asw_fade, CFunc_ASW_Fade );

BEGIN_DATADESC( CFunc_ASW_Fade )
	DEFINE_FIELD( m_bHasProxies, FIELD_BOOLEAN ),
	DEFINE_KEYFIELD( m_bCollideWithGrenades, FIELD_BOOLEAN, "CollideWithGrenades" ),
	DEFINE_KEYFIELD( m_nFadeOpacity, FIELD_CHARACTER, "fade_opacity" ),
	DEFINE_INPUT( m_bAllowFade, FIELD_BOOLEAN, "AllowFade" ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CFunc_ASW_Fade, DT_Func_ASW_Fade )
	SendPropInt( SENDINFO( m_nFadeOpacity ), 8, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bAllowFade ) ),
	SendPropBool( SENDINFO( m_bHasProxies ) ),
END_SEND_TABLE()

CFunc_ASW_Fade::CFunc_ASW_Fade()
{
	m_bHasProxies = false;
	m_bCollideWithGrenades = false;
	m_nFadeOpacity = 0;
	m_bAllowFade = true;
}

void CFunc_ASW_Fade::Spawn()
{
	BaseClass::Spawn();

	SetCollisionGroup( m_bCollideWithGrenades ? COLLISION_GROUP_NONE : ASW_COLLISION_GROUP_CEILINGS );
}

bool CFunc_ASW_Fade::ShouldFade( CASW_Inhabitable_NPC *pNPC )
{
	if ( !m_bAllowFade || !pNPC || !pNPC->IsInhabited() || !pNPC->GetCommander() )
	{
		return false;
	}

	if ( pNPC->GetCommander()->GetASWControls() == ASWC_TOPDOWN )
	{
		if ( m_bHasProxies )
		{
			Vector vecEyePosition = pNPC->EyePosition();

#ifdef DBGFLAG_ASSERT
			bool bAtLeastOneProxy = false;
#endif
			for ( CBaseEntity *pEnt = FirstMoveChild(); pEnt; pEnt = pEnt->NextMovePeer() )
			{
				CPoint_ASW_Fade_Proxy *pProxy = dynamic_cast< CPoint_ASW_Fade_Proxy * >( pEnt );
				if ( pProxy )
				{
					if ( pProxy->ShouldFade( vecEyePosition ) )
					{
						return true;
					}

#ifdef DBGFLAG_ASSERT
					bAtLeastOneProxy = true;
#endif
				}
			}

			Assert( bAtLeastOneProxy );

			return false;
		}

		return pNPC->GetAbsOrigin().z < GetAbsOrigin().z;
	}

	return false;
}

#include "cbase.h"
#include "prop_asw_fade.h"
#include "asw_fade_proxy_shared.h"
#include "asw_inhabitable_npc.h"
#include "asw_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( prop_asw_fade, CProp_ASW_Fade );

BEGIN_DATADESC( CProp_ASW_Fade )
	DEFINE_FIELD( m_bHasProxies, FIELD_BOOLEAN ),
	DEFINE_KEYFIELD( m_nFadeOpacity, FIELD_CHARACTER, "fade_opacity" ),
	DEFINE_INPUT( m_bAllowFade, FIELD_BOOLEAN, "AllowFade" ),
	DEFINE_KEYFIELD( m_vecFadeOrigin, FIELD_VECTOR, "fade_origin" ),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CProp_ASW_Fade, DT_Prop_ASW_Fade )
	SendPropInt( SENDINFO( m_nFadeOpacity ), 8, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bAllowFade ) ),
	SendPropFloat( SENDINFO( m_flFadeOriginOffset ) ),
	SendPropBool( SENDINFO( m_bHasProxies ) ),
END_SEND_TABLE()

CProp_ASW_Fade::CProp_ASW_Fade()
{
	m_bHasProxies = false;
	m_flFadeOriginOffset = 0;
	m_vecFadeOrigin = vec3_origin;
	m_nFadeOpacity = 0;
	m_bAllowFade = true;
}

void CProp_ASW_Fade::Spawn()
{
	BaseClass::Spawn();

	m_flFadeOriginOffset = m_vecFadeOrigin.z - GetAbsOrigin().z;
}

bool CProp_ASW_Fade::ShouldFade( CASW_Inhabitable_NPC *pNPC )
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

		return pNPC->GetAbsOrigin().z < GetAbsOrigin().z + m_flFadeOriginOffset;
	}

	return false;
}

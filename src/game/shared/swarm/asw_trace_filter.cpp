#include "cbase.h"
#ifdef CLIENT_DLL
#include "c_func_asw_fade.h"
#include "c_prop_asw_fade.h"
#include "c_asw_player.h"
#include "c_asw_marine.h"
#define CFunc_ASW_Fade C_Func_ASW_Fade
#define CProp_ASW_Fade C_Prop_ASW_Fade
#define CASW_Player C_ASW_Player
#else
#include "func_asw_fade.h"
#include "prop_asw_fade.h"
#include "asw_player.h"
#include "asw_marine.h"
#endif
#include "asw_trace_filter.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_Trace_Filter::CASW_Trace_Filter( CBasePlayer *pPlayer, Collision_Group_t collisionGroup )
	: BaseClass( NULL, collisionGroup )
{
	m_pMarine = assert_cast<CASW_Player *>( pPlayer )->GetViewMarine();
}

CASW_Trace_Filter::CASW_Trace_Filter( CASW_Marine *pMarine, Collision_Group_t collisionGroup )
	: BaseClass( NULL, collisionGroup ), m_pMarine( pMarine )
{
}

bool CASW_Trace_Filter::ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
{
	if ( !m_pMarine )
	{
		return BaseClass::ShouldHitEntity( pServerEntity, contentsMask );
	}

	if ( !m_pMarine->IsInhabited() || !m_pMarine->GetCommander() || m_pMarine->GetCommander()->GetASWControls() != 1 )
	{
		if ( pServerEntity == m_pMarine )
		{
			return false;
		}
	}
	else
	{
		CFunc_ASW_Fade *pBrush = dynamic_cast<CFunc_ASW_Fade *>( pServerEntity );
		if ( pBrush && pBrush->GetAbsOrigin().z > m_pMarine->GetAbsOrigin().z )
		{
			return false;
		}
		CProp_ASW_Fade *pProp = dynamic_cast<CProp_ASW_Fade *>( pServerEntity );
#ifdef GAME_DLL
#define m_vecFadeOrigin m_vecFadeOrigin.Get()
#endif
		if ( pProp && pProp->m_vecFadeOrigin.z > m_pMarine->GetAbsOrigin().z )
		{
			return false;
		}
	}
	return BaseClass::ShouldHitEntity( pServerEntity, contentsMask );
}

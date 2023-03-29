#include "cbase.h"
#ifdef CLIENT_DLL
#include "c_func_asw_fade.h"
#include "c_prop_asw_fade.h"
#include "c_asw_player.h"
#include "c_asw_inhabitable_npc.h"
#define CFunc_ASW_Fade C_Func_ASW_Fade
#define CProp_ASW_Fade C_Prop_ASW_Fade
#define CASW_Player C_ASW_Player
#else
#include "func_asw_fade.h"
#include "prop_asw_fade.h"
#include "asw_player.h"
#include "asw_inhabitable_npc.h"
#endif
#include "asw_trace_filter.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CASW_Trace_Filter::CASW_Trace_Filter( CBasePlayer *pPlayer, Collision_Group_t collisionGroup )
	: BaseClass( NULL, collisionGroup )
{
	m_pNPC = assert_cast<CASW_Player *>( pPlayer )->GetViewNPC();
}

CASW_Trace_Filter::CASW_Trace_Filter( CASW_Inhabitable_NPC *pNPC, Collision_Group_t collisionGroup )
	: BaseClass( NULL, collisionGroup ), m_pNPC( pNPC )
{
}

bool CASW_Trace_Filter::ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
{
	if ( !m_pNPC )
	{
		return BaseClass::ShouldHitEntity( pServerEntity, contentsMask );
	}

	if ( !m_pNPC->IsInhabited() || !m_pNPC->GetCommander() || m_pNPC->GetCommander()->GetASWControls() != ASWC_TOPDOWN )
	{
		if ( pServerEntity == m_pNPC )
		{
			return false;
		}
	}
	else
	{
		CFunc_ASW_Fade *pBrush = dynamic_cast< CFunc_ASW_Fade * >( pServerEntity );
		if ( pBrush && pBrush->ShouldFade( m_pNPC ) )
		{
			return false;
		}
		CProp_ASW_Fade *pProp = dynamic_cast< CProp_ASW_Fade * >( pServerEntity );
		if ( pProp && pProp->ShouldFade( m_pNPC ) )
		{
			return false;
		}
	}
	return BaseClass::ShouldHitEntity( pServerEntity, contentsMask );
}

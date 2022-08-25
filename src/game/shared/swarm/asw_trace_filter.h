#ifndef INCLUDED_ASW_TRACE_FILTER_H
#define INCLUDED_ASW_TRACE_FILTER_H
#ifdef _WIN32
#pragma once
#endif

#include "util_shared.h"

#ifdef CLIENT_DLL
#define CBasePlayer C_BasePlayer
#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
#endif

class CBasePlayer;
class CASW_Inhabitable_NPC;

class CASW_Trace_Filter : public CTraceFilterSimple
{
public:
	CASW_Trace_Filter( CBasePlayer *pPlayer, Collision_Group_t collisionGroup );
	CASW_Trace_Filter( CASW_Inhabitable_NPC *pNPC, Collision_Group_t collisionGroup );

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask );

	CASW_Inhabitable_NPC *m_pNPC;

private:
	typedef CTraceFilterSimple BaseClass;
};

#endif /* INCLUDED_ASW_TRACE_FILTER_H */

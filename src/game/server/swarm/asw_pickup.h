#ifndef _DEFINED_ASW_PICKUP_H
#define _DEFINED_ASW_PICKUP_H

#include "items.h"
#include "asw_shareddefs.h"
#include "iasw_server_usable_entity.h"

class CASW_Player;
class CASW_Marine;
class CASW_Inhabitable_NPC;

class CASW_Pickup : public CItem, public IASW_Server_Usable_Entity
{
public:
	DECLARE_CLASS( CASW_Pickup, CItem );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Spawn();

	bool m_bFreezePickup;	// if set, the pickup won't be physically simulated, but will be frozen in place

	virtual bool AllowedToPickup( CASW_Inhabitable_NPC *pNPC );

	// IASW_Server_Usable_Entity implementation
	virtual CBaseEntity* GetEntity() { return this; }
	virtual bool IsUsable( CBaseEntity *pUser );
	virtual bool RequirementsMet( CBaseEntity *pUser ) { return true; }
	virtual void ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType );
	virtual void NPCUsing( CASW_Inhabitable_NPC *pNPC, float deltatime ) { }
	virtual void NPCStartedUsing( CASW_Inhabitable_NPC *pNPC ) { }
	virtual void NPCStoppedUsing( CASW_Inhabitable_NPC *pNPC ) { }
	virtual bool NeedsLOSCheck() { return true; }
};

#endif /* _DEFINED_ASW_PICKUP_H */
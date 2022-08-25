#ifndef _INCLUDED_ASW_SERVER_USABLE_ENTITY_H
#define _INCLUDED_ASW_SERVER_USABLE_ENTITY_H

class CASW_Inhabitable_NPC;

abstract_class IASW_Server_Usable_Entity
{
public:
	virtual CBaseEntity *GetEntity() = 0;
	virtual bool IsUsable( CBaseEntity *pUser ) = 0;
	virtual bool RequirementsMet( CBaseEntity *pUser ) = 0;
	virtual void ActivateUseIcon( CASW_Inhabitable_NPC *pUser, int nHoldType ) = 0;
	virtual void NPCStartedUsing( CASW_Inhabitable_NPC *pUser ) = 0;
	virtual void NPCStoppedUsing( CASW_Inhabitable_NPC *pUser ) = 0;
	virtual void NPCUsing( CASW_Inhabitable_NPC *pUser, float fDeltaTime ) = 0;
	virtual bool NeedsLOSCheck() = 0;
};

#endif // _INCLUDED_ASW_SERVER_USABLE_ENTITY_H

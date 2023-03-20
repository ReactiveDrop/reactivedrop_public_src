#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CASW_Weapon_Revive_Tool C_ASW_Weapon_Revive_Tool
#define CASW_Revive_Tombstone C_ASW_Revive_Tombstone
#define CASW_Marine C_ASW_Marine
#define CASW_Marine_Resource C_ASW_Marine_Resource

struct dlight_t;
#else
#include "asw_weapon.h"
#endif

class CASW_Marine;
class CASW_Marine_Resource;

#ifdef RD_7A_WEAPONS
DECLARE_AUTO_LIST( IASW_Revive_Tool_Auto_List );
DECLARE_AUTO_LIST( IASW_Revive_Tombstone_Auto_List );

class CASW_Weapon_Revive_Tool : public CASW_Weapon, public IASW_Revive_Tool_Auto_List
{
public:
	DECLARE_CLASS( CASW_Weapon_Revive_Tool, CASW_Weapon );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	IMPLEMENT_AUTO_LIST_GET();

#ifdef CLIENT_DLL
#else
	DECLARE_DATADESC();
#endif

	void Precache() override;

	// This weapon does nothing when you click.
	void PrimaryAttack() override {}
	void SecondaryAttack() override {}
	bool IsOffensiveWeapon() override { return false; }

	Class_T Classify() override { return ( Class_T )CLASS_ASW_REVIVE_TOOL; }
};

class CASW_Revive_Tombstone : public CBaseAnimating, public IASW_Revive_Tombstone_Auto_List,
#ifdef CLIENT_DLL
	public IASW_Client_Usable_Entity
#else
	public IASW_Server_Usable_Entity
#endif
{
public:
	DECLARE_CLASS( CASW_Revive_Tombstone, CBaseAnimating );
	DECLARE_NETWORKCLASS();

	CASW_Revive_Tombstone();
	~CASW_Revive_Tombstone();
	IMPLEMENT_AUTO_LIST_GET();

	void Precache() override;
	bool IsUsable( CBaseEntity *pUser ) override;
	bool NeedsLOSCheck() override { return false; }

#ifdef CLIENT_DLL
	void OnDataChanged( DataUpdateType_t updateType ) override;
	void ClientThink() override;

	bool m_bHologramActive;
	bool m_bIsReviving;

	dlight_t *m_pLightGlow;

	// IASW_Client_Usable_Entity implementation
	bool GetUseAction( ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser ) override;
	void CustomPaint( int ix, int iy, int alpha, vgui::Panel *pUseIcon ) override {}
	bool ShouldPaintBoxAround() override { return false; }
#else
	DECLARE_DATADESC();

	static CASW_Revive_Tombstone *MaybeCreateTombstone( CASW_Marine *pMarine, const CTakeDamageInfo &info, bool bGibbed );

	void Spawn() override;
	bool CreateVPhysics() override;
	void TombstoneThink();

	int m_iPathFails;
	EHANDLE m_hKeepUpright;
	bool m_bKeepUpright;

	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_hGroundEntity );

	// IASW_Server_Usable_Entity implementation
	bool RequirementsMet( CBaseEntity *pUser ) override { return true; }
	void ActivateUseIcon( CASW_Inhabitable_NPC *pUser, int nHoldType ) override;
	void NPCStartedUsing( CASW_Inhabitable_NPC *pUser ) override {}
	void NPCStoppedUsing( CASW_Inhabitable_NPC *pUser ) override {}
	void NPCUsing( CASW_Inhabitable_NPC *pUser, float fDeltaTime ) override {}
#endif

	Class_T Classify() override { return ( Class_T )CLASS_ASW_REVIVE_TOOL_MARKER; }

	CNetworkHandle( CASW_Marine_Resource, m_hMarineResource );
	CNetworkVar( bool, m_bGibbed );
	CNetworkVar( float, m_flReviveTime );
};
#endif

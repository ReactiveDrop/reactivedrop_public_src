#pragma once

#include "c_asw_inhabitable_npc.h"
#include "iasw_client_usable_entity.h"

class C_ASW_Marine;

class C_ASW_Colonist : public C_ASW_Inhabitable_NPC, public IASW_Client_Usable_Entity
{
	DECLARE_CLASS( C_ASW_Colonist, C_ASW_Inhabitable_NPC );
public:
	DECLARE_CLIENTCLASS();

	C_ASW_Colonist();
	C_ASW_Colonist( const C_ASW_Colonist & ) = delete;

	bool IsAimTarget() override { return m_bAimTarget && BaseClass::IsAimTarget(); }
	Class_T Classify() override { return ( Class_T )CLASS_ASW_COLONIST; }

	// IASW_Client_Usable_Entity implementation
	C_BaseEntity *GetEntity() override { return this; }
	bool IsUsable( C_BaseEntity *pUser ) override;
	bool GetUseAction( ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser ) override;
	void CustomPaint( int ix, int iy, int alpha, vgui::Panel *pUseIcon ) override {}
	bool ShouldPaintBoxAround() override { return m_bFollowOnUse; }
	bool NeedsLOSCheck() override { return m_bFollowOnUse; }

	CNetworkVar( bool, m_bAimTarget );
	CNetworkVar( bool, m_bFollowOnUse );
	CNetworkHandle( C_ASW_Marine, m_hFollowingMarine );
};

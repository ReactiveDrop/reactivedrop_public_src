#include "cbase.h"
#include "c_asw_use_area.h"
#include "c_asw_marine.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include "asw_shareddefs.h"
#include "rd_weapon_generic_object_shared.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Use_Area, DT_ASW_Use_Area, CASW_Use_Area )
	RecvPropEHandle( RECVINFO(m_hUseTarget) ),
	RecvPropBool( RECVINFO(m_bUseAreaEnabled) ),
	RecvPropEHandle( RECVINFO( m_hPanelProp ) ),
	RecvPropString( RECVINFO( m_iHeldObjectName ) ),
END_RECV_TABLE()

C_ASW_Use_Area::C_ASW_Use_Area()
{

}

C_BaseEntity* C_ASW_Use_Area::GetUseTarget()
{
	return GetUseTargetHandle().Get();
}

// check we're near enough
bool C_ASW_Use_Area::IsUsable(C_BaseEntity *pUser)
{
	return m_bUseAreaEnabled.Get() && CollisionProp()->IsPointInBounds(pUser->WorldSpaceCenter());
}

bool C_ASW_Use_Area::GetUseAction(ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser)
{
	// fill in by subclasses
	return false;
}

bool C_ASW_Use_Area::CheckHeldObject( C_ASW_Inhabitable_NPC *pNPC )
{
	if ( !pNPC )
	{
		return false;
	}

	if ( !m_iHeldObjectName[0] )
	{
		// no object required
		return true;
	}

	C_RD_Weapon_Generic_Object *pObject = dynamic_cast<C_RD_Weapon_Generic_Object *>( pNPC->GetActiveASWWeapon() );
	if ( !pObject )
	{
		return false;
	}

	return FStrEq( m_iHeldObjectName, pObject->m_iOriginalName );
}

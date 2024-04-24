#include "cbase.h"
#include "asw_scanner_objects_enumerator.h"
#include "rd_crafting_defs.h"

#ifdef CLIENT_DLL
	#include "c_asw_button_area.h"
	#include "c_asw_computer_area.h"
	#include "c_asw_door.h"
	#include "c_asw_inhabitable_npc.h"
	#define CASW_Button_Area C_ASW_Button_Area
	#define CASW_Computer_Area C_ASW_Computer_Area
	#define CASW_Door C_ASW_Door
	#define CASW_Inhabitable_NPC C_ASW_Inhabitable_NPC
#else
	#include "asw_button_area.h"
	#include "asw_computer_area.h"
	#include "asw_door.h"
	#include "asw_inhabitable_npc.h"
	#include "asw_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static bool ValidScannerObject( CBaseEntity *pEnt )
{
	if ( pEnt->IsInhabitableNPC() )
	{
		CASW_Inhabitable_NPC *pNPC = assert_cast< CASW_Inhabitable_NPC * >( pEnt );
		return pNPC->GetHealth() > 0 && pNPC->Classify() != CLASS_ASW_MARINE && pNPC->Classify() != CLASS_ASW_COLONIST;
	}

	int iClass = pEnt->Classify();
	if ( iClass == CLASS_ASW_BUTTON_PANEL || iClass == CLASS_ASW_COMPUTER_AREA )
		return true;

	if ( iClass == CLASS_ASW_DOOR )
	{
		CASW_Door *pDoor = assert_cast< CASW_Door * >( pEnt );
		if ( pDoor->IsMoving() )
			return true;

		return false;
	}

#ifdef RD_7A_DROPS
	if ( iClass == CLASS_RD_CRAFTING_MATERIAL_PICKUP )
	{
		CRD_Crafting_Material_Pickup *pPickup = assert_cast< CRD_Crafting_Material_Pickup * >( pEnt );
		if ( !pPickup->m_bAnyoneFound )
			return false;

#ifdef CLIENT_DLL
		return pPickup->m_iLastMaterialType != RD_CRAFTING_MATERIAL_NONE;
#else
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			// only count players who are controlling a character
			CASW_Player *pPlayer = ToASW_Player( UTIL_PlayerByIndex( i ) );
			if ( !pPlayer || !pPlayer->GetNPC() )
				continue;

			if ( pPickup->m_MaterialAtLocation[i - 1] != RD_CRAFTING_MATERIAL_NONE )
				return true;
		}

		return false;
#endif
	}
#endif

	return false;
}

CASW_Scanner_Objects_Enumerator::CASW_Scanner_Objects_Enumerator( float radius, Vector &vecCenter ) :
	m_vecScannerCenter(vecCenter)
{
	m_flRadius = radius;
	m_flRadiusSquared = radius * radius;
	m_Objects.RemoveAll();
}

int	CASW_Scanner_Objects_Enumerator::GetObjectCount()
{
	return m_Objects.Count();
}

CBaseEntity *CASW_Scanner_Objects_Enumerator::GetObject( int index )
{
	if ( index < 0 || index >= GetObjectCount() )
		return NULL;

	return m_Objects[ index ];
}

// Actual work code
IterationRetval_t CASW_Scanner_Objects_Enumerator::EnumElement( IHandleEntity *pHandleEntity )
{
#ifndef CLIENT_DLL
	CBaseEntity *pEnt = gEntList.GetBaseEntity( pHandleEntity->GetRefEHandle() );
#else
	C_BaseEntity *pEnt = ClientEntityList().GetBaseEntityFromHandle( pHandleEntity->GetRefEHandle() );
#endif

	if ( pEnt == NULL )
		return ITERATION_CONTINUE;

	if ( !ValidScannerObject( pEnt ) )
		return ITERATION_CONTINUE;

	Vector deltaPos = pEnt->GetAbsOrigin() - m_vecScannerCenter;
	if ( deltaPos.LengthSqr() > m_flRadiusSquared )
		return ITERATION_CONTINUE;

	if ( m_vecScannerCenter.z - pEnt->GetAbsOrigin().z > m_flRadius / 3 )
		return ITERATION_CONTINUE;

	CHandle< CBaseEntity > h;
	h = pEnt;
	m_Objects.AddToTail( h );

	return ITERATION_CONTINUE;
}

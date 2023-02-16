#include "cbase.h"
#include "asw_scanner_objects_enumerator.h"

#ifdef CLIENT_DLL
	#include "c_ai_basenpc.h"
	#include "c_asw_alien.h"
	#include "c_asw_buzzer.h"
	#include "c_asw_door.h"
	#include "c_asw_computer_area.h"
	#include "c_asw_button_area.h"
	#define CASW_Buzzer C_ASW_Buzzer
	#define CASW_Alien C_ASW_Alien
	#define CASW_Door C_ASW_Door
	#define CASW_Grub C_ASW_Grub
	#define CASW_Computer_Area C_ASW_Computer_Area
	#define CASW_Button_Area C_ASW_Button_Area
#else
	#include "ai_basenpc.h"
	#include "asw_alien.h"
	#include "asw_buzzer.h"
	#include "asw_door.h"
	#include "asw_computer_area.h"
	#include "asw_button_area.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

namespace
{
	bool ValidScannerObject(CBaseEntity *pEnt)
	{
		//Orange. Do not bother checking this. Our current existing asw_grub is CASW_Simple_Grub and quite rare thing to be checked from 1st position.
		//// dron't show grubs, they're too small
		//CASW_Grub *pGrub = dynamic_cast<CASW_Grub*>(pEnt);
		//if (pGrub)
		//	return false;
		if ( pEnt->IsAlienClassType() )
		{
			CASW_Alien* pAlien = assert_cast<CASW_Alien*>(pEnt);
			return pAlien->GetHealth() > 0;
		}

		//assume pEnt != NULL since called only from EnumElement() with corresponding check above inside it
		switch ( (int) pEnt->Classify() )
		{
		case CLASS_ASW_DOOR:
			{
				CASW_Door* pDoor = assert_cast<CASW_Door*>(pEnt);
				if (pDoor->IsMoving())
					return true;
				return false;
			}
		case CLASS_ASW_BUZZER:
			{
				CASW_Buzzer* pBuzzer = assert_cast<CASW_Buzzer*>(pEnt);
				return pBuzzer->GetHealth() > 0;
			}
		case CLASS_ASW_COMPUTER_AREA:
			{
				return true;
			}
		case CLASS_ASW_BUTTON_PANEL:
			{
				return true;
			}
		default:
			{
				return false;
			}
		}
	}
}

// Enumator class for ragdolls being affected by explosive forces
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

	if (!ValidScannerObject(pEnt))
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
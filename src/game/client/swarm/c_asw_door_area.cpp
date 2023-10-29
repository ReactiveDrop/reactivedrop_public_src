#include "cbase.h"
#include "c_asw_door.h"
#include "c_asw_door_area.h"
#include "c_asw_marine.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include "asw_weapon_welder_shared.h"
#include "asw_util_shared.h"
#include "asw_hud_master.h"
#include "asw_input.h"
#include "rd_steam_input.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Door_Area, DT_ASW_Door_Area, CASW_Door_Area )

END_RECV_TABLE()

C_ASW_Door_Area::C_ASW_Door_Area()
{
}

C_ASW_Door *C_ASW_Door_Area::GetASWDoor()
{
	C_BaseEntity *pUseTargetH = GetUseTargetHandle().Get();
	if ( pUseTargetH && pUseTargetH->Classify() == CLASS_ASW_DOOR )
		return assert_cast< C_ASW_Door * >( pUseTargetH );
	return NULL;
}

bool C_ASW_Door_Area::GetUseAction( ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser )
{
	C_ASW_Door *pDoor = GetASWDoor();
	if ( !pDoor || !pUser || !pDoor->m_bCanPlayerWeld )
		return false;

	bool bHasWelder = pUser->Weapon_OwnsThisType( "asw_weapon_welder" ) != NULL;

	if ( !bHasWelder )
	{
		CASW_Hud_Master *pHUDMaster = GET_HUDELEMENT( CASW_Hud_Master );
		if ( pHUDMaster )
		{
			int nWelderPosition = pHUDMaster->GetHotBarSlot( "asw_weapon_welder" );

			if ( nWelderPosition != -1 && pHUDMaster->OwnsHotBarSlot( pUser->GetCommander(), nWelderPosition ) )
			{
				bHasWelder = true;
				if ( nWelderPosition >= 100 )
				{
					V_strncpy( action.szCommand, "+walk", sizeof( action.szCommand ) );
				}
				else
				{
					V_snprintf( action.szCommand, sizeof( action.szCommand ), "asw_squad_hotbar %i", nWelderPosition + 1 );
				}

				// if we don't have a direct bind, try asking for a radial menu instead
				if ( !g_RD_Steam_Input.Key_LookupBindingEx( action.szCommand, -1, 0, ASWInput()->ControllerModeActive() ) )
				{
					V_snprintf( action.szCommand, sizeof( action.szCommand ), "+mouse_menu ASW_UseExtra%d", nWelderPosition >= 100 ? 2 : 1 );

					if ( !g_RD_Steam_Input.Key_LookupBindingEx( action.szCommand, -1, 0, ASWInput()->ControllerModeActive() ) )
					{
						// final attempt: use the version of the radial menu that is affected by the +walk command
						V_strncpy( action.szCommand, "+mouse_menu ASW_UseExtra", sizeof( action.szCommand ) );
					}
				}
			}
		}
	}
	else
	{
		V_strncpy( action.szCommand, "+grenade1", sizeof( action.szCommand ) );
	}

	// if door is sealed
		// add sealed icon with bar showing seal percent
	if ( pDoor->GetHealth() > 0 )
	{
		if ( pDoor->GetSealAmount() > 0 )
		{
			if ( pDoor->GetSealAmount() >= 1.0f )
				action.iUseIconTexture = pDoor->GetFullySealedIconTextureID();
			else
				action.iUseIconTexture = pDoor->GetSealedIconTextureID();
			TryLocalize( pDoor->GetSealedIconText(), action.wszText, sizeof( action.wszText ) );
			action.UseTarget = this;
			action.fProgress = pDoor->GetSealAmount();
			action.UseIconRed = 255;
			action.UseIconGreen = 255;
			action.UseIconBlue = 255;
			action.bShowUseKey = bHasWelder;
			action.iInventorySlot = -1;

			return true;
		}
		else if ( pUser->GetActiveWeapon() )
		{
			if ( bHasWelder )
			{
				if ( pDoor->GetSealAmount() >= 1.0f )
				{
					action.iUseIconTexture = pDoor->GetFullySealedIconTextureID();
				}
				else
				{
					action.iUseIconTexture = pDoor->GetSealedIconTextureID();
				}

				TryLocalize( pDoor->GetUnsealedIconText(), action.wszText, sizeof( action.wszText ) );
				action.UseTarget = this;
				action.fProgress = pDoor->GetSealAmount();
				action.UseIconRed = 255;
				action.UseIconGreen = 255;
				action.UseIconBlue = 255;
				action.bShowUseKey = true;
				action.iInventorySlot = -1;
				return true;
			}
		}
	}
	return false;
}

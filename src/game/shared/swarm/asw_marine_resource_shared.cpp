#include "cbase.h"
#include "rd_inventory_shared.h"
#ifdef CLIENT_DLL
#include "c_asw_marine_resource.h"
#else
#include "asw_marine_resource.h"
#include "asw_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL
#define COMMANDER_ID "<local>"
#else
#define COMMANDER_ID m_OriginalCommander ? m_OriginalCommander->GetASWNetworkID() : "<null>"
#endif

void CASW_Marine_Resource::ClearInvalidEquipData()
{
	if ( m_EquippedItemData.m_Suit.IsSet() )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_EquippedItemData.m_Suit.m_iItemDefID );
		Assert( pDef );
		if ( !pDef || !pDef->ItemSlotMatches( "marine_any" ) || pDef->EquipIndex != GetProfileIndex() )
		{
			Warning( "Clearing suit item for marine resource entindex %d (player %s) - item %llu (def %d) is not valid for this slot (profile %d)\n", entindex(), COMMANDER_ID, m_EquippedItemData.m_Suit.m_iItemInstanceID, m_EquippedItemData.m_Suit.m_iItemDefID, GetProfileIndex() );
			m_EquippedItemData.m_Suit.GetForModify().Reset();
		}
	}

	if ( m_EquippedItemData.m_Weapon1.IsSet() )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_EquippedItemData.m_Weapon1.m_iItemDefID );
		Assert( pDef );
		if ( !pDef || !pDef->ItemSlotMatches( "weapon" ) || pDef->EquipIndex != m_iInitialWeaponsInSlots[ASW_INVENTORY_SLOT_PRIMARY] )
		{
			Warning( "Clearing primary weapon item for marine resource entindex %d (player %s) - item %llu (def %d) is not valid for this slot (equip %d)\n", entindex(), COMMANDER_ID, m_EquippedItemData.m_Weapon1.m_iItemInstanceID, m_EquippedItemData.m_Weapon1.m_iItemDefID, m_iInitialWeaponsInSlots[ASW_INVENTORY_SLOT_PRIMARY] );
			m_EquippedItemData.m_Weapon1.GetForModify().Reset();
		}
	}

	if ( m_EquippedItemData.m_Weapon2.IsSet() )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_EquippedItemData.m_Weapon2.m_iItemDefID );
		Assert( pDef );
		if ( !pDef || !pDef->ItemSlotMatches( "weapon" ) || pDef->EquipIndex != m_iInitialWeaponsInSlots[ASW_INVENTORY_SLOT_SECONDARY] )
		{
			Warning( "Clearing secondary weapon item for marine resource entindex %d (player %s) - item %llu (def %d) is not valid for this slot (equip %d)\n", entindex(), COMMANDER_ID, m_EquippedItemData.m_Weapon2.m_iItemInstanceID, m_EquippedItemData.m_Weapon2.m_iItemDefID, m_iInitialWeaponsInSlots[ASW_INVENTORY_SLOT_SECONDARY] );
			m_EquippedItemData.m_Weapon2.GetForModify().Reset();
		}
	}

	if ( m_EquippedItemData.m_Extra.IsSet() )
	{
		const ReactiveDropInventory::ItemDef_t *pDef = ReactiveDropInventory::GetItemDef( m_EquippedItemData.m_Extra.m_iItemDefID );
		Assert( pDef );
		if ( !pDef || !pDef->ItemSlotMatches( "extra" ) || pDef->EquipIndex != m_iInitialWeaponsInSlots[ASW_INVENTORY_SLOT_EXTRA] )
		{
			Warning( "Clearing equipment item for marine resource entindex %d (player %s) - item %llu (def %d) is not valid for this slot (equip %d)\n", entindex(), COMMANDER_ID, m_EquippedItemData.m_Extra.m_iItemInstanceID, m_EquippedItemData.m_Extra.m_iItemDefID, m_iInitialWeaponsInSlots[ASW_INVENTORY_SLOT_EXTRA] );
			m_EquippedItemData.m_Extra.GetForModify().Reset();
		}
	}
}

#include "cbase.h"
#include "convar.h"
#include "entitylist.h"
#include "asw_weapon.h"
#include "asw_marine.h"
#include "asw_item_regen.h"
#include "asw_marine_profile.h"
#include "asw_game_resource.h"
#include "asw_marine_resource.h"
#include "asw_equipment_list.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar rm_item_regen_interval( "rm_item_regen_interval", "60", FCVAR_NONE, "Interval between item regeneration", true, 1.0f, false, 0.0f );

LINK_ENTITY_TO_CLASS( asw_item_regen, CASW_Item_Regen );

CASW_Item_Regen::CASW_Item_Regen()
{
}

CASW_Item_Regen::~CASW_Item_Regen()
{
}

void CASW_Item_Regen::Spawn()
{
	BaseClass::Spawn();
	SetNextThink( gpGlobals->curtime ); // Think now
}

extern ConVar asw_horde_override;

void CASW_Item_Regen::Think()
{
	BaseClass::Think();
	CBaseEntity *pEntity = NULL;
	if ( asw_horde_override.GetBool() )
	{
		while ( ( pEntity = gEntList.FindEntityByClassname( pEntity, "asw_marine" ) ) != NULL )
		{
			if ( pEntity->Classify() == CLASS_ASW_MARINE )
			{
				CASW_Marine *pMarine = assert_cast< CASW_Marine * >( pEntity );

				for ( int k = 0; k < ASW_MAX_MARINE_WEAPONS; k++ )
				{
					CASW_Weapon *pWeapon = pMarine->GetASWWeapon( k );
					if ( !pWeapon )
						continue;

					const CASW_EquipItem *pItem = pWeapon->GetEquipItem();
					if ( pItem && pItem->m_bIsExtra )
					{
						pWeapon->m_iClip1 = pWeapon->m_iClip1 + 1;
						if ( pWeapon->m_iClip1 > pWeapon->GetMaxClip1() )
							pWeapon->m_iClip1 = pWeapon->GetMaxClip1();
					}
					else
					{
						pWeapon->m_iClip2 = pWeapon->m_iClip2 + 1;
						if ( pWeapon->m_iClip2 > pWeapon->GetMaxClip2() )
							pWeapon->m_iClip2 = pWeapon->GetMaxClip2();
					}
				}
			}
		}
	}

	SetNextThink( gpGlobals->curtime + rm_item_regen_interval.GetFloat() );
}

#include "cbase.h"
#include "c_asw_pickup_weapon.h"
#include "asw_gamerules.h"
#include "c_asw_marine.h"
#include "c_asw_marine_resource.h"
#include "c_asw_weapon.h"
#include "asw_equipment_list.h"
#include "asw_weapon_parse.h"
#include <vgui/ISurface.h>
#include <vgui_controls/Panel.h>
#include "asw_util_shared.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//--------------------
// Base weapon pickup
//--------------------

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Pickup_Weapon, DT_ASW_Pickup_Weapon, CASW_Pickup_Weapon )
	RecvPropInt		(RECVINFO(m_iBulletsInGun)),
	RecvPropInt		(RECVINFO(m_iClips)),
	RecvPropInt		(RECVINFO(m_iSecondary)),
	RecvPropBool		(RECVINFO(m_bIsTemporaryPickup)),
END_RECV_TABLE()

C_ASW_Pickup_Weapon::C_ASW_Pickup_Weapon()
{
	m_nUseIconTextureID = -1;
	m_bWideIcon = false;
}

void C_ASW_Pickup_Weapon::InitPickup()
{
	CASW_EquipItem *pItem = g_ASWEquipmentList.GetEquipItemFor( GetWeaponClass() );
	if ( !pItem )
		return;

	V_snprintf( m_szUseIconText, sizeof( m_szUseIconText ), "%s", pItem->m_szShortName );

	m_nUseIconTextureID = g_ASWEquipmentList.GetEquipIconTexture( !pItem->m_bIsExtra, pItem->m_iItemIndex );
	m_bWideIcon = !pItem->m_bIsExtra;
}

void C_ASW_Pickup_Weapon::GetUseIconText( wchar_t *unicode, int unicodeBufferSizeInBytes )
{
	wchar_t wszWeaponName[ 128 ];
	TryLocalize( m_szUseIconText, wszWeaponName, sizeof( wszWeaponName ) );

	if ( m_bSwappingWeapon )
	{
		g_pVGuiLocalize->ConstructString( unicode, unicodeBufferSizeInBytes, g_pVGuiLocalize->Find("#asw_swap_weapon_format"), 1, wszWeaponName );
	}
	else
	{
		g_pVGuiLocalize->ConstructString( unicode, unicodeBufferSizeInBytes, g_pVGuiLocalize->Find("#asw_take_weapon_format"), 1, wszWeaponName );
	}
}

bool C_ASW_Pickup_Weapon::GetUseAction(ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser)
{
	if ( BaseClass::GetUseAction( action, pUser ) )
	{
		if ( action.bShowUseKey )
		{
			C_ASW_Marine *pMarine = C_ASW_Marine::AsMarine( pUser );
			action.iInventorySlot = pMarine ? pMarine->GetWeaponPositionForPickup( GetWeaponClass(), m_bIsTemporaryPickup ) : -1;
		}
// 		if ( action.UseIconRed == 255 && action.UseIconGreen == 255 && action.UseIconBlue == 255 )
// 		{
// 			action.UseIconRed = 66;
// 			action.UseIconGreen = 142;
// 			action.UseIconBlue = 192;
// 		}
		action.bWideIcon = m_bWideIcon;
		return true;
	}
	return false;
}


//---------
// Rifle
//---------

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Pickup_Weapon_Rifle, DT_ASW_Pickup_Weapon_Rifle, CASW_Pickup_Weapon_Rifle )
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Rifle::C_ASW_Pickup_Weapon_Rifle()
{
}

//---------
// Prototype Rifle
//---------

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Pickup_Weapon_PRifle, DT_ASW_Pickup_Weapon_PRifle, CASW_Pickup_Weapon_PRifle )
END_RECV_TABLE()

C_ASW_Pickup_Weapon_PRifle::C_ASW_Pickup_Weapon_PRifle()
{
}

//---------
// Autogun
//---------

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Pickup_Weapon_Autogun, DT_ASW_Pickup_Weapon_Autogun, CASW_Pickup_Weapon_Autogun )
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Autogun::C_ASW_Pickup_Weapon_Autogun()
{
}

//---------
// Assault Shotgun
//---------

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Pickup_Weapon_Assault_Shotgun, DT_ASW_Pickup_Weapon_Assault_Shotgun, CASW_Pickup_Weapon_Assault_Shotgun )
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Assault_Shotgun::C_ASW_Pickup_Weapon_Assault_Shotgun()
{
}

//---------
// Pistol
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Pistol, DT_ASW_Pickup_Weapon_Pistol, CASW_Pickup_Weapon_Pistol)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Pistol::C_ASW_Pickup_Weapon_Pistol()
{
}

//---------
// Shotgun
//---------

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Pickup_Weapon_Shotgun, DT_ASW_Pickup_Weapon_Shotgun, CASW_Pickup_Weapon_Shotgun )
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Shotgun::C_ASW_Pickup_Weapon_Shotgun()
{
}

//---------
// Tesla Gun
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Tesla_Gun, DT_ASW_Pickup_Weapon_Tesla_Gun, CASW_Pickup_Weapon_Tesla_Gun)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Tesla_Gun::C_ASW_Pickup_Weapon_Tesla_Gun()
{
}

//---------
// Railgun
//---------

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Pickup_Weapon_Railgun, DT_ASW_Pickup_Weapon_Railgun, CASW_Pickup_Weapon_Railgun )
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Railgun::C_ASW_Pickup_Weapon_Railgun()
{
}

//---------
// Healgun
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Heal_Gun, DT_ASW_Pickup_Weapon_Heal_Gun, CASW_Pickup_Weapon_Heal_Gun)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Heal_Gun::C_ASW_Pickup_Weapon_Heal_Gun()
{
}

//---------
// PDW
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_PDW, DT_ASW_Pickup_Weapon_PDW, CASW_Pickup_Weapon_PDW)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_PDW::C_ASW_Pickup_Weapon_PDW()
{
}

//---------
// Flamer
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Flamer, DT_ASW_Pickup_Weapon_Flamer, CASW_Pickup_Weapon_Flamer)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Flamer::C_ASW_Pickup_Weapon_Flamer()
{
}

//---------
// Minigun
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Minigun, DT_ASW_Pickup_Weapon_Minigun, CASW_Pickup_Weapon_Minigun)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Minigun::C_ASW_Pickup_Weapon_Minigun()
{
}

//---------
// Sniper
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Sniper_Rifle, DT_ASW_Pickup_Weapon_Sniper_Rifle, CASW_Pickup_Weapon_Sniper_Rifle)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Sniper_Rifle::C_ASW_Pickup_Weapon_Sniper_Rifle()
{
}

//---------
// Chainsaw
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Chainsaw, DT_ASW_Pickup_Weapon_Chainsaw, CASW_Pickup_Weapon_Chainsaw)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Chainsaw::C_ASW_Pickup_Weapon_Chainsaw()
{
}

//---------
// Grenade Launcher
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Grenade_Launcher, DT_ASW_Pickup_Weapon_Grenade_Launcher, CASW_Pickup_Weapon_Grenade_Launcher)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Grenade_Launcher::C_ASW_Pickup_Weapon_Grenade_Launcher()
{
}

//---------
// Desert Eagle
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_DEagle, DT_ASW_Pickup_Weapon_DEagle, CASW_Pickup_Weapon_DEagle)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_DEagle::C_ASW_Pickup_Weapon_DEagle()
{
}

//---------
// Devastator
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Devastator, DT_ASW_Pickup_Weapon_Devastator, CASW_Pickup_Weapon_Devastator)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Devastator::C_ASW_Pickup_Weapon_Devastator()
{
}

//---------
// Combat Rifle
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_CombatRifle, DT_ASW_Pickup_Weapon_CombatRifle, CASW_Pickup_Weapon_CombatRifle)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_CombatRifle::C_ASW_Pickup_Weapon_CombatRifle()
{
}

//---------
// HealAmp Gun
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_HealAmp_Gun, DT_ASW_Pickup_Weapon_HealAmp_Gun, CASW_Pickup_Weapon_HealAmp_Gun)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_HealAmp_Gun::C_ASW_Pickup_Weapon_HealAmp_Gun()
{
}

//---------
// Heavy Rifle
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Heavy_Rifle, DT_ASW_Pickup_Weapon_Heavy_Rifle, CASW_Pickup_Weapon_Heavy_Rifle)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Heavy_Rifle::C_ASW_Pickup_Weapon_Heavy_Rifle()
{
}

//---------
// Med Rifle
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_MedRifle, DT_ASW_Pickup_Weapon_MedRifle, CASW_Pickup_Weapon_MedRifle)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_MedRifle::C_ASW_Pickup_Weapon_MedRifle()
{
}

//---------
// Fire Extinguisher
//---------

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Pickup_Weapon_FireExtinguisher, DT_ASW_Pickup_Weapon_FireExtinguisher, CASW_Pickup_Weapon_FireExtinguisher )
END_RECV_TABLE()

C_ASW_Pickup_Weapon_FireExtinguisher::C_ASW_Pickup_Weapon_FireExtinguisher()
{
}

//---------
// Mining Laser
//---------

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Pickup_Weapon_Mining_Laser, DT_ASW_Pickup_Weapon_Mining_Laser, CASW_Pickup_Weapon_Mining_Laser )
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Mining_Laser::C_ASW_Pickup_Weapon_Mining_Laser()
{
}

//---------
// 50CalMg
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_50CalMG, DT_ASW_Pickup_Weapon_50CalMG, CASW_Pickup_Weapon_50CalMG)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_50CalMG::C_ASW_Pickup_Weapon_50CalMG()
{
}

//---------
// Ricochet
//---------

IMPLEMENT_CLIENTCLASS_DT(C_ASW_Pickup_Weapon_Ricochet, DT_ASW_Pickup_Weapon_Ricochet, CASW_Pickup_Weapon_Ricochet)
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Ricochet::C_ASW_Pickup_Weapon_Ricochet()
{
}

//---------
// Flechette
//---------

IMPLEMENT_CLIENTCLASS_DT( C_ASW_Pickup_Weapon_Flechette, DT_ASW_Pickup_Weapon_Flechette, CASW_Pickup_Weapon_Flechette )
END_RECV_TABLE()

C_ASW_Pickup_Weapon_Flechette::C_ASW_Pickup_Weapon_Flechette()
{
}
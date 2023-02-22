#ifndef _DEFINED_C_ASW_PICKUP_WEAPON_H
#define _DEFINED_C_ASW_PICKUP_WEAPON_H

#include "c_asw_pickup.h"
#include "utldict.h"

class C_ASW_Pickup_Weapon : public C_ASW_Pickup
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon, C_ASW_Pickup );
	DECLARE_CLIENTCLASS();

	C_ASW_Pickup_Weapon();
	virtual bool AllowedToPickup(C_ASW_Inhabitable_NPC *pNPC);
	virtual bool GetUseAction(ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser);
	virtual const char* GetWeaponClass() { return "asw_weapon_rifle"; }
	virtual int GetUseIconTextureID() { return m_nUseIconTextureID; }
	virtual void GetUseIconText( wchar_t *unicode, int unicodeBufferSizeInBytes );

	virtual void InitPickup();

	CNetworkVar(int, m_iBulletsInGun);
	CNetworkVar(int, m_iClips);
	CNetworkVar(int, m_iSecondary);
	CNetworkVar(bool, m_bIsTemporaryPickup);

	int m_nUseIconTextureID;
	bool m_bWideIcon;
	bool m_bSwappingWeapon;
};

class C_ASW_Pickup_Weapon_Rifle : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Rifle, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_rifle"; }
	C_ASW_Pickup_Weapon_Rifle();
};

class C_ASW_Pickup_Weapon_PRifle : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_PRifle, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_prifle"; }
	C_ASW_Pickup_Weapon_PRifle();
};

class C_ASW_Pickup_Weapon_Autogun : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Autogun, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_autogun"; }
	C_ASW_Pickup_Weapon_Autogun();
};

class C_ASW_Pickup_Weapon_Assault_Shotgun : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Assault_Shotgun, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_vindicator"; }
	C_ASW_Pickup_Weapon_Assault_Shotgun();
};

class C_ASW_Pickup_Weapon_Pistol : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Pistol, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_pistol"; }
	C_ASW_Pickup_Weapon_Pistol();
};

class C_ASW_Pickup_Weapon_Shotgun : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Shotgun, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_shotgun"; }
	C_ASW_Pickup_Weapon_Shotgun();
};

class C_ASW_Pickup_Weapon_Tesla_Gun : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS(C_ASW_Pickup_Weapon_Tesla_Gun, C_ASW_Pickup_Weapon);
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_tesla_gun"; }
	C_ASW_Pickup_Weapon_Tesla_Gun();
};

class C_ASW_Pickup_Weapon_Railgun : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Railgun, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_railgun"; }
	C_ASW_Pickup_Weapon_Railgun();
};

class C_ASW_Pickup_Weapon_Heal_Gun : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Heal_Gun, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_heal_gun"; }
	C_ASW_Pickup_Weapon_Heal_Gun();
};

class C_ASW_Pickup_Weapon_PDW : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_PDW, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_pdw"; }
	C_ASW_Pickup_Weapon_PDW();
};

class C_ASW_Pickup_Weapon_Flamer : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Flamer, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_flamer"; }
	C_ASW_Pickup_Weapon_Flamer();
};

class C_ASW_Pickup_Weapon_Minigun : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Minigun, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_minigun"; }
	C_ASW_Pickup_Weapon_Minigun();
};

class C_ASW_Pickup_Weapon_Sniper_Rifle : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Sniper_Rifle, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_sniper_rifle"; }
	C_ASW_Pickup_Weapon_Sniper_Rifle();
};

class C_ASW_Pickup_Weapon_Chainsaw : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Chainsaw, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_chainsaw"; }
	C_ASW_Pickup_Weapon_Chainsaw();
};

class C_ASW_Pickup_Weapon_Grenade_Launcher : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS(C_ASW_Pickup_Weapon_Grenade_Launcher, C_ASW_Pickup_Weapon);
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_grenade_launcher"; }
	C_ASW_Pickup_Weapon_Grenade_Launcher();
};

class C_ASW_Pickup_Weapon_DEagle : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_DEagle, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_deagle"; }
	C_ASW_Pickup_Weapon_DEagle();
};

class C_ASW_Pickup_Weapon_Devastator : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Devastator, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_devastator"; }
	C_ASW_Pickup_Weapon_Devastator();
};

class C_ASW_Pickup_Weapon_CombatRifle : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_CombatRifle, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_combat_rifle"; }
	C_ASW_Pickup_Weapon_CombatRifle();
};

class C_ASW_Pickup_Weapon_HealAmp_Gun : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_HealAmp_Gun, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_healamp_gun"; }
	C_ASW_Pickup_Weapon_HealAmp_Gun();
};

class C_ASW_Pickup_Weapon_Heavy_Rifle : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Heavy_Rifle, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_heavy_rifle"; }
	C_ASW_Pickup_Weapon_Heavy_Rifle();
};

class C_ASW_Pickup_Weapon_MedRifle : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_MedRifle, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_medrifle"; }
	C_ASW_Pickup_Weapon_MedRifle();
};

class C_ASW_Pickup_Weapon_FireExtinguisher : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_FireExtinguisher, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_fire_extinguisher"; }
	C_ASW_Pickup_Weapon_FireExtinguisher();
};

class C_ASW_Pickup_Weapon_Mining_Laser : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Mining_Laser, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_mining_laser"; }
	C_ASW_Pickup_Weapon_Mining_Laser();
};

class C_ASW_Pickup_Weapon_50CalMG : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_50CalMG, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_50calmg"; }
	C_ASW_Pickup_Weapon_50CalMG();
};

class C_ASW_Pickup_Weapon_Ricochet : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Ricochet, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_ricochet"; }
	C_ASW_Pickup_Weapon_Ricochet();
};

class C_ASW_Pickup_Weapon_Flechette : public C_ASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( C_ASW_Pickup_Weapon_Flechette, C_ASW_Pickup_Weapon );
	DECLARE_CLIENTCLASS();

	virtual const char* GetWeaponClass() { return "asw_weapon_flechette"; }
	C_ASW_Pickup_Weapon_Flechette();
};

#endif /* _DEFINED_C_ASW_PICKUP_WEAPON_H */
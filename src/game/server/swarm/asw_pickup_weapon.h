#ifndef _DEFINED_ASW_PICKUP_WEAPON_H
#define _DEFINED_ASW_PICKUP_WEAPON_H

#include "asw_pickup.h"

class CASW_Player;
class CASW_Weapon;
class CASW_Marine;

class CASW_Pickup_Weapon : public CASW_Pickup
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon, CASW_Pickup );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	static bool VismonCallback( CBaseEntity *pPickup, CBasePlayer *pViewingPlayer );

	CASW_Pickup_Weapon();

	virtual void Spawn( void );
	virtual void ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType ); // player has used this item
	virtual void InitFrom( CASW_Marine *pMarine, CASW_Weapon *pWeapon ); // sets the pickup up with the contents of the specified marine+weapon
	virtual const char *GetWeaponClass() { return "asw_weapon_rifle"; }
	virtual void InitWeapon( CASW_Marine *pMarine, CASW_Weapon *pWeapon );
	virtual bool AllowedToPickup( CASW_Inhabitable_NPC *pNPC );

	virtual int GetNumClips() { return m_iClips; }
	virtual int GetNumBulletsInGun() { return m_iBulletsInGun; }
	virtual int GetSecondary() { return m_iSecondary; }

	CNetworkVar(int, m_iBulletsInGun);
	CNetworkVar(int, m_iClips);
	CNetworkVar(int, m_iSecondary);
	CNetworkVar(bool, m_bIsTemporaryPickup);
};

class CASW_Pickup_Weapon_Rifle : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Rifle, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_rifle"; }
};

class CASW_Pickup_Weapon_PRifle : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_PRifle, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_prifle"; }
};

class CASW_Pickup_Weapon_Autogun : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Autogun, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_autogun"; }
};

class CASW_Pickup_Weapon_Assault_Shotgun : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Assault_Shotgun, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_vindicator"; }
};

class CASW_Pickup_Weapon_Pistol : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Pistol, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_pistol"; }
};

class CASW_Pickup_Weapon_Shotgun : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Shotgun, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_shotgun"; }
};

class CASW_Pickup_Weapon_Tesla_Gun : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS(CASW_Pickup_Weapon_Tesla_Gun, CASW_Pickup_Weapon);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_tesla_gun"; }
};

class CASW_Pickup_Weapon_Railgun : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Railgun, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_railgun"; }
};

class CASW_Pickup_Weapon_Heal_Gun : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Heal_Gun, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_heal_gun"; }
};

class CASW_Pickup_Weapon_PDW : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_PDW, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_pdw"; }
};

class CASW_Pickup_Weapon_Flamer : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Flamer, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_flamer"; }
};

class CASW_Pickup_Weapon_Minigun: public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS(CASW_Pickup_Weapon_Minigun, CASW_Pickup_Weapon);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_minigun"; }
};

class CASW_Pickup_Weapon_Sniper_Rifle : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS(CASW_Pickup_Weapon_Sniper_Rifle, CASW_Pickup_Weapon);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_sniper_rifle"; }
};

class CASW_Pickup_Weapon_Chainsaw : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Chainsaw, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_chainsaw"; }
};

class CASW_Pickup_Weapon_Grenade_Launcher : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Grenade_Launcher, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_grenade_launcher"; }
};

class CASW_Pickup_Weapon_DEagle : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_DEagle, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_deagle"; }
};

class CASW_Pickup_Weapon_Devastator : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Devastator, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_devastator"; }
};

class CASW_Pickup_Weapon_CombatRifle : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_CombatRifle, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_combat_rifle"; }
};

class CASW_Pickup_Weapon_HealAmp_Gun : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_HealAmp_Gun, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_healamp_gun"; }
};

class CASW_Pickup_Weapon_Heavy_Rifle : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Heavy_Rifle, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_heavy_rifle"; }
};

class CASW_Pickup_Weapon_MedRifle : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_MedRifle, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_medrifle"; }
};

class CASW_Pickup_Weapon_FireExtinguisher : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_FireExtinguisher, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_fire_extinguisher"; }
};

class CASW_Pickup_Weapon_Mining_Laser : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Mining_Laser, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_mining_laser"; }
};

class CASW_Pickup_Weapon_50CalMG : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_50CalMG, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_50calmg"; }
};

class CASW_Pickup_Weapon_Ricochet : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Ricochet, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache(void);
	virtual void Spawn(void);
	virtual const char* GetWeaponClass() { return "asw_weapon_ricochet"; }
};

class CASW_Pickup_Weapon_Flechette : public CASW_Pickup_Weapon
{
public:
	DECLARE_CLASS( CASW_Pickup_Weapon_Flechette, CASW_Pickup_Weapon );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache( void );
	virtual void Spawn( void );
	virtual const char* GetWeaponClass() { return "asw_weapon_flechette"; }
};

#endif /* _DEFINED_ASW_PICKUP_WEAPON_H */

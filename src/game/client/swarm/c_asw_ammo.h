#ifndef _DEFINED_C_ASW_AMMO_H
#define _DEFINED_C_ASW_AMMO_H

#include "c_asw_pickup.h"

class C_ASW_Ammo : public C_ASW_Pickup
{
public:	
	DECLARE_CLASS( C_ASW_Ammo, C_ASW_Pickup );

	C_ASW_Ammo();

	virtual bool AllowedToPickup( C_ASW_Inhabitable_NPC *pNPC );

	char m_szAmmoFullText[32];
	char m_szNoGunText[32];
	int m_iAmmoIndex;
};

class C_ASW_Ammo_Rifle : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Rifle, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakeRifleAmmo; }
	C_ASW_Ammo_Rifle();

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_AMMO_RIFLE; }
};

class C_ASW_Ammo_Autogun : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Autogun, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakeAutogunAmmo; }
	C_ASW_Ammo_Autogun();

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_AMMO_AUTOGUN; }
};

class C_ASW_Ammo_Shotgun : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Shotgun, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakeShotgunAmmo; }
	C_ASW_Ammo_Shotgun();

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_AMMO_SHOTGUN; }
};

class C_ASW_Ammo_Assault_Shotgun : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Assault_Shotgun, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakeVindicatorAmmo; }
	C_ASW_Ammo_Assault_Shotgun();

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_AMMO_ASSAULT_SHOTGUN; }
};

class C_ASW_Ammo_Flamer : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Flamer, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakeFlamerAmmo; }
	C_ASW_Ammo_Flamer();

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_AMMO_FLAMER; }
};

class C_ASW_Ammo_Pistol : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Pistol, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakePistolAmmo; }
	C_ASW_Ammo_Pistol();

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_AMMO_PISTOL; }
};

class C_ASW_Ammo_Mining_Laser : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Mining_Laser, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakeMiningLaserAmmo; }
	C_ASW_Ammo_Mining_Laser();

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_AMMO_MINING_LASER; }
};

class C_ASW_Ammo_Railgun : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Railgun, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakeRailgunAmmo; }
	C_ASW_Ammo_Railgun();

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_AMMO_RAILGUN; }
};

class C_ASW_Ammo_Chainsaw : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Chainsaw, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTake; }
	C_ASW_Ammo_Chainsaw();

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_AMMO_CHAINSAW; }
};

class C_ASW_Ammo_PDW : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_PDW, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakePDWAmmo; }
	C_ASW_Ammo_PDW();

	// Classification
	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_AMMO_PDW; }
};

class C_ASW_Ammo_AR2 : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_AR2, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakeRifleAmmo; }
	C_ASW_Ammo_AR2();

	// Classification
	virtual Class_T		Classify( void ) { return ( Class_T )CLASS_ASW_AMMO_AR2; }
};

class C_ASW_Ammo_Grenade_Launcher : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Grenade_Launcher, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTake; }
	C_ASW_Ammo_Grenade_Launcher();

	// Classification
	virtual Class_T		Classify( void ) { return ( Class_T )CLASS_ASW_AMMO_GRENADE_LAUNCHER; }
};

class C_ASW_Ammo_Sniper_Rifle : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Sniper_Rifle, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakeRifleAmmo; }
	C_ASW_Ammo_Sniper_Rifle();

	// Classification
	virtual Class_T		Classify( void ) { return ( Class_T )CLASS_ASW_AMMO_SNIPER_RIFLE; }
};

class C_ASW_Ammo_Heavy_Rifle : public C_ASW_Ammo
{
public:
	DECLARE_CLASS( C_ASW_Ammo_Heavy_Rifle, C_ASW_Ammo );
	DECLARE_CLIENTCLASS();

	virtual int GetUseIconTextureID() { BaseClass::GetUseIconTextureID(); return s_nUseIconTakeRifleAmmo; }
	C_ASW_Ammo_Heavy_Rifle();

	// Classification
	virtual Class_T		Classify( void ) { return ( Class_T )CLASS_ASW_AMMO_HEAVY_RIFLE; }
};

#endif /* _DEFINED_C_ASW_AMMO_H */
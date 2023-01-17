#ifndef ASW_WEAPON_50CALMG_SHARED_H__
#define ASW_WEAPON_50CALMG_SHARED_H__

#ifdef GAME_DLL
	#include "asw_weapon_rifle.h"
#else // CLIENT_DLL
	#include "c_asw_weapon_rifle.h"
	#define CASW_Weapon_50CalMG C_ASW_Weapon_50CalMG
	#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
#endif // GAME_DLL

// Overpowered gun to be placed on maps. Can't be picked in lobby, 
// cannot pick up ammo. 
class CASW_Weapon_50CalMG : public CASW_Weapon_Rifle
{
public:
	DECLARE_CLASS(CASW_Weapon_50CalMG, CASW_Weapon_Rifle);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_50CalMG();
	virtual ~CASW_Weapon_50CalMG();
	virtual void Precache();

	virtual void SecondaryAttack();

	virtual const Vector& GetAngularBulletSpread()
	{
		static Vector cone(60, 60, 10);
		return cone;
	}

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone( 0.17365, 0.17365, 0.02 ); // VECTOR_CONE_20DEGREES with flattened Z
		return cone;
	}

	virtual float GetWeaponDamage();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

#else 
	virtual bool HasSecondaryExplosive(void) const { return false; }
	virtual bool GroundSecondary() { return false; }
#endif

	virtual Class_T		Classify(void) { return (Class_T)CLASS_ASW_50CALMG; }
};


#endif // ASW_WEAPON_50CALMG_SHARED_H__

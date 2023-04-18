#ifndef asw_weapon_combat_rifle_shared_h__
#define asw_weapon_combat_rifle_shared_h__

#ifdef GAME_DLL
	#include "asw_weapon_rifle.h"
#else // CLIENT_DLL
	#include "c_asw_weapon_rifle.h"
	#define CASW_Weapon_CombatRifle C_ASW_Weapon_CombatRifle
	#define CASW_Weapon_Rifle C_ASW_Weapon_Rifle
#endif // GAME_DLL

#include "basegrenade_shared.h"

class CASW_Weapon_CombatRifle : public CASW_Weapon_Rifle
{
public:
	DECLARE_CLASS(CASW_Weapon_CombatRifle, CASW_Weapon_Rifle);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_CombatRifle();
	virtual ~CASW_Weapon_CombatRifle();
	virtual void Precache();

	virtual void SecondaryAttack();

	virtual const Vector& GetAngularBulletSpread()
	{
		static Vector cone(60, 60, 10);
		return cone;
	}

	virtual float GetWeaponDamage();
	//virtual void PrimaryAttack();
	//virtual void ItemPostFrame();

	//virtual void FireBullets(CASW_Marine *pMarine, FireBulletsInfo_t *pBulletsInfo);

	//virtual float GetMovementScale();

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#else 
	virtual bool HasSecondaryExplosive(void) const { return false; }
	virtual bool GroundSecondary() { return false; }
#endif
	virtual bool HasBuckshotSecondaryAttack() { return true; }
	virtual const char *GetMagazineGibModelName() const override { return "models/weapons/empty_clips/combatrifle_empty_clip.mdl"; }

	virtual Class_T		Classify(void) { return (Class_T)CLASS_ASW_COMBAT_RIFLE; }
};


#endif // asw_weapon_combat_rifle_shared_h__

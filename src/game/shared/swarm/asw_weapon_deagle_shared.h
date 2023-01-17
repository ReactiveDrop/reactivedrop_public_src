#ifndef asw_weapon_deagle_h__
#define asw_weapon_deagle_h__

#include "asw_weapon_pistol_shared.h"

#ifdef CLIENT_DLL
	#define CASW_Weapon_DEagle C_ASW_Weapon_DEagle
#endif

class CASW_Weapon_DEagle : public CASW_Weapon_Pistol
{
public:
	DECLARE_CLASS( CASW_Weapon_DEagle, CASW_Weapon_Pistol );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CASW_Weapon_DEagle();
	virtual ~CASW_Weapon_DEagle();
	virtual void Precache();

	virtual void PrimaryAttack();
	virtual void ItemPostFrame();
	virtual void ItemBusyFrame();
	virtual float GetWeaponDamage();
	virtual float GetFireRate(void);

	virtual void FireBullets(CASW_Marine *pMarine, FireBulletsInfo_t *pBulletsInfo);

	virtual const Vector& GetBulletSpread( void );

	int ASW_SelectWeaponActivity(int idealActivity);

#ifndef CLIENT_DLL
	DECLARE_DATADESC();
#endif

	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_DEAGLE; }
protected:
	//CNetworkVar( bool, m_bCanShoot);	// marine moves slow until this moment
	bool m_bCanShoot;
};

#endif // asw_weapon_deagle_h__


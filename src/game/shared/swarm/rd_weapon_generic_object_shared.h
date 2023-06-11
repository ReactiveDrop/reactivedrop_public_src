#pragma once

#ifdef CLIENT_DLL
#include "c_asw_weapon.h"
#define CRD_Weapon_Generic_Object C_RD_Weapon_Generic_Object
#define CASW_Weapon C_ASW_Weapon
#else
#include "asw_weapon.h"
#endif

#include "asw_shareddefs.h"

class CRD_Weapon_Generic_Object : public CASW_Weapon
{
public:
	DECLARE_CLASS( CRD_Weapon_Generic_Object, CASW_Weapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CRD_Weapon_Generic_Object();
	~CRD_Weapon_Generic_Object();

#ifdef GAME_DLL
	DECLARE_DATADESC();
	virtual bool KeyValue( const char *szKeyName, const char *szValue );
	virtual void Spawn();
	virtual void MarineDropped( CASW_Marine *pMarine );

	CNetworkVar( string_t, m_iOriginalName );

	COutputEvent m_OnPrimaryAttack;
	COutputEvent m_OnSecondaryAttack;
	COutputEvent m_OnReload;
#endif

	virtual Class_T Classify() override { return (Class_T)CLASS_RD_WEAPON_GENERIC_OBJECT; }
	virtual void Precache() override;
	virtual void PrimaryAttack() override;
	virtual void SecondaryAttack() override;
	virtual bool Reload() override;
	virtual void Equip( CBaseCombatCharacter *pOwner ) override;
	virtual void Drop( const Vector & vecVelocity ) override;
	virtual bool ShouldMarineMoveSlow() override;
	virtual float GetMovementScale() override;
	virtual bool IsOffensiveWeapon() override;
	virtual int ASW_SelectWeaponActivity( int idealActivity ) override;
	virtual const char *GetViewModel( int viewmodelindex = 0 ) const override { return m_szWorldModel; }
	virtual const char *GetWorldModel() const override { return m_szWorldModel; }
	virtual const char *GetPrintName() const override { return m_szCarriedName; }
	int GetMaxClip1() const override { return WEAPON_NOCLIP; }
	int GetMaxClip2() const override { return WEAPON_NOCLIP; }
	int GetDefaultClip1() const override { return 1; }
	int GetDefaultClip2() const override { return 1; }

	CNetworkVar( float, m_flMoveSpeedMultiplier );
	CNetworkVar( bool, m_bLargeObject );
	CNetworkVar( bool, m_bUseBoneMerge );
	CNetworkQAngle( m_angCarriedAngle );
	CNetworkVector( m_vecCarriedOffset );
	CNetworkString( m_szWorldModel, 256 );
	CNetworkString( m_szCarriedName, 256 );
	CNetworkString( m_szEquipIcon, 256 );

#ifdef CLIENT_DLL
	wchar_t m_wszCarriedName[256];
	char m_iOriginalName[MAX_PATH];
	int m_iEquipIcon;
	virtual void PostDataUpdate( DataUpdateType_t updateType ) override;
	virtual bool GetUseAction( ASWUseAction & action, C_ASW_Inhabitable_NPC *pUser ) override;
	virtual int GetUseIconTextureID() override;
#endif
};

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
#endif

	virtual Class_T Classify() { return (Class_T)CLASS_RD_WEAPON_GENERIC_OBJECT; }
	virtual void Precache();
	virtual void PrimaryAttack() { }
	virtual void Equip( CBaseCombatCharacter *pOwner );
	virtual void Drop( const Vector & vecVelocity );
	virtual bool ShouldMarineMoveSlow();
	virtual float GetMovementScale();
	virtual bool IsOffensiveWeapon();
	virtual int ASW_SelectWeaponActivity( int idealActivity );
	virtual const char *GetViewModel( int viewmodelindex = 0 ) const { return m_szWorldModel; }
	virtual const char *GetWorldModel() const { return m_szWorldModel; }

	CNetworkVar( float, m_flMoveSpeedMultiplier );
	CNetworkVar( bool, m_bLargeObject );
	CNetworkVar( bool, m_bUseBoneMerge );
	CNetworkQAngle( m_angCarriedAngle );
	CNetworkVector( m_vecCarriedOffset );
	CNetworkString( m_szWorldModel, 256 );
	CNetworkString( m_szCarriedName, 256 );

#ifdef CLIENT_DLL
	wchar_t m_wszCarriedName[256];
	char m_iOriginalName[MAX_PATH];
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual bool GetUseAction( ASWUseAction & action, C_ASW_Inhabitable_NPC *pUser );
#endif
};

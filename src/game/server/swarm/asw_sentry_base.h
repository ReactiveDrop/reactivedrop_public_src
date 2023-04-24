#ifndef ASW_SENTRY_BASE_H
#define ASW_SENTRY_BASE_H
#pragma once

#include "asw_shareddefs.h"
#include "iasw_server_usable_entity.h"

class CASW_Player;
class CASW_Marine;
class CASW_Sentry_Top;

class CASW_Sentry_Base : public CBaseAnimating, public IASW_Server_Usable_Entity
{
public:
	DECLARE_CLASS( CASW_Sentry_Base, CBaseAnimating );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_ENT_SCRIPTDESC();

	virtual void Precache();

	CASW_Sentry_Base();
	virtual ~CASW_Sentry_Base();
	EHANDLE m_hBait[4];
	void SetBait( CBaseEntity *pBait1, CBaseEntity *pBait2, CBaseEntity *pBait3, CBaseEntity *pBait4 )
	{
		m_hBait[0] = pBait1;
		m_hBait[1] = pBait2;
		m_hBait[2] = pBait3;
		m_hBait[3] = pBait4;
	}
	void	Spawn( void );
	void	AnimThink( void );
	virtual int ShouldTransmit( const CCheckTransmitInfo *pInfo );
	int UpdateTransmitState();	
	
	void PlayDeploySound();
	CASW_Sentry_Top* GetSentryTop();
	HSCRIPT ScriptGetSentryTop();
	CNetworkHandle( CASW_Sentry_Top, m_hSentryTop );
	CNetworkHandle( CASW_Marine, m_hDeployer );
	CNetworkVar( bool, m_bAssembled );
	CNetworkVar( bool, m_bIsInUse );
	CNetworkVar( float, m_fAssembleProgress );
	CNetworkVar( float, m_fAssembleCompleteTime );
	CNetworkVar( int, m_iAmmo );
	CNetworkVar( int, m_iMaxAmmo );
	CNetworkVar( bool, m_bSkillMarineHelping );
	float m_fSkillMarineHelping;
	float m_fDamageScale;
	bool m_bAlreadyTaken;

	void OnFiredShots( int nNumShots = 1 );
	inline int GetAmmo() { return m_iAmmo; }
	void SetAmmo( int nAmmo ) { m_iAmmo = nAmmo; }
	int ScriptGetMaxAmmo();

	// IASW_Server_Usable_Entity implementation
	virtual CBaseEntity *GetEntity() { return this; }
	virtual bool IsUsable( CBaseEntity *pUser );
	virtual bool RequirementsMet( CBaseEntity *pUser ) { return true; }
	virtual void ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType );
	virtual void NPCUsing( CASW_Inhabitable_NPC *pNPC, float deltatime );
	virtual void NPCStartedUsing( CASW_Inhabitable_NPC *pNPC );
	virtual void NPCStoppedUsing( CASW_Inhabitable_NPC *pNPC );
	virtual bool NeedsLOSCheck() { return true; }

	virtual int OnTakeDamage( const CTakeDamageInfo &info );
	virtual void Event_Killed( const CTakeDamageInfo &info );

	virtual Class_T		Classify( void ) { return (Class_T) CLASS_ASW_SENTRY_BASE; }

	/// Here are the different types of sentry gun we support.
	/// This array must match the entries in sm_gunTypeToEntityName
	enum GunType_t
	{
		kAUTOGUN = 0,
		kCANNON,
		kFLAME,
		kICE,
		// not a valid gun type:
		kGUNTYPE_MAX
	};

	static const char *GetEntityNameForGunType( GunType_t guntype );
	static const char *GetWeaponNameForGunType( GunType_t guntype );
	static int GetBaseAmmoForGunType( GunType_t guntype );
	GunType_t GetGunType( void ) const;
	inline void SetGunType( GunType_t iType );
	inline void SetGunType( int iType );

	CNetworkHandle( CASW_Player, m_hOriginalOwnerPlayer );
	CNetworkVar( int, m_iInventoryEquipSlot );
	// sentry item validity was checked when the sentry was deployed.
	// there's a chance players could do some convoluted process where
	// they partially upload dynamic item data before mission start
	// and finish uploading it after the sentry is placed, but they
	// can also just write their own code that increments stats directly,
	// so this is not something we should spend resources trying to prevent.
	bool IsInventoryEquipSlotValid() const { return !!m_hOriginalOwnerPlayer && m_iInventoryEquipSlot != -1; }

protected:
	
	CNetworkVar( int, m_nGunType );

	// specifications for info about the sentry gun types
	struct SentryGunTypeInfo_t 
	{
		SentryGunTypeInfo_t( const char *entname, const char *weaponname, const int baseammo ) :
			m_entityName(entname), m_weaponName(weaponname), m_nBaseAmmo(baseammo) {};

		const char *m_entityName; ///< name of sentry top entity
		const char *m_weaponName; ///< name of the weapon class that deployed this sentry
		int m_nBaseAmmo; ///< ammo to start with
	};

	static const SentryGunTypeInfo_t sm_gunTypeToInfo[kGUNTYPE_MAX];
};

inline void CASW_Sentry_Base::SetGunType( int iType )
{
	Assert( iType >= 0 && iType < kGUNTYPE_MAX );
	SetGunType( (GunType_t) iType );
}

inline void CASW_Sentry_Base::SetGunType( CASW_Sentry_Base::GunType_t iType )
{
	m_nGunType = iType;
}

#endif /* ASW_SENTRY_BASE_H */
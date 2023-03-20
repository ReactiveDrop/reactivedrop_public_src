#ifndef _INCLUDED_C_ASW_WEAPON_H
#define _INCLUDED_C_ASW_WEAPON_H
#ifdef _WIN32
#pragma once
#endif

#include "asw_weapon_shared.h"
#include "iasw_client_usable_entity.h"
#include "basecombatweapon_shared.h"
#include "glow_outline_effect.h"
#include "rd_inventory_shared.h"

class C_ASW_Player;
class C_ASW_Marine;
class CASW_WeaponInfo;
class C_BreakableProp;
class CASW_EquipItem;

extern ConVar rd_ground_shooting;

class C_ASW_Weapon : public C_BaseCombatWeapon, public IASW_Client_Usable_Entity
{
	DECLARE_CLASS( C_ASW_Weapon, C_BaseCombatWeapon );
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_ENT_SCRIPTDESC();

					C_ASW_Weapon();
	virtual			~C_ASW_Weapon();
	virtual void Precache();
	virtual void UpdateOnRemove();
	virtual void Spawn();
	virtual void ItemPostFrame( void );
	virtual void ItemBusyFrame(void);
	virtual void ClientPostFrame() { } // in singleplayer the client_dll doesn't run a standard itempostframe on the weapon, some items need this though, so use this function
	virtual void OnDataChanged( DataUpdateType_t type );
	virtual bool ShouldPredict( void );
	virtual C_BasePlayer* GetPredictionOwner( void );
	virtual void CreateBayonet();
	virtual bool SupportsBayonet();
	virtual void CreateLaserSight();
	CHandle<C_BreakableProp> m_hBayonet;
	EHANDLE m_hLaserSight;

	virtual int DrawModel( int flags, const RenderableInstance_t &instance );
	virtual IClientModelRenderable*	GetClientModelRenderable();

	void DiffPrint(  char const *fmt, ... );

	// effects
	virtual int		GetMuzzleAttachment( void );
	virtual void	SetMuzzleAttachment( int nNewAttachment );
	virtual float	GetMuzzleFlashScale( void );
	virtual Vector	GetMuzzleFlashTint();
	virtual void	ProcessMuzzleFlashEvent();
	virtual int		LookupAttachment( const char *pAttachmentName );
	virtual const char* GetUTracerType();
	virtual const char* GetTracerEffectName() { return "tracer_default"; }	// particle effect name
	virtual const char* GetMuzzleEffectName() { return "muzzle_rifle"; }	// particle effect name
	virtual const char *GetMagazineGibModelName() const { return NULL; }
	virtual int GetMagazineGibModelSkin() const { return 0; }
	virtual void DropMagazineGib();
	virtual bool Simulate();
	virtual bool ShouldMarineFlame() { return false; } // if true, the marine emits flames from his flame emitter
	virtual bool ShouldMarineFireExtinguish() { return false; } // is true, the marine emits fire extinguisher stuff from his emitter
	virtual bool ShouldMarineMinigunShoot() { return false; } // if true, the marine shoots from minigun
	virtual void OnMuzzleFlashed() { }	// called when the weapon muzzle flashes
	virtual bool HasMuzzleFlashSmoke() { return false; }	
	virtual void ASWReloadSound(int iType);	// play one of the 3 part reload sounds
	virtual const char* GetPartialReloadSound(int iPart);
	virtual int GetChargesForHUD() { return Clip1(); }
	virtual float GetBatteryCharge() { return -1.0f; }			// for weapons which want a battery charge display on the HUD
	virtual float GetMinBatteryChargeToActivate() { return -1.0f; }
	virtual bool PrefersFlatAiming() const { return false; }
	virtual void ClientThink();
	float m_fLastMuzzleFlashTime;	
	float m_fMuzzleFlashScale;

	virtual bool HasSecondaryExplosive( void ) const { return false; }

	virtual bool			IsCarriedByLocalPlayer( void );
	virtual bool			IsActiveByLocalPlayer( void );
	virtual ShadowType_t			ShadowCastType();

	bool ShouldDraw();

	virtual void MouseOverEntity(C_BaseEntity* pEnt, Vector vecWorldCursor) { }

	// shared code
	virtual void PrimaryAttack( void );
	virtual void SecondaryAttack();
	virtual bool SecondaryAttackUsesPrimaryAmmo() { return false; }
	virtual bool SecondaryAttackEqualsPrimary() { return false; }
	virtual bool ReloadOrSwitchWeapons( void );
	virtual const char *GetASWShootSound( int iIndex, int &iPitch );
	virtual void WeaponSound( WeaponSound_t sound_type, float soundtime = 0.0f );
	virtual bool HasAmmo();
	virtual bool PrimaryAmmoLoaded( void );
	virtual float GetReloadTime();
	virtual float GetReloadFailTime();
	virtual bool Reload();
	virtual bool ASWReload( int iClipSize1, int iClipSize2, int iActivity );
	virtual void SendReloadEvents();
	bool IsReloading() const;
	virtual void FinishReload( void );
	virtual float	GetFireRate( void );
	bool IsFiring();// const;
	virtual void GetButtons(bool& bAttack1, bool& bAttack2, bool& bReload, bool& bOldReload, bool& bOldAttack1 );
	virtual bool ShouldMarineMoveSlow();
	virtual float GetMovementScale();			// get overall current movement scale (incorporates slowdown from heavy weapons, reloading, firing)
	virtual float GetTurnRateModifier();
	C_ASW_Player* GetCommander();
	C_ASW_Marine* GetMarine();
	virtual bool IsPredicted( void ) const;
	virtual bool ViewModelIsMarineAttachment() const { return false; }
	bool Holster( CBaseCombatWeapon *pSwitchingTo );
	virtual void Equip(CBaseCombatCharacter *pOwner);
	virtual void SetWeaponVisible( bool visible );
	virtual void ApplyWeaponSwitchTime(float fSwitchDelay);
	bool m_bSwitchingWeapons;
	virtual void SendWeaponSwitchEvents();
	virtual const float GetAutoAimAmount();
	virtual const float GetVerticalAdjustOnlyAutoAimAmount();
	virtual const float GetAutoAimRadiusScale() { return 1.0f; }
	virtual bool ASWCanBeSelected() { return true; }
	virtual bool ShouldFlareAutoaim() { return false; }
	virtual int GetEquipmentListIndex() { return m_iEquipmentListIndex; }
	const CASW_EquipItem *GetEquipItem() const;
	const CASW_WeaponInfo *GetWeaponInfo() const;
	virtual bool OffhandActivate() { return false; };
	virtual bool WantsOffhandPostFrame() { return m_bShotDelayed; }
	virtual bool SendWeaponAnim( int iActivity );
	virtual void CalcBoneMerge( CStudioHdr *hdr, int boneMask, CBoneBitList &boneComputed );
	virtual int ASW_SelectWeaponActivity(int idealActivity);
	virtual float GetWeaponPvpDamageBase();
	virtual int GetWeaponSkillId();
	virtual int GetWeaponSubSkillId();
	virtual float GetWeaponDamage();
	virtual bool AllowsAutoSwitchFrom( void ) const;
	virtual int AmmoClickPoint(); // when this many rounds or less left in the gun, it will click when firing
	virtual void LowAmmoSound();
	virtual bool IsOffensiveWeapon() { return true; }		// is this weapon an offensive gun type weapon (as opposed to a utility item)
	virtual bool DisplayClipsDoubled() { return false; }    // dual weilded guns should show ammo doubled up to complete the illusion of holding two guns
	virtual bool CanDoForcedAction( int iForcedAction ) { return true; }		// check if we're allowed to perform a forced action (certain abilities limit this)
	void PlaySoundDirectlyToOwner( const char *szSoundName );
	void PlaySoundToOthers( const char *szSoundName );
	virtual float GetPassiveMeleeDamageScale() { return 1.0f; }

	virtual void OnStoppedFiring();
	virtual void OnStartedRoll();
	virtual void ClearIsFiring();
	bool m_bIsFiring;
	float m_fTurnRateModifier;
	float m_fFiringTurnRateModifier;
	float m_fReloadClearFiringTime;

	// delayed firing (flares, hand grenades, etc.)
	virtual void DelayedAttack() { }
	virtual float GetThrowGravity() { return 1.0f; }
	bool m_bShotDelayed;
	float m_flDelayedFire;
	bool m_bOldHidden;

	// powerups
	void MakePoweredUp( bool bPoweredUp ) { m_bPoweredUp = bPoweredUp; } 
	bool m_bPoweredUp;

	// fast reload
	float m_fReloadStart;
	float m_fReloadProgress;
	float m_fFastReloadStart;
	float m_fFastReloadEnd;
	bool m_bFastReloadSuccess;
	bool m_bFastReloadFailure;
	float m_flReloadFailTime;			// time from when you fail an active reload until you can shoot again
	// attachments
	int m_nMuzzleAttachment;
	int m_nLastMuzzleAttachment;
	// body groups
	int m_nMagazineBodyGroup;
	int m_nLaserBodyGroup;

	// ground shooting (aiming at the ground)
	virtual bool SupportsGroundShooting() { return rd_ground_shooting.GetBool(); } // was false

	// Functions for weapons on the ground
	virtual bool AllowedToPickup( C_ASW_Inhabitable_NPC *pNPC );
	bool m_bSwappingWeapon;
	bool m_bIsTemporaryPickup;

	// check if this weapon wants to perform a sync kill
	virtual bool CheckSyncKill( byte &forced_action, short &sync_kill_ent ) { return false; }

	// IASW_Client_Usable_Entity
	virtual C_BaseEntity* GetEntity() { return this; }
	virtual bool IsUsable( C_BaseEntity *pUser );
	virtual bool GetUseAction( ASWUseAction &action, C_ASW_Inhabitable_NPC *pUser );
	virtual void CustomPaint(int ix, int iy, int alpha, vgui::Panel *pUseIcon) { }
	virtual bool ShouldPaintBoxAround() { return true; }
	virtual bool NeedsLOSCheck() { return true; }

	virtual int GetUseIconTextureID();
	int m_nUseIconTextureID;

	bool m_bWeaponCreated;

	// laser pointer
	virtual bool ShouldShowLaserPointer();
	virtual bool ShouldAlignWeaponToLaserPointer();
	virtual const char *GetLaserPointerEffectName() const { return "weapon_laser_sight"; }
	void SimulateLaserPointer();
	void CreateLaserPointerEffect( bool bLocalPlayer, int iAttachment );
	void RemoveLaserPointerEffect();
	CUtlReference<CNewParticleEffect> m_pLaserPointerEffect;
	virtual float GetLaserPointerRange( void ) { return 600; }
	void SetLaserTargetEntity( C_BaseEntity* pEnt ) { m_hLaserTargetEntity = pEnt; }
	C_BaseEntity* GetLaserTargetEntity() { return m_hLaserTargetEntity.Get(); }
	bool m_bLocalPlayerControlling;
	EHANDLE m_hLaserTargetEntity;
	Vector m_vecLaserPointerDirection;

	// muzzle flash
	void CreateMuzzleFlashEffect( int iAttachment );
	void RemoveMuzzleFlashEffect();
	CUtlReference<CNewParticleEffect> m_pMuzzleFlashEffect;
	float m_fMuzzleFlashTime;

	// returns true if secondary attack should aim at ground where
	// mouse pointer points at 
	virtual bool GroundSecondary();

	virtual const char *GetPrintName() const override;
	virtual int GetMaxClip1() const override;
	virtual int GetMaxClip2() const override;
	virtual int GetDefaultClip1() const override;
	virtual int GetDefaultClip2() const override;

	virtual int DisplayClip1() { return Clip1(); }
	virtual int DisplayMaxClip1() { return GetMaxClip1(); }
	virtual int DisplayClip2() { return Clip2(); }
	virtual int DisplayMaxClip2() { return GetMaxClip2(); }

	CNetworkVar( AccountID_t, m_iOriginalOwnerSteamAccount );
	CNetworkVarEmbedded( CRD_ItemInstance, m_InventoryItemData );

private:	
	C_ASW_Weapon( const C_ASW_Weapon & ); // not defined, not accessible

protected:
	int m_iEquipmentListIndex;
	CASW_EquipItem *m_pEquipItem;

	CGlowObject m_GlowObject;
};

class C_RD_Weapon_Accessory : public C_BaseAnimating
{
	DECLARE_CLASS( C_RD_Weapon_Accessory, C_BaseAnimating );
public:
	int m_iAccessoryIndex;
};

#endif /* _INCLUDED_C_ASW_WEAPON_H */
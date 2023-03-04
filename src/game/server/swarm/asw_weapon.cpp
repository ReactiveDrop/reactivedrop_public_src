#include "cbase.h"
#include "asw_weapon.h"
#include "asw_marine.h"
#include "asw_player.h"
#include "asw_marine_skills.h"
#include "asw_marine_profile.h"
#include "asw_weapon_parse.h"
#include "util_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( asw_weapon, CASW_Weapon );

static void *SendProxy_WeaponItemDataForOwningPlayer( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
{
	static CRD_ItemInstance s_BlankInstance;

	const CASW_Weapon *pWeapon = static_cast< const CASW_Weapon * >( pStructBase );
	CASW_Player *pPlayer = pWeapon->m_hOriginalOwnerPlayer;
	if ( !pPlayer || pWeapon->m_iInventoryEquipSlotIndex == -1 )
		return &s_BlankInstance;

	return &pPlayer->m_EquippedItemData[pWeapon->m_iInventoryEquipSlotIndex];
}

BEGIN_NETWORK_TABLE_NOBASE( CASW_Weapon, DT_ASWLocalWeaponData )
	SendPropIntWithMinusOneFlag( SENDINFO(m_iClip2 ), 8 ),
	SendPropInt( SENDINFO(m_iSecondaryAmmoType ), 8 ),

	SendPropFloat(SENDINFO(m_fReloadStart)),
	SendPropFloat(SENDINFO(m_fFastReloadStart)),
	SendPropFloat(SENDINFO(m_fFastReloadEnd)),
END_NETWORK_TABLE()

BEGIN_NETWORK_TABLE_NOBASE( CASW_Weapon, DT_ASWActiveLocalWeaponData )
	SendPropTime( SENDINFO( m_flNextPrimaryAttack ) ),
	SendPropTime( SENDINFO( m_flNextSecondaryAttack ) ),
	SendPropInt( SENDINFO( m_nNextThinkTick ) ),
	SendPropTime( SENDINFO( m_flTimeWeaponIdle ) ),
END_NETWORK_TABLE()

IMPLEMENT_SERVERCLASS_ST(CASW_Weapon, DT_ASW_Weapon)
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),	
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),
	SendPropBool(SENDINFO(m_bIsFiring)),
	SendPropBool(SENDINFO(m_bInReload)),
	SendPropBool(SENDINFO(m_bSwitchingWeapons)),
	SendPropExclude( "DT_BaseCombatWeapon", "LocalWeaponData" ),
	SendPropExclude( "DT_BaseCombatWeapon", "LocalActiveWeaponData" ),
	// BenLubar: both of these tables are required for the fast reload bar to work properly,
	// so we have to send them to all players no matter what.
	SendPropDataTable("ASWLocalWeaponData", 0, &REFERENCE_SEND_TABLE(DT_ASWLocalWeaponData) ),
	SendPropDataTable("ASWActiveLocalWeaponData", 0, &REFERENCE_SEND_TABLE(DT_ASWActiveLocalWeaponData) ),
	SendPropBool(SENDINFO(m_bFastReloadSuccess)),
	SendPropBool(SENDINFO(m_bFastReloadFailure)),
	SendPropBool(SENDINFO(m_bPoweredUp)),
	SendPropIntWithMinusOneFlag( SENDINFO(m_iClip1 ), 8 ),
	SendPropInt( SENDINFO(m_iPrimaryAmmoType ), 8 ),
	SendPropBool(SENDINFO(m_bIsTemporaryPickup)),
	SendPropInt( SENDINFO( m_iOriginalOwnerSteamAccount ), -1, SPROP_UNSIGNED ),
	SendPropDataTable( "m_InventoryItemData", 0, &REFERENCE_SEND_TABLE( DT_RD_ItemInstance ), SendProxy_WeaponItemDataForOwningPlayer ),
END_SEND_TABLE()

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CASW_Weapon )
	DEFINE_FIELD(m_iEquipmentListIndex, FIELD_INTEGER),
	DEFINE_FIELD( m_bShotDelayed,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flDelayedFire,	FIELD_TIME ),	
	DEFINE_FIELD( m_fReloadStart, FIELD_TIME ),
	DEFINE_FIELD( m_fFastReloadStart, FIELD_TIME ),
	DEFINE_FIELD( m_fFastReloadEnd, FIELD_TIME ),
	DEFINE_KEYFIELD( m_bIsTemporaryPickup, FIELD_BOOLEAN, "IsTemporaryPickup" ),
END_DATADESC()

BEGIN_ENT_SCRIPTDESC( CASW_Weapon, CBaseCombatWeapon, "Alien Swarm weapon" )
END_SCRIPTDESC()

ConVar asw_weapon_safety_hull("asw_weapon_safety_hull", "0", FCVAR_CHEAT, "Size of hull used to check for AI shots going too near a friendly");
extern ConVar asw_debug_alien_damage;
extern ConVar rd_server_marine_backpacks;

CASW_Weapon::CASW_Weapon()
{
	SetPredictionEligible(true);
	m_iEquipmentListIndex = -1;

	m_bSwitchingWeapons = false;

	m_fMinRange1	= 0;
	m_fMaxRange1	= 512;
	m_fMinRange2	= 0;
	m_fMaxRange2	= 512;
	
	m_bShotDelayed = 0;
	m_flDelayedFire = 0;
	m_fReloadClearFiringTime = 0;
	m_flReloadFailTime = 1.0;
	m_bFastReloadSuccess = false;
	m_bFastReloadFailure = false;

	m_bPoweredUp = false;
	m_bIsTemporaryPickup = false;
	m_iOriginalOwnerSteamAccount = 0;
	m_hOriginalOwnerPlayer = NULL;
}


CASW_Weapon::~CASW_Weapon()
{

}

int CASW_Weapon::WeaponRangeAttack1Condition( float flDot, float flDist )
{
	if (!IsOffensiveWeapon())
		return	COND_NO_WEAPON;	// make sure the AI doesn't try to attack with this

 	if ( UsesPrimaryAmmo() && !HasPrimaryAmmo() )
 	{
 		return COND_NO_PRIMARY_AMMO;
 	}
 	else if ( flDist < m_fMinRange1) 
 	{
 		return COND_TOO_CLOSE_TO_ATTACK;
 	}
 	else if (flDist > m_fMaxRange1) 
 	{
 		return COND_TOO_FAR_TO_ATTACK;
 	}

 	return COND_CAN_RANGE_ATTACK1;
}

void CASW_Weapon::MarineDropped(CASW_Marine* pMarine)
{
	ClearIsFiring();
}

int	CASW_Weapon::UpdateTransmitState()
{	
	return SetTransmitState( FL_EDICT_ALWAYS );
}

int CASW_Weapon::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	return FL_EDICT_ALWAYS;
}

bool CASW_Weapon::ShouldAlienFlinch(CBaseEntity *pAlien, const CTakeDamageInfo &info)
{
	const CASW_WeaponInfo* pWpnInfo = GetWeaponInfo();
	if (!pWpnInfo)
		return false;
	float fFlinchChance = pWpnInfo->m_fFlinchChance;
	if (asw_debug_alien_damage.GetBool())
		Msg("BaseFlinch chance %f ", fFlinchChance);

	CBaseCombatCharacter* pOwner = GetOwner();
	if ( pOwner && pOwner->Classify() == CLASS_ASW_MARINE )
	{
		CASW_Marine* pMarine = assert_cast<CASW_Marine*>( pOwner );
		CASW_Marine_Profile* pMarineProfile = pMarine->GetMarineProfile();
		if ( pMarineProfile && pMarineProfile->GetMarineClass() == MARINE_CLASS_SPECIAL_WEAPONS )
		{
			// this is a special weapons marine, so we need to add our flinch bonus onto it
			fFlinchChance += pWpnInfo->m_fStoppingPowerFlinchBonus * MarineSkills()->GetSkillBasedValueByMarine(pMarine, ASW_MARINE_SKILL_STOPPING_POWER);
			if (asw_debug_alien_damage.GetBool())
				Msg("Boosted by specialweaps to %f ", fFlinchChance);
		}
	}

	//CALL_ATTRIB_HOOK_FLOAT( fFlinchChance, mod_stopping );

	if (pAlien)
	{
		int iHealth = pAlien->GetHealth();
		int iDamage = info.GetDamage();
		float fAlienHealth = float(iHealth + iDamage) / float(pAlien->GetMaxHealth());
		fFlinchChance *= fAlienHealth;
		if (asw_debug_alien_damage.GetBool())
			Msg("adjusted by alien health (%f) to %f ", fAlienHealth, fFlinchChance);
	}

	float f = random->RandomFloat();
	bool bResult = ( f < fFlinchChance);
	if (asw_debug_alien_damage.GetBool())
		Msg("random float is %f shouldflinch = %d\n", f, bResult);
	return bResult;
}

//-----------------------------------------------------------------------------
// Purpose: Weapons ignore other weapons when LOS tracing
//-----------------------------------------------------------------------------
class CASWWeaponLOSFilter : public CTraceFilterSkipTwoEntities
{
	DECLARE_CLASS( CASWWeaponLOSFilter, CTraceFilterSkipTwoEntities );
public:
	CASWWeaponLOSFilter( IHandleEntity *pHandleEntity, IHandleEntity *pHandleEntity2, int collisionGroup ) :
		CTraceFilterSkipTwoEntities( pHandleEntity, pHandleEntity2, collisionGroup )
	{
	}
	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		CBaseEntity *pEntity = (CBaseEntity *)pServerEntity;

		if ( pEntity->GetCollisionGroup() == COLLISION_GROUP_WEAPON )
			return false;

		return BaseClass::ShouldHitEntity( pServerEntity, contentsMask );
	}
};

//-----------------------------------------------------------------------------
// Purpose: Check the weapon LOS for an owner at an arbitrary position
//			If bSetConditions is true, LOS related conditions will also be set
//-----------------------------------------------------------------------------
bool CASW_Weapon::WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
	bool bHasLOS = BaseClass::WeaponLOSCondition(ownerPos, targetPos, bSetConditions);

	// if the weapon has LOS, then do another wider trace to check we don't hit any friendlies
	//   this is to stop the AI marines shooting way too close to other marines, which stops the player thinking about positioning so much
	if (bHasLOS && GetOwner() && asw_weapon_safety_hull.GetFloat() > 0)
	{
		CAI_BaseNPC* npcOwner = GetOwner()->MyNPCPointer();	
		Vector vecRelativeShootPosition;
		VectorSubtract( npcOwner->Weapon_ShootPosition(), npcOwner->GetAbsOrigin(), vecRelativeShootPosition );	// Find its relative shoot position
		Vector barrelPos = ownerPos + vecRelativeShootPosition;	
		CASWWeaponLOSFilter traceFilter( GetOwner(), npcOwner->GetEnemy(), COLLISION_GROUP_BREAKABLE_GLASS );	// Use the custom LOS trace filter
		trace_t tr;
		UTIL_TraceHull( barrelPos, targetPos, Vector(-asw_weapon_safety_hull.GetFloat(), -asw_weapon_safety_hull.GetFloat(), -asw_weapon_safety_hull.GetFloat()),
						Vector(asw_weapon_safety_hull.GetFloat(), asw_weapon_safety_hull.GetFloat(), asw_weapon_safety_hull.GetFloat()),	MASK_SHOT, &traceFilter, &tr );

		if ( tr.fraction == 1.0 || tr.m_pEnt == npcOwner->GetEnemy() )	
			return true;

		// if a friendly is in the way, then we report failure
		CBaseCombatCharacter *pBCC = ToBaseCombatCharacter( tr.m_pEnt );
		if ( pBCC ) 
		{
			if ( npcOwner->IRelationType( pBCC ) == D_HT )
				return true;

			if ( bSetConditions )
			{
				npcOwner->SetCondition( COND_WEAPON_BLOCKED_BY_FRIEND );
			}
			return false;
		}
	}

	return bHasLOS;
}

ConVar rm_destroy_empty_weapon( "rm_destroy_empty_weapon", "1", FCVAR_NONE, "If 1 the weapon with 0 clips will be destroyed, e.g. medkit" );

bool CASW_Weapon::DestroyIfEmpty( bool bDestroyWhenActive, bool bCheckSecondaryAmmo )
{
	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )
		return false;

	bool bActive = ( pMarine->GetActiveASWWeapon() == this );
	if ( bActive && !bDestroyWhenActive )
		return false;

	if ( bCheckSecondaryAmmo && ( m_iClip2 || ( UsesClipsForAmmo2() &&  pMarine->GetAmmoCount(m_iSecondaryAmmoType) > 0 ) ) )
		return false;

#ifndef CLIENT_DLL
	// riflemod: commented weapon destruction on empty
	if ( rm_destroy_empty_weapon.GetBool() && !m_iClip1 && ( !UsesClipsForAmmo1() || pMarine->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 ) )
	{

		if ( rd_server_marine_backpacks.GetBool() && pMarine->GetASWWeapon(2) != this && pMarine->GetASWWeapon(ASW_TEMPORARY_WEAPON_SLOT) != this )
			pMarine->RemoveBackPackModel();

		pMarine->Weapon_Detach(this);
		if (bActive)
			pMarine->SwitchToNextBestWeapon(NULL);
		Kill();
		return true;

	}
#endif
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: Drop/throw the weapon with the given velocity.
//-----------------------------------------------------------------------------
void CASW_Weapon::Drop( const Vector &vecVelocity )
{
	// cancel any reload in progress.
	m_bInReload = false;

	StopAnimation();
	StopFollowingEntity( );
	SetMoveType( MOVETYPE_FLYGRAVITY );
	// clear follow stuff, setup for collision
	SetGravity(1.0);
	m_iState = WEAPON_NOT_CARRIED;
	RemoveEffects( EF_NODRAW );

	//If it was dropped then there's no need to respawn it.
	AddSpawnFlags( SF_NORESPAWN );

	FallInit();
	SetGroundEntity( NULL );
	SetTouch(NULL);

	IPhysicsObject *pObj = VPhysicsGetObject();
	if ( pObj != NULL )
	{
		AngularImpulse	angImp( 200, 200, 200 );
		pObj->AddVelocity( &vecVelocity, &angImp );
		
		// reactivedrop: force high mass for weapons, 
		// to prevent them flying away from explosions 
		// when marines dies and weapons are droped, we want him to be able
		// to pick them up if he is revived 
		if (pObj->GetMass() < 300)
			pObj->SetMass(300);

		//Msg("Weapon mass: %f \n", pObj->GetMass());
	}
	else
	{
		SetAbsVelocity( vecVelocity );
	}
	SetNextThink( gpGlobals->curtime + 1.0f );
	SetOwnerEntity( NULL );
	SetOwner( NULL );
	SetModel( GetWorldModel() );
	m_nSkin = GetWeaponInfo()->m_iPlayerModelSkin;
}


// player has used this item
void CASW_Weapon::ActivateUseIcon( CASW_Inhabitable_NPC *pNPC, int nHoldType )
{
	if ( nHoldType == ASW_USE_HOLD_START )
		return;

	CASW_Marine *pMarine = CASW_Marine::AsMarine( pNPC );
	if ( !pMarine )
		return;

	pMarine->TakeWeaponPickup( this );
	if ( pMarine->IsInhabited() && pMarine->GetCommander() )
	{
		pMarine->GetCommander()->m_hUseKeyDownEnt = NULL;
		pMarine->GetCommander()->m_flUseKeyDownTime = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// Is anyone carrying it?
//-----------------------------------------------------------------------------
bool CASW_Weapon::IsBeingCarried() const
{
	return ( GetOwner() != NULL );
}

//-----------------------------------------------------------------------------
bool CASW_Weapon::IsCarriedByLocalPlayer()
{
	if ( gpGlobals->maxClients <= 1 && !engine->IsDedicatedServer() )
	{
		CBaseCombatCharacter* pOwner = GetOwner();
		if ( pOwner && pOwner->Classify() == CLASS_ASW_MARINE )
		{
			CASW_Marine* pMarine = assert_cast<CASW_Marine*>( pOwner );
			return ( pMarine->IsInhabited() && pMarine->GetCommander() == UTIL_GetListenServerHost() );
		}
	}

	return false;
}

void CASW_Weapon::FallInit()
{
	BaseClass::FallInit();

	// We can't touch triggers if we are a trigger ourself.
	RemoveSolidFlags( FSOLID_TRIGGER );
}

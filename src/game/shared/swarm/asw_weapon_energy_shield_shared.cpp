#include "cbase.h"
#include "asw_weapon_energy_shield_shared.h"
#include "asw_gamerules.h"
#ifdef CLIENT_DLL
#include "c_asw_marine.h"
#else
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_fail_advice.h"
#endif
#include "asw_marine_skills.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
ConVar rd_energy_shield_burst_count( "rd_energy_shield_burst_count", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of shots in burst", true, 1, false, 255 );
ConVar rd_energy_shield_burst_rest_ratio( "rd_energy_shield_burst_rest_ratio", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "amount of time rifle spends idle between bursts (to have rest time match active time, set to 1)", true, 0, false, 0 );
ConVar rd_energy_shield_holster_burst( "rd_energy_shield_holster_burst", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "allow holstering to cancel burst" );
ConVar rd_energy_shield_holster_shield( "rd_energy_shield_holster_shield", "0", FCVAR_CHEAT | FCVAR_REPLICATED, "allow holstering to cancel shield" );
ConVar rd_energy_shield_activation_blocks_shooting( "rd_energy_shield_activation_blocks_shooting", "0.5", FCVAR_CHEAT | FCVAR_REPLICATED, "minimum delay between activating shield and shooting next burst" );
#ifdef GAME_DLL
ConVar rd_energy_shield_touch_interval( "rd_energy_shield_touch_interval", "0.75", FCVAR_CHEAT, "time between damage ticks for the energy shield's electric dissolve" );
#endif
extern ConVar rd_shield_rifle_dmg_base;

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Energy_Shield, DT_ASW_Weapon_Energy_Shield );

BEGIN_NETWORK_TABLE( CASW_Weapon_Energy_Shield, DT_ASW_Weapon_Energy_Shield )
#ifdef CLIENT_DLL
	RecvPropEHandle( RECVINFO( m_hShield ) ),
#else
	SendPropEHandle( SENDINFO( m_hShield ) ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( asw_weapon_energy_shield, CASW_Weapon_Energy_Shield );
PRECACHE_WEAPON_REGISTER( asw_weapon_energy_shield );

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CASW_Weapon_Energy_Shield )
	DEFINE_PRED_FIELD( m_hShield, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#else
BEGIN_DATADESC( CASW_Weapon_Energy_Shield )
	DEFINE_FIELD( m_hShield, FIELD_EHANDLE ),
END_DATADESC()
#endif

CASW_Weapon_Energy_Shield::CASW_Weapon_Energy_Shield()
{
	m_hShield = NULL;
}

CASW_Weapon_Energy_Shield::~CASW_Weapon_Energy_Shield()
{
#ifdef GAME_DLL
	if ( m_hShield.Get() )
	{
		UTIL_Remove( m_hShield );
		m_hShield = NULL;
	}
#endif
}

#ifdef CLIENT_DLL
const char *CASW_Weapon_Energy_Shield::GetTracerEffectName( int iShot )
{
	if ( iShot == 0 )
		return "tracer_shieldrifle_first";
	if ( iShot == GetBurstCount() - 1 )
		return "tracer_shieldrifle_last";
	return "tracer_shieldrifle";
}

const char *CASW_Weapon_Energy_Shield::GetMuzzleEffectName( int iShot )
{
	if ( iShot == 0 )
		return "muzzle_shieldrifle_first";
	if ( iShot == GetBurstCount() - 1 )
		return "muzzle_shieldrifle_last";
	return "muzzle_shieldrifle";
}

void CASW_Weapon_Energy_Shield::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

void CASW_Weapon_Energy_Shield::ClientThink()
{
	BaseClass::ClientThink();

	if ( m_iShieldPoseParameter == -2 )
	{
		m_iShieldPoseParameter = LookupPoseParameter( "energy_shield" );
	}

	if ( m_iShieldPoseParameter != -1 )
	{
		SetPoseParameter( m_iShieldPoseParameter, m_hShield.Get() ? 2.0f : 0.0f );
	}
}
#else
void CASW_Weapon_Energy_Shield::DestroyShield()
{
	Assert( m_hShield.Get() );
	if ( !m_hShield.Get() )
		return;

	// TODO: FX
	UTIL_Remove( m_hShield );
	m_hShield = NULL;
}
#endif

void CASW_Weapon_Energy_Shield::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem( "tracer_shieldrifle" );
	PrecacheParticleSystem( "tracer_shieldrifle_first" );
	PrecacheParticleSystem( "tracer_shieldrifle_last" );
	PrecacheParticleSystem( "muzzle_shieldrifle" );
	PrecacheParticleSystem( "muzzle_shieldrifle_first" );
	PrecacheParticleSystem( "muzzle_shieldrifle_last" );

#ifdef GAME_DLL
	UTIL_PrecacheOther( "asw_energy_shield_shield" );
#endif
}

void CASW_Weapon_Energy_Shield::SecondaryAttack()
{
	CASW_Player *pPlayer = GetCommander();
	if ( !pPlayer )
		return;

	CASW_Marine *pMarine = GetMarine();
	if ( !pMarine )
		return;

	bool bUsesSecondary = UsesSecondaryAmmo();
	bool bUsesClips = UsesClipsForAmmo2();
	int iAmmoCount = pMarine->GetAmmoCount( m_iSecondaryAmmoType );
	bool bInWater = ( pMarine->GetWaterLevel() == 3 );
	if ( m_hShield.Get() || ( bUsesSecondary && ( ( bUsesClips && m_iClip2 <= 0 ) || ( !bUsesClips && iAmmoCount <= 0 ) ) ) || bInWater || m_bInReload )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	// temp until secondary fire is a little more ready for testing
	SendWeaponAnim( ACT_VM_DRYFIRE );
	BaseClass::WeaponSound( EMPTY );
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
	return;

#ifndef CLIENT_DLL
	pMarine->OnWeaponFired( this, 1, true );

	CASW_Energy_Shield *pShield = assert_cast< CASW_Energy_Shield * >( CreateEntityByName( "asw_energy_shield_shield" ) );
	Assert( pShield );
	if ( pShield )
	{
		pShield->SetOwnerEntity( pMarine );
		pShield->SetParent( this );
		pShield->SetLocalOrigin( vec3_origin );
		pShield->SetLocalAngles( vec3_angle );
		pShield->ChangeTeam( pMarine->GetTeamNumber() );
		pShield->m_hCreatorMarine = pMarine;
		pShield->m_hCreatorWeapon = this;
		DispatchSpawn( pShield );
		m_hShield = pShield;
	}
#endif

	SendWeaponAnim( GetSecondaryAttackActivity() );

	pMarine->DoAnimationEvent( PLAYERANIMEVENT_FIRE_GUN_SECONDARY );

	if ( bUsesClips )
	{
		m_iClip2 -= 1;
	}
	else
	{
		pMarine->RemoveAmmo( 1, m_iSecondaryAmmoType );
	}

#ifndef CLIENT_DLL
	ASWFailAdvice()->OnMarineUsedSecondary();
#endif

	// shield activation interrupts burst fire for a short duration
	m_flNextPrimaryAttack = MAX( m_flNextPrimaryAttack, gpGlobals->curtime + rd_energy_shield_activation_blocks_shooting.GetFloat() );

	// wait a second before allowing the energy shield to be deployed again (it can't be deployed while it's out anyway)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
}

int CASW_Weapon_Energy_Shield::GetBurstCount() const
{
	return rd_energy_shield_burst_count.GetInt();
}

float CASW_Weapon_Energy_Shield::GetBurstRestRatio() const
{
	return rd_energy_shield_burst_rest_ratio.GetFloat();
}

bool CASW_Weapon_Energy_Shield::CanHolster()
{
	if ( !rd_energy_shield_holster_shield.GetBool() && m_hShield.Get() )
		return false;

	return BaseClass::CanHolster();
}

bool CASW_Weapon_Energy_Shield::HolsterCancelsBurstFire() const
{
	return rd_energy_shield_holster_burst.GetBool();
}

void CASW_Weapon_Energy_Shield::UpdateOnRemove( )
{
#ifdef GAME_DLL
	if ( m_hShield.Get() )
	{
		DestroyShield();
	}
#endif

	BaseClass::UpdateOnRemove();
}

bool CASW_Weapon_Energy_Shield::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef GAME_DLL
	if ( m_hShield.Get() )
	{
		DestroyShield();
	}
#endif

	return BaseClass::Holster( pSwitchingTo );
}

void CASW_Weapon_Energy_Shield::Drop( const Vector &vecVelocity )
{
#ifdef GAME_DLL
	if ( m_hShield.Get() )
	{
		DestroyShield();
	}
#endif

	BaseClass::Drop( vecVelocity );
}

float CASW_Weapon_Energy_Shield::GetWeaponBaseDamageOverride()
{
	return rd_shield_rifle_dmg_base.GetFloat();
}

int CASW_Weapon_Energy_Shield::GetWeaponSkillId()
{
	return ASW_MARINE_SKILL_ACCURACY;
}

int CASW_Weapon_Energy_Shield::GetWeaponSubSkillId()
{
	return ASW_MARINE_SUBSKILL_ACCURACY_SHIELD_RIFLE_DMG;
}

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Energy_Shield, DT_ASW_Energy_Shield );

BEGIN_NETWORK_TABLE( CASW_Energy_Shield, DT_ASW_Energy_Shield )
#ifdef CLIENT_DLL
	RecvPropEHandle( RECVINFO( m_hCreatorMarine ) ),
	RecvPropEHandle( RECVINFO( m_hCreatorWeapon ) ),
	RecvPropFloat( RECVINFO( m_flLastHitTime ) ),
	RecvPropFloat( RECVINFO( m_flExpireTime ) ),
#else
	SendPropEHandle( SENDINFO( m_hCreatorMarine ) ),
	SendPropEHandle( SENDINFO( m_hCreatorWeapon ) ),
	SendPropFloat( SENDINFO( m_flLastHitTime ), 32, SPROP_NOSCALE | SPROP_CHANGES_OFTEN ),
	SendPropFloat( SENDINFO( m_flExpireTime ), 32, SPROP_NOSCALE ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( asw_energy_shield_shield, CASW_Energy_Shield );

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CASW_Energy_Shield )
END_PREDICTION_DATA()
#else
BEGIN_DATADESC( CASW_Energy_Shield )
	DEFINE_FIELD( m_hCreatorMarine, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hCreatorWeapon, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flLastHitTime, FIELD_TIME ),
	DEFINE_FIELD( m_flExpireTime, FIELD_TIME ),
	DEFINE_THINKFUNC( TouchThink ),
END_DATADESC()
#endif

CASW_Energy_Shield::CASW_Energy_Shield()
{
	m_hCreatorMarine = NULL;
	m_hCreatorWeapon = NULL;

#ifdef GAME_DLL
	SetModelName( AllocPooledString( "models/items/shield_bubble/rifle_shield.mdl" ) );
#endif
}

CASW_Energy_Shield::~CASW_Energy_Shield()
{
}

#ifdef CLIENT_DLL
void CASW_Energy_Shield::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

void CASW_Energy_Shield::ClientThink()
{
	BaseClass::ClientThink();
}
#else
static constexpr const char *s_pTouchThink = "TouchThink";
void CASW_Energy_Shield::Spawn()
{
	AddEffects( EF_NOSHADOW );

	SetModel( STRING( GetModelName() ) );

	BaseClass::Spawn();

	SetCollisionGroup( ASW_COLLISION_GROUP_SHIELD );
	SetSolid( SOLID_VPHYSICS );
	AddSolidFlags( FSOLID_TRIGGER );
	SetMoveType( MOVETYPE_NONE );
	CreateVPhysics();
	SetNavIgnore();

	SetContextThink( &CASW_Energy_Shield::TouchThink, gpGlobals->curtime + rd_energy_shield_touch_interval.GetFloat(), s_pTouchThink );
}

bool CASW_Energy_Shield::CreateVPhysics()
{
	VPhysicsInitShadow( false, false );

	return true;
}

void CASW_Energy_Shield::StartTouch( CBaseEntity *pOther )
{
	BaseClass::StartTouch( pOther );

	if ( pOther == GetOwnerEntity() )
		return;

	if ( !ASWGameRules() || !ASWGameRules()->ShouldCollide( GetCollisionGroup(), pOther->GetCollisionGroup() ) )
		return;

	if ( m_vecTouching.Find( pOther ) != m_vecTouching.InvalidIndex() )
		return;

	m_vecTouching.AddToTail( pOther );
}
void CASW_Energy_Shield::EndTouch( CBaseEntity *pOther )
{
	BaseClass::EndTouch( pOther );

	m_vecTouching.FindAndRemove( pOther );
}

void CASW_Energy_Shield::TouchThink()
{
	if ( m_hCreatorMarine.Get() && m_hCreatorWeapon.Get() && m_vecTouching.Count() )
	{
		CTakeDamageInfo info( this, m_hCreatorMarine, m_hCreatorWeapon, MarineSkills()->GetSkillBasedValueByMarine( m_hCreatorMarine, ASW_MARINE_SKILL_ENGINEERING, ASW_MARINE_SUBSKILL_ENGINEERING_SHIELD_DAMAGE ), DMG_DISSOLVE | DMG_SHOCK );
		info.SetAmmoType( m_hCreatorWeapon->GetSecondaryAmmoType() );
		info.SetDamagePosition( m_hCreatorMarine->Weapon_ShootPosition() );

		FOR_EACH_VEC_BACK( m_vecTouching, i )
		{
			CBaseEntity *pEnt = m_vecTouching[i];
			if ( !pEnt )
			{
				m_vecTouching.Remove( i );
				continue;
			}

			if ( pEnt->m_takedamage == DAMAGE_NO )
			{
				continue;
			}

			pEnt->TakeDamage( info );
		}
	}

	SetContextThink( &CASW_Energy_Shield::TouchThink, gpGlobals->curtime + rd_energy_shield_touch_interval.GetFloat(), s_pTouchThink );
}

void CASW_Energy_Shield::OnProjectileHit( CBaseEntity *pProjectile )
{
	pProjectile->SetThink( NULL );
	pProjectile->SetMoveType( MOVETYPE_FLYGRAVITY );
	pProjectile->SetGravity( -0.02f );
	pProjectile->SetLocalVelocity( vec3_origin );
	pProjectile->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_ELECTRICAL_FAST, WorldSpaceCenter() );

	CASW_Marine_Resource *pMR = m_hCreatorMarine.Get() ? m_hCreatorMarine->GetMarineResource() : NULL;
	if ( pMR )
	{
		pMR->m_iEnergyShieldProjectilesDestroyed++;
	}

	m_flLastHitTime = gpGlobals->curtime;
}
#endif

void CASW_Energy_Shield::Precache()
{
	BaseClass::Precache();

	PrecacheModel( STRING( GetModelName() ) );
}
#endif

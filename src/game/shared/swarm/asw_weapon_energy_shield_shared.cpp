#include "cbase.h"
#include "asw_weapon_energy_shield_shared.h"
#include "asw_gamerules.h"
#ifdef CLIENT_DLL
#include "c_asw_marine.h"
#include "dlight.h"
#include "iefx.h"
#else
#include "asw_marine.h"
#include "asw_marine_resource.h"
#include "asw_fail_advice.h"
#include "particle_parse.h"
#include "soundenvelope.h"
#endif
#include "asw_marine_skills.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifdef RD_7A_WEAPONS
ConVar rd_energy_shield_burst_count( "rd_energy_shield_burst_count", "3", FCVAR_CHEAT | FCVAR_REPLICATED, "number of shots in burst", true, 1, false, 255 );
ConVar rd_energy_shield_burst_rest_ratio( "rd_energy_shield_burst_rest_ratio", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "amount of time rifle spends idle between bursts (to have rest time match active time, set to 1)", true, 0, false, 0 );
ConVar rd_energy_shield_burst_penalty_ratio( "rd_energy_shield_burst_penalty_ratio", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "multiple of overall fire rate player must wait between bursts to reset spread penalty" );
ConVar rd_energy_shield_burst_penalty_shield( "rd_energy_shield_burst_penalty_shield", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "minimum spread penalty when shield is active" );
ConVar rd_energy_shield_burst_sound_count( "rd_energy_shield_burst_sound_count", "10", FCVAR_CHEAT | FCVAR_REPLICATED, "number of consecutive bursts to get maximum pitch" );
ConVar rd_energy_shield_burst_sound_pitch( "rd_energy_shield_burst_sound_pitch", "50", FCVAR_CHEAT | FCVAR_REPLICATED, "maximum pitch change" );
ConVar rd_energy_shield_holster_burst( "rd_energy_shield_holster_burst", "1", FCVAR_CHEAT | FCVAR_REPLICATED, "allow holstering to cancel burst" );
ConVar rd_energy_shield_holster_shield( "rd_energy_shield_holster_shield", "2", FCVAR_CHEAT | FCVAR_REPLICATED, "allow holstering to cancel shield (2 = holster does not cancel, only pauses)" );
ConVar rd_energy_shield_activation_blocks_shooting( "rd_energy_shield_activation_blocks_shooting", "0.5", FCVAR_CHEAT | FCVAR_REPLICATED, "minimum delay between activating shield and shooting next burst" );
#ifdef GAME_DLL
ConVar rd_energy_shield_touch_interval( "rd_energy_shield_touch_interval", "0.2", FCVAR_CHEAT, "time between damage ticks for the energy shield's electric dissolve" );
ConVar rd_energy_shield_distance_min( "rd_energy_shield_distance_min", "40", FCVAR_CHEAT, "shield min distance in front of marine" );
ConVar rd_energy_shield_distance_max( "rd_energy_shield_distance_max", "70", FCVAR_CHEAT, "shield max distance in front of marine (it stays this far out unless there's a wall)" );
ConVar rd_energy_shield_height_min( "rd_energy_shield_height_min", "32", FCVAR_CHEAT, "shield height above floor when marine is aiming horizontally or lower" );
ConVar rd_energy_shield_height_max( "rd_energy_shield_height_max", "64", FCVAR_CHEAT, "shield height above floor when marine is aiming straight up" );
ConVar rd_energy_shield_move_speed( "rd_energy_shield_move_speed", "10", FCVAR_CHEAT, "speed multiplier for shield moving forward and backward" );
ConVar rd_energy_shield_ff( "rd_energy_shield_ff", "0", FCVAR_CHEAT, "does the energy shield damage friendly characters?" );
#endif
extern ConVar rd_shield_rifle_dmg_base;

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Weapon_Energy_Shield, DT_ASW_Weapon_Energy_Shield );

BEGIN_NETWORK_TABLE( CASW_Weapon_Energy_Shield, DT_ASW_Weapon_Energy_Shield )
#ifdef CLIENT_DLL
	RecvPropEHandle( RECVINFO( m_hShield ) ),
	RecvPropInt( RECVINFO( m_iConsecutiveBurstPenalty ) ),
	RecvPropFloat( RECVINFO( m_flResetConsecutiveBurstAfter ) ),
#else
	SendPropEHandle( SENDINFO( m_hShield ) ),
	SendPropInt( SENDINFO( m_iConsecutiveBurstPenalty ), 7, SPROP_UNSIGNED | SPROP_CHANGES_OFTEN ),
	SendPropFloat( SENDINFO( m_flResetConsecutiveBurstAfter ), 32, SPROP_NOSCALE | SPROP_CHANGES_OFTEN ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( asw_weapon_energy_shield, CASW_Weapon_Energy_Shield );
PRECACHE_WEAPON_REGISTER( asw_weapon_energy_shield );

#ifdef CLIENT_DLL
BEGIN_PREDICTION_DATA( CASW_Weapon_Energy_Shield )
	DEFINE_PRED_FIELD( m_hShield, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_iConsecutiveBurstPenalty, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL( m_flResetConsecutiveBurstAfter, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),
END_PREDICTION_DATA()
#else
BEGIN_DATADESC( CASW_Weapon_Energy_Shield )
	DEFINE_FIELD( m_hShield, FIELD_EHANDLE ),
	DEFINE_FIELD( m_iConsecutiveBurstPenalty, FIELD_INTEGER ),
	DEFINE_FIELD( m_flResetConsecutiveBurstAfter, FIELD_TIME ),
END_DATADESC()
#endif

CASW_Weapon_Energy_Shield::CASW_Weapon_Energy_Shield()
{
	m_hShield = NULL;
	m_iConsecutiveBurstPenalty = 0;
	m_flResetConsecutiveBurstAfter = 0.0f;

#ifdef CLIENT_DLL
	m_iShieldPoseParameter = -2;
	m_pOverheatEffect = NULL;
	m_pProjectEffect = NULL;
#endif
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

const char *CASW_Weapon_Energy_Shield::GetPartialReloadSound( int iPart )
{
	if ( iPart == 1 )
		return "ASW_Weapon_Energy_Shield.ReloadB";
	if ( iPart == 2 )
		return "ASW_Weapon_Energy_Shield.ReloadC";
	return "ASW_Weapon_Energy_Shield.ReloadA";
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

	int iOverheat = m_iConsecutiveBurstPenalty;
	if ( m_flResetConsecutiveBurstAfter <= gpGlobals->curtime )
	{
		iOverheat = 0;
	}
	// don't check for the shield; it only matters while firing (just having the shield doesn't overheat the barrel)

	if ( iOverheat == 0 && m_pOverheatEffect.GetObject() )
	{
		m_pOverheatEffect->StopEmission();
		m_pOverheatEffect = NULL;
	}
	else if ( iOverheat != 0 && !m_pOverheatEffect.GetObject() )
	{
		m_pOverheatEffect = ParticleProp()->Create( "shieldrifle_overheat", PATTACH_POINT_FOLLOW, "muzzle" );
	}

	if ( m_pOverheatEffect.GetObject() )
	{
		m_pOverheatEffect->SetControlPoint( 1, Vector( iOverheat, GetBulletSpread().x, 0 ) );
	}

	if ( m_iShieldPoseParameter == -2 )
	{
		m_iShieldPoseParameter = LookupPoseParameter( "energy_shield" );
	}

	if ( m_iShieldPoseParameter != -1 )
	{
		// TODO: animate?
		SetPoseParameter( m_iShieldPoseParameter, m_hShield.Get() ? 2.0f : 0.0f );
	}

	if ( !m_hShield.Get() && m_pProjectEffect.GetObject() )
	{
		m_pProjectEffect->StopEmission( false, true );
		m_pProjectEffect = NULL;
	}
	else if ( m_hShield.Get() && !m_pProjectEffect.GetObject() )
	{
		m_pProjectEffect = ParticleProp()->Create( "shieldrifle_project", PATTACH_POINT_FOLLOW, "emitter" );
	}
}
#else
void CASW_Weapon_Energy_Shield::DestroyShield( bool bViolent )
{
	Assert( m_hShield.Get() );
	if ( !m_hShield.Get() )
		return;

	CBaseEntity *pHelpHelpImBeingSupressed = ( CBaseEntity * )te->GetSuppressHost();
	te->SetSuppressHost( NULL );
	if ( bViolent )
	{
		CPASAttenuationFilter filter( m_hShield->GetAbsOrigin(), "ASW_Weapon_Energy_Shield.Shatter" );
		EmitSound( filter, entindex(), "ASW_Weapon_Energy_Shield.Shatter", &m_hShield->GetAbsOrigin() );
		DispatchParticleEffect( "energy_shield_dissipate_violent", m_hShield->GetAbsOrigin(), m_hShield->GetAbsAngles() );
	}
	else
	{
		CPASAttenuationFilter filter( m_hShield->GetAbsOrigin(), "ASW_Weapon_Energy_Shield.TurnOff" );
		EmitSound( filter, entindex(), "ASW_Weapon_Energy_Shield.TurnOff", &m_hShield->GetAbsOrigin() );
		DispatchParticleEffect( "energy_shield_dissipate", m_hShield->GetAbsOrigin(), m_hShield->GetAbsAngles() );
	}
	te->SetSuppressHost( pHelpHelpImBeingSupressed );

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
	PrecacheParticleSystem( "shieldrifle_overheat" );
	PrecacheParticleSystem( "shieldrifle_project" );
	PrecacheParticleSystem( "energy_shield_dissipate" );
	PrecacheParticleSystem( "energy_shield_appear" );

	PrecacheScriptSound( "ASW_Weapon_Energy_Shield.TurnOn" );
	PrecacheScriptSound( "ASW_Weapon_Energy_Shield.TurnOff" );
	PrecacheScriptSound( "ASW_Weapon_Energy_Shield.ReloadA" );
	PrecacheScriptSound( "ASW_Weapon_Energy_Shield.ReloadB" );
	PrecacheScriptSound( "ASW_Weapon_Energy_Shield.ReloadC" );

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

#ifndef CLIENT_DLL
	pMarine->OnWeaponFired( this, 1, true );

	CASW_Energy_Shield *pShield = assert_cast< CASW_Energy_Shield * >( CreateEntityByName( "asw_energy_shield_shield" ) );
	Assert( pShield );
	if ( pShield )
	{
		pShield->SetOwnerEntity( pMarine );
		pShield->SetParent( this );
		pShield->SetLocalOrigin( Vector( rd_energy_shield_distance_min.GetFloat(), 0, rd_energy_shield_height_min.GetFloat() ) );
		pShield->SetLocalAngles( QAngle( 0, 0, 0 ) );
		pShield->ChangeTeam( pMarine->GetTeamNumber() );
		pShield->m_hCreatorMarine = pMarine;
		pShield->m_hCreatorWeapon = this;
		pShield->m_flCreateTime = gpGlobals->curtime;
		pShield->m_takedamage = DAMAGE_YES;
		pShield->m_iMaxHealth = MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_ENGINEERING, ASW_MARINE_SUBSKILL_ENGINEERING_SHIELD_HEALTH );
		pShield->m_iHealth = pShield->m_iMaxHealth;
		pShield->m_flExpireTime = gpGlobals->curtime + MarineSkills()->GetSkillBasedValueByMarine( pMarine, ASW_MARINE_SKILL_ENGINEERING, ASW_MARINE_SUBSKILL_ENGINEERING_SHIELD_DURATION );
		DispatchSpawn( pShield );
		CBaseEntity *pHelpHelpImBeingSupressed = ( CBaseEntity * )te->GetSuppressHost();
		te->SetSuppressHost( NULL );
		pShield->EmitSound( "ASW_Weapon_Energy_Shield.TurnOn" );
		DispatchParticleEffect( "energy_shield_appear", PATTACH_ABSORIGIN_FOLLOW, pShield );
		te->SetSuppressHost( pHelpHelpImBeingSupressed );
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

void CASW_Weapon_Energy_Shield::OnStartedBurst()
{
	BaseClass::OnStartedBurst();

	if ( gpGlobals->curtime < m_flResetConsecutiveBurstAfter )
	{
		m_iConsecutiveBurstPenalty++;
	}
	else
	{
		m_iConsecutiveBurstPenalty = 0;
	}

	if ( m_hShield.Get() )
	{
		m_iConsecutiveBurstPenalty = MAX( m_iConsecutiveBurstPenalty, rd_energy_shield_burst_penalty_shield.GetInt() );
	}

	m_flResetConsecutiveBurstAfter = gpGlobals->curtime + GetFireRate() * rd_energy_shield_burst_penalty_ratio.GetFloat();
}

void CASW_Weapon_Energy_Shield::UpdateOnRemove()
{
#ifdef GAME_DLL
	if ( m_hShield.Get() )
	{
		DestroyShield( true );
	}
#endif

	BaseClass::UpdateOnRemove();
}

bool CASW_Weapon_Energy_Shield::Holster( CBaseCombatWeapon *pSwitchingTo )
{
#ifdef GAME_DLL
	if ( m_hShield.Get() && rd_energy_shield_holster_shield.GetInt() <= 1 )
	{
		DestroyShield( false );
	}
#endif

	return BaseClass::Holster( pSwitchingTo );
}

void CASW_Weapon_Energy_Shield::Drop( const Vector &vecVelocity )
{
#ifdef GAME_DLL
	if ( m_hShield.Get() )
	{
		DestroyShield( !GetMarine() || !GetMarine()->IsAlive() );
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

const Vector &CASW_Weapon_Energy_Shield::GetBulletSpread()
{
	static const Vector cones[] =
	{
		VECTOR_CONE_1DEGREES,
		VECTOR_CONE_1DEGREES,
		VECTOR_CONE_2DEGREES,
		VECTOR_CONE_4DEGREES,
		VECTOR_CONE_7DEGREES,
		VECTOR_CONE_10DEGREES,
		VECTOR_CONE_15DEGREES,
		VECTOR_CONE_20DEGREES,
	};

	return cones[clamp( m_iConsecutiveBurstPenalty, 0, NELEMS( cones ) - 1 )];
}

const char *CASW_Weapon_Energy_Shield::GetASWShootSound( int iIndex, int &iPitch )
{
	if ( iIndex == SINGLE || iIndex == SINGLE_NPC )
	{
		iPitch = RemapValClamped( m_iConsecutiveBurstPenalty, 0, rd_energy_shield_burst_sound_count.GetInt(), 100, 100 + rd_energy_shield_burst_sound_pitch.GetFloat() );
	}

	return BaseClass::GetASWShootSound( iIndex, iPitch );
}

IMPLEMENT_NETWORKCLASS_ALIASED( ASW_Energy_Shield, DT_ASW_Energy_Shield );

BEGIN_NETWORK_TABLE( CASW_Energy_Shield, DT_ASW_Energy_Shield )
#ifdef CLIENT_DLL
	RecvPropEHandle( RECVINFO( m_hCreatorMarine ) ),
	RecvPropEHandle( RECVINFO( m_hCreatorWeapon ) ),
	RecvPropFloat( RECVINFO( m_flLastHitTime ) ),
	RecvPropFloat( RECVINFO( m_flExpireTime ) ),
	RecvPropInt( RECVINFO( m_iHealth ) ),
	RecvPropInt( RECVINFO( m_iMaxHealth ) ),
#else
	SendPropEHandle( SENDINFO( m_hCreatorMarine ) ),
	SendPropEHandle( SENDINFO( m_hCreatorWeapon ) ),
	SendPropFloat( SENDINFO( m_flLastHitTime ), 32, SPROP_NOSCALE | SPROP_CHANGES_OFTEN ),
	SendPropFloat( SENDINFO( m_flExpireTime ), 32, SPROP_NOSCALE ),
	SendPropInt( SENDINFO( m_iHealth ) ),
	SendPropInt( SENDINFO( m_iMaxHealth ) ),
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
	DEFINE_SOUNDPATCH( m_pSoundLoop ),
	DEFINE_THINKFUNC( TouchThink ),
END_DATADESC()
#endif

CASW_Energy_Shield::CASW_Energy_Shield()
{
	m_hCreatorMarine = NULL;
	m_hCreatorWeapon = NULL;

#ifdef CLIENT_DLL
	m_pDLight = NULL;
	m_bWasHidden = false;
#else
	SetModelName( AllocPooledString( "models/items/shield_bubble/rifle_shield.mdl" ) );
	m_flCurrentPosition = 0.0f;
	m_pSoundLoop = NULL;
#endif
}

CASW_Energy_Shield::~CASW_Energy_Shield()
{
#ifdef CLIENT_DLL
	if ( m_pDLight && m_pDLight->key == entindex() )
	{
		m_pDLight->die = gpGlobals->curtime;
	}
#else
	if ( m_pSoundLoop )
	{
		CSoundEnvelopeController::GetController().SoundDestroy( m_pSoundLoop );
		m_pSoundLoop = NULL;
	}
#endif
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

	if ( !m_pDLight || m_pDLight->key != entindex() )
	{
		m_pDLight = effects->CL_AllocDlight( entindex() );
	}
	m_pDLight->origin = GetAbsOrigin() + Vector( 0, 0, 16 ) - Forward() * 16;
	m_pDLight->color.r = 220;
	m_pDLight->color.g = 180;
	m_pDLight->color.b = 40;
	m_pDLight->color.exponent = 1;
	m_pDLight->flags = 0;
	m_pDLight->radius = 130.0f;
	m_pDLight->die = m_flExpireTime;

	bool bHiddenNow = m_hCreatorMarine.Get() && ( m_hCreatorMarine->GetCurrentMeleeAttack() != NULL || m_hCreatorMarine->GetActiveASWWeapon() != m_hCreatorWeapon.Get() );
	if ( m_bWasHidden != bHiddenNow )
	{
		if ( bHiddenNow )
			DispatchParticleEffect( "energy_shield_dissipate_fast", PATTACH_ABSORIGIN_FOLLOW, this );
		else
			DispatchParticleEffect( "energy_shield_appear_fast", PATTACH_ABSORIGIN_FOLLOW, this );
		m_bWasHidden = bHiddenNow;
	}
}
#else
static constexpr const char *s_pPositionThink = "PositionThink";
static constexpr const char *s_pTouchThink = "TouchThink";
void CASW_Energy_Shield::Spawn()
{
	AddEffects( EF_NOSHADOW );

	SetModel( STRING( GetModelName() ) );

	BaseClass::Spawn();

	SetCollisionGroup( ASW_COLLISION_GROUP_SHIELD );
	SetSolid( SOLID_VPHYSICS );
	SetBloodColor( DONT_BLEED );
	CreateVPhysics();
	AddSolidFlags( FSOLID_TRIGGER | FSOLID_NOT_STANDABLE | FSOLID_TRIGGER_TOUCH_DEBRIS );

	SetMoveType( MOVETYPE_NONE );
	SetNavIgnore();

	CBroadcastRecipientFilter filter;
	m_pSoundLoop = CSoundEnvelopeController::GetController().SoundCreate( filter, entindex(), "ASW_Weapon_Energy_Shield.Loop" );
	CSoundEnvelopeController::GetController().Play( m_pSoundLoop, 1.0f, PITCH_NORM );

	SetContextThink( &CASW_Energy_Shield::PositionThink, gpGlobals->curtime, s_pPositionThink );
	SetContextThink( &CASW_Energy_Shield::TouchThink, gpGlobals->curtime + rd_energy_shield_touch_interval.GetFloat(), s_pTouchThink );
}

bool CASW_Energy_Shield::CreateVPhysics()
{
	VPhysicsInitShadow( false, false );

	return true;
}

void CASW_Energy_Shield::PhysicsSimulate()
{
	// don't reflect projectiles if we're not going to fizzle them
	if ( IsShieldInactive() )
	{
		AddSolidFlags( FSOLID_NOT_SOLID );
		CSoundEnvelopeController::GetController().SoundChangeVolume( m_pSoundLoop, 0.3f, 0.0f );
	}
	else
	{
		RemoveSolidFlags( FSOLID_NOT_SOLID );
		CSoundEnvelopeController::GetController().SoundChangeVolume( m_pSoundLoop, 1.0f, 0.0f );
	}

	BaseClass::PhysicsSimulate();
}

bool CASW_Energy_Shield::IsShieldInactive() const
{
	if ( !m_hCreatorMarine.Get() )
		return true;
	if ( !m_hCreatorWeapon.Get() )
		return true;
	if ( m_hCreatorMarine->GetActiveASWWeapon() != m_hCreatorWeapon.Get() )
		return true;

	if ( m_hCreatorMarine->GetCurrentMeleeAttack() )
		return true;
	if ( m_hCreatorWeapon->IsFiring() )
		return true;

	return false;
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

	if ( !IsShieldInactive() && CheckProjectileHit( pOther ) )
		return;

	m_vecTouching.AddToTail( pOther );
}
void CASW_Energy_Shield::EndTouch( CBaseEntity *pOther )
{
	BaseClass::EndTouch( pOther );

	m_vecTouching.FindAndRemove( pOther );
}

int CASW_Energy_Shield::OnTakeDamage( const CTakeDamageInfo &info )
{
	int result = BaseClass::OnTakeDamage( info );

	if ( result )
	{
		EmitSound( "ASW_Weapon_Energy_Shield.Damaged" );
		m_flLastHitTime = gpGlobals->curtime;
	}

	return result;
}
void CASW_Energy_Shield::Event_Killed( const CTakeDamageInfo &info )
{
	if ( m_hCreatorWeapon.Get() )
	{
		m_hCreatorWeapon->DestroyShield( true );
	}

	BaseClass::Event_Killed( info );

	UTIL_Remove( this );
}

void CASW_Energy_Shield::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	float flDot = DotProduct( vecDir, Forward() );
	if ( flDot > 0 )
	{
		// ignore hits from the back
		return;
	}

	BaseClass::TraceAttack( info, vecDir, ptr );
}

void CASW_Energy_Shield::PositionThink()
{
	const float flInterval = 0.1f;

	if ( !IsShieldInactive() )
	{
		Vector gunOrigin = GetMoveParent()->GetAbsOrigin();
		QAngle gunAngles = GetMoveParent()->GetAbsAngles();
		QAngle aimAngle = m_hCreatorMarine->ASWEyeAngles();
		Vector aimForward, aimLeft;

		gunOrigin.z += 16;
		AngleVectors( aimAngle, &aimForward, &aimLeft, NULL );
		float flVerticalAim = clamp( aimForward.z, 0, 1 );
		// flatten
		aimForward.z = aimLeft.z = 0;
		aimForward.NormalizeInPlace();
		aimLeft.NormalizeInPlace();

		trace_t tr0, tr1, tr2;
		CTraceFilterSimple filter( NULL, COLLISION_GROUP_NONE );
		const float flPad = 4.0f;
		const float flSide = 0.5f;
		UTIL_TraceLine( gunOrigin + aimForward * ( rd_energy_shield_distance_min.GetFloat() + flPad ), gunOrigin + aimForward * ( rd_energy_shield_distance_max.GetFloat() + flPad ), MASK_SOLID_BRUSHONLY, &filter, &tr0 );
		UTIL_TraceLine( gunOrigin + ( aimForward + aimLeft * flSide ) * ( rd_energy_shield_distance_min.GetFloat() + flPad ), gunOrigin + ( aimForward + aimLeft * flSide ) * ( rd_energy_shield_distance_max.GetFloat() + flPad ), MASK_SOLID_BRUSHONLY, &filter, &tr1 );
		UTIL_TraceLine( gunOrigin + ( aimForward - aimLeft * flSide ) * ( rd_energy_shield_distance_min.GetFloat() + flPad ), gunOrigin + ( aimForward - aimLeft * flSide ) * ( rd_energy_shield_distance_max.GetFloat() + flPad ), MASK_SOLID_BRUSHONLY, &filter, &tr2 );
		float fraction = MIN( MIN( tr0.fraction, tr1.fraction ), tr2.fraction );
		if ( m_flCurrentPosition > fraction )
			m_flCurrentPosition = fraction;
		else
			m_flCurrentPosition = Approach( fraction, m_flCurrentPosition, flInterval * rd_energy_shield_move_speed.GetFloat() );

		float flDist = Lerp( m_flCurrentPosition, rd_energy_shield_distance_min.GetFloat(), rd_energy_shield_distance_max.GetFloat() );
		float flHeight = Lerp( flVerticalAim, rd_energy_shield_height_min.GetFloat(), rd_energy_shield_height_max.GetFloat() );
		QAngle localAngles( 0, aimAngle.y - gunAngles.y, 0 );
		Vector localForward;
		AngleVectors( localAngles, &localForward );
		Vector vecOrigin = localForward * flDist;
		vecOrigin.z += flHeight;

		SetLocalOrigin( vecOrigin );
		SetLocalAngles( localAngles );
	}

	if ( gpGlobals->curtime > m_flExpireTime )
	{
		m_hCreatorWeapon->DestroyShield( false );
	}

	SetContextThink( &CASW_Energy_Shield::PositionThink, gpGlobals->curtime + flInterval, s_pPositionThink );
}

void CASW_Energy_Shield::TouchThink()
{
	if ( !IsShieldInactive() )
	{
		CTakeDamageInfo info( this, m_hCreatorMarine, m_hCreatorWeapon, MarineSkills()->GetSkillBasedValueByMarine( m_hCreatorMarine, ASW_MARINE_SKILL_ACCURACY, ASW_MARINE_SUBSKILL_ACCURACY_SHIELD_DMG ), DMG_DISSOLVE );
		info.SetAmmoType( m_hCreatorWeapon->GetSecondaryAmmoType() );
		info.SetDamagePosition( m_hCreatorMarine->Weapon_ShootPosition() );
		info.SetDamageForce( m_hCreatorMarine->Forward() );

		FOR_EACH_VEC_BACK( m_vecTouching, i )
		{
			CBaseEntity *pEnt = m_vecTouching[i];
			if ( !pEnt )
			{
				m_vecTouching.Remove( i );
				continue;
			}

			if ( CheckProjectileHit( pEnt ) )
			{
				continue;
			}

			if ( pEnt->m_takedamage != DAMAGE_YES )
			{
				continue;
			}

			if ( !rd_energy_shield_ff.GetBool() && m_hCreatorMarine->IRelationType( pEnt ) == D_LI )
			{
				continue;
			}

			trace_t tr;
			CTraceFilterOnlyHitThis filter( pEnt );
			UTIL_TraceLine( WorldSpaceCenter(), pEnt->GetAutoAimCenter(), MASK_ALL, &filter, &tr );
			if ( tr.fraction < 1.0f )
			{
				bool bWasDead = pEnt->m_lifeState == LIFE_DEAD;
				pEnt->DispatchTraceAttack( info, Forward(), &tr );
				ApplyMultiDamage();
				if ( pEnt->m_lifeState == LIFE_DYING || ( !bWasDead && pEnt->m_lifeState == LIFE_DEAD ) )
				{
					CPASAttenuationFilter sndfilter( tr.endpos, "ASW_Weapon_Energy_Shield.BurnKill" );
					EmitSound( sndfilter, entindex(), "ASW_Weapon_Energy_Shield.BurnKill", &tr.endpos );
				}
				else
				{
					CPASAttenuationFilter sndfilter( tr.endpos, "ASW_Weapon_Energy_Shield.Burn" );
					EmitSound( sndfilter, entindex(), "ASW_Weapon_Energy_Shield.Burn", &tr.endpos );
				}
				m_flLastHitTime = gpGlobals->curtime;
			}
		}
	}

	SetContextThink( &CASW_Energy_Shield::TouchThink, gpGlobals->curtime + rd_energy_shield_touch_interval.GetFloat(), s_pTouchThink );
}

bool CASW_Energy_Shield::CheckProjectileHit( CBaseEntity *pProjectile )
{
	if ( pProjectile->Classify() == CLASS_ASW_DOOR )
	{
		// don't hurt doors at all
		return true;
	}

	if ( pProjectile->GetGroundEntity() )
	{
		// don't fizzle grounded projectiles
		return false;
	}

	if ( pProjectile->Classify() == CLASS_ASW_MORTAR_SHELL ||
		pProjectile->GetCollisionGroup() == COLLISION_GROUP_PROJECTILE ||
		pProjectile->GetCollisionGroup() == ASW_COLLISION_GROUP_NPC_GRENADES ||
		pProjectile->GetCollisionGroup() == ASW_COLLISION_GROUP_ALIEN_MISSILE ||
		pProjectile->GetCollisionGroup() == HL2COLLISION_GROUP_COMBINE_BALL )
	{
		OnProjectileHit( pProjectile );
		return true;
	}

	return false;
}

void CASW_Energy_Shield::OnProjectileHit( CBaseEntity *pProjectile )
{
	if ( IsShieldInactive() )
		return;

	CBaseEntity *pSimple = CreateEntityByName( "simple_physics_prop" );
	if ( pSimple )
	{
		pSimple->SetAbsOrigin( pProjectile->GetAbsOrigin() );
		pSimple->SetAbsAngles( pProjectile->GetAbsAngles() );
		pSimple->SetLocalVelocity( pProjectile->GetLocalVelocity() / 16.0f );
		pSimple->SetLocalAngularVelocity( pProjectile->GetLocalAngularVelocity() );
		pSimple->SetModelIndex( pProjectile->GetModelIndex() );
		if ( pProjectile->GetBaseAnimating() )
		{
			pSimple->GetBaseAnimating()->m_nBody = pProjectile->GetBaseAnimating()->m_nBody;
			pSimple->GetBaseAnimating()->m_nSkin = pProjectile->GetBaseAnimating()->m_nSkin;
		}
		pSimple->SetMoveType( MOVETYPE_FLYGRAVITY );
		pSimple->SetSolid( SOLID_NONE );
		pSimple->SetGravity( -0.02f );
		pSimple->GetBaseAnimating()->Dissolve( NULL, gpGlobals->curtime, false, ENTITY_DISSOLVE_NORMAL, WorldSpaceCenter() );
		UTIL_Remove( pProjectile );
	}

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

	PrecacheParticleSystem( "energy_shield_dissipate_fast" );
	PrecacheParticleSystem( "energy_shield_dissipate_violent" );
	PrecacheParticleSystem( "energy_shield_appear_fast" );

	PrecacheScriptSound( "ASW_Weapon_Energy_Shield.Loop" );
	PrecacheScriptSound( "ASW_Weapon_Energy_Shield.Burn" );
	PrecacheScriptSound( "ASW_Weapon_Energy_Shield.BurnKill" );
	PrecacheScriptSound( "ASW_Weapon_Energy_Shield.Shatter" );
	PrecacheScriptSound( "ASW_Weapon_Energy_Shield.Damaged" );
}
#endif
